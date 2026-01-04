/// @brief helpers methods

#include "obj.h"

#ifdef _WIN32
#include <windows.h>
#include <libloaderapi.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

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