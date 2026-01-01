#include "log.h"
#include "str.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void LOG_CTRL(const char *action, const char *value) {
    printf("\x1b[1m\x1b[44;97m ^%c \x1b[0m \x1b[96mCTRL\x1b[0m: action [%s] value \"%s\"\n",
           action[0], action, value);
}

void LOG_ERR(const char *action, const char *value, const char *msg) {
    char* box = STR_REPLACEALL(msg, "\n", "\n\x1b[41;97m    \x1b[0m       ");
    printf("\x1b[1m\x1b[41;97m :< \x1b[0m \x1b[91mFAIL\x1b[0m: action [%s] value \"%s\"\n",
           action, value);
    printf("\x1b[41m    \x1b[0m       %s\n", box);
    free(box);
}

void LOG_WARN(const char *value) {
    char* box = STR_REPLACEALL(value, "\n", "\n\x1b[33;97m    \x1b[0m       ");
    printf("\x1b[1m\x1b[33;97m ?? \x1b[0m \x1b[93mWARN\x1b[0m: %s\n", box);
    free(box);
}

void LOG_INFO(const char *value) {
    char* box = STR_REPLACEALL(value, "\n", "\n\x1b[42;97m    \x1b[0m       ");
    printf("\x1b[1m\x1b[42;97m ?? \x1b[0m \x1b[92mINFO\x1b[0m: %s\n", box);
    free(box);
}

void LOG_INFOF(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (len < 0) {
        va_end(args);
        return;
    }

    char *raw = malloc(len + 1);
    if (!raw) {
        va_end(args);
        return;
    }

    vsnprintf(raw, len + 1, format, args);
    va_end(args);

    char *box = STR_REPLACEALL(
        raw,
        "\n",
        "\n\x1b[42;97m    \x1b[0m       "
    );

    printf(
        "\x1b[1m\x1b[42;97m ?? \x1b[0m \x1b[92mINFO\x1b[0m: %s\n",
        box ? box : raw
    );

    free(box);
    free(raw);
}

void LOG_YAY(const char *value) {
    char* box = STR_REPLACEALL(value, "\n", "\n\x1b[44m    \x1b[0m       ");
    printf("\x1b[1m\x1b[44;97m :3 \x1b[0m \x1b[95mOKAY\x1b[0m: %s\n", box);
    free(box);
}

void LOG(const char *who, const char *value) {
    char* box = STR_REPLACEALL(value, "\n", "\n\x1b[44m    \x1b[0m       ");
    printf("\x1b[1m\x1b[44;97m  - \x1b[0m \x1b[90m%s\x1b[0m: %s\n", who, box);
    free(box);
}


void LOG_FATAL(const char *action, const char *value, const char *msg) {
    char* box = STR_REPLACEALL(msg, "\n", "\n\x1b[41m    \x1b[0m       %s");
    printf(
        "\x1b[1m\x1b[41;97m !? \x1b[0m \x1b[91mDEAD\x1b[0m: action [%s] value \"%s\"\n"
        "\x1b[41;97m    \x1b[0m       %s\n"
        "\x1b[41;97m    \x1b[0m       exited -1 with [object]\n"
        "\x1b[41;97m    \x1b[0m       (object): {\n"
        "\x1b[41;97m    \x1b[0m           \"body\": {\n"
        "\x1b[41;97m    \x1b[0m               \"action\": \"%s\",\n"
        "\x1b[41;97m    \x1b[0m               \"value\": \"%s\"\n"
        "\x1b[41;97m    \x1b[0m           }\n"
        "\x1b[41;97m    \x1b[0m       }\n",
        action, value, box, action, value
    );
}