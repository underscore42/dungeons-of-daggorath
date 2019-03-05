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
// Filename: oslink.cpp
//
// Implementation of OS_Link class

#include <iostream>
#include <fstream>

using namespace std;

#include "oslink.h"
#include "dodgame.h"
#include "viewer.h"
#include "sched.h"
#include "player.h"
#include "dungeon.h"
#include "parser.h"
#include "object.h"
#include "creature.h"
#include "enhanced.h"
#include <GL/glu.h> // for gluOrtho2D

extern Creature     creature;
extern Object       object;
extern Dungeon      dungeon;
extern Player       player;
extern Coordinate   crd;
extern Viewer       viewer;
extern dodGame      game;
extern Scheduler    scheduler;
extern Parser       parser;

// Constructor
OS_Link::OS_Link() : window(0), width(0), height(0),
    gamefileLen(50), keyLen(256),
    audio_rate(44100), audio_format(AUDIO_S16),
    audio_channels(2), audio_buffers(512),
    oglctx(0)
{
    printf ("OS_LINK Constructor\n");
#define MACOSX
#ifdef MACOSX
    strcpy(pathSep, "/");
#else
    strcpy(pathSep, "\\");
#endif

    strcpy(confDir, "conf");
    strcpy(soundDir, "sound");
    strcpy(savedDir, "saved");
    memset(gamefile, 0, gamefileLen);
}

// This routine will eventually need updated to allow
// user customization of screen size and resolution.
// It currently asks for an 1024x768 screen size.
// Updated - Now defaults to whatever is in the opts.ini file
// if opts.ini doesn't exist or has invalid or missing values
// uses defaults set by loadDefaults function (1024x768)
void OS_Link::init()
{
    printf("Starting OS_Link::init()\n");
    loadOptFile();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
        quitSDL(1);
    }

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    {
        fprintf(stderr, "Unable to open audio!\n");
        quitSDL(1);
    }

    creature.LoadSounds();
    object.LoadSounds();
    scheduler.LoadSounds();
    player.LoadSounds();

    Mix_AllocateChannels(4);
    Mix_Volume(-1, MIX_MAX_VOLUME);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    createWindow(width, height);
    changeVideoRes(width, height);

    memset(keys, parser.C_SP, keyLen);

    keys[SDL_SCANCODE_A] = 'A';
    keys[SDL_SCANCODE_B] = 'B';
    keys[SDL_SCANCODE_C] = 'C';
    keys[SDL_SCANCODE_D] = 'D';
    keys[SDL_SCANCODE_E] = 'E';
    keys[SDL_SCANCODE_F] = 'F';
    keys[SDL_SCANCODE_G] = 'G';
    keys[SDL_SCANCODE_H] = 'H';
    keys[SDL_SCANCODE_I] = 'I';
    keys[SDL_SCANCODE_J] = 'J';
    keys[SDL_SCANCODE_K] = 'K';
    keys[SDL_SCANCODE_L] = 'L';
    keys[SDL_SCANCODE_M] = 'M';
    keys[SDL_SCANCODE_N] = 'N';
    keys[SDL_SCANCODE_O] = 'O';
    keys[SDL_SCANCODE_P] = 'P';
    keys[SDL_SCANCODE_Q] = 'Q';
    keys[SDL_SCANCODE_R] = 'R';
    keys[SDL_SCANCODE_S] = 'S';
    keys[SDL_SCANCODE_T] = 'T';
    keys[SDL_SCANCODE_U] = 'U';
    keys[SDL_SCANCODE_V] = 'V';
    keys[SDL_SCANCODE_W] = 'W';
    keys[SDL_SCANCODE_X] = 'X';
    keys[SDL_SCANCODE_Y] = 'Y';
    keys[SDL_SCANCODE_Z] = 'Z';
    keys[SDL_SCANCODE_BACKSPACE] = parser.C_BS;
    keys[SDL_SCANCODE_RETURN] = parser.C_CR;
    keys[SDL_SCANCODE_SPACE] = parser.C_SP;

    // Delay to wait for monitor to change modes if necessary
    // This ought to be made more intelligent
    SDL_Delay(1000);

    game.COMINI();
    while (true)
    {
        scheduler.SCHED();
        if (scheduler.ZFLAG == 0xFF)
        {
            game.LoadGame();
            scheduler.ZFLAG = 0;
        }
        else
        {
            if (game.AUTFLG)
            {
                if (game.demoRestart)
                {
                    // Restart demo
                    game.hasWon = false;
                    game.DEMOPTR = 0;
                    object.Reset();
                    creature.Reset();
                    parser.Reset();
                    player.Reset();
                    scheduler.Reset();
                    viewer.Reset();
                    dungeon.VFTPTR = 0;
                    game.COMINI();
                }
                else
                {
                    // Start new game
                    game.AUTFLG = false;
                    game.Restart();
                }
            }
            else
            {
                game.Restart();
            }
        }
    }
    printf("Init complete\n");
}

