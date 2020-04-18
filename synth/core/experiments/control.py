#!/usr/bin/env python3

from collections import namedtuple
from copy import copy
from enum import Enum, auto

from relations import Relation, Universe


class _named:
    def __init__(self):
        self._name = type(self).__name__
        super().__init__()

    def name(self, name):
        self._name = name
        return self

    def __repr__(self):
        return self._name


class _typed:
    def __init__(self, typ=float):
        self.type = typ
        super().__init__()


class Port:
    def __repr__(self):
        for (k, v) in self.owner.__dict__.items():
            if v == self:
                return f'{self.owner}.{k.strip("_")}'

        return type(self).__name__

class InputPort(Port): ...

class Input(InputPort, _typed): ...

class OutputPort(Port): ...

class Output(OutputPort, _typed): ...


class Ported:
    @property
    def ports(self):
        return iter(self._ports)

    @ports.setter
    def ports(self, ports):
        self._ports = ports
        for p in ports:
            p.owner = self

    @property
    def input_ports(self):
        return (p for p in self.ports if isinstance(p, InputPort))

    @property
    def output_ports(self):
        return (p for p in self.ports if isinstance(p, OutputPort))


class Control(_typed, _named, Ported):
    def __init__(self, typ=float):
        super().__init__(typ)
        self.out = Output(typ)
        self.ports = (self.out, )

class MIDINotePitchControl(Control): ...

class MIDIModulationControl(Control): ...

class MIDIExpressionControl(Control): ...


class Module(_named, Ported): ...


class Link: ...

class LinkType(Link):
    def __init__(self, d_type, s_type, c_type, dest, src, ctl, scale=1):
        self.d_type = d_type
        self.s_type = s_type
        self.c_type = c_type
        self.dest = dest
        self.src = src
        self.ctl = ctl
        self.scale = scale
    def __repr__(self):
        def type_name(x_type):
            if x_type == type(None):
                return '-'
            return x_type.__name__
        if self.c_type not in (float, type(None)):
            nt = 3
        elif self.s_type not in (float, type(None)):
            nt = 2
        elif self.d_type is not float:
            nt = 1
        else:
            nt = 0
        rep = f'Link<'
        if nt >= 1:
            rep += f'{type_name(self.d_type)}'
        if nt >= 2:
            rep += f', {type_name(self.s_type)}'
        if nt >= 3:
            rep += f', {type_name(self.c_type)}'
        rep += f'>({self.dest}'
        if self.src or self.ctl:
            rep += f', {self.src}'
            if self.ctl:
                rep += f', {self.ctl}'
        if self.scale != 1:
            rep += f', scale={self.scale}'
        rep += ')'
        return rep

    def is_simple(self):
        return (self.src
                and not self.ctl
                and self.src.type == self.dest.type
                and self.scale == 1)

def make_link(dest, src=None, ctl=None, scale=1):
    d_type = dest.type
    s_type = src.type if src else type(None)
    c_type = ctl.type if ctl else type(None)

    return LinkType(d_type, s_type, c_type, dest, src, ctl, scale)


# The plan has a sequence of actions
# The actions have indices for modules, ports, and controls.
# The links know the ports' types.
# The actions hold the links.
# The plan.make_foo method can map indices to objects.
#
# Given action:
#   action -> needed indices (action)
#   indices -> objects (make_foo)
#   objects -> function (action)
#

class Action(_named): ...

class EvalAction(Action):
    def __init__(self, ctl):
        self.ctl = ctl
    def __repr__(self):
        return f'eval({self.ctl})'

class ClearAction(Action):
    def __init__(self, port, scale):
        self.port = port
        self.scale = scale
    def __repr__(self):
        return f'clear({self.port}, {self.scale})'
    def make_exec(self, lookup):
        port = lookup(self.dest_port_index)
        buf = port.buf
        n = len(buf)
        value = type(buf[0])(self.scale)
        def exec():
            for i in range(n):
                buf[i] = value
        return exec

