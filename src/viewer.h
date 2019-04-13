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
// Filename: viewer.h
//
// This class manages drawing to the screen, including
// setting up the OpenGL data.  It's a work in progress.
//
// At the moment it is huge, which means that it really
// needs to be broken into smaller, more well-defined
// classes.  But it works for the present.

#ifndef DOD_VIEWER_HEADER
#define DOD_VIEWER_HEADER

#include "dod.h"
#include "dodgame.h"


class Viewer
{
public:

    Viewer();

    void Reset();

    void setup_opengl();

    // This is the main renderer routine. It draws either the map, or the
    // 3D/Examine-Status-Text Area.
    void draw_game();

    // This is the renderer method used to do the wizard fade in/out. It's only
    // used during the opening. It is syncronized with the 30Hz buzz and the
    // wizard crashing sound.
    bool draw_fade();

    // Same as above, but used for the intermission
    void enough_fade();

    // Same as above, but used for death & victory
    void death_fade(const int* WIZ);

    // Display various messages
    void displayCopyright();
    void displayWelcomeMessage();
    void displayDeath();
    void displayWinner();
    void displayEnough();
    void displayPrepare();

    // Draws a text block
    void drawArea(const TXB * a) const;

    // Fills a text block with spaces
    void clearArea(TXB * a) const;

    // A torch that is in use will show up highlighted in the EXAMINE screen.
    void drawTorchHighlite() const;

    // This method updates the screen if necessary. Called from the scheduler
    // every 3 tenths of a second.
    int LUKNEW();

    // Updates the screen.
    void PUPDAT();

    // Sets lighting values.
    void PUPSUB();

    // Updates the Left and Right hand in the status line
    void STATUS();

    // Show prompt (a dot and an underscore)
    void PROMPT();

    // Fills TXTEXA with the text for the EXAMINE screen.
    void EXAMIN();

    // Put a newline in current text block
    void PCRLF();

    // Prints the object at index X to the current text block
    void PRTOBJ(int X, bool highlite);

    // Print compressed string to primary or current text block
    void OUTSTI(const dodBYTE * comp);

    // Print string to primary or current text block
    void OUTSTR(const dodBYTE * str);

    // Outputs character to primary or current text block
    void OUTCHR(dodBYTE c);

    // Put a character in current text block
    void TXTXXX(dodBYTE c);

    // Scrolls the text area up by one line
    void TXTSCR();

    // This is the 3D-Viewport rendering routine
    void VIEWER();

    // Sets the perspective scale
    void SETSCL();

    // Used by 3D-Viewer, draws a vector list
    void DRAWIT(const int * vl);

    // Used by 3D-Viewer, checks for around-the-corner creature
    void PDRAW(const int * vl, dodBYTE dir, dodBYTE pdir);

    // Prepares for drawing creature with either magical or physical lighting
    void CMRDRW(const int * vl, int creNum);

    // Calculates fade (line-pixelation) based on lighting
    void SETFAD();

    // Scales X-coordinate
    dodSHORT ScaleX(int x) const { return ((x - VCNTRX) * VXSCAL) / 127; }
    float ScaleXf(float x) const { return ((x - VCNTRX) * VXSCALf) / 127.0f; }

    // Scales Y-coordinate
    dodSHORT ScaleY(int y) const { return ((y - VCNTRY) * VYSCAL) / 127; }
    float ScaleYf(float y) const { return ((y - VCNTRY) * VYSCALf) / 127.0f; }

    // Draws the map; showSeerMap bool determines VISION or SEER mode
    void MAPPER();

    void setVidInv(bool inv);

    // Draws non-font vector lists
    void drawVectorList(const int* VLA) const;

    // Draws a line
    void drawVector(float X0, float Y0, float X1, float Y1) const;

    // This method renders the wizard fade in/out animations.
    // The parameter fadeMode indicates which of the four fades
    // to do:
    //   1 = Beginning before game starts
    //   2 = Intermission after killing wizards image
    //   3 = Death
    //   4 = Victory
    bool ShowFade(int fadeMode);

    // Draws the menu
    // mainMenu  - menu instance to draw
    // menu_id   - the menu number to draw
    // highlight - the menu item to highlight
    void drawMenu(const menu& mainMenu, int menu_id, int highlight) const;

    // Draws a menu list
    // x         - the top-left x-coordinate
    // y         - the top-left y-coordinate
    // title     - the title of the list
    // list      - the list to be drawn
    // listSize  - the size of the list
    // highlight - the item that's highlighted
    void drawMenuList(int x, int y, const char *title, const char *list[], int listSize, int highlight) const;

    // Draws a menu scroll bar
    // title   - the title of the scrollbar
    // current - the current percentage
    void drawMenuScrollbar(const char *title , int current) const;

    // Vars:   title - the title of the string box
    // Draws a menu string box
    void drawMenuStringTitle(const char *) const;

    // Draws a menu string box
    // Vars:   title - the title of the string box
    void drawMenuString(const char *title) const;

    // Draws the "About" Box
    void aboutBox() const;

public:

    dodBYTE     VCTFAD;
    dodBYTE     RANGE;
    bool        showSeerMap;
    Uint32      delay, delay1, delay2;
    bool        done;           // if we're done fading
    int         fadeVal;        // how much we fade with each iteration
    dodBYTE     UPDATE;
    dodSHORT    display_mode;   // 0 = map, 1 = 3D, 2 = Examine, 3 = Prepare

    static constexpr int fadChannel = 3;    // channel for fade sound effect
    static constexpr int buzzStep  = 300;
    static constexpr int midPause  = 2500;
    static constexpr int prepPause = 2500;