// Quits application
void OS_Link::quitSDL(int code)
{
    Mix_CloseAudio();
    SDL_GL_DeleteContext(oglctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(code);
}

// Used to check for keystrokes and application termination
void OS_Link::process_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            handle_key_down(&event.key.keysym);
            break;
        case SDL_QUIT:
            quitSDL(0);
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                SDL_GL_SwapWindow(window);
            break;
        }
    }
}

// Processes key strokes.
void OS_Link::handle_key_down(SDL_Keysym * keysym)
{
    dodBYTE c;
    if (viewer.display_mode == Viewer::MODE_MAP)
    {
        switch(keysym->scancode)
        {
        case SDL_SCANCODE_ESCAPE:
            main_menu();
            break;
        default:
            viewer.display_mode = Viewer::MODE_3D;
            --viewer.UPDATE;
            parser.KBDPUT(32); // This is a (necessary ???) hack.
            break;
        }
    }
    else
    {
        switch(keysym->scancode)
        {
        case SDL_SCANCODE_RSHIFT:
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_LGUI:
        case SDL_SCANCODE_RGUI:
        case SDL_SCANCODE_MODE:
        case SDL_SCANCODE_APPLICATION:
        case SDL_SCANCODE_NUMLOCKCLEAR:
        case SDL_SCANCODE_CAPSLOCK:
        case SDL_SCANCODE_SCROLLLOCK:
            // ignore these keys
            return;

        case SDL_SCANCODE_ESCAPE:
            main_menu();   // Enter the meta-menu routine
            return;

//      case SDLK_EXCLAIM: c = '!'; break;
//      case SDLK_LESS: c = '<'; break;
//      case SDLK_GREATER: c = '>'; break;
//      case SDLK_QUESTION: c = '?'; break;
//      case SDLK_UNDERSCORE: c = '_'; break;
//      case SDLK_PERIOD: c = '.'; break;
//      case SDLK_LEFTBRACKET: c = '{'; break;
//      case SDLK_RIGHTBRACKET: c = '}'; break;

        default:
            c = keys[keysym->scancode];
            break;
        }
        parser.KBDPUT(c);
    }
}

/*********************************************************
  Member: main_menu

  Function: Implements the menu, and dispatches commands

  Returns:  true  - If a new game is started
            false - otherwise
*********************************************************/
bool OS_Link::main_menu()
{
    bool end = false;
    static int row = 0, col = 0;
    static menu mainMenu;

    scheduler.pause(true);
    viewer.drawMenu(mainMenu, col, row);

    do
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_RETURN:
                    end = menu_return(col, row, mainMenu);

                    // Used for Wizard fade functions, if it's a new game, it will trigger a key press
                    if(col == FILE_MENU_SWITCH && row == FILE_MENU_NEW)
                        return true;

                    break;
                case SDL_SCANCODE_UP:
                    (row < 1) ? row = mainMenu.getMenuSize(col) - 1 : row--;
                    break;
                case SDL_SCANCODE_DOWN:
                    (row > mainMenu.getMenuSize(col) - 2) ? row = 0 : row++;
                    break;
                case SDL_SCANCODE_LEFT:
                    (col < 1) ? col = NUM_MENU - 1 : col--;
                    row = 0;
                    break;
                case SDL_SCANCODE_RIGHT:
                    (col > 1) ? col = 0 : col++;
                    row = 0;
                    break;
                case SDL_SCANCODE_ESCAPE:
                    end = true;
                    break;
                default:
                    break;
                }
                viewer.drawMenu(mainMenu, col, row);
                break;
            case SDL_QUIT:
                quitSDL(0);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
            }
        }
    }
    while(!end);

    scheduler.pause(false);

    return false;
}