class AliasAction(Action):
    def __init__(self, dest, src):
        self.dest = dest
        self.src = src
    def __repr__(self):
        return f'alias({self.dest}, {self.src})'

class CopyAction(Action):
    def __init__(self, dest, src, ctl, link):
        self.dest = dest
        self.src = src
        self.ctl = ctl
        self.link = link
    def __repr__(self):
        return f'copy({self.dest}, {self.src}, {self.ctl})'

class AddAction(Action):
    def __init__(self, dest, src, ctl, link):
        self.dest = dest
        self.src = src
        self.ctl = ctl
        self.link = link
    def __repr__(self):
        return f'add({self.dest}, {self.src}, {self.ctl})'

class RenderAction(Action):
    def __init__(self, mod):
        self.mod = mod
    def __repr__(self):
        return f'render({self.mod})'


class Resolver:
    def __init__(self, controls, modules):
        self.controls = Universe(controls)
        self.modules = Universe(modules)
        self.ports = Universe(self._collect_ports())

    def _collect_ports(self):
        ports = []
        for c in self.controls:
            ports.append(c.out)
        for m in self.modules:
            ports.extend(m.ports)
        return ports


class Plan(namedtuple('Plan', 't_prep v_prep pre_run v_run post_run')):

    def prep_timbre(self, timbre):
        resolver = Resolver(timbre.controls, timbre.modules)
        for a in self.tprep:
            a.do_prep(resolver)
    def prep_voice(self, voice, timbre): ...
    def make_timbre_exec(self, timbre): ...
    def make_voice_exec(self, voice, timbre): ...


# InputPort<T> should have a clear method.
# InputPort<T> should have an alias/unalias method.

# Mod Network
# make plan...
#   starting w/ main output, work backward to find reachable modules/controls.
#   classify modules as pre-voice, voice, and post-voice.
#   generate prep and run for each set.
# the plan consists of...
#   a resolver
#   two prep lists (timbre, voice)
#   three run lists (pre-voice, voice, post-voice)
# That's a lot.
# Controls have no inputs.  They can always be voice or pre-voice.
# Controls can always be rendered before modules of the same scope.
#
# Data structures
#   module and control masks for reachable modules and controls
#   module adjacency matrix
#   port source matrix


# Algorithm.
#
# Inputs:
#    timbre controls
#    timbre modules
#    voice controls
#    voice modules
#    links
# Outputs:
#    timbre prep plan
#    voice prep plan
#    pre-voice run plan
#    voice run plan
#    post-voice run plan
#
# build module predecessor matrix.
# build port source matrix.
# build module control matrix.
#
# # collect used modules
# def collect(succ_set, modules):
#     modules = {}
#     while True:
#         pred = {}
#         for m in modules:
#             pred |= predecessors[m]
#         return pred & ~m_set
# post_v_modules = collect({main_output}, t_modules)
# v_modules = collect(post_v_modules, v_modules)
# pre_v_modules = collecxt(v_modules, t_modules)
# modules_used = post_v | pre_v | v
#
# XXX How can we test whether any paths v->pre or post->v exist?
# Verify pre_v_modules is disjoint from post_v_modules.
#
# # collect controls
# controls_used = {}
# for m in modules_used:
#     for p in m.ports():
#         if p.ctl is a control:
#             controls_used.add(p.ctl)
# t_controls_used = {c for c in controls_used if c.is_timbre}
# v_controls_used = {c for c in controls_used if c.is_voice}

# # build pre-voice prep plan:
# tprep = []
# for c in t_controls_used:
#     tprep.append(evaluate(c))
# for m in pre_v_modules:
#     for p in m.input_ports:
#         if p has no connections:
#             clear(p)
#         else if p has one simple connection:
#             alias(p, src)
#         else
#             unalias(p)
# for m in post_v_modules:
#     for p in m.input_ports:
#         if p has source or ctl in v:
#             clear(p)
#
# # build voice prep plan
# vprep = []
# for c in v_controls_used:
#     vprep.append(evaluate(c))
# for m in v_modules:
#     for p in m.input_ports:.,
#         if p has no connections:
#             clear(p)
#         etc. etc. etc.
#         you know how this goes

