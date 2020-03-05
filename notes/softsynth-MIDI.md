# MIDI for softsynth

receive messages.  dispatch.

Have a registry for handlers of various message types.

    E.g., a note on/off handler schedules voices.
    A pitch bend handler puts pitch bend into the mod matrix.
    An RPN/NRPN handler handles the *RPN, data entry, and inc and dec CCs.
    MTC handler?

Have another registry for mappers of various message types.

One handler will accept note on/off messages and use those to schedule
voices.



## SYSEX

None for now.

## So...

    class MIDI_dispatcher:

        def __init__(self):
            ...

        def register_sth_handler(sth, handler):
            self.sth_handlers[sth] = handler

        def receive_MIDI_msg(msg):
            status = msg.status_byte
            sth = status >> 4
            handler = self.sth_handlers[sth]
            if handler:
                handler(msg)