/* Function to process menu commands
*
*  Returns:  false - if menu should be redrawn
*            true  - otherwise */
bool OS_Link::menu_return(int menu_id, int item, menu Menu)
{
    switch(menu_id)
    {
    // File Menu
    case FILE_MENU_SWITCH:
        switch(item)
        {
        case FILE_MENU_NEW:
            //New Game
            scheduler.pause(false);  // Needed so that the game can be paused again later

            if(!game.AUTFLG)
            {
                game.hasWon = true;
                game.demoRestart = false;
            }
            return true;
            break;

        case FILE_MENU_RETURN:
            //Return
            return true;
            break;

        case FILE_MENU_ABORT:
            //Abort (Restart)
            scheduler.pause(false);  // Needed so that the game can be paused again later

            if(!game.AUTFLG)
            {
                game.AUTFLG = true;
                game.hasWon = true;
                game.demoRestart = true;
            }
            return true;
            break;

        case FILE_MENU_EXIT:
            //Exit
            quitSDL(0);
        }

    // Configuration Menu
    case CONFIG_MENU_SWITCH:
        switch(item)
        {
        case CONFIG_MENU_FULL_SCREEN:
            //Full Screen
        {
            const char *menuList[] = { "WINDOWED", "BORDERLESS", "FULLSCREEN" };

            const int displaytype = menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3);
            if (displaytype >= 0)
                changeWindowMode(WindowMode(displaytype));
        }
        return false;
        break;

        case CONFIG_MENU_VIDEO_RES:
            // Video Res
        {
            const char *menuList[] = { "640X480", "800X600", "1024X768", "1280X960" };
            const int widths[] = { 640, 800, 1024, 1280 };
            const int heights[] = { 480, 600, 768, 960 };
            const int offset = menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 4);
            if (offset >= 0)
                changeVideoRes(widths[offset], heights[offset]);
        }
        return false;
        break;

        case CONFIG_MENU_GRAPHICS:
            // Graphics (Normal /HIRes / vect)
        {
            const char *menuList[] = { "NORMAL GRAPHICS", "HIRES GRAPHICS", "VECTOR GRAPHICS" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3))
            {
            case 0:
                g_options &= ~(OPT_VECTOR | OPT_HIRES);
                break;

            case 1:
                g_options &= ~(OPT_VECTOR);
                g_options |= OPT_HIRES;
                break;

            case 2:
                g_options &= ~(OPT_HIRES);
                g_options |= OPT_VECTOR;
                break;

            default:
                return false;
                break;
            }
        }
        return true;
        break;

        case CONFIG_MENU_COLOR:
            // Color (B&W / Art. / Full)
        {
            const char *menuList[] = { "BLACK WHITE" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1))
            {
            default:
                return false;
                break;
            }
        }
        return true;
        break;

        case CONFIG_MENU_VOLUME:
            // Volume
        {
            volumeLevel = menu_scrollbar("VOLUME LEVEL", 0, 128, volumeLevel);
            Mix_Volume(-1, volumeLevel);
        }
        return false;
        break;

        case CONFIG_MENU_SAVEDIR:
            // Save Dir
        {
            const char *menuList[] = { "EDIT OPTS.INI FILE" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1))
            {
            default:
                return false;
                break;
            }
        }
        return false;
        break;

        case CONFIG_MENU_CREATURE_SPEED:
            // Creature Speed
        {
            const char *menuList[2] = {"COCO", "CUSTOM"};

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
            {
            case 0:
                //Coco Speed
                creature.creSpeedMul = 200;
                creature.UpdateCreSpeed();
                break;
            case 1:
                //Custom Speed
                creature.creSpeedMul = menu_scrollbar("CREATURE SPEED", 50, 200, volumeLevel);
                creature.UpdateCreSpeed();
                return false;
                break;

            default:
                return false;
                break;
            }
        }
        break;

        case CONFIG_MENU_REGEN_SPEED:
            // Regen Speed
        {
            const char *menuList[] = { "5 MINUTES", "3 MINUTES", "1 MINUTE" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3))
            {
            case 0:
                creatureRegen = 5;
                scheduler.updateCreatureRegen(creatureRegen);
                break;
            case 1:
                creatureRegen = 3;
                scheduler.updateCreatureRegen(creatureRegen);
                break;
            case 2:
                creatureRegen = 1;
                scheduler.updateCreatureRegen(creatureRegen);
                break;
            default:
                return false;
                break;
            }
        }
        return true;
        break;

        case CONFIG_MENU_RANDOM_MAZE:
            // Random Mazes
        {
            const char *menuList[] = { "ON", "OFF" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
            {
            case 0:
                game.RandomMaze = true;
                break;
            case 1:
                game.RandomMaze = false;
                break;
            default:
                return false;
                break;
            }
        }
        return false;
        break;

        case CONFIG_MENU_SND_MODE:
            // Sound Style (Sync, Stereo)
        {
            const char *menuList[2] = {"STEREO", "MONO"};

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
            {
            case 0:
                g_options |= OPT_STEREO;
                break;

            case 1:
                g_options &= ~OPT_STEREO;
                break;

            default:
                return false;
                break;
            }
        }
        break;

        case CONFIG_MENU_SAVE_OPT:
            saveOptFile();
            return true;
            break;

        case CONFIG_MENU_DEFAULTS:
            loadDefaults();
            changeVideoRes(width, height);
            return true;
            break;
        }

    // Help menu
    case HELP_MENU_SWITCH:
        switch(item)
        {
        case HELP_MENU_HOWTOPLAY:
            // How to play
        {
            const char *menuList[] = { "SEE FILE HOWTOPLAY.TXT" };

            menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
        }
        return false;
        break;

        case HELP_MENU_LICENSE:
            // License
        {
            const char *menuList[] = { "SEE FILE README.TXT" };

            menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
        }
        return false;
        break;

        case HELP_MENU_ABOUT:
            // About
        {
            SDL_Event event;

            viewer.aboutBox();
            while(true)
            {
                while(SDL_PollEvent(&event))
                {
                    switch(event.type)
                    {
                    case SDL_KEYDOWN:
                        return false;
                        break;
                    case SDL_QUIT:
                        quitSDL(0);  // Quits SDL
                        break;
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                            SDL_GL_SwapWindow(window);
                        break;
                    }
                }
            }
            return false;
        }
        break;
        }
    }
    return true;
}

