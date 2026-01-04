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
#include <regex.h>
#include <dirent.h>
#include "cJSON/cJSON.h"
#include "inih/ini.h"
#include "log.h"
#include "obj.h"
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#ifdef _WIN32
#include <windows.h>
#include <libloaderapi.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

#include <string.h>

/// @brief we hate inih
int hndl_ini_alfa(void *user, const char *section, const char *name, const char *value)
{
    char **out = user;
    if (strcmp(name, "profile") == 0) {
        *out = strdup(value);
        return 0;
    }
    return 1;
}

/// @brief returns the current select profile of that game
/// @param game that game
/// @return select profile as a long
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

/// @brief get the profile data as a cjson
/// @param game the game
/// @param profile profile id
/// @return cjson data
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
        char msg[24] = "";
        snprintf(msg, sizeof(msg), "could not find profile%ld", profile);
        LOG_ERR("profile", "file_not_found", msg);
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

/// @brief returns a pointer of profiles, with the size of the pointer prefixed
/// @param game (you lost) the game
/// @return all valid profile ids of that game
long* GET_PROFILES(char* game)
{
    char* app_dir = GET_APP_DIR();
    char games_path[MAX_PATH];
    char profile_path[MAX_PATH];
    char games_meta_path[MAX_PATH];

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

    snprintf(profile_path, MAX_PATH, "%s%s/profile", games_path, cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(pro, "data")));

    regex_t reg;
    regcomp(&reg, "^profile[0-9]+$", 0);

    DIR *dir = opendir(profile_path);
    if (!dir) return NULL;

    struct dirent *ent;
    long *profiles = NULL;
    size_t mrrp = 0;

    while ((ent = readdir(dir))) {
        if (regexec(&reg, ent->d_name, 1, NULL, 0) != 0) { continue; }

        long id = strtol(ent->d_name + 7, NULL, 10);

        char *tmp = realloc(profiles, (mrrp + 1) * sizeof *profiles);
        if (!tmp) break;
        profiles = tmp;
        
        profiles[mrrp + 1] = id;
        mrrp++;
    }

    profiles[0] = (long)mrrp;

    closedir(dir);
    return profiles;
}

/// @brief check if that profile id exists for that game
/// @param game that game
/// @param profile profile id
/// @return Boolean
int PROFILE_EXISTS(char* game, long profile)
{
    long* profiles = GET_PROFILES(game);
    for(int i = 0; i < profiles[0]; i++)
    {
        if(profiles[i + 1] == profile)
        {
            return 1;
        }
    }
    return 0;
}

/// @brief sets the current profile of that game to that id
/// @param game that game
/// @param profile profile id
/// @return 0 if success and error code if fail
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

    snprintf(game_root, MAX_PATH, "%s/game/%s", app_dir, game);
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

    cJSON *profile_json = GET_PROFILE_DATA(game, profile);
    if (!profile_json) {
        LOG_FATAL("profile", "profile_load_fail", "could not load profile json");
    }

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

    char profile_base[MAX_PATH];
    snprintf(profile_base, MAX_PATH, "%s/profile/profile%ld", game_root, profile);
    GET_DIR(profile_base);

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

        char *rel = malloc(path_len + 1);
        memcpy(rel, raw, path_len);
        rel[path_len] = '\0';

        free(raw);

        char src_data[MAX_PATH];
        snprintf(
            src_data,
            MAX_PATH,
            "%s/%s",
            cJSON_GetStringValue(pfroot),
            rel
        );

        free(rel);


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

// some shit

static cJSON *PROFILE_BUILD = NULL;
static char   PROFILE_GAME[64];
static long   PROFILE_ID = -1;

static int LUA_NEW_PROFOLDER(lua_State *L)
{
    const char *scope   = luaL_checkstring(L, 1);
    const char *chapter = luaL_checkstring(L, 2);
    const char *name    = luaL_checkstring(L, 3);

    LOG_INFOF("lua:new_profolder %s %s %s", scope, chapter, name);

    return 0;
}

