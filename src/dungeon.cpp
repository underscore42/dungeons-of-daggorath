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
// Filename: dungeon.cpp
//
// Implementation of Dungeon class

#include <random>

#include "dungeon.h"
#include "dodgame.h"
#include "player.h"
#include "sched.h"

extern Scheduler    scheduler;
extern Player       player;
extern dodGame      game;

void Dungeon::printMaze()
{
    for (unsigned idx = 0; idx < 1024; idx += 32)
    {
        for (unsigned x = 0; x < 3; ++x)
        {
            for (unsigned row = 0; row < 32; ++row)
            {
                dodBYTE val = MAZLND[idx + row];
                dodBYTE n = (val & N_WALL);
                dodBYTE e = (val & E_WALL) >> 2;
                dodBYTE s = (val & S_WALL) >> 4;
                dodBYTE w = (val & W_WALL) >> 6;
                switch (x)
                {
                case 0:
                    printf("·%c", NS[n]);
                    if (row >= 31)
                    {
                        printf("·");
                    }
                    break;
                case 1:
                    printf("%c", EW[w]);
                    if (val == 0xFF)
                        printf("#");
                    else
                        printf(" ");
                    if (row >= 31)
                    {
                        printf("%c", EW[e]);
                    }
                    break;
                case 2:
                    if (idx >= 992)
                    {
                        printf("·%c", NS[s]);
                        if (row >= 31)
                        {
                            printf("·");
                        }
                    }
                }
            }
            if (x < 2)
            {
                printf("\n");
            }
        }
    }
    printf("\n");
}

Dungeon::Dungeon() : VFTPTR(0)
{
    //Original seed values will be overwritten (in Player::setInitialObjects())
    //if new random map game.
    SetLEVTABOrig();

    //Original vertical feature table values will be overwritten (in Dungeon::DGNGEN())
    //if new random map game.
    SetVFTTABOrig();
}

