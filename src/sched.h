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
    Scheduler();

    // Resets values of this class to their defaults
    void Reset();

    // Load sound files used by the scheduler
    void LoadSounds();

    // Creates initial Task Blocks
    void SYSTCB();

    // This is the Main Loop of the game. Originally it was a port of the
    // scheduling algorithm used in the source code, but it has been entirely
    // replaced with a simpler algorithm that works better on modern platforms.
    //
    // It uses milliseconds instead of JIFFYs. And it does not have any queues,
    // but a simple array of Task objects.
    void SCHED();

    // This is the heart of the game, literally. It manages the heartbeat, calls
    // for the screen to be redrawn, and polls the OS for key strokes.
    //
    // This routine is called 60 times per second from the scheduler, and also
    // from the blocking loops after each sound, which allows the heartbeat to
    // mix in with the other sounds.
    void CLOCK();

    // Gets next available Task Block and updates the index
    int GETTCB() { ++TCBPTR; return (TCBPTR - 1); }

    // Used by wizard fade in/out function
    bool keyCheck();

    // Used by wizard fade in/out function
    bool keyHandler(SDL_Keysym * keysym);

    // Save game
    void SAVE();

    // Load game
    void LOAD();

    // Vars: state - Used to toggle current pause state
    //             - true - pause the game, false - unpause the game
    // Note: Only saves state of TCBs, no actual sidestepping of execution
    void pause(bool state);

    // Vars: newTime - The new creature regen time (in minutes)
    // Note: This takes effect after the next creature regen
    void updateCreatureRegen(int newTime)
    {
        TCBLND[5].frequency = 60000 * newTime;
    }

    // all tasks
    Task TCBLND[38];

    enum   // task IDs
    {
        TID_CLOCK        = 0,
        TID_PLAYER       = 1,
        TID_REFRESH_DISP = 2,
        TID_HRTSLOW      = 3,
        TID_TORCHBURN    = 4,
        TID_CRTREGEN     = 5,
        TID_CRTMOVE      = 6,
    };

    Uint32 curTime;
    Uint32 elapsedTime;

    Mix_Chunk * hrtSound[2];
    static constexpr int hrtChannel = 0;

    // indicates if we should save or load a game
    dodBYTE ZFLAG;

private:
    // pointer to end of task blocks
    int TCBPTR;

    // disk error string
    dodBYTE DERR[10];
};

#endif // DOD_SCHEDULER_HEADER
