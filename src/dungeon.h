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
// Filename: dungeon.h
//
// This class manages the maze data, and includes
// methods related to row/column calculations.

#ifndef DOD_DUNGEON_HEADER
#define DOD_DUNGEON_HEADER

#include "dod.h"
#include "rng.h"

extern RNG rng; // Included here for inlines below

class Dungeon
{
public:

    Dungeon();

    // Prints a text drawing of the maze.
    void printMaze();

    // Sets original maze seed values.
    void SetLEVTABOrig();

    // Sets original vertical feature table values.
    void SetVFTTABOrig();

    //Override original vertical feature table values with new ones.
    //Will override other level's col & row when during map generation.
    void SetVFTTABRandomMap();

    // Override seeds with true random numbers.
    void SetLEVTABRandomMap();

    // Builds the maze
    void DGNGEN();

    // Adds vertical features
    void CalcVFI();

    // Returns the index for the given row and column
    int RC2IDX(dodBYTE R, dodBYTE C) const;

    // Checks if a step can be taken from the given row/col in the given direction
    bool STEPOK(dodBYTE R, dodBYTE C, dodBYTE dir) const;

    // Checks if a hole/ladder is in cell
    // It has to check above and below, since each
    // vertical feature is stored only once in the VFT
    // Returns VF_HOLE_UP, VF_LADDER_UP, VF_HOLE_DOWN, VF_LADDER_DOWN or VF_NULL
    dodBYTE VFIND(const RowCol& rc) const;

    // Checks for a wall in the given direction
    bool TryMove(dodBYTE dir) const;

    // Returns cell at row, col
    dodBYTE& cell(dodBYTE row, dodBYTE col)
    {
        return MAZLND[row * 32 + col];
    }

    // Returns cell at row, col
    const dodBYTE& cell(dodBYTE row, dodBYTE col) const
    {
        return MAZLND[row * 32 + col];
    }

    // The 32x32 maze
    dodBYTE MAZLND[1024];

    // The cells around the room we're interested in
    dodBYTE NEIBOR[9];

    // The RNG seeds for the current maze
    dodBYTE LEVTAB[7];

    RowCol  DROW;

    // Step table as row col pairs.
    static constexpr int STPTAB[8] = {
        -1,  0, // North
         0,  1, // East
         1,  0, // South
         0, -1  // West
    };

    // Array of vertical features (holes and ladders) for all levels
    // One entry consists of 3 consecutive dodBYTEs = type, row, col
    dodBYTE VFTTAB[42];

    // Points to current level in VFTTAB
    int VFTPTR;

    enum wall_t
    {
        N_WALL = 0x03,
        E_WALL = 0x03 << 2,
        S_WALL = 0x03 << 4,
        W_WALL = 0x03 << 6,
    };

    enum horizontal_feature_t
    {
        HF_PAS = 0,
        HF_DOR = 1,
        HF_SDR = 2,
        HF_WAL = 3,
    };

    enum vertical_feature_t
    {
        VF_HOLE_UP     = 0,
        VF_LADDER_UP   = 1,
        VF_HOLE_DOWN   = 2,
        VF_LADDER_DOWN = 3,
        VF_NULL        = 255,
    };

private:

    // Returns false if R, C is out of bounds
    bool BORDER(dodBYTE R, dodBYTE C) const;

    // Adds doors to dungeon
    void MAKDOR(const dodBYTE * table);

    // Finds surrounding cells and fills NEIBOR array
    void FRIEND(RowCol RC);

    // Retrieve a random direction and distance
    void RndDstDir(dodBYTE * DIR, dodBYTE * DST) const;

    // Used by VFIND
    bool VFINDsub(dodBYTE & a, int & u, const RowCol& rc) const;

    // Mask values for walls
    static constexpr dodBYTE MSKTAB[4] = {0x03, 0x0C, 0x30, 0xC0};

    // Door mask values
    static constexpr dodBYTE DORTAB[4] = {HF_DOR, HF_DOR << 2, HF_DOR << 4, HF_DOR << 6};

    // Secret door mask values
    static constexpr dodBYTE SDRTAB[4] = {HF_SDR, HF_SDR << 2, HF_SDR << 4, HF_SDR << 6};

    // How to display horizontal features in the printMaze method
    static constexpr char NS[4] = {' ', '-', '=', '-'};
    static constexpr char EW[4] = {' ', '|', ')', '|'};
};


inline void Dungeon::RndDstDir(dodBYTE * DIR, dodBYTE * DST) const
{
    *DIR = (rng.RANDOM() & 3);
    *DST = (rng.RANDOM() & 7) + 1;
}

inline bool Dungeon::BORDER(dodBYTE R, dodBYTE C) const
{
    if ((R & 224) != 0) return false;
    if ((C & 224) != 0) return false;
    return true;
}

inline int Dungeon::RC2IDX(dodBYTE R, dodBYTE C) const
{
    R &= 31;
    C &= 31;
    return (R * 32 + C);
}

#endif // DOD_DUNGEON_HEADER