void Dungeon::DGNGEN()
{
    int     mzctr;
    int     maz_idx;
    dodBYTE a_row;
    dodBYTE a_col;
    dodBYTE b_row;
    dodBYTE b_col;
    dodBYTE DIR;
    dodBYTE DST;
    RowCol  DROW;
    RowCol  ROW;

    /* Phase 1: Create Maze */

    /* Set Cells to 0xFF */
    for (mzctr = 0; mzctr < 1024; ++mzctr)
    {
        MAZLND[mzctr] = 0xFF;
    }

    //Initialize Random Number Generator
    rng.setSEED(LEVTAB[game.LEVEL], LEVTAB[game.LEVEL + 1], LEVTAB[game.LEVEL + 2]);
    int cell_ctr = 500;  // Room Counter

    /* Set Starting Room */
    if (!game.RandomMaze || game.IsDemo)
    {
        //Is this an original game?  Yes:
        a_col = (rng.RANDOM() & 31);
        a_row = (rng.RANDOM() & 31);
        DROW.setRC(a_row, a_col);
        RndDstDir(&DIR, &DST);

        //Make sure the vertical feature table isn't overwritten from previous new game.
        SetVFTTABOrig();
    }
    else      //Is this an original game?  No:
    {
        switch (game.LEVEL)
        {
        case 0:
        case 3:
            a_col = (rng.RANDOM() & 31);
            a_row = (rng.RANDOM() & 31);
            break;
        case 1:
            a_row = VFTTAB[5];
            a_col = VFTTAB[6];
            break;
        case 2:
            a_row = VFTTAB[9];
            a_col = VFTTAB[10];
            break;
        default:
            a_row = VFTTAB[14];
            a_col = VFTTAB[15];
            break;
        }

        if (player.PROW == 0x10 && player.PCOL == 0x0B && game.LEVEL == 0)
        {
            //Are we starting a new game?
            player.PROW = a_row;
            player.PCOL = a_col;

            //Override vertical features.
            //Will override other level's col & row during map generation.
            SetVFTTABRandomMap();
            VFTTAB[1] = a_row;
            VFTTAB[2] = a_col;
        }  //Are we starting a new game?

        //Original didn't tunnel out original room.
        //Need to do it now so that player doesn't start in wall in beginning of game.
        //Also need to make sure ladder back up to each level is in a tunneled out room.
        DROW.setRC(a_row, a_col);
        RndDstDir(&DIR, &DST);
        maz_idx = RC2IDX(a_row, a_col);
        MAZLND[maz_idx] = 0;
        --cell_ctr;
    }  //Is this an original game?

    // Carve out all the rooms
    while (cell_ctr > 0)
    {
        /* Take a step */
        b_row = DROW.row;
        b_col = DROW.col;
        b_row += STPTAB[DIR * 2];
        b_col += STPTAB[(DIR * 2) + 1];

        /* Check if it's out of bounds */
        if (BORDER(b_row, b_col) == false)
        {
            RndDstDir(&DIR, &DST);
            continue;
        }

        /* Store index and temp room */
        maz_idx = RC2IDX(b_row, b_col);
        ROW.setRC(b_row, b_col);

        /* If not yet touched */
        if (MAZLND[maz_idx] == 0xFF)
        {
            FRIEND(ROW);
            if (NEIBOR[3] + NEIBOR[0] + NEIBOR[1] == 0 ||
                NEIBOR[1] + NEIBOR[2] + NEIBOR[5] == 0 ||
                NEIBOR[5] + NEIBOR[8] + NEIBOR[7] == 0 ||
                NEIBOR[7] + NEIBOR[6] + NEIBOR[3] == 0)
            {
                RndDstDir(&DIR, &DST);
                continue;
            }
            MAZLND[maz_idx] = 0;
            --cell_ctr;
        }
        if (cell_ctr > 0)
        {
            DROW = ROW;
            --DST;
            if (DST == 0)
            {
                RndDstDir(&DIR, &DST);
                continue;
            }
            else
            {
                continue;
            }
        }
    }

    /* Phase 2: Create Walls */

    for (DROW.row = 0; DROW.row < 32; ++DROW.row)
    {
        for (DROW.col = 0; DROW.col < 32; ++DROW.col)
        {
            maz_idx = RC2IDX(DROW.row, DROW.col);
            if (MAZLND[maz_idx] != 0xFF)
            {
                FRIEND(DROW);
                if (NEIBOR[1] == 0xFF)
                    MAZLND[maz_idx] |= N_WALL;
                if (NEIBOR[3] == 0xFF)
                    MAZLND[maz_idx] |= W_WALL;
                if (NEIBOR[5] == 0xFF)
                    MAZLND[maz_idx] |= E_WALL;
                if (NEIBOR[7] == 0xFF)
                    MAZLND[maz_idx] |= S_WALL;
            }
        }
    }

    /* Phase 3: Create Doors/Secret Doors */

    for (mzctr = 0; mzctr < 70; ++mzctr)
    {
        MAKDOR(this->DORTAB);
    }

    for (mzctr = 0; mzctr < 45; ++mzctr)
    {
        MAKDOR(this->SDRTAB);
    }

    /* Phase 4: Create vertical feature */
    if (game.RandomMaze && !game.IsDemo && (game.LEVEL == 0 || game.LEVEL == 1 || game.LEVEL == 3))
    {
        do
        {
            do
            {
                a_col = (rng.RANDOM() & 31);
                a_row = (rng.RANDOM() & 31);
                ROW.setRC(a_row, a_col);
                maz_idx = RC2IDX(a_row, a_col);
            }
            while (MAZLND[maz_idx] == 0xFF);
        }
        while ((game.LEVEL == 0 && VFTTAB[1] == a_row && VFTTAB[2] == a_col) ||
               (game.LEVEL == 1 && VFTTAB[5] == a_row && VFTTAB[6] == a_col));
        switch (game.LEVEL)
        {
        case 0:
            if (VFTTAB[5] == 0 && VFTTAB[6] == 0)
            {
                VFTTAB[5] = a_row;
                VFTTAB[6] = a_col;
            }
            break;
        case 1:
            if (VFTTAB[9] == 0 && VFTTAB[10] == 0)
            {
                VFTTAB[9] = a_row;
                VFTTAB[10] = a_col;
            }
            break;
        default:
            if (VFTTAB[14] == 0 && VFTTAB[15] == 0)
            {
                VFTTAB[14] = a_row;
                VFTTAB[15] = a_col;
            }
            break;
        }
    }

    // Spin the RNG
    int spin;
    if (scheduler.curTime == 0)
    {
        if (game.LEVEL == 0)
            spin = 6;
        else
            spin = 21;
    }
    else
    {
        spin = (scheduler.curTime % 60);
    }

    while (spin > 0)
    {
        rng.RANDOM();
        --spin;
    }
}

void Dungeon::CalcVFI()
{
    dodBYTE lvl = game.LEVEL;
    dodBYTE idx = 0;
    do
    {
        VFTPTR = idx;
        while (VFTTAB[idx++] != 0xFF)
            ;   // loop !!!
        --lvl;
    }
    while (lvl != 0xFF);
}

dodBYTE Dungeon::VFIND(const RowCol& rc) const
{
    int u = VFTPTR;
    dodBYTE a = 0;
    bool res = VFINDsub(a, u, rc);
    if (res == true)
        return a;
    res = VFINDsub(a, u, rc);
    if (res == true)
        return a + 2;
    else
        return -1;
}

bool Dungeon::VFINDsub(dodBYTE & a, int & u, const RowCol& rc) const
{
    dodBYTE r, c;

    do
    {
        a = VFTTAB[u++];
        if (a == 0xFF)
            return false;
        r = VFTTAB[u++];
        c = VFTTAB[u++];
    }
    while ( !((r == rc.row) && (c == rc.col)) );
    return true;
}

