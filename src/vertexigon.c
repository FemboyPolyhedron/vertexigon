#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "help.h"
#include "profile.h"

typedef struct Flag
{
    char *  f_name;
    char ** f_value;
} Flag;

Flag *SterilzeArgs(int argc, char *argv[])
{
    Flag *flags=calloc(argc-1,sizeof(Flag));

    for (int i = 1; i < argc; i++)
    {
        char name[256];
        char **valu = calloc(1024, sizeof(char*));

        int j_j = 0;
        int j_k = 0;
        int eol = 0;
        int h_v = 0;

        Flag f;
        for(int j = 0; 1; j++)
        {
            if(argv[i][j] == '=')
            {
                name[j] = '\0';
                j_j = j + 1;
                break;
            }
            else if(argv[i][j] == '\0')
            {
                name[j] = '\0';
                f.f_name = strdup(name);
                f.f_value = malloc(2 * sizeof(char *));
                f.f_value[0] = strdup("1");
                f.f_value[1] = NULL;
                flags[i - 1] = f;
                h_v = 1;
                break;
            }
            else
            {
                name[j] = argv[i][j];
            }
        }

        if(h_v == 1) { free(valu); continue; }

        j_k = j_j;

        for(int j = 0; !eol; j++)
        {
            char value[1024];
            int str = 0;

            for(int k = 0; 1; k++)
            {
                if(argv[i][j_k + k] == '&' && str == 0)
                {
                    value[k] = '\0';
                    j_k = j_k + k + 1;
                    break;
                }
                else if(argv[i][j_k + k] == '\0' && str == 0)
                {
                    if(k == 0)
                    {
                        LOG_FATAL("SterilzeArgs", "value_nil_error", "Arguments values are nil when arguments where present");
                        exit(-1);
                    }

                    value[k] = '\0';
                    eol = 1;
                    break;
                }
                else if(argv[i][j_k + k] == '"')
                {
                    value[k] = '"';
                    str = !str;
                }
                else
                {
                    value[k] = argv[i][j_k + k];
                }
            }

            valu[j] = strdup(value);
        }

        f.f_name = strdup(name);
        f.f_value = malloc(sizeof(char*) * 1024);

        for(int i = 0; i < 1024; i++)
        {
            f.f_value[i] = valu[i];
        }

        flags[i - 1] = f;
    }

    return flags;
}

int find_flag_index(Flag *flags, int argc, const char *name)
{
    for (int i = 0; i < argc - 1; i++)
        if (flags[i].f_name && strcmp(flags[i].f_name, name) == 0)
            return i;
    return -1;
}

Flag find_flag(Flag *flags, int argc, const char *name)
{
    static char *no_value[] = { "-1", NULL };

    int i = find_flag_index(flags, argc, name);
    return (i == -1) ? (Flag){ NULL, no_value } : flags[i];
}

#ifdef _WIN32
#include <windows.h>
void wincon(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
    SetConsoleOutputCP(CP_UTF8);
}
#endif