class ModNetwork:

    def __init__(self, tc, tm, vc, vm, links, cvalues):
        self.tcontrols = tc
        self.tmodules = tm
        self.vcontrols = vc
        self.vmodules = vm
        self.links = links
        self.cvalues = cvalues
        self.resolver = Resolver(tc + vc, tm + vm)
        self._all_links = self._collect_all_links()

    def make_plan(self, output_modules):
        self._mod_predecessors = self._calc_mod_predecessors()
        self._port_sources = self._calc_port_sources()
        self._links_to = self._calc_links_to()

        # partition reeachable modules into pre, voice, and post.
        mod_parts = self._partition_modules_used(output_modules)

        # find reachable controls
        controls_used = self._find_controls_used()

        # assemble timbre prep actions
        t_prep = self._assemble_prep_actions(controls_used.timbre,
                                             mod_parts.pre | mod_parts.post)

        # assemble voice prep actions
        v_prep = self._assemble_prep_actions(controls_used.voice,
                                             mod_parts.voice)

        # assemble pre run actions.
        no_modules = self.resolver.modules.none()
        pre_run = self._assemble_run_actions(controls_used.timbre,
                                             mod_parts.pre,
                                             no_modules)

        # assemble voice run actions
        v_run = self._assemble_run_actions(controls_used.voice,
                                           mod_parts.voice, mod_parts.pre)

        # assemble post run actions.
        no_controls = self.resolver.controls.none()
        post_run = self._assemble_run_actions(no_controls,
                                              mod_parts.post,
                                              mod_parts.pre | mod_parts.voice)

        return Plan(
            t_prep=t_prep,
            v_prep=v_prep,
            pre_run=pre_run,
            v_run=v_run,
            post_run=post_run
            )

    def _partition_modules_used(self, output_modules):
        # partition reachable modules into pre-voice, voice, and post-voice.
        outputs_used = self.resolver.modules.subset(output_modules)
        all_tmods = self.resolver.modules.subset(self.tmodules)
        all_vmods = self.resolver.modules.subset(self.vmodules)

        post_mods = outputs_used | self._collect_pred(outputs_used, all_tmods)
        voice_mods = self._collect_pred(post_mods, all_vmods)
        assert voice_mods <= all_vmods
        pre_mods = self._collect_pred(voice_mods, all_tmods)
        assert not (pre_mods & post_mods)
        assert (pre_mods | post_mods) <= all_tmods
        ModuleParts = namedtuple('ModuleParts', 'pre voice post')
        return ModuleParts(pre_mods, voice_mods, post_mods)

    def _find_controls_used(self):
        tcontrols_used = self.resolver.controls.none()
        vcontrols_used = self.resolver.controls.none()
        for link in self.links:
            if link.ctl and isinstance(link.ctl.owner, Control):
                if link.ctl.owner in self.tcontrols:
                    tcontrols_used.add(link.ctl.owner)
                if link.ctl.owner in self.vcontrols:
                    vcontrols_used.add(link.ctl.owner)
        ControlsUsed = namedtuple('CtlsUsed', 'timbre voice')
        return ControlsUsed(tcontrols_used, vcontrols_used)

    def _assemble_prep_actions(self, controls, modules):
        prep = []
        for ci in controls.iter_bits():
            prep.append(EvalAction(ci))
        for m in modules.iter_members():
            for p in m.input_ports:
                link_count = 0
                s_link = None
                for link in self._links_to.get(p).iter_members():
                    link_count += 1
                    if link.is_simple() and link.src.owner in modules:
                        s_link = link
                di = self.resolver.ports.index(p)
                if link_count == 0:
                    prep.append(ClearAction(di, 0))
                elif link_count == 1 and s_link:
                    si = self.resolver.ports.index(s_link.src)
                    prep.append(AliasAction(di, si))
                else:
                    prep.append(AliasAction(di, -1))
        return prep

    def _assemble_run_actions(self, controls, section, done):
        run = []
        for c in controls.iter_bits():
            run.append(EvalAction(c))
        while section - done:
            ready = self.resolver.modules.none()
            for mi in section.iter_bits():
                m = self.resolver.modules[mi]
                if m in done:
                    continue
                mod_preds = self._mod_predecessors.at(mi)
                if mod_preds <= done:
                    ready.add(m)
            if ready == 0:
                raise RuntimeError("can't compute graph")
            for m in ready.iter_members():
                mi = self.resolver.modules.index(m)
                for dest in m.input_ports:
                    di = self.resolver.ports.index(dest)
                    copied = False
                    for link in self._links_to.at(di).iter_members():
                        si = self.resolver.ports.find(link.src)
                        if link.is_simple():
                            links = self._links_to.at(di)
                            if len(links) == 1:
                                # skip singke simple links
                                continue
                        ci = self.resolver.ports.find(link.ctl)
                        if not copied:
                            run.append(CopyAction(di, si, ci, link))
                            copied = True
                        else:
                            run.append(AddAction(di, si, ci, link))
                run.append(RenderAction(mi))

            done |= ready
        return run

    def _collect_pred(self, succ, candidates):
        """collect direct and indirect predecessors of `succ`
           that are in `candidates`
        """
        pred = self.resolver.modules.none()
        cur = succ
        while True:
            prev = self.resolver.modules.none()
            for mi in cur.iter_bits():
                prev |= self._mod_predecessors.at(mi)
            prev &= candidates
            if prev == 0:
                break
            pred |= prev
            cur = prev
        return pred

    def _collect_all_links(self):
        return Universe(self.links)

    def _calc_mod_predecessors(self):
        predecessors = Relation(self.resolver.modules, self.resolver.modules)
        for link in self.links:
            dest_mod = link.dest.owner
            if link.src:
                src_mod = link.src.owner
                predecessors.add(dest_mod, src_mod)
            if link.ctl:
                ctl_mod = link.ctl.owner
                if isinstance(ctl_mod, Module):
                    predecessors.add(dest_mod, ctl_mod)
        return predecessors

    def _calc_port_sources(self):
        port_sources = Relation(self.resolver.ports, self.resolver.ports)
        for link in self.links:
            if link.src:
                port_sources.add(link.dest, link.src)
            if link.ctl:
                port_sources.add(link.dest, link.ctl)
        return port_sources

    def _calc_links_to(self):
        links_to = Relation(self.resolver.ports, self._all_links)
        for link in self._all_links:
            links_to.add(link.dest, link)
        return links_to

