#include "Backup.hpp"
#ifdef SD_ROOT_HOST
#define NIMBUS_UPDATE_PATH "/3ds/nimbus/update"
#else
#include "common.hpp"
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>

#ifndef ROMFS_ROOT
#define ROMFS_ROOT "romfs:"
#endif
#ifndef SD_ROOT
#define SD_ROOT ""
#endif

namespace {
struct Item { const char *key, *target, *source; };
const Item items[] = {
    {"act",       "/luma/sysmodules/0004013000003202.ips", "0004013000003202.ips"},
    {"friends",   "/luma/sysmodules/0004013000003802.ips", "0004013000003802.ips"},
    {"http",      "/luma/sysmodules/0004013000002902.ips", "0004013000002902.ips"},
    {"socket",    "/luma/sysmodules/0004013000002E02.ips", "0004013000002E02.ips"},
    {"ssl",       "/luma/sysmodules/0004013000002F02.ips", "0004013000002F02.ips"},
    {"miiverse1", "/luma/titles/000400300000BC02/code.ips", "000400300000BC02.ips"},
    {"miiverse2", "/luma/titles/000400300000BD02/code.ips", "000400300000BD02.ips"},
    {"miiverse3", "/luma/titles/000400300000BE02/code.ips", "000400300000BE02.ips"},
    {"plugin",    "/luma/plugins/nimbus.3gx",              "nimbus.3gx"},
    {"juxt",      "/3ds/juxt-prod.pem",                    "juxt-prod.pem"},
};
const size_t N = sizeof(items) / sizeof(items[0]);
std::string sd(const char *p) { return std::string(SD_ROOT) + p; }
const char *CHANGED = "System files changed; originals kept in " OPENPAK_STATE_DIR;

bool exists(const char *p) { struct stat st; return stat(p, &st) == 0; }

bool read(const char *p, std::vector<unsigned char> &out) {
    FILE *f = fopen(p, "rb");
    if (!f) return false;
    out.clear();
    unsigned char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) out.insert(out.end(), buf, buf + n);
    bool ok = !ferror(f);
    fclose(f);
    return ok;
}

bool same(const char *a, const char *b) {
    std::vector<unsigned char> x, y;
    return read(a, x) && read(b, y) && x == y;
}

bool ours(const std::string &target, const char *managed, const char *previous) {
    return (exists(managed) && same(target.c_str(), managed)) || (exists(previous) && same(target.c_str(), previous));
}

void mkdirs(const char *file) {
    char p[256];
    snprintf(p, sizeof(p), "%s", file);
    for (char *c = p + 1; *c; ++c)
        if (*c == '/') { *c = 0; mkdir(p, 0777); *c = '/'; }
}

// Write through a temp file, so an interrupted copy never leaves half a file under the real name.
bool copy(const char *src, const char *dst) {
    std::vector<unsigned char> data;
    if (!read(src, data)) return false;
    mkdirs(dst);
    char tmp[300];
    snprintf(tmp, sizeof(tmp), "%s.openpak-tmp", dst);
    FILE *f = fopen(tmp, "wb");
    if (!f) return false;
    bool ok = data.empty() || fwrite(data.data(), 1, data.size(), f) == data.size();
    ok = fclose(f) == 0 && ok;
    if (!ok) { std::remove(tmp); return false; }
    std::remove(dst);
    return std::rename(tmp, dst) == 0;
}

bool mark(const char *p) { mkdirs(p); FILE *f = fopen(p, "wb"); if (!f) return false; fclose(f); return true; }

struct Paths { char managed[200], original[200], absent[200], previous[200]; };
Paths paths(const Item &it) {
    Paths p;
    snprintf(p.managed, sizeof(p.managed), SD_ROOT OPENPAK_STATE_DIR "/%s.managed", it.key);
    snprintf(p.original, sizeof(p.original), SD_ROOT OPENPAK_STATE_DIR "/%s.original", it.key);
    snprintf(p.absent, sizeof(p.absent), SD_ROOT OPENPAK_STATE_DIR "/%s.absent", it.key);
    snprintf(p.previous, sizeof(p.previous), SD_ROOT OPENPAK_STATE_DIR "/%s.previous", it.key);
    return p;
}
}

