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
// Filename: sched.h
//
// This class manages the scheduler.

#ifndef DOD_SCHEDULER_HEADER
#define DOD_SCHEDULER_HEADER

#include "dod.h"

struct Mix_Chunk;
struct SDL_Keysym;

class Scheduler
{
public:
    // Constructor
    Scheduler();

    // Public Interface
    void        SYSTCB();
    void        SCHED();
    void        CLOCK();
    int         GETTCB();
    bool        keyCheck();
    bool        keyHandler(SDL_Keysym * keysym);
    void        Reset();
    void        SAVE();
    void        LOAD();
    void        LoadSounds();
    bool        EscCheck();
    bool        EscHandler(SDL_Keysym * keysym);
    void        pause(bool state);
    void        updateCreatureRegen(int newTime);

    // Public Data Fields
    Task    TCBLND[38];

    enum   // task IDs
    {
        TID_CLOCK       = 0,
        TID_PLAYER      = 1,
        TID_REFRESH_DISP = 2,
        TID_HRTSLOW     = 3,
        TID_TORCHBURN   = 4,
        TID_CRTREGEN    = 5,
        TID_CRTMOVE     = 6,
    };

    Uint32      curTime;
    Uint32      elapsedTime;

    Mix_Chunk * hrtSound[2];
    static constexpr int hrtChannel = 0;

    dodBYTE     ZFLAG;

private:
    int TCBPTR;

    // disk error string
    dodBYTE DERR[15];
};

#endif // DOD_SCHEDULER_HEADER
