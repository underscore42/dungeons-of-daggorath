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
// Filename: player.cpp
//
// Implementation of Player class

#include "player.h"
#include "dodgame.h"
#include "viewer.h"
#include "sched.h"
#include "parser.h"
#include "object.h"
#include "dungeon.h"
#include "creature.h"
#include "oslink.h"
#include "enhanced.h"


extern OS_Link      oslink;
extern Creature     creature;
extern Object       object;
extern Viewer       viewer;
extern dodGame      game;
extern Scheduler    scheduler;
extern Parser       parser;
extern Dungeon      dungeon;

Player::Player() : PPOW(PLRBLK.P_ATPOW),
    PMGO(PLRBLK.P_ATMGO),
    PMGD(PLRBLK.P_ATMGD),
    PPHO(PLRBLK.P_ATPHO),
    PPHD(PLRBLK.P_ATPHD),
    PDAM(PLRBLK.P_ATDAM),
    turnDelay(20),
    moveDelay(25),
    wizDelay(500)
{
    Reset();
}

void Player::Reset()
{
    PROW = 12;
    PCOL = 22;
    PPOW = ((0x17 << 8) | 160); // = 6048
    POBJWT = 35;
    FAINT = 0;
    PRLITE = 0;
    PMLITE = 0;
    PLHAND = -1;
    PRHAND = -1;
    PTORCH = -1;
    BAGPTR = -1;
    EMPHND.obj_type = Object::OBJT_WEAPON;
    EMPHND.obj_reveal_lvl = 0;
    EMPHND.P_OCMGO = 0;
    EMPHND.P_OCPHO = 5;
    PDAM = 0;
    PDIR = 0;
    HEARTF = 0;
    HEARTC = 0;
    HEARTR = 0;
    HEARTS = 0;
    HBEATF = 0;
    turning = false;
}

void Player::LoadSounds()
{
    klink = Utils::LoadSound("12_klink.wav");
    thud = Utils::LoadSound("14_thud.wav");
    bang = Utils::LoadSound("15_bang.wav");
}

int Player::PLAYER()
{
    // Update Task's next_time
    scheduler.TCBLND[Scheduler::TID_PLAYER].next_time = scheduler.curTime +
            scheduler.TCBLND[Scheduler::TID_PLAYER].frequency;

    dodBYTE c;
    if (game.AUTFLG == 0)
    {
        // Process Keyboard Buffer
        do
        {
            c = parser.KBDGET();
            if (c == 0)
            {
                return 0;
            }
            if (FAINT != 0)
            {
                while (parser.KBDGET() != 0)
                    ;   // loop !!!
                return 0;
            }

            // Convert from ASCII to Internal Codes
            if (c >= 'A' && c <= 'Z')
            {
                c &= 0x1F;
            }
            else if (c == parser.C_BS)
            {
                c = parser.I_BS;
            }
            else if (c == parser.C_CR)
            {
                c = parser.I_CR;
            }
            else
            {
                c = parser.I_SP;
            }

            if(!HUMAN(c))
                return -1;
        }
        while (true);
    }
    else
    {
        // Process Autoplay Commands
        const int tokCnt = game.DEMO_CMDS[game.DEMOPTR++];
        if (tokCnt == 0)
        {
            game.WAIT();
            game.WAIT();
            game.hasWon = true;
            game.demoRestart = true;
            return 0;
        }

        // Feed next autoplay command to HUMAN
        dodBYTE * X, * U;
        int Xup;
        int tokCtr = 1;
        do
        {
            if (tokCtr == 1)
            {
                X = &parser.CMDTAB[game.DEMO_CMDS[game.DEMOPTR]];
            }
            else if (tokCtr == 2)
            {
                X = &parser.DIRTAB[game.DEMO_CMDS[game.DEMOPTR]];
            }
            else
            {
                X = &object.GENTAB[game.DEMO_CMDS[game.DEMOPTR]];
            }
            ++game.DEMOPTR;
            dodBYTE objstr[10];
            U = &objstr[1];
            parser.EXPAND(X, &Xup, U);
            ++U;
            game.WAIT();
            do
            {
                HUMAN(*U);
                ++U;
            }
            while (*U != 0xFF);
            HUMAN(parser.I_SP);
            ++tokCtr;
        }
        while (tokCtr <= tokCnt);
        --viewer.UPDATE;
        viewer.draw_game();
        HUMAN(parser.I_CR);
    }

    return 0;
}

