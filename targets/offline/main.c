#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "midi-file.h"

static MIDI_file mf;

static bool is_text_event(const MIDI_event *evt)
{
    if (evt->type == ET_META)
        switch (evt->status_byte) {
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
                return true;
        }
    return false;
}

static void play_file(const char *file_name)
{
    MIDI_status s;

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror(file_name);
        exit(1);
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror(file_name);
        exit(1);
    }
    void *addr = mmap(NULL,
                      st.st_size,
                      PROT_READ,
                      MAP_FILE | MAP_PRIVATE,
                      fd,
                      0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    s = init_MIDI_file(addr, st.st_size, &mf);
    if (s) {
        fprintf(stderr, "init_MIDI_file: %d %s, %s\n",
                s, MIDI_status_name(s), MIDI_status_description(s));
        exit(1);
    }

    printf("MIDI format %u, %lu track%s\n",
           mf.format, mf.track_count, &"s"[mf.track_count == 1]);

    MIDI_iterator it;
    s = init_MIDI_file_iterator(&mf, &it);
    if (s) {
        fprintf(stderr, "init_MIDI_file_iterator: %d %s, %s\n",
                s, MIDI_status_name(s), MIDI_status_description(s));
        exit(1);
    }

    MIDI_event evt;
    int i = 0;
    while (MIDI_iter_next(&it, &evt) != MIDI_ITER_END) {
        printf("event %-7s %x", MIDI_event_name(evt.type), evt.status_byte);
        if (is_text_event(&evt)) {
            printf(" \"%*s\"",
                   (int)evt.data_size, (const char *)evt.data_bytes);
        } else {
            for (size_t j = 0; j < evt.data_size; j++)
                printf(" %x", evt.data_bytes[j]);
        }
        printf("\n");
        i++;
    }
    printf("%d events\n", i);
    destroy_MIDI_iterator(&it);
    destroy_MIDI_file(&mf);
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "use: offline file.mid\n");
        exit(1);
    }
    play_file(argv[1]);
    return 0;
}
