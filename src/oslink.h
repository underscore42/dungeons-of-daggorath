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
// Filename: oslink.h
//
// This class manages the SDL operations, which abstract
// the link to the operating system.  By keeping these
// separate, it will be somewhat easier to change to a
// different library if necessary.

#ifndef OS_LINK_HEADER
#define OS_LINK_HEADER

#include "dod.h"
#include "parser.h"

// Arbitrary Length of 80, maybe be changed if needed
#define MAX_FILENAME_LENGTH 80
class OS_Link
{
public:
    // Constructor
    OS_Link();

    // Public Interface
    void init();            // main entry point for dod application
    void quitSDL(int code); // shuts down SDL before exiting
    void process_events();  // used mainly to retrieve keystrokes
    bool main_menu();       // used to implement the meta-menu
    bool saveOptFile(void);
    void buildSaveGamePath();

    // Public Data Fields
    SDL_Window* window;
    int     width;  // actual screen width after video setup
    int     height; // same for height
    int     windowed_width;  // width when we were last in windowed mode
    int     windowed_height; // height when we were last in windowed mode
    int     volumeLevel; // Volume level

    static constexpr size_t pathSepLen = 2;
    static constexpr size_t gamefileLen = MAX_FILENAME_LENGTH + pathSepLen + Parser::MAX_TOKENLEN + 5; //  + 5 because we need to append ".dod\0"
    static constexpr size_t keyLen = 256;

    char    gamefile[gamefileLen]; // current savefile that we're loading or saving
    char    pathSep[2];

    char    confDir[5];
    char    soundDir[6];
    char    savedDir[MAX_FILENAME_LENGTH + 1];
    dodBYTE keys[keyLen];

    int     audio_rate;
    Uint16  audio_format;
    int     audio_channels;
    int     audio_buffers;

private:

    enum WindowMode
    {
        WINDOWED   = 0,
        BORDERLESS = 1,
        FULLSCREEN = 2
    };

    // Internal Implementation
    void handle_key_down(SDL_Keysym * keysym);  // keyboard handler
    bool menu_return(int, int, menu);       // Used by main menu
    int  menu_list(int x, int y, const char *title, const char *list[], int listSize);
    void menu_string(char *newString, const char *title, size_t maxLength);
    int  menu_scrollbar(const char *title, int min, int max, int current);
    void loadOptFile(void);
    void loadDefaults(void);
    void createWindow(int width, int height);
    void changeWindowMode(WindowMode newmode);
    void changeVideoRes(int newWidth, int newHeight);

    // Data Fields
    SDL_GLContext oglctx;
    WindowMode    mode;
    int           creatureRegen; // Creature Regen Speed
};

#endif // OS_LINK_HEADER