    dodBYTE     Scale[21];
    float       Scalef[21];
    int *       LArch[4];
    int *       FArch[4];
    int *       RArch[4];

    char        textArea[(32 * 4) + 1];
    char        examArea[(32 * 19) + 1];
    char        statArea[(32 * 1) + 1];

    TXB         TXTPRI;     // Primary TXB where user can input text
    TXB         TXTEXA;     // TXB for the EXAMINE screen
    TXB         TXTSTS;     // TXB for the status line

    GLfloat     bgColor[3];
    GLfloat     fgColor[3];
    dodBYTE     RLIGHT;
    dodBYTE     MLIGHT;
    dodBYTE     OLIGHT;
    dodBYTE     VXSCAL;
    dodBYTE     VYSCAL;
    float       VXSCALf;
    float       VYSCALf;

    dodBYTE     TXBFLG;
    TXB *       TXB_U;
    int         tcaret;
    int         tlen;

    dodBYTE     enough1[21];
    dodBYTE     enough2[20];
    dodBYTE     winner1[21];
    dodBYTE     winner2[17];
    dodBYTE     death[21];
    dodBYTE     copyright[21];
    dodBYTE     welcome1[14];
    dodBYTE     welcome2[20];
    dodBYTE     prepstr[6];
    dodBYTE     exam1[9];
    dodBYTE     exam2[8];
    dodBYTE     exam3[7];

    int         LINES[11];

    dodBYTE     HLFSTP;
    dodBYTE     BAKSTP;
    dodBYTE     NEWLIN;

    enum
    {
        MODE_MAP = 0,
        MODE_3D,
        MODE_EXAMINE,
        MODE_TITLE,

        FADE_BEGIN = 1,
        FADE_MIDDLE,
        FADE_DEATH,
        FADE_VICTORY,
    };

private:

    // Draws font vectors
    void drawVectorListAQ(const int* VLA) const;

    // Draws a character
    void drawCharacter(char c) const;

    // Draws a string
    void drawString(int x, int y, const char * str, int len) const;

    void drawString_internal(int x, int y, const dodBYTE * str, int len) const;

    char dod_to_ascii(dodBYTE c) const;

    const dodSHORT VCNTRX;
    const dodSHORT VCNTRY;
    dodBYTE     MAGFLG;
    int         HLFSCL;
public:
    // letters
    int A_VLA[33];
    int B_VLA[49];
    int C_VLA[41];
    int D_VLA[33];
    int E_VLA[33];
    int F_VLA[25];
    int G_VLA[49];
    int H_VLA[25];
    int I_VLA[25];
    int J_VLA[25];
    int K_VLA[65];
    int L_VLA[17];
    int M_VLA[41];
    int N_VLA[41];
    int O_VLA[33];
    int P_VLA[33];
    int Q_VLA[57];
    int R_VLA[57];
    int S_VLA[57];
    int T_VLA[33];
    int U_VLA[25];
    int V_VLA[41];
    int W_VLA[41];
    int X_VLA[73];
    int Y_VLA[41];
    int Z_VLA[57];
    // numbers
    int NM0_VLA[33];
    int NM1_VLA[25];
    int NM2_VLA[49];
    int NM3_VLA[57];
    int NM4_VLA[33];
    int NM5_VLA[49];
    int NM6_VLA[41];
    int NM7_VLA[49];
    int NM8_VLA[57];
    int NM9_VLA[49];
    // special characters
    int PER_VLA[9];
    int UND_VLA[9];
    int EXP_VLA[17];
    int QSM_VLA[49];
    int SHL_VLA[9];
    int SHR_VLA[33];
    int LHL_VLA[17];
    int LHR_VLA[41];
    int FSL_VLA[41];    //Forward Slash
    int BSL_VLA[41];    //Back Slash
    int PCT_VLA[57];    //Percent
    int PLS_VLA[17];    //Plus
    int DSH_VLA[9];     //Dash
    int * AZ_VLA[50];

    // creatures
    int SP_VLA[39];
    int WR_VLA[42];
    int SC_VLA[41];
    int BL_VLA[66];
    int GL_VLA[141];
    int VI_VLA[65];
    int S1_VLA[130];
    int S2_VLA[126];
    int K1_VLA[153];
    int K2_VLA[149];
    int W0_VLA[133];

    // wizard with crescent shaped sceptre point
    int W1_VLA[199];
    // wizard with star shaped sceptre point
    int W2_VLA[185];

    // ladder
    int LAD_VLA[56];
    // hole up
    int HUP_VLA[29];
    // hole down
    int HDN_VLA[19];

    // ceiling
    int CEI_VLA[6];

    // left peekaboo shape
    int LPK_VLA[12];
    // right peekaboo shape
    int RPK_VLA[12];

    // forward secret door
    int FSD_VLA[8];
    // left secret door
    int LSD_VLA[8];
    // right secret door
    int RSD_VLA[8];
    // right wall
    int RWAL_VLA[10];
    // left wall
    int LWAL_VLA[10];
    // forward wall
    int FWAL_VLA[11];
    // right passage
    int RPAS_VLA[15];
    // left passage
    int LPAS_VLA[15];
    // forwards passage
    int FPAS_VLA[1];
    // right door
    int RDOR_VLA[24];
    // left door
    int LDOR_VLA[24];
    // forward door
    int FDOR_VLA[25];

    int FLATAB[3];
    int ** FLATABv[3];

    // object lookup
    int * FWDOBJ[6];

    // creatures
    int * FWDCRE[12];

    // objects
    int SHIE_VLA[14];
    int SWOR_VLA[11];
    int TORC_VLA[10];
    int RING_VLA[12];
    int SCRO_VLA[12];
    int FLAS_VLA[10];
};

#endif // DOD_VIEWER_HEADER
