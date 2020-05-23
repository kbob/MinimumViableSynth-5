#ifndef MIDI_DEFS_included
#define MIDI_DEFS_included

#include <cstdint>

namespace midi {

    enum class StatusByte : std::uint8_t {

        // Channel Voice Messages
        NOTE_OFF                        = 0x80,
        NOTE_ON                         = 0x90,
        POLY_KEY_PRESSURE               = 0xA0,
        CONTROL_CHANGE                  = 0xB0,
        PROGRAM_CHANGE                  = 0xC0,
        CHANNEL_PRESSURE                = 0xD0,
        PITCH_BEND                      = 0xE0,

        // Channel Mode Messages
        SELECT_CHANNEL_MODE             = 0xB0,

        // System Messages
        //   System Exclusive
        SYSTEM_EXCLUSIVE                = 0xF0,

        //   System Common
        MTC_QTR_FRAME                   = 0xF1,
        SONG_POSITION                   = 0xF2,
        SONG_SELECT                     = 0xF3,
        TUNE_REQUEST                    = 0xF6,
        EOX                             = 0xF7,

        //   System Real Time
        TIMING_CLOCK                    = 0xF8,
        START                           = 0xFA,
        CONTINUE                        = 0xFB,
        STOP                            = 0xFC,
        ACTIVE_SENSING                  = 0xFE,
        SYSTEM_RESET                    = 0xFF,

    };

    enum class ControllerNumber : std::uint8_t {
        BANK_SELECT_MSB                 = 0,
        BANK_SELECT_LSB                 = 32,
        MOD_WHEEL_MSB                   = 1,
        MOD_WHEEL_LSB                   = 33,
        BREATH_CONTROLLER_MSB           = 2,
        BREATH_CONTROLLER_LSB           = 34,
        FOOT_CONTROLLER_MSB             = 4,
        FOOT_CONTROLLER_LSB             = 36,
        PORTAMENTO_TIME_MSB             = 5,
        PORTAMENTO_TIME_LSB             = 37,
        DATA_ENTRY_MSB                  = 6,
        DATA_ENTRY_LSB                  = 38,
        CHANNEL_VOLUME_MSB              = 7,
        CHANNEL_VOLUME_LSB              = 39,
        BALANCE_MSB                     = 8,
        BALANCE_LSB                     = 40,
        PAN_MSB                         = 10,
        PAN_LSB                         = 42,
        EXPRESSION_CONTROLLER_MSB       = 11,
        EXPRESSION_CONTROLLER_LSB       = 43,
        EFFECT_CONTROL_1_MSB            = 12,
        EFFECT_CONTROL_1_LSB            = 44,
        EFFECT_CONTROL_2_MSB            = 13,
        EFFECT_CONTROL_2_LSB            = 45,
        GENERAL_PURPOSE_1_MSB           = 16,
        GENERAL_PURPOSE_1_LSB           = 48,
        GENERAL_PURPOSE_2_MSB           = 17,
        GENERAL_PURPOSE_2_LSB           = 49,
        GENERAL_PURPOSE_3_MSB           = 18,
        GENERAL_PURPOSE_3_LSB           = 50,
        GENERAL_PURPOSE_4_MSB           = 19,
        GENERAL_PURPOSE_4_LSB           = 51,
        DAMPER_PEDAL                    = 64,
        PORTAMENTO_ON_OFF               = 65,
        SOSTENUTO                       = 66,
        SOFT_PEDAL                      = 67,
        LEGATO_FOOTSWITCH               = 68,
        HOLD_2                          = 69,
        SOUND_CONTROLLER_1              = 70,
        SOUND_VARIATION                 = 70,
        SOUND_CONTROLLER_2              = 71,
        TIMBRE_INTENSITY                = 71,
        SOUND_CONTROLLER_3              = 72,
        RELEASE_TIME                    = 72,
        SOUND_CONTROLLER_4              = 73,
        ATTACK_TIME                     = 73,
        SOUND_CONTROLLER_5              = 74,
        BRIGHTNESS                      = 74,
        SOUND_CONTROLLER_6              = 75,
        SOUND_CONTROLLER_7              = 76,
        SOUND_CONTROLLER_8              = 77,
        SOUND_CONTROLLER_9              = 78,
        SOUND_CONTROLLER_10             = 79,
        GENERAL_PURPOSE_5               = 80,
        GENERAL_PURPOSE_6               = 81,
        GENERAL_PURPOSE_7               = 82,
        GENERAL_PURPOSE_8               = 83,
        PORTAMENTO_CONTROL              = 84,
        EFFECTS_1_DEPTH                 = 91,
        EXTERNAL_EFFECTS_DEPTH          = 91,
        EFFECTS_2_DEPTH                 = 92,
        TREMOLO_DEPTH                   = 92,
        EFFECTS_3_DEPTH                 = 93,
        CHORUS_DEPTH                    = 93,
        EFFECTS_4_DEPTH                 = 94,
        CELESTE_DEPTH                   = 94,
        EFFECTS_5_DEPTH                 = 95,
        PHASER_DEPTH                    = 95,
        DATA_INCREMENT                  = 96,
        DATA_DECREMENT                  = 97,
        NRPN_LSB                        = 98,
        NRPN_MSB                        = 99,
        RPN_LSB                         = 100,
        RPN_MSB                         = 101,
    };

