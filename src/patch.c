/// patch.c
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discord] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief applies bps patches using profile1 as vanilla
/// "we could have used lift bro" - me at 31 december (holiday) 2025 12:25 am

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "cJSON/cJSON.h"
#include "log.h"
#include "patch.h"

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #ifndef MAX_PATH
        #define MAX_PATH 260
    #endif
#else
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
    #include <errno.h>
    #ifndef MAX_PATH
        #define MAX_PATH 4096
    #endif
#endif

// dir helpers
char* GET_APP_DIR()
{
    #ifdef _WIN32
        char path[MAX_PATH];
        char pathh[MAX_PATH];
        HMODULE hModule = GetModuleHandle(NULL);
        GetModuleFileName(hModule, path, MAX_PATH);
        char *last_sep = strrchr(path, '\\');
        char *last_sepFwd = strrchr(path, '/');
        if (last_sep) {
            *last_sep = '\0';
        }
        if (last_sepFwd) {
            *last_sepFwd = '\0';
        }
        snprintf(pathh, MAX_PATH, "%s/../", path);
    #else
        char path[PATH_MAX];
        char pathh[PATH_MAX];
        readlink("/proc/self/exe", path, sizeof(path) - 1);
        char *last_sep = strrchr(path, '\\');
        char *last_sepFwd = strrchr(path, '/');
        if (last_sep) {
            *last_sep = '\0';
        }
        if (last_sepFwd) {
            *last_sepFwd = '\0';
        }
        snprintf(pathh, PATH_MAX, "%s/../", path);
    #endif

    return strdup(pathh);
}

void GET_DIR(char *s)
{
    char *l = 0;
    if (!s) return;
    for (; *s; ++s) if (*s=='/'||*s=='\\') l=s;
    if (l) l[1]=0; else *s=0;
}

// file exists?
static int ex(const char* p)
{
    FILE* f = fopen(p, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

// read file
static int rd(const char* p, uint8_t** b, size_t* z)
{
    FILE* f;
    long sz;

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
        return -4;
    }

    fclose(f);
    *z = (size_t)sz;
    return 0;
}

// write to file
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

// copy file
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

// make dir
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
}

// read profile chapter metadata arg0
// all data before the first 0x0
static char* arg0(const char* p)
{
    FILE* f;
    long sz;
    uint8_t* b;
    size_t i, n;
    char* s;

    f = fopen(p, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz <= 0)
    {
        fclose(f);
        return NULL;
    }

    b = malloc((size_t)sz);
    fread(b, 1, (size_t)sz, f);
    fclose(f);

    n = (size_t)sz;
    for (i = 0; i < (size_t)sz; i++)
    {
        if (b[i] == 0x00)
        {
            n = i;
            break;
        }
    }

    if (n == 0)
    {
        free(b);
        return NULL;
    }

    s = malloc(n + 1);
    memcpy(s, b, n);
    s[n] = 0;
    free(b);

    return s;
}

// read bps block (bytes after first 0x00)
// ends at EOF or next 0x00
static int bpsblk(const char* p, uint8_t** bb, size_t* bz)
{
    FILE* f;
    long sz;
    uint8_t* b;
    size_t i;
    size_t s;
    size_t e;

    *bb = NULL;
    *bz = 0;

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

    b = malloc((size_t)sz);
    if (!b)
    {
        fclose(f);
        return -3;
    }

    if (fread(b, 1, (size_t)sz, f) != (size_t)sz)
    {
        fclose(f);
        free(b);
        return -4;
    }

    fclose(f);

    for (i = 0; i < (size_t)sz; i++)
        if (b[i] == 0x00)
            break;

    if (i >= (size_t)sz)
    {
        free(b);
        return 0;
    }

    s = i + 1;
    if (s >= (size_t)sz)
    {
        free(b);
        return 0;
    }

    e = (size_t)sz;
    for (i = s; i < (size_t)sz; i++)
    {
        if (b[i] == 0x00)
        {
            e = i;
            break;
        }
    }

    if (e <= s)
    {
        free(b);
        return 0;
    }

    *bz = e - s;
    *bb = malloc(*bz);
    memcpy(*bb, b + s, *bz);

    free(b);
    return 0;
}

// bps varint
static uint64_t vi(const uint8_t** p)
{
    uint64_t v = 0;
    uint64_t s = 1;

    for (;;)
    {
        uint8_t x = *(*p)++;
        v += (x & 0x7F) * s;
        if (x & 0x80)
            break;
        s <<= 7;
        v += s;
    }

    return v;
}

// minimal bps apply cause lazy
static int bps(const uint8_t* in, size_t iz, const uint8_t* pt, size_t pz, uint8_t** out, size_t* oz)
{
    const uint8_t* p;
    uint64_t s0, s1, ms;
    uint8_t* o;
    size_t ip = 0;
    size_t op = 0;
    int64_t so = 0;
    int64_t to = 0;

    if (pz < 4 || memcmp(pt, "BPS1", 4) != 0)
        return -1;

    p = pt + 4;
    s0 = vi(&p);
    s1 = vi(&p);
    ms = vi(&p);

    if (s0 != iz)
        return -2;

    p += ms;
    o = calloc(1, (size_t)s1);

    while (op < (size_t)s1)
    {
        uint64_t d = vi(&p);
        uint64_t l = (d >> 2) + 1;

        switch (d & 3)
        {
            case 0:
                memcpy(o + op, in + ip, l);
                ip += l;
                op += l;
                break;

            case 1:
                memcpy(o + op, p, l);
                p += l;
                op += l;
                break;

            case 2:
            {
                uint64_t of = vi(&p);
                so += (of & 1) ? -(int64_t)(of >> 1) : (int64_t)(of >> 1);
                memcpy(o + op, in + so, l);
                op += l;
                break;
            }

            case 3:
            {
                uint64_t of = vi(&p);
                to += (of & 1) ? -(int64_t)(of >> 1) : (int64_t)(of >> 1);
                memmove(o + op, o + to, l);
                op += l;
                break;
            }
        }
    }

    *out = o;
    *oz  = (size_t)s1;
    return 0;
}

