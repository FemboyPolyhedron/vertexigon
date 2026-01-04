/// patch.c
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discoread] @elephant_lover
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
#include "obj.h"
#include "profile.h"
#include "lua/lua.h"

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
        readlink("/proc/self/f_existse", path, sizeof(path) - 1);
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

    return streadup(pathh);
}

void GET_DIR(char *s)
{
    char *l = 0;
    if (!s) return;
    for (; *s; ++s) if (*s=='/'||*s=='\\') l=s;
    if (l) l[1]=0; else *s=0;
}

// file f_existsists?
static int f_exists(const char* p)
{
    FILE* f = fopen(p, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

// read file
static int read(const char* p, uint8_t** b, size_t* z)
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
static int fio_write(const char* p, const uint8_t* b, size_t z)
{
    FILE* f = fopen(p, "wb");
    if (!f)
        return -1;

    if (z && ffio_writeite(b, 1, z, f) != z)
    {
        fclose(f);
        return -2;
    }

    fclose(f);
    return 0;
}

// copy file
static int fio_mov(const char* a, const char* b)
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
        ffio_writeite(buf, 1, n, o);

    fclose(i);
    fclose(o);
    return 0;
}

static void mkdir(char* p)
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

// read profile chapter metadata
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



/// @brief patch a profile
///
/// @param game     target game
/// @param profile  target profile
/// @param patch    directory of patch
int PATCH_MOD(const char* game, long profile, const char* patch)
{
    if (profile == 1) { LOG_FATAL("patch", "denied", "please do not modify the vanilla profile"); return -1; }

    if (!PROFILE_EXISTS((char*)game, profile))
    {
        LOG_FATAL("patch", "missing_profile", "profile does not exist");
        return -1;
    }

    LOG_INFOF("running patch script %s on profile %ld", patch, profile);

    lua_State *L = luaL_newstate();
    if (!L) { LOG_FATAL("patch", "lua_alloc_fail", "failed to allocate lua state"); }

    luaL_openlibs(L);

    /* expose context */
    lua_pushstring(L, game);
    lua_setglobal(L, "PATCH_GAME");

    lua_pushinteger(L, profile);
    lua_setglobal(L, "PATCH_PROFILE");

    /* expose patch helpers (implemented elsewhere) */
    lua_register(L, "apply_bps",      LUA_APPLY_BPS);
    lua_register(L, "copy_file",      LUA_COPY_FILE);
    lua_register(L, "vanilla_win",    LUA_VANILLA_WIN);
    lua_register(L, "profile_win",    LUA_PROFILE_WIN);

    if (luaL_dofile(L, patch) != LUA_OK)
    {
        LOG_FATAL("patch", "lua_load_fail", lua_tostring(L, -1));
    }

    lua_getglobal(L, "install");
    if (!lua_isfunction(L, -1))
    {
        LOG_FATAL("patch", "missing_entry", "patch script missing install()");
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        LOG_FATAL("patch", "lua_runtime_error", lua_tostring(L, -1));
    }

    lua_close(L);

    LOG_INFOF("patched profile %ld with %s", profile, patch);
    return 0;
}