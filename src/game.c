/// game.c
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discoread] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief add/remove games 
/// utmt exists :broken_heart:

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "cJSON/cJSON.h"
#include "log.h"
#include "patch.h"
#include "obj.h"
#include "profile.h"

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

int NEW_GAME(char* game, char* script)
{
    
}