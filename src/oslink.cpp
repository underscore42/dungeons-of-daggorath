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

#include <charconv>
#include <iostream>
#include <fstream>

#include <sys/stat.h> // for mkdir

// config.h is generated via config.h.in through the build system
#include "config.h"

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

extern Creature     creature;
extern Object       object;
extern Dungeon      dungeon;
extern Player       player;
extern Coordinate   crd;
extern Viewer       viewer;
extern dodGame      game;
extern Scheduler    scheduler;
extern Parser       parser;

OS_Link::OS_Link() : window(0), width(0), height(0),
    audio_rate(44100), audio_format(AUDIO_S16),
    audio_channels(2), audio_buffers(512),
    dod("dungeons-of-daggorath"),
    confDir(buildConfigPath()),
    savedDir(buildSavePath()),
    soundDir(std::string(DOD_ASSET_PATH) + "/" + dod + "/sound/"),
    oglctx(0)
{
}

std::string OS_Link::buildConfigPath() const
{
    const char* xdg_conf = std::getenv("XDG_CONFIG_HOME");

    std::string tmp;
    tmp.reserve(64);

    if (xdg_conf)
        tmp = xdg_conf;
    else
    {
        tmp = std::getenv("HOME");
        tmp += "/.config";
    }

    tmp += "/" + dod + "/";
    return tmp;
}

std::string OS_Link::buildSavePath() const
{
    const char* xdg_data = std::getenv("XDG_DATA_HOME");

    std::string tmp;
    tmp.reserve(64);

    if (xdg_data)
        tmp = xdg_data;
    else
    {
        tmp = std::getenv("HOME");
        tmp += "/.local/share";
    }

    tmp += "/" + dod + "/";
    return tmp;
}

void OS_Link::init()
{
    printf("Starting OS_Link::init()\n");
    loadOptFile();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        quitSDL(EXIT_FAILURE);
    }

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    {
        fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
        quitSDL(EXIT_FAILURE);
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
}

void OS_Link::quitSDL(int code)
{
    Mix_CloseAudio();
    SDL_GL_DeleteContext(oglctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(code);
}

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
                quitSDL(EXIT_SUCCESS);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
        }
    }
}

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
                parser.KBDPUT(parser.C_SP); // This is a (necessary ???) hack.
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
            default:
                c = keys[keysym->scancode];
                break;
        }
        parser.KBDPUT(c);
    }
}

bool OS_Link::main_menu()
{
    static int row = 0, col = 0;
    static menu mainMenu;

    scheduler.pause(true);
    viewer.drawMenu(mainMenu, col, row);

    bool end = false;
    do
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                {
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
                }
                case SDL_QUIT:
                    quitSDL(EXIT_SUCCESS);
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

bool OS_Link::menu_return(int menu_id, int item, menu Menu)
{
    switch(menu_id)
    {
        // File Menu
        case FILE_MENU_SWITCH:
            return handleFileMenuSwitch(menu_id, item, Menu);

        // Configuration Menu
        case CONFIG_MENU_SWITCH:
            return handleConfigMenuSwitch(menu_id, item, Menu);

        // Help menu
        case HELP_MENU_SWITCH:
            return handleHelpMenuSwitch(menu_id, item, Menu);
    }
    return true;
}

bool OS_Link::handleFileMenuSwitch(int menu_id, int item, menu Menu)
{
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

        case FILE_MENU_RETURN:
            return true;

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

        case FILE_MENU_EXIT:
            //Exit
            quitSDL(EXIT_SUCCESS);
    }
    return true;
}