bool Dungeon::TryMove(dodBYTE dir) const
{
    const dodBYTE c = cell(player.PROW, player.PCOL);
    const dodBYTE a = ((c >> (dir * 2)) & 3);

    return a != 3;
}

void Dungeon::MAKDOR(const dodBYTE * table)
{
    int     maz_idx;
    dodBYTE val;
    dodBYTE DIR;
    RowCol  ROW;

    do
    {
        do
        {
            const dodBYTE a_col = (rng.RANDOM() & 31);
            const dodBYTE a_row = (rng.RANDOM() & 31);
            ROW.setRC(a_row, a_col);
            maz_idx = RC2IDX(a_row, a_col);
            val = MAZLND[maz_idx];
        }
        while (val == 0xFF);

        DIR = (rng.RANDOM() & 3);
    }
    while ((val & MSKTAB[DIR]) != 0);

    MAZLND[maz_idx] |= table[DIR];

    ROW.row += STPTAB[DIR * 2];
    ROW.col += STPTAB[(DIR * 2) + 1];
    DIR += 2;
    DIR &= 3;
    maz_idx = RC2IDX(ROW.row, ROW.col);
    MAZLND[maz_idx] |= table[DIR];
}

void Dungeon::FRIEND(RowCol RC)
{
    int u = 0;

    for (dodBYTE r3 = RC.row; r3 <= (RC.row + 2); ++r3)
    {
        for (dodBYTE c3 = RC.col; c3 <= (RC.col + 2); ++c3)
        {
            if (BORDER(r3 - 1, c3 - 1) == false)
            {
                NEIBOR[u] = 0xFF;
            }
            else
            {
                NEIBOR[u] = cell(r3 - 1, c3 - 1);
            }
            ++u;
        }
    }
}

bool Dungeon::STEPOK(dodBYTE R, dodBYTE C, dodBYTE dir) const
{
    R += STPTAB[dir * 2];
    C += STPTAB[(dir * 2) + 1];
    if (BORDER(R, C) == false) return false;
    if (cell(R, C) == 255) return false;
    return true;
}

void Dungeon::SetVFTTABOrig()
{
    VFTTAB[0] = -1;
    VFTTAB[1] = 1;
    VFTTAB[2] = 0;
    VFTTAB[3] = 23;
    VFTTAB[4] = 0;
    VFTTAB[5] = 15;
    VFTTAB[6] = 4;
    VFTTAB[7] = 0;
    VFTTAB[8] = 20;
    VFTTAB[9] = 17;
    VFTTAB[10] = 1;
    VFTTAB[11] = 28;
    VFTTAB[12] = 30;
    VFTTAB[13] = -1;
    VFTTAB[14] = 1;
    VFTTAB[15] = 2;
    VFTTAB[16] = 3;
    VFTTAB[17] = 0;
    VFTTAB[18] = 3;
    VFTTAB[19] = 31;
    VFTTAB[20] = 0;
    VFTTAB[21] = 19;
    VFTTAB[22] = 20;
    VFTTAB[23] = 0;
    VFTTAB[24] = 31;
    VFTTAB[25] = 0;
    VFTTAB[26] = -1;
    VFTTAB[27] = -1;
    VFTTAB[28] = 0;
    VFTTAB[29] = 0;
    VFTTAB[30] = 31;
    VFTTAB[31] = 0;
    VFTTAB[32] = 5;
    VFTTAB[33] = 0;
    VFTTAB[34] = 0;
    VFTTAB[35] = 22;
    VFTTAB[36] = 28;
    VFTTAB[37] = 0;
    VFTTAB[38] = 31;
    VFTTAB[39] = 16;
    VFTTAB[40] = -1;
    VFTTAB[41] = -1;
}

void Dungeon::SetLEVTABOrig()
{
    LEVTAB[0] = 0x73;
    LEVTAB[1] = 0xC7;
    LEVTAB[2] = 0x5D;
    LEVTAB[3] = 0x97;
    LEVTAB[4] = 0xF3;
    LEVTAB[5] = 0x13;
    LEVTAB[6] = 0x87;
}

void Dungeon::SetVFTTABRandomMap()
{
    VFTTAB[0] = 0;
    VFTTAB[1] = 0;
    VFTTAB[2] = 0;
    VFTTAB[3] = -1;
    VFTTAB[4] = 1;
    VFTTAB[5] = 0;
    VFTTAB[6] = 0;
    VFTTAB[7] = -1;
    VFTTAB[8] = 1;
    VFTTAB[9] = 0;
    VFTTAB[10] = 0;
    VFTTAB[11] = -1;
    VFTTAB[12] = -1;
    VFTTAB[13] = 1;
    VFTTAB[14] = 0;
    VFTTAB[15] = 0;
    VFTTAB[16] = -1;
    VFTTAB[17] = -1;
}

void Dungeon::SetLEVTABRandomMap()
{
    std::random_device rdev;
    std::mt19937 rng(rdev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0);

    for (unsigned i = 0; i < 7; ++i)
        LEVTAB[i] = dist(rng);
}