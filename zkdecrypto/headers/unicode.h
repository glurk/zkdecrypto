#ifndef _UNICODE_H_
#define _UNICODE_H_

#include <string.h>

#define wchar unsigned short

#define UNI_FULLBAR 0x2588
#define UNI_HALFBAR 0x258C
#define UNI_LITEBAR 0x2591
#define UNI_HORZBAR 0x203E
#define UNI_VERTBAR 0x2502

int ustrcpy(wchar*,char*);
int ustrlen(wchar*);
int ustrcat(wchar*,wchar*);
int ustrcat(wchar*,wchar);
int ustrcat(wchar*,char*);

#endif
