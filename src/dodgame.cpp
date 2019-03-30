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
// Filename: dodgame.cpp
//
// Implementation of dodGame class.

#include "dodgame.h"
#include "player.h"
#include "object.h"
#include "viewer.h"
#include "sched.h"
#include "creature.h"
#include "parser.h"
#include "dungeon.h"
#include "oslink.h"

extern OS_Link      oslink;
extern Dungeon      dungeon;
extern Parser       parser;
extern Creature     creature;
extern Object       object;
extern Player       player;
extern Viewer       viewer;
extern Scheduler    scheduler;

#define D_EXAMINE       "010f"
#define D_PULL_RIGHT    "032605"
#define D_TORCH         "19"
#define D_USE_RIGHT     "023705"
#define D_LOOK          "011e"
#define D_MOVE          "0122"
#define D_PULL_LEFT     "032601"
#define D_SHIELD        "0f"
#define D_SWORD         "14"
#define D_ATTACK_RIGHT  "020105"
#define D_TURN_RIGHT    "023305"
#define D_END           "00"

dodGame::dodGame() : LEVEL(2), AUTFLG(true), hasWon(false),
    demoRestart(true), DEMOPTR(0)
{
    Utils::LoadFromHex(DEMO_CMDS,
                       D_EXAMINE
                       D_PULL_RIGHT D_TORCH
                       D_USE_RIGHT
                       D_LOOK
                       D_MOVE
                       D_PULL_LEFT  D_SHIELD
                       D_PULL_RIGHT D_SWORD
                       D_MOVE D_MOVE
                       D_ATTACK_RIGHT
                       D_TURN_RIGHT D_MOVE D_MOVE D_MOVE
                       D_TURN_RIGHT D_MOVE D_MOVE
                       D_END
                      );
}

void dodGame::COMINI()
{
    Uint32 ticks1, ticks2;

    scheduler.SYSTCB();
    object.CreateAll();
    player.HBEATF = 0;
    viewer.clearArea(&viewer.TXTSTS);
    viewer.clearArea(&viewer.TXTPRI);
    viewer.VXSCAL = 0x80;
    viewer.VYSCAL = 0x80;
    viewer.VXSCALf = 128.0f;
    viewer.VYSCALf = 128.0f;
    AUTFLG = viewer.ShowFade(Viewer::FADE_BEGIN);
    player.setInitialObjects(AUTFLG);
    viewer.displayPrepare();
    viewer.display_mode = Viewer::MODE_TITLE;
    viewer.draw_game();

    // Delay with "PREPARE!" on screen
    ticks1 = SDL_GetTicks();
    do
    {
        oslink.process_events();
        ticks2 = SDL_GetTicks();
    }
    while (ticks2 < ticks1 + viewer.prepPause);

    creature.NEWLVL();
    if (AUTFLG)
    {
        // do map
        viewer.display_mode = Viewer::MODE_MAP;
        viewer.showSeerMap = true;
        --viewer.UPDATE;
        viewer.draw_game();
        // wait 3 seconds
        ticks1 = SDL_GetTicks();
        do
        {
            oslink.process_events();
            ticks2 = SDL_GetTicks();
        }
        while (ticks2 < ticks1 + 3000);
    }
    INIVU();
    viewer.PROMPT();
}

void dodGame::Restart()
{
    object.Reset();
    creature.Reset();
    parser.Reset();
    player.Reset();
    scheduler.Reset();
    viewer.Reset();
    hasWon = false;

    dungeon.VFTPTR = 0;
    scheduler.SYSTCB();
    object.CreateAll();
    player.HBEATF = 0;
    player.setInitialObjects(false);
    viewer.displayPrepare();
    viewer.displayCopyright();
    viewer.display_mode = Viewer::MODE_TITLE;
    viewer.draw_game();

    // Delay with "PREPARE!" on screen
    const Uint32 ticks1 = SDL_GetTicks();
    Uint32 ticks2;
    do
    {
        oslink.process_events();
        ticks2 = SDL_GetTicks();
    }
    while (ticks2 < ticks1 + 2500);

    creature.NEWLVL();
    INIVU();
    viewer.PROMPT();
}

void dodGame::LoadGame()
{
    scheduler.LOAD();
    viewer.setVidInv((LEVEL % 2) ? true : false);
    --viewer.UPDATE;
    viewer.draw_game();
    INIVU();
    viewer.PROMPT();
}

void dodGame::INIVU()
{
    viewer.clearArea(&viewer.TXTSTS);
    viewer.clearArea(&viewer.TXTPRI);
    player.HUPDAT();
    ++player.HEARTC;
    --player.HEARTF;
    --player.HBEATF;
    viewer.STATUS();
    player.PLOOK();
}

void dodGame::WAIT()
{
    const Uint32 ticks1 = SDL_GetTicks();
    scheduler.curTime = ticks1;

    do
    {
        if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
        {
            scheduler.CLOCK();
            if (AUTFLG && demoRestart == false)
            {
                return;
            }
            scheduler.EscCheck();
        }
        scheduler.curTime = SDL_GetTicks();
    }
    while (scheduler.curTime < ticks1 + 1350);
}
