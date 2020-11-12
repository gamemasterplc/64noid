#include <string.h>
#include "hash.h"

u32 HashGetPath(const char *string)
{
	//Initialize Hash Properties
    u32 hash = 16777619;
	int i;
	//Calculate Hash
    for (i=0; string[i]; i++) {
		if(string[i] == '\\') {
			//Normalize Slash Direction
			hash ^= '/';
		} else {
			hash ^= string[i];
		}
        hash *= 2166136261U; //Must be Unsigned Integer to Prevent Warning
    }
    return hash;
}

u32 HashGet(const char *string)
{
	//Initialize Hash Properties
    u32 hash = 16777619;
	int i;
	//Calculate Hash
    for (i=0; string[i]; i++) {
		hash ^= string[i];
        hash *= 2166136261U; //Must be Unsigned Integer to Prevent Warning
    }
    return hash;
}