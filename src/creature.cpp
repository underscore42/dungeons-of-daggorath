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
// Filename: creature.cpp
//
// Implementation of Creature class

#include "creature.h"
#include "dodgame.h"
#include "dungeon.h"
#include "object.h"
#include "sched.h"
#include "viewer.h"
#include "player.h"
#include "oslink.h"
#include "math.h"
#include "enhanced.h"

extern OS_Link      oslink;
extern Player       player;
extern RNG          rng;
extern Viewer       viewer;
extern dodGame      game;
extern Dungeon      dungeon;
extern Object       object;
extern Scheduler    scheduler;

Creature::Creature()
:
    creChannel(1),
    creChannelv(2)
{
    Reset();
}

void Creature::Reset()
{
    creSpeedMul = 100;

    CMXPTR = 0;
    FRZFLG = 0;

    //                          POW  MGO  MGD  PHO  PHD   TMV   TAT
    CDBTAB[CRT_SPIDER  ] = CDB(  32,   0, 255, 128, 255, 2300, 1100);
    CDBTAB[CRT_VIPER   ] = CDB(  56,   0, 255,  80, 128, 1500,  700);
    CDBTAB[CRT_GIANT1  ] = CDB( 200,   0, 255,  52, 192, 2900, 2300);
    CDBTAB[CRT_BLOB    ] = CDB( 304,   0, 255,  96, 167, 3100, 3100);
    CDBTAB[CRT_KNIGHT1 ] = CDB( 504,   0, 128,  96,  60, 1300,  700);
    CDBTAB[CRT_GIANT2  ] = CDB( 704,   0, 128, 128,  48, 1700, 1300);
    CDBTAB[CRT_SCORPION] = CDB( 400, 255, 128, 255, 128,  500,  400);
    CDBTAB[CRT_KNIGHT2 ] = CDB( 800,   0,  64, 255,   8, 1300,  700);
    CDBTAB[CRT_WRAITH  ] = CDB( 800, 192,  16, 192,   8,  300,  300);
    CDBTAB[CRT_GALDROG ] = CDB(1000, 255,   5, 255,   3,  400,  300);
    CDBTAB[CRT_WIZIMG  ] = CDB(1000, 255,   6, 255,   0, 1300,  700);
    CDBTAB[CRT_WIZARD  ] = CDB(8000, 255,   6, 255,   0, 1300,  700);

    if (game.VisionScroll)  //Do we need to replace a snake with a blob w/ a vision scroll?  Yes:
        Utils::LoadFromDecDigit(CMXLND, "984300000000240666000000000406840010000000866400222222244801");
    else  //Do we need to replace a snake with a blob w/ a vision scroll?  No:
        Utils::LoadFromDecDigit(CMXLND, "994200000000240666000000000406840010000000866400222222244801");
}

void Creature::UpdateCreSpeed()
{
    for (int ccc = 0; ccc < 12; ++ccc)
    {
        CDBTAB[ccc].P_CDTMV = ((int)((float)CDBTAB[ccc].P_CDTMV * (float)creSpeedMul / 100.0));
        CDBTAB[ccc].P_CDTAT = ((int)((float)CDBTAB[ccc].P_CDTAT * (float)creSpeedMul / 100.0));
    }
}

void Creature::LoadSounds()
{
    creSound[0] = Utils::LoadSound("00_squeak.wav");
    creSound[1] = Utils::LoadSound("01_rattle.wav");
    creSound[2] = Utils::LoadSound("02_growl.wav");
    creSound[3] = Utils::LoadSound("03_beoop.wav");
    creSound[4] = Utils::LoadSound("04_klank.wav");
    creSound[5] = Utils::LoadSound("05_grawl.wav");
    creSound[6] = Utils::LoadSound("06_pssst.wav");
    creSound[7] = Utils::LoadSound("07_kklank.wav");
    creSound[8] = Utils::LoadSound("08_pssht.wav");
    creSound[9] = Utils::LoadSound("09_snarl.wav");
    creSound[10] = Utils::LoadSound("0A_bdlbdl.wav");
    creSound[11] = Utils::LoadSound("0B_bdlbdl.wav");
    clank = Utils::LoadSound("13_clank.wav");
    kaboom = Utils::LoadSound("16_kaboom.wav");
    buzz = Utils::LoadSound("19_buzz.wav");
}

