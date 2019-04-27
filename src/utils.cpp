#include <string>

#include "dod.h"
#include "oslink.h"

extern OS_Link oslink;

Mix_Chunk *Utils::LoadSound(const char *snd)
{
    const std::string fn = oslink.soundDir + snd;
    Mix_Chunk* sample = Mix_LoadWAV(fn.c_str());
    if (!sample)
        printf("Could not load sound '%s'\n", fn.c_str());
    return sample;
}
