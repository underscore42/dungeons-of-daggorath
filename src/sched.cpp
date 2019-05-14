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
// Filename: sched.cpp
//
// Implementation of the Scheduler class

#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace std;

#include "sched.h"
#include "player.h"
#include "viewer.h"
#include "oslink.h"
#include "creature.h"
#include "parser.h"
#include "dodgame.h"
#include "dungeon.h"
#include "object.h"

extern Object   object;
extern RNG      rng;
extern Dungeon  dungeon;
extern dodGame  game;
extern Creature creature;
extern Player   player;
extern Viewer   viewer;
extern OS_Link  oslink;

Scheduler::Scheduler()
{
    Reset();

    // DISK ERROR !!!
    Utils::LoadFromHex(DERR, "69133580B293E40DEF60");
}

void Scheduler::Reset()
{
    curTime = 0;
    elapsedTime = 0;
    TCBPTR = 0;
    ZFLAG = 0;

    for (int ctr = 0; ctr < 38; ++ctr)
        TCBLND[ctr].clear();
}

void Scheduler::LoadSounds()
{
    hrtSound[0] = Utils::LoadSound("17_heart.wav");
    hrtSound[1] = Utils::LoadSound("18_heart.wav");
}

void Scheduler::SYSTCB()
{
    for (int ctr = 0; ctr < 38; ++ctr)
    {
        TCBLND[ctr].clear();
    }

    TCBPTR = 0;

    TCBLND[0].type = TID_CLOCK;
    TCBLND[0].frequency = 17;       // One JIFFY
    GETTCB();

    TCBLND[1].type = TID_PLAYER;
    TCBLND[1].frequency = 17;       // One JIFFY
    GETTCB();

    TCBLND[2].type = TID_REFRESH_DISP;
    TCBLND[2].frequency = 300;      // Three TENTHs
    GETTCB();

    TCBLND[3].type = TID_HRTSLOW;
    GETTCB();

    TCBLND[4].type = TID_TORCHBURN;
    TCBLND[4].frequency = 5000;     // Five Seconds
    GETTCB();

    TCBLND[5].type = TID_CRTREGEN;
    TCBLND[5].frequency = 300000;   // Five minutes
    GETTCB();

    TCBPTR = 6; // point to end of task blocks
}

void Scheduler::SCHED()
{
    const Uint32 targetFrameTime = 1000 / 60; // 60 FPS

    // Set next_time to a sane value, this also helps to keep repeated intros
    // in sync.
    curTime = SDL_GetTicks();
    for (int i = 0; i < TCBPTR; ++i)
        TCBLND[i].next_time = curTime + TCBLND[i].frequency;

    // Main game execution loop
    for (;;)
    {
        curTime = SDL_GetTicks();

        for (int ctr = 0; ctr < TCBPTR; ++ctr)
        {
            if (curTime >= TCBLND[ctr].next_time)
            {
                switch (TCBLND[ctr].type)
                {
                case TID_CLOCK:        CLOCK();           break;
                case TID_PLAYER:       player.PLAYER();   break;
                case TID_REFRESH_DISP: viewer.LUKNEW();   break;
                case TID_HRTSLOW:      player.HSLOW();    break;
                case TID_TORCHBURN:    player.BURNER();   break;
                case TID_CRTREGEN:     creature.CREGEN(); break;
                case TID_CRTMOVE:
                    creature.CMOVE(ctr, TCBLND[ctr].data);
                    break;
                default:
                    break;
                }
            }
        }

        if (ZFLAG != 0) // Saving or Loading
        {
            if (ZFLAG == 0xFF)
            {
                return; // Load game abandons current game
            }
            else
            {
                SAVE();
                ZFLAG = 0;
            }
        }

        if (player.PLRBLK.P_ATPOW < player.PLRBLK.P_ATDAM)
        {
            return; // Death
        }

        if (game.hasWon)
        {
            return; // Victory
        }

        // Limit FPS
        if ((SDL_GetTicks() - curTime) < targetFrameTime)
            SDL_Delay(targetFrameTime - (SDL_GetTicks() - curTime));
    };
}

void Scheduler::CLOCK()
{
    // Update elapsed time
    elapsedTime = curTime - TCBLND[0].prev_time;

    // Reality check
    if (elapsedTime > 126 * 17)
    {
        elapsedTime = 126 * 17;
    }

    if (elapsedTime >= 17)
    {
        // Update Task's next_time
        TCBLND[0].next_time = curTime + TCBLND[0].frequency;

        // Update Task's prev_time
        TCBLND[0].prev_time = curTime;
        if (player.HBEATF != 0)
        {
            player.HEARTC -= (elapsedTime / 17);
            if ((player.HEARTC & 0x80) != 0)
            {
                player.HEARTC = 0;
            }
            if (player.HEARTC == 0)
            {
                player.HEARTC = player.HEARTR;

                // make sound
                Mix_PlayChannel(hrtChannel, hrtSound[(dodBYTE) (player.HEARTS + 1)], 0);
                while (Mix_Playing(hrtChannel) == 1) ; // !!!

                if (player.HEARTF != 0)
                {
                    if ((player.HEARTS & 0x80) != 0)
                    {
                        // small
                        viewer.statArea[15] = '<';
                        viewer.statArea[16] = '>';
                        player.HEARTS = 0;
                    }
                    else
                    {
                        // large
                        viewer.statArea[15] = '{';
                        viewer.statArea[16] = '}';
                        player.HEARTS = -1;
                    }
                    if (!player.turning)
                    {
                        --viewer.UPDATE;
                        viewer.draw_game();
                    }
                }
            }
        }
    }

    if (player.FAINT == 0)
    {
        if (game.AUTFLG)
        {
            // Abort demo on keypress
            if (keyCheck())
            {
                game.hasWon = true;
                game.demoRestart = false;
            }
        }
        else
        {
            // Perform Keyboard Input
            oslink.process_events();
        }
    }
}