void Creature::NEWLVL()
{
    CMXPTR = game.LEVEL * CTYPES;
    dungeon.CalcVFI();

    for (int i = 0; i < 32; ++i)
        CCBLND[i].clear();

    scheduler.SYSTCB();
    dungeon.DGNGEN();

    for (unsigned a = CTYPES - 1; a-- > 0;)
    {
        for (unsigned b = 0; b < CMXLND[CMXPTR + a]; ++b)
            CBIRTH(creature_type_t(a));
    }

    int u = -1;
    object.OFINDF = 0;
    for (;;)
    {
        const int idx = object.FNDOBJ();
        if (idx == -1)
        {
            break;
        }
        if (object.OCBLND[idx].P_OCOWN == 0xFF)
        {
            for (;;)
            {
                ++u;
                if (u == 32)
                {
                    u = 0;
                }
                if (CCBLND[u].P_CCUSE != 0)
                {
                    const int tmp = CCBLND[u].P_CCOBJ;
                    CCBLND[u].P_CCOBJ = idx;
                    object.OCBLND[idx].P_OCPTR = tmp;
                    break;
                }
            }
        }
    }

    // Determine video invert Setting
    viewer.setVidInv((game.LEVEL % 2) ? true : false);
}

void Creature::CBIRTH(creature_type_t typ)
{
    int u = -1;
    do
    {
        ++u;
    }
    while (CCBLND[u].P_CCUSE != 0);
    --CCBLND[u].P_CCUSE;

    CCBLND[u].creature_id = typ;
    CCBLND[u].P_CCPOW = CDBTAB[typ].P_CDPOW;
    CCBLND[u].P_CCMGO = CDBTAB[typ].P_CDMGO;
    CCBLND[u].P_CCMGD = CDBTAB[typ].P_CDMGD;
    CCBLND[u].P_CCPHO = CDBTAB[typ].P_CDPHO;
    CCBLND[u].P_CCPHD = CDBTAB[typ].P_CDPHD;
    CCBLND[u].P_CCTMV = CDBTAB[typ].P_CDTMV;
    CCBLND[u].P_CCTAT = CDBTAB[typ].P_CDTAT;

    dodBYTE rw, cl;
    for (;;)
    {
        for (;;)
        {
            cl = rng.RANDOM() & 31;
            rw = rng.RANDOM() & 31;
            const int maz_idx = dungeon.RC2IDX(rw, cl);
            //printf("          %02X, %02X = %02X\n", rw, cl, dungeon.MAZLND[maz_idx]);
            if (dungeon.MAZLND[maz_idx] != 0xFF)
                break;
        }

        if (CFIND(rw, cl))
            break;
    }

    //printf("----- %02X: %02X, %02X -----\n", typ, rw, cl);
    CCBLND[u].P_CCROW = rw;
    CCBLND[u].P_CCCOL = cl;

    const int TCBindex = scheduler.GETTCB();
    scheduler.TCBLND[TCBindex].data = u;
    scheduler.TCBLND[TCBindex].type = Scheduler::TID_CRTMOVE;
    scheduler.TCBLND[TCBindex].frequency = CCBLND[u].P_CCTMV;
}

bool Creature::CFIND(dodBYTE rw, dodBYTE cl) const
{
    return CFIND2(RowCol(rw, cl)) == -1;
}

int Creature::CFIND2(RowCol rc) const
{
    int ctr = 0;
    while (ctr < 32)
    {
        if (CCBLND[ctr].P_CCROW == rc.row && CCBLND[ctr].P_CCCOL == rc.col)
        {
            if (CCBLND[ctr].P_CCUSE != 0)
            {
                return ctr;
            }
        }
        ++ctr;
    }
    return -1;
}


int Creature::CREGEN()
{
    // Update Task's next_time
    scheduler.TCBLND[Scheduler::TID_CRTREGEN].next_time =
        scheduler.curTime +
        scheduler.TCBLND[Scheduler::TID_CRTREGEN].frequency;

    dodBYTE B = CTYPES - 1;
    dodBYTE A = 0;
    do
    {
        A += CMXLND[CMXPTR + B];
        --B;
    }
    while (B != 255);
    if (A < 32)
    {
        A = rng.RANDOM();
        if (g_cheats & CHEAT_REGEN_SCALING)
        {
            switch (game.LEVEL)
            {
            case 0:
                A %= 4;
                break;
            case 1:
                A %= 5;
                A += 1;
                break;
            case 2:
                A %= 6;
                A += 1;
                break;
            case 3:
                A %= 7;
                A += 2;
                break;
            case 4:
                A %= 7;
                A += 3;
                break;
            }
        }
        else
        {
            A &= 7;
            A += 2;
        }
        ++CMXLND[CMXPTR + A];
    }
    return 0;
}

