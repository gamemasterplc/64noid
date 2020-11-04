#include <string.h>
#include "hash.h"

u32 HashGetPath(const char *string)
{
    u32 hash = 16777619;
	int i;
	int length = strlen(string);
    for (i = 0; string[i] != '\0'; i++) {
		if(string[i] == '\\') {
			hash ^= '/';
		} else {
			hash ^= string[i];
		}
        hash *= 2166136261U;
    }
    return hash;
}

u32 HashGet(const char *string)
{
    u32 hash = 16777619;
	int i;
    for (i = 0; string[i] != '\0'; i++) {
		hash ^= string[i];
        hash *= 2166136261U;
    }
    return hash;
}