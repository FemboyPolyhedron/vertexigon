/// profile.c
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discord] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief handle profiles
/// ever heard of deltarune modding tool? no?

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "cJSON/cJSON.h"
#include "inih/ini.h"
#include "log.h"

#ifdef _WIN32
#include <windows.h>
#include <libloaderapi.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

#include <string.h>

int hndl_ini_alfa(void *user, const char *section, const char *name, const char *value)
{
    char **out = user;
    if (strcmp(name, "profile") == 0) {
        *out = strdup(value);
        return 0;
    }
    return 1;
}


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

long GET_PROFILE(char* game)
{   
    char* app_dir = GET_APP_DIR();
    char games_path[MAX_PATH];
    char games_meta_path[MAX_PATH];
    char ini_path[MAX_PATH];
    long profile;
    char* profile_str = NULL;

    snprintf(games_meta_path, MAX_PATH, "%s/game/games.json", app_dir);

    FILE *file = fopen(games_meta_path, "rb");

    if (!file) {
        LOG_ERR("profile", "file_not_found", "could not find critical meta file /game/games.json");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(file_size + 1);
    
    if (fread(buf, 1, file_size, file) != (size_t)file_size) {
        fclose(file);
        free(buf);
        free(app_dir);
        return -1;
    }
    buf[file_size] = '\0';
    fclose(file);

    cJSON *games = cJSON_Parse(buf);
    if (!games) { free(buf); free(app_dir); LOG_ERR("profile", "cjson_parse_err", "error while parsing /game/games.json"); return -1; }

    cJSON *pro = cJSON_GetObjectItemCaseSensitive(games, game);

    if (!pro) {
        char *msg = malloc(snprintf(NULL, 0, "could not find game %s", game) + 1);
        snprintf(msg, snprintf(NULL, 0, "could not find game %s", game) + 1, "could not find game %s", game);
        LOG_ERR("profile", "missing_entry", msg);
        cJSON_Delete(games);
        free(buf);
        free(app_dir);
        free(msg);
        return -1;
    }

    if (!cJSON_IsString(cJSON_GetObjectItemCaseSensitive(pro, "id")) || strcmp(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(pro, "id")), game) != 0)
    {
        char *msg = malloc(snprintf(NULL, 0, "could not find game %s", game) + 1);
        snprintf(msg, snprintf(NULL, 0, "could not find game %s", game) + 1, "could not find game %s", game);
        LOG_ERR("profile", "invalid_data", msg);
        cJSON_Delete(games);
        free(buf);
        free(app_dir);
        free(msg);
        return -1;
    }

    strcpy(games_path, games_meta_path);

    GET_DIR(games_path);

    snprintf(ini_path, MAX_PATH, "%s%s/%s.ini", games_path, game, game);

    ini_parse(ini_path, hndl_ini_alfa, &profile_str);
    
    if (!profile_str) { cJSON_Delete(games); free(buf); free(app_dir); return 1; }
    profile = strtol(profile_str, NULL, 10);

    free(buf);
    free(app_dir);
    free(profile_str);
    return profile;
}

cJSON* GET_PROFILE_DATA(char* game, long profile)
{   
    char* app_dir = GET_APP_DIR();
    char games_path[MAX_PATH];
    char profile_path[MAX_PATH];
    char games_meta_path[MAX_PATH];
    char ini_path[MAX_PATH];
    cJSON* data;

    snprintf(games_meta_path, MAX_PATH, "%sgame/games.json", app_dir);

    FILE *file = fopen(games_meta_path, "rb");

    if (!file) {
        LOG_ERR("profile", "file_not_found", "could not find critical meta file /game/games.json");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(file_size + 1);
    
    if (fread(buf, 1, file_size, file) != (size_t)file_size) {
        fclose(file);
        free(buf);
        free(app_dir);
        return NULL;
    }
    buf[file_size] = '\0';
    fclose(file);

    cJSON *games = cJSON_Parse(buf);
    if (!games) { free(buf); free(app_dir); LOG_ERR("profile", "cjson_parse_err", "error while parsing /game/games.json"); return NULL; }

    cJSON *pro = cJSON_GetObjectItemCaseSensitive(games, game);

    if (!pro) {
        char *msg = malloc(snprintf(NULL, 0, "could not find game %s", game) + 1);
        snprintf(msg, snprintf(NULL, 0, "could not find game %s", game) + 1, "could not find game %s", game);
        LOG_ERR("profile", "missing_entry", msg);
        cJSON_Delete(games);
        free(buf);
        free(app_dir);
        free(msg);
        return NULL;
    }

    if (!cJSON_IsString(cJSON_GetObjectItemCaseSensitive(pro, "id")) || strcmp(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(pro, "id")), game) != 0)
    {
        char *msg = malloc(snprintf(NULL, 0, "could not find game %s", game) + 1);
        snprintf(msg, snprintf(NULL, 0, "could not find game %s", game) + 1, "could not find game %s", game);
        LOG_ERR("profile", "invalid_data", msg);
        cJSON_Delete(games);
        free(buf);
        free(app_dir);
        free(msg);
        return NULL;
    }

    strcpy(games_path, games_meta_path);
    GET_DIR(games_path);

    snprintf(profile_path, MAX_PATH, "%s%s/profile/profile%ld", games_path, cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(pro, "data")), profile);
    
    FILE *prof_file = fopen(profile_path, "rb");
    if (!prof_file) {
        cJSON_Delete(games);
        free(buf);
        free(app_dir);
        LOG_ERR("profile", "file_not_found", "could not find critical meta file /game/games.json");
        return NULL;
    }

    fseek(prof_file, 0, SEEK_END);
    long prof_file_size = ftell(prof_file);
    fseek(prof_file, 0, SEEK_SET);
    char *buf2 = malloc(prof_file_size + 1);

    if (fread(buf2, 1, prof_file_size, prof_file) != (size_t)prof_file_size) {
        fclose(prof_file);
        cJSON_Delete(games);
        free(buf);
        free(buf2);
        free(app_dir);
        return NULL;
    }
    buf2[prof_file_size] = '\0';
    fclose(prof_file);

    data = cJSON_Parse(buf2);

    cJSON_Delete(games);
    free(buf);
    free(buf2);
    free(app_dir);
    return data;
}

