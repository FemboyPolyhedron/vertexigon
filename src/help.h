#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

typedef signed int Boolean;

typedef struct Argument
{
    char * a_n;
    char * a_t;
    char * a_d;
    Boolean a_r;
} Argument;

typedef struct HelpData
{
    char      * h_tn;
    char      * h_td;
    Argument  * h_ta;
    size_t      h_ta_len;
} HelpData;

HelpData *GET_HELP(const char* tool, const char* lang);