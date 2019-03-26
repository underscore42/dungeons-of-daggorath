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
// Filename: parser.cpp
//
// Implementation of Parser class

#include "parser.h"
#include "viewer.h"

extern Viewer viewer;

// Constructor
Parser::Parser() : LINPTR(0),
    PARFLG(0),
    PARCNT(0),
    FULFLG(0),
    KBDHDR(0),
    KBDTAL(0),
    LINEND(0),
    TOKEND(0)
{
    int ctr;
    for (ctr = 0; ctr < 33; ++ctr)
    {
        KBDBUF[ctr] = 0;
        LINBUF[ctr] = 0;
        TOKEN[ctr] = 0;
        OBJSTR[ctr] = 0;
        STRING[ctr] = 0;
    }
    STRING[33] = 0;
    STRING[34] = 0;

    M_PROM1[0] = I_CR;
    M_PROM1[1] = I_DOT;
    M_PROM1[2] = I_BAR;
    M_PROM1[3] = I_BS;
    M_PROM1[4] = I_NULL;

    M_CURS[0] = I_BAR;
    M_CURS[1] = I_BS;
    M_CURS[2] = I_NULL;

    M_ERAS[0] = I_SP;
    M_ERAS[1] = I_BS;
    M_ERAS[2] = I_BS;
    M_ERAS[3] = I_BAR;
    M_ERAS[4] = I_BS;
    M_ERAS[5] = I_NULL;

    Utils::LoadFromHex(CERR, "177BD0");
    Utils::LoadFromHex(CMDTAB, "0F30034A046B2806C4B440200927C0380B80B52E28180E5A003012E185D42018F7AC201AFB142021563030245B142C202747DC20295938182B32802834C78480283530D8A0");
    Utils::LoadFromHex(DIRTAB, "0620185350282493A280200411AC300327D5C4102B002008FBB8");
}

void Parser::Reset()
{
    LINPTR = 0;
    PARFLG = 0;
    PARCNT = 0;
    FULFLG = 0;
    KBDHDR = 0;
    KBDTAL = 0;
    LINEND = 0;
    TOKEND = 0;
    int ctr;
    for (ctr = 0; ctr < 33; ++ctr)
    {
        KBDBUF[ctr] = 0;
        LINBUF[ctr] = 0;
        TOKEN[ctr] = 0;
        OBJSTR[ctr] = 0;
        STRING[ctr] = 0;
    }
    STRING[33] = 0;
    STRING[34] = 0;
}

void Parser::KBDPUT(dodBYTE c)
{
    KBDBUF[KBDTAL] = c;
    ++KBDTAL;
    KBDTAL &= 31;
}

dodBYTE Parser::KBDGET()
{
    dodBYTE c = 0;
    if (KBDHDR == KBDTAL)
        return c;
    c = KBDBUF[KBDHDR];
    ++KBDHDR;
    KBDHDR &= 31;
    return c;
}

int Parser::PARSER(dodBYTE * pTABLE, dodBYTE & A, dodBYTE & B, bool norm)
{
    int     U, Xup, Y;
    dodBYTE retA, retB;

    if (norm)
    {
        A = 0;
        B = 0;
        if (!GETTOK())
        {
            return 0;
        }
    }
    else
    {
        A = 0;
    }

    PARFLG = 0;
    FULFLG = 0;
    B = *pTABLE;
    ++pTABLE;
    PARCNT = B;

PARS10:
    U = 0;
    EXPAND(pTABLE, &Xup, 0);
    pTABLE += Xup;
    Y = 2;

PARS12:
    B = TOKEN[U++];
    if (B == 0xFF)
    {
        goto PARS20;
    }
    if (B != STRING[Y++])
    {
        goto PARS30;
    }
    if (STRING[Y] != I_NULL && STRING[Y] != 0)
    {
        goto PARS12;
    }
    if (TOKEN[U] != 0xFF && TOKEN[U] != 0)
    {
        goto PARS30;
    }
    --FULFLG;

PARS20:
    if (PARFLG != 0)
    {
        goto PARS90;
    }
    ++PARFLG;
    B = STRING[1];
    retA = A;
    retB = B;

PARS30:
    ++A;
    --PARCNT;
    if (PARCNT != 0)
    {
        goto PARS10;
    }

    if (PARFLG != 0)
    {
        A = retA;
        B = retB;
        return 1;
    }

PARS90:
    A = 0xFF;
    B = 0xFF;
    return -1;
}

bool Parser::GETTOK()
{
    int     U = 0;
    int     X = LINPTR;
    dodBYTE A;

    do
    {
        A = LINBUF[X++];
    }
    while (A == 0);
    goto GTOK22;

GTOK20:
    A = LINBUF[X++];

GTOK22:
    if (A == 0 || A == 0xFF)
    {
        goto GTOK30;
    }
    TOKEN[U++] = A;
    if (U < 32)
    {
        goto GTOK20;
    }

GTOK30:
    TOKEN[U++] = 0xFF;
    LINPTR = X;

    if (TOKEN[0] == 0xFF)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Parser::EXPAND(const dodBYTE* src, int* src_offset, dodBYTE* dst)
{
    if (!dst)
    {
        dst = &STRING[0];
        ++dst;
    }

    const dodBYTE* const src_beg = src;

    // first 5 bits contain the length of the expanded string minus one
    const unsigned total_len = (*src >> 3) + 1;

    unsigned offset = 1; // at which 5 bit char are we

    for (unsigned i = 0; i < total_len; ++i)
    {
        // wrap around because the offset pattern repeats after 8 * 5 bit chars
        offset &= 7;

        switch (offset)
        {
            case 0:
                *dst = *src >> 3;
                break;
            case 1:
                *dst = (*src << 2) & 0x1f;
                ++src;
                *dst |= *src >> 6;
                break;
            case 2:
                *dst = (*src >> 1) & 0x1f;
                break;
            case 3:
                *dst = (*src << 4) & 0x1f;
                ++src;
                *dst |= *src >> 4;
                break;
            case 4:
                *dst = (*src << 1) & 0x1f;
                ++src;
                *dst |= *src >> 7;
                break;
            case 5:
                *dst = (*src >> 2) & 0x1f;
                break;
            case 6:
                *dst = (*src << 3) & 0x1f;
                ++src;
                *dst |= *src >> 5;
                break;
            case 7:
                *dst = *src & 0x1f;
                ++src;
                break;
        }
        ++dst;
        ++offset;
    }

    *dst = I_NULL;
    *src_offset = src - src_beg;
    if (offset < 8)
        ++*src_offset;
}

void Parser::CMDERR() const
{
     viewer.OUTSTI(CERR);
}

int Parser::PARHND()
{
    int     res;
    dodBYTE A, B;

    res = PARSER(DIRTAB, A, B, true);
    if (res != 1)
    {
        CMDERR();
        return -1;
    }
    if (A == 0 || A == 1)
    {
        return A;
    }
    else
    {
        CMDERR();
        return -1;
    }
}