int main(int argc, char *argv[])
{
    #ifdef _WIN32
    wincon();
    #endif
    if (argc > 1)
    {
        Flag* args = SterilzeArgs(argc, argv);
        if (strtol(find_flag(args, argc, "help").f_value[0], NULL, 10) == 1) {
            HelpData help = *GET_HELP("generic", "en-us");
            size_t cap = 128;
            size_t len = 0;
            char *args = malloc(cap);
            args[0] = '\0';

            size_t cap2 = 128;
            size_t len2 = 0;
            char *args2 = malloc(cap2);
            args2[0] = '\0';

            for (int i = 0; i < help.h_ta_len; i++) {
                int need = snprintf(
                    NULL, 0,
                    " %s%s %s%s",
                    help.h_ta[i].a_r ? "<" : "[",
                    help.h_ta[i].a_t,
                    help.h_ta[i].a_n,
                    help.h_ta[i].a_r ? ">" : "]"
                );

                if (len2 + need + 1 > cap2) {
                    cap2 = (len2 + need + 1) * 2;
                    args2 = realloc(args2, cap2);
                    if (!args2) return 1; // handle OOM
                }

                len2 += snprintf(
                    args2 + len2,
                    cap2 - len2,
                    " %s%s %s%s",
                    help.h_ta[i].a_r ? "<" : "[",
                    help.h_ta[i].a_t,
                    help.h_ta[i].a_n,
                    help.h_ta[i].a_r ? ">" : "]"
                );
            }

            for (int i = 0; i < help.h_ta_len; i++) {

                int need = snprintf(
                    NULL, 0,
                    "\n\n%s%s %s%s\n    %s",
                    help.h_ta[i].a_r ? "<" : "[",
                    help.h_ta[i].a_t,
                    help.h_ta[i].a_n,
                    help.h_ta[i].a_r ? ">" : "]",
                    help.h_ta[i].a_d
                );

                if (len + need + 1 > cap) {
                    cap = (len + need + 1) * 2;
                    args = realloc(args, cap);
                    if (!args) return 1; // handle OOM
                }

                len += snprintf(
                    args + len,
                    cap - len,
                    "\n\n%s%s %s%s\n    %s",
                    help.h_ta[i].a_r ? "<" : "[",
                    help.h_ta[i].a_t,
                    help.h_ta[i].a_n,
                    help.h_ta[i].a_r ? ">" : "]",
                    help.h_ta[i].a_d
                );
            }

            LOG_INFOF("%s%s\n\n%s%s", help.h_tn, args2, help.h_td, args);
        }
        else if(find_flag_index(args, argc, "profile") == 0 && strtol(find_flag(args, argc, "profile").f_value[0], NULL, 10) == 1)
        {   
            if(find_flag_index(args, argc, "set") == 1 && strtol(find_flag(args, argc, "set").f_value[0], NULL, 10) == 1)
            {
                LOG_INFOF("setting profile to %ld", strtol(find_flag(args, argc, "pf").f_value[0], NULL, 10));
                SET_PROFILE(find_flag(args, argc, "game").f_value[0], strtol(find_flag(args, argc, "pf").f_value[0], NULL, 10));
            }
            else if(find_flag_index(args, argc, "get") == 1 && strtol(find_flag(args, argc, "get").f_value[0], NULL, 10) == 1)
            {
                LOG_INFOF("%ld", GET_PROFILE(find_flag(args, argc, "game").f_value[0]));
            }
            else
            {
                LOG_INFO("profile <set|get|list> [int pf]");
            }
        }
        else
        {
            LOG_FATAL("parse", "invalid", "Invalid Command");
        }
    }
    else
    {
        HelpData help = *GET_HELP("generic", "en-us");
        size_t cap = 128;
        size_t len = 0;
        char *args = malloc(cap);
        args[0] = '\0';

        size_t cap2 = 128;
        size_t len2 = 0;
        char *args2 = malloc(cap2);
        args2[0] = '\0';

        for (int i = 0; i < help.h_ta_len; i++) {
            int need = snprintf(
                NULL, 0,
                " %s%s %s%s",
                help.h_ta[i].a_r ? "<" : "[",
                help.h_ta[i].a_t,
                help.h_ta[i].a_n,
                help.h_ta[i].a_r ? ">" : "]"
            );

            if (len2 + need + 1 > cap2) {
                cap2 = (len2 + need + 1) * 2;
                args2 = realloc(args2, cap2);
                if (!args2) return 1; // handle OOM
            }

            len2 += snprintf(
                args2 + len2,
                cap2 - len2,
                " %s%s %s%s",
                help.h_ta[i].a_r ? "<" : "[",
                help.h_ta[i].a_t,
                help.h_ta[i].a_n,
                help.h_ta[i].a_r ? ">" : "]"
            );
        }

        for (int i = 0; i < help.h_ta_len; i++) {

            int need = snprintf(
                NULL, 0,
                "\n\n%s%s %s%s\n    %s",
                help.h_ta[i].a_r ? "<" : "[",
                help.h_ta[i].a_t,
                help.h_ta[i].a_n,
                help.h_ta[i].a_r ? ">" : "]",
                help.h_ta[i].a_d
            );

            if (len + need + 1 > cap) {
                cap = (len + need + 1) * 2;
                args = realloc(args, cap);
                if (!args) return 1; // handle OOM
            }

            len += snprintf(
                args + len,
                cap - len,
                "\n\n%s%s %s%s\n    %s",
                help.h_ta[i].a_r ? "<" : "[",
                help.h_ta[i].a_t,
                help.h_ta[i].a_n,
                help.h_ta[i].a_r ? ">" : "]",
                help.h_ta[i].a_d
            );
        }

        LOG_INFOF("%s%s\n\n%s%s", help.h_tn, args2, help.h_td, args);
    }
    return 0;
}