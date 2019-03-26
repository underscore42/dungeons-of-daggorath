/****************************************
Daggorath PC-Port Version 0.2.1
Richard Hunerlach
November 13, 2002

The copyright for Dungeons of Daggorath
is held by Douglas J. Morgan.
(c) 1982, DynaMicro
*****************************************/

// Dungeons of Daggorath
// PC-Port
// Filename: dod.cpp
//
// All primary objects are declared in global space
// to minimize dereferencing of pointers.  The main
// function simply calls init on the oslink object
// which contains all the SDL setup code.
//
// Every other file that uses these primary objects
// needs to include the objects header and declare
// an external reference to the object, which will
// resolve to the declarations below.  This can
// probably be simplified with some common mechanism.

#include "dod.h"
#include "dodgame.h"
#include "player.h"
#include "object.h"
#include "creature.h"
#include "dungeon.h"
#include "sched.h"
#include "viewer.h"
#include "oslink.h"
#include "parser.h"

dodGame     game;
Coordinate  crd;
RNG         rng;
Player      player;
Object      object;
Creature    creature;
Dungeon     dungeon;
Scheduler   scheduler;
Viewer      viewer;
OS_Link     oslink;
Parser      parser;

// Could include some command line arguments for
// various purposes (like configurations) if desired later.
//
// However, I think a config file would be nicer
//

void printvls();


int main(int argc, char * argv[])
{
    printf( "Starting DoD.\n" );

    oslink.init();
    return EXIT_SUCCESS;
}


// Prints a function call such as:
// Utils::LoadFromDecDigit(SHIE_VLA, "06AC86C080BA7AA880A486AC86");
void printalpha(int * vl, int len, const char * name)
{
    int ctr;
    printf("Utils::LoadFromDecDigit(%s, \"", name);
    for (ctr = 1; ctr < len; ++ctr)
    {
        printf("%02X", vl[ctr]);
    }
    printf("\");\n");
}

void printvls()
{
    printalpha(viewer.SHIE_VLA, 14, "SHIE_VLA");
    printalpha(viewer.SWOR_VLA, 11, "SWOR_VLA");
    printalpha(viewer.TORC_VLA, 10, "TORC_VLA");
    printalpha(viewer.RING_VLA, 12, "RING_VLA");
    printalpha(viewer.SCRO_VLA, 12, "SCRO_VLA");
    printalpha(viewer.FLAS_VLA, 10, "FLAS_VLA");
}