bool OS_Link::handleConfigMenuSwitch(int menu_id, int item, menu Menu)
{
    switch(item)
    {
        case CONFIG_MENU_FULL_SCREEN:
        {
            const char *menuList[] = { "WINDOWED", "BORDERLESS", "FULLSCREEN" };

            const int displaytype = menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3);
            if (displaytype >= 0)
                changeWindowMode(WindowMode(displaytype));
            return false;
        }
        case CONFIG_MENU_VIDEO_RES:
        {
            const char *menuList[] = { "640X480", "800X600", "1024X768", "1280X960" };
            const int widths[] = { 640, 800, 1024, 1280 };
            const int heights[] = { 480, 600, 768, 960 };
            const int offset = menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 4);
            if (offset >= 0)
                changeVideoRes(widths[offset], heights[offset]);
            return false;
        }
        case CONFIG_MENU_GRAPHICS:
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
            }
            return true;
        }
        case CONFIG_MENU_COLOR:
            // Color (B&W / Art. / Full)
        {
            const char *menuList[] = { "BLACK WHITE" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1))
            {
                default:
                    return false;
            }
            return true;
        }
        case CONFIG_MENU_VOLUME:
        {
            volumeLevel = menu_scrollbar("VOLUME LEVEL", 0, 128, volumeLevel);
            Mix_Volume(-1, volumeLevel);
            return false;
        }
        case CONFIG_MENU_CREATURE_SPEED:
        {
            const char *menuList[2] = {"COCO", "CUSTOM"};

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
            {
                case 0:
                    //Coco Speed
                    creature.creSpeedMul = 100;
                    creature.UpdateCreSpeed();
                    break;
                case 1:
                    //Custom Speed
                    creature.creSpeedMul = menu_scrollbar("CREATURE SPEED", 50, 200, creature.creSpeedMul);
                    creature.UpdateCreSpeed();
                    return false;
                default:
                    return false;
            }
            break;
        }
        case CONFIG_MENU_REGEN_SPEED:
        {
            const char *menuList[] = { "5 MINUTES", "3 MINUTES", "1 MINUTE" };

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3))
            {
                case 0: creatureRegen = 5; break;
                case 1: creatureRegen = 3; break;
                case 2: creatureRegen = 1; break;
                default:
                    return false;
            }
            scheduler.updateCreatureRegen(creatureRegen);
            return true;
        }
        case CONFIG_MENU_RANDOM_MAZE:
        {
            const char *menuList[] = { "ON", "OFF" };

            const int randomMaze = menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2);
            if (randomMaze < 0)
                return false;
            game.RandomMaze = randomMaze == 0;
            return false;
        }
        case CONFIG_MENU_SND_MODE:
        {
            const char *menuList[2] = {"STEREO", "MONO"};

            switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
            {
                case 0: g_options |= OPT_STEREO;  break;
                case 1: g_options &= ~OPT_STEREO; break;
                default:
                    return false;
            }
            break;
        }
        case CONFIG_MENU_SAVE_OPT:
            saveOptFile();
            return true;
        case CONFIG_MENU_DEFAULTS:
            loadDefaults();
            changeVideoRes(width, height);
            return true;
    }
    return true;
}

bool OS_Link::handleHelpMenuSwitch(int menu_id, int item, menu Menu)
{
    switch(item)
    {
        case HELP_MENU_HOWTOPLAY:
        {
            const char *menuList[] = { "SEE FILE HOWTOPLAY.TXT" };

            menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
            return false;
        }
        case HELP_MENU_LICENSE:
        {
            const char *menuList[] = { "SEE FILE README.TXT" };

            menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
            return false;
        }
        case HELP_MENU_ABOUT:
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
                        case SDL_QUIT:
                            quitSDL(EXIT_SUCCESS);
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
    }
    return true;
}

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
                    case SDL_SCANCODE_UP:
                        (currentChoice < 1) ? currentChoice = listSize - 1 : currentChoice--;
                        break;
                    case SDL_SCANCODE_DOWN:
                        (currentChoice > listSize - 2) ? currentChoice = 0 : currentChoice++;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        return(-1);
                    default:
                        break;
                }
                break;
            case SDL_QUIT:
                quitSDL(EXIT_SUCCESS);
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

int OS_Link::menu_scrollbar(const char *title, int min, int max, int current)
{
    const int oldvalue  = current; //Save the old value in case the user escapes
    const int increment = (max - min) / 31;  // 31 is the number of columns

    // Calculate a relative max and min and corresponding current number
    const int newMax = increment * 31;
    const int newMin = 0;
    current = current - min;

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
                    case SDL_SCANCODE_LEFT:
                        (current > newMin) ? current -= increment : current = newMin;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        (current < newMax) ? current += increment : current = newMax;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        return(oldvalue);
                    default:
                        break;
                }
                viewer.drawMenuScrollbar(title, (current - newMin) / increment);
                break;
            case SDL_QUIT:
                quitSDL(EXIT_SUCCESS);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    SDL_GL_SwapWindow(window);
                break;
            }
        }
    }
}