static int LUA_NEW_PROFILE_MAIN(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    long id          = luaL_checkinteger(L, 2);

    PROFILE_ID = id;

    PROFILE_BUILD = cJSON_CreateObject();
    if (!PROFILE_BUILD) { luaL_error(L, "cjson alloc failed"); }

    char idbuf[64];
    snprintf(idbuf, sizeof(idbuf), "%s:%ld", PROFILE_GAME, id);

    cJSON_AddStringToObject(PROFILE_BUILD, "id", idbuf);
    cJSON_AddStringToObject(PROFILE_BUILD, "name", name);
    cJSON_AddArrayToObject(PROFILE_BUILD, "packages");

    cJSON *game = cJSON_CreateObject();
    cJSON_AddItemToObject(PROFILE_BUILD, PROFILE_GAME, game);

    return 0;
}

static int LUA_NEW_PROFILE_DATA(lua_State *L)
{
    const char *pfname  = luaL_checkstring(L, 1);
    const char *chapter = luaL_checkstring(L, 2);

    if (!PROFILE_BUILD) { luaL_error(L, "profile not initialized"); }

    cJSON *game = cJSON_GetObjectItemCaseSensitive(PROFILE_BUILD, PROFILE_GAME);
    if (!game) { luaL_error(L, "game object missing"); }

    char rel[128];
    snprintf(rel, sizeof(rel), "./%s", pfname);

    cJSON_AddStringToObject(game, chapter, rel);
    return 0;
}

static int RUN_PROFILE_LUA(char *game, long id)
{
    lua_State *L = luaL_newstate();
    if (!L) { LOG_FATAL("lua", "alloc_fail", "lua state alloc failed"); }

    luaL_openlibs(L);

    memset(PROFILE_GAME, 0, sizeof(PROFILE_GAME));
    strncpy(PROFILE_GAME, game, sizeof(PROFILE_GAME) - 1);

    PROFILE_BUILD = NULL;
    PROFILE_ID    = id;

    lua_register(L, "new_profolder",    LUA_NEW_PROFOLDER);
    lua_register(L, "new_profile_main", LUA_NEW_PROFILE_MAIN);
    lua_register(L, "new_profile_data", LUA_NEW_PROFILE_DATA);

    if (luaL_dofile(L, "profile.lua") != LUA_OK)
        LOG_FATAL("lua", "load_fail", lua_tostring(L, -1));

    lua_getglobal(L, "_profile_create");
    if (!lua_isfunction(L, -1))
        LOG_FATAL("lua", "missing_entry", "_profile_create not found");

    lua_pushstring(L, "profile");
    lua_pushinteger(L, id);

    if (lua_pcall(L, 2, 0, 0) != LUA_OK)
        LOG_FATAL("lua", "runtime_error", lua_tostring(L, -1));

    lua_close(L);
    return 0;
}

/// @brief makes a new profile
/// @param game game to make profile on
/// @param id the id of the new profile
/// @return error code
int NEW_PROFILE(char* game, long id)
{
    if(PROFILE_EXISTS(game, id)) { LOG_FATAL("profile", "already_exists", "failed to create profile, profile already exists"); return -1; }

    LOG_INFOF("creating profile for %s with id %s:%ld", game, game, id);

    RUN_PROFILE_LUA(game, id);

    if (!PROFILE_BUILD) { LOG_FATAL("profile", "build_fail", "profile json not created"); }

    char *out = cJSON_Print(PROFILE_BUILD);
    if (!out) { LOG_FATAL("profile", "json_fail", "cjson print failed"); }

    char *app_dir = GET_APP_DIR();
    char path[MAX_PATH];

    snprintf(path, MAX_PATH, "%s/game/%s/profile/profile%ld", app_dir, game, id);

    FILE *f = fopen(path, "w");
    if (!f) { LOG_FATAL("profile", "file_open_fail", path); }

    fputs(out, f);
    fclose(f);

    free(out);
    cJSON_Delete(PROFILE_BUILD);
    free(app_dir);

    PROFILE_BUILD = NULL;
    PROFILE_ID    = -1;

    LOG_INFOF("profile %s:%ld created", game, id);
    return 0;
}