int Creature::CMOVE(int task, int cidx)
{
    if (FRZFLG)
    {
        updateTaskTime(task, cidx);
        return 0;
    }

    // ignore dead creatures
    if (CCBLND[cidx].P_CCUSE == 0)
    {
        return 0;
    }

    // pick up object
    if (CCBLND[cidx].creature_id != CRT_SCORPION &&
        CCBLND[cidx].creature_id < CRT_WIZIMG &&
        !(
            game.CreaturesIgnoreObjects &&
            CCBLND[cidx].P_CCROW == player.PROW &&
            CCBLND[cidx].P_CCCOL == player.PCOL
        )
    )
    {
        object.OFINDF = 0;
        const int oidx = object.OFIND(RowCol(CCBLND[cidx].P_CCROW,
                                             CCBLND[cidx].P_CCCOL));
        if (oidx != -1)
        {
            object.OCBLND[oidx].P_OCPTR = CCBLND[cidx].P_CCOBJ;
            CCBLND[cidx].P_CCOBJ = oidx;
            --object.OCBLND[oidx].P_OCOWN;
            viewer.PUPDAT();
            updateTaskTime(task, cidx);
            return 0;
        }
    }

    // attack player
    if (CCBLND[cidx].P_CCROW == player.PROW &&
        CCBLND[cidx].P_CCCOL == player.PCOL)
    {
        // do creature sound
        Mix_PlayChannel(creChannel, creSound[CCBLND[cidx].creature_id], 0);
        while (Mix_Playing(creChannel) == 1)
        {
            if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
            {
                scheduler.CLOCK();
                if (game.AUTFLG && game.demoRestart == false)
                {
                    return 0;
                }
            }
            scheduler.curTime = SDL_GetTicks();
        }

        // set player shielding parameters
        dodBYTE shA = 0x80;
        dodBYTE shB = 0x80;

        dodSHORT shD, shD2;
        if (player.PLHAND != -1 && object.OCBLND[player.PLHAND].obj_type == Object::OBJT_SHIELD)
        {
            shD = (((int)shA << 8) | shB);
            shD2 = (((int)object.OCBLND[player.PLHAND].P_OCXX0 << 8) |
                    object.OCBLND[player.PLHAND].P_OCXX1);
            if (shD2 < shD)
            {
                shA = (shD2 >> 8);
                shB = (shD2 & 255);
            }
        }

        if (player.PRHAND != -1 && object.OCBLND[player.PRHAND].obj_type == Object::OBJT_SHIELD)
        {
            shD = (((int)shA << 8) | shB);
            shD2 = (((int)object.OCBLND[player.PRHAND].P_OCXX0 << 8) |
                    object.OCBLND[player.PRHAND].P_OCXX1);
            if (shD2 < shD)
            {
                shA = (shD2 >> 8);
                shB = (shD2 & 255);
            }
        }

        player.PMGD = shA;
        player.PPHD = shB;

        // process attack

        if (!(g_cheats & CHEAT_INVULNERABLE))
        {
            if (player.ATTACK(CCBLND[cidx].P_CCPOW, player.PPOW, player.PDAM))
            {
                // make CLANK sound
                Mix_PlayChannel(creChannel, clank, 0);
                while (Mix_Playing(creChannel) == 1)
                {
                    if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
                    {
                        scheduler.CLOCK();
                        if (game.AUTFLG && game.demoRestart == false)
                        {
                            return 0;
                        }
                    }
                    scheduler.curTime = SDL_GetTicks();
                }

                player.DAMAGE(CCBLND[cidx].P_CCPOW, CCBLND[cidx].P_CCMGO,
                              CCBLND[cidx].P_CCPHO, player.PPOW,
                              player.PMGD, player.PPHD, &player.PDAM);
            }
        }

        player.HUPDAT();
        scheduler.TCBLND[task].next_time = scheduler.curTime +
                                           CCBLND[cidx].P_CCTAT;
        return 0;
    }

    bool doRandom = false;
    // look for player along ROW axis
    if (CCBLND[cidx].P_CCROW == player.PROW)
    {
        const int dir = CCBLND[cidx].P_CCCOL < player.PCOL ? 1 : 3;

        doRandom = moveTowardPlayer(task, cidx, dir);

        if (!doRandom)
            return 0;
    }

    // look for player along COL axis
    if (CCBLND[cidx].P_CCCOL == player.PCOL)
    {
        const int dir = CCBLND[cidx].P_CCROW < player.PROW ? 2 : 0;

        doRandom = moveTowardPlayer(task, cidx, dir);

        if (!doRandom)
            return 0;
    }

    // player not seen so make random move
    if (doRandom || (CCBLND[cidx].P_CCROW != player.PROW &&
                     CCBLND[cidx].P_CCCOL != player.PCOL))
    {
        // directions to try:
        const dodBYTE MOVTAB[7] = {0, 3, 1, 0, 1, 3, 0};

        int X = 0;
        dodBYTE rnd = rng.RANDOM();
        if ((rnd & 128) == 0)
        {
            X += 3;
        }
        rnd &= 3;
        if (rnd == 0)
        {
            X += 1;
        }
        int loop = 3;
        do
        {
            if (CWALK(MOVTAB[X++], &CCBLND[cidx]))
            {
                updateTaskTime(task, cidx);
                return 0;
            }
            --loop;
        }
        while (loop != 0);
        CWALK(2, &CCBLND[cidx]);
    }

    updateTaskTime(task, cidx);

    return 0;
}