namespace Backup {

int stage(char *err, size_t errlen) {
    int changed = 0;
    for (size_t i = 0; i < N; ++i) {
        // The files inside the app are the current ones; the update folder is how earlier
        // releases delivered them and still works.
        char from_app[200], from_sd[260];
        snprintf(from_app, sizeof(from_app), ROMFS_ROOT "/openpak/%s", items[i].source);
        snprintf(from_sd, sizeof(from_sd), SD_ROOT NIMBUS_UPDATE_PATH "/%s", items[i].source);
        Paths p = paths(items[i]);
        const char *src = exists(from_app) ? from_app : exists(from_sd) ? from_sd : nullptr;
        if (!src) continue;
        if (!exists(p.managed) || !same(src, p.managed)) {
            // The copy an earlier release installed: still recognised as OpenPak's until it is replaced.
            if (exists(p.managed) && !exists(p.previous) && !copy(p.managed, p.previous)) {
                snprintf(err, errlen, "Could not stage %s", items[i].key); return -1;
            }
            if (!copy(src, p.managed)) { snprintf(err, errlen, "Could not stage %s", items[i].key); return -1; }
            ++changed;
        }
        if (src == from_sd) std::remove(src);
    }
    return changed;
}

int managed() {
    int n = 0;
    for (size_t i = 0; i < N; ++i) if (exists(paths(items[i]).managed)) ++n;
    return n;
}

int live() {
    int n = 0;
    for (size_t i = 0; i < N; ++i) {
        Paths p = paths(items[i]);
        std::string t = sd(items[i].target);
        if (exists(p.managed) && exists(t.c_str()) && same(t.c_str(), p.managed)) ++n;
    }
    return n;
}

bool install(char *err, size_t errlen) {
    // Check everything first, so a file somebody else changed stops the switch before any write.
    for (size_t i = 0; i < N; ++i) {
        Paths p = paths(items[i]);
        std::string t = sd(items[i].target);
        if (!exists(p.managed) || !exists(t.c_str())) continue;
        bool recorded = exists(p.original) || exists(p.absent);
        if (recorded && !ours(t, p.managed, p.previous) && !(exists(p.original) && same(t.c_str(), p.original))) {
            snprintf(err, errlen, "%s", CHANGED);
            return false;
        }
    }
    for (size_t i = 0; i < N; ++i) {
        Paths p = paths(items[i]);
        std::string t = sd(items[i].target);
        if (!exists(p.managed)) continue;
        if (!exists(p.original) && !exists(p.absent)) {
            bool ok = (exists(t.c_str()) && !ours(t, p.managed, p.previous)) ? copy(t.c_str(), p.original) : mark(p.absent);
            if (!ok) { snprintf(err, errlen, "Could not back up %s", items[i].key); return false; }
        }
        if (!(exists(t.c_str()) && same(t.c_str(), p.managed)) && !copy(p.managed, t.c_str())) {
            snprintf(err, errlen, "Could not install %s", items[i].key);
            return false;
        }
        std::remove(p.previous);
    }
    return true;
}

bool remove(char *err, size_t errlen) {
    for (size_t i = 0; i < N; ++i) {
        Paths p = paths(items[i]);
        std::string t = sd(items[i].target);
        if (!exists(p.original) && !exists(p.absent)) continue;
        if (!exists(t.c_str())) continue;
        if (!ours(t, p.managed, p.previous) && !(exists(p.original) && same(t.c_str(), p.original))) {
            snprintf(err, errlen, "%s", CHANGED);
            return false;
        }
    }
    for (size_t i = 0; i < N; ++i) {
        Paths p = paths(items[i]);
        std::string t = sd(items[i].target);
        bool hasOrig = exists(p.original), hasAbsent = exists(p.absent);
        if (!hasOrig && !hasAbsent) continue;
        if (hasOrig) {
            if (!(exists(t.c_str()) && same(t.c_str(), p.original)) && !copy(p.original, t.c_str())) {
                snprintf(err, errlen, "Could not restore system setup; originals kept");
                return false;
            }
        } else if (exists(t.c_str())) {
            std::remove(t.c_str());
        }
        std::remove(p.original);
        std::remove(p.absent);
        std::remove(p.previous);
    }
    return true;
}

}
