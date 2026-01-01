/// getmod.c
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discord] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief fetch and install mods
/// curl moment

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "cJSON/cJSON.h"

#include "getmod.h"
#include "patch.h"
#include "log.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#endif

static void mk_ts(char* o, size_t z)
{
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    snprintf(
        o, z,
        "%04d%02d%02d-%02d-%02d-%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
}

static int ex(const char* p)
{
    FILE* f = fopen(p, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

static int rd(const char* p, uint8_t** b, size_t* z)
{
    FILE* f;
    long sz;

    *b = NULL;
    *z = 0;

    f = fopen(p, "rb");
    if (!f)
        return -1;

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz <= 0)
    {
        fclose(f);
        return -2;
    }

    *b = malloc((size_t)sz);
    if (!*b)
    {
        fclose(f);
        return -3;
    }

    if (fread(*b, 1, (size_t)sz, f) != (size_t)sz)
    {
        fclose(f);
        free(*b);
        *b = NULL;
        return -4;
    }

    fclose(f);
    *z = (size_t)sz;
    return 0;
}

static int wr(const char* p, const uint8_t* b, size_t z)
{
    FILE* f = fopen(p, "wb");
    if (!f)
        return -1;

    if (z && fwrite(b, 1, z, f) != z)
    {
        fclose(f);
        return -2;
    }

    fclose(f);
    return 0;
}

static int cp(const char* a, const char* b)
{
    FILE* i;
    FILE* o;
    char buf[4096];
    size_t n;

    i = fopen(a, "rb");
    if (!i)
        return -1;

    o = fopen(b, "wb");
    if (!o)
    {
        fclose(i);
        return -2;
    }

    while ((n = fread(buf, 1, sizeof(buf), i)) > 0)
        fwrite(buf, 1, n, o);

    fclose(i);
    fclose(o);
    return 0;
}

// makedir
static void mkp(char* p)
{
    char* s;

    for (s = p; *s; s++)
    {
        if (*s == '/' || *s == '\\')
        {
            char c = *s;
            *s = 0;
#ifdef _WIN32
            _mkdir(p);
#else
            mkdir(p, 0755);
#endif
            *s = c;
        }
    }

#ifdef _WIN32
    _mkdir(p);
#else
    mkdir(p, 0755);
#endif
}

static void norm_slash(char* s)
{
    for (; *s; s++)
        if (*s == '\\')
            *s = '/';
}

// zip and nightmares
static void mod_from_src(const char* src, char* mod, size_t mz)
{
    const char* s;
    size_t n;

    s = strrchr(src, '/');
    if (!s) s = strrchr(src, '\\');
    if (!s) s = src;
    else s++;

    strncpy(mod, s, mz - 1);
    mod[mz - 1] = 0;

    n = strlen(mod);

    // strip query junk if someone pastes github ?raw=1 or whatever
    for (size_t i = 0; i < n; i++)
    {
        if (mod[i] == '?' || mod[i] == '#')
        {
            mod[i] = 0;
            n = i;
            break;
        }
    }

    // drop extension
    if (n > 4 && strcmp(mod + (n - 4), ".zip") == 0)
        mod[n - 4] = 0;
    else if (n > 7 && strcmp(mod + (n - 7), ".tar.gz") == 0)
        mod[n - 7] = 0;
}

// mod ids cannot go do the equiv of sudo rm ./ -rf --no-preserve-root
static int ok_mod(const char* s)
{
    if (!s || !*s)
        return 0;

    // no paths no funny business
    if (strstr(s, "..") || strchr(s, '/') || strchr(s, '\\') || strchr(s, ':'))
        return 0;

    for (; *s; s++)
    {
        char c = *s;
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_' || c == '-' || c == '.')
            continue;
        return 0;
    }

    return 1;
}

static int http_get(const char* url, const char* out)
{
    char cmd[2048];

#ifdef _WIN32
    snprintf(
        cmd, sizeof(cmd),
        "curl.exe -L --fail --silent --show-error \"%s\" -o \"%s\"",
        url, out
    );
#else
    snprintf(
        cmd, sizeof(cmd),
        "curl -L --fail --silent --show-error \"%s\" -o \"%s\"",
        url, out
    );
#endif

    return system(cmd);
}

// extracto zip
static int unzip_to(const char* zip, const char* dst_dir)
{
    char cmd[2048];

#ifdef _WIN32
    //zip tar whatever
    snprintf(
        cmd, sizeof(cmd),
        "tar -xf \"%s\" -C \"%s\"",
        zip, dst_dir
    );
#else
    //linux please untar our zip
    snprintf(
        cmd, sizeof(cmd),
        "unzip -o \"%s\" -d \"%s\"",
        zip, dst_dir
    );
#endif

    return system(cmd);
}

// delete everything in patch dir that isn't .bps or .vtxg
static void clean_patch_dir(const char* dir)
{
    char cmd[4096];

#ifdef _WIN32
    // for /r is cursed but it works (and yes i hate it)
    snprintf(
        cmd, sizeof(cmd),
        "powershell -NoProfile -Command \""
        "$d='%s';"
        "Get-ChildItem -LiteralPath $d -Recurse -File | "
        "Where-Object { $_.Extension -ne '.bps' -and $_.Name -ne '.vtxg' } | "
        "Remove-Item -Force -ErrorAction SilentlyContinue\"",
        dir
    );
    system(cmd);
#else
    snprintf(
        cmd, sizeof(cmd),
        "sh -c 'cd \"%s\" && find . -type f ! -name \"*.bps\" ! -name \".vtxg\" -delete'",
        dir
    );
    system(cmd);
#endif
}

// if no .vtxg in repo/zip, fallback to list
typedef struct KM
{
    const char* id;
    const char* name;
} KM;

static const KM known[] =
{
    { "30tbps", "30 Text Boxes Per Second" },
    { NULL, NULL }
};

static int known_mod(const char* mod)
{
    for (int i = 0; known[i].id; i++)
        if (strcmp(known[i].id, mod) == 0)
            return 1;
    return 0;
}

static int fallback_vtxg_write(const char* dir, const char* mod)
{
    if(strcmp(mod, "30tbps") == 1)
    {
        char p[1024];
        FILE* f;

        snprintf(p, sizeof(p), "%s/.vtxg", dir);

        f = fopen(p, "wb");
        if (!f)
            return -1;

        fprintf(f,
            "local data = {\n"
            "  id = \"%s\",\n"
            "  patches = {\n"
            "    menu = { { mod=\"%s\", target=\"menu.bps\", dir=\"%s\" } },\n"
            "    ch1  = { { mod=\"%s\", target=\"ch1.bps\",  dir=\"%s\" } },\n"
            "    ch2  = { { mod=\"%s\", target=\"ch2.bps\",  dir=\"%s\" } },\n"
            "    ch3  = { { mod=\"%s\", target=\"ch3.bps\",  dir=\"%s\" } },\n"
            "    ch4  = { { mod=\"%s\", target=\"ch4.bps\",  dir=\"%s\" } }\n"
            "  }\n"
            "}\n"
            "install(data)\n",
            mod,
            mod, dir,
            mod, dir,
            mod, dir,
            mod, dir,
            mod, dir
        );

        fclose(f);
    }
    return 0;
}

// <pathToWin><0x00><mod><0x01><bpsTarget><0x02><bpsDir><0x03>...
static int meta_add(uint8_t** b, size_t* z, const char* s)
{
    size_t n;
    uint8_t* nb;

    n = strlen(s);
    nb = realloc(*b, *z + n);
    if (!nb)
        return -1;

    memcpy(nb + *z, s, n);
    *b = nb;
    *z += n;
    return 0;
}

static int meta_byte(uint8_t** b, size_t* z, uint8_t x)
{
    uint8_t* nb = realloc(*b, *z + 1);
    if (!nb)
        return -1;

    nb[*z] = x;
    *b = nb;
    *z += 1;
    return 0;
}

static int meta_build(const char* win, const char* mod, const char* tgt, const char* dir, uint8_t** out, size_t* oz)
{
    uint8_t* b = NULL;
    size_t z = 0;

    // arg0
    if (meta_add(&b, &z, win) != 0) goto die;
    if (meta_byte(&b, &z, 0x00) != 0) goto die;

    // entry
    if (mod && *mod)
    {
        if (meta_add(&b, &z, mod) != 0) goto die;
        if (meta_byte(&b, &z, 0x01) != 0) goto die;
    }
    else
    {
        // allow empty mod name, but keep field positions
        if (meta_byte(&b, &z, 0x01) != 0) goto die;
    }

    if (meta_add(&b, &z, tgt) != 0) goto die;
    if (meta_byte(&b, &z, 0x02) != 0) goto die;
    if (meta_add(&b, &z, dir) != 0) goto die;
    if (meta_byte(&b, &z, 0x03) != 0) goto die;

    *out = b;
    *oz  = z;
    return 0;

die:
    free(b);
    return -1;
}

// append a bps entry to file
// if file doesnt have a directory to patch sep then add one
static int meta_append_file(const char* p, const char* mod, const char* tgt, const char* dir)
{
    uint8_t* b;
    size_t z;
    size_t i;
    size_t a0;
    uint8_t* nb = NULL;
    size_t nz = 0;

    if (rd(p, &b, &z) != 0)
        return -1;

    a0 = z;
    for (i = 0; i < z; i++)
        if (b[i] == 0x00)
        {
            a0 = i;
            break;
        }

    // keep arg0 only
    nb = malloc(a0 + 1);
    if (!nb)
    {
        free(b);
        return -2;
    }

    memcpy(nb, b, a0);
    nz = a0;

    // ensure separator
    nb[nz++] = 0x00;

    // append new entry
    {
        uint8_t* e;
        size_t ez;

        if (meta_build("", mod, tgt, dir, &e, &ez) != 0)
        {
            free(nb);
            free(b);
            return -3;
        }

        size_t j = 0;
        while (j < ez && e[j] != 0x00) j++;
        if (j < ez) j++;

        nb = realloc(nb, nz + (ez - j));
        memcpy(nb + nz, e + j, ez - j);
        nz += (ez - j);

        free(e);
    }

    free(b);

    // write back
    {
        int r = wr(p, nb, nz);
        free(nb);
        return r;
    }
}

// GET_MOD:
// - downloads src (url or local) into patch/<mod>-<ts>/raw
// - if zip, extracts into patch/<mod>-<ts>/ (and deletes raw)
// - cleans directory (keeps only .bps + .vtxg)
// - if no .vtxg and mod is known: generates fallback .vtxg
int GET_MOD(const char* src, char* mod, char* ts)
{
    char* ad;
    char dst[1024];
    char raw[1024];
    int is_zip;

    ad = GET_APP_DIR();

    mk_ts(ts, 64);

    mod_from_src(src, mod, 128);
    norm_slash(mod);

    // mod id sanitization (folder name)
    if (!ok_mod(mod))
    {
        LOG_FATAL("get", "bad_mod_id", "mod name contains illegal chars (no slashes, no .., no colon, etc)");
        free(ad);
        return -1;
    }

    snprintf(dst, sizeof(dst), "%s/patch/%s-%s", ad, mod, ts);
    mkp(dst);

    snprintf(raw, sizeof(raw), "%s/raw", dst);

    is_zip = 0;
    {
        size_t n = strlen(src);
        if (n >= 4 && strcmp(src + (n - 4), ".zip") == 0)
            is_zip = 1;
    }

    if (strncmp(src, "http://", 7) == 0 || strncmp(src, "https://", 8) == 0)
    {
        if (http_get(src, raw) != 0)
        {
            LOG_FATAL("get", "http_fail", src);
            free(ad);
            return -1;
        }
    }
    else
    {
        if (cp(src, raw) != 0)
        {
            LOG_FATAL("get", "file_not_found", src);
            free(ad);
            return -1;
        }
    }

    // if it looks like a zip, extract
    if (is_zip)
    {
        if (unzip_to(raw, dst) != 0)
        {
            LOG_FATAL("get", "zip_extract_fail", src);
            free(ad);
            return -1;
        }

        remove(raw);
    }

    // keep only what patcher cares about
    clean_patch_dir(dst);

    // if no .vtxg generate fallback only for known mods
    {
        char vtxg[1024];
        snprintf(vtxg, sizeof(vtxg), "%s/.vtxg", dst);

        if (!ex(vtxg))
        {
            if (known_mod(mod))
            {
                if (fallback_vtxg_write(dst, mod) != 0)
                {
                    LOG_FATAL("get", "fallback_vtxg_fail", dst);
                    free(ad);
                    return -1;
                }
            }
            else
            {
                LOG_FATAL("get", "missing_vtxg", "mod has no .vtxg and is not in known list");
                free(ad);
                return -1;
            }
        }
    }

    LOG_INFOF("fetched mod %s at %s", mod, ts);
    free(ad);
    return 0;
}

int INSTALL_MOD(const char* game, long profile, const char* mod, const char* ts)
{
    char* ad;
    char pdir[1024];

    if (profile == 1)
    {
        LOG_FATAL("install", "denied", "please do not modify vanilla profile");
        return -1;
    }

    if (!ok_mod(mod))
    {
        LOG_FATAL("install", "bad_mod_id", "mod id is illegal");
        return -1;
    }

    ad = GET_APP_DIR();

    snprintf(pdir, sizeof(pdir), "%s/patch/%s-%s", ad, mod, ts);
    if (!ex(pdir))
    {
        // ex() uses fopen so directory check is fake. just ❤️Proceed
    }

    // this installer step is intentionally stupid ass
    //
    // mod\x01ch1.bps\x02<appdir>/patch/mod-ts\x03
    // to the profile<profile>_<chapter> file.=
    //
    // if you want chapter subdirs later, set dir accordingly in metadata.=

    // add entries for common files if they exist in patch dir
    {
        const char* chs[] = { "menu", "ch1", "ch2", "ch3", "ch4" };
        const char* bpss[] = { "menu.bps", "ch1.bps", "ch2.bps", "ch3.bps", "ch4.bps" };

        for (int i = 0; i < 5; i++)
        {
            char bpsp[1024];
            snprintf(bpsp, sizeof(bpsp), "%s/%s", pdir, bpss[i]);

            if (!ex(bpsp)) { continue; }

            char gsj[1024];
            uint8_t* gb;
            size_t gz;
            cJSON* gs;
            cJSON* g;
            cJSON* di;

            snprintf(gsj, sizeof(gsj), "%s/game/games.json", ad);
            if (rd(gsj, &gb, &gz) != 0)
            {
                LOG_FATAL("install", "file_not_found", gsj);
                free(ad);
                return -1;
            }

            gs = cJSON_Parse((char*)gb);
            free(gb);

            if (!gs)
            {
                LOG_FATAL("install", "cjson_parse_err", gsj);
                free(ad);
                return -1;
            }

            g  = cJSON_GetObjectItemCaseSensitive(gs, game);
            di = g ? cJSON_GetObjectItemCaseSensitive(g, "data") : NULL;

            if (!di || !cJSON_IsString(di))
            {
                cJSON_Delete(gs);
                LOG_FATAL("install", "invalid_game", "missing data in games.json");
                free(ad);
                return -1;
            }

            // normalize "./deltarune" to "deltarune"
            const char* d = cJSON_GetStringValue(di);
            while (*d == '.' || *d == '/' || *d == '\\') d++;

            char mfp[1024];
            snprintf(mfp, sizeof(mfp), "%s/game/%s/profile/profile%ld/profile%ld_%s", ad, d, profile, profile, chs[i]);

            if (meta_append_file(mfp, mod, bpss[i], pdir) != 0)
            {
                cJSON_Delete(gs);
                LOG_FATAL("install", "meta_write_fail", mfp);
                free(ad);
                return -1;
            }

            cJSON_Delete(gs);
        }
    }

    PATCH_MOD(game, profile, mod, ts);

    LOG_INFOF("installed mod %s into profile %ld", mod, profile);
    free(ad);
    return 0;
}
