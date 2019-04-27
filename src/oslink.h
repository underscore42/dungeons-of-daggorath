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

#include <string>

#include "dod.h"
#include "parser.h"

class OS_Link
{
public:

    OS_Link();

    // main entry point for dod application
    void init();

    // Quits application and shuts down SDL before exiting
    void quitSDL(int code);

    // Used to check for keystrokes and application termination
    void process_events();

    // Implements the menu, and dispatches commands
    // Returns: true  - If a new game is started
    //          false - otherwise
    bool main_menu();

    // Function used to save the options file from current settings
    // Returns: true - file saved successfully, false - file not saved
    bool saveOptFile() const;

    // builds up the filename of the savegame to load/save
    void buildSaveGamePath();

    // the application window
    SDL_Window* window;

    // actual screen width after video setup
    int width;

    // actual screen height after video setup
    int height;

    // width when we were last in windowed mode
    int windowed_width;

    // height when we were last in windowed mode
    int windowed_height;

    // Audio volume level
    int volumeLevel;

    static constexpr size_t keyLen = 256;

    // Maps a pressed key to a character that is put into the game's keyboard buffer
    dodBYTE keys[keyLen];

    int    audio_rate;
    Uint16 audio_format;
    int    audio_channels;
    int    audio_buffers;

    // current savefile that we're loading or saving
    std::string gamefile;

    const std::string dod;
    const std::string confDir;
    const std::string savedDir;
    const std::string soundDir;

private:

    enum WindowMode
    {
        WINDOWED   = 0,
        BORDERLESS = 1,
        FULLSCREEN = 2
    };

    std::string buildConfigPath() const;
    std::string buildSavePath() const;

    // Processes key strokes.
    void handle_key_down(SDL_Keysym * keysym);

    // Function used to draw a list, move among that list, and return the item selected
    // Arguments: x        - The top-left x-coordinate to draw list at
    //            y        - The top-left y-coordinate to draw list at
    //            title    - The title of the list
    //            list     - An array of strings (the list to be chosen from
    //            listSize - The size of the array
    // Returns: -1 if ESC was pressed else the index of the selected element
    int menu_list(int x, int y, const char *title, const char *list[], int listSize);

    // Function used to draw a scrollbar, and return the value
    // Arguments: title   - The title of the entry
    //            min     - The minimum value the scroll bar can take
    //            max     - The maximum value the scroll bar can take
    //            current - The current position of the scrollbar
    // Returns: The value the user entered, or if they hit escape, the original
    //          value.
    int menu_scrollbar(const char *title, int min, int max, int current);

    // Loads options from config file
    void loadOptFile();

    // Sets default option values
    void loadDefaults();

    // Creates the application window and the OpenGL context
    // Arguments: width  - initial width of window
    //            height - initial height of window
    void createWindow(int width, int height);

    // Function used to swap fullscreen mode
    // Arguments: newmode - the new window mode
    void changeWindowMode(WindowMode newmode);

    // Function used to change the video resolution
    // Arguments: newWidth  - desired screen width
    //            newHeight - desired screen height
    void changeVideoRes(int newWidth, int newHeight);

    // Function to process menu commands
    // Returns: false - if menu should be redrawn
    //          true  - otherwise
    bool menu_return(int, int, menu);

    // handle the file menu
    bool handleFileMenuSwitch(int menu_id, int item, menu Menu);

    // handle the config menu
    bool handleConfigMenuSwitch(int menu_id, int item, menu Menu);

    // handle the help menu
    bool handleHelpMenuSwitch(int menu_id, int item, menu Menu);

    // OpenGL context
    SDL_GLContext oglctx;

    // current window mode
    WindowMode mode;

    // Creature Regen Speed
    int creatureRegen;
};

#endif // OS_LINK_HEADER