bool Player::HUMAN(dodBYTE c)
{
    // Check if we are displaying the map
    if (HEARTF == 0)
    {
        game.INIVU();
        viewer.PROMPT();
    }

    if (c == parser.I_CR)
    {
carriage_return:
        viewer.OUTCHR(parser.I_SP);
        parser.LINBUF[parser.LINPTR] = Parser::I_NULL;
        parser.LINBUF[parser.LINPTR + 1] = Parser::I_NULL;
        parser.LINPTR = 0;

        if (!PreTranslateCommand(&parser.LINBUF[0]))
        {
            game.AUTFLG = true;
            game.demoRestart = true;
            return false;
        }

        // dispatch to proper routine
        dodBYTE A, B;
        const int res = parser.PARSER(&parser.CMDTAB[0], A, B, true);
        if (res == 1)
        {
            // dispatch
            switch (A)
            {
            case Parser::CMD_ATTACK:  PATTK(); break;
            case Parser::CMD_CLIMB:   PCLIMB(); break;
            case Parser::CMD_DROP:    PDROP(); break;
            case Parser::CMD_EXAMINE: PEXAM(); break;
            case Parser::CMD_GET:     PGET();  break;
            case Parser::CMD_INCANT:  PINCAN(); break;
            case Parser::CMD_LOOK:    PLOOK(); break;
            case Parser::CMD_MOVE:    PMOVE(); break;
            case Parser::CMD_PULL:    PPULL(); break;
            case Parser::CMD_REVEAL:  PREVEA(); break;
            case Parser::CMD_STOW:    PSTOW(); break;
            case Parser::CMD_TURN:    PTURN(); break;
            case Parser::CMD_USE:     PUSE();  break;
            case Parser::CMD_ZLOAD:   PZLOAD(); break;
            case Parser::CMD_ZSAVE:   PZSAVE(); break;
            }
        }
        if (res == -1)
        {
            parser.CMDERR();
        }

        if ((HEARTF != 0) && (FAINT == 0))
        {
            viewer.PROMPT();
        }

        parser.LINPTR = 0;
        return true;
    }
    if (c == parser.I_BS)
    {
        if (parser.LINPTR == 0)
        {
            return true;
        }
        --parser.LINPTR;
        viewer.OUTSTR(parser.M_ERAS);
        return true;
    }
    // Buffer normal characters
    viewer.OUTCHR(c);
    parser.LINBUF[parser.LINPTR] = c;
    ++parser.LINPTR;
    viewer.OUTSTR(parser.M_CURS);
    if (parser.LINPTR >= 32)
    {
        goto carriage_return;
    }
    return true;
}

int Player::HSLOW()
{
    PLRBLK.P_ATDAM -= (PLRBLK.P_ATDAM >> 6);
    if ((PLRBLK.P_ATDAM & 0x8000) != 0)
    {
        PLRBLK.P_ATDAM = 0;
    }
    HUPDAT();

    // Update Task's next_time
    scheduler.TCBLND[Scheduler::TID_HRTSLOW].next_time = scheduler.curTime +
            (HEARTR * 17);

    return 0;
}

void Player::HUPDAT()
{
    // Heartrate in source:
    //
    //               PPOW * 64
    // HEARTR = ------------------- - 19
    //           PPOW + (PDAM * 2)
    //
    // The original division routine added one to the integer quotient
    // so that [(1/5) == 1], [(5/5) == 2], [(10/5) == 3], etc.
    // The formula below reflects that peculiarity by only subtracting 18.
    //
    HEARTR = ((PLRBLK.P_ATPOW * 64) / (PLRBLK.P_ATPOW + PLRBLK.P_ATDAM * 2)) - 18;

    if (FAINT == 0)
    {
        // not in a faint
        if (HEARTR <= 3 || (HEARTR & 128) != 0)
        {
            // do faint
            FAINT = -1;
            viewer.clearArea(&viewer.TXTPRI);
            viewer.OLIGHT = viewer.RLIGHT;
            do
            {
                --viewer.MLIGHT;
                --viewer.UPDATE;
                viewer.draw_game();
                --viewer.RLIGHT;
                const Uint32 ticks1 = SDL_GetTicks();
                scheduler.curTime = ticks1;
                do
                {
                    if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
                    {
                        scheduler.CLOCK();
                        scheduler.EscCheck();
                    }
                    scheduler.curTime = SDL_GetTicks();
                }
                while (scheduler.curTime < ticks1 + 750);
            }
            while (viewer.RLIGHT != 248);   // not equal to -8
            --viewer.UPDATE;
            parser.KBDHDR = 0;
            parser.KBDTAL = 0;
        }
    }
    else
    {
        // in a faint
        if (HEARTR >= 4 && (HEARTR & 128) == 0)
        {
            // do recover from faint
            do
            {
                --viewer.UPDATE;
                viewer.draw_game();
                ++viewer.MLIGHT;
                ++viewer.RLIGHT;
                const Uint32 ticks1 = SDL_GetTicks();
                scheduler.curTime = ticks1;
                do
                {
                    if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
                    {
                        scheduler.CLOCK();
                        scheduler.EscCheck();
                    }
                    scheduler.curTime = SDL_GetTicks();
                }
                while (scheduler.curTime < ticks1 + 750);
            }
            while (viewer.RLIGHT != viewer.OLIGHT);
            FAINT = 0;
            viewer.PROMPT();
            --viewer.UPDATE;
        }
    }

    if (PLRBLK.P_ATPOW < PLRBLK.P_ATDAM)
    {
        // Do death
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            ; // clear event buffer
        }
        viewer.clearArea(&viewer.TXTSTS);
        viewer.clearArea(&viewer.TXTPRI);
        viewer.ShowFade(Viewer::FADE_DEATH);
    }
}

