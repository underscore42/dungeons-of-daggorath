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
// Filename: player.h
//
// This class manages the Player data

#ifndef DOD_PLAYER_HEADER
#define DOD_PLAYER_HEADER

#include "dod.h"

class Player
{
public:
    Player();

    void Reset();
    void LoadSounds();

    // This method gets called from the scheduler as often as possible. It
    // retrieves keyboard input, or commands from the demo data.
    int PLAYER();

    // Handle a character that was typed in at the prompt.
    bool HUMAN(dodBYTE c);

    // This method gets called from the scheduler with a frequency equal to the
    // current heart rate. It performs damage recovery, indicated by slowing the
    // heartbeat.
    int HSLOW();

    // This method is called every five seconds. It will manage the lit torch's
    // timers.
    //
    // The full burn time is stored in XX0 in 5-second units, so 15 minutes
    // equals 180 5-second units.
    //
    // The magical light values and physical light values are stored in minutes.
    // Hence the conversion half way through.
    //
    // This is a hack to give the main timer more granularity, so that it
    // doesn't lose a whole minute on each level change, but I had to leave the
    // other values as minutes because they are used elsewhere in a complicated
    // formula to determine the ligthing (line pixelation/fade) values for the
    // 3D viewer.
    int BURNER();

    // Calculates heartbeat based on power level and damage.
    // Also processes fainting and death.
    void HUPDAT();

    // Used during initialization to set data for either the built-in demo or
    // starting a live game
    void setInitialObjects(bool isGame);

    // Processes ATTACK command
    void PATTK();

    // Processes CLIMB command
    void PCLIMB();

    // Processes DROP command
    void PDROP();

    // Processes EXAMINE command
    void PEXAM();

    // Processes GET command
    void PGET();

    // Processes INCANT command
    void PINCAN();

    // Processes LOOK command
    void PLOOK();

    // Processes MOVE command
    void PMOVE();

    // Processes PULL command
    void PPULL();

    // Processes REVEAL command
    void PREVEA();

    // Processes STOW command
    void PSTOW();

    // Processes TURN command
    void PTURN();

    // Processes USE command
    void PUSE();

    // Processes ZLOAD command
    // actual load happens in Scheduler::LOAD()
    void PZLOAD();

    // Processes ZSAVE command
    void PZSAVE();

    // Attempts to move player in given direction
    bool PSTEP(dodBYTE dir);

    // Determines if an attack strikes its target
    // AP = Player attack power, DP = creature attack power, dd = creature damage
    bool ATTACK(int AP, int DP, int DD) const;

    // Calculates and assesses damage from a successful attack
    bool DAMAGE(int AP, int AMO, int APO,
                int DP, int DMD, int DPD, dodSHORT * DD);

    // Turning Animation
    void ShowTurn(dodBYTE A);

    // Player position
    dodBYTE PROW;
    dodBYTE PCOL;

    // Total weight of things that the player carries
    dodSHORT POBJWT;

    // Player attack block
    ATB PLRBLK;

    // These reference into PLRBLK
    dodSHORT& PPOW;
    dodBYTE&  PMGO;
    dodBYTE&  PMGD;
    dodBYTE&  PPHO;
    dodBYTE&  PPHD;
    dodSHORT& PDAM;

    // Pointers to the objects the player is carrying
    int PLHAND;
    int PRHAND;

    // Direction the player is facing
    dodBYTE PDIR;

    // Pointer to the torch the player is using
    int PTORCH;

    // Inherent regular light of player (without a torch)
    dodBYTE PRLITE;

    // Inherent magical light of player (without a torch)
    dodBYTE PMLITE;

    // If the player has fainted (0=false, 255=true)
    dodBYTE FAINT;

    // Points to the objects in the player's bag
    int BAGPTR;

    // Heart beat flag (0=off, 255=on)
    dodBYTE HEARTF;

    // number of jiffies until next heartbeat
    dodBYTE HEARTC;

    // Current heart rate (in number of jiffies)
    dodBYTE HEARTR;

    // Which heart size to show (0=big, 255=small)
    dodBYTE HEARTS;

    // If we should play the heart beat sound (0=false, 255=true)
    dodBYTE HBEATF;

    // Empty hand can be still be used to attack
    OCB EMPHND;

    int turnDelay;
    int moveDelay;
    int wizDelay;

    // If we are in the TURN animation
    bool turning;

    Mix_Chunk * klink;
    Mix_Chunk * thud;
    Mix_Chunk * bang;

private:

    // Used when climbing up or down a level.
    void showPrepareScreen() const;

    // Returns true if the game was not won by incanting the current ring.
    bool handleRingIncant(int hand, dodBYTE objtype) const;

    // Delays with an interruptible loop for duration milliseconds.
    // Returns false if loop was interrupted.
    bool delayBy(Uint32 duration) const;

    // Plays a sound and waits until it finished with a loop that calls
    // Scheduler::CLOCK().
    // If checkForDemoAbort is true we return false early if the demo was
    // aborted by the user.
    // Returns true if the sound finished playing.
    bool playSound(int channel, Mix_Chunk* chunk, bool checkForDemoAbort = false) const;

    dodBYTE exps[3];
};

#endif // DOD_PLAYER_HEADER
