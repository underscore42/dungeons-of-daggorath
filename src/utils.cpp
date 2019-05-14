#include "dod.h"
#include "oslink.h"

extern OS_Link oslink;

Mix_Chunk *Utils::LoadSound(const char *snd)
{
    char fn[256];
    sprintf(fn, "%s%s%s", oslink.soundDir, oslink.pathSep, snd);
    Mix_Chunk* sample = Mix_LoadWAV(fn);
    if (!sample)
        printf("Could not load sound '%s'\n", fn);
    return sample;
}
