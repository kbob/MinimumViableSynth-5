#ifndef MIDI_FILE_included
#define MIDI_FILE_included

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum MIDI_status {
    MS_OK = 0,                  // OK - no error
    MS_NO_MEM,                  // Memory allocation failed.
    MS_NOT_MIDI_FILE,           // File does not have "MThd" signature.
    MS_FILE_TRUNCATED,          // File is truncated.
    MS_UNKNOWN_FORMAT,          // Unknown format value.
    MS_INVALID_TRACK_COUNT,     // Format 0 file must have one track.
    MS_NOT_ITERABLE,            // Format 2 file can't be iterated.
    MS_UNPARSEABLE,             // Track data can't be parsed.
    MIDI_status_count
} MIDI_status;

extern const char *MIDI_status_name(MIDI_status);
extern const char *MIDI_status_description(MIDI_status);

// typedef struct MIDI_cursor MIDI_cursor;

typedef enum MIDI_file_format {
    FORMAT_0 = 0,               // Format 0 - single track
    FORMAT_1 = 1,               // Format 1 - several simultaneous tracks
    FORMAT_2 = 2,               // Format 2 - several independent tracks
    MIDI_file_format_count
} MIDI_file_format;

typedef struct MIDI_track {
    const uint8_t      *start;
    size_t              size;
} MIDI_track;

typedef struct MIDI_file {
    const uint8_t      *start;
    size_t              size;
    MIDI_file_format    format;
    uint16_t            division;
    size_t              track_count;
    MIDI_track         *tracks;
} MIDI_file;

// XXX these should return error codes.
extern MIDI_status init_MIDI_file(const char *data,
                                  size_t size,
                                  MIDI_file *mf_out);
extern MIDI_status verify_MIDI_FILE(const MIDI_file *);
extern void destroy_MIDI_file(MIDI_file *mf);

typedef enum MIDI_event_type {
                                //
    ET_MIDI,                    // MIDI channel event
    ET_SYSEX,                   // MIDI SYSEX, first part
    ET_META,                    // META event
    ET_RAW,                     // raw data, e.g., SYSEX continuation or MTC
    MIDI_event_type_count
} MIDI_event_type;

extern const char *MIDI_event_name(MIDI_event_type);

typedef struct MIDI_event {
    uint32_t            timestamp;
    MIDI_event_type     type;
    uint16_t            track;
    uint8_t             channel;
    uint8_t             status_byte;
    const uint8_t      *data_bytes;
    size_t              data_size;
    const uint8_t      *raw_bytes;
} MIDI_event;

typedef struct MIDI_timing {
    uint32_t            usec_per_quarter;
    uint16_t            division;
    uint8_t             sig_numerator;
    uint8_t             sig_denominator;
    uint8_t             sig_cc;
    uint8_t             sig_bb;
} MIDI_timing;

typedef struct MIDI_track_state MIDI_track_state;

typedef struct MIDI_iterator {
    const MIDI_file    *file;
    MIDI_timing         timing;
    uint32_t            time_ticks;
    uint64_t            time_usecs;
    size_t              track_count;
    MIDI_track_state   *tracks;
    // MIDI_cursor        *track_cursors;
    // uint32_t           *track_times;
} MIDI_iterator;

extern MIDI_status init_MIDI_file_iterator(const MIDI_file *,
                                           MIDI_iterator *it_out);
extern MIDI_status init_MIDI_track_iterator(const MIDI_file *,
                                            size_t track_no,
                                            MIDI_iterator *it_out);
extern void destroy_MIDI_iterator(MIDI_iterator *);

// MIDI_iter_next returns the relative taim (in microseconds) until
// the event.
// If the iteration is finished, MIDI_iter_next returns MIDI_ITER_END.
#define MIDI_ITER_END UINT32_MAX
extern uint32_t MIDI_iter_next(MIDI_iterator *, MIDI_event *);

#endif /* !MIDI_FILE_included */
