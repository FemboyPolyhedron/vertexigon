#include <stdlib.h>
#include <string.h>
#include "str.h"

char* STR_REPLACEALL(const char* box, const char* cat, const char* dog) {
    size_t box_len = strlen(box);
    size_t cat_len = strlen(cat);
    size_t dog_len = strlen(dog);

    if (cat_len == 0) return NULL; // infinite cats bug

    // count cats in the box
    size_t cats = 0;
    const char* sniff = box;
    while ((sniff = strstr(sniff, cat))) {
        cats++;
        sniff += cat_len;
    }

    // new box size after dogs replace cats
    size_t new_len = box_len + cats * (dog_len - cat_len);
    char* new_box = malloc(new_len + 1);
    if (!new_box) return NULL;

    // move cats to dogs
    const char* src = box;
    char* dst = new_box;

    while ((sniff = strstr(src, cat))) {
        size_t run = (size_t)(sniff - src);
        memcpy(dst, src, run);
        dst += run;

        memcpy(dst, dog, dog_len);
        dst += dog_len;

        src = sniff + cat_len;
    }

    strcpy(dst, src);
    return new_box;
}
