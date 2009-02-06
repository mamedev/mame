// new code to allow the PinMAME rom loading to compile

#define NORMALREGION(size, reg)  ROM_REGION(size, reg, ROMREGION_ERASE00)
#define NORMALREGIONE(size, reg) ROM_REGION(size, reg, ROMREGION_ERASE00)
#define SOUNDREGION(size ,reg)   ROM_REGION(size, reg, ROMREGION_ERASE00)
#define SOUNDREGIONE(size ,reg)  ROM_REGION(size, reg, ROMREGION_ERASE00)

/*-----------------
/  define the game
/------------------*/
#define CORE_GAMEDEF(name, ver, longname, year, manuf, machine, flag) \
  GAME(year,name##_##ver,0,machine,name,name,ROT0,manuf,longname,flag | GAME_NOT_WORKING | GAME_NO_SOUND )


#define CORE_CLONEDEF(name, ver, clonever, longname, year, manuf, machine,flag) \
  GAME(year,name##_##ver,name##_##clonever,machine,name,name,ROT0,manuf,longname,flag | GAME_NOT_WORKING | GAME_NO_SOUND)

#if 0
#define CORE_GAMEDEFNV(name, longname, year, manuf, machine, flag) \
  GAME(year,name,0,machine,name,name,ROT0,manuf,longname,flag | GAME_NOT_WORKING | GAME_NO_SOUND)
  
#define CORE_CLONEDEFNV(name, cl, longname, year, manuf, machine,flag) \
  GAME(year,name,cl,machine,name,name,ROT0,manuf,longname,flag | GAME_NOT_WORKING | GAME_NO_SOUND)

#define CORE_GAMEDEFNVR90(name, longname, year, manuf, machine, flag) \
  GAME(year,name,0,machine,name,name,ROT90,manuf,longname,flag | GAME_NOT_WORKING | GAME_NO_SOUND)
#endif  

/************************************************************************************************
*************************************************************************************************
 Old PinMAME code below for reference ONLY
*************************************************************************************************
************************************************************************************************/

#if 0

#ifndef INC_CORE
#define INC_CORE

#include "wpcsam.h"
#include "gen.h"
#include "sim.h"

/*-- some convenience macros --*/
#ifndef FALSE
  #define FALSE (0)
#endif
#ifndef TRUE
  #define TRUE (1)
#endif

#ifdef MAME_DEBUG
  #define DBGLOG(x) logerror x
#else
  #define DBGLOG(x)
#endif



/*-- convenience macro for handling bits --*/
#define GET_BIT0 (data & 0x01) >> 0
#define GET_BIT1 (data & 0x02) >> 1
#define GET_BIT2 (data & 0x04) >> 2
#define GET_BIT3 (data & 0x08) >> 3
#define GET_BIT4 (data & 0x10) >> 4
#define GET_BIT5 (data & 0x20) >> 5
#define GET_BIT6 (data & 0x40) >> 6
#define GET_BIT7 (data & 0x80) >> 7


/*-- default screen size */
#ifdef VPINMAME
#  define CORE_SCREENX 640
#  define CORE_SCREENY 400
#else /* VPINMAME */
#  define CORE_SCREENX 320
#  define CORE_SCREENY 256
#endif /* VPINMAME */


/*--------------
/  Input ports
/---------------*/
/* strange but there are no way to define IMP and TOG with key without using BITX */
#define COREPORT_BIT(mask, name, key) \
   PORT_BITX(mask,IP_ACTIVE_HIGH,IPT_BUTTON1,name,key,IP_JOY_NONE)
#define COREPORT_BITIMP(mask, name, key) \
   PORT_BITX(mask,IP_ACTIVE_HIGH,IPT_BUTTON1 | IPF_IMPULSE | (1<<8),name,key,IP_JOY_NONE)
#define COREPORT_BITTOG(mask, name, key) \
   PORT_BITX(mask,IP_ACTIVE_HIGH,IPT_BUTTON1 | IPF_TOGGLE,name,key,IP_JOY_NONE)
#define COREPORT_DIPNAME(mask,default,name) \
   PORT_DIPNAME(mask,default,name)
#define COREPORT_DIPSET(mask,name) \
   PORT_DIPSETTING(mask,name)

/*-- only used in standard inport --*/
#define COREPORT_BITDEF(mask, type, key) \
   PORT_BITX(mask,IP_ACTIVE_HIGH, type, IP_NAME_DEFAULT, key, IP_JOY_DEFAULT)

/*----------------
/  Common inports
/-----------------*/
#define CORE_PORTS \
  PORT_START /* 0 */ \
    COREPORT_BIT(0x0001, "Column 1",  KEYCODE_Q) \
    COREPORT_BIT(0x0002, "Column 2",  KEYCODE_W) \
    COREPORT_BIT(0x0004, "Column 3",  KEYCODE_E) \
    COREPORT_BIT(0x0008, "Column 4",  KEYCODE_R) \
    COREPORT_BIT(0x0010, "Column 5",  KEYCODE_T) \
    COREPORT_BIT(0x0020, "Column 6",  KEYCODE_Y) \
    COREPORT_BIT(0x0040, "Column 7",  KEYCODE_U) \
    COREPORT_BIT(0x0080, "Column 8",  KEYCODE_I) \
    COREPORT_BIT(0x0100, "Row 1",     KEYCODE_A) \
    COREPORT_BIT(0x0200, "Row 2",     KEYCODE_S) \
    COREPORT_BIT(0x0400, "Row 3",     KEYCODE_D) \
    COREPORT_BIT(0x0800, "Row 4",     KEYCODE_F) \
    COREPORT_BIT(0x1000, "Row 5",     KEYCODE_G) \
    COREPORT_BIT(0x2000, "Row 6",     KEYCODE_H) \
    COREPORT_BIT(0x4000, "Row 7",     KEYCODE_J) \
    COREPORT_BIT(0x8000, "Row 8",     KEYCODE_K) \
  PORT_START /* 1 */ \
    COREPORT_BIT(0x0001, "Left Flipper",   KEYCODE_LSHIFT)  \
    COREPORT_BIT(0x0002, "Right Flipper",  KEYCODE_RSHIFT) \
    COREPORT_BIT(0x0004, "U Left Flipper",  KEYCODE_A)  \
    COREPORT_BIT(0x0008, "Y Right Flipper", KEYCODE_L)

/*-----------------------
/ Access to common ports
/------------------------*/
/*-- manual switch keys --*/
#define CORE_MANSWINPORT    0
#define CORE_MANSWCOLUMNS   0x00ff
#define CORE_MANSWROWS      0xff00

/*-- common keys (start, tilt etc) --*/
#define CORE_FLIPINPORT     1
#define CORE_LLFLIPKEY      0x0001
#define CORE_LRFLIPKEY      0x0002
#define CORE_ULFLIPKEY      0x0004
#define CORE_URFLIPKEY      0x0008

#define CORE_SIMINPORT      1  /* Inport for simulator */
#define CORE_COREINPORT     2  /* Inport for core use */

// Macro to ease switch assignment
#define CORE_SETKEYSW(value, mask, swcol) \
  coreGlobals.swMatrix[(swcol)] = (coreGlobals.swMatrix[(swcol)] & ~(mask)) | ((value) & (mask))

/*------------------------------------------------------
/ Flipper hardware is described with the following macros
/  (macros use FLIP_LL, FLIP_LR, FLIP_UL, FLIP_UR)
/ FLIP_SW()      Flipper switches available
/ FLIP_SWNO(L,R) Flipper switch numbers if other than default (Pre-fliptronics)
/ FLIP_SOL()     CPU controlled flippers
/ Example: CPU controlled upper right flipper
/   FLIP_SW(FLIP_LL | FLIP_LR | FLIP_UR) + FLIP_SOL(FLIP_LL | FLIP_LR | FLIP_UR)
/ Example: Flippers not controlled by CPU
/   FLIP_SWNO(swFlipL, swFlipR)
/--------------------------------------------------------------*/
/*-- flipper names --*/
#define FLIP_LR        (0x1)
#define FLIP_LL        (0x2)
#define FLIP_UR        (0x4)
#define FLIP_UL        (0x8)
#define FLIP_L         (FLIP_LL | FLIP_LR)
#define FLIP_U         (FLIP_UL | FLIP_UR)

/*-- definition macros --*/
#define FLIP_BUT(x)    ((x)<<16)
#define FLIP_SW(x)     ((x)<<20)
#define FLIP_SWNO(l,r) (((l)<<8)|(r)|FLIP_SW(FLIP_L))
#define FLIP_EOS(x)    ((x)<<28)
#define FLIP_SOL(x)    (((x)<<24)|FLIP_EOS(x))

#define FLIP_SWL(x)    (((x)>>8)&0xff)
#define FLIP_SWR(x)    ((x)&0xff)

/*---------------------
/  Exported variables
/----------------------*/
#define CORE_FLIPSTROKETIME 2 /* Timer for flipper to reach top VBLANKs */

/*-----------------------------
/  Generic Display layout data
/------------------------------*/
/* The different kind of display units */
#define CORE_SEG16    0 // 16 segments
#define CORE_SEG16R   1 // 16 segments with comma and period reversed
#define CORE_SEG10    2 // 9  segments and comma
#define CORE_SEG9     3 // 9  segments
#define CORE_SEG8     4 // 7  segments and comma
#define CORE_SEG8D    5 // 7  segments and period
#define CORE_SEG7     6 // 7  segments
#define CORE_SEG87    7 // 7  segments, comma every three
#define CORE_SEG87F   8 // 7  segments, forced comma every three
#define CORE_SEG98    9 // 9  segments, comma every three
#define CORE_SEG98F  10 // 9  segments, forced comma every three
#define CORE_SEG7S   11 // 7  segments, small
#define CORE_SEG7SC  12 // 7  segments, small, with comma
#define CORE_SEG16S  13 // 16 segments with split top and bottom line
#define CORE_DMD     14 // DMD Display
#define CORE_VIDEO   15 // VIDEO Display

#define CORE_IMPORT   0x10 // Link to another display layout
#define CORE_SEGHIBIT 0x20
#define CORE_SEGREV   0x40
#define CORE_DMDNOAA  0x80
#define CORE_SEGMASK  0x1f // Note that CORE_IMPORT must be part of the segmask as well!

#define CORE_SEG8H    (CORE_SEG8  | CORE_SEGHIBIT)
#define CORE_SEG7H    (CORE_SEG7  | CORE_SEGHIBIT)
#define CORE_SEG87H   (CORE_SEG87 | CORE_SEGHIBIT)
#define CORE_SEG87FH  (CORE_SEG87F| CORE_SEGHIBIT)
#define CORE_SEG7SH   (CORE_SEG7S | CORE_SEGHIBIT)
#define CORE_SEG7SCH  (CORE_SEG7SC| CORE_SEGHIBIT)

#define DMD_MAXX 256
#define DMD_MAXY 64

typedef UINT8 tDMDDot[DMD_MAXY+2][DMD_MAXX+2];

/* Shortcuts for some common display sizes */
#define DISP_SEG_16(row,type)    {4*row, 0, 20*row, 16, type}
#define DISP_SEG_7(row,col,type) {4*row,16*col,row*20+col*8+1,7,type}
#define DISP_SEG_CREDIT(no1,no2,type) {2,2,no1,1,type},{2,4,no2,1,type}
#define DISP_SEG_BALLS(no1,no2,type)  {2,8,no1,1,type},{2,10,no2,1,type}
#define DISP_SEG_IMPORT(x) {0,0,0,1,CORE_IMPORT,x}
/* display layout structure */
/* Don't know how the LCD got in there. Should have been LED but now it
   handles all kinds of displays so we call it dispLayout.
   Keep the typedef of core_tLCDLayout for some time. */
struct core_dispLayout {
  UINT16 top, left, start, length, type;
  void *ptr;
};
typedef struct core_dispLayout core_tLCDLayout, *core_ptLCDLayout;

#define PINMAME_VIDEO_UPDATE(name) int (name)(struct mame_bitmap *bitmap, const struct rectangle *cliprect, const struct core_dispLayout *layout)
typedef int (*ptPinMAMEvidUpdate)(struct mame_bitmap *bitmap, const struct rectangle *cliprect, const struct core_dispLayout *layout);
extern void video_update_core_dmd(struct mame_bitmap *bitmap, const struct rectangle *cliprect, tDMDDot dotCol, const struct core_dispLayout *layout);

/*----------------------
/ WPC driver constants
/-----------------------*/
/* Solenoid numbering */
/*       WPC          */
/*  1-28 Standard                */
/* 33-36 Upper flipper solenoids */
/* 37-40 Standard (WPC95 only)   */
/* 41-44 - "" - Copy of above    */
/* 45-48 Lower flipper solenoids */
/* 49-50 Simulated               */
/* 51-44                         */
/*       S9/S11            */
/*  1- 8 Standard 'A'-side */
/*  9-16 Standard          */
/* 17-22 Special           */
/* 23    Flipper & SS Enabled Sol (fake) */
/* 25-32 Standard 'C'-side */
/* 37-41 Sound overlay board */
/*       S7 */
/*  1-16 Standard */
/* 17-24 Special */
/* 25    Flipper & SS Enabled Sol (fake) */
/*       BY17/BY35         */
/*  1-15 Standard Pulse */
/* 17-20 Standard Hold */
/*       GTS80 */
/*  1- 9 Standard */
/* 10    GameOn (fake) */
/* 11    Tilt (for GI) (fake) */
#define CORE_FIRSTEXTSOL   37
#define CORE_FIRSTUFLIPSOL 33
#define CORE_FIRSTCUSTSOL  51
#define CORE_FIRSTLFLIPSOL 45
#define CORE_FIRSTSIMSOL   49

#define CORE_SSFLIPENSOL  23
#define CORE_FIRSTSSSOL   17

#define CORE_SOLBIT(x) (1<<((x)-1))

/*  Flipper Solenoid numbers */
#define sLRFlip     (CORE_FIRSTLFLIPSOL+1)
#define sLRFlipPow  (CORE_FIRSTLFLIPSOL+0)
#define sLLFlip     (CORE_FIRSTLFLIPSOL+3)
#define sLLFlipPow  (CORE_FIRSTLFLIPSOL+2)
#define sURFlip     (CORE_FIRSTUFLIPSOL+1)
#define sURFlipPow  (CORE_FIRSTUFLIPSOL+0)
#define sULFlip     (CORE_FIRSTUFLIPSOL+3)
#define sULFlipPow  (CORE_FIRSTUFLIPSOL+2)

/*-- Flipper solenoid bits --*/
#define CORE_LRFLIPSOLBITS 0x03
#define CORE_LLFLIPSOLBITS 0x0c
#define CORE_URFLIPSOLBITS 0x30
#define CORE_ULFLIPSOLBITS 0xc0

/*-- create a custom solenoid number --*/
/* example: #define swCustom CORE_CUSTSOLNO(1)  // custom solenoid 1 */
#define CORE_CUSTSOLNO(n) (CORE_FIRSTCUSTSOL-1+(n))

#define CORE_STDLAMPCOLS   8
#define CORE_STDSWCOLS    12

#define CORE_COINDOORSWCOL   0   /* internal array number */
#define CORE_MAXSWCOL       16   /* switch columns (0-9=sw matrix, 10=coin door, 11=cabinet/flippers) */
#define CORE_FLIPPERSWCOL   11   /* internal array number */
#define CORE_CUSTSWCOL     CORE_STDSWCOLS  /* first custom (game specific) switch column */
#define CORE_MAXLAMPCOL     42   /* lamp column (0-7=std lamp matrix 8- custom) */
#define CORE_CUSTLAMPCOL   CORE_STDLAMPCOLS  /* first custom lamp column */
#define CORE_MAXPORTS        8   /* Maximum input ports */
#define CORE_MAXGI           5   /* Maximum GI strings */

/*-- create a custom switch number --*/
/* example: #define swCustom CORE_CUSTSWNO(1,2)  // custom column 1 row 2 */
#define CORE_CUSTSWNO(c,r) ((CORE_CUSTSWCOL-1+c)*10+r)

/*-------------------
/  Flipper Switches
/ in column FLIPPERSWCOL
/--------------------*/
#define CORE_SWLRFLIPEOSBIT 0x01
#define CORE_SWLRFLIPBUTBIT 0x02
#define CORE_SWLLFLIPEOSBIT 0x04
#define CORE_SWLLFLIPBUTBIT 0x08
#define CORE_SWURFLIPEOSBIT 0x10
#define CORE_SWURFLIPBUTBIT 0x20
#define CORE_SWULFLIPEOSBIT 0x40
#define CORE_SWULFLIPBUTBIT 0x80

#define CORE_FIRSTSIMROW   80 /* first free row on display */

/*-- Colours --*/
#define CORE_COLOR(x)  Machine->pens[(x)]
#define COL_DMD        1
#define COL_DMDOFF     (COL_DMD+0)
#define COL_DMD33      (COL_DMD+1)
#define COL_DMD66      (COL_DMD+2)
#define COL_DMDON      (COL_DMD+3)
#define COL_DMDCOUNT   4
#define COL_LAMP       (COL_DMD+COL_DMDCOUNT)
#define COL_LAMPCOUNT  8
#define COL_SHADE(x)   (COL_LAMPCOUNT+(x))
#define COL_DMDAA      (COL_LAMP+COL_LAMPCOUNT*2)
#define COL_DMDAACOUNT 7
#define COL_SEGAAON1   (COL_DMDAA+COL_DMDAACOUNT)
#define COL_SEGAAON2   (COL_SEGAAON1+1)
#define COL_SEGAAOFF1  (COL_SEGAAON1+2)
#define COL_SEGAAOFF2  (COL_SEGAAON1+3)
#define COL_SEGAACOUNT 4
#define COL_COUNT      (COL_SEGAAON1+COL_SEGAACOUNT)

/* Lamp Colors */
#define BLACK       (COL_LAMP+0)
#define WHITE       (COL_LAMP+1)
#define GREEN       (COL_LAMP+2)
#define RED         (COL_LAMP+3)
#define ORANGE      (COL_LAMP+4)
#define YELLOW      (COL_LAMP+5)
#define LBLUE       (COL_LAMP+6)
#define LPURPLE     (COL_LAMP+7)

/*-------------------------------------------
/  Draw data. draw lamps,switches,solenoids
/  in this way instead of a matrix
/--------------------------------------------*/
typedef struct {
  UINT8 x,y, color;
} core_tDrawData;

typedef struct {
  UINT8 totnum;	 	 /*Total # of lamp positions defined - Up to 4 Max*/
  core_tDrawData lamppos[4];      /*Can support up to 4 lamp positions for each lamp matrix entry!*/
} core_tLampData;		 /*This means, one lamp matrix entry can share up to 4 bulbs on the playfield*/

typedef struct {
  core_tDrawData startpos;	/*Starting Coordinates to draw matrix*/
  core_tDrawData size;		/*Size of lamp matrix*/
  core_tLampData lamps[CORE_MAXLAMPCOL*8];      /*Can support up to 160 lamps!*/
} core_tLampDisplay;

#define CORE_SEGCOUNT 64
#ifdef LSB_FIRST
typedef union { struct { UINT8 lo, hi; } b; UINT16 w; } core_tSeg[CORE_SEGCOUNT];
#else /* LSB_FIRST */
typedef union { struct { UINT8 hi, lo; } b; UINT16 w; } core_tSeg[CORE_SEGCOUNT];
#endif /* LSB_FIRST */
typedef struct {
  UINT8  swMatrix[CORE_MAXSWCOL];
  UINT8  invSw[CORE_MAXSWCOL];   /* Active low switches */
  UINT8  lampMatrix[CORE_MAXLAMPCOL], tmpLampMatrix[CORE_MAXLAMPCOL];
  core_tSeg segments;     /* segments data from driver */
  UINT16 drawSeg[CORE_SEGCOUNT]; /* segments drawn */
  UINT32 solenoids;       /* on power driver bord */
  UINT32 solenoids2;      /* flipper solenoids */
  UINT32 pulsedSolState;  /* current pulse value of solenoids on driver board */
  UINT64 lastSol;         /* last state of all solenoids */
  int    gi[CORE_MAXGI];  /* WPC gi strings */
  int    simAvail;        /* simulator (keys) available */
  int    soundEn;         /* Sound enabled ? */
  int    diagnosticLed;	  /* data relating to diagnostic led(s)*/
  char   segDim[CORE_SEGCOUNT]; /* segments dimming */
} core_tGlobals;
extern core_tGlobals coreGlobals;
/* shortcut for coreGlobals */
#define cg coreGlobals
extern struct pinMachine *coreData;
/*Exported variables*/
/*-- There are no custom fields in the game driver --*/
/*-- so I have to invent some by myself. Each driver --*/
/*-- fills in one of these in the game_init function --*/
typedef struct {
  UINT64  gen;                /* Hardware Generation */
  const struct core_dispLayout *lcdLayout; /* LCD display layout */
  struct {
    UINT32  flippers;      /* flippers installed (see defines below) */
    int     swCol, lampCol, custSol; /* Custom switch columns, lamp columns and solenoids */
    UINT32  soundBoard, display;
    UINT32  gameSpecific1, gameSpecific2;
    /*-- custom functions --*/
    int  (*getSol)(int solNo);        /* get state of custom solenoid */
    void (*handleMech)(int mech);     /* update switches depending on playfield mechanics */
    int  (*getMech)(int mechNo);      /* get status of mechanics */
    void (*drawMech)(BMTYPE **line); /* draw game specific hardware */
    core_tLampDisplay *lampData;      /* lamp layout */
    wpc_tSamSolMap   *solsammap;      /* solenoids samples */
  } hw;
  const void *simData;
  struct { /* WPC specific stuff */
    char serialNo[21];  /* Securty chip serial number */
    UINT8 invSw[CORE_MAXSWCOL]; /* inverted switches (e.g. optos) */
    /* common switches */
    struct { int start, tilt, sTilt, coinDoor, shooter; } comSw;
  } wpc;
  struct { /* S3-S11 specific stuff (incl DE) */
    int muxSol;  /* S11 Mux solenoid */
    int ssSw[8]; /* Special solenoid switches */
  } sxx;
  /* simulator data */
} core_tGameData;
extern const core_tGameData *core_gameData;

extern const int core_bcd2seg9[]; /* BCD to 9 segment display */
extern const int core_bcd2seg9a[]; /* BCD to 9 segment display, missing 6 top line */
extern const int core_bcd2seg7[]; /* BCD to 7 segment display */
extern const int core_bcd2seg7a[]; /* BCD to 7 segment display, missing 6 top line */
extern const int core_bcd2seg7e[]; /* BCD to 7 segment display with A to E letters */
#define core_bcd2seg  core_bcd2seg7

/*-- Exported Display handling functions--*/
void core_updateSw(int flipEn);

/*-- text output functions --*/
void core_textOut(char *buf, int length, int x, int y, int color);
void CLIB_DECL core_textOutf(int x, int y, int color, const char *text, ...);

/*-- lamp handling --*/
void core_setLamp(UINT8 *lampMatrix, int col, int row);
void core_setLampBlank(UINT8 *lampMatrix, int col, int row);

/*-- switch handling --*/
extern void core_setSw(int swNo, int value);
extern int core_getSw(int swNo);
extern void core_updInvSw(int swNo, int inv);

/*-- get a switch column. (colEn=bits) --*/
extern int core_getSwCol(int colEn);

/*-- solenoid handling --*/
extern int core_getSol(int solNo);
extern int core_getPulsedSol(int solNo);
extern UINT64 core_getAllSol(void);

/*-- nvram handling --*/
extern void core_nvram(void *file, int write, void *mem, int length, UINT8 init);

/* makes it easier to swap bits */
extern const UINT8 core_swapNyb[16];
INLINE UINT8 core_revbyte(UINT8 x) { return (core_swapNyb[x & 0xf]<<4)|(core_swapNyb[x>>4]); }
INLINE UINT8 core_revnyb(UINT8 x) { return core_swapNyb[x]; }
INLINE UINT16 core_revword(UINT16 x) {
	UINT8 lo,hi;
	lo = core_revbyte(x & 0x00ff);
	hi = core_revbyte((x & 0xff00)>>8);
	return ((lo<<8) | hi);
}

/*-- core DIP handling --*/
//  Get the status of a DIP bank (8 dips)
extern int core_getDip(int dipBank);

/*-- Easy Bit Column to Number conversion
 *   Convert Bit Column Data to corresponding #, ie, if Bit 3=1, return 3 - Zero Based (Bit1=1 returns 0)
 *   Assumes only 1 bit is set at a time. --*/
INLINE int core_BitColToNum(int tmp)
{
	int data=0, i=0;
	do {
		if (tmp & 1) data += i;
		i++;
	} while (tmp >>= 1);
	return data;
}

extern MACHINE_DRIVER_EXTERN(PinMAME);
#endif /* INC_CORE */

#endif


