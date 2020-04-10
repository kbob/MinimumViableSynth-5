#!/usr/bin/env python3

from copy import copy
from enum import Enum, auto

# An action is simply a std::function<void()>.
# Or a callable in Python.

class named:
    def __init__(self):
        self._name = type(self).__name__
        super().__init__()
    def name(self, name):
        self._name = name
        return self
    def __repr__(self):
        return self._name

class Port:
    def __repr__(self):
        for (k, v) in self.owner.__dict__.items():
            if v == self:
                return f'{self.owner}.{k.strip("_")}'

        return type(self).__name__
class InputPort(Port): ...
class Input(InputPort):
    def __init__(self, typ):
        super().__init__()
        self.type = typ
class OutputPort(Port): ...
class Output(OutputPort):
    def __init__(self, typ=float):
        super().__init__()
        self.type = typ

class ported:
    def ports(self, *ports):
        self._ports = ports
        for p in ports:
            p.owner = self

class Control(named, ported):
    def __init__(self, typ=float):
        super().__init__()
        self.out = Output(typ)
class MIDINotePitchControl(Control): ...
class MIDIModulationControl(Control): ...
class MIDIExpressionControl(Control): ...

class Module(named, ported):
    def ports(self, *ports):
        self._ports = ports
        for p in ports:
            p.owner = self

# class Link: ...
# class SimpleLink(Link):
#     def __init__(self, src, dest):
#         self.src = src
#         self.dest = dest
#     def __repr__(self):
#         return f'{self.src} -> {self.dest}'
# class ControlLink(Link):
#     def __init__(self, src, dest, ctl=None, scale=1):
#         self.src = src
#         self.dest = dest
#         self.ctl = ctl
#         self.scale = scale
#     def __repr__(self):
#         factors = [repr(x) for x in (self.src, self.ctl) if x is not None]
#         if self.scale != 1:
#             factors.append(str(self.scale))
#         return f'{" * ".join(factors)} -> {self.dest}'

class Link:
    ...
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

def make_link(dest, src=None, ctl=None, scale=1):
    d_type = dest.type
    s_type = src.type if src else type(None)
    c_type = ctl.type if ctl else type(None)

    return LinkType(d_type, s_type, c_type, dest, src, ctl, scale)

# A `ControlLink` has:
#     a destination, which is an `Input<D>`.
#     an optional source, which is an `Output<S>`.
#     an optional control, which is either a `Control` or an `Output<C>``.
#     an optional scale, which is a scalar `D` constant.
#
# A `Control` has:
#     an `Output<T>`` called `out`.

class Plan:
    def __init__(self):
        self.prep = []
        self.run = []
    def add_prep(self, action):
        self.prep.append(action)
    def add_run(self, action):
        self.run.append(action)
    def prep_voice(self, voice, timbre):
        ...
    def make_exec(self, voice, timbre):
        ...

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
        self.exec = None

    def start_note(self, note):
        self.note = note
        self.state = VoiceState.Active

    def release_note(self):
        ...

    def kill_note(self):
        ...

class Timbre(named):
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
    def set_patch(self, patch):
        self.patch = patch
    def init_voice(self, voice): ...
    def update_voice(self, voice): ...
    def create_patch(self): ...
    def apply_patch(self, patch): ...

class Synth(named):
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
    def __init__(self, polyphony=1,timbrality=1):
        super().__init__()
        self.finalized   = False
        self.polyphony   = polyphony
        self.timbrality  = timbrality
        self.tmodules    = []
        self.vmodules    = []
        self.tcontrols   = []
        self.vcontrols   = []
        # self.links       = []
        self.output_port = None
        self.voice_alloc = None

    # construction
    def voice_allocator(self, valloc):
        self.voice_alloc = valloc
        return self
    def timbre_module(self, module):
        assert not self.finalized
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
    # def connection(self, src, dest):
    #     assert not self.finalized
    #     self.links.append(make_link(dest, src))
    #     return self
    # def control_connection(self, src, dest, ctl=None):
    #     assert not self.finalized
    #     self.links.append(make_link(dest, src, ctl))
    #     return self
    def finalize(self, output):
        assert not self.finalized
        self.output_port = output
        self.finalized = True
        self.voices = [Voice(vmodules=copy(self.vmodules),
                             vcontrols = copy(self.vcontrols))
                       for i in range(self.polyphony)]
        self.timbres = [Timbre(tmodules=copy(self.tmodules),
                               tcontrols=copy(self.tcontrols))
                               .name(chr(ord('A') + i))
                        for i in range(self.timbrality)]
        return self

    def make_patch(self):
        assert self.finalized
        return Patch(self)

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
    def connect(self, dest, src=None, ctl=None, scale=1):
        self.links.append(make_link(dest, src, ctl, scale))
        return self

def main():
    class LFOscillator(Module):
        def __init__(self):
            super().__init__()
            self.out = Output(float)
            self.ports(self.out)
    class AREnvelope(Module):
        def __init__(self):
            super().__init__()
            self.attack = Input(float)
            self.release = Input(float)
            self.out = Output(float)
            self.ports(self.attack, self.release, self.out)
    class QBLOscillator(Module):
        def __init__(self):
            super().__init__()
            self.pitch = Input(float)
            self.out = Output(float)
            self.ports(self.pitch, self.out)
    class Filter(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input(float)
            self.out = Output(float)
            self.ports(self.in_, self.out)
    class Amp(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input(float)
            self.gain = Input(float)
            self.out = Output(float)
            self.ports(self.in_, self.gain, self.out)
    class AudioOut(Module):
        def __init__(self):
            super().__init__()
            self.in_ = Input(float)
            self.ports(self.in_)
    lfo1 = LFOscillator().name('LFO1')
    env = AREnvelope().name('Env1')
    osc1 = QBLOscillator().name('Osc1')
    filter = Filter()
    dca = Amp().name('DCA')
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
         .timbre_module(main)
         .vcontrol(mpitch)
         .tcontrol(mexp)
         .tcontrol(mmod)
         # .connection(osc1.out, filter.in_)
         # .connection(filter.out, amp.in_)
         # .connection(env.out, amp.gain)
         .finalize(main.out)
        )
    p = (s.make_patch()
         .connect(osc1.pitch, env.out, lfo1.out, 0.3)
         .connect(filter.in_, osc1.out)
         .connect(dca.in_, filter.out)
         .connect(dca.gain, env.out)
         .connect(main.in_, dca.out))
    s.timbres[0].set_patch(p)
    print(s)
    print(f'  polyphony     = {s.polyphony}')
    print(f'  timbrality    = {s.timbrality}')
    print(f'  voice alloc   = {s.voice_alloc}')
    print(f'  t modules     = {s.tmodules}')
    print(f'  t controls    = {s.tcontrols}')
    print(f'  v modules     = {s.vmodules}')
    print(f'  v controls    = {s.vcontrols}')
    # print(f'  links         = [')
    # for l in s.links:
    #     print(f'                    {l},')
    # print(f'                  ]')
    print(f'  voices        = [')
    for v in s.voices:
        print(f'    voice           = {v}')
        print(f'      vmodules      = {v.vmodules}')
        print(f'      vcontrols     = {v.vcontrols}')
    print(f'                  ]')
    print(f'  timbres       = [')
    for t in s.timbres:
        print(f'    timbre          = {t}')
        print(f'      patch         = {t.patch}')
        print(f'        links       = [')
        for l in t.patch.links:
            print(f'                        {l},')
        print(f'                      ]')
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
