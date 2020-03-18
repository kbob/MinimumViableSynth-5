from collections import defaultdict, namedtuple
from pprint import pprint

class Port:
    def __init__(self, name):
        self.name = name
        self.module = None
    def __repr__(self):
        return getattr(self, 'name', self.__class__.__name__)
    def fqpn(self):
        return f'{repr(self.module)}.{repr(self)}'

class Control:
    pass

class Input(Port):
    pass

class ControlInput(Input, Control):
    pass

class Output(Port):
    pass

class ControlOutput(Output, Control):
    pass

#       #       #       #       #       #       #       #       #       #

class Link(namedtuple('Link', 'src dest control', defaults=(None, ))):
    def __repr__(self):
        rest = f', {self[2]}' if self[2] is not None else ''
        return f'{type(self).__name__}({self[0]!r}, {self[1]!r}{rest})'
    def is_active(self):
        return True

class ControlLink(Link):
    def __init__(self, src, dest, control=None):
        self.intensity = 1
        self.enabled = False
        self.transform = 0
    def is_active(self):
        return self.enabled and self.intensity != 0

def make_link(src, dest, control=None):
    if isinstance(dest, Control):
        return ControlLink(src, dest, control)
    else:
        return Link(src, dest)

#       #       #       #       #       #       #       #       #       #

class Module:
    def __repr__(self):
        return getattr(self, 'name', self.__class__.__name__)
    @property
    def ports(self):
        return getattr(self, '_ports', [])
    @ports.setter
    def ports(self, ports):
        self._ports = ports
        for p in ports:
            p.module = self

#       #       #       #       #       #       #       #       #       #

class Action:
    pass

class Render(Action):
    def __init__(self, module):
        self.module = module
    def __repr__(self):
        return f'Render({self.module})'

class Copy(Action):
    def __init__(self, src_mod, dest_mod, link):
        self.src_mod = src_mod
        self.dest_mod = dest_mod
        self.link = link
    def __repr__(self):
        return f'Copy({self.src_mod}, {self.dest_mod}, {self.link})'

class Add(Action):
    def __init__(self, src_mod, dest_mod, link):
        self.src_mod = src_mod
        self.dest_mod = dest_mod
        self.link = link
    def __repr__(self):
        return f'Add({self.src_mod}, {self.dest_mod}, {self.link})'

class Clear(Action):
    def __init__(self, dest_mod, dest_port):
        self.dest_mod = dest_mod
        self.dest_port = dest_port
    def __repr__(self):
        dm = self.dest_mod
        dp = self.dest_port
        return f'Clear({dm}.{dp})'

class Alias(Action):
    def __init__(self, src_mod, dest_mod, link):
        self.src_mod = src_mod
        self.dest_mod = dest_mod
        self.link = link
    def __repr__(self):
        return f'Alias({self.src_mod}, {self.dest_mod}, {self.link})'

#       #       #       #       #       #       #       #       #       #

class SignalGraph:

    def __init__(self):
        self.modules = []
        self.module_inputs = defaultdict(list)
        self.module_outputs = defaultdict(list)
        # `self.owned_links` holds links that this graph owns.
        # `self.links` holds all links, whether owned or not.
        self.owned_links = []
        self.links = []

    def module(self, mod):
        self.modules.append(mod)
        for p in mod.ports:
            if isinstance(p, Input):
                self.module_inputs[mod].append(p)
            if isinstance(p, Output):
                self.module_outputs[mod].append(p)
        return self

    def connect(self, src, dest):
        # create an unnamed link, add it to the graph, and retain ownership.
        link = Link(src, dest)
        self.connection(link)   # do this first; it may assert.
        self.owned_links.append(link)
        return self

    # A ControlInput can have many ControlLinks.
    # A ControlInput can have no (signal)Links.
    # A (signal)Input can have one (signal)Link.
    # A (signal)Input can have no ControlLinks.

    # A ControlOutput can feed many ControlLinks.
    # A ControlOutput can feed many (signal)Links.
    # A (signal)Output can feed many (signal)Links.
    # A (signal)Output can feed many ControlLinks.

    def connection(self, link):
        # add a non-owned link to the graph.
        src = link.src
        dest = link.dest
        assert isinstance(src, Output)
        assert isinstance(dest, Input)
        if isinstance(dest, ControlInput):
            assert isinstance(link, ControlLink)
        else:
            assert not isinstance(link, ControlLink)
            assert not list(self.gen_links(dest=dest))
        assert link not in self.links
        self.links.append(link)
        return self

    def disconnect(self, src, dest, control=None):
        def find_link(src, dest, control):
            for link in self.links:
                if link == (src, dest, control):
                    return link

        link = find_link(src, dest, control)
        self.links.remove(link)
        if link in self.owned_links:
            self.owned_links.remove(link)
            del link
        return self

    def gen_links(self, type_=None, src=None, dest=None, control=None):
        for link in self.links:
            if type_ and type(link) != type_:
                continue
            if src and link.src != src:
                continue
            if dest and link.dest != dest:
                continue
            if control and link.control != control:
                continue
            yield link

    def plan(self):

        n_modules = len(self.modules)
        assert n_modules < 32
        module_predecessor_mask = [0] * n_modules
        port_sources = defaultdict(list)
        for link in self.links:
            src = link.src
            dest = link.dest
            pred_index = self.modules.index(src.module)
            succ_index = self.modules.index(dest.module)
            module_predecessor_mask[succ_index] |= 1 << pred_index
            if src not in port_sources[dest]:
                port_sources[dest].append(src)

        def control_links_between(src, dest):
            yield from self.gen_links(type_=ControlLink, src=src, dest=dest)

        order = []

        done_mask = 0
        all_done_mask = (1 << n_modules) - 1
        while done_mask != all_done_mask:

            # Collect all modules ready to process.
            ready_mask = sum(1 << i
                             for i in range(n_modules)
                             if (module_predecessor_mask[i] &~ done_mask) == 0
                             ) & ~done_mask
            if not ready_mask:
                raise RuntimeError('cycle in graph')

            for i in range(n_modules):
                if not ready_mask & 1 << i:
                    continue
                mod = self.modules[i]

                # Prep the ready module's inputs.
                for dest in self.module_inputs[mod]:
                    for src in port_sources[dest]:
                        action = Copy
                        for link in control_links_between(src, dest):
                            src_mod = src.module
                            order.append(action(src_mod, mod, link))
                            action = Add

                # Render the ready module.
                order.append(Render(mod))

            done_mask |= ready_mask
            # print(f'ready = {ready_mask:0{n_modules}b}')

        prep = []

        for mod in self.modules:
            # print(f'mod = {mod}')
            for dest in self.module_inputs[mod]:
                # print(f'    dest = {dest.fqpn()}')
                links = list(self.gen_links(None, None, dest, None))
                # print(f'    links = {links}')
                if links:
                    link = links[0]
                    if len(links) == 1 and type(link) == Link:
                        src_mod = link.src.module
                        prep.append(Alias(src_mod, mod, link))
                else:
                    prep.append(Clear(mod, dest))



        Plan = namedtuple('Plan', 'prep order')
        return Plan(prep, order)

    def dump(self):

        print(f'modules = {self.modules}\n')

        for (map_name, fq) in {'module_inputs': False,
                               'module_outputs': False,
                               }.items():
            print(f'{map_name} = {{')
            for (k, v) in getattr(self, map_name).items():
                if fq:
                    k = k.fqpn()
                    v = f'[{", ".join(p.fqpn() for p in v)}]'
                print(f'    {k}: {v},')
            print('}\n')

        def link_name(link):
            tname = type(link).__name__
            src = link.src.fqpn()
            dest = link.dest.fqpn()
            rest = f', {link.control}' if link.control is not None else ''
            return f'{tname}({src}, {dest}{rest})'

        print('links = [')
        for link in self.links:
            print(f'    {link_name(link)}')
        print(']\n')

        print('owned_links = [')
        for link in self.owned_links:
            print(f'    {link_name(link)}')
        print(']\n')