# The module and control sets belong to the synth.
# The links belong to the patches.
# The timbre has a copy of the per-timbre modules and controls.
# The voice has a copy of the per-voice modules and controls.
# The timbre has a patch and a plan.
#
# A mod network needs the modules, controls, links, and values.
#
# Fine.
#     class Synth:
#         def apply_patch(patch, timbre):
#             timbre.patch = patch
#             timbre.plan = self._make_plan(patch)


class VoiceState(Enum):
    Idle = auto()
    Active = auto()
    Releasing = auto()
    Stopping = auto()

class Voice:
    # Voices are static.  N voices are created at startup
    # and allocated to timbres as needed.
    # Voice has state, owning timbre, modules, controls, exec.
    def __init__(self, vmodules, vcontrols):
        self.state = VoiceState.Idle
        self.vmodules = vmodules
        self.vcontrols = vcontrols
        self.vexec = None

    def start_note(self, note):
        self.note = note
        self.state = VoiceState.Active

    def release_note(self):
        ...

    def kill_note(self):
        ...

class Timbre(_named):
    # Timbres are static.  N timbres are created at startup.
    # A Timbre has:
    #     a patch reference
    #     a "factory default" patch
    #     a plan
    #     per timbre modules
    #     per timbre controls
    #     an exec
    # A Timbre can:
    #     create a patch
    #     apply a patch
    # The plan is recomputed when the patch changes (replaced or modified).
    # To compute the plan, the timbre needs reference to the voice moudles
    # and controls.

    def __init__(self, tmodules, tcontrols):
        self.patch = None
        self.plan = None
        self.tmodules = tmodules
        self.tcontrols = tcontrols
    def init_voice(self, voice): ...
    def update_voice(self, voice): ...
    # def create_patch(self): ...
    # def apply_patch(self, patch): ...


