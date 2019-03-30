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
// Filename: dodgame.h
//
// This class is intended to be a controller class, but
// so much of the functionality is closely associated
// with the primary objects that there is not much going
// on here.  As the game grows, this may change.

#ifndef DOD_GAME_HEADER
#define DOD_GAME_HEADER

#include "dod.h"

class dodGame
{
public:

    dodGame();

    // Common initialization
    void COMINI();

    // Initializes 3D viewer
    void INIVU();

    // Starts a new game
    void Restart();

    // Loads a game
    void LoadGame();

    // Pause 1.35 seconds
    void WAIT();

    // Dungeon level (0-4)
    dodBYTE LEVEL;

    // If we're showing the demo gameplay in the intro
    bool IsDemo;

    // If random mazes should be created
    bool RandomMaze;

    // If we should fix the physical and magical defense values for bronze and
    // leather shield. These values were wrong in the original game.
    bool ShieldFix;

    // Allow player to find a vision scroll on the first level
    bool VisionScroll;

    // If creatures should ignore objects instead of picking them up
    bool CreaturesIgnoreObjects;

    // If we instantly spawn a new creature once the player kills a creature
    bool CreaturesInstaRegen;

    // If we mark doors on the map
    bool MarkDoorsOnScrollMaps;

    // Autoplay (demo) flag
    bool AUTFLG;

    // true if the player won the game
    bool hasWon;

    // if we should play the demo again
    bool demoRestart;

    // current offset into DEMO_CMDS
    int DEMOPTR;

    // commands for the intro gameplay demo
    dodBYTE DEMO_CMDS[256];
};

#endif // DOD_GAME_HEADER