int SET_PROFILE(char* game, long profile)
{
    char* app_dir = GET_APP_DIR();
    char games_meta_path[MAX_PATH];
    char game_root[MAX_PATH];
    char ini_path[MAX_PATH];
    char tmp_path[MAX_PATH];
    char dat_path[MAX_PATH];

    FILE *file;
    FILE *in;
    FILE *out;

    long file_size;
    char *buf;

    cJSON *games;
    cJSON *pro;

    char line[512];
    int found = 0;

    /* ---- load games.json ---- */

    snprintf(games_meta_path, MAX_PATH, "%s/game/games.json", app_dir);

    file = fopen(games_meta_path, "rb");
    if (!file) {
        LOG_FATAL("profile", "file_not_found", "could not find game/games.json");
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buf = malloc(file_size + 1);
    fread(buf, 1, file_size, file);
    buf[file_size] = '\0';
    fclose(file);

    games = cJSON_Parse(buf);
    if (!games) {
        LOG_FATAL("profile", "cjson_parse_err", "error while parsing games.json");
    }

    pro = cJSON_GetObjectItemCaseSensitive(games, game);
    if (!pro) {
        LOG_FATAL("profile", "missing_entry", "could not find game");
    }

    /* ---- game root ---- */

    snprintf(game_root, MAX_PATH, "%s/game/%s", app_dir, game);

    /* ---- update ini ---- */

    snprintf(ini_path, MAX_PATH, "%s/%s.ini", game_root, game);
    snprintf(tmp_path, MAX_PATH, "%s/%s.ini.tmp", game_root, game);

    in  = fopen(ini_path, "r");
    out = fopen(tmp_path, "w");

    if (!out) {
        LOG_FATAL("profile", "ini_open_fail", ini_path);
    }

    if (in) {
        while (fgets(line, sizeof(line), in)) {
            if (strncmp(line, "profile=", 8) == 0) {
                fprintf(out, "profile=%ld\n", profile);
                found = 1;
            } else {
                fputs(line, out);
            }
        }
        fclose(in);
    }

    if (!found)
        fprintf(out, "profile=%ld\n", profile);

    fclose(out);
    remove(ini_path);
    rename(tmp_path, ini_path);

    /* ---- load profile JSON ---- */

    cJSON *profile_json = GET_PROFILE_DATA(game, profile);
    if (!profile_json) {
        LOG_FATAL("profile", "profile_load_fail", "could not load profile json");
    }

    /* ---- load dat json ---- */

    cJSON *data_item = cJSON_GetObjectItemCaseSensitive(pro, "data");
    if (!cJSON_IsString(data_item)) {
        LOG_FATAL("profile", "invalid_game", "missing data field in games.json");
    }

    snprintf(
        dat_path,
        MAX_PATH,
        "%s/game/%s/dat",
        app_dir,
        cJSON_GetStringValue(data_item)
    );

    FILE *df = fopen(dat_path, "rb");
    if (!df) {
        LOG_FATAL("profile", "file_not_found", dat_path);
    }

    fseek(df, 0, SEEK_END);
    long dsz = ftell(df);
    fseek(df, 0, SEEK_SET);

    char *dbuf = malloc(dsz + 1);
    fread(dbuf, 1, dsz, df);
    dbuf[dsz] = '\0';
    fclose(df);

    cJSON *dat_json = cJSON_Parse(dbuf);
    if (!dat_json) {
        LOG_FATAL("profile", "dat_parse_fail", "error parsing dat json");
    }

    free(dbuf);

    cJSON *pfpath   = cJSON_GetObjectItemCaseSensitive(dat_json, "pfpath");
    cJSON *destpath = cJSON_GetObjectItemCaseSensitive(dat_json, "destpath");
    cJSON *game_map = cJSON_GetObjectItemCaseSensitive(profile_json, game);

    if (!pfpath || !destpath || !game_map) {
        LOG_FATAL("profile", "invalid_profile", "pfpath/destpath/game map missing");
    }

    /* ---- profile base dir ---- */

    char profile_base[MAX_PATH];
    snprintf(profile_base, MAX_PATH, "%s/profile/profile%ld", game_root, profile);
    GET_DIR(profile_base);

    /* ---- apply per chapter ---- */

    cJSON *it;
    cJSON_ArrayForEach(it, game_map)
    {
        if (!cJSON_IsString(it))
            continue;

        const char *key = it->string;

        cJSON *pfroot  = cJSON_GetObjectItemCaseSensitive(pfpath, key);
        cJSON *dstroot = cJSON_GetObjectItemCaseSensitive(destpath, key);

        if (!cJSON_IsString(pfroot) || !cJSON_IsString(dstroot))
            continue;

        /* ---- open plaintext profileX_chY ---- */

        char prof_ch_file[MAX_PATH];
        snprintf(
            prof_ch_file,
            MAX_PATH,
            "%s/%s",
            profile_base,
            cJSON_GetStringValue(it)
        );

        FILE *pf = fopen(prof_ch_file, "r");
        if (!pf) {
            LOG_ERR("profile", "missing_profile_ch", prof_ch_file);
            continue;
        }

        fseek(pf, 0, SEEK_END);
        long psz = ftell(pf);
        fseek(pf, 0, SEEK_SET);

        if (psz <= 0) {
            fclose(pf);
            LOG_ERR("profile", "empty_profile_ch", prof_ch_file);
            continue;
        }

        uint8_t *raw = malloc(psz);
        fread(raw, 1, psz, pf);
        fclose(pf);

        /* find first 0x00 */
        size_t path_len = psz;
        for (size_t i = 0; i < psz; i++) {
            if (raw[i] == 0x00) {
                path_len = i;
                break;
            }
        }

        if (path_len == 0) {
            free(raw);
            LOG_ERR("profile", "empty_profile_ch", prof_ch_file);
            continue;
        }

        /* extract only arg0 (base .win path) */
        char *rel = malloc(path_len + 1);
        memcpy(rel, raw, path_len);
        rel[path_len] = '\0';

        free(raw);


        /* ---- build source .win path ---- */

        char src_data[MAX_PATH];
        snprintf(
            src_data,
            MAX_PATH,
            "%s/%s",
            cJSON_GetStringValue(pfroot),
            rel
        );

        free(rel);

        /* ---- delete any existing *.win in destination ---- */

        WIN32_FIND_DATAA fd;
        char find_pat[MAX_PATH];

        snprintf(
            find_pat,
            MAX_PATH,
            "%s\\*.win",
            cJSON_GetStringValue(dstroot)
        );

        HANDLE h = FindFirstFileA(find_pat, &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
            {
                char del_path[MAX_PATH];
                snprintf(
                    del_path,
                    MAX_PATH,
                    "%s/%s",
                    cJSON_GetStringValue(dstroot),
                    fd.cFileName
                );
                remove(del_path);
            }
            while (FindNextFileA(h, &fd));

            FindClose(h);
        }

        /* ---- destination filename = source filename ---- */

        const char *fname = strrchr(src_data, '/');
        if (!fname)
            fname = strrchr(src_data, '\\');
        fname = fname ? fname + 1 : src_data;

        char dst_data[MAX_PATH];
        snprintf(
            dst_data,
            MAX_PATH,
            "%s/%s",
            cJSON_GetStringValue(dstroot),
            fname
        );

        FILE *inw  = fopen(src_data, "rb");
        FILE *outw = fopen(dst_data, "wb");

        if (!inw || !outw) {
            LOG_ERR("profile", "copy_fail", src_data);
            if (inw)  fclose(inw);
            if (outw) fclose(outw);
            continue;
        }

        char cbuf[4096];
        size_t n;
        while ((n = fread(cbuf, 1, sizeof(cbuf), inw)) > 0)
            fwrite(cbuf, 1, n, outw);

        fclose(inw);
        fclose(outw);
    }

    cJSON_Delete(dat_json);
    cJSON_Delete(profile_json);
    cJSON_Delete(games);
    free(buf);
    free(app_dir);

    LOG_INFOF("profile set to %ld", profile);
    return 0;
}