class Patch:
    # A Patch is dynamic.  Can be loaded, saved, modified.
    # A Patch affects one Timbre.
    # Patch has:
    #     ref to a synth
    #     a set of ControlLinks
    #     values for all Controls
    def __init__(self, synth):
        self.synth = synth
        self.links = []
        self.cvalues = []
    def connect(self, dest, src=None, ctl=None, scale=1):
        assert isinstance(dest, InputPort)
        assert src is None or isinstance(src, OutputPort)
        assert ctl is None or isinstance(ctl, (OutputPort, Control))
        if isinstance(ctl, Control):
            ctl = ctl.out
        self.links.append(make_link(dest, src, ctl, scale))
        return self


class Synth(_named):
    # Synth has:
    #     polyphony count
    #     per voice modules
    #     per timbre modules
    #     per voice controls
    #     per timbre controls
    #     XXX NO permanent connections
    #     ref to an output port
    #
    # Synth can:
    #     XXX NO create patch
    #     XXX NO create plan from patch
    #     allocate voices
    def __init__(self, polyphony=1, timbrality=1):
        super().__init__()
        self.finalized   = False
        self.polyphony   = polyphony
        self.timbrality  = timbrality
        self.tmodules    = []
        self.vmodules    = []
        self.tcontrols   = []
        self.vcontrols   = []
        self.omodules    = []
        self.voice_alloc = None

    # construction
    def voice_allocator(self, valloc):
        self.voice_alloc = valloc
        return self

    def timbre_module(self, module):
        assert not self.finalized
        self.tmodules.append(module)
        return self

    def output_module(self, module):
        assert not self.finalized
        self.omodules.append(module)
        self.tmodules.append(module)
        return self

    def voice_module(self, module):
        assert not self.finalized
        self.vmodules.append(module)
        return self

    def tcontrol(self, control):
        assert not self.finalized
        self.tcontrols.append(control)
        return self

    def vcontrol(self, control):
        assert not self.finalized
        self.vcontrols.append(control)
        return self

    def finalize(self):
        assert not self.finalized
        self.finalized = True
        self.voices = [Voice(vmodules=copy(self.vmodules),
                             vcontrols=copy(self.vcontrols))
                       for i in range(self.polyphony)]
        self.timbres = [Timbre(tmodules=copy(self.tmodules),
                               tcontrols=copy(self.tcontrols))
                        .name(chr(ord('A') + i))
                        for i in range(self.timbrality)]
        return self

    def make_patch(self):
        assert self.finalized
        return Patch(self)

    def apply_patch(self, patch, timbre):
        timbre.patch = patch
        timbre.plan = self._make_plan(patch)

    def _make_plan(self, patch):
        mod_network = ModNetwork(self.tcontrols,
                                 self.tmodules,
                                 self.vcontrols,
                                 self.vmodules,
                                 patch.links,
                                 patch.cvalues)
        return mod_network.make_plan(self.omodules)


