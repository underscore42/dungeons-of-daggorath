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
// Filename: object.h
//
// This class manages the objects (torches, etc.) in the
// game.

#ifndef DOD_OBJECT_HEADER
#define DOD_OBJECT_HEADER

#include "dod.h"

class Object
{
public:
    Object();

    void Reset();

    void LoadSounds();

    // Creates all the objects during initialization
    void CreateAll();

    // Finds objects in the OCB table
    int FNDOBJ();

    // Returns the object's name at index idx in Parser::TOKEN.
    void OBJNAM(int idx) const;

    // Finds object on the floor in a cell. Returns the index of the object or
    // -1.
    int OFIND(RowCol rc);

    // Resets OFINDP so that FNDOBJ() starts searching from the beginning again
    void resetFindPointer() { OFINDP = -1; }

    // Creates new object
    int OBIRTH(dodBYTE OBJTYP, dodBYTE OBJLVL);

    // Fills in default values for object
    void OCBFIL(dodBYTE OBJTYP, int ptr);

    // Parses an object name. Returns true if parsed successfully.
    // is_specific is true if it's a specific object like PINE TORCH and not
    // just TORCH.
    bool PAROBJ(dodBYTE& objclass, dodBYTE& objtype, bool& is_specific) const;

    // Holds all the created objects
    OCB OCBLND[72];

    // Adjective table (BRONZE, LEATHER, WOOD, ...)
    dodBYTE ADJTAB[119];

    // Generic object name table (FLASK, RING, SHIELD, ...)
    dodBYTE GENTAB[30];

    // weights for the six object classes
    int OBJWGT[6] = {0x05, 0x01, 0x0A, 0x19, 0x19, 0x0A};

    int objChannel = 1;
    Mix_Chunk * objSound[6];

    enum object_type_t
    {
        OBJ_RING_SUPREME   =  0,
        OBJ_RING_JOULE     =  1,
        OBJ_SWORD_ELVISH   =  2,
        OBJ_SHIELD_MITHRIL =  3,
        OBJ_SCROLL_SEER    =  4,
        OBJ_FLASK_THEWS    =  5,
        OBJ_RING_RIME      =  6,
        OBJ_SCROLL_VISION  =  7,
        OBJ_FLASK_ABYE     =  8,
        OBJ_FLASK_HALE     =  9,
        OBJ_TORCH_SOLAR    = 10,
        OBJ_SHIELD_BRONZE  = 11,
        OBJ_RING_VULCAN    = 12,
        OBJ_SWORD_IRON     = 13,
        OBJ_TORCH_LUNAR    = 14,
        OBJ_TORCH_PINE     = 15,
        OBJ_SHIELD_LEATHER = 16,
        OBJ_SWORD_WOOD     = 17,
        OBJ_RING_FINAL     = 18,    // incanted
        OBJ_RING_ENERGY    = 19,    // incanted
        OBJ_RING_ICE       = 20,    // incanted
        OBJ_RING_FIRE      = 21,    // incanted
        OBJ_RING_GOLD      = 22,
        OBJ_FLASK_EMPTY    = 23,
        OBJ_TORCH_DEAD     = 24
    };

    enum object_class_t
    {
        OBJT_FLASK  = 0,
        OBJT_RING   = 1,
        OBJT_SCROLL = 2,
        OBJT_SHIELD = 3,
        OBJT_WEAPON = 4,
        OBJT_TORCH  = 5
    };

    // points to the end of the object list in OCBLND
    int OCBPTR;

    // index into OCBLND at which to start searching in FNDOBJ()
    int OFINDP;

private:
    // Contains all base stats for all object types.
    ODB ODBTAB[25];

    // Contains extra stats for rings, shields and torches.
    XDB XXXTAB[11];

    // object distribution table: each byte specifies how many objects of a
    // certain type exist across all dungeon levels and at which level they
    // start to appear.
    dodBYTE OMXTAB[18];

    // Unrevealed objects behave like their most basic instance, i.e. a bronze
    // shield behaves like a leather shield. This array maps all object classes
    // to their basic instance.
    dodBYTE GENVAL[6] = {0xFF, 0xFF, 0xFF, 0x10, 0x11, 0x0F};
};

#endif // DOD_OBJECT_HEADER
