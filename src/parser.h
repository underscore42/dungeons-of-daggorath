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
    int PARSER(dodBYTE * X, dodBYTE &A, dodBYTE &B, bool norm);

    // Prints ???
    void CMDERR() const;

    // Parses LEFT or RIGHT hand
    int PARHND();

    static constexpr size_t MAX_TOKENLEN = 33;

    // Public Data Member
    dodSHORT    LINPTR;
    dodBYTE     PARFLG;
    dodBYTE     PARCNT;
    dodBYTE     VERIFY;
    dodBYTE     FULFLG;
    dodBYTE     KBDHDR;
    dodBYTE     KBDTAL;
    dodBYTE     BUFFLG;
    dodBYTE     KBDBUF[33];
    dodBYTE     LINBUF[33];
    dodSHORT    LINEND;
    dodBYTE     TOKEN[MAX_TOKENLEN];
    dodBYTE     TOKEND;
    dodBYTE     STRING[35];
    dodBYTE     SWCHAR[11];
    dodBYTE     OBJSTR[33];
    dodBYTE     CMDTAB[69];
    dodBYTE     DIRTAB[26];

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

    dodBYTE M_PROM1[5];
    dodBYTE M_CURS[3];
    dodBYTE M_ERAS[6];
    dodBYTE CERR[3];
};

#endif // DOD_PARSER_HEADER