/*****************************************************************************
*  Function used to draw a list, move among that list, and return the item selected
*
*  Arguments: x        - The top-left x-coordinate to draw list at
*             y        - The top-left y-coordinate to draw list at
*             title    - The title of the list
*             list     - An array of strings (the list to be chosen from
*             listSize - The size of the array
*  Returns: -1 if ESC was pressed else the index of the selected element
******************************************************************************/
int OS_Link::menu_list(int x, int y, const char *title, const char *list[], int listSize)
{
    int currentChoice = 0;

    while(true)
    {
        viewer.drawMenuList(x, y, title, list, listSize, currentChoice);
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_RETURN:
                    return(currentChoice);
                    break;

                case SDL_SCANCODE_UP:
                    (currentChoice < 1) ? currentChoice = listSize - 1 : currentChoice--;
                    break;

                case SDL_SCANCODE_DOWN:
                    (currentChoice > listSize - 2) ? currentChoice = 0 : currentChoice++;
                    break;

                case SDL_SCANCODE_ESCAPE:
                    return(-1);
                    break;

                default:
                    break;
                }
                break;
            case SDL_QUIT:
                quitSDL(0);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
            }
        }
    } // End of while loop

    return(-1);
}

/*****************************************************************************
*  Function used to draw a scrollbar, and return the value
*
*  Arguments: title     - The title of the entry
*             min       - The minimum value the scroll bar can take
*             max       - The maximum value the scroll bar can take
*             current   - The current position of the scrollbar
*
*  Returns: The value the user entered, or if they hit escape, the original
*           value.
******************************************************************************/
int OS_Link::menu_scrollbar(const char *title, int min, int max, int current)
{
    int oldvalue  = current; //Save the old value in case the user escapes
    int increment = (max - min) / 31;  // 31 is the number of columns

    // Calculate a relative max and min and corresponding current number
    int newMax    = increment * 31;
    int newMin    = 0;
    current   = current - min;

    viewer.drawMenuScrollbar(title, (current - newMin) / increment);

    while(true)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_RETURN:
                    return(current + min);  // Readjust back to absolute value
                    break;

                case SDL_SCANCODE_LEFT:
                    (current > newMin) ? current -= increment : current = newMin;
                    break;

                case SDL_SCANCODE_RIGHT:
                    (current < newMax) ? current += increment : current = newMax;
                    break;

                case SDL_SCANCODE_ESCAPE:
                    return(oldvalue);
                    break;

                default:
                    break;
                }
                viewer.drawMenuScrollbar(title, (current - newMin) / increment);
                break;
            case SDL_QUIT:
                quitSDL(0);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
            }
        }
    }
}

