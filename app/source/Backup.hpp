#pragma once
// OpenPak's system files on the SD card: the sysmodule and title patches, the plugin and
// the Juxt certificate. Same rules as the Switch Patcher: before OpenPak replaces a file it
// keeps the original as <key>.original (or <key>.absent when there was none); the OpenPak
// version is kept as <key>.managed; selecting Nintendo puts the exact original back.
#include <stddef.h>

#define OPENPAK_STATE_DIR "/3ds/openpak/system"

namespace Backup {
    // Copies the files inside the app (romfs:/openpak, or the old update folder) into the managed
    // copies when they differ. Touches nothing live. Returns how many changed, -1 on failure.
    int stage(char *err, size_t errlen);
    // Number of managed files, and how many of them are live in the SD card layout.
    int managed();
    int live();
    // OpenPak selected: back up what is there, then install the managed files.
    bool install(char *err, size_t errlen);
    // Nintendo selected: restore the originals exactly.
    bool remove(char *err, size_t errlen);
}
