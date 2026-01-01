#pragma once
#include "cJSON/cJSON.h"

long GET_PROFILE(char* game);
cJSON* GET_PROFILE_DATA(char* game, long profile);
int SET_PROFILE(char* game, long profile);