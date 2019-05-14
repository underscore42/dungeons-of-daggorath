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
// Filename: parser.h
//
// This class will manage the command parser

#ifndef DOD_PARSER_HEADER
#define DOD_PARSER_HEADER

#include "dod.h"

class Parser
{
public:
    // Constructor
    Parser();

    void Reset();

    // This method puts a character into the DoD buffer
    void KBDPUT(dodBYTE c);

    // This method gets a character from the DoD buffer
    dodBYTE KBDGET();

    // Expands packed string src into dst and stores how far we read from src in
    // src_offset. If dst is 0 then the string will be expanded into the
    // internal STRING buffer.
    void EXPAND(const dodBYTE* src, int* src_offset, dodBYTE* dst);

    // Stores next token from LINBUF in TOKEN, returns true if there was a token
    bool GETTOK();

    // Gets and classifies the next token
    // A = TYPE, B = CLASS
    int PARSER(const dodBYTE * X, dodBYTE &A, dodBYTE &B, bool norm);

    // Prints ???
    void CMDERR() const;

    // Parses LEFT or RIGHT hand
    int PARHND();

    static constexpr size_t MAX_TOKENLEN = 33;

    // Pointer into LINBUF
    dodSHORT LINPTR;

    // if we found a full match instead of a partial match in PARSER()
    dodBYTE FULFLG;

    // Read pointer into KBDBUF
    dodBYTE KBDHDR;

    // Write pointer into KBDBUF
    dodBYTE KBDTAL;

    dodBYTE KBDBUF[33];
    dodBYTE LINBUF[33];
    dodBYTE TOKEN[MAX_TOKENLEN];
    dodBYTE STRING[35];
    dodBYTE CMDTAB[69];
    dodBYTE DIRTAB[26];

    enum
    {
        C_BS = 0x08,
        C_CR = 0x0D,
        C_SP = 0x20,

        I_SP = 0x00,
        I_BAR = 0x1C,
        I_DOT = 0x1E,
        I_CR = 0x1F,
        I_EXCL = 0x1B,
        I_QUES = 0x1D,
        I_SHL = 0x20,
        I_SHR = 0x21,
        I_LHL = 0x22,
        I_LHR = 0x23,
        I_BS = 0x24,
        I_NULL = 0xff, // string terminator char

        CMD_ATTACK = 0,
        CMD_CLIMB,
        CMD_DROP,
        CMD_EXAMINE,
        CMD_GET,
        CMD_INCANT,
        CMD_LOOK,
        CMD_MOVE,
        CMD_PULL,
        CMD_REVEAL,
        CMD_STOW,
        CMD_TURN,
        CMD_USE,
        CMD_ZLOAD,
        CMD_ZSAVE,

        DIR_LEFT = 0,
        DIR_RIGHT,
        DIR_BACK,
        DIR_AROUND,
        DIR_UP,
        DIR_DOWN,
    };

    // The prompt '._'
    dodBYTE M_PROM1[5] = {I_CR, I_DOT, I_BAR, I_BS, I_NULL};

    // The cursor '_'
    dodBYTE M_CURS[3] = {I_BAR, I_BS, I_NULL};

    // Used when player presses backspace
    dodBYTE M_ERAS[6] = {I_SP, I_BS, I_BS, I_BAR, I_BS, I_NULL};

    // The three ? that get printed to indicate an input error
    dodBYTE CERR[3];
};

#endif // DOD_PARSER_HEADER
