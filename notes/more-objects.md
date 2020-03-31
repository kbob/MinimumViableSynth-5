Polyphony
Mulitimbrality
Mod "Matrix"


synth has voice prototype
voice prototype has module prototypes
                    connection prototypes
                    flow graph
voice prototype is "cloned" or "instantiated" or something
to create the actual voice.


synth has voices
voice has module instances
          connection instances
      references flow graph's plan


subclass Module w/ ModuleInstance.
ModuleInstance calls super's render; defines


CRTP:
template <class ModuleType>
class ModulePrototype {};
class Oscillator : public ModulePrototype<Oscillator> {};

- prototype is used to generate graph and plan.
- Osc has the I/O buffers and render function
- does this buy us anything?


graph refers to module by its position in the voice.  v[i]
                port by its position in the module.  v[i].p[j]
                simple_connection by src and dest ports.
                                                    (v[i0].p[j0], v[i1].p[j1])
                control_connection as above plus control/param address.
                                                    (..., "CC 7")
specify the first voice, then clone it n-1 times.
Separate graph for non-voice modules.

So graph and voice are the same thing?


Better: there are three index types: mod_index, inport_index, outport_index.
They can each be uint8_t.  Or maybe just two.


Each plan is associated with a Timbre.  Voices move from Timbre to Timbre.
So when a voice is initialized, the plan's prep actions are applied to it.
Voice has two arrays of pointers that map module or port index to Module
or Port objects.

---------------------------------

change actions to hold indices, not pointers.
synth creates a voice, populates it, then clones it n-1 times.
ModNetwork - has a list of modules and voices.  Connections are
added to the mod network, and plans are generated from there.
