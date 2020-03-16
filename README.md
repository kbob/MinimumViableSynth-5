# Minimum Viable Synth, Version 5

*The Minimum Viable Synth is dead, long live the Minimum Viable
 Synth!*

I am starting over on the egregiously misnamed Minimum Viable Synth.
In large part it's because I found Will Pirkle's textbook,
[**Designing Software Synthesizer Plug-Ins in
C++](https://www.willpirkle.com/synthbook/).  Also, I've got a Teensy
4.0 that I want to use, and I've also pledged for the [Electrosmith
Daisy Kickstarter
campaign](https://www.kickstarter.com/projects/electro-smith/daisy-an-embedded-platform-for-music),
and I want the MVS to run on these.

# Directories

`synth/` is the platform-independent synth engine.

`synth/core/` contains the plumbing.

`synth/util/` is functions that aren't specific to synths but not part
of any one platform.

`synth/osc`, `synth/filt`, `synth/mod`, ...  collections of
synth modules.

`targets/` has specific configurations of the synth engine, *e.g.*,
NanoSynth, FM Synth, sampler.

`platforms/` adapts the synth engine to a platform, *e.g.*, MacOS AU,
Teensy Audio Library, offline.

`products/` adapts (target, platform) pairs, *e.g.*, nanosynth AU,
Teensy FM synth, offline sampler.  This is where complete synths are.

#       #       #       #       #       #       #       #       #       #       #