int Player::BURNER()
{
    // Update Task's next_time
    scheduler.TCBLND[Scheduler::TID_TORCHBURN].next_time =
        scheduler.curTime +
        scheduler.TCBLND[Scheduler::TID_TORCHBURN].frequency;

    if (PTORCH == -1)
        return 0;

    dodSHORT A = object.OCBLND[PTORCH].P_OCXX0;
    if (A == 0)
        return 0;

    --A;
    object.OCBLND[PTORCH].P_OCXX0 = A;

    // Convert A to minutes
    (A % 12 == 0) ? A /= 12 : A = (A / 12) + 1;

    if (!(g_cheats & CHEAT_TORCH))
    {
        if (A <= 5)
        {
            object.OCBLND[PTORCH].obj_id = Object::OBJ_TORCH_DEAD;
            object.OCBLND[PTORCH].obj_reveal_lvl = 0;
        }
        if (A < object.OCBLND[PTORCH].P_OCXX1)
        {
            object.OCBLND[PTORCH].P_OCXX1 = A;
        }
        if (A < object.OCBLND[PTORCH].P_OCXX2)
        {
            object.OCBLND[PTORCH].P_OCXX2 = A;
        }
    }

    return 0;
}

void Player::setInitialObjects(bool isDemo)
{
    int x, y;

    game.IsDemo = isDemo;
    if (isDemo)
    {
        game.LEVEL = 2;
        dungeon.SetLEVTABOrig();  //Make sure the original seeds aren't overwritten from pervious new game.
        // demo: iron sword, pine torch, leather shield

        x = object.OBIRTH(Object::OBJ_SWORD_IRON, 0);
        ++object.OCBLND[x].P_OCOWN;
        object.OCBFIL(Object::OBJ_SWORD_IRON, x);
        object.OCBLND[x].obj_reveal_lvl = 0;
        BAGPTR = x;
        y = x;

        x = object.OBIRTH(Object::OBJ_TORCH_PINE, 0);
        ++object.OCBLND[x].P_OCOWN;
        object.OCBFIL(Object::OBJ_TORCH_PINE, x);
        object.OCBLND[x].obj_reveal_lvl = 0;
        object.OCBLND[y].P_OCPTR = x;
        y = x;

        x = object.OBIRTH(Object::OBJ_SHIELD_LEATHER, 0);
        ++object.OCBLND[x].P_OCOWN;
        object.OCBFIL(Object::OBJ_SHIELD_LEATHER, x);
        object.OCBLND[x].obj_reveal_lvl = 0;
        object.OCBLND[y].P_OCPTR = x;
    }
    else
    {
        game.LEVEL = 0;
        if (!game.RandomMaze)  //Do we have an original maze?  Yes:
            dungeon.SetLEVTABOrig();  //Make sure the original seeds aren't overwritten from pervious new game.
        else  //Do we have an original maze?  No:
            dungeon.SetLEVTABRandomMap();  //Sets random seeds for maze.
        PROW = 0x10;
        PCOL = 0x0B;
        PLRBLK.P_ATPOW = 160;

        if (g_cheats & CHEAT_ITEMS)
        {
            x = object.OBIRTH(Object::OBJ_SWORD_IRON, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_SWORD_IRON, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            BAGPTR = x;
            y = x;

            x = object.OBIRTH(Object::OBJ_SHIELD_MITHRIL, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_SHIELD_MITHRIL, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            object.OCBLND[y].P_OCPTR = x;
            y = x;

            x = object.OBIRTH(Object::OBJ_SCROLL_SEER, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_SCROLL_VISION, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            object.OCBLND[y].P_OCPTR = x;
            y = x;

            x = object.OBIRTH(Object::OBJ_TORCH_LUNAR, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_TORCH_LUNAR, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            object.OCBLND[y].P_OCPTR = x;
        }
        else
        {
            x = object.OBIRTH(Object::OBJ_TORCH_PINE, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_TORCH_PINE, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            BAGPTR = x;
            y = x;

            x = object.OBIRTH(Object::OBJ_SWORD_WOOD, 0);
            ++object.OCBLND[x].P_OCOWN;
            object.OCBFIL(Object::OBJ_SWORD_WOOD, x);
            object.OCBLND[x].obj_reveal_lvl = 0;
            object.OCBLND[y].P_OCPTR = x;
        }
    }
}

void Player::PATTK()
{
    const int res = parser.PARHND();
    if (res == -1)
        return;

    const int idx = res == 0 ? PLHAND : PRHAND;

    OCB *U = idx == -1 ? &EMPHND : &object.OCBLND[idx];

    PMGO = U->P_OCMGO;
    PPHO = U->P_OCPHO;
    PDAM += (PPOW * (((int) PMGO + (int) PPHO) / 8)) >> 7;

    // make sound for appropriate object
    if (!playSound(object.objChannel, object.objSound[U->obj_type], true))
        return;

    if (U->obj_id >= Object::OBJ_RING_ENERGY && U->obj_id <= Object::OBJ_RING_FIRE)
    {
        if (!(g_cheats & CHEAT_RING))
        {
            --U->P_OCXX0;
            if (U->P_OCXX0 == 0)
            {
                U->obj_id = Object::OBJ_RING_GOLD;
                object.OCBFIL(Object::OBJ_RING_GOLD, idx);
                U->obj_reveal_lvl = 0;
                viewer.STATUS();
            }
        }
    }

    const int cidx = creature.CFIND2(RowCol(PROW, PCOL));
    if (cidx == -1)
    {
        HUPDAT();
        return;
    }

    if (!ATTACK(PLRBLK.P_ATPOW, creature.CCBLND[cidx].P_CCPOW,
                creature.CCBLND[cidx].P_CCDAM))
    {
        HUPDAT();
        return;
    }

    if (PTORCH == -1 || object.OCBLND[PTORCH].obj_id == Object::OBJ_TORCH_DEAD)
    {
        if ((rng.RANDOM() & 3) != 0)
        {
            HUPDAT();
            return;
        }
    }

    // make KLINK sound
    if (!playSound(object.objChannel, klink, true))
        return;

    viewer.OUTSTI(viewer.exps);

    // do damage
    if (DAMAGE(PLRBLK.P_ATPOW, PLRBLK.P_ATMGO, PLRBLK.P_ATPHO,
               creature.CCBLND[cidx].P_CCPOW,
               creature.CCBLND[cidx].P_CCMGD,
               creature.CCBLND[cidx].P_CCPHD,
               &creature.CCBLND[cidx].P_CCDAM) == true)
    {
        // Creature still alive
        HUPDAT();
        return;
    }

    int optr = creature.CCBLND[cidx].P_CCOBJ;
    while (optr != -1)
    {
        object.OCBLND[optr].P_OCOWN = 0;
        object.OCBLND[optr].P_OCROW = creature.CCBLND[cidx].P_CCROW;
        object.OCBLND[optr].P_OCCOL = creature.CCBLND[cidx].P_CCCOL;
        optr = object.OCBLND[optr].P_OCPTR;
    }

    --creature.CMXLND[creature.CMXPTR + creature.CCBLND[cidx].creature_id];
    creature.CCBLND[cidx].P_CCUSE = 0;
    viewer.PUPDAT();

    // do loud explosion sound
    if (!playSound(object.objChannel, bang, true))
        return;

    PPOW += (creature.CCBLND[cidx].P_CCPOW >> 3);
    if ((PPOW & 0x8000) != 0)
    {
        PPOW = 0x7FFF;
    }

    if (creature.CCBLND[cidx].creature_id == Creature::CRT_WIZIMG)
    {
        // Wizard's Image Killed
        // transport to 4th level

        // do fade in with message

        // Pause so player can see scroll
        const Uint32 ticks1 = SDL_GetTicks();
        Uint32 ticks2;
        do
        {
            ticks2 = SDL_GetTicks();
        }
        while (ticks2 < ticks1 + wizDelay);

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            ; // clear event buffer
        }
        viewer.clearArea(&viewer.TXTSTS);
        viewer.clearArea(&viewer.TXTPRI);
        viewer.done = false;
        viewer.fadeVal = -2;
        viewer.VCTFAD = 32;
        viewer.VXSCAL = 0x80;
        viewer.VYSCAL = 0x80;
        viewer.VXSCALf = 128.0f;
        viewer.VYSCALf = 128.0f;
        /*
                // Start buzz
                Mix_Volume(viewer.fadChannel, 0);
                Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);

                while (!viewer.done)
                {
                    viewer.enough_fade();
                    scheduler.EscCheck();
                }

                // Stop buzz
                Mix_HaltChannel(viewer.fadChannel);
        */
        viewer.ShowFade(Viewer::FADE_MIDDLE);

        BAGPTR = PTORCH;
        if (PTORCH != -1)
        {
            object.OCBLND[PTORCH].P_OCPTR = -1;
        }

        POBJWT = 200;
        game.LEVEL = 3;
        creature.NEWLVL();

        dodBYTE val, r, c;
        do
        {
            c = (rng.RANDOM() & 31);
            r = (rng.RANDOM() & 31);
            val = dungeon.MAZLND[dungeon.RC2IDX(r, c)];
        }
        while (val == 0xFF);

        PROW = r;
        PCOL = c;

        game.INIVU();
    }

    if (creature.CCBLND[cidx].creature_id != Creature::CRT_WIZARD)
    {
        HUPDAT();
        if (game.CreaturesInstaRegen) creature.CREGEN();  //Regen creature intantly after death if option selected.
        return;
    }

    // Killed the real Wizard
    --creature.FRZFLG;
    PRLITE = 0x07;
    PMLITE = 0x13;
    object.OCBPTR = 1;
    BAGPTR = -1;
    PTORCH = -1;
    PRHAND = -1;
    PLHAND = -1;
    game.INIVU();
    HUPDAT();
    return;
}

bool Player::ATTACK(int AP, int DP, int DD)
{
    int T0 = 15;
    int Dval = (DP - DD) * 4;

    do
    {
        Dval -= AP;
        if (Dval < 0)
        {
            break;
        }
        --T0;
    }
    while (T0 > 0);

    const int pidx = T0 - 3;
    int adjust;
    if (pidx > 0)
    {
        adjust = pidx * 10;
    }
    else
    {
        adjust = pidx * 25;
    }

    // FIXME shouldn't this calculation be done in dodBYTES?
    const int ret = rng.RANDOM() + adjust - 127;

    return ret >= 0;
}

bool Player::DAMAGE(int AP, int AMO, int APO,
                    int DP, int DMD, int DPD, dodSHORT * DD)
{
    int a;

    a = (AP * AMO) >> 7;
    a = (a  * DMD) >> 7;
    *DD += (dodSHORT) a;

    a = (AP * APO) >> 7;
    a = (a  * DPD) >> 7;
    *DD += (dodSHORT) a;

    return (dodSHORT) DP > *DD;
}

void Player::PCLIMB()
{
    const RowCol rc(PROW, PCOL);
    const dodBYTE vres = dungeon.VFIND(rc);
    if (vres == Dungeon::VF_NULL)
    {
        parser.CMDERR();
        return;
    }

    dodBYTE A, B;
    const int res = parser.PARSER(parser.DIRTAB, A, B, true);
    if (res <= 0)
    {
        parser.CMDERR();
        return;
    }

    if (A == Parser::DIR_UP)
    {
        // Climb Up
        if (vres == Dungeon::VF_LADDER_UP ||
                ((vres == Dungeon::VF_HOLE_UP) && creature.FRZFLG))// can only climb up pits after you win
        {
            showPrepareScreen();
            --game.LEVEL;
            creature.NEWLVL();
            game.INIVU();
            return;
        }
    }
    else if (A == Parser::DIR_DOWN)
    {
        // Climb Down
        if (vres == Dungeon::VF_LADDER_DOWN || vres == Dungeon::VF_HOLE_DOWN)
        {
            showPrepareScreen();
            ++game.LEVEL;
            creature.NEWLVL();
            game.INIVU();
            return;
        }
    }

    parser.CMDERR();
}

void Player::PDROP()
{
    const int res = parser.PARHND();
    if (res == -1)
    {
        return;
    }
    if (res == 0 && PLHAND == -1)
    {
        parser.CMDERR();
        return;
    }
    if (res == 1 && PRHAND == -1)
    {
        parser.CMDERR();
        return;
    }

    int idx;
    if (res == 0)
    {
        idx = PLHAND;
        PLHAND = -1;
    }
    else
    {
        idx = PRHAND;
        PRHAND = -1;
    }

    object.OCBLND[idx].P_OCOWN = 0;
    object.OCBLND[idx].P_OCROW = PROW;
    object.OCBLND[idx].P_OCCOL = PCOL;
    object.OCBLND[idx].P_OCLVL = game.LEVEL;

    POBJWT -= object.OBJWGT[object.OCBLND[idx].obj_type];
    HUPDAT();

    viewer.STATUS();
    viewer.PUPDAT();
}

void Player::PEXAM()
{
    viewer.display_mode = Viewer::MODE_EXAMINE;
    viewer.PUPDAT();
}

void Player::PGET()
{
    const int res = parser.PARHND();
    if (res == -1)
    {
        return;
    }
    if (res == 0 && PLHAND != -1)
    {
        parser.CMDERR();
        return;
    }
    if (res == 1 && PRHAND != -1)
    {
        parser.CMDERR();
        return;
    }
    if ( !object.PAROBJ() )
    {
        return;
    }

    int idx;
    bool match = false;
    object.OFINDF = 0;
    do
    {
        idx = object.OFIND(RowCol(PROW, PCOL));
        if (idx == -1)
        {
            parser.CMDERR();
            return;
        }
        if (object.SPEFLG == 0)
        {
            if (object.OBJCLS == object.OCBLND[idx].obj_type)
            {
                match = true;
            }
        }
        else
        {
            if (object.OBJTYP == object.OCBLND[idx].obj_id)
            {
                match = true;
            }
        }
    }
    while (match == false);

    if (res == 0)
    {
        PLHAND = idx;
    }
    else
    {
        PRHAND = idx;
    }
    ++object.OCBLND[idx].P_OCOWN;

    POBJWT += object.OBJWGT[object.OCBLND[idx].obj_type];
    HUPDAT();

    viewer.STATUS();
    viewer.PUPDAT();
}

void Player::PINCAN()
{
    dodBYTE A, B;
    const int res = parser.PARSER(object.ADJTAB, A, B, true);
    if (res <= 0)
        return;

    if (parser.FULFLG == 0)
        return;

    object.OBJTYP = A;
    object.OBJCLS = B;

    if (PLHAND != -1 && object.OCBLND[PLHAND].obj_type == Object::OBJT_RING)
    {
        if (handleRingIncant(PLHAND))
            return;
    }

    if (PRHAND != -1 && object.OCBLND[PRHAND].obj_type == Object::OBJT_RING)
    {
        if (handleRingIncant(PRHAND))
            return;
    }
}

void Player::PLOOK()
{
    viewer.display_mode = Viewer::MODE_3D;
    viewer.PUPDAT();
}

void Player::PMOVE()
{
    dodBYTE A, B;
    const int res = parser.PARSER(parser.DIRTAB, A, B, true);
    if (res < 0)
    {
        parser.CMDERR();
    }
    else if (res == 0)
    {
        // Move Forward
        --viewer.HLFSTP;
        viewer.PUPDAT();
        if (!delayBy(moveDelay / 2))
            return;

        viewer.HLFSTP = 0;
        PSTEP(0);
        PDAM += (POBJWT / 8) + 3;
        HUPDAT();
        --viewer.UPDATE;
        viewer.draw_game();
        delayBy(moveDelay / 2);
    }
    else if (A == Parser::DIR_BACK)
    {
        // Move Back
        --viewer.BAKSTP;
        viewer.PUPDAT();
        if (!delayBy(moveDelay / 2))
            return;
        viewer.BAKSTP = 0;
        PSTEP(2);
        PDAM += (POBJWT / 8) + 3;
        HUPDAT();
        --viewer.UPDATE;
        viewer.draw_game();
        delayBy(moveDelay / 2);
    }
    else if (A == Parser::DIR_RIGHT)
    {
        // Move Right
        if (PSTEP(1))
        {
            if (viewer.display_mode == Viewer::MODE_3D)
            {
                ShowTurn(Parser::DIR_RIGHT);
            }
        }
        PDAM += (POBJWT / 8) + 3;
        HUPDAT();
        --viewer.UPDATE;
        viewer.draw_game();
    }
    else if (A == Parser::DIR_LEFT)
    {
        // Move Left
        if (PSTEP(3))
        {
            if (viewer.display_mode == Viewer::MODE_3D)
            {
                ShowTurn(Parser::DIR_LEFT);
            }
        }
        PDAM += (POBJWT / 8) + 3;
        HUPDAT();
        --viewer.UPDATE;
        viewer.draw_game();
    }
    else
    {
        parser.CMDERR();
    }
}

void Player::PPULL()
{
    if (BAGPTR == -1)
    {
        parser.CMDERR();
        return;
    }

    const int res = parser.PARHND();
    if (res == -1)
    {
        return;
    }
    if (res == 0 && PLHAND != -1)
    {
        parser.CMDERR();
        return;
    }
    if (res == 1 && PRHAND != -1)
    {
        parser.CMDERR();
        return;
    }
    if ( !object.PAROBJ() )
    {
        return;
    }

    bool onHead = true;
    bool match = false;
    int curPtr, prevPtr;

    do
    {
        if (onHead)
        {
            curPtr = BAGPTR;
        }
        else
        {
            prevPtr = curPtr;
            curPtr = object.OCBLND[curPtr].P_OCPTR;
            if (curPtr == -1)
            {
                parser.CMDERR();
                return;
            }
        }

        if (object.SPEFLG == 0)
        {
            if (object.OCBLND[curPtr].obj_type == object.OBJCLS)
            {
                match = true;
            }
        }
        else
        {
            if (object.OCBLND[curPtr].obj_id == object.OBJTYP)
            {
                match = true;
            }
        }
        if (match)
        {
            break;
        }
        if (onHead)
        {
            onHead = false;
        }
    }
    while (true);

    if (onHead)
    {
        BAGPTR = object.OCBLND[curPtr].P_OCPTR;
    }
    else
    {
        object.OCBLND[prevPtr].P_OCPTR = object.OCBLND[curPtr].P_OCPTR;
    }

    if (res == 0)
    {
        PLHAND = curPtr;
    }
    else
    {
        PRHAND = curPtr;
    }

    if (curPtr == PTORCH)
    {
        PTORCH = -1;
    }

    viewer.STATUS();
    viewer.PUPDAT();
}

void Player::PREVEA()
{
    const int res = parser.PARHND();
    if (res == -1)
        return;

    if (res == 0 && PLHAND == -1)
        return;

    if (res == 1 && PRHAND == -1)
        return;

    const int idx = res == 0 ? PLHAND : PRHAND;

    const int req = object.OCBLND[idx].obj_reveal_lvl;
    if (req && ((req * 25 <= PPOW) || (g_cheats & CHEAT_REVEAL) || (req == 50 && game.VisionScroll && 400 <= PPOW)))
    {
        object.OCBFIL(object.OCBLND[idx].obj_id, idx);
        object.OCBLND[idx].obj_reveal_lvl = 0;
        viewer.STATUS();
    }
}

void Player::PSTOW()
{
    const int res = parser.PARHND();
    if (res == -1)
        return;

    if (res == 0 && PLHAND == -1)
    {
        parser.CMDERR();
        return;
    }
    if (res == 1 && PRHAND == -1)
    {
        parser.CMDERR();
        return;
    }

    if (res == 0)
    {
        object.OCBLND[PLHAND].P_OCPTR = BAGPTR;
        BAGPTR = PLHAND;
        PLHAND = -1;
    }
    else
    {
        object.OCBLND[PRHAND].P_OCPTR = BAGPTR;
        BAGPTR = PRHAND;
        PRHAND = -1;
    }
    viewer.STATUS();
    viewer.PUPDAT();
}

void Player::PTURN()
{
    dodBYTE A, B;
    const int res = parser.PARSER(parser.DIRTAB, A, B, true);
    if (res != 1)
    {
        parser.CMDERR();
        return;
    }
    if (A == Parser::DIR_LEFT)
    {
        // Left Turn
        --PDIR;
        PDIR = (PDIR & 3);
        if (viewer.display_mode == Viewer::MODE_3D)
        {
            ShowTurn(Parser::DIR_LEFT);
        }
        --viewer.UPDATE;
        viewer.draw_game();
        return;
    }
    else if (A == Parser::DIR_RIGHT)
    {
        // Right Turn
        ++PDIR;
        PDIR = (PDIR & 3);
        if (viewer.display_mode == Viewer::MODE_3D)
        {
            ShowTurn(Parser::DIR_RIGHT);
        }
        --viewer.UPDATE;
        viewer.draw_game();
        return;
    }
    else if (A == Parser::DIR_AROUND)
    {
        // About Face
        PDIR += 2;
        PDIR = (PDIR & 3);
        if (viewer.display_mode == Viewer::MODE_3D)
        {
            ShowTurn(Parser::DIR_AROUND);
        }
        --viewer.UPDATE;
        viewer.draw_game();
        return;
    }
    else
    {
        parser.CMDERR();
        return;
    }
}

void Player::ShowTurn(dodBYTE A)
{
    const int inc = 32;
    const int lines = 8;
    const int y0 = 17;
    const int y1 = 135;

    int times, offset, dir;
    switch (A)
    {
    case Parser::DIR_LEFT:
        offset = 8;
        dir = 1;
        times = 1;
        break;
    case Parser::DIR_RIGHT:
        offset = 248;
        dir = -1;
        times = 1;
        break;
    case Parser::DIR_AROUND:
        offset = 248;
        dir = -1;
        times = 2;
        break;
    default:
        break;
    }

    viewer.VXSCAL = 0x80;
    viewer.VYSCAL = 0x80;
    viewer.VXSCALf = 128.0f;
    viewer.VYSCALf = 128.0f;
    viewer.RANGE = 0;
    viewer.SETFAD();
    glColor3fv(viewer.fgColor);

    bool redraw = true;
    turning = true;
    for (int ctr = 0; ctr < times; ++ctr)
    {
        for (int x = 0; x < lines; ++x)
        {
            const Uint32 ticks1 = SDL_GetTicks();
            do
            {
                scheduler.curTime = SDL_GetTicks();
                if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
                {
                    scheduler.CLOCK();
                    if (game.AUTFLG && game.demoRestart == false)
                    {
                        turning = false;
                        return;
                    }
                    redraw = true;
                }

                if (redraw)
                {
                    glClear(GL_COLOR_BUFFER_BIT);
                    glLoadIdentity();
                    viewer.drawVectorList(viewer.LINES);
                    viewer.drawVector((x * inc * dir) + offset, y0, (x * inc * dir) + offset, y1);
                    viewer.drawArea(&viewer.TXTSTS);
                    viewer.drawArea(&viewer.TXTPRI);
                    SDL_GL_SwapWindow(oslink.window);
                    redraw = false;
                }
            }
            while (scheduler.curTime < ticks1 + turnDelay);
        }
    }
    turning = false;
    --HEARTF;
}

void Player::PUSE()
{
    const int res = parser.PARHND();
    if (res == -1)
        return;
    if (res == 0 && PLHAND == -1)
        return;
    if (res == 1 && PRHAND == -1)
        return;

    const int idx = res == 0 ? PLHAND : PRHAND;

    if (object.OCBLND[idx].obj_type  == Object::OBJT_TORCH)
    {
        PTORCH = idx;
        if (res == 0)
        {
            object.OCBLND[PLHAND].P_OCPTR = BAGPTR;
            BAGPTR = PLHAND;
            PLHAND = -1;
        }
        else
        {
            object.OCBLND[PRHAND].P_OCPTR = BAGPTR;
            BAGPTR = PRHAND;
            PRHAND = -1;
        }
        viewer.STATUS();
        viewer.PUPDAT();

        // make torch sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        viewer.PUPDAT();
    }
    else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_THEWS)
    {
        PPOW += 1000;
        object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
        object.OCBLND[idx].obj_reveal_lvl = 0;

        // make flask sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        viewer.STATUS();
        HUPDAT();
    }
    else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_HALE) // Hale Flask
    {
        PDAM = 0;
        object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
        object.OCBLND[idx].obj_reveal_lvl = 0;

        // make flask sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        viewer.STATUS();
        HUPDAT();
    }
    else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_ABYE) // Abye Flask
    {
        if (!(g_cheats & CHEAT_INVULNERABLE))
            PDAM += (short) ((double) PPOW * 0.8);

        object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
        object.OCBLND[idx].obj_reveal_lvl = 0;

        // make flask sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        viewer.STATUS();
        HUPDAT();
    }
    else if (object.OCBLND[idx].obj_id  == Object::OBJ_SCROLL_SEER)
    {
        viewer.showSeerMap = true;
        if (object.OCBLND[idx].obj_reveal_lvl != 0)
        {
            return;
        }

        // make scroll sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        HEARTF = 0;
        viewer.display_mode = Viewer::MODE_MAP;
        viewer.PUPDAT();
    }
    else if (object.OCBLND[idx].obj_id  == Object::OBJ_SCROLL_VISION)
    {
        viewer.showSeerMap = false;
        if (object.OCBLND[idx].obj_reveal_lvl != 0)
        {
            return;
        }

        // make scroll sound
        playSound(object.objChannel, object.objSound[object.OCBLND[idx].obj_type]);

        HEARTF = 0;
        viewer.display_mode = Viewer::MODE_MAP;
        viewer.PUPDAT();
    }
}

void Player::PZLOAD()
{
    oslink.buildSaveGamePath();

    // this sets the flag to 255 to signal that we should load
    --scheduler.ZFLAG;
}

void Player::PZSAVE()
{
    oslink.buildSaveGamePath();

    // this sets the flag to 1 to signal that we should save
    ++scheduler.ZFLAG;
}

bool Player::PSTEP(dodBYTE dir)
{
    dodBYTE B = dir + PDIR;
    B &= 3;
    if (dungeon.STEPOK(PROW, PCOL, B))
    {
        PROW += dungeon.STPTAB[B * 2];
        PCOL += dungeon.STPTAB[B * 2 + 1];
        return true;
    }
    else
    {
        // do thud sound
        playSound(object.objChannel, thud);
        return false;
    }
}

void Player::showPrepareScreen() const
{
    viewer.displayPrepare();
    const dodSHORT temp = viewer.display_mode;
    viewer.display_mode = Viewer::MODE_TITLE;
    viewer.draw_game();
    const Uint32 ticks1 = SDL_GetTicks();
    scheduler.curTime = ticks1;
    do
    {
        if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
        {
            scheduler.CLOCK();
        }
        scheduler.curTime = SDL_GetTicks();
    }
    while (scheduler.curTime < ticks1 + viewer.prepPause);
    viewer.display_mode = temp;
}

bool Player::handleRingIncant(int hand) const
{
    if (object.OCBLND[hand].P_OCXX1 != object.OBJTYP)
        return false;

    object.OCBLND[hand].obj_id = object.OBJTYP;
    object.OCBFIL(object.OBJTYP, hand);

    // make ring sound
    playSound(object.objChannel, object.objSound[object.OCBLND[hand].obj_type]);

    viewer.STATUS();
    viewer.PUPDAT();
    object.OCBLND[hand].P_OCXX1 = -1;
    if (object.OBJTYP == Object::OBJ_RING_FINAL)
    {
        // winner
        SDL_Event event;
        while(SDL_PollEvent(&event))
            ; // clear event buffer

        // Pause so player can see status line
        const Uint32 ticks1 = SDL_GetTicks();
        Uint32 ticks2;
        do
        {
            ticks2 = SDL_GetTicks();
        }
        while (ticks2 < ticks1 + wizDelay);

        viewer.clearArea(&viewer.TXTSTS);
        viewer.clearArea(&viewer.TXTPRI);
        viewer.ShowFade(Viewer::FADE_VICTORY);
        game.hasWon = true;
    }
    else
        return true;

    return false;
}

bool Player::delayBy(Uint32 duration) const
{
    const Uint32 ticks1 = SDL_GetTicks();
    scheduler.curTime = ticks1;
    do
    {
        if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
        {
            scheduler.CLOCK();
            if (game.AUTFLG && game.demoRestart == false)
                return false;
        }
        scheduler.curTime = SDL_GetTicks();
    }
    while (scheduler.curTime < ticks1 + duration);

    return true;
}

bool Player::playSound(int channel, Mix_Chunk* chunk, bool checkForDemoAbort) const
{
    Mix_PlayChannel(channel, chunk, 0);
    while (Mix_Playing(channel) == 1)
    {
        if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
        {
            scheduler.CLOCK();
            if (checkForDemoAbort && game.AUTFLG && game.demoRestart == false)
                return false;
        }
        scheduler.curTime = SDL_GetTicks();
    }

    return true;
}
