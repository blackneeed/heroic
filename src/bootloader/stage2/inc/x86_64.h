#ifndef X86_64_H
#define X86_64_H
#include <stdint.h>

typedef struct _disk_params {
    uint8_t drive_number;
    uint8_t edd_present;
    uint8_t sectors_per_track;
    uint8_t heads;
} __attribute__((packed)) disk_params_t;

extern void bios_get_disk_geometry(disk_params_t *params);
extern void bios_check_edd_presence(disk_params_t *params);
#endif