void OS_Link::loadOptFile()
{
    loadDefaults(); // In case some variables aren't in the opts file, and if no file exists

    std::ifstream fin;
    fin.open(confDir + "opts.ini");
    if (!fin)
        return;

    std::string inputString;
    while(std::getline(fin, inputString))
    {
        const auto breakPoint = inputString.find("=");
        const std::string option = inputString.substr(0, breakPoint);
        const std::string value  = inputString.substr(breakPoint + 1);

        if (option.empty() || value.empty())
            continue;

        if(option == "graphicsMode")
        {
            if(value == "NORMAL")
                g_options &= ~(OPT_VECTOR | OPT_HIRES);
            else if(value == "HIRES")
            {
                g_options &= ~(OPT_VECTOR);
                g_options |= OPT_HIRES;
            }
            else if(value == "VECTOR")
            {
                g_options &= ~(OPT_HIRES);
                g_options |= OPT_VECTOR;
            }
        }
        else if(option == "stereoMode")
        {
            if(value == "STEREO")
                g_options |= OPT_STEREO;
            else if(value == "MONO")
                g_options &= ~OPT_STEREO;
        }
        else
        {
            int intval;
            const auto result = std::from_chars(value.data(), value.data() + value.size(), intval);
            if (result.ec != std::errc())
            {
                printf("error in config file at '%s=%s': invalid integer value\n", option.c_str(), value.c_str());
                continue;
            }

            if      (option == "creatureSpeed"         ) creature.creSpeedMul        = intval;
            else if (option == "volumeLevel"           ) volumeLevel                 = intval;
            else if (option == "windowMode"            ) mode                        = WindowMode(intval);
            else if (option == "screenWidth"           ) width                       = intval;
            else if (option == "creatureRegen"         ) creatureRegen               = intval;
            else if (option == "RandomMaze"            ) game.RandomMaze             = intval;
            else if (option == "ShieldFix"             ) game.ShieldFix              = intval;
            else if (option == "VisionScroll"          ) game.VisionScroll           = intval;
            else if (option == "CreaturesIgnoreObjects") game.CreaturesIgnoreObjects = intval;
            else if (option == "CreaturesInstaRegen"   ) game.CreaturesInstaRegen    = intval;
            else if (option == "MarkDoorsOnScrollMaps" ) game.MarkDoorsOnScrollMaps  = intval;
        }
    }

    fin.close();
    scheduler.updateCreatureRegen(creatureRegen);
    creature.UpdateCreSpeed();
}

bool OS_Link::saveOptFile() const
{
    using namespace std;
    ofstream fout;

    if (mkdir(confDir.c_str(), 0700) == -1 && errno != EEXIST)
    {
        printf("could not create config directory: %s\n", strerror(errno));
        return false;
    }

    fout.open(confDir + "opts.ini");
    if(!fout)
        return false;

    fout << "creatureSpeed=" << creature.creSpeedMul << endl;
    fout << "turnDelay=" << player.turnDelay << endl;
    fout << "moveDelay=" << player.moveDelay << endl;
    fout << "volumeLevel=" << volumeLevel << endl;
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

void OS_Link::loadDefaults()
{
    volumeLevel = MIX_MAX_VOLUME;
    creature.creSpeedMul = 100;
    creature.UpdateCreSpeed();
    mode = WINDOWED;
    width = 1024;
    height = 768;
    creatureRegen = 5;
    scheduler.updateCreatureRegen(creatureRegen);

    g_options &= ~(OPT_VECTOR | OPT_HIRES);
    g_options |= OPT_STEREO;
}

void OS_Link::buildSaveGamePath()
{
    memset(parser.TOKEN, -1, 33);

    gamefile = savedDir;

    if (parser.GETTOK())
    {
        size_t tctr = 0;
        while (parser.TOKEN[tctr] != 0xFF)
        {
            gamefile.push_back(parser.TOKEN[tctr] + 'A' - 1);
            ++tctr;
        }
        gamefile += ".dod";
    }
    else
    {
        gamefile += "game.dod";
    }
}

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

        const int display = SDL_GetWindowDisplayIndex(window);
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
        exit(EXIT_FAILURE);
    }

    windowed_width  = width;
    windowed_height = height;

    if (!oglctx)
    {
        oglctx = SDL_GL_CreateContext(window);
        if (!oglctx)
        {
            fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
        printf("OpenGL: %s\n", glGetString(GL_VERSION));
    }

    viewer.setup_opengl();
}

void OS_Link::changeVideoRes(int newWidth, int newHeight)
{
    const int scaledHeight = newWidth * 3 / 4;
    if (scaledHeight > newHeight)
    {
        // keep 4:3 ratio by deriving the new width from the desired height
        width  = newHeight * 4 / 3;
        height = newHeight;
    }
    else
    {
        width  = newWidth;
        height = scaledHeight;
    }

    crd.setCurWH(width);

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
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