/*****************************************************************************
*  Function used to draw a box for a string entry, then return it
*
*  Arguments: newString - The string to be returned
*             title     - The title of the entry
*             maxLength - The maximum size of the entry
******************************************************************************/
void OS_Link::menu_string(char *newString, const char *title, size_t maxLength)
{
    *newString = '\0';
    viewer.drawMenuStringTitle(title);
    viewer.drawMenuString(newString);

    while(true)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_RETURN:
                    return;
                    break;

                case SDL_SCANCODE_RSHIFT:
                case SDL_SCANCODE_LSHIFT:
                case SDL_SCANCODE_RCTRL:
                case SDL_SCANCODE_LCTRL:
                case SDL_SCANCODE_RALT:
                case SDL_SCANCODE_LALT:
                case SDL_SCANCODE_LGUI:
                case SDL_SCANCODE_RGUI:
                case SDL_SCANCODE_MODE:
                case SDL_SCANCODE_APPLICATION: //was SDLK_COMPOSE:
                case SDL_SCANCODE_NUMLOCKCLEAR:
                case SDL_SCANCODE_CAPSLOCK:
                case SDL_SCANCODE_SCROLLLOCK:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_DOWN:
                    // ignore these keys
                    break;

                case SDL_SCANCODE_BACKSPACE:
                case SDL_SCANCODE_LEFT:
                    if(strlen(newString) > 0)
                    {
                        *(newString + strlen(newString) - 1) = '\0';
                        viewer.drawMenuStringTitle(title);  // Update with the new word
                        viewer.drawMenuString(newString);
                    }
                    break;

                case SDL_SCANCODE_ESCAPE:
                    *(newString) = '\0';
                    return;
                    break;

                default:
                    if(strlen(newString) < maxLength)
                    {
                        *(newString + strlen(newString) + 1) = '\0';
                        *(newString + strlen(newString)) = keys[event.key.keysym.sym];
                        viewer.drawMenuStringTitle(title);  // Update with the new word
                        viewer.drawMenuString(newString);
                    }
                    break;
                }
                break;
            case SDL_QUIT:
                quitSDL(0);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
            }
        }
    } // End of while loop
}

