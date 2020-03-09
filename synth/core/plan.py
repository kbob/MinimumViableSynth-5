from collections import defaultdict
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

class Link:
    def __init__(self, src, dest):
        self.src = src
        self.dest = dest
    def is_active(self):
        return True

class ControlLink(Link):
    def __init__(self, src, dest, intensity=1, enabled=False):
        super().__init__(src, dest)
        self.intensity = intensity
        self.enabled = enabled
    def is_active(self):
        return self.enabled and self.intensity != 0

def make_link(src, dest):
    if isinstance(dest, Control):
        return ControlLink(src, dest)
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
    def __init__(self, src_mod, src_port, dest_mod, dest_port):
        self.src_mod = src_mod
        self.src_port = src_port
        self.dest_mod = dest_mod
        self.dest_port = dest_port
    def __repr__(self):
        sm = self.src_mod
        sp = self.src_port
        dm = self.dest_mod
        dp = self.dest_port
        return f'Copy({sm}.{sp}, {dm}.{dp})'

class Zero(Action):
    def __init__(self, dest_mod, dest_port):
        self.dest_mod = dest_mod
        self.dest_port = dest_port
    def __repr__(self):
        dm = self.dest_mod
        dp = self.dest_port
        return f'Zero({dm}.{dp})'

#       #       #       #       #       #       #       #       #       #
# So suppose `links` is a map from ``(src, dest)`` pairs to
# `Link` objects.  A basic `Link` object has no data, but a
# `ControlLink` has `intensity` and `enable` fields.
#
# To enable, disable, or change the intensity of a `ControlLink`,
# you have to look it up in the map, then assign the member
# (or call a method to assign it).
#
# I am thinking the map should be implemented as a vector of
# `(src, dest, link)` triples.
# Pointers into the map are cached, along with their gen number.
# When a pointer is newly created or out of date, search
# the vector linearly for the right element.

class SignalGraph:
    def __init__(self):
        self.modules = []
        self.port_modules = {}
        self.module_inputs = defaultdict(list)
        self.module_outputs = defaultdict(list)
        # `self.links` holds links that this graph owns.
        # `self.link_map` maps (src, dest) pairs to links,
        # including both owned and non-owned links.
        self.links = []
        self.link_map = {}
        self.port_destinations = defaultdict(list)
        self.port_sources = defaultdict(list)

    def module(self, mod):
        self.modules += [mod]
        for p in mod.ports:
            self.port_modules[p] = mod
            if isinstance(p, Input):
                self.module_inputs[mod] += [p]
            if isinstance(p, Output):
                self.module_outputs[mod] += [p]
        return self

    def connect(self, src, dest):
        # create an unnamed link, add it to the graph, and retain ownership.
        link = make_link(src, dest)
        self.links += [link]
        return self.connection(link)

    def connection(self, link):
        # add a non-owned link to the graph.
        src = link.src
        dest = link.dest
        assert isinstance(src, Output)
        assert isinstance(dest, Input)
        if isinstance(dest, Control):
            assert (src, dest) not in self.link_map
        else:
            assert dest not in {l[1] for l in self.link_map}
        self.link_map[(src, dest)] = link
        self.port_destinations[src] += [dest]
        self.port_sources[dest] += [src]
        return self

    def disconnect(self, src, dest):
        link = self.link_map[(src, dest)]
        del self.link_map[(src, dest)]
        self.port_destinations[src].remove(dest)
        self.port_sources[dest].remove(src)
        if link in self.links:
            self.links.remove(link)
        return self

    def plan(self):
        def is_active(src, dest):
            return self.link_map[(src, dest)].is_active()

        def is_ready(m):
            return all(self.port_modules[src] in done
                       for dest in self.module_inputs[m]
                       for src in self.port_sources[dest]
                       if is_active(src, dest))
        order = []
        done = set()
        while len(done) < len(self.modules):
            ready = [m for m in self.modules if m not in done and is_ready(m)]
            if not ready:
                raise RuntimeError('cycle in graph')

            # Zero all unconnected inputs to ready modules.
            for mod in ready:
                for dest in self.module_inputs[mod]:
                    if not any(self.link_map[(src, dest)].is_active()
                               for src in self.port_sources[dest]):
                        order += [Zero(mod, dest)]

            # Render all ready modules.
            order += [Render(m) for m in ready]

            # Copy all ready modules' outputs to ControlLinks.
            for src_mod in ready:
                for out in self.module_outputs[src_mod]:
                    for dest in self.port_destinations[out]:
                        link = self.link_map[(out, dest)]
                        if isinstance(link, ControlLink) and link.is_active():
                            dest_mod = self.port_modules[dest]
                            order += [Copy(src_mod, out, dest_mod, dest)]
            done |= set(ready)
            # print(f'ready = {ready}')
            # print(f'done = {done}')
        return order

    def fqpn(self, port):
        return f'{self.port_modules[port]}.{port}'

    def dump(self):

        print(f'modules = {self.modules}\n')

        for (map_name, fq) in {'port_modules': False,
                               'module_inputs': False,
                               'module_outputs': False,
                               'port_destinations': True,
                               'port_sources': True}.items():
            print(f'{map_name} = {{')
            for (k, v) in getattr(self, map_name).items():
                if fq:
                    k = self.fqpn(k)
                    v = f'[{", ".join(self.fqpn(p) for p in v)}]'
                print(f'    {k}: {v},')
            print('}\n')

        def link_name(link):
            cname = '' if type(link) == Link else link.__class__.__name__
            return f'{cname}({self.fqpn(link.src)}, {self.fqpn(link.dest)})'

        print('links = [')
        for link in self.links:
            print(f'    ({self.fqpn(link.src)}, {self.fqpn(link.dest)}),')
        print(']\n')

        print('link_map = {')
        for ((src, dest), link) in self.link_map.items():
            key_name = f'({self.fqpn(src)}, {self.fqpn(dest)})'
            value_name = link_name(link)
            value_name = f'{link.__class__.__name__}'
            print(f'    {key_name}: {value_name},')
        print('}\n')

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

        self.gain_link = gain_link = make_link(o.out, a.gain)

        self.graph = (SignalGraph()
                        .module(o)
                        .module(a)
                        .module(m)
                        .connect(o.out, a.in_)
                        .connect(a.out, m.in_[0])
                        .connection(gain_link)
                        )

def dump_order(order):
    print('order = [')
    for action in order:
        print(f'    {action}')
    print(']\n')

ss = StupidSynth()
ss.graph.dump()
order = ss.graph.plan()
print('# With attenuator gain disabled')
dump_order(order)

ss.gain_link.enabled = True
order = ss.graph.plan()
print('# With attenuator gain enabled')
dump_order(order)
