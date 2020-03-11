from collections import defaultdict, namedtuple
from pprint import pprint

class Port:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return getattr(self, 'name', self.__class__.__name__)

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

class Module:
    def __init__(self):
        self.ports = []
    def __repr__(self):
        return getattr(self, 'name', self.__class__.__name__)

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

#       #       #       #       #       #       #       #       #       #

class SignalGraph:

    def __init__(self):
        self.modules = []
        self.port_modules = {}
        self.module_inputs = defaultdict(list)
        self.module_outputs = defaultdict(list)
        # `self.owned_links` holds links that this graph owns.
        # `self.links` holds all links, whether owned or not.
        self.owned_links = []
        self.links = []

    def module(self, mod):
        self.modules.append(mod)
        for p in mod.ports:
            self.port_modules[p] = mod
            if isinstance(p, Input):
                self.module_inputs[mod].append(p)
            if isinstance(p, Output):
                self.module_outputs[mod].append(p)
        return self

    def connect(self, src, dest):
        # create an unnamed link, add it to the graph, and retain ownership.
        link = Link(src, dest)
        self.owned_links.append(link)
        return self.connection(link)

    def connection(self, link):
        # add a non-owned link to the graph, preferably a ControlLink.
        src = link.src
        dest = link.dest
        assert isinstance(src, Output)
        assert isinstance(dest, Input)
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

    def plan(self):

        n_modules = len(self.modules)
        module_predecessor_mask = [0] * n_modules
        port_destinations = defaultdict(list)
        port_sources = defaultdict(list)
        for link in self.links:
            src = link.src
            dest = link.dest
            pred = self.port_modules[src]
            succ = self.port_modules[dest]
            pred_index = self.modules.index(pred)
            succ_index = self.modules.index(succ)
            module_predecessor_mask[succ_index] |= 1 << pred_index
            if dest not in port_destinations[src]:
                port_destinations[src].append(dest)
            if src not in port_sources[dest]:
                port_sources[dest].append(src)

        def links_between(src, dest):
            for link in self.links:
                if (link.src == src and
                    link.dest == dest):
                    yield link

        def control_links_between(src, dest):
            for link in self.links:
                if (isinstance(link, ControlLink) and
                    link.src == src and
                    link.dest == dest):
                    yield link

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
                            src_mod = self.port_modules[src]
                            order.append(action(src_mod, mod, link))
                            action = Add

                # Render the ready module.
                order.append(Render(mod))

            done_mask |= ready_mask
            print(f'ready = {ready_mask:0{n_modules}b}')

        # XXX emit list of unconnected input ports.

        return order

    def fqpn(self, port):
        return f'{self.port_modules[port]}.{port}'

    def dump(self):

        print(f'modules = {self.modules}\n')

        for (map_name, fq) in {'port_modules': False,
                               'module_inputs': False,
                               'module_outputs': False,
                               }.items():
            print(f'{map_name} = {{')
            for (k, v) in getattr(self, map_name).items():
                if fq:
                    k = self.fqpn(k)
                    v = f'[{", ".join(self.fqpn(p) for p in v)}]'
                print(f'    {k}: {v},')
            print('}\n')

        def link_name(link):
            tname = type(link).__name__
            src = self.fqpn(link.src)
            dest = self.fqpn(link.dest)
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

# ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ---- -- ----

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

def dump_order(order):
    print('order = [')
    for action in order:
        print(f'    {action}')
    print(']\n')

ss = StupidSynth()
ss.graph.dump()
order = ss.graph.plan()
# print('# With attenuator gain disabled')
dump_order(order)

# ss.gain_link.enabled = True
# order = ss.graph.plan()
# print('# With attenuator gain enabled')
# dump_order(order)
