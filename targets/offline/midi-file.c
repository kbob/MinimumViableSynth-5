#include "midi-file.h"

#include <assert.h>             // XXX
// #include <stdio.h>              // XXX
#include <stdlib.h>
#include <string.h>

typedef char fourcc[4];

bool fourcc_eq(fourcc a, fourcc b)
{
    for (size_t i = 0; i < 4; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// MIDI_cursor

typedef struct cursor {
    const uint8_t *start;
    size_t pos;
    size_t size;
} cursor;

static void init_cursor(const uint8_t *start, const uint8_t size, cursor *c)
{
    c->start = start;
    c->pos = 0;
    c->size = size;
}

static const uint8_t *cursor_ptr(const cursor *c)
{
    return &c->start[c->pos];
}

static bool cursor_read_bytes(cursor *c, size_t n, uint8_t *out)
{
    if (c->pos + n > c->size)
        return false;
    if (out)
        for (size_t i = 0; i < n; i++)
            out[i] = c->start[c->pos + i];
    c->pos += n;
    return true;
}

// get 1-byte integer, do not advance
static bool cursor_peek_i1(const cursor *c, uint8_t *out)
{
    if (c->pos + 1 > c->size)
        return false;
    if (out)
        *out = c->start[c->pos];
    return true;
}

// Read a one byte int.
static bool cursor_read_i1(cursor *c, uint8_t *out)
{
    bool ok = cursor_peek_i1(c, out);
    if (ok)
        c->pos++;
    return ok;
}

// Read a two byte int, big endian.
static bool cursor_read_i2(cursor *c, uint16_t *out)
{
    if (c->pos + 2 > c->size)
        return false;
    const uint8_t *p = c->start + c->pos;
    uint16_t i = p[0] << 8 | p[1];
    c->pos += 2;
    if (out)
        *out = i;
    return true;
}

// Read a three byte int, big endian
static bool cursor_read_i3(cursor *c, uint32_t *out)
{
    if (c->pos + 3 > c->size)
        return false;
    const uint8_t *p = c->start + c->pos;
    uint16_t i = p[0] << 16 | p[1] << 8 | p[2];
    c->pos += 3;
    if (out)
        *out = i;
    return true;
}

// Read a four byte int, big endian.
static bool cursor_read_i4(cursor *c, uint32_t *out)
{
    if (c->pos + 4 > c->size)
        return false;
    const uint8_t *p = c->start + c->pos;
    uint32_t i = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3] << 0;
    c->pos += 4;
    if (out)
        *out = i;
    return true;
}

// read variable-length quantity (VLQ)
// A VLQ is 1-4 bytes, each byte except the last has the MSB set.
static bool cursor_read_vlq(cursor *c, uint32_t *out)
{
    uint32_t vlq = 0;
    for (int i = 0; i < 4; i++) {
        if (c->pos + i > c->size)
            return false;
        uint8_t b = c->start[c->pos + i];
        vlq = vlq << 7 | (b & 0x7F);
        if (!(b & 0x80)) {
            c->pos += i + 1;
            if (out)
                *out = vlq;
            return true;
        }
    }
    return false;
}

// Like read_vlq, except return 0xFFFFFFFF on failure.
static uint32_t cursor_read_time(cursor *c)
{
    uint32_t t = MIDI_ITER_END;
    (void)cursor_read_vlq(c, &t);
    return t;
}

static bool cursor_read_chunk(cursor *c, fourcc fourcc_out, cursor *chunk_out)
{
    // A chunk has a four char code, then a 4-byte length (big endian),
    // then <length> bytes of data.
    if (c->pos + 4 + 4 > c->size)
        return false;
    if (fourcc_out)
        for (size_t i = 0; i < 4; i++)
            fourcc_out[i] = (char) c->start[c->pos + i];
    c->pos += 4;
    uint32_t chunk_length;
    if (!cursor_read_i4(c, &chunk_length))
        return false;
    if (c->pos + chunk_length > c->size)
        return false;
    if (chunk_out) {
        chunk_out->start = c->start + c->pos;
        chunk_out->pos = 0;
        chunk_out->size = chunk_length;
    }
    c->pos += chunk_length;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// MIDI_file

MIDI_status init_MIDI_file(const char *data, size_t size, MIDI_file *mf_out)
{
    memset(mf_out, 0, sizeof *mf_out);
    mf_out->start = (const uint8_t *)data;
    mf_out->size = size;
    cursor fc;                  // file cursor
    init_cursor((const uint8_t *)data, size, &fc);

    // parse header
    // "MThd"
    // header length, 4 bytes, must be >= 6
    // format,        2 bytes
    // ntracks,       2 bytes
    // division,      2 bytes
    // unknown stuff, ??? bytes
    fourcc head_fourcc;
    cursor hc;                  // header cursor
    if (!cursor_read_chunk(&fc, head_fourcc, &hc))
        return MS_FILE_TRUNCATED;
    if (!fourcc_eq(head_fourcc, "MThd"))
        return MS_NOT_MIDI_FILE;
    uint16_t format, track_count, division;
    if (!cursor_read_i2(&hc, &format) ||
        !cursor_read_i2(&hc, &track_count) ||
        !cursor_read_i2(&hc, &division))
    {
        return MS_FILE_TRUNCATED;
    }
    if (format > 2)
        return MS_UNKNOWN_FORMAT;
    if (format == FORMAT_0 && track_count != 1)
        return MS_INVALID_TRACK_COUNT;
    mf_out->format = format;
    mf_out->track_count = track_count;
    mf_out->division = division;
    mf_out->tracks = calloc(track_count, sizeof *mf_out->tracks);
    if (!mf_out->tracks)
        return MS_NO_MEM;

    // Read chunks, save the MTrk chunks, until we have enough.
    uint16_t track = 0;
    while (track < track_count) {
        // MIDI files may, but are not required to, have chunks
        // aligned to even byte boundaries.
        // Our heuristic is, if the first byte is NUL and the
        // offset is odd, we'll skip one byte.
        if ((fc.pos & 1) && fc.start[fc.pos] == 0)
            fc.pos++;
        fourcc track_fourcc;
        cursor tc;              // track cursor
        if (!cursor_read_chunk(&fc, track_fourcc, &tc))
            return MS_FILE_TRUNCATED;
        if (!fourcc_eq(track_fourcc, "MTrk"))
            continue;
        MIDI_track *tp = &mf_out->tracks[track];
        tp->start = tc.start;
        tp->size = tc.size;
        track++;
    }
    return MS_OK;
}

void destroy_MIDI_file(MIDI_file *mf)
{
    free(mf->tracks);
    memset(mf, 0, sizeof *mf);
}

////////////////////////////////////////////////////////////////////////////////
// MIDI_iterator

struct MIDI_track_state {
    cursor cur;
    uint32_t time;
    uint8_t running_status;
    uint8_t channel;
};

static MIDI_status init_iterator(const MIDI_file *mf,
                                 size_t track_count,
                                 MIDI_iterator *it_out)
{
    MIDI_track_state *tracks = calloc(track_count, sizeof *tracks);
    if (!tracks)
        return MS_NO_MEM;

    memset(it_out, 0, sizeof *it_out);
    it_out->file = mf;
    it_out->timing.usec_per_quarter = 500000;   // default 120 bpm
    it_out->timing.division = mf->division;
    it_out->timing.sig_numerator = 4;           // 4/4 time
    it_out->timing.sig_denominator = 4;         // (N.B., not log denominator)
    it_out->timing.sig_cc = 24;                 // 24 clocks/quarter
    it_out->timing.sig_bb = 8;                  // 8 32nds/quarter
    it_out->time_ticks = 0;
    it_out->time_usecs = 0;

    it_out->track_count = track_count;
    it_out->tracks = tracks;
    return MS_OK;
}

MIDI_status init_MIDI_file_iterator(const MIDI_file *mf, MIDI_iterator *it_out)
{
    if (mf->format != FORMAT_0 && mf->format != FORMAT_1)
        return MS_NOT_ITERABLE;

    MIDI_status status = init_iterator(mf, mf->track_count, it_out);
    if (status)
        return status;

    for (size_t i = 0; i < mf->track_count; i++) {
        // cursor *tc = &it_out->track_cursors[i];
        cursor *tc = &it_out->tracks[i].cur;
        const MIDI_track *track = &mf->tracks[i];
        init_cursor(track->start, track->size, tc);
        it_out->tracks[i].time = cursor_read_time(tc);
    }
    return MS_OK;
}

MIDI_status init_MIDI_track_iterator(const MIDI_file *mf,
                                     size_t track_no,
                                     MIDI_iterator *it_out)
{
    MIDI_status status = init_iterator(mf, 1, it_out);
    if (status)
        return status;

    // cursor *tc = &it_out->track_cursors[0];
    cursor *tc = &it_out->tracks[0].cur;
    const MIDI_track *track = &mf->tracks[track_no];
    init_cursor(track->start, track->size, tc);
    it_out->tracks[0].time = cursor_read_time(tc);
    return MS_OK;
}

void destroy_MIDI_iterator(MIDI_iterator *it)
{
    free(it->tracks);
    memset(it, 0, sizeof *it);
}

static void process_meta_event(const MIDI_event *evt,
                               MIDI_iterator *it,
                               MIDI_track_state *track)
{
    switch (evt->status_byte) {

    // case 0x00:                  // FF 00 02 Sequence Number
    //     break;

    // case 0x01:                  // FF 01 len text Text Event
    //     break;

    // case 0x02:                  // FF 02 len text Copyright Notice
    //     break;

    // case 0x03:                  // FF 03 len text Sequence/Track Name
    //     break;

    // case 0x04:                  // FF 04 len text Instrument Name
    //     break;

    // case 0x05:                  // FF 05 len text Lyric
    //     break;

    // case 0x06:                  // FF 06 len text Marker
    //     break;

    // case 0x07:                  // FF 07 len text Cue Point
    //     break;

    case 0x20:                  // FF 20 01 cc MIDI Channel Prefix
        if (evt->data_size)
            track->channel = evt->data_bytes[0];
        break;

    case 0x2F:                  // FF 2F 00 End of Track
        track->cur.pos = track->cur.size;
        break;

    case 0x51:                  // FF 51 03 tttttt Set Tempo
                                // (in microseconds per MIDI quarter-note)
        if (evt->data_size == 3) {
            uint32_t tmp;
            if (cursor_read_i3(&track->cur, &tmp))
                it->timing.usec_per_quarter = tmp;
        }
        break;

    // case 0x54:                  // FF 54 05 hr mn se fr ff SMPTE Offset
    //     break;

    case 0x58:                  // FF 58 04 nn dd cc bb Time Signature
        if (evt->data_size == 4) {
            it->timing.sig_numerator = evt->data_bytes[0];
            it->timing.sig_denominator = 1 << evt->data_bytes[1];
            it->timing.sig_cc = evt->data_bytes[2];
            it->timing.sig_bb = evt->data_bytes[3];
        }
        break;

    // case 0x59:                  // FF 59 02 sf mi Key Signature
    //     break;

    // case 0x7F:              // FF 7F len data Sequencer Specific Meta-Event
    //     break;

    default:
        break;
    }
}

static bool read_event(MIDI_iterator *it,
                       MIDI_track_state *track,
                       MIDI_event *evt_out)
{
    MIDI_event evt_tmp;
    memset(&evt_tmp, 0, sizeof evt_tmp);
    cursor *tc = &track->cur;

    evt_tmp.raw_bytes = cursor_ptr(tc);
    evt_tmp.channel = track->channel;

    uint8_t b0;
    if (!cursor_peek_i1(tc, &b0))
        return false;
    if (b0 & 0x80) {            // consume status byte
        (void)cursor_read_i1(tc, NULL);
    } else {                    // This event uses running status.
        b0 = track->running_status;
        if (b0 == 0)
            return false;
    }
    uint8_t b0l = b0 & 0xF;
    uint8_t b0h = b0 >> 4;

    evt_tmp.status_byte = b0;
    evt_tmp.data_bytes = cursor_ptr(tc);

    if (b0h < 0xF) {     // 80-EF - channel voice message
        track->channel = 0;
        track->running_status = b0;
        evt_tmp.type = ET_MIDI;
        evt_tmp.channel = b0l;
        // 0xC (Program Change) and 0xD (Channel Pressure) are two-byte
        // messages; the others are three bytes.
        evt_tmp.data_size = (b0h == 0xC || b0h == 0xD) ? 1 : 2;
    } else if (b0 == 0xF0) {    // F0 - SYSEX first segment
        evt_tmp.type = ET_SYSEX;
        track->running_status = 0;
        uint32_t tmp;
        if (!cursor_read_vlq(tc, &tmp))
            return false;
        evt_tmp.data_size = tmp;
    } else if (b0 == 0xF7) {    // F7 - SYSEX continuation
        evt_tmp.type = ET_RAW;
        track->running_status = 0;
        uint32_t tmp;
        if (!cursor_read_vlq(tc, &tmp))
            return false;
        evt_tmp.data_size = tmp;
    } else if (b0 == 0xFF) {    // FF - meta event
        evt_tmp.type = ET_META;
        track->running_status = 0;
        if (!cursor_read_i1(tc, &evt_tmp.status_byte))
            return false;
        uint32_t tmp;
        if (!cursor_read_vlq(tc, &tmp))
            return false;
        evt_tmp.data_bytes = cursor_ptr(tc);
        evt_tmp.data_size = tmp;
        process_meta_event(&evt_tmp, it, track);
    } else {                    // F1-F6, F8-FE - illegal
        return false;
    }
    cursor_read_bytes(tc, evt_tmp.data_size, NULL);
    if (evt_out) {
        // evt_out->timestamp and ->track are set to zero here.
        // Override in caller.
        *evt_out = evt_tmp;
    }

    return true;
}

uint32_t MIDI_iter_next(MIDI_iterator *it, MIDI_event *evt_out)
{
    do {

        // Find track with earliest event.
        // When tied, take lowest numbered track.
        size_t best_i = 0;
        uint32_t best_time = MIDI_ITER_END;
        for (size_t i = 0; i < it->track_count; i++) {
            uint32_t t = it->tracks[i].time;
            if (best_time > t) {
                best_time = t;
                best_i = i;
            }
        }

        // All tracks are finished.
        if (best_time == MIDI_ITER_END)
            return MIDI_ITER_END;

        // collect the next event.
        MIDI_track_state *track = &it->tracks[best_i];
        if (read_event(it, track, evt_out)) {
            if (evt_out) {
                evt_out->timestamp = best_time;
                evt_out->track = best_i;
            }
            uint32_t next_time = cursor_read_time(&track->cur);
            it->tracks[best_i].time = next_time;
            return next_time - best_time;
        } else {
            track->time = MIDI_ITER_END;
        }

    } while (1);                // Failed to read event.  Try again.
}

////////////////////////////////////////////////////////////////////////////////
// strings

static struct {
    const char *name;
    const char *desc;
} status_strings[] = {
#define E(s, d) [(s)] = { #s, d },
    E(MS_OK, "no error")
    E(MS_NO_MEM, "Memory allocation failed.")
    E(MS_NOT_MIDI_FILE, "File does not have \"MThd\" signature.")
    E(MS_FILE_TRUNCATED, "File is truncated.")
    E(MS_UNKNOWN_FORMAT, "Unknown format value")
    E(MS_INVALID_TRACK_COUNT, "Format 0 file must have one track.")
    E(MS_NOT_ITERABLE, "Format 2 file can't be iterated.")
    E(MS_UNPARSEABLE, "Track data can't be parsed.")
#undef E
};
const size_t status_strings_size = (&status_strings)[1] - status_strings;

const char *MIDI_status_name(MIDI_status s)
{
    if (s >= 0 && s < status_strings_size)
        return status_strings[s].name;
    return "?";
}

const char *MIDI_status_description(MIDI_status s)
{
    if (s >= 0 && s < status_strings_size)
        return status_strings[s].desc;
    return "?";
}

static const char *etype_names[] = {
#define E(t) [(t)] = #t,
    E(ET_MIDI)
    E(ET_SYSEX)
    E(ET_META)
    E(ET_RAW)
#undef E
};

const size_t etype_names_size = (&etype_names)[1] - etype_names;

const char *MIDI_event_name(MIDI_event_type t)
{
    if (t >= 0 && t < etype_names_size)
        return etype_names[t];
    return "?";
}
