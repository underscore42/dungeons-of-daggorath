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

Parser::Parser()
{
    Reset();

    Utils::LoadFromHex(CERR, "177BD0");
    Utils::LoadFromHex(CMDTAB, "0F30034A046B2806C4B440200927C0380B80B52E28180E5A003012E185D42018F7AC201AFB142021563030245B142C202747DC20295938182B32802834C78480283530D8A0");
    Utils::LoadFromHex(DIRTAB, "0620185350282493A280200411AC300327D5C4102B002008FBB8");
}

void Parser::Reset()
{
    LINPTR = 0;
    FULFLG = 0;
    KBDHDR = 0;
    KBDTAL = 0;

    memset(KBDBUF, 0, sizeof(KBDBUF));
    memset(LINBUF, 0, sizeof(LINBUF));
    memset(TOKEN,  0, sizeof(TOKEN ));
    memset(STRING, 0, sizeof(STRING));
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

int Parser::PARSER(const dodBYTE * pTABLE, dodBYTE & A, dodBYTE & B, bool norm)
{
    A = 0;
    if (norm)
    {
        B = 0;
        if (!GETTOK())
            return 0;
    }

    dodBYTE retA, retB;

    const dodBYTE num_words = *pTABLE;
    ++pTABLE;

    FULFLG = 0;
    dodBYTE num_matches = 0;
    for (A = 0; A < num_words; ++A) // loop through all words in pTABLE
    {
        int Xup;
        EXPAND(pTABLE, &Xup, 0);
        pTABLE += Xup;

        // compare TOKEN against word
        for (unsigned U = 0, Y = 2;;)
        {
            B = TOKEN[U];

            const bool end_of_token = B == 0xFF;
            const bool end_of_word  = STRING[Y] == 0 || STRING[Y] == I_NULL;
            const bool match        = end_of_token
                                    ? end_of_word
                                    : B == STRING[Y];

            if (!match && !end_of_token)
                break;

            if (!end_of_token)
                ++U;

            if (!end_of_word)
                ++Y;

            if (end_of_token)
            {
                // if we get here we have at least a partial match
                // for example 'L' does match the 'LOOK' command
                ++num_matches;
                retA = A;
                retB = STRING[1];
                // check if this is actually a full match
                if (end_of_word)
                    --FULFLG;

                break;
            }
        }

        // if we find another successful match it means the input is ambiguous
        if (num_matches > 1)
            break;
    }

    if (num_matches == 1)
    {
        A = retA;
        B = retB;
        return 1;
    }
    else
    {
        A = 0xFF;
        B = 0xFF;
        return -1;
    }
}

bool Parser::GETTOK()
{
    unsigned X = LINPTR;

    // skip leading spaces
    while (LINBUF[X] == 0)
        ++X;

    unsigned U = 0;
    for (; U < 32 - X; ++U)
    {
        const dodBYTE A = LINBUF[X++];

        if (A == 0 || A == 0xFF)
            break;

        TOKEN[U] = A;
    }

    TOKEN[U] = 0xFF;
    LINPTR = X;

    return TOKEN[0] != 0xFF;
}

void Parser::EXPAND(const dodBYTE* src, int* src_offset, dodBYTE* dst)
{
    if (!dst)
    {
        dst = &STRING[0];
        ++dst;
    }

    const dodBYTE* const src_beg = src;

    // first 5 bits contain the length of the expanded string. add one for the
    // string termination character.
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
    dodBYTE A, B;

    const int res = PARSER(DIRTAB, A, B, true);
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

