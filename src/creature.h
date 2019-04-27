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
// Filename: creature.h
//
// This class manages the creature data and movement

#ifndef DOD_CREATURE_HEADER
#define DOD_CREATURE_HEADER

#include "dod.h"

class Creature
{
public:
    Creature();

    void Reset();

    // Load creature sound effects
    void LoadSounds();

    // This routine creates a new dungeon level, filling it with objects and
    // creatures. It should probably be moved to the Dungeon class.
    void NEWLVL();

    // This method is called from the scheduler once every five
    // minutes. It will generate random new creatures.
    int CREGEN();

    // This method is called from the scheduler to move the creatures. This is
    // where most of the creature logic resides. It's frequency is determined by
    // the creature type and its location relative to the player.
    int CMOVE(int task, int cidx);

    // This routine attempts to move the creature in the given direction.
    bool CWALK(dodBYTE dir, CCB * cr) const;

    // Returns false if a creature exists at position rw, cl.
    bool CFIND(dodBYTE rw, dodBYTE cl) const;

    // Checks if a creature exists at rc and returns its index in CCBLND
    // else returns -1.
    int CFIND2(RowCol rc) const;

    // Returns creature speed multiplier
    int getCreatureSpeed() const { return creSpeedMul; }

    // Update creature speed
    void UpdateCreSpeed(int newSpeed);

    // All creatures in the current level
    CCB CCBLND[32];

    // Indicates if all creatures are frozen
    dodBYTE FRZFLG;

    // Current offset into CMXLND
    int CMXPTR;

    // Tells us how many creatures for each type should be created in each level
    dodBYTE CMXLND[60];

    Mix_Chunk * creSound[12];
    Mix_Chunk * clank;
    Mix_Chunk * kaboom;
    Mix_Chunk * buzz;

    // Sound channels
    const int creChannel;
    const int creChannelv;

    enum creature_type_t : dodBYTE
    {
        CRT_SPIDER   = 0,
        CRT_VIPER    = 1,
        CRT_GIANT1   = 2,
        CRT_BLOB     = 3,
        CRT_KNIGHT1  = 4,
        CRT_GIANT2   = 5,
        CRT_SCORPION = 6,
        CRT_KNIGHT2  = 7,
        CRT_WRAITH   = 8,
        CRT_GALDROG  = 9,
        CRT_WIZIMG   = 10,
        CRT_WIZARD   = 11,
        CTYPES       = 12
    };

private:
    // Creates a new creature and places it in the maze
    void CBIRTH(creature_type_t typ);

    // Used when creature is on same row or column as the player.
    // Returns false if the player is reachable from the creature's position.
    bool moveTowardPlayer(int task, int cidx, int dir);

    // Update next_time of task depending on whether the creature is attacking
    // the player or just moving around.
    void updateTaskTime(int task, int cidx) const;

    // All creature types and their stats
    CDB CDBTAB[12];

    // Creature speed multiplier
    int creSpeedMul;
};

#endif // DOD_CREATURE_HEADER