#       #       #       #       #       #       #       #       #       #

# Voice
#     has storage for each module's state.
#     has storage for each Output's data.
#     has storage for each ControlInput's data.

class Voice:
    def __init__(self):
        self.module_states = [m.create_state() for m in graph.modules]
        self.input_data = [[i.data() for j in range(MAX_FRAMES)]
                           for i in graph.inputs
                           if isinstance(i, ControlPort)]
        self.output_data = [[o.data() for i in range(MAX_FRAMES)]
                            for o in graph.outputs]

    def assign_note(self, note):
        # do something with the note.
        for (i, m) in enumerate(graph.modules):
            m.init_state(self.module_states[i])
        for action in plan.prep:
            action()

    def render(self):
        for (i, d) in zip(graph.inputs, self.input_data):
            i.set_ptr(d)
        for (o, d) in zip(graph.outputs, self.output_data):
            o.set_ptr(d)
        for action in plan.order:
            action()
        # where is the output?


# ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- --
#       #       #       #       #       #       #       #       #       #
# ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- --

class QBLOscillator(Module):
    def __init__(self):
        self.pitch_bend = Input('pitch_bend')
        self.modulation = Input('modulation')
        self.out = Output('out')
        self.ports = [self.pitch_bend, self.modulation, self.out]

class Attenuator(Module):

    def __init__(self):
        self.in_ = Input('in')
        self.gain = ControlInput('gain')
        self.out = Output('out')
        self.ports = [self.in_, self.gain, self.out]

class Mixer(Module):

    def __init__(self, n):
        self.in_ = [Input(f'in_{i}') for i in range(n)]
        self.gain = [Input(f'gain_{i}') for i in range(n)]
        self.out = Output('out')
        self.ports = [*self.in_, *self.gain, self.out]

class StupidSynth:

    def __init__(self):
        self.osc = o = QBLOscillator()
        self.atten = a = Attenuator()
        self.mix = m = Mixer(1)

        o.name = 'Osc1'

        self.gain_link = gain_link = make_link(o.out, a.gain, 'CC 7')
        self.gain_link2 = gain_link2 = make_link(o.out, a.gain, 'CC 3')

        self.graph = (SignalGraph()
                        .module(o)
                        .module(a)
                        .module(m)
                        .connect(o.out, a.in_)
                        .connect(a.out, m.in_[0])
                        .connection(gain_link)
                        .connection(gain_link2)
                        )

def dump_plan(plan):
    print('plan = (')
    sep = int
    for member in ('prep', 'order'):
        sep()
        sep = print
        print(f'    {member} = [')
        for action in getattr(plan, member):
            print(f'        {action}')
    print(')\n')

ss = StupidSynth()
ss.graph.dump()
plan = ss.graph.plan()
# print('# With attenuator gain disabled')
dump_plan(plan)

# ss.gain_link.enabled = True
# plan = ss.graph.plan()
# print('# With attenuator gain enabled')
# dump_plan(plan)