bool Scheduler::keyCheck()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            return ( keyHandler(&event.key.keysym) );
            break;
        case SDL_QUIT:
            oslink.quitSDL(0); // eventually change to meta-menu
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                SDL_GL_SwapWindow(oslink.window);
            break;
        }
    }
    return false;
}

bool Scheduler::keyHandler(SDL_Keysym * keysym)
{
    switch(keysym->scancode)
    {
    case SDL_SCANCODE_ESCAPE:
    {
        Mix_HaltChannel(viewer.fadChannel);

        const bool rc = oslink.main_menu();  // calls the meta-menu

        Mix_Volume(viewer.fadChannel, 0);
        Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
        return rc;
    }
    default:
        return true;
    }
}

void Scheduler::SAVE()
{
    ofstream    fout;
    int         ctr;
    char        outstr[64];

    fout.open(oslink.gamefile, ios::trunc);
    if (!fout)
    {
        // DISK ERROR
        viewer.OUTSTI(DERR);
        viewer.PROMPT();
        return;
    }

    sprintf(outstr, "%d", game.LEVEL);
    fout << outstr << endl;
    sprintf(outstr, "%d", dungeon.VFTPTR);
    fout << outstr << endl;

    for (ctr = 0; ctr < 1024; ++ctr)
    {
        sprintf(outstr, "%d", dungeon.MAZLND[ctr]);
        fout << outstr << endl;
    }

    sprintf(outstr, "%d", player.PROW);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PCOL);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.POBJWT);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PPOW);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PLHAND);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PRHAND);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PDAM);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PDIR);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PTORCH);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PRLITE);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.PMLITE);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.FAINT);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.BAGPTR);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.HEARTF);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.HEARTC);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.HEARTR);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.HEARTS);
    fout << outstr << endl;
    sprintf(outstr, "%d", player.HBEATF);
    fout << outstr << endl;
    sprintf(outstr, "%d", rng.SEED[0]);
    fout << outstr << endl;
    sprintf(outstr, "%d", rng.SEED[1]);
    fout << outstr << endl;
    sprintf(outstr, "%d", rng.SEED[2]);
    fout << outstr << endl;
    sprintf(outstr, "%d", rng.carry);
    fout << outstr << endl;
    sprintf(outstr, "%d", creature.FRZFLG);
    fout << outstr << endl;
    sprintf(outstr, "%d", creature.CMXPTR);
    fout << outstr << endl;

    for (ctr = 0; ctr < 60; ++ctr)
    {
        sprintf(outstr, "%d", creature.CMXLND[ctr]);
        fout << outstr << endl;
    }

    for (ctr = 0; ctr < 32; ++ctr)
    {
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPOW);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCMGO);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCMGD);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPHO);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPHD);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCTMV);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCTAT);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCOBJ);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCDAM);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCUSE);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].creature_id);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCDIR);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCROW);
        fout << outstr << endl;
        sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCCOL);
        fout << outstr << endl;
    }

    const int OFINDF = 0;
    sprintf(outstr, "%d", OFINDF);
    fout << outstr << endl;
    sprintf(outstr, "%d", object.OCBPTR);
    fout << outstr << endl;
    sprintf(outstr, "%d", object.OFINDP);
    fout << outstr << endl;

    for (ctr = 0; ctr < 72; ++ctr)
    {
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCPTR);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCROW);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCCOL);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCLVL);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCOWN);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX0);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX1);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX2);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].obj_id);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].obj_type);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].obj_reveal_lvl);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCMGO);
        fout << outstr << endl;
        sprintf(outstr, "%d", object.OCBLND[ctr].P_OCPHO);
        fout << outstr << endl;
    }

    sprintf(outstr, "%d", viewer.RLIGHT);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.MLIGHT);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.OLIGHT);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.VXSCAL);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.VYSCAL);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.TXBFLG);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.tcaret);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.tlen);
    fout << outstr << endl;
    sprintf(outstr, "%d", viewer.NEWLIN);
    fout << outstr << endl;

    //Save current game levels' rnd seeds, as they are no longer hard-coded.
    for (ctr = 0; ctr <= 6; ++ctr)
    {
        sprintf(outstr, "%d", dungeon.LEVTAB[ctr]);
        fout << outstr << endl;
    }
    //Save vertical features table, as they are no longer hard-coded.

    for (ctr = 0; ctr <= 41; ++ctr)
    {
        sprintf(outstr, "%d", dungeon.VFTTAB[ctr]);
        fout << outstr << endl;
    }
    sprintf(outstr, "%d", game.RandomMaze);
    fout << outstr << endl;
    sprintf(outstr, "%d", game.ShieldFix);
    fout << outstr << endl;
    sprintf(outstr, "%d", game.VisionScroll);
    fout << outstr << endl;
    sprintf(outstr, "%d", game.CreaturesIgnoreObjects);
    fout << outstr << endl;
    sprintf(outstr, "%d", game.CreaturesInstaRegen);
    fout << outstr << endl;
    sprintf(outstr, "%d", game.MarkDoorsOnScrollMaps);
    fout << outstr << endl;

    fout.close();
}