def main():
    class LFOscillator(Module):
        def __init__(self):
            super().__init__()
            self.rate = Input()
            self.out = Output()
            self.ports = (self.rate, self.out)
    class AREnvelope(Module):
        def __init__(self):
            super().__init__()
            self.attack = Input()
            self.release = Input()
            self.out = Output()
            self.ports = (self.attack, self.release, self.out)
    class QBLOscillator(Module):
        def __init__(self):
            super().__init__()
            self.pitch = Input()
            self.out = Output()
            self.ports = (self.pitch, self.out)
    class Filter(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input()
            self.out = Output()
            self.ports = (self.in_, self.out)
    class Amp(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input()
            self.gain = Input()
            self.out = Output()
            self.ports = (self.in_, self.gain, self.out)
    class Chorus(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input()
            self.out = Output()
            self.ports = (self.in_, self.out)
    class AudioOut(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input()
            self.ports = (self.in_)
    lfo1 = LFOscillator().name('LFO1')
    env = AREnvelope().name('Env1')
    osc1 = QBLOscillator().name('Osc1')
    filter = Filter()
    dca = Amp().name('DCA')
    chorus = Chorus()
    main = Amp().name('main')
    mpitch = MIDINotePitchControl()
    mexp = MIDIExpressionControl()
    mmod = MIDIModulationControl()
    s = (Synth(polyphony=2, timbrality=1)
         .name('StupidSynth')
         .voice_allocator('Random First')
         .timbre_module(lfo1)
         .voice_module(env)
         .voice_module(osc1)
         .voice_module(filter)
         .voice_module(dca)
         .timbre_module(chorus)
         .output_module(main)
         .vcontrol(mpitch)
         .tcontrol(mexp)
         .tcontrol(mmod)
         .finalize()
         )
    p = (s.make_patch()
         .connect(env.release, None, mexp)
         .connect(osc1.pitch, None, mpitch)
         .connect(osc1.pitch, env.out, lfo1.out, 0.3)
         .connect(filter.in_, osc1.out)
         .connect(dca.in_, filter.out)
         .connect(dca.gain, env.out)
         .connect(chorus.in_, dca.out)
         .connect(main.in_, chorus.out)
         )
    s.apply_patch(p, s.timbres[0])

    print(s)
    print(f'  polyphony     = {s.polyphony}')
    print(f'  timbrality    = {s.timbrality}')
    print(f'  voice alloc   = {s.voice_alloc}')
    print(f'  t modules     = {s.tmodules}')
    print(f'  t controls    = {s.tcontrols}')
    print(f'  v modules     = {s.vmodules}')
    print(f'  v controls    = {s.vcontrols}')
    print(f'  voices        = [')
    for v in s.voices:
        print(f'    voice           = {v}')
        print(f'      vmodules        = {v.vmodules}')
        print(f'      vcontrols       = {v.vcontrols}')
    print(f'                  ]')
    print(f'  timbres       = [')
    for t in s.timbres:
        print(f'    timbre          = {t}')
        print(f'      patch         = {t.patch}')
        print(f'      plan          = (')
        print(f'        t_prep          = [')
        for action in t.plan.t_prep:
            print(f'                            {action},')
        print(f'                          ]')
        print(f'        v_prep          = [')
        for action in t.plan.v_prep:
            print(f'                            {action},')
        print(f'                          ]')
        print(f'        pre_run         = {t.plan.pre_run}')
        print(f'        v_run           = [')
        for action in t.plan.v_run:
            print(f'                            {action},')
        print(f'                          ]')
        print(f'        post_run        = {t.plan.post_run}')
        print(f'                      )')
        print(f'        links         = [')
        for l in t.patch.links:
            print(f'                          {l},')
        print(f'                        ]')
        print(f'      tmodules      = {t.tmodules}')
        print(f'      tcontrols     = {t.tcontrols}')
    print(f'                  ]')
    print()

if __name__ == '__main__':
    main()

# Classes
#     action ...
#     control ...
#     link ...
#     mod network
#     mod vector
#     module ...
#     plan
#     port ...
#     synth ...
#     timbre
#     voice