    enum class ChannelModeNumber : std::uint8_t {
        ALL_SOUND_OFF                   = 120,
        RESET_ALL_CONTROLLERS           = 121,
        LOCAL_CONTROL                   = 122,
        ALL_NOTES_OFF                   = 123,
        OMNI_MODE_OFF                   = 124,
        OMNI_MODE_ON                    = 125,
        MONO_MODE_ON                    = 126,
        POLY_MODE_ON                    = 127,

    };

    enum class RPN : std::uint16_t {
        PITCH_BEND_SENSITIVITY          = 0x0000,
        FINE_TUNING                     = 0x0001,
        COARSE_TUNING                   = 0x0002,
        TUNING_PROGRAM_SELECT           = 0x0003,
        TUNING_BANK_SELECT              = 0x0004,
    };

    enum class SysexSubId : std::uint8_t {
        NON_COMMERCIAL                  = 0x7D,
        NON_REAL_TIME                   = 0x7E,
        REAL_TIME                       = 0x7F,
    };

    enum class UniversalSysexNonRealTime : std::uint8_t {
        SAMPLE_DUMP_HEADER              = 0x01,
        SAMPLE_DATA_PACKET              = 0x02,
        SAMPLE_DUMP_REQUEST             = 0x03,
        MIDI_TIME_CODE                  = 0x04,
        SAMPLE_DUMP_EXTENSIONS          = 0x05,
        GENERAL_INFORMATION             = 0x06,
        FILE_DUMP                       = 0x07,
        MIDI_TUNING_STANDARD            = 0x08,
        GENERAL_MIDI                    = 0x09,
        END_OF_FILE                     = 0x7B,
        WAIT                            = 0x7C,
        CANCEL                          = 0x7D,
        NAK                             = 0x7E,
        ACK                             = 0x7F,
    };

    enum class UniversalSysexRealTime : std::uint8_t {
        MIDI_TIME_CODE                  = 0x01,
        MIDI_SHOW_CONTROL               = 0x02,
        NOTATION_INFORMATION            = 0x03,
        DEVICE_CONTROL                  = 0x04,
        REAL_TIME_MTC_CODING            = 0x05,
        MIDI_MACHINE_CONTROL_COMMANFS   = 0x06,
        MIDI_MACHINE_CONTROL_RESPONSES  = 0x07,
        MIDI_TUNING_STANDARD            = 0x08,
    };

    enum class ChannelMode : std::uint8_t {
        OMNI_POLY                       = 1,
        OMNI_MONO                       = 2,
        POLY                            = 3,
        MONO                            = 4,
    };

    // To be added as needed:
    //   MTC quarter frame message types
    //   MTC sub-IDs (non real time)
    //   sample dump extension sub-IDs
    //   general information sub-IDs
    //   file dump sub-IDs
    //   MIDI tuning standard sub-IDs
    //   general MIDI sub-IDs
    //   MTC sub-IDs (real time)
    //   MIDI Show Control sub-IDs
    //   notation information sub-IDs
    //   device control sub-IDs
    //   real time MTC cueing sub-IDs
    //   MIDI Machine Control commands
    //   MIDI Machine Control responses
    //   MIDI tuning standard sub-IDs
    //   Everything MIDI 2.0

}

#endif /* !MIDI_DEFS_included */