/******************************************************************************
*  Function used to load & parse options file
*
*  Arguments: None
******************************************************************************/
void OS_Link::loadOptFile(void)
{
    char     inputString[80];
    char     fn[20];
    int      in;
    ifstream fin;
    char *   breakPoint;

    loadDefaults(); // In case some variables aren't in the opts file, and if no file exists

    sprintf(fn, "%s%s%s", confDir, pathSep, "opts.ini");

    fin.open(fn);
    if (!fin)
    {
        return;
    }

    fin >> inputString;
    while(fin)
    {
        breakPoint = strchr(inputString, '=');

        // Ignore strings that have no equals, or are only an equals, or have no end
        if(breakPoint || breakPoint == inputString || breakPoint == (inputString + strlen(inputString) - 1))
        {
            *(breakPoint) = '\0';
            breakPoint++;

            if(!strcmp(inputString, "creatureSpeed"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) creature.creSpeedMul = in;
            }
            else if(!strcmp(inputString, "turnDelay"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) player.turnDelay = in;
            }
            else if(!strcmp(inputString, "moveDelay"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) player.moveDelay = in;
            }
            else if(!strcmp(inputString, "graphicsMode"))
            {
                if(!strcmp(breakPoint, "NORMAL"))
                    g_options &= ~(OPT_VECTOR | OPT_HIRES);
                else if(!strcmp(breakPoint, "HIRES"))
                {
                    g_options &= ~(OPT_VECTOR);
                    g_options |= OPT_HIRES;
                }
                else if(!strcmp(breakPoint, "VECTOR"))
                {
                    g_options &= ~(OPT_HIRES);
                    g_options |= OPT_VECTOR;
                }
            }
            else if(!strcmp(inputString, "stereoMode"))
            {
                if(!strcmp(breakPoint, "STEREO"))
                    g_options |= OPT_STEREO;
                else if(!strcmp(breakPoint, "MONO"))
                    g_options &= ~OPT_STEREO;
            }
            else if(!strcmp(inputString, "volumeLevel"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) volumeLevel = in;
            }
            else if(!strcmp(inputString, "saveDirectory"))
            {
                strncpy(savedDir, "saved", MAX_FILENAME_LENGTH);
            }
            else if(!strcmp(inputString, "windowMode"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) mode = WindowMode(in);
            }
            else if(!strcmp(inputString, "screenWidth"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) width = in;
            }
            else if(!strcmp(inputString, "creatureRegen"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) creatureRegen = in;
            }
            else if(!strcmp(inputString, "RandomMaze"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.RandomMaze = in;
            }
            else if(!strcmp(inputString, "ShieldFix"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.ShieldFix = in;
            }
            else if(!strcmp(inputString, "VisionScroll"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.VisionScroll = in;
            }
            else if(!strcmp(inputString, "CreaturesIgnoreObjects"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.CreaturesIgnoreObjects = in;
            }
            else if(!strcmp(inputString, "CreaturesInstaRegen"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.CreaturesInstaRegen = in;
            }
            else if(!strcmp(inputString, "MarkDoorsOnScrollMaps"))
            {
                if (1 == sscanf(breakPoint, "%d", &in)) game.MarkDoorsOnScrollMaps = in;
            }
        }

        fin >> inputString;
    }

    fin.close();
    scheduler.updateCreatureRegen(creatureRegen);
    creature.UpdateCreSpeed();
}

/******************************************************************************
*  Function used to save the options file from current settings
*
*  Arguments: None
*
*  Returns:   true - file saved successfully, false - file not saved
******************************************************************************/
bool OS_Link::saveOptFile(void)
{
    ofstream fout;
    char     fn[MAX_FILENAME_LENGTH];

    sprintf(fn, "%s%s%s", confDir, pathSep, "opts.ini");

    fout.open(fn);
    if(!fout)
        return false;

    fout << "creatureSpeed=" << creature.creSpeedMul << endl;
    fout << "turnDelay=" << player.turnDelay << endl;
    fout << "moveDelay=" << player.moveDelay << endl;
    fout << "volumeLevel=" << volumeLevel << endl;
    fout << "saveDirectory=" << savedDir << endl;
    fout << "windowMode=" << mode << endl;
    fout << "screenWidth=" << width << endl;
    fout << "creatureRegen=" << creatureRegen << endl;

    fout << "graphicsMode=";
    if(g_options & OPT_VECTOR)
        fout << "VECTOR" << endl;
    else if(g_options & OPT_HIRES)
        fout << "HIRES" << endl;
    else
        fout << "NORMAL" << endl;

    fout << "stereoMode=";
    if(g_options & OPT_STEREO)
        fout << "STEREO" << endl;
    else
        fout << "MONO" << endl;

    fout << "RandomMaze=" << game.RandomMaze << endl;
    fout << "ShieldFix=" << game.ShieldFix << endl;
    fout << "VisionScroll=" << game.VisionScroll << endl;
    fout << "CreaturesIgnoreObjects=" << game.CreaturesIgnoreObjects << endl;
    fout << "CreaturesInstaRegen=" << game.CreaturesInstaRegen << endl;
    fout << "MarkDoorsOnScrollMaps=" << game.MarkDoorsOnScrollMaps << endl;

    fout.close();

    return true;
}

/******************************************************************************
*  Function used to load the options file from current settings
*
*  Arguments: None
******************************************************************************/
void OS_Link::loadDefaults(void)
{
    player.turnDelay = 37;
    player.moveDelay = 500;
    volumeLevel = MIX_MAX_VOLUME;
    creature.creSpeedMul = 200;
    creature.UpdateCreSpeed();
    strcpy(savedDir, "saved");
    mode = WINDOWED;
    width = 1024;
    height = 768;
    creatureRegen = 5;
    scheduler.updateCreatureRegen(creatureRegen);

    g_options &= ~(OPT_VECTOR | OPT_HIRES);
    g_options |= OPT_STEREO;
}

/******************************************************************************
*  Function used to swap fullscreen mode
*
*  Arguments: newmode - the new window mode
******************************************************************************/
void OS_Link::changeWindowMode(WindowMode newmode)
{
    if (mode == newmode)
        return;

    const WindowMode oldmode = mode;
    mode = newmode;

    if (newmode == WINDOWED)
    {
        SDL_SetWindowFullscreen(window, 0);
        changeVideoRes(windowed_width, windowed_height);
    }
    else if (newmode == BORDERLESS)
    {
        if (oldmode == FULLSCREEN)
        {
            // Because of buggy behavior in SDL2 the sane thing to do here is to
            // switch to windowed mode first.
            // https://bugzilla.libsdl.org/show_bug.cgi?id=3357
            SDL_SetWindowFullscreen(window, 0);
            // The delay is necessary, 100ms is enough on my computer.
            SDL_Delay(250);
        }

        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        int display = SDL_GetWindowDisplayIndex(window);
        SDL_DisplayMode dispmode;
        SDL_GetDisplayMode(display, 0, &dispmode);

        changeVideoRes(dispmode.w, dispmode.h);
    }
    else if (newmode == FULLSCREEN)
    {
        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) != 0)
        {
            mode = oldmode;
            fprintf(stderr, "SDL_SetWindowFullscreen: %s\n", SDL_GetError());
        }
    }

    SDL_ShowCursor(newmode == WINDOWED ? SDL_ENABLE : SDL_DISABLE);
    SDL_GL_SwapWindow(window);
}

/******************************************************************************
*  Creates the application window and the OpenGL context
*
*  Arguments: width  - initial width of window
*             height - initial height of window
******************************************************************************/
void OS_Link::createWindow(int width, int height)
{
    if (window)
        return;

    window = SDL_CreateWindow("Dungeons of Daggorath",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_OPENGL);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        exit(1);
    }

    windowed_width  = width;
    windowed_height = height;

    if (!oglctx)
        oglctx = SDL_GL_CreateContext(window);

    viewer.setup_opengl();
}

/******************************************************************************
*  Function used to change the video resolution
*
*  Arguments: newWidth  - desired screen width
*             newHeight - desired screen height
******************************************************************************/
void OS_Link::changeVideoRes(int newWidth, int newHeight)
{
    const int scaledHeight = (int) (newWidth * 0.75);
    if (scaledHeight > newHeight)
    {
        // keep 4:3 ratio by deriving the new width from the desired height
        width  = newHeight / 0.75;
        height = newHeight;
    }
    else
    {
        width  = newWidth;
        height = scaledHeight;
    }

    crd.setCurWH((double) width);

    if (mode == BORDERLESS)
    {
        SDL_SetWindowSize(window, newWidth, newHeight);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        // center viewport on screen
        glViewport((newWidth  - width ) / 2,
                   (newHeight - height) / 2, width, height);

    }
    else if (mode == WINDOWED)
    {
        SDL_SetWindowSize(window, width, height);

        windowed_width  = width;
        windowed_height = height;

        glViewport(0, 0, width, height);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