void Scheduler::LOAD()
{
    ifstream    fin;
    int         ctr, in;
    char        instr[64];

    fin.open(oslink.gamefile);
    if (!fin)
    {
        // DISK ERROR
        viewer.OUTSTI(DERR);
        viewer.PROMPT();
        return;
    }

    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) game.LEVEL = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) dungeon.VFTPTR = in;

    for (ctr = 0; ctr < 1024; ++ctr)
    {
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) dungeon.MAZLND[ctr] = in;
    }

    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PROW = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PCOL = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.POBJWT = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PPOW = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PLHAND = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PRHAND = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PDAM = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PDIR = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PTORCH = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PRLITE = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.PMLITE = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.FAINT = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.BAGPTR = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.HEARTF = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.HEARTC = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.HEARTR = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.HEARTS = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) player.HBEATF = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) rng.SEED[0] = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) rng.SEED[1] = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) rng.SEED[2] = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) rng.carry = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) creature.FRZFLG = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) creature.CMXPTR = in;

    for (ctr = 0; ctr < 60; ++ctr)
    {
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CMXLND[ctr] = in;
    }

    for (ctr = 0; ctr < 32; ++ctr)
    {
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCPOW = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCMGO = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCMGD = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCPHO = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCPHD = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCTMV = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCTAT = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCOBJ = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCDAM = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCUSE = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].creature_id = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCDIR = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCROW = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) creature.CCBLND[ctr].P_CCCOL = in;
    }

    int OFINDF;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) OFINDF = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) object.OCBPTR = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) object.OFINDP = in;

    for (ctr = 0; ctr < 72; ++ctr)
    {
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCPTR = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCROW = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCCOL = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCLVL = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCOWN = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCXX0 = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCXX1 = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCXX2 = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].obj_id = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].obj_type = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].obj_reveal_lvl = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCMGO = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) object.OCBLND[ctr].P_OCPHO = in;
    }

    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.RLIGHT = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.MLIGHT = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.OLIGHT = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.VXSCAL = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.VYSCAL = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.TXBFLG = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.tcaret = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.tlen = in;
    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) viewer.NEWLIN = in;
    //Original save games ended here.

    fin >> instr;
    if (1 == sscanf(instr, "%d", &in)) //Do we have more data to load?  Yes:
    {
        //Load current game levels' rnd seeds, as they are no longer hard-coded.
        dungeon.LEVTAB[0] = in;
        for (ctr = 1; ctr <= 6; ++ctr)
        {
            fin >> instr;
            if (1 == sscanf(instr, "%d", &in)) dungeon.LEVTAB[ctr] = in;
        }

        //Load vertical features table, as they are no longer hard-coded.
        for (ctr = 0; ctr <= 41; ++ctr)
        {
            fin >> instr;
            if (1 == sscanf(instr, "%d", &in)) dungeon.VFTTAB[ctr] = in;
        }
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.RandomMaze = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.ShieldFix = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.VisionScroll = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.CreaturesIgnoreObjects = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.CreaturesInstaRegen = in;
        fin >> instr;
        if (1 == sscanf(instr, "%d", &in)) game.MarkDoorsOnScrollMaps = in;
    }
    else      //Do we have more data to load?  No:
    {
        //Old save game.  Must be old save with original map.
        //Put in original rnd seeds & vertical features table.
        dungeon.SetLEVTABOrig();
        dungeon.SetVFTTABOrig();
        game.RandomMaze = false;
        game.ShieldFix = false;
        game.VisionScroll = false;
        game.CreaturesIgnoreObjects = false;
        game.CreaturesInstaRegen = false;
        game.MarkDoorsOnScrollMaps = false;
    }  //Do we have more data to load?

    fin.close();

}

void Scheduler::pause(bool state)
{
    // The current pause state of the game
    static bool   curState = false;
    static Uint32 savedTime;

    if(!curState && state)
    {
        savedTime = curTime;
    }
    else if(curState && !state)
    {
        curTime = SDL_GetTicks();

        for(int i = 0; i < TCBPTR; i++)
        {
            TCBLND[i].next_time += (curTime - savedTime);
            TCBLND[i].prev_time += (curTime - savedTime);
        }
    }
}