bool Creature::moveTowardPlayer(int task, int cidx, int dir)
{
    bool reachable = true;
    dodBYTE r = CCBLND[cidx].P_CCROW;
    dodBYTE c = CCBLND[cidx].P_CCCOL;
    do
    {
        if (!dungeon.STEPOK(r, c, dir))
        {
            reachable = false;
            break;
        }
        r += dungeon.STPTAB[dir * 2];
        c += dungeon.STPTAB[(dir * 2) + 1];
    }
    while (!(r == player.PROW && c == player.PCOL));

    if (reachable)
    {
        CCBLND[cidx].P_CCDIR = dir;
        CWALK(0, &CCBLND[cidx]);
        updateTaskTime(task, cidx);
    }

    return !reachable;
}

void Creature::updateTaskTime(int task, int cidx) const
{
    if (CCBLND[cidx].P_CCROW == player.PROW &&
        CCBLND[cidx].P_CCCOL == player.PCOL)
    {
        viewer.PUPDAT();
        scheduler.TCBLND[task].next_time = scheduler.curTime +
                                           CCBLND[cidx].P_CCTAT;
    }
    else
    {
        scheduler.TCBLND[task].next_time = scheduler.curTime +
                                           CCBLND[cidx].P_CCTMV;
    }
}

bool Creature::CWALK(dodBYTE dir, CCB * cr) const
{
    dir += cr->P_CCDIR;
    dir &= 3;

    if (!dungeon.STEPOK(cr->P_CCROW, cr->P_CCCOL, dir))
        return false;

    const dodBYTE r = cr->P_CCROW + dungeon.STPTAB[dir * 2];
    const dodBYTE c = cr->P_CCCOL + dungeon.STPTAB[dir * 2 + 1];
    if (!CFIND(r, c))
        return false;

    // calculate distance to player
    const dodBYTE rd = r > player.PROW ? r - player.PROW : player.PROW - r;
    const dodBYTE cd = c > player.PCOL ? c - player.PCOL : player.PCOL - c;

    const dodBYTE big   = rd > cd ? rd : cd;
    const dodBYTE small = rd > cd ? cd : rd;

    if (big > 8 || small > 2)
    {
        cr->P_CCROW = r;
        cr->P_CCCOL = c;
        cr->P_CCDIR = dir;
        return true;
    }

    if (rng.RANDOM() & 1) // make a sound 50% of the time
    {
        // make sound
        if (g_options & OPT_STEREO)
        {
            // get x / y position of sound relative to player location
            int xpos = cr->P_CCROW - player.PROW;
            int ypos = cr->P_CCCOL - player.PCOL;

            // translate x/y position into x(thru ear axis)
            //position relative to player facing direction
            switch (player.PDIR & 3)
            {
            case 0:
                xpos = ypos;
                break;
            case 1:
                xpos = xpos;
                break;
            case 2:
                xpos = -ypos;
                break;
            case 3:
                xpos = -xpos;
                break;
            }

            // NOTE: this could be improved.
            // distant monsters pan too quickly
            // while nearby monsters pan too slowly
            // the clipping step below should clip to a smaller range
            // for close monsters and a wider range to far monsters

            // clip the pan angle to between -4 and 4
            if (xpos > 2) xpos = 2;
            else if (xpos < -2) xpos = -2;

            // convert the pan angle into a number from 0 to 255
            const int pan = 127 + xpos * 63;

            // convert the pan value into L/R volumes with a crossfading algorithm
            // like so:
            //
            // pan  ____....----''''
            //
            // panL ''''''--..______
            //
            // panR ______..--''''''

            int panr = (int)(pan * 1.5f);
            if (panr > 255) panr = 255;

            int panl = (int)(511 - (pan * 1.5f));
            if (panl > 255) panl = 255;

            // pan the sound effect before playing it
            Mix_SetPanning(creChannelv, panl, panr);
        }

        Mix_Volume(creChannelv, (MIX_MAX_VOLUME / 8) * (9 - big));
        Mix_PlayChannel(creChannelv, creSound[cr->creature_id], 0);
        while (Mix_Playing(creChannelv) == 1)
        {
            if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
            {
                scheduler.CLOCK();
            }
            scheduler.curTime = SDL_GetTicks();
        }
    }

    cr->P_CCROW = r;
    cr->P_CCCOL = c;
    cr->P_CCDIR = dir;

    return true;
}

