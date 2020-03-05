What?

Targets

    Batch
    AU
    Teensy/Electosmith
    Cocoa App?

Synth Core Runtime

    init(...)
    render(frame_count, buffer_out)
    control(changelist)

    module query functions
    connection query functions
    control query functions

Synth Utility

    MIDI_to_control()
    param_to_control()

Platform Dependencies

    audio I/O
    MIDI I/O
    math

Batch I/O

    play MIDI file
    output .WAV and STFT
    single-threaded for debugging

Embedded I/O

    Stereo line out
    serial MIDI in
    USB MIDI in (device)
    USB MIDI in (host/OTG)
    Bluetooth MIDI in
    WiFi MIDI in

    call synthcore from loop().

Command Line

    real-time IAC MIDI in
    play CoreAudio

AU I/O

    param_to_control()
    handle MIDI messages

Questions

    how can the synth core report errors?
      (e.g., modulation cycles)

Limitations

    monotimbral
    fixed config
    no FX
    defer Cocoa app.
    MIDI channel 1 only.
    no multicore.
    no oversampling, no rate conversion