// parse and apply chain:
// <name><0x01><path><0x02><name><0x01><path><0x02>...
static void chain_apply(uint8_t* bb, size_t bz, const uint8_t* vin, size_t vz, uint8_t** out, size_t* oz)
{
    uint8_t* cur;
    size_t cz;
    size_t p = 0;

    cur = malloc(vz);
    memcpy(cur, vin, vz);
    cz = vz;

    while (p < bz)
    {
        size_t n0 = p;
        size_t n1 = n0;
        size_t p0;
        size_t p1;

        for (; n1 < bz; n1++)
            if (bb[n1] == 0x01)
                break;

        if (n1 >= bz)
            break;

        p0 = n1 + 1;
        p1 = p0;

        for (; p1 < bz; p1++)
            if (bb[p1] == 0x02)
                break;

        if (p0 >= p1)
        {
            p = (p1 < bz) ? (p1 + 1) : bz;
            continue;
        }

        char* fp = malloc((p1 - p0) + 1);
        memcpy(fp, bb + p0, (p1 - p0));
        fp[p1 - p0] = 0;

        uint8_t* pb;
        size_t pz;
        uint8_t* ob;
        size_t nz;

        if (rd(fp, &pb, &pz) == 0)
        {
            if (bps(cur, cz, pb, pz, &ob, &nz) == 0)
            {
                free(cur);
                cur = ob;
                cz = nz;
            }
            free(pb);
        }

        free(fp);
        p = (p1 < bz) ? (p1 + 1) : bz;
    }

    *out = cur;
    *oz  = cz;
}

/// @brief patch a profile
///
/// @param game     target game
/// @param profile  target profile
/// @param mod      mod name
/// @param ts       timestamp of download
int PATCH_MOD(const char* game, long profile, const char* mod, const char* ts)
{
    if(profile == 1) { LOG_FATAL("patch", "denied", "please do not modify the vanilla profile"); return -1; }

    char* ad;
    char gsj[MAX_PATH];
    char datp[MAX_PATH];

    cJSON* gs;
    cJSON* g;
    cJSON* di;
    cJSON* dj;
    cJSON* pf;

    char pr1[MAX_PATH];
    char prN[MAX_PATH];

    ad = GET_APP_DIR();

    snprintf(gsj, MAX_PATH, "%s/game/games.json", ad);
    {
        uint8_t* b;
        size_t z;
        rd(gsj, &b, &z);
        gs = cJSON_Parse((char*)b);
        free(b);
    }

    g  = cJSON_GetObjectItemCaseSensitive(gs, game);
    di = cJSON_GetObjectItemCaseSensitive(g, "data");

    snprintf(datp, MAX_PATH, "%s/game/%s/dat", ad, cJSON_GetStringValue(di));
    {
        uint8_t* b;
        size_t z;
        rd(datp, &b, &z);
        dj = cJSON_Parse((char*)b);
        free(b);
    }

    pf = cJSON_GetObjectItemCaseSensitive(dj, "pfpath");

    // profile1 is vanilla unless user goes enshittifing the data
    snprintf(pr1, MAX_PATH, "%s/game/%s/profile/profile1", ad, cJSON_GetStringValue(di));
    snprintf(prN, MAX_PATH, "%s/game/%s/profile/profile%ld", ad, cJSON_GetStringValue(di), profile);

    mkp(prN);

    cJSON* it;
    cJSON_ArrayForEach(it, pf)
    {
        const char* k = it->string;
        const char* pfroot = cJSON_GetStringValue(it);

        char p1f[MAX_PATH];
        char pnf[MAX_PATH];

        char* r1;
        char* rn;

        uint8_t* bb;
        size_t bz;

        char v0[MAX_PATH];
        char v1[MAX_PATH];
        char tmp[MAX_PATH];

        uint8_t *vb, *ob;
        size_t vz, oz;

        snprintf(p1f, MAX_PATH, "%s/profile1_%s", pr1, k);
        snprintf(pnf, MAX_PATH, "%s/profile%ld_%s", prN, profile, k);

        r1 = arg0(p1f);
        rn = arg0(pnf);

        if (!r1 || !rn)
            continue;

        bpsblk(pnf, &bb, &bz);

        snprintf(v0, MAX_PATH, "%s/%s", pfroot, r1);
        snprintf(v1, MAX_PATH, "%s/%s", pfroot, rn);
        snprintf(tmp, MAX_PATH, "%s.tmp", v1);

        // if no bps chain just copy
        if (!bb || bz == 0)
        {
            cp(v0, tmp);
        }
        else
        {
            rd(v0, &vb, &vz);
            chain_apply(bb, bz, vb, vz, &ob, &oz);
            wr(tmp, ob, oz);
            free(vb);
            free(ob);
            free(bb);
        }

        if (ex(v1))
            remove(v1);
        rename(tmp, v1);

        free(r1);
        free(rn);
    }

    cJSON_Delete(dj);
    cJSON_Delete(gs);
    free(ad);

    LOG_INFOF("patched profile %ld with %s downloaded at %s", profile, mod, ts);
    return 0;
}
