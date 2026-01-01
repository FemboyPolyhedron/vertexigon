#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "help.h"
#include "cJSON/cJSON.h"

#if platform == windows
    #include <windows.h>
    #include <libloaderapi.h> 
#else
    #include <unistd.h>
    #include <limits.h>
#endif

HelpData *GET_HELP(const char* tool, const char* lang)
{
    #if platform == windows
        char path[MAX_PATH];
        char help_path[MAX_PATH];
        HMODULE hModule = GetModuleHandle(NULL);
        GetModuleFileName(hModule, path, MAX_PATH);
        char *last_sep = strrchr(path, '\\');
        if (last_sep) {
            *last_sep = '\0';
        }
        snprintf(help_path, MAX_PATH, "%s/../lang/%s/tool/%s.json", path, lang, tool);
    #else
        char path[PATH_MAX];
        char help_path[PATH_MAX];
        readlink("/proc/self/exe", path, sizeof(path) - 1);
        snprintf(help_path, MAX_PATH, "%s/../lang/%s/tool/%s.json", path, lang, tool);
    #endif

    FILE *file = fopen(help_path, "r");

    if (!file) {
        char *msg = malloc(snprintf(NULL, 0, "could not get help for %s in %s", tool, help_path) + 1);
        snprintf(msg, snprintf(NULL, 0, "could not get help for %s in %s", tool, help_path) + 1, "could not get help for %s in %s", tool, help_path);
        LOG_ERR("lang:40", "file_not_found", msg);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(file_size + 1);
    
    fread(buf, 1, file_size, file);
    buf[file_size] = '\0';
    fclose(file);

    cJSON *rt = cJSON_Parse(buf);
    free(buf);

    if (!rt) {
        char *msg = malloc(snprintf(NULL, 0, "invalid language data for %s", tool) + 1);
        snprintf(msg, (snprintf(NULL, 0, "invalid language data for %s", tool) + 1), "invalid language data for %s", tool);
        LOG_ERR("lang:60", "json_invalid", msg);
        return NULL;
    }

    cJSON *tn = cJSON_GetObjectItemCaseSensitive(rt, "h_tn");
    cJSON *td = cJSON_GetObjectItemCaseSensitive(rt, "h_td");
    cJSON *ta = cJSON_GetObjectItemCaseSensitive(rt, "h_ta");

    HelpData *help_data = malloc(sizeof(HelpData));

    help_data->h_tn = strdup(tn ? cJSON_GetStringValue(tn) : NULL);
    help_data->h_td = strdup(td ? cJSON_GetStringValue(td) : NULL);

    if (!cJSON_IsString(tn) || !cJSON_IsString(td) || !cJSON_IsArray(ta)) {
        char *msg = malloc(snprintf(NULL, 0, "invalid language data for %s", tool) + 1);
            snprintf(msg, (snprintf(NULL, 0, "invalid language data for %s", tool) + 1), "invalid language data for %s", tool);
            LOG_ERR("lang:76", "json_invalid", msg);
        return NULL;
    }

    size_t acount = cJSON_GetArraySize(ta);
    help_data->h_ta_len = acount;
    help_data->h_ta = calloc(acount, sizeof(Argument));

    for (size_t i = 0; i < acount; i++) {
        cJSON *arg = cJSON_GetArrayItem(ta, i);
        if (!cJSON_IsObject(arg)) {
            char *msg = malloc(snprintf(NULL, 0, "invalid language data for %s", tool) + 1);
            snprintf(msg, (snprintf(NULL, 0, "invalid language data for %s", tool) + 1), "invalid language data for %s", tool);
            LOG_ERR("lang:89", "json_invalid", msg);
            free(help_data->h_ta);
            free(help_data->h_tn);
            free(help_data->h_td);
            free(help_data);
            return NULL;
        }

        cJSON *an = cJSON_GetObjectItemCaseSensitive(arg, "a_n");
        cJSON *at = cJSON_GetObjectItemCaseSensitive(arg, "a_t");
        cJSON *ar = cJSON_GetObjectItemCaseSensitive(arg, "a_r");
        cJSON *ad = cJSON_GetObjectItemCaseSensitive(arg, "a_d");

        if (!cJSON_IsString(an) || !cJSON_IsString(at) || !cJSON_IsString(ad) || !cJSON_IsBool(ar)) {
            char *msg = malloc(snprintf(NULL, 0, "invalid language data for %s", tool) + 1);
            snprintf(msg, (snprintf(NULL, 0, "invalid language data for %s", tool) + 1), "invalid language data for %s", tool);
            LOG_ERR("lang:105", "json_invalid", msg);
            for (size_t j = 0; j < help_data->h_ta_len; j++) {
                free(help_data->h_ta[j].a_n);
                free(help_data->h_ta[j].a_t);
                free(help_data->h_ta[j].a_d);
            }
            free(help_data->h_ta);
            free(help_data->h_tn);
            free(help_data->h_td);
            free(help_data);
            return NULL;
        }

        help_data->h_ta[i].a_n = strdup(an->valuestring);
        help_data->h_ta[i].a_t = strdup(at->valuestring);
        help_data->h_ta[i].a_d = strdup(ad->valuestring);
        help_data->h_ta[i].a_r = cJSON_IsTrue(ar);
    }

    cJSON_Delete(rt);
    return help_data;
}