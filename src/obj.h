#include <stdint.h>

#define td typedef

td void* nul;

#define us unsigned
td us char BYTE;
td us short ASM_WORD;
td us long ASM_DWORD;
td us long long ASM_QWORD;
#undef us 

#define sn signed
td sn long Int32;
td sn long long Int64;
#undef sn

#define st struct

#undef st

#undef td

char* GET_APP_DIR();
void GET_DIR(char *s);