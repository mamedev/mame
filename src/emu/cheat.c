/*********************************************************************

    cheat.c

    MAME cheat system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "ui.h"
#include "uimenu.h"
#include "machine/eeprom.h"
#include "cheat.h"
#include <ctype.h>
#include <math.h>

#ifdef MESS
#include "cheatms.h"
#endif

#define OSD_READKEY_KLUDGE	1

/***************************************************************************
    MACROS
***************************************************************************/

/* easy bitfield extraction and setting, uses *_Shift, *_ShiftedMask, and *_Mask enums */
#define EXTRACT_FIELD(data, name)				(((data) >> k##name##_Shift) & k##name##_ShiftedMask)
#define SET_FIELD(data, name, in)				(data = (data & ~(k##name##_ShiftedMask << k##name##_Shift)) | (((in) & k##name##_ShiftedMask) << k##name##_Shift))
#define TEST_FIELD(data, name)					((data) & k##name##_Mask)
#define SET_MASK_FIELD(data, name)				((data) |= k##name##_Mask)
#define CLEAR_MASK_FIELD(data, name)			((data) &= ~(k##name##_Mask))
#define TOGGLE_MASK_FIELD(data, name)			((data) ^= k##name##_Mask)

#define DEFINE_BITFIELD_ENUM(name, end, start)	k##name##_Shift = (int)(end), 											\
												k##name##_ShiftedMask = (int)(0xFFFFFFFF >> (32 - (start - end + 1))),	\
												k##name##_Mask = (int)(k##name##_ShiftedMask << k##name##_Shift)

#define kRegionListLength						(REGION_MAX - REGION_INVALID)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CHEAT_FILENAME_MAX_LEN					255

/********** BIT FIELD **********/

enum
{
	/* cheat code */
	DEFINE_BITFIELD_ENUM(OneShot,					0,	0),
	DEFINE_BITFIELD_ENUM(Type,						1,	2),
	DEFINE_BITFIELD_ENUM(Operation,					3,	4),
	DEFINE_BITFIELD_ENUM(TypeParameter,				5,	7),
	DEFINE_BITFIELD_ENUM(UserSelectEnable,			8,	8),
	DEFINE_BITFIELD_ENUM(UserSelectMinimumDisplay,	9,	9),
	DEFINE_BITFIELD_ENUM(UserSelectMinimum,			10,	10),
	DEFINE_BITFIELD_ENUM(UserSelectBCD,				11,	11),
	DEFINE_BITFIELD_ENUM(Prefill,					12,	13),
	DEFINE_BITFIELD_ENUM(RemoveFromList,			14, 14),
	DEFINE_BITFIELD_ENUM(LinkExtension,				15,	15),
	DEFINE_BITFIELD_ENUM(LinkEnable,				16,	16),
	DEFINE_BITFIELD_ENUM(LinkCopyPreviousValue,		17,	17),
	DEFINE_BITFIELD_ENUM(OperationParameter,		18,	18),
	DEFINE_BITFIELD_ENUM(OperationExtend,			19,	19),
	DEFINE_BITFIELD_ENUM(BytesUsed,					20,	21),
	DEFINE_BITFIELD_ENUM(Endianness,				22,	22),
	DEFINE_BITFIELD_ENUM(RestorePreviousValue,		23, 23),
	DEFINE_BITFIELD_ENUM(LocationParameter,			24,	28),
	DEFINE_BITFIELD_ENUM(LocationType,				29,	31),

	/* watch (unused ?) */
//  DEFINE_BITFIELD_ENUM(Watch_AddValue,            0,  15),
//  DEFINE_BITFIELD_ENUM(Watch_Label,               16, 17),
//  DEFINE_BITFIELD_ENUM(Watch_DisplayType,         18, 19),

	/* command */
	DEFINE_BITFIELD_ENUM(AutoSaveEnabled,			0,	0),
	DEFINE_BITFIELD_ENUM(SearchBox,					1,	2),		// 0 = Minimum, 1 = Classic, 2 = Advanced
	DEFINE_BITFIELD_ENUM(DontPrintNewLabels,		3,	3),		// in options menu, it is reverted.
	DEFINE_BITFIELD_ENUM(ActivationKeyMessage,		4,	4),
	DEFINE_BITFIELD_ENUM(LoadOldFormat,				5,	5),
	DEFINE_BITFIELD_ENUM(Debug,						6,	6)
};

/********** OPERATION **********/

enum		// types
{
	kType_NormalOrDelay = 0,
	kType_WaitForModification,
	kType_IgnoreIfDecrementing,
	kType_Watch
};

enum		// operations
{
	kOperation_WriteMask = 0,
	kOperation_AddSubtract,
	kOperation_ForceRange,
	kOperation_SetOrClearBits,
	kOperation_WriteMatch,
	// kOperation_Unused5,
	// kOperation_Unused6,
	kOperation_None = 7
};

//enum      // prefills (unused?)
//{
//  kPrefill_Disable = 0,
//  kPrefill_UseFF,
//  kPrefill_Use00,
//  kPrefill_Use01
//};

enum		// location types
{
	kLocation_Standard = 0,
	kLocation_MemoryRegion,
	kLocation_HandlerMemory,
	kLocation_Custom,
	kLocation_IndirectIndexed,
	kLocation_ProgramSpace
};

enum		// location parameters
{
	kCustomLocation_Comment = 0,
	kCustomLocation_EEPROM,
	kCustomLocation_Select,
	kCustomLocation_AssignActivationKey,
	kCustomLocation_Enable,
	kCustomLocation_Overclock,
	kCustomLocation_RefreshRate
};

enum		// flags for action
{
	/* set for wait for modification or ignore if decrementing cheats when
       the targeted value has changed
       cleared after the operation is performed */
	kActionFlag_WasModified =		1 << 0,

	/* set for one shot cheats after the operation is performed */
	kActionFlag_OperationDone =		1 << 1,

	/* set if the extendData field is being used by something other than a mask value */
	kActionFlag_IgnoreMask =		1 << 2,

	/* set if the lastValue field contains valid data and can be restored if needed */
	kActionFlag_LastValueGood =		1 << 3,

	/* set after value changes from prefill value */
	kActionFlag_PrefillDone =		1 << 4,

	/* set after prefill value written */
	kActionFlag_PrefillWritten =	1 << 5,

	/* masks */
	kActionFlag_StateMask =			kActionFlag_OperationDone |
									kActionFlag_LastValueGood |
									kActionFlag_PrefillDone |
									kActionFlag_PrefillWritten,
	kActionFlag_InfoMask =			kActionFlag_WasModified |
									kActionFlag_IgnoreMask,
	kActionFlag_PersistentMask =	kActionFlag_LastValueGood
};

enum		// flags for entry
{
	/* true when the cheat is active */
	kCheatFlag_Active =					1 << 0,

	/* true if the cheat is entirely one shot */
	kCheatFlag_OneShot =				1 << 1,

	/* true if the cheat will work with one shot in special case */
	kCheatFlag_DoOneShot =				1 << 2,

	/* true if the cheat is entirely null (ex. a comment) */
	kCheatFlag_Null =					1 << 3,

	/* true if the cheat contains a user-select element */
	kCheatFlag_UserSelect =				1 << 4,

	/* true if the cheat is a select cheat */
	kCheatFlag_Select =					1 << 5,

	/* true if the cheat has been assigned an 1st activation key */
	kCheatFlag_HasActivationKey1 =		1 << 6,

	/* true if the cheat has been assigned an 2nd activation key */
	kCheatFlag_HasActivationKey2 =		1 << 7,

	/* true if the activation key is being pressed */
	kCheatFlag_ActivationKeyPressed =	1 << 8,

	/* true if the cheat has been edited or is a new cheat */
	kCheatFlag_Dirty =					1 << 9,

	/* masks */
	kCheatFlag_StateMask =			kCheatFlag_Active |
									kCheatFlag_DoOneShot,
	kCheatFlag_InfoMask =			kCheatFlag_OneShot |
									kCheatFlag_DoOneShot |
									kCheatFlag_Null |
									kCheatFlag_UserSelect |
									kCheatFlag_Select |
									kCheatFlag_HasActivationKey1 |
									kCheatFlag_HasActivationKey2 |
									kCheatFlag_ActivationKeyPressed,
	kCheatFlag_PersistentMask =		kCheatFlag_Active |
									kCheatFlag_DoOneShot |
									kCheatFlag_HasActivationKey1 |
									kCheatFlag_HasActivationKey2 |
									kCheatFlag_ActivationKeyPressed |
									kCheatFlag_Dirty
};

/********** WATCH **********/

enum		// types
{
	kWatchLabel_None = 0,
	kWatchLabel_Address,
	kWatchLabel_String,

	kWatchLabel_MaxPlusOne
};

enum		// display types
{
	kWatchDisplayType_Hex = 0,
	kWatchDisplayType_Decimal,
	kWatchDisplayType_Binary,
	kWatchDisplayType_ASCII,

	kWatchDisplayType_MaxPlusOne
};

/********** REGION **********/

enum		// flags
{
	/* true if enabled for search */
	kRegionFlag_Enabled =		1 << 0

	/* true if the memory region has no mapped memory and uses a memory handler (unused?) */
//  kRegionFlag_UsesHandler =   1 << 1
};

enum		// types
{
	kRegionType_CPU = 0,
	kRegionType_Memory
};

enum		// default search regions
{
	kSearchSpeed_Fast = 0,		// RAM + some banks
	kSearchSpeed_Medium,		// RAM + BANKx
	kSearchSpeed_Slow,			// all memory areas except ROM, NOP, and custom handlers
	kSearchSpeed_VerySlow,		// all memory areas except ROM and NOP
	kSearchSpeed_AllMemory,		// entire CPU address space

	kSearchSpeed_Max = kSearchSpeed_AllMemory
};

/********** SEARCH **********/

enum		// operands
{
	kSearchOperand_Current = 0,
	kSearchOperand_Previous,
	kSearchOperand_First,
	kSearchOperand_Value,

	kSearchOperand_Max = kSearchOperand_Value
};

enum		// bits
{
	kSearchSize_8Bit = 0,
	kSearchSize_16Bit,
	kSearchSize_24Bit,
	kSearchSize_32Bit,
	kSearchSize_1Bit,

	kSearchSize_Max = kSearchSize_1Bit
};

enum		// comparisons
{
	kSearchComparison_LessThan = 0,
	kSearchComparison_GreaterThan,
	kSearchComparison_EqualTo,
	kSearchComparison_LessThanOrEqualTo,
	kSearchComparison_GreaterThanOrEqualTo,
	kSearchComparison_NotEqual,
	kSearchComparison_IncreasedBy,
	kSearchComparison_NearTo,

	kSearchComparison_Max = kSearchComparison_NearTo
};

/********** OTHERS **********/

enum
{
	kVerticalKeyRepeatRate =		8,
	kHorizontalFastKeyRepeatRate =	5,
	kHorizontalSlowKeyRepeatRate =	8
};

enum
{
	kSearchBox_Minimum = 0,
	kSearchBox_Classic,
	kSearchBox_Advanced
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/********** ACTION **********/

struct CheatAction
{
	UINT32	type;
	UINT32	address;
	UINT32	data;
	UINT32	extendData;
	UINT32	originalDataField;

	INT32	frameTimer;
	UINT32	lastValue;

	UINT32	flags;

	UINT8	** cachedPointer;
	UINT32	cachedOffset;

	char	* optionalName;
};

typedef struct CheatAction	CheatAction;

/********** ENTRY **********/

struct CheatEntry
{
	char			* name;
	char			* comment;

	INT32			actionListLength;
	CheatAction		* actionList;

	int				activationKey1;
	int				activationKey2;

	UINT32			flags;
	int				selection;
};

typedef struct CheatEntry	CheatEntry;

/********** WATCH **********/

struct WatchInfo
{
	UINT32			address;
	UINT8			cpu;
	UINT8			numElements;
	UINT8			elementBytes;
	UINT8			labelType;
	UINT8			displayType;
	UINT8			skip;
	UINT8			elementsPerLine;
	INT8			addValue;
	INT8			addressShift;
	INT8			dataShift;
	UINT32			xor;

	UINT8			locationType;

	float			x, y;

	CheatEntry *	linkedCheat;

	char			label[256];
};

typedef struct WatchInfo	WatchInfo;

/********** REGION **********/

struct SearchRegion
{
	UINT32	address;
	UINT32	length;

	UINT8	targetType;
	UINT8	targetIdx;

	UINT8	flags;

	UINT8	* cachedPointer;
	const address_map_entry
			* writeHandler;

	UINT8	* first;
	UINT8	* last;

	UINT8	* status;

	UINT8	* backupLast;
	UINT8	* backupStatus;

	char	name[32];		// 12345678 - 12345678 BANK31

	UINT32	numResults;
	UINT32	oldNumResults;
};

typedef struct SearchRegion	SearchRegion;

/********** SEARCH **********/

struct OldSearchOptions
{
	UINT8	status;
	UINT8	energy;
	UINT8	operand;
	UINT32	value;
	UINT32	delta;
};

typedef struct OldSearchOptions	OldSearchOptions;

struct SearchInfo
{
	INT32				regionListLength;
	SearchRegion		* regionList;

	char				* name;

	INT8				bytes;			// 0 = 1, 1 = 2, 2 = 3, 3 = 4, 4 = bit
	UINT8				swap;
	UINT8				sign;
	INT8				lhs;
	INT8				rhs;
	INT8				comparison;

	UINT8				targetType;		// cpu/region
	UINT8				targetIdx;		// cpu or region index

	UINT32				value;

	UINT8				searchSpeed;

	UINT32				numResults;
	UINT32				oldNumResults;

	INT32				currentRegionIdx;
	INT32				currentResultsPage;

	UINT8				backupValid;

	OldSearchOptions	oldOptions;
};

typedef struct SearchInfo	SearchInfo;

/********** CPU **********/

struct CPUInfo
{
	UINT8	type;
	UINT8	dataBits;
	UINT8	addressBits;
	UINT8	addressCharsNeeded;
	UINT32	addressMask;
	UINT8	endianness;
	UINT8	addressShift;
};

typedef struct CPUInfo	CPUInfo;

/********** MENU **********/

struct MenuStringList
{
	const char	** mainList;	// editable menu item lists
	const char	** subList;
	char		* flagList;

	char	** mainStrings;		// lists of useable strings
	char	** subStrings;

	char	* buf;				// string storage

	UINT32	length;				// number of menu items supported
	UINT32	numStrings;			// number of strings supported
	UINT32	mainStringLength;	// max length of main string
	UINT32	subStringLength;	// max length of sub string
};

typedef struct MenuStringList	MenuStringList;

struct MenuItemInfoStruct
{
	UINT32	subcheat;
	UINT32	fieldType;
	UINT32	extraData;
};

typedef struct MenuItemInfoStruct	MenuItemInfoStruct;

/***************************************************************************
    EXPORTED VARIABLES
***************************************************************************/

static const char			* cheatfile = NULL;

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static emu_timer			* periodic_timer;

static CheatEntry			* cheatList = NULL;
static INT32				cheatListLength = 0;

static WatchInfo			* watchList = NULL;
static INT32				watchListLength = 0;

static SearchInfo			* searchList = NULL;
static INT32				searchListLength = 0;
static INT32				currentSearchIdx = 0;

static CPUInfo				cpuInfoList[MAX_CPU];
static CPUInfo				regionInfoList[kRegionListLength];

//static int                    cheatEngineWasActive = 0;
static int					foundCheatDatabase = 0;
static int					cheatsDisabled = 0;
static int					watchesDisabled = 0;

static int					fullMenuPageHeight = 0;

static char					mainDatabaseName[CHEAT_FILENAME_MAX_LEN + 1];

static MenuStringList		menuStrings;

static MenuItemInfoStruct	* menuItemInfo;
static INT32				menuItemInfoLength = 0;

static UINT32				cheatOptions = 2;

static const char *const kCheatNameTemplates[] =
{
    "Infinite Credits",
   "Infinite Time",
   "Infinite Lives",
   "Infinite Energy",
   "Invincibility",
   "Infinite Time PL1",
   "Infinite Lives PL1",
   "Infinite Energy PL1",
   "Drain all Energy Now! PL1",
   "Invincibility PL1",
   "Infinite Time PL2",
   "Infinite Lives PL2",
   "Infinite Energy PL2",
   "Drain all Energy Now! PL2",
   "Invincibility PL2",
   "Infinite Ammo",
   "Infinite Bombs",
   "Select starting level",
   "Rapid Fire",
   "Infinite Ammo PL1",
   "Infinite Bombs PL1",
   "Select Score PL1",
   "Rapid Fire PL1",
   "Infinite Ammo PL2",
   "Infinite Bombs PL2",
   "Select Score PL2",
   "Rapid Fire PL2",
   "Display the Correct Answer",
   "Infinite ",
   "Always have ",
   "Get ",
   "Lose ",
   "Finish this ",
   "---> <ENTER> To Edit <---",
   "\0"
};

static CPUInfo rawCPUInfo =
{
	0,			// type
	8,			// dataBits
	8,			// addressBits
	1,			// addressCharsNeeded
	CPU_IS_BE	// endianness
};

static const int kSearchByteIncrementTable[] =
{
	1,
	2,
	3,
	4,
	1
};

static const int kSearchByteStep[] =
{
	1,
	2,
	1,
	4,
	1
};

static const int	kSearchByteDigitsTable[] =
{
	2,
	4,
	6,
	8,
	1
};

static const int	kSearchByteDecDigitsTable[] =
{
	3,
	5,
	8,
	10,
	1
};

static const UINT32 kSearchByteMaskTable[] =
{
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF,
	0xFFFFFFFF,
	0x00000001
};

static const UINT32	kSearchByteSignBitTable[] =
{
	0x00000080,
	0x00008000,
	0x00800000,
	0x80000000,
	0x00000000
};

static const UINT32 kSearchByteUnsignedMaskTable[] =
{
	0x0000007F,
	0x00007FFF,
	0x007FFFFF,
	0x7FFFFFFF,
	0x00000001
};

static const UINT32	kCheatSizeMaskTable[] =
{
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF,
	0xFFFFFFFF
};

static const UINT32	kCheatSizeDigitsTable[] =
{
	2,
	4,
	6,
	8
};

static const int	kByteConversionTable[] =
{
	kSearchSize_8Bit,
	kSearchSize_16Bit,
	kSearchSize_24Bit,
	kSearchSize_32Bit,
	kSearchSize_32Bit
};

static const int	kWatchSizeConversionTable[] =
{
	kSearchSize_8Bit,
	kSearchSize_16Bit,
	kSearchSize_24Bit,
	kSearchSize_32Bit,
	kSearchSize_8Bit
};

static const int	kSearchOperandNeedsInit[] =
{
	0,
	1,
	1,
	0
};

static const UINT32 kPrefillValueTable[] =
{
	0x00,
	0xFF,
	0x00,
	0x01
};

static const char *const kWatchLabelStringList[] =
{
	"None",
	"Address",
	"String"
};

static const char *const kWatchDisplayTypeStringList[] =
{
	"Hex",
	"Decimal",
	"Binary",
	"ASCII"
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/********** MAIN **********/
static TIMER_CALLBACK( cheat_periodic );
static void 	cheat_exit(running_machine *machine);

/********** SPECIAL KEY HANDLING **********/
static int		ShiftKeyPressed(void);
static int		ControlKeyPressed(void);
static int		AltKeyPressed(void);

static int		UIPressedRepeatThrottle(running_machine *machine, int code, int baseSpeed);

/********** DIRECT KEY INPUT **********/
static int		ReadHexInput(void);

static char *	DoDynamicEditTextField(char * buf);
static void		DoStaticEditTextField(char * buf, int size);
static UINT32	DoEditHexField(running_machine *machine, UINT32 data);
static UINT32	DoEditHexFieldSigned(UINT32 data, UINT32 mask);
static INT32	DoEditDecField(INT32 data, INT32 min, INT32 max);

/********** VALUE SHIFT OR CONVERTION **********/
static UINT32	DoShift(UINT32 input, INT8 shift);

static UINT32	BCDToDecimal(UINT32 value);
static UINT32	DecimalToBCD(UINT32 value);

/********** STRINGS **********/
static void		RebuildStringTables(void);
static void		RequestStrings(UINT32 length, UINT32 numStrings, UINT32 mainStringLength, UINT32 subStringLength);
static void		InitStringTable(void);
static void		FreeStringTable(void);

static char *	CreateStringCopy(char * buf);

/********** ADDITIONAL MENU FOR CHEAT **********/
static INT32	UserSelectValueMenu(running_machine *machine, int selection, CheatEntry * entry);

/********** CHEAT MENU **********/
static int		EnableDisableCheatMenu(running_machine *machine, int selection, int firstTime);
static int		AddEditCheatMenu(running_machine *machine, int selection);
static int		EditCheatMenu(running_machine *machine, CheatEntry * entry, int index, int selection);

static int		DoSearchMenuMinimum(running_machine *machine, int selection);	// minimum mode
static int		DoSearchMenuClassic(running_machine *machine, int selection);	// classic mode
static int		DoSearchMenu(running_machine *machine, int selection);			// advanced mode
static int		SelectSearchRegions(running_machine *machine, int selection, SearchInfo * search);
static int		ViewSearchResults(running_machine *machine, int selection, int firstTime);

static int		ChooseWatch(running_machine *machine, int selection);
static int		EditWatch(running_machine *machine, WatchInfo * entry, int selection);

static int		SelectOptions(running_machine *machine, int selection);
static int		SelectSearch(running_machine *machine, int selection);

/********** ENTRY LIST **********/
static void		ResizeCheatList(UINT32 newLength);
static void		ResizeCheatListNoDispose(UINT32 newLength);

static void		AddCheatBefore(UINT32 idx);
static void		DeleteCheatAt(UINT32 idx);

static void		DisposeCheat(CheatEntry * entry);
static CheatEntry *	GetNewCheat(void);

/********** ACTION LIST **********/
static void		ResizeCheatActionList(CheatEntry * entry, UINT32 newLength);
static void		ResizeCheatActionListNoDispose(CheatEntry * entry, UINT32 newLength);

static void		AddActionBefore(CheatEntry * entry, UINT32 idx);
static void		DeleteActionAt(CheatEntry * entry, UINT32 idx);
static void		DisposeAction(CheatAction * action);

/********** WATCH LIST **********/
static void		InitWatch(WatchInfo * info, UINT32 idx);
static void		ResizeWatchList(UINT32 newLength);
static void		ResizeWatchListNoDispose(UINT32 newLength);

static void		AddWatchBefore(UINT32 idx);
static void		DeleteWatchAt(UINT32 idx);
static void		DisposeWatch(WatchInfo * watch);
static WatchInfo *	GetUnusedWatch(void);

static void		AddCheatFromWatch(WatchInfo * watch);
static void		SetupCheatFromWatchAsWatch(CheatEntry * entry, WatchInfo * watch);

/********** SEARCH LIST **********/
static void		ResizeSearchList(UINT32 newLength);
static void		ResizeSearchListNoDispose(UINT32 newLength);

static void		AddSearchBefore(UINT32 idx);
static void		DeleteSearchAt(UINT32 idx);

static void		InitSearch(SearchInfo * info);

static void		DisposeSearchRegions(SearchInfo * info);
static void		DisposeSearch(UINT32 idx);
static SearchInfo *	GetCurrentSearch(void);

static void		FillBufferFromRegion(SearchRegion * region, UINT8 * buf);
static UINT32	ReadRegionData(SearchRegion * region, UINT32 offset, UINT8 size, UINT8 swap);

static void		BackupSearch(SearchInfo * info);
static void		RestoreSearchBackup(SearchInfo * info);
static void		BackupRegion(SearchRegion * region);
static void		RestoreRegionBackup(SearchRegion * region);
static void		SetSearchRegionDefaultName(SearchRegion * region);
static void		AllocateSearchRegions(SearchInfo * info);
static void		BuildSearchRegions(running_machine *machine, SearchInfo * info);

/********** CODE LOADER **********/
static int		ConvertOldCode(int code, int cpu, int * data, int * extendData);
static void		HandleLocalCommandCheat(running_machine *machine, UINT32 type, UINT32 address, UINT32 data, UINT32 extendData, char * name, char * description);

static void		LoadCheatFile(running_machine *machine, char * fileName);
static void		LoadCheatDatabase(running_machine *machine);

static void		DisposeCheatDatabase(void);
static void		ReloadCheatDatabase(running_machine *machine);

static void		SaveCheat(running_machine *machine, CheatEntry * entry, int selection, int saveCode);
static void		DoAutoSaveCheats(running_machine *machine);

/********** CODE ADDITION **********/
static void		AddCheatFromResult(SearchInfo * search, SearchRegion * region, UINT32 address);
static void		AddCheatFromFirstResult(SearchInfo * search);
static void		AddWatchFromResult(SearchInfo * search, SearchRegion * region, UINT32 address);

/********** SEARCH **********/
static UINT32	SearchSignExtend(SearchInfo * search, UINT32 value);
static UINT32	ReadSearchOperand(UINT8 type, SearchInfo * search, SearchRegion * region, UINT32 address);
static UINT32	ReadSearchOperandBit(UINT8 type, SearchInfo * search, SearchRegion * region, UINT32 address);
static UINT8	DoSearchComparison(SearchInfo * search, UINT32 lhs, UINT32 rhs);
static UINT32	DoSearchComparisonBit(SearchInfo * search, UINT32 lhs, UINT32 rhs);
//static UINT8  IsRegionOffsetValid(SearchInfo * search, SearchRegion * region, UINT32 offset);

#define IsRegionOffsetValid	IsRegionOffsetValidBit

static UINT8	IsRegionOffsetValidBit(SearchInfo * search, SearchRegion * region, UINT32 offset);
static void		InvalidateRegionOffset(SearchInfo * search, SearchRegion * region, UINT32 offset);
static void		InvalidateRegionOffsetBit(SearchInfo * search, SearchRegion * region, UINT32 offset, UINT32 invalidate);
static void		InvalidateEntireRegion(SearchInfo * search, SearchRegion * region);

static void		InitializeNewSearch(SearchInfo * search);
static void		UpdateSearch(SearchInfo * search);

static void		DoSearch(SearchInfo * search);

/********** MEMORY ACCESSORS  **********/
static UINT8 **	LookupHandlerMemory(UINT8 cpu, UINT32 address, UINT32 * outRelativeAddress);

static UINT32	DoCPURead(UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap);
static UINT32	DoMemoryRead(UINT8 * buf, UINT32 address, UINT8 bytes, UINT8 swap, CPUInfo * info);
static void		DoCPUWrite(UINT32 data, UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap);
static void		DoMemoryWrite(UINT32 data, UINT8 * buf, UINT32 address, UINT8 bytes, UINT8 swap, CPUInfo * info);

/********** CPU **********/
static UINT8	CPUNeedsSwap(UINT8 cpu);
static UINT8	RegionNeedsSwap(UINT8 region);

static CPUInfo *	GetCPUInfo(UINT8 cpu);
static CPUInfo *	GetRegionCPUInfo(UINT8 region);

static UINT32	SwapAddress(UINT32 address, UINT8 dataSize, CPUInfo * info);

static UINT32	ReadData(CheatAction * action);
static void		WriteData(CheatAction * action, UINT32 data);

/********** WATCH **********/
static void		WatchCheatEntry(CheatEntry * entry, UINT8 associate);
static void		AddActionWatch(CheatAction * action, CheatEntry * entry);
static void		RemoveAssociatedWatches(CheatEntry * entry);

static void		ResetAction(CheatAction * action);
static void		ActivateCheat(CheatEntry * entry);
static void		DeactivateCheat(CheatEntry * entry);
static void		TempDeactivateCheat(CheatEntry * entry);

static void		cheat_periodicOperation(CheatAction * action);
static void		cheat_periodicAction(running_machine *machine, CheatAction * action);
static void		cheat_periodicEntry(running_machine *machine, CheatEntry * entry);

static void		UpdateAllCheatInfo(void);
static void		UpdateCheatInfo(CheatEntry * entry, UINT8 isLoadTime);

static int		IsAddressInRange(CheatAction * action, UINT32 length);

static void		BuildCPUInfoList(running_machine *machine);

/*--------------------------------------------------------------
  special key handler - check pressing shift, ctrl or alt key
--------------------------------------------------------------*/

static int ShiftKeyPressed(void)
{
	return (input_code_pressed(KEYCODE_LSHIFT) || input_code_pressed(KEYCODE_RSHIFT));
}

static int ControlKeyPressed(void)
{
	return (input_code_pressed(KEYCODE_LCONTROL) || input_code_pressed(KEYCODE_RCONTROL));
}

static int AltKeyPressed(void)
{
	return (input_code_pressed(KEYCODE_LALT) || input_code_pressed(KEYCODE_RALT));
}

/*---------------------------------------------------------------------------------------------------------------------
  ReadKeyAsync - dirty hack until osd_readkey_unicode is supported in MAMEW re-implementation of osd_readkey_unicode
---------------------------------------------------------------------------------------------------------------------*/

#if 1

#if OSD_READKEY_KLUDGE

static int ReadKeyAsync(int flush)
{
	int	code;

	if(flush)		// check key input
	{
		while(input_code_poll_keyboard_switches(TRUE) != INPUT_CODE_INVALID) ;

		return 0;
	}

	while(1)		// check pressed key
	{
		code = input_code_poll_keyboard_switches(FALSE);

		if(code == INPUT_CODE_INVALID)
		{
			return 0;
		}
		else if((code >= KEYCODE_A) && (code <= KEYCODE_Z))
		{
			if(ShiftKeyPressed())
			{
				return 'A' + (code - KEYCODE_A);
			}
			else
			{
				return 'a' + (code - KEYCODE_A);
			}
		}
		else if((code >= KEYCODE_0) && (code <= KEYCODE_9))
		{
			if(ShiftKeyPressed())
			{
				return ")!@#$%^&*("[code - KEYCODE_0];
			}
			else
			{
				return '0' + (code - KEYCODE_0);
			}
		}
		else if((code >= KEYCODE_0_PAD) && (code <= KEYCODE_9_PAD))
		{
			return '0' + (code - KEYCODE_0_PAD);
		}
		else if(code == KEYCODE_TILDE)
		{
			if(ShiftKeyPressed())
			{
				return '~';
			}
			else
			{
				return '`';
			}
		}
		else if(code == KEYCODE_MINUS)
		{
			if(ShiftKeyPressed())
			{
				return '_';
			}
			else
			{
				return '-';
			}
		}
		else if(code == KEYCODE_EQUALS)
		{
			if(ShiftKeyPressed())
			{
				return '+';
			}
			else
			{
				return '=';
			}
		}
		else if(code == KEYCODE_BACKSPACE)
		{
			return 0x08;
		}
		else if(code == KEYCODE_OPENBRACE)
		{
			if(ShiftKeyPressed())
			{
				return '{';
			}
			else
			{
				return '[';
			}
		}
		else if(code == KEYCODE_CLOSEBRACE)
		{
			if(ShiftKeyPressed())
			{
				return '}';
			}
			else
			{
				return ']';
			}
		}
		else if(code == KEYCODE_COLON)
		{
			if(ShiftKeyPressed())
			{
				return ':';
			}
			else
			{
				return ';';
			}
		}
		else if(code == KEYCODE_QUOTE)
		{
			if(ShiftKeyPressed())
			{
				return '\"';
			}
			else
			{
				return '\'';
			}
		}
		else if(code == KEYCODE_BACKSLASH)
		{
			if(ShiftKeyPressed())
			{
				return '|';
			}
			else
			{
				return '\\';
			}
		}
		else if(code == KEYCODE_COMMA)
		{
			if(ShiftKeyPressed())
			{
				return '<';
			}
			else
			{
				return ',';
			}
		}
		else if(code == KEYCODE_STOP)
		{
			if(ShiftKeyPressed())
			{
				return '>';
			}
			else
			{
				return '.';
			}
		}
		else if(code == KEYCODE_SLASH)
		{
			if(ShiftKeyPressed())
			{
				return '?';
			}
			else
			{
				return '/';
			}
		}
		else if(code == KEYCODE_SLASH_PAD)
		{
			return '/';
		}
		else if(code == KEYCODE_ASTERISK)
		{
			return '*';
		}
		else if(code == KEYCODE_MINUS_PAD)
		{
			return '-';
		}
		else if(code == KEYCODE_PLUS_PAD)
		{
			return '+';
		}
		else if(code == KEYCODE_SPACE)
		{
			return ' ';
		}
	}
}

#define osd_readkey_unicode ReadKeyAsync

#endif

#endif

/*-------------------------------------
  old_style_menu - export menu items
-------------------------------------*/

static void old_style_menu(const char **items, const char **subitems, char *flag, int selected, int arrowize_subitem)
{
	static ui_menu_item item_list[1000];
	int menu_items;

	for (menu_items = 0; items[menu_items]; menu_items++)
	{
		item_list[menu_items].text = items[menu_items];
		item_list[menu_items].subtext = subitems ? subitems[menu_items] : NULL;
		item_list[menu_items].flags = 0;
		if (flag && flag[menu_items])
			item_list[menu_items].flags |= MENU_FLAG_INVERT;
		if (menu_items == selected)
		{
			if (arrowize_subitem & 1)
				item_list[menu_items].flags |= MENU_FLAG_LEFT_ARROW;
			if (arrowize_subitem & 2)
				item_list[menu_items].flags |= MENU_FLAG_RIGHT_ARROW;
		}
	}
	ui_menu_draw(item_list, menu_items, selected, NULL);
}

/*-------------------------------------------------
  UIPressedRepoeatThrottle - key repeat handling
-------------------------------------------------*/

static int UIPressedRepeatThrottle(running_machine *machine, int code, int baseSpeed)
{
	static int	lastCode = -1;
	static int	lastSpeed = -1;
	static int	incrementTimer = 0;
	int			pressed = 0;

	const int	kDelayRampTimer = 10;

	if(input_type_pressed(machine, code, 0))
	{
		if(lastCode != code)			// different key pressed
		{
			lastCode = code;
			lastSpeed = baseSpeed;
			incrementTimer = kDelayRampTimer * lastSpeed;
		}
		else							// same key pressed
		{
			incrementTimer--;

			if(incrementTimer <= 0)
			{
				incrementTimer = kDelayRampTimer * lastSpeed;

				lastSpeed /= 2;

				if(lastSpeed < 1)
					lastSpeed = 1;

				pressed = 1;
			}
		}
	}
	else
	{
		if(lastCode == code)
			lastCode = -1;
	}

	return input_ui_pressed_repeat(machine, code, lastSpeed);
}

/*---------------------------------
  ReadHexInput - check hex input
---------------------------------*/

static int ReadHexInput(void)
{
	int	i;

	for(i = 0; i < 10; i++)
	{
		if(input_code_pressed_once(KEYCODE_0 + i))
			return i;
	}

	for(i = 0; i < 10; i++)
	{
		if(input_code_pressed_once(KEYCODE_0_PAD + i))
			return i;
	}

	for(i = 0; i < 6; i++)
	{
		if(input_code_pressed_once(KEYCODE_A + i))
			return i + 10;
	}

	return -1;
}

/*-----------------------------------------------------------------
  DoDynamicEditTextField - edit text field with direct key input
-----------------------------------------------------------------*/

static char * DoDynamicEditTextField(char * buf)
{
	char	code = osd_readkey_unicode(0) & 0xFF;

	if(code == 0x08)
	{
		/* ----- backspace ----- */
		if(buf)
		{
			size_t	length = strlen(buf);

			if(length > 0)
			{
				buf[length - 1] = 0;

				if(length > 1)
					buf = realloc(buf, length);
				else
				{
					free(buf);

					buf = NULL;
				}
			}
		}
	}
	else
	{
		if(isprint(code))
		{
			if(buf)
			{
				size_t	length = strlen(buf);

				buf = realloc(buf, length + 2);

				buf[length] = code;
				buf[length + 1] = 0;
			}
			else
			{
				buf = malloc(2);

				buf[0] = code;
				buf[1] = 0;
			}
		}
	}

	return buf;
}

/*------------------------
  DoStaticEditTextField
------------------------*/

static void DoStaticEditTextField(char * buf, int size)
{
	char	code = osd_readkey_unicode(0) & 0xFF;
	size_t	length;

	if(!buf)
		return;

	length = strlen(buf);

	if(code == 0x08)
	{
		if(length > 0)
		{
			buf[length - 1] = 0;
		}
	}
	else if(isprint(code))
	{
		if(length + 1 < size)
		{
			buf[length] = code;
			buf[length + 1] = 0;
		}
	}
}

/*------------------------------------------------------------------
  DoEditHexField - edit hex field with direct key input (unsigned)
------------------------------------------------------------------*/

static UINT32 DoEditHexField(running_machine *machine, UINT32 data)
{
	INT8	key;

	if(input_code_pressed_once(KEYCODE_BACKSPACE))
	{
		/* ----- backspace ----- */
		data >>= 4;

		return data;
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_CLEAR))
		{
			data = 0;

			return data;
		}
	}

	key = ReadHexInput();

	if(key != -1)
	{
		data <<= 4;
		data |= key;
	}

	return data;
}

/*-----------------------------------------------------------------------
  DoEditHexFieldSigned - edit hex field with direct key input (signed)
-----------------------------------------------------------------------*/

static UINT32 DoEditHexFieldSigned(UINT32 data, UINT32 mask)
{
	INT8	key;
	UINT32	isNegative = data & mask;

	if(isNegative)
		data |= mask;

	key = ReadHexInput();

	if(key != -1)
	{
		if(isNegative)
			data = (~data) + 1;

		data <<= 4;
		data |= key;

		if(isNegative)
			data = (~data) + 1;
	}
	else
	{
		if(input_code_pressed_once(KEYCODE_MINUS))
			data = (~data) + 1;
	}

	return data;
}

/*--------------------------------------------------------
  DoEditDecField - edit dec field with direct key input
--------------------------------------------------------*/

static INT32 DoEditDecField(INT32 data, INT32 min, INT32 max)
{
	char	code = osd_readkey_unicode(0) & 0xFF;

	if((code >= '0') && (code <= '9'))
	{
		data *= 10;
		data += (code - '0');
	}
	else
	{
		if(code == '-')
			data = -data;
		else
		{
			/* ----- backspace ----- */
			if(code == 0x08)
				data /= 10;
		}
	}

	/* ----- adjust value ----- */
	if(data < min)
		data = min;
	if(data > max)
		data = max;

	return data;
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*---------------------------------------
  cheat_init - initialize cheat system
---------------------------------------*/

void cheat_init(running_machine *machine)
{
	/* ----- initialize lists ----- */
	cheatList =				NULL;
	cheatListLength =		0;

	watchList =				NULL;
	watchListLength =		0;

	searchList =			NULL;
	searchListLength =		0;

#ifdef MESS
	InitMessCheats(machine);
#endif

	/* ----- initialize flags ----- */
	currentSearchIdx =			0;
	foundCheatDatabase =		0;
	cheatsDisabled =			0;
	watchesDisabled =			0;
	cheatOptions =				2;

	fullMenuPageHeight =	floor(1.0f / ui_get_line_height()) - 1;

	/* ----- initialize CPU info for cheat system ----- */
	BuildCPUInfoList(machine);

	/* ----- load cheat database ----- */
	LoadCheatDatabase(machine);

	ResizeSearchList(1);
	ResizeWatchList(20);

	/* ----- initialize search regions ----- */
	BuildSearchRegions(machine, GetCurrentSearch());
	AllocateSearchRegions(GetCurrentSearch());

	/* ----- initialize string table ----- */
	InitStringTable();

	periodic_timer = timer_alloc(cheat_periodic, NULL);
	timer_adjust_periodic(periodic_timer, video_screen_get_frame_period(machine->primary_screen), 0, video_screen_get_frame_period(machine->primary_screen));

	add_exit_callback(machine, cheat_exit);
}

/*---------------------------------
  cheat_exit - free cheat system
---------------------------------*/

static void cheat_exit(running_machine *machine)
{
	int	i;

	/* ----- save all cheats automatically if needed ----- */
	if(TEST_FIELD(cheatOptions, AutoSaveEnabled))
		DoAutoSaveCheats(machine);

	/* ----- free database ----- */
	DisposeCheatDatabase();

	/* ----- free watch lists ----- */
	if(watchList)
	{
		for(i = 0; i < watchListLength; i++)
			DisposeWatch(&watchList[i]);

		free(watchList);

		watchList = NULL;
	}

	/* ----- free search lists ----- */
	if(searchList)
	{
		for(i = 0; i < searchListLength; i++)
			DisposeSearch(i);

		free(searchList);

		searchList = NULL;
	}

	/* ----- free string table ----- */
	FreeStringTable();

	free(menuItemInfo);
	menuItemInfo = NULL;

#ifdef MESS
	StopMessCheats();
#endif

	/* ----- free flags ----- */
	cheatListLength =			0;
	watchListLength =			0;
	searchListLength =			0;
	currentSearchIdx =			0;
//  cheatEngineWasActive =      0;
	foundCheatDatabase =		0;
	cheatsDisabled =			0;
	watchesDisabled =			0;
	mainDatabaseName[0] =		0;
	menuItemInfoLength =		0;
	cheatOptions =				0;
}

/*-----------------------------------------------------
  cheat_menu - management for the cheat general menu
-----------------------------------------------------*/

int cheat_menu(running_machine *machine, int selection)
{
	enum
	{
		kMenu_EnableDisable = 0,
		kMenu_AddEdit,
		kMenu_Search,
		kMenu_ChooseWatch,
		kMenu_ReloadDatabase,
		kMenu_Options,
		kMenu_Return,

		kMenu_Max
	};

	ui_menu_item	menu_item[kMenu_Max];

	INT32			sel;
	UINT8			total = 0;

	static INT32	submenu_choice = 0;
	static int		firstEntry = 0;
	static int		lastPstn = 0;

	/* ----- allocate memory for item list ----- */
	memset(menu_item, 0, sizeof(menu_item));

//  cheatEngineWasActive = 1;

	sel = lastPstn;

	/********** SUB MENU **********/
	if(submenu_choice)
	{
		switch(sel)
		{
			case kMenu_EnableDisable:
				submenu_choice = EnableDisableCheatMenu(machine, submenu_choice, firstEntry);
				break;

			case kMenu_AddEdit:
				submenu_choice = AddEditCheatMenu(machine, submenu_choice);
				break;

			case kMenu_Search:
				switch(EXTRACT_FIELD(cheatOptions, SearchBox))
				{
					case kSearchBox_Minimum:
						submenu_choice = DoSearchMenuMinimum(machine, submenu_choice);
						break;

					case kSearchBox_Classic:
						submenu_choice = DoSearchMenuClassic(machine, submenu_choice);
						break;

					case kSearchBox_Advanced:
						submenu_choice = DoSearchMenu(machine, submenu_choice);
						break;
				}
				break;

			case kMenu_ChooseWatch:
				submenu_choice = ChooseWatch(machine, submenu_choice);
				break;

			case kMenu_Options:
				submenu_choice = SelectOptions(machine, submenu_choice);
				break;
		}

		firstEntry = 0;

		/* ----- quick menu switching ------ */
		if(submenu_choice < 0)
		{
			switch(submenu_choice)
			{
				case -1:
					lastPstn = kMenu_EnableDisable;
					break;

				case -2:
					lastPstn = kMenu_AddEdit;
					break;

				case -3:
					lastPstn = kMenu_Search;
					break;

				case -4:
					lastPstn = kMenu_ChooseWatch;
					break;

				default:
					ui_popup_time(1, "undefined menu switching : %x", submenu_choice);
			}

			submenu_choice = 1;
		}

		return sel + 1;
	}

	/********** MENU CONSTRUCION **********/
	menu_item[total++].text = "Enable/Disable a Cheat";				// Enable/Disable a Cheat

	menu_item[total++].text = "Add/Edit a Cheat";			// Add/Edit a Cheat

	switch(EXTRACT_FIELD(cheatOptions, SearchBox))						// Search a Cheat
	{
		case kSearchBox_Minimum:
			menu_item[total++].text = "Search a Cheat (Minimum Mode)";
			break;

		case kSearchBox_Classic:
			menu_item[total++].text = "Search a Cheat (Classic Mode)";
			break;

		case kSearchBox_Advanced:
			menu_item[total++].text = "Search a Cheat (Advanced Mode)";
			break;
	}

	menu_item[total++].text = "Configure Watchpoints";				// Configure Watchpoints

	menu_item[total++].text = "Reload Database";			// Reload Cheat Database

	menu_item[total++].text = "Options";					// Options

	menu_item[total++].text = "Return to Main Menu";			// return to the MAME general menu

	menu_item[total].text = NULL;										// terminate array

	/* ----- adjust cursor postion ----- */
	if(sel < 0)
		sel = 0;
	if(sel > (total - 1))
		sel = total - 1;

	/* ----- print it ----- */
	ui_menu_draw(menu_item, total, sel, NULL);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(sel < (total - 1))
			sel++;
		else
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(sel > 0)
			sel--;
		else
			sel = total - 1;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(sel)
		{
			case kMenu_Return:
				submenu_choice = 0;
				sel = -1;
				lastPstn = 0;		// if cursor is now on the return item, adjust cursor keeper
				break;

			case kMenu_ReloadDatabase:
				ReloadCheatDatabase(machine);
				break;

			default:
				firstEntry = 1;
				submenu_choice = 1;
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
		ReloadCheatDatabase(machine);

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(sel == kMenu_Return)
			lastPstn = 0;			// if cursor is now on the return item, adjust cursor keeper

		sel = -1;
	}

	/* ----- keep current cursor position ----- */
	if(sel >= 0)
		lastPstn = sel;

	return sel + 1;
}

/*----------
  DoShift
----------*/

static UINT32 DoShift(UINT32 input, INT8 shift)
{
	if(shift > 0)
		return input >> shift;
	else
		return input << -shift;
}

/*----------------------------------------
  BCDToDecimal - convert a value to hex
----------------------------------------*/

static UINT32 BCDToDecimal(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	multiplier = 1;
	int		i;

	for(i = 0; i < 8; i++)
	{
		accumulator += (value & 0xF) * multiplier;

		multiplier *= 10;
		value >>= 4;
	}

	return accumulator;
}

/*----------------------------------------
  DecimalToBCD - convert a value to dec
----------------------------------------*/

static UINT32 DecimalToBCD(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	divisor = 10;
	int		i;

	for(i = 0; i < 8; i++)
	{
		UINT32	temp;

		temp = value % divisor;
		value -= temp;
		temp /= divisor / 10;

		accumulator += temp << (i * 4);

		divisor *= 10;
	}

	return accumulator;
}

/*----------------------
  RebuildStringTables
----------------------*/

static void RebuildStringTables(void)
{
	UINT32	storageNeeded, i;
	char	* traverse;

	storageNeeded =				(menuStrings.mainStringLength + menuStrings.subStringLength) * menuStrings.numStrings;

	menuStrings.mainList =		(const char **)	realloc((char *)	menuStrings.mainList,		sizeof(char *) * menuStrings.length);
	menuStrings.subList =		(const char **)	realloc((char *)	menuStrings.subList,		sizeof(char *) * menuStrings.length);
	menuStrings.flagList =						realloc(			menuStrings.flagList,		sizeof(char)   * menuStrings.length);
	menuStrings.mainStrings =					realloc(			menuStrings.mainStrings,	sizeof(char *) * menuStrings.numStrings);
	menuStrings.subStrings =					realloc(			menuStrings.subStrings,		sizeof(char *) * menuStrings.numStrings);
	menuStrings.buf =							realloc(			menuStrings.buf,			sizeof(char)   * storageNeeded);

	if(	(!menuStrings.mainList && menuStrings.length) ||
		(!menuStrings.subList && menuStrings.length) ||
		(!menuStrings.flagList && menuStrings.length) ||
		(!menuStrings.mainStrings && menuStrings.numStrings) ||
		(!menuStrings.subStrings && menuStrings.numStrings) ||
		(!menuStrings.buf && storageNeeded))
	{
		fatalerror(	"cheat: memory allocation error\n"
					"	length =			%.8X\n"
					"	numStrings =		%.8X\n"
					"	mainStringLength =	%.8X\n"
					"	subStringLength =	%.8X\n"
					"%p %p %p %p %p %p\n",
					menuStrings.length,
					menuStrings.numStrings,
					menuStrings.mainStringLength,
					menuStrings.subStringLength,

					menuStrings.mainList,
					menuStrings.subList,
					menuStrings.flagList,
					menuStrings.mainStrings,
					menuStrings.subStrings,
					menuStrings.buf);
	}

	traverse = menuStrings.buf;

	for(i = 0; i < menuStrings.numStrings; i++)
	{
		menuStrings.mainStrings[i] = traverse;
		traverse += menuStrings.mainStringLength;

		menuStrings.subStrings[i] = traverse;
		traverse += menuStrings.subStringLength;
	}
}

/*-----------------------------------------------------------
  RequestStrings - check list length and rebuild if needed
-----------------------------------------------------------*/

static void RequestStrings(UINT32 length, UINT32 numStrings, UINT32 mainStringLength, UINT32 subStringLength)
{
	UINT8	changed = 0;

	if(menuStrings.length < length)
	{
		menuStrings.length = length;

		changed = 1;
	}

	if(menuStrings.numStrings < numStrings)
	{
		menuStrings.numStrings = numStrings;

		changed = 1;
	}

	if(menuStrings.mainStringLength < mainStringLength)
	{
		menuStrings.mainStringLength = mainStringLength;

		changed = 1;
	}

	if(menuStrings.subStringLength < subStringLength)
	{
		menuStrings.subStringLength = subStringLength;

		changed = 1;
	}

	if(changed)
		RebuildStringTables();
}

/*------------------
  InitStringTable
------------------*/

static void InitStringTable(void)
{
	memset(&menuStrings, 0, sizeof(MenuStringList));
}

/*------------------
  FreeStringTable
------------------*/

static void FreeStringTable(void)
{
	free((char *)menuStrings.mainList);
	free((char *)menuStrings.subList);
	free(menuStrings.flagList);
	free(menuStrings.mainStrings);
	free(menuStrings.subStrings);
	free(menuStrings.buf);

	memset(&menuStrings, 0, sizeof(MenuStringList));
}

/*---------------------------------------------------------------------------
  CreateStringCopy - copy stirng. It is only called on LoadCheatFile() now
---------------------------------------------------------------------------*/

static char * CreateStringCopy(char * buf)
{
	char	* temp = NULL;

	if(buf)
	{
		size_t	length = strlen(buf) + 1;

		temp = malloc(length);

		if(temp)
			memcpy(temp, buf, length);
	}

	return temp;
}

/*------------------------------------------------------------
  UserSelectValueMenu - management for value-selection menu
------------------------------------------------------------*/

static INT32 UserSelectValueMenu(running_machine *machine, int selection, CheatEntry * entry)
{
	char					buf[2048];
	int						sel;
	CheatAction				* action;
	static INT32			value = -1;
	static int				firstTime = 1;
	int						delta = 0;
	int						displayValue;
	int						keyValue;
	int						forceUpdate = 0;

	sel =		selection - 1;

	action = &entry->actionList[0];

	/* ----- if we're just entering, save the value ----- */
	if(firstTime)
	{
		UINT32	min = EXTRACT_FIELD(action->type, UserSelectMinimum);
		UINT32	max = action->originalDataField + min;

		value = ReadData(action);

		/* ----- and check for valid BCD values ----- */
		if(TEST_FIELD(action->type, UserSelectBCD))
		{
			value = BCDToDecimal(value);
			value = DecimalToBCD(value);
		}

		if(value < min)
			value = max;
		if(value > max)
			value = min;

		action->data = value;
		firstTime = 0;
	}

	displayValue = value;

	/* ----- if the minimum display value is one, add one to the display value ----- */
	if(TEST_FIELD(action->type, UserSelectMinimumDisplay))
	{
		if(TEST_FIELD(action->type, UserSelectBCD))
			displayValue = BCDToDecimal(displayValue);			// bcd -> dec

		displayValue++;

		if(TEST_FIELD(action->type, UserSelectBCD))
			displayValue = DecimalToBCD(displayValue);			// dec -> bcd
	}

	/* ----- print it ----- */
	if(TEST_FIELD(action->type, UserSelectBCD))
		sprintf(buf, "\tSelect a value\n\t%.2X\n", displayValue);
	else
		sprintf(buf, "\tSelect a value\n\t%.2X (%d)\n", displayValue, displayValue);

	/* ----- create fake menu strings ----- */
	strcat(buf, "\t OK ");

	/* ----- print it ----- */
	ui_draw_message_window(buf);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
		delta = -1;

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
		delta = 1;

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		/* ### redundant?? probably can be removed ### */
		if(!firstTime)
		{
			int	i;

			/* ----- copy data field if copy previous value ----- */
			for(i = 0; i < entry->actionListLength; i++)
			{
				CheatAction	* traverse = &entry->actionList[i];

				INT32	copyValue = value;

				if(TEST_FIELD(traverse->type, LinkEnable) && TEST_FIELD(traverse->type, LinkCopyPreviousValue))
				{
					INT32 newData = traverse->originalDataField;

					if(TEST_FIELD(traverse->type, UserSelectBCD))
					{
						copyValue = BCDToDecimal(value);
						copyValue = DecimalToBCD(value);

						newData = BCDToDecimal(newData);
						newData = DecimalToBCD(newData);
					}

					copyValue += newData;

					if(TEST_FIELD(traverse->type, UserSelectBCD))
					{
						copyValue = BCDToDecimal(copyValue);
						copyValue = DecimalToBCD(copyValue);
					}
				}

				if(!i || TEST_FIELD(traverse->type, LinkCopyPreviousValue))
					traverse->data = copyValue;
			}

			ActivateCheat(entry);
		}

		firstTime = 1;
		sel = -1;
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		firstTime = 1;
		sel = -1;
	}

	/* ----- get other key inputs ----- */
	keyValue = ReadHexInput();

	/* ----- BCD check for input key ----- */
	if((keyValue != -1) && TEST_FIELD(action->type, UserSelectBCD) && ((keyValue < 0 ) || (keyValue > 9)))
		keyValue = -1;

	/* ----- if we got a key ----- */
	if(keyValue != -1)
	{
		/* ----- add it ----- */
		value <<= 4;

		switch(EXTRACT_FIELD(action->type, BytesUsed))
		{
			case 0: // 8 bit
				value &= 0xF0;
				break;
			case 1: // 16 bit
				value &= 0xFFF0;
				break;
			case 2: // 24 bit
				value &= 0xFFFFF0;
				break;
			case 3: // 32 bit
				value &= 0xFFFFFFF0;
				break;
		}

		value |= keyValue & 0x0F;

		delta = 0;
		forceUpdate = 1;
	}

	/* ----- wrap-around with BCD stuff. ### this is a really bad way to do this ----- */
	if(delta || forceUpdate)
	{
		INT32	min = EXTRACT_FIELD(action->type, UserSelectMinimum);
		INT32	max = action->originalDataField + min;

		if(TEST_FIELD(action->type, UserSelectBCD))
			value = BCDToDecimal(value);

		value += delta;

		if(TEST_FIELD(action->type, UserSelectBCD))
			value = DecimalToBCD(value);

		if(value < min)
			value = max;
		if(value > max)
			value = min;
	}

	return sel + 1;
}

/*-------------------------------------------------
  CommentMenu - management comment for code menu
-------------------------------------------------*/

static INT32 CommentMenu(running_machine *machine, int selection, CheatEntry * entry)
{
	char	buf[2048];
	int		sel;
	const char	* comment;

	if(!entry)
		return 0;

	sel = selection - 1;

	/********** MENU CONSTRUCTION **********/
	if(entry->comment && entry->comment[0])
		comment = entry->comment;
	else
		comment = "(none)";

	sprintf(buf, "%s\n\t OK ", comment);

	/* ----- print it ----- */
	ui_draw_message_window(buf);

	/********** KEY HANDLING **********/
	if(input_ui_pressed(machine, IPT_UI_SELECT) || input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*--------------------------------------------------------------
  EnableDisableCheatMenu - management for Enable/Disable menu
--------------------------------------------------------------*/

static int EnableDisableCheatMenu(running_machine *machine, int selection, int firstTime)
{
	INT32			sel;
	static INT32	submenu_choice = 0;
	static INT32	submenu_id = 0;		// 0 = none, 1 = comment, 2 = value-selection, 3 = editCheat
	const char		** menu_item;
	const char		** menu_subitem;
	char			* flagBuf;
	INT32			i;
	INT32			total = 0;

	CheatEntry		* entry;

	RequestStrings(cheatListLength + 5, 0, 0, 0);

	menu_item = menuStrings.mainList;
	menu_subitem = menuStrings.subList;
	flagBuf = menuStrings.flagList;

	sel = selection - 1;

	/********** SUB MENU **********/
	if(submenu_choice)
	{
		switch(submenu_id)
		{
			case 1:
				submenu_choice = CommentMenu(machine, submenu_choice, &cheatList[sel]);
				break;

			case 2:
				submenu_choice = UserSelectValueMenu(machine, submenu_choice, &cheatList[sel]);
				break;

			case 3:
				submenu_choice = EditCheatMenu(machine, &cheatList[sel], sel, submenu_choice);
				break;

			default:
				submenu_choice = 0;
		}

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenu_choice == -1)
//      {
//          submenu_choice = 0;
//          sel = -2;
//      }

		return sel + 1;
	}

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < cheatListLength; i++)
	{
		CheatEntry	* traverse = &cheatList[i];

		/* ----- get code title ----- */
		if(traverse->name)
			menu_item[total] = traverse->name;
		else
			menu_item[total] = "null name";

		menu_subitem[total] = NULL;

		/* ----- get subitem ----- */
		if(traverse->flags & kCheatFlag_Select)			// label-selection gets sub-lebel name or "OFF"
		{
			/* ----- no "OFF" item if one shot ----- */
			if((traverse->flags & kCheatFlag_OneShot) && !traverse->selection)
				traverse->selection = 1;					// adjust to 1st linked-code

			/* ----- set sub-label name or "OFF" to subitem ----- */
			if(traverse->selection && (traverse->selection < traverse->actionListLength))
				menu_subitem[total] = traverse->actionList[traverse->selection].optionalName;
			else
				menu_subitem[total] = "Off";
		}
		else							// others get "ON" or "OFF"
		{
			/* ----- add subitems for all cheats that are not comments ----- */
			if(!(traverse->flags & kCheatFlag_Null))
			{
				if(traverse->flags & kCheatFlag_OneShot)
					menu_subitem[total] = "Set";
				else
				{
					if(traverse->flags & kCheatFlag_Active)
						menu_subitem[total] = "On";
					else
						menu_subitem[total] = "Off";
				}
			}
		}

		/* ----- get comment flag ----- */
		if(traverse->comment && traverse->comment[0])
			flagBuf[total++] = 1;
		else
			flagBuf[total++] = 0;
	}

	/* ----- if no code, set special message ----- */
	if(cheatListLength == 0)
	{
		if(foundCheatDatabase)			// the database found but no code
		{
			menu_item[total] = "there are no cheats for this game";
			menu_subitem[total] = NULL;
			flagBuf[total++] = 0;
		}
		else					// the database itself not found
		{
			menu_item[total] = "cheat database not found";
			menu_subitem[total] = NULL;
			flagBuf[total++] = 0;

			menu_item[total] = "unzip it and place it in the MAME directory";
			menu_subitem[total] = NULL;
			flagBuf[total++] = 0;
		}
	}

	/* ----- set return item ----- */
	menu_item[total] = "Return to Prior Menu";
	menu_subitem[total] = NULL;
	flagBuf[total++] = 0;

	menu_item[total] = 0;	// terminate array
	menu_subitem[total] = 0;
	flagBuf[total] = 0;

	/* ----- adjust cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel > (total - 1))
		sel = total - 1;

	/* ----- if cursor is on comment code, skip it ----- */
	if(cheatListLength && firstTime)
	{
		while((sel < total - 1) && (cheatList[sel].flags & kCheatFlag_Null))
			sel++;
	}

	/* ----- draw it ----- */
	old_style_menu(menu_item, menu_subitem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;

		if(cheatListLength)
		{
			/* ----- if cursor is on comment code, skip it ----- */
			for(i = 0; (i < fullMenuPageHeight / 2) && (sel < total - 1) && (cheatList[sel].flags & kCheatFlag_Null); i++)
				sel++;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
		else
		{
			if(cheatListLength)
			{
				/* ----- if cursor is on comment code, skip it ----- */
				for(i = 0; (i < fullMenuPageHeight / 2) && (sel != total - 1) && (cheatList[sel].flags & kCheatFlag_Null); i++)
				{
					sel--;

					if(sel < 0)
						sel = total - 1;
				}
			}
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if((sel >= 0) && (sel < cheatListLength))
		entry = &cheatList[sel];
	else
		entry = NULL;

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		if((sel < (total - 1)) && entry)
		{
			if(entry->flags & kCheatFlag_Select)		// label-selection
			{
				/* ----- move selection to unLinkExtension code or last linked-code ----- */
				do{
					entry->selection--;

					if(entry->flags & kCheatFlag_OneShot)
					{
						if(entry->selection <= 0)
							entry->selection = entry->actionListLength - 1;		// first -> last

						/* ----- no action when press arrow key ----- */
						entry->flags &= ~kCheatFlag_DoOneShot;
					}
					else
					{
						if(entry->selection < 0)
							entry->selection = entry->actionListLength - 1;		// OFF -> last

						if(entry->selection == 0)
							DeactivateCheat(entry);		// OFF
						else
						{
							if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
								ActivateCheat(entry);	// ON
						}
					}
				}while(entry->selection && TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension));
			}
			else						// others
			{
				if(!(entry->flags & kCheatFlag_Null) && !(entry->flags & kCheatFlag_OneShot))
				{
					int active = entry->flags & kCheatFlag_Active;

					active ^= 0x01;

					/* ----- get the user's selected value if needed ----- */
					if((entry->flags & kCheatFlag_UserSelect) && active)
					{
						submenu_id = 2;
						submenu_choice = 1;
					}
					else
					{
						if(active)
							ActivateCheat(entry);		// ON
						else
							DeactivateCheat(entry);		// OFF
					}
				}
			}
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		if((sel < (total - 1)) && entry)
		{
			if(entry->flags & kCheatFlag_Select)		// label-selection
			{
				/* ----- move selection to unLinkExtension code or 1st code ----- */
				do{
					entry->selection++;

					if(entry->flags & kCheatFlag_OneShot)
					{
						if(entry->selection >= entry->actionListLength)
						{
							entry->selection = 1;		// last -> first

							if(entry->selection >= entry->actionListLength)
								entry->selection = 0;
						}
						/* ----- no action when press arrow key ----- */
						entry->flags &= ~kCheatFlag_DoOneShot;
					}
					else				// others
					{
						if(entry->selection >= entry->actionListLength)
						{
							entry->selection = 0;		// last -> OFF

							DeactivateCheat(entry);		// OFF
						}
						else
						{
							if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
								ActivateCheat(entry);	// ON
						}
					}
				}while(entry->selection && TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension));
			}
			else
			{
				if(!(entry->flags & kCheatFlag_Null) && !(entry->flags & kCheatFlag_OneShot))
				{
					int active = entry->flags & kCheatFlag_Active;

					active ^= 0x01;		// toggle On and OFF

					/* ----- get the user's selected value if needed ----- */
					if((entry->flags & kCheatFlag_UserSelect) && active)
					{
						submenu_id = 2;
						submenu_choice = 1;
					}
					else
					{
						if(active)
							ActivateCheat(entry);		// ON
						else
							DeactivateCheat(entry);		// OFF
					}
				}
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(sel == (total - 1))
		{
			/* ----- return to prior menu ----- */
			submenu_choice = 0;
			sel = -1;
		}
		else
		{
			if((sel < (total - 1)) && entry)
			{
				if(ShiftKeyPressed())
				{
					if(entry->comment && entry->comment[0])
					{
						/* ----- display comment ------ */
						submenu_id = 1;
						submenu_choice = 1;
					}
					else
						goto doActivate;
				}
				else
				{
					doActivate:

					if(entry->flags & kCheatFlag_UserSelect)
					{
						/* ----- value-selection menu ----- */
						submenu_id = 2;
						submenu_choice = 1;
					}
					else
					{
						ActivateCheat(entry);		// ON

						/* ----- set do one shot flag if label-selection ----- */
						if((entry->flags & kCheatFlag_Select) && (entry->flags & kCheatFlag_OneShot))
							entry->flags |= kCheatFlag_DoOneShot;

						/* ----- display one shot message if one shot ----- */
						if(entry->flags & kCheatFlag_OneShot)
							ui_popup_time(1, "%s activated", entry->name);
					}
				}
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
		WatchCheatEntry(entry, 0);

	if(ShiftKeyPressed())
	{
		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))			// shift + save = save all codes
		{
			for(i = 0; i < cheatListLength; i++)
				SaveCheat(machine, &cheatList[i], 0, 0);

			ui_popup_time(1, "%d cheats saved", cheatListLength);
		}

		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))			// shift + add = add new emypty code
			AddCheatBefore(sel);

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))		// shift + delete = delete selected code
			DeleteCheatAt(sel);
	}
	else
	{
		if(ControlKeyPressed())
		{
			if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))		// ctrl + save = save activation key
			{
				if((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2))
				{
					SaveCheat(machine, entry, sel, 1);

					ui_popup_time(1, "activation key saved");
				}
				else
					ui_popup_time(1, "no activation key");
			}
		}
		else
		{
			if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
				SaveCheat(machine, entry, 0, 0);			// save current entry
		}
	}

	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT) && entry)		// edit selected code
	{
		submenu_id = 3;
		submenu_choice = 1;
	}

	if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
		ReloadCheatDatabase(machine);

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_IN, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : enable/disable -> add/edit ----- */
		sel = -3;

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_OUT, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : enable/disable -> watchpoint list ----- */
		sel = -5;

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*--------------------------------------------------------
  AddEditCheatMenu - management for Add/Edit cheat menu
--------------------------------------------------------*/

static int AddEditCheatMenu(running_machine *machine, int selection)
{
	INT32			sel;
	static INT32	submenuChoice = 0;
	static INT32	submenuCheat = 0;
	const char		** menu_item;
	INT32			i;
	INT32			total = 0;
	CheatEntry		* entry;

	sel = selection - 1;

	RequestStrings(cheatListLength + 2, 0, 0, 0);

	menu_item = menuStrings.mainList;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		submenuChoice = EditCheatMenu(machine, &cheatList[submenuCheat], submenuCheat, submenuChoice);

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenuChoice == -1)
//      {
//          submenuChoice = 0;
//          sel = -2;
//      }

		return sel + 1;
	}

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < cheatListLength; i++)
	{
		CheatEntry	* traverse = &cheatList[i];

		if(traverse->name)
			menu_item[total++] = traverse->name;		// code title
		else
			menu_item[total++] = "(none)";
	}

	menu_item[total++] = "Return to Prior Menu";		// return

	menu_item[total] = NULL;					// terminate array

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* ----- draw it ----- */
	old_style_menu(menu_item, NULL, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(sel < (total - 1))
		entry = &cheatList[sel];
	else
		entry = NULL;

	if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			for(i = 0; i < cheatListLength; i++)		// shift + save = save all codes
				SaveCheat(machine, &cheatList[i], 0, 0);

			ui_popup_time(1, "%d cheats saved", cheatListLength);
		}
		else
		{
			if(ControlKeyPressed())
			{
				if((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2))
				{
					SaveCheat(machine, entry, sel, 1);	// ctrl + save = save activation key

					ui_popup_time(1, "activation key saved");
				}
				else
					ui_popup_time(1, "no activation key");
			}
			else
				SaveCheat(machine, entry, 0, 0);		// save current entry
		}
	}

	if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
		AddCheatBefore(sel);			// insert empty code

	if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
		DeleteCheatAt(sel);			// delete selected code

	if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
		WatchCheatEntry(entry, 0);				// watch selected code

	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))		// edit selected code
	{
		/* ----- if selected item is not return, go to edit menu ----- */
		if(sel < (total - 1))
		{
			submenuCheat = sel;
			submenuChoice = 1;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		/* ----- if selected item is return, return previous menu ----- */
		if(sel >= (total - 1))
		{
			submenuChoice = 0;
			sel = -1;
		}
		/* ----- otherwise, go to edit menu (same as pressing edit key) ----- */
		else
		{
			submenuCheat = sel;
			submenuChoice = 1;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
		ReloadCheatDatabase(machine);

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_IN, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : add/edit -> search ----- */
		sel = -4;

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_OUT, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : add/edit -> enable/disable ----- */
		sel = -2;

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*------------------------------------------------
  EditCheatMenu - management for edit code menu
------------------------------------------------*/

static int EditCheatMenu(running_machine *machine, CheatEntry * entry, int index, int selection)
{
	static const char *const kTypeNames[] =
	{
		"Normal/Delay",
		"Wait",
		"Ignore Decrement",
		"Watch",
		"Comment",
		"Select"
	};

	static const char *const kNumbersTable[] =
	{
		"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",
		"8",	"9",	"10",	"11",	"12",	"13",	"14",	"15",
		"16",	"17",	"18",	"19",	"20",	"21",	"22",	"23",
		"24",	"25",	"26",	"27",	"28",	"29",	"30",	"31"
	};

	static const char *const kOperationNames[] =
	{
		"Write",
		"Add/Subtract",
		"Force Range",
		"Set/Clear Bits",
		"Write If Match",
		"Unused (5)",
		"Unused (6)",
		"Null"
	};

	static const char *const kAddSubtractNames[] =
	{
		"Add",
		"Subtract"
	};

	static const char *const kSetClearNames[] =
	{
		"Set",
		"Clear"
	};

	static const char *const kPrefillNames[] =
	{
		"None",
		"FF",
		"00",
		"01"
	};

	static const char *const kEndiannessNames[] =
	{
		"Normal",
		"Swap"
	};

	static const char *const kRegionNames[] =
	{
		"CPU1",		"CPU2",		"CPU3",		"CPU4",		"CPU5",		"CPU6",		"CPU7",		"CPU8",
		"GFX1",		"GFX2",		"GFX3",		"GFX4",		"GFX5",		"GFX6",		"GFX7",		"GFX8",
		"PROMS",
		"SOUND1",	"SOUND2",	"SOUND3",	"SOUND4",	"SOUND5",	"SOUND6",	"SOUND7",	"SOUND8",
		"USER1",	"USER2",	"USER3",	"USER4",	"USER5",	"USER6",	"USER7",	"USER8"
	};

	static const char *const kLocationNames[] =
	{
		"Normal",
		"Region",
		"Mapped Memory",
		"Custom",
		"Relative Address",
		"Program Space",
		"Unused (6)",
		"Unused (7)"
	};

	static const char *const kCustomLocationNames[] =
	{
		"Comment",
		"EEPROM",
		"Select",
		"Unused (3)",	"Unused (4)",	"Unused (5)",	"Unused (6)",	"Unused (7)",
		"Unused (8)",	"Unused (9)",	"Unused (10)",	"Unused (11)",	"Unused (12)",
		"Unused (13)",	"Unused (14)",	"Unused (15)",	"Unused (16)",	"Unused (17)",
		"Unused (18)",	"Unused (19)",	"Unused (20)",	"Unused (21)",	"Unused (22)",
		"Unused (23)",	"Unused (24)",	"Unused (25)",	"Unused (26)",	"Unused (27)",
		"Unused (28)",	"Unused (29)",	"Unused (30)",	"Unused (31)"
	};

	static const char *const kSizeNames[] =
	{
		"8 Bit",
		"16 Bit",
		"24 Bit",
		"32 Bit"
	};

	enum
	{
		kType_Name = 0,					//  text        name
										//  NOTE:   read from base cheat (for idx == 0)
		kType_ExtendName,				//  text        extraName
										//  NOTE:   read from subcheat for (idx > 0) && (cheat[0].type == Select)
		kType_Comment,					//  text        comment
										//  NOTE:   read from base cheat (for idx == 0)
		kType_ActivationKey1,			//  key         1st activationKey
										//  NOTE:   read from base cheat (for idx == 0)
		kType_ActivationKey2,			//  key         2nd activationKey
										//  NOTE:   read from base cheat (for idx == 0)
			// if(idx > 0 && (cheat[0].type == Select))
			kType_LinkExtension,		//  select      LinkExtension       Off - On
		kType_Type,						//  select      Type                Normal/Delay - Wait - Ignore Decrement - Watch -
										//                                  Comment - Select
										//  NOTE: also uses location type field for comment and select
		// if((Type != Comment) && (Type != Select))
			// if(Type != Watch)
				kType_OneShot,			//  select      OneShot             Off - On
				kType_RestorePreviousValue,
										//  select      RestorePreviousValue
										//                                  Off - On
			// if((Type == Normal/Delay) || (Type == Wait))
				kType_Delay,			//  value       TypeParameter       0 - 7
			// if(Type == Ignore Decrement)
				kType_IgnoreDecrementBy,//  value       TypeParameter       0 - 7
			// if(Type == Watch)
				kType_WatchSize,		//  value       Data                0x01 - 0xFF (stored as 0x00 - 0xFE)
										//  NOTE: value is packed in to 0x000000FF
				kType_WatchSkip,		//  value       Data                0x00 - 0xFF
										//  NOTE: value is packed in to 0x0000FF00
				kType_WatchPerLine,		//  value       Data                0x00 - 0xFF
										//  NOTE: value is packed in to 0x00FF0000
				kType_WatchAddValue,	//  value       Data                -0x80 - 0x7F
										//  NOTE: value is packed in to 0xFF000000
				kType_WatchFormat,		//  select      TypeParameter       Hex - Decimal - Binary - ASCII
										//  NOTE: value is packed in to 0x03
				kType_WatchLabel,		//  select      TypeParameter       Off - On
										//  NOTE: value is packed in to 0x04
				// and set operation to null
			// else
				kType_Operation,		//  select      Operation           Write - Add/Subtract - Force Range - Set/Clear Bits - Match to Write -
										//                                  Null
			// if((Operation == Write) && (LocationType != Relative Address))
				kType_WriteMask,		//  value       extendData          0x00000000 - 0xFFFFFFFF
			// if(Operation == Add/Subtract)
				kType_AddSubtract,		//  select      OperationParameter  Add - Subtract
				// if(LocationType != Relative Address)
					// if(OperationParameter == Add)
						kType_AddMaximum,
										//  value       extendData          0x00000000 - 0xFFFFFFFF
					// else
						kType_SubtractMinimum,
										//  value       extendData          0x00000000 - 0xFFFFFFFF
			// if((Operation == Force Range) && (LocationType != Relative Address))
				kType_RangeMinimum,		//  value       extendData          0x00 - 0xFF / 0xFFFF
										//  NOTE: value is packed in to upper byte of extendData (as a word)
				kType_RangeMaximum,		//  value       extendData          0x00 - 0xFF / 0xFFFF
										//  NOTE: value is packed in to lower byte of extendData (as a word)
			// if(Operation == Set/Clear)
				kType_SetClear,			//  select      OperationParameter  Set - Clear
			// if(Operation == Match to Write)
				kType_WriteMatch,		//  value       extendData          0x00000000 - 0xFFFFFFFF
			// if((Operation != Null) || (Type == Watch))
				// if(Type != Watch)
					kType_Data,
					kType_UserSelect,		//  select      UserSelectEnable    Off - On
					// if(UserSelect == On)
						kType_UserSelectMinimumDisp,
											//  value       UserSelectMinimumDisplay
											//                                  0 - 1
						kType_UserSelectMinimum,
											//  value       UserSelectMinimum   0 - 1
						kType_UserSelectBCD,//  select      UserSelectBCD       Off - On
						kType_Prefill,		//  select      UserSelectPrefill   None - FF - 00 - 01
					// if(idx > 0)
						kType_CopyPrevious,	//  select      LinkCopyPreviousValue
										//                                  Off - On
				kType_ByteLength,		//  value       BytesUsed           1 - 4
				// if(bytesUsed > 0)
					kType_Endianness,	//  select      Endianness          Normal - Swap
				kType_LocationType,		//  select      LocationType        Normal - Region - Mapped Memory - EEPROM -
										//                                  Relative Address - Program Space
										//  NOTE: also uses LocationParameter for EEPROM type
				// if((LocationType == Normal) || (LocationType == HandlerMemory) || (LocationType == ProgramSpace)
					kType_CPU,			//  value       LocationParameter   0 - 31
				// if(LocationType == Region)
					kType_Region,		//  select      LocationParameter   CPU1 - CPU2 - CPU3 - CPU4 - CPU5 - CPU6 - CPU7 -
										//                                  CPU8 - GFX1 - GFX2 - GFX3 - GFX4 - GFX5 - GFX6 -
										//                                  GFX7 - GFX8 - PROMS - SOUND1 - SOUND2 - SOUND3 -
										//                                  SOUND4 - SOUND5 - SOUND6 - SOUND7 - SOUND8 -
										//                                  USER1 - USER2 - USER3 - USER4 - USER5 - USER6 -
										//                                  USER7
				// if(LocationType == RelativeAddress)
					kType_PackedCPU,	//  value       LocationParameter   0 - 7
										//  NOTE: packed in to upper three bits of LocationParameter
					kType_PackedSize,	//  value       LocationParameter   1 - 4
										//  NOTE: packed in to lower two bits of LocationParameter
					kType_AddressIndex,	//  value       extendData          -0x80000000 - 0x7FFFFFFF
				kType_Address,

		kType_Return,
		kType_Divider,

		kType_Max
	};

	INT32				sel;
	const char			** menuItem;
	const char			** menuSubItem;
	char				* flagBuf;
	char				** extendDataBuf;		// FFFFFFFF (-80000000)
	char				** addressBuf;			// FFFFFFFF
	char				** dataBuf;				// 80000000 (-2147483648)
	char				** watchSizeBuf;		// FF
	char				** watchSkipBuf;		// FF
	char				** watchPerLineBuf;		// FF
	char				** watchAddValueBuf;	// FF
	INT32				i;
	INT32				total = 0;
	MenuItemInfoStruct	* info = NULL;
	CheatAction			* action = NULL;
	UINT8				isSelect = 0;
	static UINT8		editActive = 0;
	UINT32				increment = 1;
	UINT8				dirty = 0;
	static INT32      currentNameTemplate = 0;

	astring *			temp_string_1 = astring_alloc();
	astring *			temp_string_2 = astring_alloc();

	if(!entry)
		return 0;

	/********** MENU CONSTRUCTION **********/
	if(menuItemInfoLength < (kType_Max * entry->actionListLength) + 2)
	{
		menuItemInfoLength = (kType_Max * entry->actionListLength) + 2;

		menuItemInfo = realloc(menuItemInfo, menuItemInfoLength * sizeof(MenuItemInfoStruct));
	}

	RequestStrings((kType_Max * entry->actionListLength) + 2, 7 * entry->actionListLength, 24, 0);

	menuItem =			menuStrings.mainList;
	menuSubItem =		menuStrings.subList;
	flagBuf =			menuStrings.flagList;
	extendDataBuf =		&menuStrings.mainStrings[entry->actionListLength * 0];
	addressBuf =		&menuStrings.mainStrings[entry->actionListLength * 1];
	dataBuf =			&menuStrings.mainStrings[entry->actionListLength * 2];
	watchSizeBuf =		&menuStrings.mainStrings[entry->actionListLength * 3];	// these fields are wasteful
	watchSkipBuf =		&menuStrings.mainStrings[entry->actionListLength * 4];	// but the alternative is even more ugly
	watchPerLineBuf =	&menuStrings.mainStrings[entry->actionListLength * 5];
	watchAddValueBuf =	&menuStrings.mainStrings[entry->actionListLength * 6];

	sel = selection - 1;

	memset(flagBuf, 0, (kType_Max * entry->actionListLength) + 2);

	/* ----- create menu items ----- */
	for(i = 0; i < entry->actionListLength; i++)
	{
		CheatAction	* traverse = &entry->actionList[i];

		UINT32	type =					EXTRACT_FIELD(traverse->type, Type);
		UINT32	typeParameter =			EXTRACT_FIELD(traverse->type, TypeParameter);
		UINT32	operation =				EXTRACT_FIELD(traverse->type, Operation) |
										EXTRACT_FIELD(traverse->type, OperationExtend) << 2;
		UINT32	operationParameter =	EXTRACT_FIELD(traverse->type, OperationParameter);
		UINT32	locationType =			EXTRACT_FIELD(traverse->type, LocationType);
		UINT32	locationParameter =		EXTRACT_FIELD(traverse->type, LocationParameter);

		UINT8		wasCommentOrSelect =	0;

		if(isSelect)
		{
			/* ----- do extend name field ----- */
			menuItemInfo[total].subcheat = i;
			menuItemInfo[total].fieldType = kType_ExtendName;
			menuItem[total] = "Name";

			if(traverse->optionalName)
				menuSubItem[total++] = traverse->optionalName;
			else
				menuSubItem[total++] = "(none)";
		}

		/* ----- create items if 1st code ----- */
		if(i == 0)
		{
			{
				/* ----- do name field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_Name;
				menuItem[total] = "Name";

				if(entry->name)
					menuSubItem[total++] = entry->name;
				else
					menuSubItem[total++] = "(none)";
			}

			{
				/* ----- do comment field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_Comment;
				menuItem[total] = "Comment";

				if(entry->comment)
					menuSubItem[total++] = entry->comment;
				else
					menuSubItem[total++] = "(none)";
			}

			{
				/* ----- do 1st activation key field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_ActivationKey1;
				menuItem[total] = "Activation Key (1st)";

				if((entry->flags & kCheatFlag_HasActivationKey1))
					menuSubItem[total++] = astring_c(input_code_name(temp_string_1, entry->activationKey1));
				else
					menuSubItem[total++] = "(none)";
			}

			{
				/* ----- do 2nd activation key field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_ActivationKey2;
				menuItem[total] = "Activation Key (2nd)";

				if((entry->flags & kCheatFlag_HasActivationKey2))
					menuSubItem[total++] = astring_c(input_code_name(temp_string_2, entry->activationKey2));
				else
					menuSubItem[total++] = "(none)";
			}

			/* ----- is this label-selection cheat ? ----- */
			if((locationType == kLocation_Custom) && (locationParameter == kCustomLocation_Select))
				isSelect = 1;
		}

		/* ----- create item if linked code on label-selection ----- */
		if(i && isSelect)
		{
			/* ----- do link extension field ----- */
			menuItemInfo[total].subcheat = i;
			menuItemInfo[total].fieldType = kType_LinkExtension;
			menuItem[total] = "Link Extension";
			menuSubItem[total++] = TEST_FIELD(traverse->type, LinkExtension) ? "On" : "Off";
		}

		{
			/* ----- do type field ----- */
			menuItemInfo[total].subcheat = i;
			menuItemInfo[total].fieldType = kType_Type;
			menuItem[total] = "Type";

			if(locationType == kLocation_Custom)
			{
				if(locationParameter == kCustomLocation_Comment)
				{
					/* ----- comment ----- */
					wasCommentOrSelect = 1;

					menuSubItem[total++] = kTypeNames[4];
				}
				else
				{
					if(locationParameter == kCustomLocation_Select)
					{
						/* ----- label-selection ----- */
						wasCommentOrSelect = 1;

						menuSubItem[total++] = kTypeNames[5];
					}
					else
						menuSubItem[total++] = kTypeNames[type & 3];
				}
			}
			else
				menuSubItem[total++] = kTypeNames[type & 3];
		}

		/* ----- create items if not comment or label-selection ----- */
		if(!wasCommentOrSelect)
		{
			if(type != kType_Watch)
			{
				{
					/* ----- do one shot field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_OneShot;
					menuItem[total] = "One Shot";
					menuSubItem[total++] = TEST_FIELD(traverse->type, OneShot) ? "On" : "Off";
				}

				{
					/* ----- do restore previous value field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_RestorePreviousValue;
					menuItem[total] = "Restore Previous Value";
					menuSubItem[total++] = TEST_FIELD(traverse->type, RestorePreviousValue) ? "On" : "Off";
				}
			}

			if((type == kType_NormalOrDelay) || (type == kType_WaitForModification))
			{
				/* ----- do delay field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_Delay;

				if(	TEST_FIELD(traverse->type, OneShot) && TEST_FIELD(traverse->type, RestorePreviousValue) &&
					EXTRACT_FIELD(traverse->type, TypeParameter))
					menuItem[total] = "Keep";
				else
					menuItem[total] = "Delay";

				menuSubItem[total++] = kNumbersTable[typeParameter];
			}

			if(type == kType_IgnoreIfDecrementing)
			{
				/* ----- do ignore decrement by field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_IgnoreDecrementBy;
				menuItem[total] = "Ignore Decrement By";
				menuSubItem[total++] = kNumbersTable[typeParameter];
			}

			if(type == kType_Watch)
			{
				{
					/* ----- do watch size field ----- */
					sprintf(watchSizeBuf[i], "%d", (traverse->originalDataField & 0xFF) + 1);

					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchSize;
					menuItem[total] = "Watch Size";
					menuSubItem[total++] = watchSizeBuf[i];
				}

				{
					/* ----- do watch skip field ----- */
					sprintf(watchSkipBuf[i], "%d", (traverse->data >> 8) & 0xFF);

					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchSkip;
					menuItem[total] = "Watch Skip";
					menuSubItem[total++] = watchSkipBuf[i];
				}

				{
					/* ----- do watch per line field ----- */
					sprintf(watchPerLineBuf[i], "%d", (traverse->data >> 16) & 0xFF);

					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchPerLine;
					menuItem[total] = "Watch Per Line";
					menuSubItem[total++] = watchPerLineBuf[i];
				}

				{
					/* ----- do watch add value field ----- */
					{
						INT8	temp = (traverse->data >> 24) & 0xFF;

						if(temp < 0)
							sprintf(watchAddValueBuf[i], "-%.2X", -temp);
						else
							sprintf(watchAddValueBuf[i], "%.2X", temp);
					}

					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchAddValue;
					menuItem[total] = "Watch Add Value";
					menuSubItem[total++] = watchAddValueBuf[i];
				}

				{
					/* ----- do watch format field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchFormat;
					menuItem[total] = "Watch Format";
					menuSubItem[total++] = kWatchDisplayTypeStringList[(typeParameter >> 0) & 0x03];
				}

				{
					/* ----- do watch label field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_WatchLabel;
					menuItem[total] = "Watch Label";
					menuSubItem[total++] = ((typeParameter >> 2) & 0x01) ? "On" : "Off";
				}
			}
			else
			{
				/* ----- do operation field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_Operation;
				menuItem[total] = "Operation";

				if((locationType != kLocation_IndirectIndexed) && (operation == kOperation_SetOrClearBits))
				{
					/* ----- ignore Set/Clear Bit item if not relative address ----- */
					operation = kOperation_WriteMask;

					CLEAR_MASK_FIELD(traverse->type, OperationExtend);
					SET_FIELD(traverse->type, Operation, operation);
				}

				menuSubItem[total++] = kOperationNames[operation];
			}

			if((operation == kOperation_WriteMask) && (locationType != kLocation_IndirectIndexed))
			{
				/* ----- do mask field ----- */
				int	numChars;

				if(traverse->flags & kActionFlag_IgnoreMask)
				{
					menuItemInfo[total].extraData = 0xFFFFFFFF;
					numChars = 8;
				}
				else
				{
					menuItemInfo[total].extraData = kCheatSizeMaskTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
					numChars = kCheatSizeDigitsTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
				}

				sprintf(extendDataBuf[i], "%.*X", numChars, traverse->extendData);

				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_WriteMask;
				menuItem[total] = "Mask";
				menuSubItem[total++] = extendDataBuf[i];
			}

			if(operation == kOperation_AddSubtract)
			{
				{
					/* ----- do add/subtract field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_AddSubtract;
					menuItem[total] = "Add/Subtract";
					menuSubItem[total++] = kAddSubtractNames[operationParameter];
				}

				if(locationType != kLocation_IndirectIndexed)
				{
					if(operationParameter)
					{
						/* ----- do subtract minimum field ----- */
						sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_SubtractMinimum;
						menuItem[total] = "Minimum Boundary";
						menuSubItem[total++] = extendDataBuf[i];
					}
					else
					{
						/* ----- do add maximum field ----- */
						sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_AddMaximum;
						menuItem[total] = "Maximum Boundary";
						menuSubItem[total++] = extendDataBuf[i];
					}
				}
			}

			if((operation == kOperation_ForceRange) && (locationType != kLocation_IndirectIndexed))
			{
				/* ----- do range minimum field ----- */
				if(!EXTRACT_FIELD(traverse->type, BytesUsed))
					sprintf(extendDataBuf[i], "%.2X", (traverse->extendData >> 8) & 0xFF);
				else
					sprintf(extendDataBuf[i], "%.4X", (traverse->extendData >> 16) & 0xFFFF);

				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_RangeMinimum;
				menuItem[total] = "Range Minimum";
				menuSubItem[total++] = extendDataBuf[i];

				/* ----- do range maximum field ----- */
				if(!EXTRACT_FIELD(traverse->type, BytesUsed))
				{
					sprintf(extendDataBuf[i] + 3, "%.2X", (traverse->extendData >> 0) & 0xFF);
					menuSubItem[total] = extendDataBuf[i] + 3;
				}
				else
				{
					sprintf(extendDataBuf[i] + 7, "%.4X", (traverse->extendData >> 0) & 0xFFFF);
					menuSubItem[total] = extendDataBuf[i] + 7;
				}

				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_RangeMaximum;
				menuItem[total++] = "Range Maximum";
			}

			if(operation == kOperation_SetOrClearBits)
			{
				/* ----- do set/clear field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_SetClear;
				menuItem[total] = "Set/Clear";
				menuSubItem[total++] = kSetClearNames[operationParameter];
			}

			if(operation == kOperation_WriteMatch)
			{
				/* ----- do match value field ----- */
				menuItemInfo[total].subcheat = i;
				menuItemInfo[total].fieldType = kType_WriteMatch;
				menuItem[total] = "Match to Write";

				{
					int	numChars;

					if(traverse->flags & kActionFlag_IgnoreMask)
					{
						menuItemInfo[total].extraData = 0xFFFFFFFF;
						numChars = 8;
					}
					else
					{
						menuItemInfo[total].extraData = kCheatSizeMaskTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
						numChars = kCheatSizeDigitsTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
					}

					sprintf(extendDataBuf[i], "%.*X", numChars, traverse->extendData);
				}

				menuSubItem[total++] = extendDataBuf[i];
			}

			if((operation != kOperation_None) || (type == kType_Watch))
			{
				UINT32	userSelect =		TEST_FIELD(entry->actionList[0].type, UserSelectEnable);
				UINT32	bytesUsed =			EXTRACT_FIELD(traverse->type, BytesUsed);

				if(type != kType_Watch)
				{
					{
						/* ----- do data field ----- */

						sprintf(dataBuf[i], "%.*X (%d)", (int)kCheatSizeDigitsTable[bytesUsed], traverse->originalDataField, traverse->originalDataField);

						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_Data;
						menuItemInfo[total].extraData = kCheatSizeMaskTable[bytesUsed];
						menuItem[total] = "Data";
						menuSubItem[total++] = dataBuf[i];
					}

					if(!i)
					{
						/* ----- do user select field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_UserSelect;
						menuItem[total] = "User Select";
						menuSubItem[total++] = userSelect ? "On" : "Off";
					}

					if(!i && userSelect)
					{
						{
							/* ----- do user select minimum displayed value field ----- */
							menuItemInfo[total].subcheat = i;
							menuItemInfo[total].fieldType = kType_UserSelectMinimumDisp;
							menuItem[total] = "Minimum Displayed Value";
							menuSubItem[total++] = kNumbersTable[EXTRACT_FIELD(traverse->type, UserSelectMinimumDisplay)];
						}

						{
							/* ----- do user select minimum value field ----- */
							menuItemInfo[total].subcheat = i;
							menuItemInfo[total].fieldType = kType_UserSelectMinimum;
							menuItem[total] = "Minimum Value";
							menuSubItem[total++] = kNumbersTable[EXTRACT_FIELD(traverse->type, UserSelectMinimum)];
						}
					}

					if(userSelect)
					{
						/* ----- do user select BCD field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_UserSelectBCD;
						menuItem[total] = "BCD";
						menuSubItem[total++] = TEST_FIELD(traverse->type, UserSelectBCD) ? "On" : "Off";
					}

					if(userSelect || isSelect)
					{
						/* ----- do prefill field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_Prefill;
						menuItem[total] = "Prefill";
						menuSubItem[total++] = kPrefillNames[EXTRACT_FIELD(traverse->type, Prefill)];
					}

					if(i && userSelect)
					{
						/* ----- do copy previous value field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_CopyPrevious;
						menuItem[total] = "Copy Previous Value";
						menuSubItem[total++] = TEST_FIELD(traverse->type, LinkCopyPreviousValue) ? "On" : "Off";
					}
				}

				{
					/* ----- do byte length field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_ByteLength;
					menuItem[total] = "Byte Length";
					menuSubItem[total++] = kSizeNames[bytesUsed];
				}

				if(bytesUsed > 0)
				{
					/* ----- do endianness field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_Endianness;
					menuItem[total] = "Endianness";
					menuSubItem[total++] = kEndiannessNames[EXTRACT_FIELD(traverse->type, Endianness)];
				}

				{
					/* ----- do location type field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_LocationType;
					menuItem[total] = "Location";

					if(locationType == kLocation_Custom)
						menuSubItem[total++] = kCustomLocationNames[locationParameter];
					else
						menuSubItem[total++] = kLocationNames[locationType];
				}

				if(	(locationType == kLocation_Standard) || (locationType == kLocation_HandlerMemory) ||
					(locationType == kLocation_ProgramSpace))
				{
					/* ----- do cpu field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_CPU;
					menuItem[total] = "CPU";
					menuSubItem[total++] = kNumbersTable[locationParameter];
				}

				if(locationType == kLocation_MemoryRegion)
				{
					/* ----- do region field ----- */
					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_Region;
					menuItem[total] = "Region";
					menuSubItem[total++] = kRegionNames[locationParameter];
				}

				if(locationType == kLocation_IndirectIndexed)
				{
					{
						/* ----- do packed CPU field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_PackedCPU;
						menuItem[total] = "CPU";
						menuSubItem[total++] = kNumbersTable[(locationParameter >> 2) & 7];
					}

					{
						/* ----- do packed size field ----- */
						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_PackedSize;
						menuItem[total] = "Address Size";
						menuSubItem[total++] = kNumbersTable[(locationParameter & 3) + 1];
					}

					{
						/* ----- do address index field ----- */

						/* ---- swap if negative ----- */
						if(traverse->extendData & 0x80000000)
						{
							int	temp = traverse->extendData;

							temp = -temp;

							sprintf(extendDataBuf[i], "-%.8X", temp);
						}
						else
							sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

						menuItemInfo[total].subcheat = i;
						menuItemInfo[total].fieldType = kType_AddressIndex;
						menuItem[total] = "Address Index";
						menuSubItem[total++] = extendDataBuf[i];
					}
				}

				{
					/* ----- do address field ----- */
					int	charsToPrint = 8;

					switch(EXTRACT_FIELD(traverse->type, LocationType))
					{
						case kLocation_Standard:
						case kLocation_HandlerMemory:
						case kLocation_ProgramSpace:
						{
							CPUInfo	* cpuInfo = &cpuInfoList[EXTRACT_FIELD(traverse->type, LocationParameter)];

							charsToPrint = cpuInfo->addressCharsNeeded;
							menuItemInfo[total].extraData = cpuInfo->addressMask;
						}
						break;

						case kLocation_IndirectIndexed:
						{
							CPUInfo	* cpuInfo = &cpuInfoList[(EXTRACT_FIELD(traverse->type, LocationParameter) >> 2) & 7];

							charsToPrint = cpuInfo->addressCharsNeeded;
							menuItemInfo[total].extraData = cpuInfo->addressMask;
						}
						break;

						default:
							menuItemInfo[total].extraData = 0xFFFFFFFF;
					}

					sprintf(addressBuf[i], "%.*X", charsToPrint, traverse->address);

					menuItemInfo[total].subcheat = i;
					menuItemInfo[total].fieldType = kType_Address;
					menuItem[total] = "Address";
					menuSubItem[total++] = addressBuf[i];
				}
			}
		}

		if(i < (entry->actionListLength - 1))
		{
			menuItemInfo[total].subcheat = i;
			menuItemInfo[total].fieldType = kType_Divider;
			menuItem[total] = "==========";
			menuSubItem[total++] = NULL;
		}
	}

	menuItemInfo[total].subcheat =	0;				// return
	menuItemInfo[total].fieldType =	kType_Return;
	menuItem[total] =				"Return to Prior Menu";
	menuSubItem[total++] =			NULL;

	menuItemInfo[total].subcheat =	0;				// terminate arrey
	menuItemInfo[total].fieldType =	kType_Return;
	menuItem[total] =				NULL;
	menuSubItem[total] =			NULL;

	/* ----- adjuste cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	info = &menuItemInfo[sel];
	action = &entry->actionList[info->subcheat];

	/* ----- higlighted sub-item if edit mode ----- */
	if(editActive)
		flagBuf[sel] = 1;

	/* ---- draw it ----- */
	old_style_menu(menuItem, menuSubItem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(AltKeyPressed())
		increment <<= 4;
	if(ControlKeyPressed())
		increment <<= 8;
	if(ShiftKeyPressed())
		increment <<= 16;

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;
		dirty = 1;

		switch(info->fieldType)
		{
	         case kType_Name:
    	        currentNameTemplate--;

        	    if(currentNameTemplate < 0)
            	{
	               currentNameTemplate = 0;

	               while(kCheatNameTemplates[currentNameTemplate + 1][0])
	               {
    	              currentNameTemplate++;
        	       }
            	}

	            entry->name = realloc(entry->name, strlen(kCheatNameTemplates[currentNameTemplate]) + 1);
    	        strcpy(entry->name, kCheatNameTemplates[currentNameTemplate]);
        	    break;
/*
            case kType_ActivationKey1:
            case kType_ActivationKey2:
                if(info->fieldType == kType_ActivationKey1)
                {
                    entry->activationKey1--;

                    if(entry->activationKey1 < 0)
                        entry->activationKey1 = __code_max - 1;
                    if(entry->activationKey1 >= __code_max)
                        entry->activationKey1 = 0;

                    entry->flags |= kCheatFlag_HasActivationKey1;
                }
                else
                {
                    entry->activationKey2--;

                    if(entry->activationKey2 < 0)
                        entry->activationKey2 = __code_max - 1;
                    if(entry->activationKey2 >= __code_max)
                        entry->activationKey2 = 0;

                    entry->flags |= kCheatFlag_HasActivationKey2;
                }
                break;
*/
			case kType_LinkExtension:
				TOGGLE_MASK_FIELD(action->type, LinkExtension);
				break;

			case kType_Type:
			{
				UINT8	handled = 0;

				CLEAR_MASK_FIELD(action->type, OperationExtend);

				if(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom)
				{
					UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

					if(locationParameter == kCustomLocation_Comment)
					{
						SET_FIELD(action->type, LocationParameter, 0);
						SET_FIELD(action->type, LocationType, kLocation_Standard);
						SET_FIELD(action->type, Type, kType_Watch);
						SET_FIELD(action->type, Operation, kOperation_None);
						SET_MASK_FIELD(action->type, OperationExtend);

						handled = 1;
					}
					else if(locationParameter == kCustomLocation_Select)
					{
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Comment);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);

						handled = 1;
					}
				}

				if(!handled)
				{
					UINT32	type = EXTRACT_FIELD(action->type, Type);

					if(type == kType_NormalOrDelay)
					{
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Select);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);
					}
					else
					{
						SET_FIELD(action->type, Type, type - 1);
					}
				}
			}
			break;

			case kType_OneShot:
				TOGGLE_MASK_FIELD(action->type, OneShot);
				break;

			case kType_RestorePreviousValue:
				TOGGLE_MASK_FIELD(action->type, RestorePreviousValue);
				break;

			case kType_Delay:
			case kType_IgnoreDecrementBy:
			{
				UINT32	delay = (EXTRACT_FIELD(action->type, TypeParameter) - 1) & 7;

				SET_FIELD(action->type, TypeParameter, delay);
			}
			break;

			case kType_WatchSize:
				action->originalDataField = (action->originalDataField & 0xFFFFFF00) | ((action->originalDataField - 0x00000001) & 0x000000FF);
				action->data = action->originalDataField;
				break;

			case kType_WatchSkip:
				action->originalDataField = (action->originalDataField & 0xFFFF00FF) | ((action->originalDataField - 0x00000100) & 0x0000FF00);
				action->data = action->originalDataField;
				break;

			case kType_WatchPerLine:
				action->originalDataField = (action->originalDataField & 0xFF00FFFF) | ((action->originalDataField - 0x00010000) & 0x00FF0000);
				action->data = action->originalDataField;
				break;

			case kType_WatchAddValue:
				action->originalDataField = (action->originalDataField & 0x00FFFFFF) | ((action->originalDataField - 0x01000000) & 0xFF000000);
				action->data = action->originalDataField;
				break;

			case kType_WatchFormat:
			{
				UINT32	typeParameter = EXTRACT_FIELD(action->type, TypeParameter);

				typeParameter = (typeParameter & 0xFFFFFFFC) | ((typeParameter - 0x00000001) & 0x0000003);
				SET_FIELD(action->type, TypeParameter, typeParameter);
			}
			break;

			case kType_WatchLabel:
				SET_FIELD(action->type, TypeParameter, EXTRACT_FIELD(action->type, TypeParameter) ^ 0x00000004);
				break;

			case kType_Operation:
			{
				UINT32	operation = ((EXTRACT_FIELD(action->type, Operation) | EXTRACT_FIELD(action->type, OperationExtend) << 2) - 1) & 7;

				if(operation == kOperation_None)
					operation = kOperation_WriteMatch;

				if(	(operation == kOperation_SetOrClearBits) &&
					!(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed))
						operation = (operation - 1) & 7;

				if(operation >> 2)
					SET_MASK_FIELD(action->type, OperationExtend);
				else
					CLEAR_MASK_FIELD(action->type, OperationExtend);

				SET_FIELD(action->type, Operation, operation);
			}
			break;

			case kType_WriteMask:
			case kType_WriteMatch:
				action->extendData -= increment;
				action->extendData &= info->extraData;
				break;

			case kType_RangeMinimum:
				if(!EXTRACT_FIELD(action->type, BytesUsed))
					action->extendData = (action->extendData & 0xFFFF00FF) | ((action->extendData - 0x00000100) & 0x0000FF00);
				else
					action->extendData = (action->extendData & 0x0000FFFF) | ((action->extendData - 0x00010000) & 0xFFFF0000);
				break;

			case kType_RangeMaximum:
				if(!EXTRACT_FIELD(action->type, BytesUsed))
					action->extendData = (action->extendData & 0xFFFFFF00) | ((action->extendData - 0x00000001) & 0x000000FF);
				else
					action->extendData = (action->extendData & 0xFFFF0000) | ((action->extendData - 0x00000001) & 0x0000FFFF);
				break;

			case kType_AddressIndex:
			case kType_SubtractMinimum:
			case kType_AddMaximum:
				action->extendData -= increment;
				break;

			case kType_AddSubtract:
			case kType_SetClear:
				TOGGLE_MASK_FIELD(action->type, OperationParameter);
				break;

			case kType_Data:
				action->originalDataField -= increment;
				action->originalDataField &= info->extraData;
				action->data = action->originalDataField;
				break;

			case kType_UserSelect:
				TOGGLE_MASK_FIELD(action->type, UserSelectEnable);
				break;

			case kType_UserSelectMinimumDisp:
				TOGGLE_MASK_FIELD(action->type, UserSelectMinimumDisplay);
				break;

			case kType_UserSelectMinimum:
				TOGGLE_MASK_FIELD(action->type, UserSelectMinimum);
				break;

			case kType_UserSelectBCD:
				TOGGLE_MASK_FIELD(action->type, UserSelectBCD);
				break;

			case kType_Prefill:
			{
				UINT32	prefill = (EXTRACT_FIELD(action->type, Prefill) - 1) & 3;

				SET_FIELD(action->type, Prefill, prefill);
			}
				break;

			case kType_CopyPrevious:
				TOGGLE_MASK_FIELD(action->type, LinkCopyPreviousValue);
				break;

			case kType_ByteLength:
			{
				UINT32	length = (EXTRACT_FIELD(action->type, BytesUsed) - 1) & 3;

				SET_FIELD(action->type, BytesUsed, length);
			}
			break;

			case kType_Endianness:
				TOGGLE_MASK_FIELD(action->type, Endianness);
				break;

			case kType_LocationType:
			{
				UINT32	locationType = EXTRACT_FIELD(action->type, LocationType);
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				if(locationType == kLocation_Standard)
				{
					SET_FIELD(action->type, LocationType, kLocation_ProgramSpace);
					SET_FIELD(action->type, LocationParameter, (locationParameter << 2) & 0x1C);
				}
				else if(locationType == kLocation_Custom)
				{
					SET_FIELD(action->type, LocationType, kLocation_HandlerMemory);
					SET_FIELD(action->type, LocationParameter, 0);
				}
				else if(locationType == kLocation_IndirectIndexed)
				{
					SET_FIELD(action->type, LocationType, kLocation_Custom);
					SET_FIELD(action->type, LocationParameter, kCustomLocation_EEPROM);
				}
				else
				{
					locationType--;

					SET_FIELD(action->type, LocationType, locationType);
				}
			}
			break;

			case kType_CPU:
			case kType_Region:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = (locationParameter - 1) & 31;

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedCPU:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = ((locationParameter - 0x04) & 0x1C) | (locationParameter & 0x03);

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedSize:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = ((locationParameter - 0x01) & 0x03) | (locationParameter & 0x1C);

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_Address:
				action->address -= increment;
				action->address &= info->extraData;
				break;

			case kType_Return:
			case kType_Divider:
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;
		dirty = 1;

		switch(info->fieldType)
		{
	         case kType_Name:
    	        currentNameTemplate++;

        	    if((currentNameTemplate < 0) || !kCheatNameTemplates[currentNameTemplate][0])
            	{
	               currentNameTemplate = 0;
    	        }

	            entry->name = realloc(entry->name, strlen(kCheatNameTemplates[currentNameTemplate]) + 1);
    	        strcpy(entry->name, kCheatNameTemplates[currentNameTemplate]);
        	    break;
/*
            case kType_ActivationKey1:
            case kType_ActivationKey2:
                if(info->fieldType == kType_ActivationKey1)
                {
                    entry->activationKey1++;

                    if(entry->activationKey1 < 0)
                        entry->activationKey1 = __code_max - 1;
                    if(entry->activationKey1 >= __code_max)
                        entry->activationKey1 = 0;

                    entry->flags |= kCheatFlag_HasActivationKey1;
                }
                else
                {
                    entry->activationKey2++;

                    if(entry->activationKey2 < 0)
                        entry->activationKey2 = __code_max - 1;
                    if(entry->activationKey2 >= __code_max)
                        entry->activationKey2 = 0;

                    entry->flags |= kCheatFlag_HasActivationKey2;
                }
                break;
*/
			case kType_LinkExtension:
				TOGGLE_MASK_FIELD(action->type, LinkExtension);
				break;

			case kType_Type:
			{
				UINT8	handled = 0;

				CLEAR_MASK_FIELD(action->type, OperationExtend);

				if(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom)
				{
					UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

					if(locationParameter == kCustomLocation_Comment)
					{
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Select);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);

						handled = 1;
					}
					else if(locationParameter == kCustomLocation_Select)
					{
						SET_FIELD(action->type, LocationParameter, 0);
						SET_FIELD(action->type, LocationType, kLocation_Standard);
						SET_FIELD(action->type, Type, 0);

						handled = 1;
					}
				}

				if(!handled)
				{
					UINT32	type = EXTRACT_FIELD(action->type, Type);

					if(type == kType_Watch)
					{
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Comment);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);
					}
					else
					{
						SET_FIELD(action->type, Type, type + 1);

						if((type + 1) == kType_Watch)
						{
							SET_FIELD(action->type, Operation, kOperation_None);
							SET_MASK_FIELD(action->type, OperationExtend);
						}
					}
				}
			}
			break;

			case kType_OneShot:
				TOGGLE_MASK_FIELD(action->type, OneShot);
				break;

			case kType_RestorePreviousValue:
				TOGGLE_MASK_FIELD(action->type, RestorePreviousValue);
				break;

			case kType_Delay:
			case kType_IgnoreDecrementBy:
			{
				UINT32	delay = (EXTRACT_FIELD(action->type, TypeParameter) + 1) & 7;

				SET_FIELD(action->type, TypeParameter, delay);
			}
			break;

			case kType_WatchSize:
				action->originalDataField = (action->originalDataField & 0xFFFFFF00) | ((action->originalDataField + 0x00000001) & 0x000000FF);
				action->data = action->originalDataField;
				break;

			case kType_WatchSkip:
				action->originalDataField = (action->originalDataField & 0xFFFF00FF) | ((action->originalDataField + 0x00000100) & 0x0000FF00);
				action->data = action->originalDataField;
				break;

			case kType_WatchPerLine:
				action->originalDataField = (action->originalDataField & 0xFF00FFFF) | ((action->originalDataField + 0x00010000) & 0x00FF0000);
				action->data = action->originalDataField;
				break;

			case kType_WatchAddValue:
				action->originalDataField = (action->originalDataField & 0x00FFFFFF) | ((action->originalDataField + 0x01000000) & 0xFF000000);
				action->data = action->originalDataField;
				break;

			case kType_WatchFormat:
			{
				UINT32	typeParameter = EXTRACT_FIELD(action->type, TypeParameter);

				typeParameter = (typeParameter & 0xFFFFFFFC) | ((typeParameter + 0x00000001) & 0x0000003);
				SET_FIELD(action->type, TypeParameter, typeParameter);
			}
			break;

			case kType_WatchLabel:
				SET_FIELD(action->type, TypeParameter, EXTRACT_FIELD(action->type, TypeParameter) ^ 0x00000004);
				break;

			case kType_Operation:
			{
				UINT32	operation = ((EXTRACT_FIELD(action->type, Operation) | EXTRACT_FIELD(action->type, OperationExtend) << 2) + 1) & 7;

				if(operation > kOperation_WriteMatch)
					operation = kOperation_WriteMask;

				if(	(operation == kOperation_SetOrClearBits) &&
					!(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed))
						operation = (operation + 1) & 7;

				if(operation >> 2)
					SET_MASK_FIELD(action->type, OperationExtend);
				else
					CLEAR_MASK_FIELD(action->type, OperationExtend);

				SET_FIELD(action->type, Operation, operation);
			}
			break;

			case kType_WriteMask:
			case kType_WriteMatch:
				action->extendData += increment;
				action->extendData &= info->extraData;
				break;

			case kType_RangeMinimum:
				if(!EXTRACT_FIELD(action->type, BytesUsed))
					action->extendData = (action->extendData & 0xFFFF00FF) | ((action->extendData + 0x00000100) & 0x0000FF00);
				else
					action->extendData = (action->extendData & 0x0000FFFF) | ((action->extendData + 0x00010000) & 0xFFFF0000);
				break;

			case kType_RangeMaximum:
				if(!EXTRACT_FIELD(action->type, BytesUsed))
					action->extendData = (action->extendData & 0xFFFFFF00) | ((action->extendData + 0x00000001) & 0x000000FF);
				else
					action->extendData = (action->extendData & 0xFFFF0000) | ((action->extendData + 0x00000001) & 0x0000FFFF);
				break;

			case kType_AddressIndex:
			case kType_SubtractMinimum:
			case kType_AddMaximum:
				action->extendData += increment;
				break;

			case kType_AddSubtract:
			case kType_SetClear:
				TOGGLE_MASK_FIELD(action->type, OperationParameter);
				break;

			case kType_Data:
				action->originalDataField += increment;
				action->originalDataField &= info->extraData;
				action->data = action->originalDataField;
				break;

			case kType_UserSelect:
				TOGGLE_MASK_FIELD(action->type, UserSelectEnable);
				break;

			case kType_UserSelectMinimumDisp:
				TOGGLE_MASK_FIELD(action->type, UserSelectMinimumDisplay);
				break;

			case kType_UserSelectMinimum:
				TOGGLE_MASK_FIELD(action->type, UserSelectMinimum);
				break;

			case kType_UserSelectBCD:
				TOGGLE_MASK_FIELD(action->type, UserSelectBCD);
				break;

			case kType_Prefill:
			{
				UINT32	prefill = (EXTRACT_FIELD(action->type, Prefill) + 1) & 3;

				SET_FIELD(action->type, Prefill, prefill);
			}
				break;

			case kType_CopyPrevious:
				TOGGLE_MASK_FIELD(action->type, LinkCopyPreviousValue);
				break;

			case kType_ByteLength:
			{
				UINT32	length = (EXTRACT_FIELD(action->type, BytesUsed) + 1) & 3;

				SET_FIELD(action->type, BytesUsed, length);
			}
			break;

			case kType_Endianness:
				TOGGLE_MASK_FIELD(action->type, Endianness);
				break;

			case kType_LocationType:
			{
				UINT32	locationType = EXTRACT_FIELD(action->type, LocationType);
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				if(locationType == kLocation_ProgramSpace)
				{
					SET_FIELD(action->type, LocationType, kLocation_Standard);
					SET_FIELD(action->type, LocationParameter, (locationParameter >> 2) & 7);
				}
				else if(locationType == kLocation_Custom)
				{
					SET_FIELD(action->type, LocationType, kLocation_IndirectIndexed);
					SET_FIELD(action->type, LocationParameter, 0);
				}
				else if(locationType == kLocation_HandlerMemory)
				{
					SET_FIELD(action->type, LocationType, kLocation_Custom);
					SET_FIELD(action->type, LocationParameter, kCustomLocation_EEPROM);
				}
				else
				{
					locationType++;

					SET_FIELD(action->type, LocationType, locationType);
				}
			}
			break;

			case kType_CPU:
			case kType_Region:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = (locationParameter + 1) & 31;

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedCPU:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = ((locationParameter + 0x04) & 0x1C) | (locationParameter & 0x03);

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedSize:
			{
				UINT32	locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = ((locationParameter + 0x01) & 0x03) | (locationParameter & 0x1C);

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_Address:
				action->address += increment;
				action->address &= info->extraData;
				break;

			case kType_Return:
			case kType_Divider:
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;

		editActive = 0;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
		{
			editActive = 0;
		}
		else
		{
			switch(info->fieldType)
			{
				case kType_Name:
				case kType_ExtendName:
				case kType_Comment:
				case kType_ActivationKey1:
				case kType_ActivationKey2:
				case kType_WatchSize:
				case kType_WatchSkip:
				case kType_WatchPerLine:
				case kType_WatchAddValue:
				case kType_WriteMask:
				case kType_AddMaximum:
				case kType_SubtractMinimum:
				case kType_RangeMinimum:
				case kType_RangeMaximum:
				case kType_WriteMatch:
				case kType_Data:
				case kType_Address:
					osd_readkey_unicode(1);
					dirty = 1;
					editActive = 1;
					break;

				case kType_Return:
					sel = -1;
					break;
			}
		}
	}

	/********** EDIT MODE **********/
	if(editActive)
	{
		/* ----- do edit text ----- */
		dirty = 1;

		switch(info->fieldType)
		{
			case kType_Name:
				entry->name = DoDynamicEditTextField(entry->name);
				break;

			case kType_ExtendName:
				action->optionalName = DoDynamicEditTextField(action->optionalName);
				break;

			case kType_Comment:
				entry->comment = DoDynamicEditTextField(entry->comment);
				break;

			case kType_ActivationKey1:
			case kType_ActivationKey2:
			{
				if(input_ui_pressed(machine, IPT_UI_CANCEL))
				{
					if(info->fieldType == kType_ActivationKey1)
					{
						entry->activationKey1 = 0;
						entry->flags &= ~kCheatFlag_HasActivationKey1;
					}
					else
					{
						entry->activationKey2 = 0;
						entry->flags &= ~kCheatFlag_HasActivationKey2;
					}

					editActive = 0;
				}
				else
				{
					int	code = input_code_poll_switches(FALSE);

					if(code == KEYCODE_ESC)
					{
						if(info->fieldType == kType_ActivationKey1)
						{
							entry->activationKey1 = 0;
							entry->flags &= ~kCheatFlag_HasActivationKey1;
						}
						else
						{
							entry->activationKey2 = 0;
							entry->flags &= ~kCheatFlag_HasActivationKey2;
						}

						editActive = 0;
					}
					else
					{
						if((code != INPUT_CODE_INVALID) && !input_ui_pressed(machine, IPT_UI_SELECT))
						{
							if(info->fieldType == kType_ActivationKey1)
							{
								entry->activationKey1 = code;
								entry->flags |= kCheatFlag_HasActivationKey1;
							}
							else
							{
								entry->activationKey2 = code;
								entry->flags |= kCheatFlag_HasActivationKey2;
							}

							editActive = 0;
						}
					}
				}
			}
			break;

			case kType_WatchSize:
			{
				UINT32	temp = (action->originalDataField >> 0) & 0xFF;

				temp++;
				temp = DoEditHexField(machine, temp) & 0xFF;
				temp--;

				action->originalDataField = (action->originalDataField & 0xFFFFFF00) | ((temp << 0) & 0x000000FF);
				action->data = action->originalDataField;
			}
			break;

			case kType_WatchSkip:
			{
				UINT32	temp = (action->originalDataField >> 8) & 0xFF;

				temp = DoEditHexField(machine, temp) & 0xFF;

				action->originalDataField = (action->originalDataField & 0xFFFF00FF) | ((temp << 8) & 0x0000FF00);
				action->data = action->originalDataField;
			}
			break;

			case kType_WatchPerLine:
			{
				UINT32	temp = (action->originalDataField >> 16) & 0xFF;

				temp = DoEditHexField(machine, temp) & 0xFF;

				action->originalDataField = (action->originalDataField & 0xFF00FFFF) | ((temp << 16) & 0x00FF0000);
				action->data = action->originalDataField;
			}
			break;

			case kType_WatchAddValue:
			{
				UINT32	temp = (action->originalDataField >> 24) & 0xFF;

				temp = DoEditHexFieldSigned(temp, 0xFFFFFF80) & 0xFF;

				action->originalDataField = (action->originalDataField & 0x00FFFFFF) | ((temp << 24) & 0xFF000000);
				action->data = action->originalDataField;
			}
			break;

			case kType_WriteMask:
				action->extendData = DoEditHexField(machine, action->extendData);
				action->extendData &= info->extraData;
				break;

			case kType_AddMaximum:
			case kType_SubtractMinimum:
			case kType_WriteMatch:
				action->extendData = DoEditHexField(machine, action->extendData);
				break;

			case kType_RangeMinimum:
			{
				UINT32	temp;

				if(!TEST_FIELD(action->type, BytesUsed))
				{
					temp = (action->extendData >> 8) & 0xFF;
					temp = DoEditHexField(machine, temp) & 0xFF;

					action->extendData = (action->extendData & 0xFF) | ((temp << 8) & 0xFF00);
				}
				else
				{
					temp = (action->extendData >> 16) & 0xFFFF;
					temp = DoEditHexField(machine, temp) & 0xFFFF;

					action->extendData = (action->extendData & 0x0000FFFF) | ((temp << 16) & 0xFFFF0000);
				}
			}
			break;

			case kType_RangeMaximum:
			{
				UINT32	temp;

				if(!TEST_FIELD(action->type, BytesUsed))
				{
					temp = action->extendData & 0xFF;
					temp = DoEditHexField(machine, temp) & 0xFF;

					action->extendData = (action->extendData & 0xFF00) | (temp & 0x00FF);
				}
				else
				{
					temp = action->extendData & 0xFFFF;
					temp = DoEditHexField(machine, temp) & 0xFFFF;

					action->extendData = (action->extendData & 0xFFFF0000) | (temp & 0x0000FFFF);
				}
			}
			break;

			case kType_Data:
				action->originalDataField = DoEditHexField(machine, action->originalDataField);
				action->originalDataField &= info->extraData;
				action->data = action->originalDataField;
				break;

			case kType_Address:
				action->address = DoEditHexField(machine, action->address);
				action->address &= info->extraData;
				break;
		}

		if(input_ui_pressed(machine, IPT_UI_CANCEL))
			editActive = 0;
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
		{
			if(ControlKeyPressed())
				if((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2))
				{
					SaveCheat(machine, entry, index, 1);	// save activation key

					ui_popup_time(1, "activation key saved");
				}
				else
					ui_popup_time(1, "no activation key");
			else
				SaveCheat(machine, entry, 0, 0);			// save current entry
		}

		if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
			WatchCheatEntry(entry, 0);

		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
			AddActionBefore(entry, info->subcheat);

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
		{
			if(!info->subcheat)
			{
				/* ----- if selected entry is master, delete current entry then return to previous menu ----- */
				DeleteCheatAt(index);

				return 0;
			}
			else
				DeleteActionAt(entry, info->subcheat);
		}
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		sel = -1;
		editActive = 0;
	}

	if(sel == -1)
	{
		editActive = 0;
		dirty = 1;
	}

	if(dirty)
	{
		UpdateCheatInfo(entry, 0);

		entry->flags |= kCheatFlag_Dirty;
	}

	astring_free(temp_string_1);
	astring_free(temp_string_2);

	return sel + 1;
}

/*-----------------------------------------------------------
  DoSearchMenuMinimum - management for minimum search menu
-----------------------------------------------------------*/

static int DoSearchMenuMinimum(running_machine *machine, int selection)
{
	/* main */
	enum
	{
		kMenu_CPU = 0,
		kMenu_SearchItem,

		kMenu_Divider,

		kMenu_Memory,
		kMenu_ViewResult,
		kMenu_RestoreSearch,

		kMenu_Return,
	};

	/* search item */
	enum
	{
		kItem_Value,
		kItem_Timer,
		kItem_Equal,
		kItem_NotEqual,
		kItem_Less,
		kItem_Greater,

		kItem_Max = kItem_Greater,
	};

	const char		* menuItem[kMenu_Return + 2] = { 0 };
	const char		* menuSubItem[kMenu_Return + 2] = { 0 };

	char			valueBuffer[60];
	char			valueSignedBuffer[60];
	char			cpuBuffer[20];
	char			flagBuf[kMenu_Return + 2] = { 0 };

	SearchInfo		* search = GetCurrentSearch();

	INT32			sel = selection - 1;
	INT32			total =		 0;
	UINT8			doSearch =	 0;

	static INT8		searchItem = 0;
	static INT8		submenuChoice = 0;
	static UINT8	doneSaveMemory = 0;
	static UINT8	firstEntry = 0;
	static UINT8	editActive = 0;
	static UINT8	posnKeep = kMenu_SearchItem;
	static UINT32	increment = 1;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		switch(sel)
		{
			case kMenu_CPU:
				submenuChoice = SelectSearchRegions(machine, submenuChoice, GetCurrentSearch());
				break;

			case kMenu_ViewResult:
				submenuChoice = ViewSearchResults(machine, submenuChoice, firstEntry);
				break;
		}

		firstEntry = 0;

		posnKeep = sel;

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenuChoice == -1)
//          submenuChoice = 0;

		return sel + 1;
	}

	sel = posnKeep;

	/* ----- unselectable comparison search item if no save ----- */
	if(!doneSaveMemory)
		searchItem = kItem_Value;

	/********** MENU CONSTRUCTION **********/
	sprintf(cpuBuffer, "%d", search->targetIdx);		// CPU
	menuItem[total] = "CPU";
	menuSubItem[total++] = cpuBuffer;

	if(search->sign && (search->oldOptions.value & kSearchByteSignBitTable[search->bytes]))
	{
		UINT32	tempValue;

		tempValue = ~search->oldOptions.value + 1;
		tempValue &= kSearchByteUnsignedMaskTable[search->bytes];

		sprintf(valueBuffer, "-%.*X (-%d)", kSearchByteDigitsTable[search->bytes], tempValue, tempValue);
	}
	else
		sprintf(valueBuffer, "%.*X (%d)", kSearchByteDigitsTable[search->bytes],
				search->oldOptions.value & kSearchByteMaskTable[search->bytes],
				search->oldOptions.value & kSearchByteMaskTable[search->bytes]);

	switch(searchItem)
	{
		case kItem_Value:										// value
			menuItem[total] = "Value";
			menuSubItem[total++] = valueBuffer;
			break;

		case kItem_Timer:
			menuItem[total] = "Timer (+ or -)";					// timer

			/* ----- if memory has been saved, set sub-item for timer ----- */
			if(doneSaveMemory)
			{
				if(search->oldOptions.delta & kSearchByteSignBitTable[search->bytes])
				{
					UINT32	tempValue;

					tempValue = ~search->oldOptions.delta + 1;
					tempValue &= kSearchByteUnsignedMaskTable[search->bytes];

					sprintf(valueSignedBuffer, "-%.*X (-%d)", kSearchByteDigitsTable[search->bytes], tempValue, tempValue);
				}
				else
					sprintf(valueSignedBuffer, "%.*X (%d)", kSearchByteDigitsTable[search->bytes],
							search->oldOptions.delta & kSearchByteMaskTable[search->bytes],
							 search->oldOptions.delta & kSearchByteMaskTable[search->bytes]);

				menuSubItem[total++] = valueSignedBuffer;
			}
			break;

		case kItem_Equal:
			menuItem[total] = "Comparison";						// equal
			menuSubItem[total++] = "Equal";
			break;

		case kItem_NotEqual:
			menuItem[total] = "Comparison";						// not equal
			menuSubItem[total++] = "Not Equal";
			break;

		case kItem_Less:
			menuItem[total] = "Comparison";						// less
			menuSubItem[total++] = "Less";
			break;

		case kItem_Greater:
			menuItem[total] = "Comparison";						// greater
			menuSubItem[total++] = "Greater";
			break;
	}

	menuItem[total] = "--------------------";			// divider
	menuSubItem[total++] = NULL;

	if(doneSaveMemory)									// memory
		menuItem[total] = "Initialize Memory";
	else
		menuItem[total] = "Save Memory";
	menuSubItem[total++] = NULL;

	menuItem[total] = "View Last Results";		// view result
	menuSubItem[total++] = NULL;

	menuItem[total] = "Restore Previous Results";	// restore result
	menuSubItem[total++] = NULL;

	menuItem[total] = "Return to Prior Menu";	// return
	menuSubItem[total++] = NULL;

	menuItem[total] = NULL;								// terminate array
	menuSubItem[total] = NULL;

	/* ----- adjust current cursor position ----- */
	if((sel < 0) || (sel >= total))
		sel = kMenu_SearchItem;

	/* ----- higlighted sub-item if edit mode ----- */
	if(editActive)
		flagBuf[sel] = 1;

	/* ----- draw it ----- */
	old_style_menu(menuItem, menuSubItem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			increment <<= 4;

			switch(searchItem)
			{
				case kItem_Value:
					search->oldOptions.value += increment;
					search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
					break;

				case kItem_Timer:
					search->oldOptions.delta += increment;
					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
					break;
			}

			increment >>= 4;
		}
		else
		{
			sel--;

			/* ----- if divider, skip it ----- */
			if(sel == kMenu_Divider)
				sel--;

			if(sel < 0)
				sel = total - 1;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			increment <<= 4;

			switch(searchItem)
			{
				case kItem_Value:
					search->oldOptions.value -= increment;
					search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
					break;

				case kItem_Timer:
					search->oldOptions.delta -= increment;
					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
					break;
			}

			increment >>= 4;
		}
		else
		{
			sel++;

			/* ----- if divider, skip it ----- */
			if(sel == kMenu_Divider)
				sel++;

			if(sel >= total)
				sel = 0;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(searchItem)
			{
				case kItem_Value:
					search->oldOptions.value -= increment;
					search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
					break;

				case kItem_Timer:
					search->oldOptions.delta -= increment;
					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
					break;
			}
		}
		else
		{
			switch(sel)
			{
				case kMenu_CPU:
					if(search->targetIdx > 0)
					{
						search->targetIdx--;

						BuildSearchRegions(machine, search);
						AllocateSearchRegions(search);

						doneSaveMemory = 0;
					}
					break;

				case kMenu_SearchItem:
					if(doneSaveMemory)
						searchItem--;

					if(searchItem < 0)
						searchItem = kItem_Max;
					break;
			}
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(searchItem)
			{
				case kItem_Value:
					search->oldOptions.value += increment;
					search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
					break;

				case kItem_Timer:
					search->oldOptions.delta += increment;
					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
					break;
			}
		}
		else
		{
			switch(sel)
			{
				case kMenu_CPU:
					if(search->targetIdx < cpu_gettotalcpu() - 1)
					{
						search->targetIdx++;

						BuildSearchRegions(machine, search);
						AllocateSearchRegions(search);

						doneSaveMemory = 0;
					}
					break;

				case kMenu_SearchItem:
					if(doneSaveMemory)
						searchItem++;

					if(searchItem > kItem_Max)
						searchItem = kItem_Value;

					break;
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
		{
			switch(searchItem)
			{
				case kItem_Value:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Value;
					search->comparison =	kSearchComparison_EqualTo;
					search->value =			search->oldOptions.value;

					doSearch = 1;
					break;

				case kItem_Timer:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_IncreasedBy;
					search->value =			search->oldOptions.delta;

					doSearch = 1;
					break;
			}

			/* ----- finish edit mode then do search ----- */
			editActive = 0;
		}
		else
		{
			switch(sel)
			{
				case kMenu_CPU:
					submenuChoice = 1;			// go to region selection menu
					break;

				case kMenu_SearchItem:
				{

					switch(searchItem)
					{
						case kItem_Value:
						case kItem_Timer:
							editActive = 1;
							break;

						case kItem_Equal:
							search->bytes =			kSearchSize_8Bit;
							search->lhs =			kSearchOperand_Current;
							search->rhs =			kSearchOperand_Previous;
							search->comparison =	kSearchComparison_EqualTo;

							doSearch = 1;
							break;

						case kItem_NotEqual:
							search->bytes =			kSearchSize_8Bit;
							search->lhs =			kSearchOperand_Current;
							search->rhs =			kSearchOperand_Previous;
							search->comparison =	kSearchComparison_NotEqual;

							doSearch = 1;
							break;

						case kItem_Less:
							search->bytes =			kSearchSize_8Bit;
							search->lhs =			kSearchOperand_Current;
							search->rhs =			kSearchOperand_Previous;
							search->comparison =	kSearchComparison_LessThan;

							doSearch = 1;
							break;

						case kItem_Greater:
							search->bytes =			kSearchSize_8Bit;
							search->lhs =			kSearchOperand_Current;
							search->rhs =			kSearchOperand_Previous;
							search->comparison =	kSearchComparison_GreaterThan;

							doSearch = 1;
							break;
					}
				}
				break;

				case kMenu_Memory:				// save memory to search newly
					doneSaveMemory = 0;

					doSearch = 1;
					break;

				case kMenu_ViewResult:
					if(search->regionListLength)
					{
						/* ----- go to result viewer ----- */
						firstEntry = 1;
						submenuChoice = 1;
					}
					else
						/* ----- if no search region (eg, sms.c in HazeMD), don't open result viewer to avoid the crash ----- */
						ui_popup_time(1, "no search regions");
					break;

				case kMenu_RestoreSearch:
					RestoreSearchBackup(search);
					break;

				case kMenu_Return:
					posnKeep = kMenu_SearchItem;
					sel = -1;
					break;
			}
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_IN, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : search -> watchpoint list ----- */
		sel = -5;

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_OUT, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : search -> add/edit ----- */
		sel = -3;

	/* ----- edit mode ON/OFF ----- */
	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			if((searchItem == kItem_Value) || (kItem_Timer))
				editActive = 1;
		}
	}

	/* ----- memory save ----- */
	if(input_ui_pressed(machine, IPT_UI_CLEAR))
	{
		doneSaveMemory = 0;

		doSearch = 1;
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)
			editActive = 0;
		else
		{
			posnKeep = kMenu_Memory;
			sel = -1;
		}
	}

	/********** SEARCH **********/
	if(doSearch)
	{
		if(!doneSaveMemory)
			/* ----- initialize search ----- */
			InitializeNewSearch(search);

		if(((sel == kMenu_SearchItem) && (searchItem == kItem_Value)) || doneSaveMemory)
		{
			/* ----- do search ----- */
			BackupSearch(search);

			DoSearch(search);
		}

		UpdateSearch(search);

		/* ----- display message ----- */
		if(((sel == kMenu_SearchItem) && (searchItem == kItem_Value)) || doneSaveMemory)
			popmessage("%d results found", search->numResults);
		else
			popmessage("saved all memory regions");

		doneSaveMemory = 1;

		if(search->numResults == 1)
		{
			AddCheatFromFirstResult(search);

			popmessage("1 result found, added to list");
		}
	}

	/********** EDIT **********/
	if((sel == kMenu_SearchItem) && editActive)
	{
		if(searchItem == kItem_Value)
		{
			search->oldOptions.value = DoEditHexField(machine, search->oldOptions.value);

			search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
		}
		else
		{
			if(searchItem == kItem_Timer)
			{
				search->oldOptions.delta = DoEditHexField(machine, search->oldOptions.delta);

				search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
			}
		}
	}

	if(sel >= 0)
		posnKeep = sel;

	return sel + 1;
}

/*-----------------------------------------------------------
  DoSearchMenuClassic - management for classic search menu
-----------------------------------------------------------*/

static int DoSearchMenuClassic(running_machine *machine, int selection)
{
	static const char *const energyStrings[] =
	{
		"Less",
		"Greater",
		"Less or Equal",
		"Greater or Equal",
	};

	static const char *const equalOrNotStrings[] =
	{
		"Equal",
		"Not Equal",
	};

	static const char *const operandStrings[] =
	{
		"Previous Data",
		"First Data",
	};

	/* conversion table */
	static const int kEnergyComparisonConversionTable[] =
	{
		kSearchComparison_LessThan,
		kSearchComparison_GreaterThan,
		kSearchComparison_LessThanOrEqualTo,
		kSearchComparison_GreaterThanOrEqualTo
	};

	static const int kEqualOrNotConversionTable[] =
	{
		kSearchComparison_EqualTo,
		kSearchComparison_NotEqual
	};

	static const int kOperandConversionTable[] =
	{
		kSearchOperand_Previous,
		kSearchOperand_First
	};

	enum
	{
		kMenu_DoMemorySave = 0,

		kMenu_Divider,

		kMenu_CPU,
		kMenu_ValueEqual,
		kMenu_ValueNearTo,
		kMenu_Time,
		kMenu_Energy,
		kMenu_Status,
		kMenu_Operand,

		kMenu_Divider2,

		kMenu_ViewResult,
		kMenu_RestoreSearch,

		kMenu_Return,
	};

	enum
	{
		kEnergy_Less = 0,
		kEnergy_Greater,
		kEnergy_LessOrEqual,
		kEnergy_GreaterOrEqual,
	};

	const char		* menuItem[kMenu_Return + 2] = { 0 };
	const char		* menuSubItem[kMenu_Return + 2] = { 0 };

	char			valueBuffer[60];
	char			valueSignedBuffer[60];
	char			cpuBuffer[20];

	SearchInfo		* search = GetCurrentSearch();

	INT32			sel = selection - 1;
	INT32			total = 0;
	UINT32			increment = 1;
	UINT8			doSearch = 0;

	static INT8		submenuChoice = 0;
	static UINT8	doneSaveMemory = 0;
	static UINT8	firstEntry = 0;
	static int		lastPos = 0;

	sel = lastPos;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		switch(sel)
		{
			case kMenu_CPU:
				submenuChoice = SelectSearchRegions(machine, submenuChoice, GetCurrentSearch());
				break;

			case kMenu_ViewResult:
				submenuChoice = ViewSearchResults(machine, submenuChoice, firstEntry);
				break;
		}

		firstEntry = 0;

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenuChoice == -1)
//          submenuChoice = 0;

		return sel + 1;
	}

	/********** MENU CONSTRUCTION **********/
	if(!doneSaveMemory)
		menuItem[total] = "Save Memory";
	else
		menuItem[total] = "Initialize Memory";

	menuSubItem[total++] = NULL;

	menuItem[total] = "--------------------";
	menuSubItem[total++] = NULL;

	sprintf(cpuBuffer, "%d", search->targetIdx);
	menuItem[total] = "CPU";
	menuSubItem[total++] = cpuBuffer;

	if(search->sign && (search->oldOptions.value & kSearchByteSignBitTable[search->bytes]))
	{
		UINT32	tempValue;

		tempValue = ~search->oldOptions.value + 1;
		tempValue &= kSearchByteUnsignedMaskTable[search->bytes];

		sprintf(valueBuffer, "-%.*X (-%d)", kSearchByteDigitsTable[search->bytes], tempValue, tempValue);
	}
	else
		sprintf(valueBuffer, "%.*X (%d)", kSearchByteDigitsTable[search->bytes], search->oldOptions.value & kSearchByteMaskTable[search->bytes], search->oldOptions.value & kSearchByteMaskTable[search->bytes]);

	menuItem[total] = "Value (Equal)";

	if(sel >= kMenu_ValueNearTo)
		menuSubItem[total++] = " ";
	else
		menuSubItem[total++] = valueBuffer;

	menuItem[total] = "Value (NearTo)";

	if(sel >= kMenu_ValueNearTo)
		menuSubItem[total++] = valueBuffer;
	else
		menuSubItem[total++] = " ";

	menuItem[total] = "Timer (+ or -)";
	menuSubItem[total++] = " ";

	menuItem[total] = "Energy (Less or Greater)";
	menuSubItem[total++] = " ";

	menuItem[total] = "Status (Same or Different)";
	menuSubItem[total++] = " ";

	menuItem[total] = "Comparison Option";
	menuSubItem[total++] = " ";

	menuItem[total] = "--------------------";
	menuSubItem[total++] = NULL;

	menuItem[total] = "View Last Results";		// view result
	menuSubItem[total++] = NULL;

	menuItem[total] = "Restore Previous Results";	// restore result
	menuSubItem[total++] = NULL;

	menuItem[total] = "Return to Prior Menu";	// return
	menuSubItem[total++] = NULL;

	menuItem[total] = NULL;								// terminate array
	menuSubItem[total] = NULL;

	/* ----- if memory has been saved, set sub-menu items ----- */
	if(doneSaveMemory)
	{
		if(search->oldOptions.delta & kSearchByteSignBitTable[search->bytes])
		{
			UINT32	tempValue;

			tempValue = ~search->oldOptions.delta + 1;
			tempValue &= kSearchByteUnsignedMaskTable[search->bytes];

			sprintf(valueSignedBuffer, "-%.*X (-%d)", kSearchByteDigitsTable[search->bytes], tempValue, tempValue);
		}
		else
			sprintf(valueSignedBuffer, "%.*X (%d)", kSearchByteDigitsTable[search->bytes], search->oldOptions.delta & kSearchByteMaskTable[search->bytes], search->oldOptions.delta & kSearchByteMaskTable[search->bytes]);

		menuSubItem[kMenu_Time] = valueSignedBuffer;
		menuSubItem[kMenu_Energy] = energyStrings[search->oldOptions.energy];
		menuSubItem[kMenu_Status] = equalOrNotStrings[search->oldOptions.status];
		menuSubItem[kMenu_Operand] = operandStrings[search->oldOptions.operand];
	}

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* ----- draw it ----- */
	old_style_menu(menuItem, menuSubItem, 0, sel, 0);

	/********** KEY HANDLING **********/
	if(AltKeyPressed())
		increment <<= 4;

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		/* ----- if divider, skip it ----- */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2))
			sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		/* ----- if divider, skip it ----- */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2))
			sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_CPU:
				if(search->targetIdx > 0)
				{
					search->targetIdx--;

					BuildSearchRegions(machine, search);
					AllocateSearchRegions(search);

					doneSaveMemory = 0;
				}
				break;

			case kMenu_ValueEqual:
			case kMenu_ValueNearTo:
				search->oldOptions.value -= increment;

				search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_Time:
				if(doneSaveMemory)
				{
					search->oldOptions.delta -= increment;

					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
				}
				break;

			case kMenu_Energy:
				if(doneSaveMemory)
				{
					if(search->oldOptions.energy < kEnergy_GreaterOrEqual)
						search->oldOptions.energy++;
					else
						search->oldOptions.energy = kEnergy_Less;
				}
				break;

			case kMenu_Status:
				if(doneSaveMemory)
					search->oldOptions.status ^= 1;
				break;

			case kMenu_Operand:
				if(doneSaveMemory)
					search->oldOptions.operand ^= 1;
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_CPU:
				if(search->targetIdx < cpu_gettotalcpu() - 1)
				{
					search->targetIdx++;

					BuildSearchRegions(machine, search);
					AllocateSearchRegions(search);

					doneSaveMemory = 0;
				}
				break;

			case kMenu_ValueEqual:
			case kMenu_ValueNearTo:
				search->oldOptions.value += increment;

				search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_Time:
				if(doneSaveMemory)
				{
					search->oldOptions.delta += increment;

					search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
				}
				break;

			case kMenu_Energy:
				if(doneSaveMemory)
				{
					if(search->oldOptions.energy > kEnergy_Less)
						search->oldOptions.energy--;
					else
						search->oldOptions.energy = kEnergy_GreaterOrEqual;
				}
				break;

			case kMenu_Status:
				if(doneSaveMemory)
					search->oldOptions.status ^= 1;
				break;

			case kMenu_Operand:
				if(doneSaveMemory)
					search->oldOptions.operand ^= 1;
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(sel)
		{
			case kMenu_DoMemorySave:		// save memory to search newly
				doneSaveMemory = 0;

				doSearch = 1;
				break;

			case kMenu_CPU:
				submenuChoice = 1;			// go to region selection menu
				break;

			case kMenu_ValueEqual:
			case kMenu_ValueNearTo:
				search->bytes =			kSearchSize_8Bit;
				search->lhs =			kSearchOperand_Current;
				search->rhs =			kSearchOperand_Value;

				if(sel == kMenu_ValueEqual)
					search->comparison = kSearchComparison_EqualTo;		// search requested value
				else
					search->comparison = kSearchComparison_NearTo;		// search requested and requested -1 value

				search->value =			search->oldOptions.value;

				doSearch = 1;
				break;

			case kMenu_Time:
				search->bytes =			kSearchSize_8Bit;
				search->lhs =			kSearchOperand_Current;
				search->rhs =			kOperandConversionTable[search->oldOptions.operand];
				search->comparison =	kSearchComparison_IncreasedBy;
				search->value =			search->oldOptions.delta;

				doSearch = 1;
				break;

			case kMenu_Energy:
				search->bytes =			kSearchSize_8Bit;
				search->lhs =			kSearchOperand_Current;
				search->rhs =			kOperandConversionTable[search->oldOptions.operand];
				search->comparison =	kEnergyComparisonConversionTable[search->oldOptions.energy];

				doSearch = 1;
				break;

			case kMenu_Status:
				search->bytes =			kSearchSize_8Bit;
				search->lhs =			kSearchOperand_Current;
				search->rhs =			kOperandConversionTable[search->oldOptions.operand];
				search->comparison =	kEqualOrNotConversionTable[search->oldOptions.status];

				doSearch = 1;
				break;

			case kMenu_ViewResult:
				if(search->regionListLength)
				{
					/* ----- go to result viewer ----- */
					firstEntry = 1;
					submenuChoice = 1;
				}
				else
					/* ----- if no search region (eg, sms.c in HazeMD), don't open result viewer to avoid the crash ----- */
					ui_popup_time(1, "no search regions");
				break;

			case kMenu_RestoreSearch:
				/* ----- restore previous results ----- */
				RestoreSearchBackup(search);
				break;

			case kMenu_Return:
				sel = -1;
				lastPos = 0;
				break;

			default:
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_IN, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : search -> watchpoint list ----- */
		sel = -5;

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_OUT, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : search -> add/edit ----- */
		sel = -3;


	if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		/* ----- if cursor is now on the return item, adjust cursor keeper ----- */
		if(sel == kMenu_Return)
			lastPos = 0;

		sel = -1;
	}

	if(doSearch)
	{
		if(!doneSaveMemory)
			/* ----- initialize search ----- */
			InitializeNewSearch(search);

		if( (sel == kMenu_ValueEqual) || (sel == kMenu_ValueNearTo) || doneSaveMemory)
		{
			/* ----- do search ----- */
			BackupSearch(search);

			DoSearch(search);
		}

		UpdateSearch(search);

		if( (sel == kMenu_ValueEqual) || (sel == kMenu_ValueNearTo) || doneSaveMemory)
			popmessage("%d results found", search->numResults);
		else
			popmessage("saved all memory regions");

		doneSaveMemory = 1;

		if(search->numResults == 1)
		{
			AddCheatFromFirstResult(search);

			popmessage("1 result found, added to list");
		}
	}

	/* ----- edit a value if direct key input ----- */
	if((sel == kMenu_ValueEqual) || (sel == kMenu_ValueNearTo))
	{
		search->oldOptions.value = DoEditHexField(machine, search->oldOptions.value);

		search->oldOptions.value &= kSearchByteMaskTable[search->bytes];
	}
	else
	{
		if(sel == kMenu_Time)
		{
			search->oldOptions.delta = DoEditHexField(machine, search->oldOptions.delta);

			search->oldOptions.delta &= kSearchByteMaskTable[search->bytes];
		}
	}

	/* ----- keep current cursor position ----- */
	if(sel >= 0)
		lastPos = sel;

	return sel + 1;
}

/*-----------------------------------------------------
  DoSearchMenu - management for advanced search menu
-----------------------------------------------------*/

static int DoSearchMenu(running_machine *machine, int selection)
{
	/* menu stirngs */
	static const char *const kOperandNameTable[] =
	{
		"Current Data",
		"Previous Data",
		"First Data",
		"Value"
	};

	static const char *const kComparisonNameTable[] =
	{
		"Less",
		"Greater",
		"Equal",
		"Less Or Equal",
		"Greater Or Equal",
		"Not Equal",
		"Increased By Value",
		"Near To"
	};

	static const char *const kSearchByteNameTable[] =
	{
		"1",
		"2",
		"3",
		"4",
		"Bit"
	};

	enum
	{
		kMenu_LHS,
		kMenu_Comparison,
		kMenu_RHS,
		kMenu_Value,

		kMenu_Divider,

		kMenu_Size,
		kMenu_Swap,
		kMenu_Sign,
		kMenu_CPU,
		kMenu_Name,

		kMenu_Divider2,

		kMenu_DoSearch,
		kMenu_SaveMemory,

		kMenu_Divider3,

		kMenu_ViewResult,
		kMenu_RestoreSearch,

		kMenu_Return,

		kMenu_Max
	};

	const char		* menu_item[kMenu_Max + 2] =	{ 0 };
	const char		* menu_subitem[kMenu_Max + 2] =	{ 0 };

	char			valueBuffer[20];
	char			cpuBuffer[20];
	char			flagBuf[kMenu_Max + 2] = { 0 };

	SearchInfo		* search = GetCurrentSearch();

	INT32			sel = selection - 1;
	INT32			total = 0;
	UINT32			increment = 1;

	static INT8	submenuChoice = 0;
	static UINT8	doneMemorySave = 0;
	static UINT8	editActive = 0;
	static UINT8	firstEntry = 0;
	static int		lastSel = 0;

	sel = lastSel;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		switch(sel)
		{
			case kMenu_CPU:
				submenuChoice = SelectSearchRegions(machine, submenuChoice, GetCurrentSearch());
				break;

			case kMenu_ViewResult:
				submenuChoice = ViewSearchResults(machine, submenuChoice, firstEntry);
				break;
		}

		firstEntry = 0;

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenuChoice == -1)
//          submenuChoice = 0;

		return sel + 1;
	}

	/********** MENU CONSTRUCION **********/
	if((search->sign || search->comparison == kSearchComparison_IncreasedBy) && (search->value & kSearchByteSignBitTable[search->bytes]))
	{
		UINT32	tempValue;

		tempValue = ~search->value + 1;
		tempValue &= kSearchByteUnsignedMaskTable[search->bytes];

		sprintf(valueBuffer, "-%.*X", kSearchByteDigitsTable[search->bytes], tempValue);
	}
	else
		sprintf(valueBuffer, "%.*X", kSearchByteDigitsTable[search->bytes], search->value & kSearchByteMaskTable[search->bytes]);

	if(TEST_FIELD(cheatOptions, DontPrintNewLabels))
	{
		menu_item[total] = kOperandNameTable[search->lhs];
		menu_subitem[total++] = NULL;

		menu_item[total] = kComparisonNameTable[search->comparison];
		menu_subitem[total++] = NULL;

		menu_item[total] = kOperandNameTable[search->rhs];
		menu_subitem[total++] = NULL;

		menu_item[total] = valueBuffer;
		menu_subitem[total++] = NULL;
	}
	else
	{
		menu_item[total] = "LHS";
		menu_subitem[total++] = kOperandNameTable[search->lhs];

		menu_item[total] = "Comparison";
		menu_subitem[total++] = kComparisonNameTable[search->comparison];

		menu_item[total] = "RHS";
		menu_subitem[total++] = kOperandNameTable[search->rhs];

		menu_item[total] = "Value";
		menu_subitem[total++] = valueBuffer;
	}

	menu_item[total] = "---";
	menu_subitem[total++] = NULL;

	menu_item[total] = "Size";
	menu_subitem[total++] = kSearchByteNameTable[search->bytes];

	menu_item[total] = "Swap";
	menu_subitem[total++] = search->swap ? "On" : "Off";

	menu_item[total] = "Signed";
	menu_subitem[total++] = search->sign ? "On" : "Off";

	sprintf(cpuBuffer, "%d", search->targetIdx);
	menu_item[total] = "CPU";
	menu_subitem[total++] = cpuBuffer;

	menu_item[total] = "Name";
	if(search->name)
		menu_subitem[total++] = search->name;
	else
		menu_subitem[total++] = "(none)";

	menu_item[total] = "---";
	menu_subitem[total++] = NULL;

	menu_item[total] = "Do Search";
	menu_subitem[total++] = NULL;

	menu_item[total] = "Save Memory";
	menu_subitem[total++] = NULL;

	menu_item[total] = "---";
	menu_subitem[total++] = NULL;

	menu_item[total] = "View Last Results";		// view result
	menu_subitem[total++] = NULL;

	menu_item[total] = "Restore Previous Results";		// restore result
	menu_subitem[total++] = NULL;

	menu_item[total] = "Return to Prior Menu";		// return
	menu_subitem[total++] = NULL;

	menu_item[total] = NULL;								// terminate array
	menu_subitem[total] = NULL;

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* ----- higlighted sub-item if edit mode ----- */
	if(editActive)
		flagBuf[sel] = 1;

	/* ----- draw it ----- */
	old_style_menu(menu_item, menu_subitem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(AltKeyPressed())
		increment <<= 4;
	if(ControlKeyPressed())
		increment <<= 8;
	if(ShiftKeyPressed())
		increment <<= 16;

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		/* ----- if divider, skip it ----- */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2) || (sel == kMenu_Divider3))
			sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		/* ----- if divider, skip it ----- */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2) || (sel == kMenu_Divider3))
			sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_Value:
				search->value -= increment;

				search->value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_LHS:
				search->lhs--;

				if(search->lhs < kSearchOperand_Current)
					search->lhs = kSearchOperand_Max;
				break;

			case kMenu_RHS:
				search->rhs--;

				if(search->rhs < kSearchOperand_Current)
					search->rhs = kSearchOperand_Max;
				break;

			case kMenu_Comparison:
				search->comparison--;

				if(search->comparison < kSearchComparison_LessThan)
					search->comparison = kSearchComparison_Max;
				break;

			case kMenu_Size:
				search->bytes--;

				if(search->bytes < kSearchSize_8Bit)
					search->bytes = kSearchSize_Max;
				break;

			case kMenu_Swap:
				search->swap ^= 1;
				break;

			case kMenu_Sign:
				search->sign ^= 1;
				break;

			case kMenu_CPU:
				if(search->targetIdx > 0)
				{
					search->targetIdx--;

					BuildSearchRegions(machine, search);
					AllocateSearchRegions(search);

					doneMemorySave = 0;
				}
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_Value:
				search->value += increment;

				search->value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_Size:
				search->bytes++;

				if(search->bytes > kSearchSize_Max)
					search->bytes = kSearchSize_8Bit;
				break;

			case kMenu_LHS:
				search->lhs++;

				if(search->lhs > kSearchOperand_Max)
					search->lhs = kSearchOperand_Current;
				break;

			case kMenu_RHS:
				search->rhs++;

				if(search->rhs > kSearchOperand_Max)
					search->rhs = kSearchOperand_Current;
				break;

			case kMenu_Comparison:
				search->comparison++;

				if(search->comparison > kSearchComparison_Max)
					search->comparison = kSearchComparison_LessThan;
				break;

			case kMenu_Swap:
				search->swap ^= 1;
				break;

			case kMenu_Sign:
				search->sign ^= 1;
				break;

			case kMenu_CPU:
				if(search->targetIdx < cpu_gettotalcpu() - 1)
				{
					search->targetIdx++;

					BuildSearchRegions(machine, search);
					AllocateSearchRegions(search);

					doneMemorySave = 0;
				}
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			switch(sel)
			{
				case kMenu_Value:		// edit selected field
				case kMenu_Name:
					editActive = 1;
					break;

				case kMenu_CPU:			// go to region selection menu
					submenuChoice = 1;
					break;

				case kMenu_DoSearch:
					if(!doneMemorySave)
						/* ----- initialize search ----- */
						InitializeNewSearch(search);

					if((!kSearchOperandNeedsInit[search->lhs] && !kSearchOperandNeedsInit[search->rhs]) || doneMemorySave)
					{
						BackupSearch(search);

						DoSearch(search);
					}

					UpdateSearch(search);

					if((!kSearchOperandNeedsInit[search->lhs] && !kSearchOperandNeedsInit[search->rhs]) || doneMemorySave)
						popmessage("%d results found", search->numResults);
					else
						popmessage("saved all memory regions");

					doneMemorySave = 1;

					if(search->numResults == 1)
					{
						AddCheatFromFirstResult(search);

						popmessage("1 result found, added to list");
					}
					break;

				case kMenu_SaveMemory:
					InitializeNewSearch(search);

					UpdateSearch(search);

					popmessage("saved all memory regions");

					doneMemorySave = 1;
					break;

				case kMenu_ViewResult:
					if(search->regionListLength)
					{
						/* ----- go to result viewer ----- */
						firstEntry = 1;
						submenuChoice = 1;
					}
					else
						/* ----- if no search region (eg, sms.c in HazeMD), don't open result viewer to avoid the crash ----- */
						ui_popup_time(1, "no search regions");
					break;

				case kMenu_RestoreSearch:
					/* ----- restore previous results ----- */
					RestoreSearchBackup(search);
					break;

				case kMenu_Return:
					submenuChoice = 0;
					sel = -1;
					break;

				default:
					break;
			}
		}
	}

	/* ----- edit selected field  ----- */
	if(editActive)
	{
		switch(sel)
		{
			case kMenu_Value:
				search->value = DoEditHexField(machine, search->value);

				search->value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_Name:
				search->name = DoDynamicEditTextField(search->name);
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	/* ----- keep current cursor position ----- */
	if(!(sel == -1))
		lastSel = sel;

	return sel + 1;
}

/*---------------------------------------------------------------------
  SelectSearchRegions - management for search regions selection menu
---------------------------------------------------------------------*/

static int SelectSearchRegions(running_machine *machine, int selection, SearchInfo * search)
{
	static const char *const kSearchSpeedList[] =
	{
		"Fast",
		"Medium",
		"Slow",
		"Very Slow",
		"All Memory"
	};

	INT32			sel;
	const char		** menuItem;
	const char		** menuSubItem;
	INT32			i;
	INT32			total = 0;
	SearchRegion	* region;

	if(!search)
		return 0;

	sel = selection - 1;

	RequestStrings(search->regionListLength + 3, 0, 0, 0);

	/********** MENU CONSTRUCTION **********/
	menuItem =		menuStrings.mainList;
	menuSubItem =	menuStrings.subList;

	for(i = 0; i < search->regionListLength; i++)
	{
		SearchRegion	* traverse = &search->regionList[i];

		menuItem[total] = traverse->name;
		menuSubItem[total++] = (traverse->flags & kRegionFlag_Enabled) ? "On" : "Off";
	}

	menuItem[total] = "Search Speed";
	menuSubItem[total++] = kSearchSpeedList[search->searchSpeed];

	menuItem[total] = "Return to Prior Menu";		// return item
	menuSubItem[total++] = NULL;

	menuItem[total] = NULL;			// terminate array
	menuSubItem[total] = NULL;

	/* ----- adjust current cursor positon ----- */
	if(sel < 0)
		sel = 0;
	if(sel > (total - 1))
		sel = total - 1;

	if(sel < search->regionListLength)
		region = &search->regionList[sel];
	else
		region = NULL;

	/* ----- draw it ----- */
	old_style_menu(menuItem, menuSubItem, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		if(sel < search->regionListLength)
		{
			search->regionList[sel].flags ^= kRegionFlag_Enabled;

			AllocateSearchRegions(search);
		}

		if(sel == search->regionListLength)	// set search speed
		{
			if(search->searchSpeed > kSearchSpeed_Fast)
				search->searchSpeed--;
			else
				search->searchSpeed = kSearchSpeed_Max;

			BuildSearchRegions(machine, search);
			AllocateSearchRegions(search);
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		if(sel < search->regionListLength)
		{
			search->regionList[sel].flags ^= kRegionFlag_Enabled;

			AllocateSearchRegions(search);
		}

		if(sel == search->regionListLength)	// set search speed
		{
			if(search->searchSpeed < kSearchSpeed_Max)
				search->searchSpeed++;
			else
				search->searchSpeed = kSearchSpeed_Fast;

			BuildSearchRegions(machine, search);
			AllocateSearchRegions(search);
		}
	}

	if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(ShiftKeyPressed())			// shift + delete = invalid region
		{
			if(region && search)
			{
				InvalidateEntireRegion(search, region);

				ui_popup_time(1, "region invalidated - %d results remain", search->numResults);
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(sel >= total - 1)
			sel = -1;
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*--------------------------------------------------------
  ViewSearchResults - management for search result menu
--------------------------------------------------------*/

static int ViewSearchResults(running_machine *machine, int selection, int firstTime)
{
	enum
	{
		kMenu_Header = 0,
		kMenu_FirstResult,

		kMaxResultsPerPage = 100
	};

	const char		** menu_item;
	char			** buf;
	char			* header;

	SearchInfo		* search = GetCurrentSearch();
	SearchRegion	* region;

	INT32			sel;
	INT32			total = 0;
	INT32			numPages;
	INT32			resultsPerPage;
	INT32			i;
	UINT32			traverse;
	UINT8			hadResults = 0;
	INT32			numToSkip;
	UINT32			resultsFound = 0;
	UINT32			selectedAddress = 0;
	UINT32			selectedOffset = 0;
	UINT8			selectedAddressGood = 0;
	int				goToNextPage = 0;
	int				goToPrevPage = 0;

	RequestStrings(kMaxResultsPerPage + 3, kMaxResultsPerPage + 3, 80, 0);

	menu_item = menuStrings.mainList;
	buf = &menuStrings.mainStrings[1];
	header = menuStrings.mainStrings[0];

	sel = selection - 1;

	/********** FIRST SETTING **********/
	if(firstTime)
	{
		search->currentRegionIdx = 0;
		search->currentResultsPage = 0;

		/* ----- set current REGION for first display ----- */
		for(traverse = 0; traverse < search->regionListLength; traverse++)
		{
			region = &search->regionList[traverse];

			if(region->numResults)
			{
				search->currentRegionIdx = traverse;
				break;
			}
		}
	}

	/* ----- adjust current REGION ----- */
	if(search->currentRegionIdx >= search->regionListLength)
		search->currentRegionIdx = search->regionListLength - 1;
	if(search->currentRegionIdx < 0)
		search->currentRegionIdx = 0;

	region = &search->regionList[search->currentRegionIdx];

	/* ----- set and adjust maximum displayable result in the PAGE ----- */
	resultsPerPage = fullMenuPageHeight - 3;

	if(resultsPerPage <= 0)
		resultsPerPage = 1;
	else if(resultsPerPage > kMaxResultsPerPage)
		resultsPerPage = kMaxResultsPerPage;

	/* ----- get the number of total page ----- */
	if(region->flags & kRegionFlag_Enabled)
		numPages = (region->numResults / kSearchByteStep[search->bytes] + resultsPerPage - 1) / resultsPerPage;
	else
		numPages = 0;

	/* ----- adjust current PAGE ----- */
	if(search->currentResultsPage >= numPages)
		search->currentResultsPage = numPages - 1;
	if(search->currentResultsPage < 0)
		search->currentResultsPage = 0;

	/* ----- set the number of undisplay result ----- */
	numToSkip = resultsPerPage * search->currentResultsPage;

	/********** MENU CONSTRUCTION **********/
	/* ----- construct header item ----- */
	sprintf(header, "%s %d/%d", region->name, search->currentResultsPage + 1, numPages);

	menu_item[total++] = header;

	traverse = 0;

	if((region->length < kSearchByteIncrementTable[search->bytes]) || !(region->flags & kRegionFlag_Enabled))
		; // no results...
	else
	{
		/* ----- construct result item ----- */
		for(i = 0; (i < resultsPerPage) && (traverse < region->length) && (resultsFound < region->numResults);)
		{
			while(!IsRegionOffsetValid(search, region, traverse) && (traverse < region->length))
				traverse += kSearchByteStep[search->bytes];

			if(traverse < region->length)
			{
				if(numToSkip > 0)
					numToSkip--;
				else
				{
					if(total == sel)
					{
						selectedAddress =		region->address + traverse;
						selectedOffset =		traverse;
						selectedAddressGood =	1;
					}

					sprintf(	buf[total],
								"%.8X (%.*X %.*X %.*X)",
								region->address + traverse,
								kSearchByteDigitsTable[search->bytes],
								ReadSearchOperand(kSearchOperand_First, search, region, region->address + traverse),
								kSearchByteDigitsTable[search->bytes],
								ReadSearchOperand(kSearchOperand_Previous, search, region, region->address + traverse),
								kSearchByteDigitsTable[search->bytes],
								ReadSearchOperand(kSearchOperand_Current, search, region, region->address + traverse));

					menu_item[total] = buf[total];
					total++;

					i++;
				}

				traverse += kSearchByteStep[search->bytes];
				resultsFound++;
				hadResults = 1;
			}
		}
	}

	/* ----- set special message if empty REGION ----- */
	if(!hadResults)
	{
		if(search->numResults)
			menu_item[total++] = "no results for this region";
		else
			menu_item[total++] = "no results found";
	}

	menu_item[total++] = "Return to Prior Menu";		// return

	menu_item[total] = NULL;									// terminate array

	/* ----- adjust current cursor position ----- */
	if(sel <= kMenu_Header)
		sel = kMenu_FirstResult;
	if(sel > (total - 1) || !hadResults)
		sel = total - 1;

	/* ----- draw it ----- */
	old_style_menu(menu_item, NULL, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = kMenu_FirstResult;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 1)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(ShiftKeyPressed())
			search->currentResultsPage = numPages - 1;				// shift + right = go to last PAGE
		else
		{
			if(ControlKeyPressed())
			{
				search->currentRegionIdx++;							// ctrl + right = go to next REGION

				if(search->currentRegionIdx >= search->regionListLength)
					search->currentRegionIdx = 0;
			}
			else
				goToNextPage = 1;									// right = go to next PAGE
		}

		sel = kMenu_FirstResult;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(ShiftKeyPressed())
			search->currentResultsPage = 0;							// shift + left = go to first PAGE
		else
		{
			if(ControlKeyPressed())
			{
				search->currentRegionIdx--;							// ctrl + left = go to previous REGION

				if(search->currentRegionIdx < 0)
					search->currentRegionIdx = search->regionListLength - 1;
			}
			else
				goToPrevPage = 1;									// left = go to previous PAGE
		}

		sel = kMenu_FirstResult;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kHorizontalFastKeyRepeatRate))
	{
		sel -=fullMenuPageHeight;

		if(sel <= kMenu_Header)
			sel = kMenu_FirstResult;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kHorizontalFastKeyRepeatRate))
	{
		sel +=fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(input_code_pressed_once(KEYCODE_END))
		search->currentResultsPage = numPages - 1;				// go to last PAGE

	if(input_code_pressed_once(KEYCODE_HOME))
		search->currentResultsPage = 0;							// go to first PAGE

	if(UIPressedRepeatThrottle(machine, IPT_UI_PREV_GROUP, kVerticalKeyRepeatRate))
	{
		search->currentRegionIdx--;								// go to previous REGION

		if(search->currentRegionIdx < 0)
			search->currentRegionIdx = search->regionListLength - 1;

		sel = kMenu_FirstResult;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_NEXT_GROUP, kVerticalKeyRepeatRate))
	{
		search->currentRegionIdx++;								// go to next REGION

		if(search->currentRegionIdx >= search->regionListLength)
			search->currentRegionIdx = 0;

		sel = kMenu_FirstResult;
	}

	if(goToNextPage)
	{
		search->currentResultsPage++;

		/* ----- if current PAGE is the last, go to first PAGE in next REGION ----- */
		if(search->currentResultsPage >= numPages)
		{
			search->currentResultsPage = 0;
			search->currentRegionIdx++;

			/* ----- if current REGION is the last, go to first REGION ----- */
			if(search->currentRegionIdx >= search->regionListLength)
				search->currentRegionIdx = 0;

			/* ----- if next REGION is empty, search next "non-empty" REGION.
                     but incomplete because last REGION is displayed even if empty ----- */
			for(traverse = search->currentRegionIdx; traverse < search->regionListLength; traverse++)
			{
				search->currentRegionIdx = traverse;

				if(search->regionList[traverse].numResults)
					break;
			}
		}

		sel = kMenu_FirstResult;
	}

	if(goToPrevPage)
	{
		search->currentResultsPage--;

		/* ----- if current PAGE is first, go to previous REGION ----- */
		if(search->currentResultsPage < 0)
		{
			search->currentResultsPage = 0;
			search->currentRegionIdx--;

			/* ----- if current REGION is first, go to last REGION ----- */
			if(search->currentRegionIdx < 0)
				search->currentRegionIdx = search->regionListLength - 1;

			/* ----- if previous REGION is empty, search previous "non-empty" REGION.
                     but incomplete because first REGION is displayed even if empty ----- */
			for(i = search->currentRegionIdx; i >= 0; i--)
			{
				search->currentRegionIdx = i;

				if(search->regionList[i].numResults)
					break;
			}

			/* ----- go to last PAGE in previous REGION ----- */
			{
				/* ----- get the number of total page for previous REGION ----- */
				SearchRegion	* newRegion = &search->regionList[search->currentRegionIdx];
				UINT32			nextNumPages = (newRegion->numResults / kSearchByteStep[search->bytes] + resultsPerPage - 1) / resultsPerPage;

				if(nextNumPages <= 0)
					nextNumPages = 1;

				search->currentResultsPage = nextNumPages - 1;
			}
		}

		sel = kMenu_FirstResult;
	}

	if(selectedAddressGood)
	{
		if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
			AddWatchFromResult(search, region, selectedAddress);		// watch selected result

		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
			AddCheatFromResult(search, region, selectedAddress);		// add selected result as new code to cheat list

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
		{
			InvalidateRegionOffset(search, region, selectedOffset);		// delete selected result
			search->numResults--;
			region->numResults--;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			if(region && search)
			{
				InvalidateEntireRegion(search, region);					// invalidate all results in current REGION

				ui_popup_time(1, "region invalidated - %d results remain", search->numResults);
			}
		}
	}

	if((input_ui_pressed(machine, IPT_UI_SELECT) && (sel == total - 1)) || input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*----------------------------------------------------
  ChooseWatch - management for watchpoint list menu
----------------------------------------------------*/

static int ChooseWatch(running_machine *machine, int selection)
{
	const char		** menuItem;
	char			** buf;

	WatchInfo		* watch;

	static INT32	submenuChoice = 0;
	static INT32	submenuWatch = 0;
	INT32			sel;
	INT32			total = 0;
	int				i;

	RequestStrings(watchListLength + 2, watchListLength, 30, 0);

	menuItem = menuStrings.mainList;
	buf = menuStrings.mainStrings;

	sel = selection - 1;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		submenuChoice = EditWatch(machine, &watchList[submenuWatch], submenuChoice);

		/* ----- meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) ----- */
//      if(submenuChoice == -1)
//      {
//          submenuChoice = 0;
//          sel = -2;
//      }

		return sel + 1;
	}

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < watchListLength; i++)
	{
		WatchInfo	* traverse = &watchList[i];

		sprintf(buf[i], "%d:%.*X (%d)", traverse->cpu, cpuInfoList[traverse->cpu].addressCharsNeeded, traverse->address, traverse->numElements);

		menuItem[total] = buf[i];
		total++;
	}

	menuItem[total++] = "Return to Prior Menu";		// return

	menuItem[total] = NULL;									// terminate array

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* ----- draw it ----- */
	old_style_menu(menuItem, NULL, NULL, sel, 0);

	if(sel < watchListLength)
		watch = &watchList[sel];
	else
		watch = NULL;

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(ShiftKeyPressed())
	{
		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))				// shift + add = add new watchpoint
			AddWatchBefore(sel);

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))			// shift + delete = delete selected watchpoint
			DeleteWatchAt(sel);
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))				// save = save selected watchpoint as watchpoint code
		{
			if(watch)
			{
				CheatEntry	entry;

				memset(&entry, 0, sizeof(CheatEntry));

				SetupCheatFromWatchAsWatch(&entry, watch);
				SaveCheat(machine, &entry, 0, 0);
				DisposeCheat(&entry);
			}
		}

		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
		{
			if(watch)
			{
				if(ControlKeyPressed())							// ctrl + add = add selected watchpoing as watchpoint code to cheat list
				{
					CheatEntry	* entry = GetNewCheat();

					DisposeCheat(entry);
					SetupCheatFromWatchAsWatch(entry, watch);
				}
				else
					AddCheatFromWatch(watch);					// add = add selected watchpoint as cheat code to cheat list
			}
		}

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
		{
			if(ControlKeyPressed())
			{
				for(i = 0; i < watchListLength; i++)		// ctrl + delete = disable all watchpoints
					watchList[i].numElements = 0;
			}
			else
			{
				if(watch)									// delete = disable selected watchpoint
					watch->numElements = 0;
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))					// edit = edit selected watchpoint
	{
		if(sel < (total - 1))
		{
			submenuWatch = sel;
			submenuChoice = 1;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(sel == (total - 1))						// return previous menu
		{
			submenuChoice = 0;
			sel = -1;
		}
		else
		{
			if(sel < (total - 1))					// edit selected watchpoint (same as edit key)
			{
				submenuWatch = sel;
				submenuChoice = 1;
			}
		}
	}

	if(input_ui_pressed(machine, IPT_UI_CLEAR))
	{
		/* ----- reset all watchpoints ----- */
		for(i = 0; i < watchListLength; i++)
		{
			if(watch)
			{
				watchList[i].address = 0;
				watchList[i].cpu = 0;
				watchList[i].numElements = 0;
				watchList[i].elementBytes = kWatchSizeConversionTable[0];
				watchList[i].labelType = kWatchLabel_None;
				watchList[i].displayType = kWatchDisplayType_Hex;
				watchList[i].skip = 0;
				watchList[i].elementsPerLine = 0;
				watchList[i].addValue = 0;

				watchList[i].linkedCheat = NULL;

				watchList[i].label[0] = 0;
			}
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_IN, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : watchpoint list -> enable/disable ----- */
		sel = -2;

	if(UIPressedRepeatThrottle(machine, IPT_UI_ZOOM_OUT, kVerticalKeyRepeatRate))
		/* ----- quick menu switch : watchpoint list -> search ----- */
		sel = -4;

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*--------------------------------------------------
  EditWatch - management for watchpoint edit menu
--------------------------------------------------*/

static int EditWatch(running_machine *machine, WatchInfo * entry, int selection)
{
	enum
	{
		kMenu_Address = 0,
		kMenu_CPU,
		kMenu_NumElements,
		kMenu_ElementSize,
		kMenu_LabelType,
		kMenu_TextLabel,
		kMenu_DisplayType,
		kMenu_XPosition,
		kMenu_YPosition,
		kMenu_Skip,
		kMenu_ElementsPerLine,
		kMenu_AddValue,
		kMenu_AddressShift,
		kMenu_DataShift,
		kMenu_XOR,

		kMenu_Return
	};

	static const char *const kWatchSizeStringList[] =
	{
		"8 Bit",
		"16 Bit",
		"24 Bit",
		"32 Bit"
	};

	const char		** menuItem;
	const char		** menuSubItem;
	char			** buf;
	char			* flagBuf;

	INT32			sel;
	INT32			total = 0;
	UINT32			increment = 1;
	static UINT8	editActive = 0;

	/* ----- if no watch entry, direct return ----- */
	if(!entry)
		return 0;

	RequestStrings(kMenu_Return + 2, kMenu_Return, 0, 20);

	menuItem = menuStrings.mainList;
	menuSubItem = menuStrings.subList;
	buf = menuStrings.subStrings;
	flagBuf = menuStrings.flagList;

	memset(flagBuf, 0, kMenu_Return + 2);

	sel = selection - 1;

	/********** MENU CONSTRUCTION **********/
	sprintf(buf[total], "%.*X", cpuInfoList[entry->cpu].addressCharsNeeded, entry->address >> entry->addressShift);
	menuItem[total] = "Address";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->cpu);
	menuItem[total] = "CPU";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->numElements);
	menuItem[total] = "Length";
	menuSubItem[total] = buf[total];
	total++;

	menuItem[total] = "Element Size";
	menuSubItem[total++] = kWatchSizeStringList[entry->elementBytes];

	menuItem[total] = "Label Type";
	menuSubItem[total++] = kWatchLabelStringList[entry->labelType];

	menuItem[total] = "Text Label";
	if(entry->label[0])
		menuSubItem[total++] = entry->label;
	else
		menuSubItem[total++] = "(none)";

	menuItem[total] = "Display Type";
	menuSubItem[total++] = kWatchDisplayTypeStringList[entry->displayType];

	sprintf(buf[total], "%f", entry->x);
	menuItem[total] = "X";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%f", entry->y);
	menuItem[total] = "Y";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->skip);
	menuItem[total] = "Skip Bytes";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->elementsPerLine);
	menuItem[total] = "Elements Per Line";
	menuSubItem[total] = buf[total];
	total++;

	if(entry->addValue < 0)
		sprintf(buf[total], "-%.2X", -entry->addValue);
	else
		sprintf(buf[total], "%.2X", entry->addValue);

	menuItem[total] = "Add Value";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->addressShift);
	menuItem[total] = "Address Shift";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%d", entry->dataShift);
	menuItem[total] = "Data Shift";
	menuSubItem[total] = buf[total];
	total++;

	sprintf(buf[total], "%.*X", kSearchByteDigitsTable[kWatchSizeConversionTable[entry->elementBytes]], entry->xor);
	menuItem[total] = "XOR";
	menuSubItem[total] = buf[total];
	total++;

	menuItem[total] = "Return to Prior Menu";		// return
	menuSubItem[total++] = NULL;

	menuItem[total] = NULL;									// terminate array
	menuSubItem[total] = NULL;

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* ----- higlighted sub-item if edit mode ----- */
	if(editActive)
		flagBuf[sel] = 1;

	/* ----- draw it ----- */
	old_style_menu(menuItem, menuSubItem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(AltKeyPressed())
		increment <<= 4;
	if(ControlKeyPressed())
		increment <<= 8;
	if(ShiftKeyPressed())
		increment <<= 16;

	if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;

		switch(sel)
		{
			case kMenu_Address:
				entry->address = DoShift(entry->address, entry->addressShift);
				entry->address -= increment;
				entry->address = DoShift(entry->address, -entry->addressShift);

				if(entry->locationType != kLocation_MemoryRegion)
					entry->address &= cpuInfoList[entry->cpu].addressMask;

				break;

			case kMenu_CPU:
				entry->cpu--;

				if(entry->cpu >= cpu_gettotalcpu())
					entry->cpu = cpu_gettotalcpu() - 1;

				entry->address &= cpuInfoList[entry->cpu].addressMask;
				break;

			case kMenu_NumElements:
				if(entry->numElements > 0)
					entry->numElements--;
				else
					entry->numElements = 0;
				break;

			case kMenu_ElementSize:
				if(entry->elementBytes > 0)
					entry->elementBytes--;
				else
					entry->elementBytes = 0;

				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->elementBytes]];
				break;

			case kMenu_LabelType:
				if(entry->labelType > 0)
					entry->labelType--;
				else
					entry->labelType = 0;
				break;

			case kMenu_TextLabel:
				break;

			case kMenu_DisplayType:
				if(entry->displayType > 0)
					entry->displayType--;
				else
					entry->displayType = 0;
				break;

			case kMenu_XPosition:
				entry->x -= 0.01f;
				break;

			case kMenu_YPosition:
				entry->y -= 0.01f;
				break;

			case kMenu_Skip:
				if(entry->skip > 0)
					entry->skip--;
				break;

			case kMenu_ElementsPerLine:
				if(entry->elementsPerLine > 0)
					entry->elementsPerLine--;
				break;

			case kMenu_AddValue:
				entry->addValue = (entry->addValue - 1) & 0xFF;
				break;

			case kMenu_AddressShift:
				if(entry->addressShift > -31)
					entry->addressShift--;
				else
					entry->addressShift = 31;
				break;

			case kMenu_DataShift:
				if(entry->dataShift > -31)
					entry->dataShift--;
				else
					entry->dataShift = 31;
				break;

			case kMenu_XOR:
				entry->xor -= increment;
				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->elementBytes]];
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;

		switch(sel)
		{
			case kMenu_Address:
				entry->address = DoShift(entry->address, entry->addressShift);
				entry->address += increment;
				entry->address = DoShift(entry->address, -entry->addressShift);
				entry->address &= cpuInfoList[entry->cpu].addressMask;
				break;

			case kMenu_CPU:
				entry->cpu++;

				if(entry->cpu >= cpu_gettotalcpu())
					entry->cpu = 0;

				entry->address &= cpuInfoList[entry->cpu].addressMask;
				break;

			case kMenu_NumElements:
				entry->numElements++;
				break;

			case kMenu_ElementSize:
				if(entry->elementBytes < kSearchSize_32Bit)
					entry->elementBytes++;
				else
					entry->elementBytes = kSearchSize_32Bit;

				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->elementBytes]];
				break;

			case kMenu_LabelType:
				if(entry->labelType < kWatchLabel_MaxPlusOne - 1)
					entry->labelType++;
				else
					entry->labelType = kWatchLabel_MaxPlusOne - 1;
				break;

			case kMenu_TextLabel:
				break;

			case kMenu_DisplayType:
				if(entry->displayType < kWatchDisplayType_MaxPlusOne - 1)
					entry->displayType++;
				else
					entry->displayType = kWatchDisplayType_MaxPlusOne - 1;
				break;

			case kMenu_XPosition:
				entry->x += 0.01f;
				break;

			case kMenu_YPosition:
				entry->y += 0.01f;
				break;

			case kMenu_Skip:
				entry->skip++;
				break;

			case kMenu_ElementsPerLine:
				entry->elementsPerLine++;
				break;

			case kMenu_AddValue:
				entry->addValue = (entry->addValue + 1) & 0xFF;
				break;

			case kMenu_AddressShift:
				if(entry->addressShift < 31)
					entry->addressShift++;
				else
					entry->addressShift = -31;
				break;

			case kMenu_DataShift:
				if(entry->dataShift < 31)
					entry->dataShift++;
				else
					entry->dataShift = -31;
				break;

			case kMenu_XOR:
				entry->xor += increment;
				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->elementBytes]];
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;

		editActive = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;

		editActive = 0;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			switch(sel)
			{
				case kMenu_Return:
					sel = -1;
					break;

				case kMenu_Address:
				case kMenu_CPU:
				case kMenu_NumElements:
				case kMenu_TextLabel:
				case kMenu_XPosition:
				case kMenu_YPosition:
				case kMenu_AddValue:
				case kMenu_AddressShift:
				case kMenu_DataShift:
				case kMenu_XOR:
					osd_readkey_unicode(1);
					editActive = 1;
			}
		}
	}

	if(editActive)
	{
		switch(sel)
		{
			case kMenu_Address:
				entry->address = DoShift(entry->address, entry->addressShift);
				entry->address = DoEditHexField(machine, entry->address);
				entry->address = DoShift(entry->address, -entry->addressShift);
				entry->address &= cpuInfoList[entry->cpu].addressMask;
				break;

			case kMenu_CPU:
				entry->cpu = DoEditDecField(entry->cpu, 0, cpu_gettotalcpu() - 1);
				entry->address &= cpuInfoList[entry->cpu].addressMask;
				break;

			case kMenu_NumElements:
				entry->numElements = DoEditDecField(entry->numElements, 0, 99);
				break;

			case kMenu_TextLabel:
				DoStaticEditTextField(entry->label, 255);
				break;

			case kMenu_XPosition:
				entry->x = DoEditDecField(entry->x, -1000, 1000);
				break;

			case kMenu_YPosition:
				entry->y = DoEditDecField(entry->y, -1000, 1000);
				break;

			case kMenu_AddValue:
				entry->addValue = DoEditHexFieldSigned(entry->addValue, 0xFFFFFF80) & 0xFF;
				break;

			case kMenu_AddressShift:
				entry->addressShift = DoEditDecField(entry->addressShift, -31, 31);
				break;

			case kMenu_DataShift:
				entry->dataShift = DoEditDecField(entry->dataShift, -31, 31);
				break;

			case kMenu_XOR:
				entry->xor = DoEditHexField(machine, entry->xor);
				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->elementBytes]];
				break;
		}

		if(input_ui_pressed(machine, IPT_UI_CANCEL))
			editActive = 0;
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
			AddCheatFromWatch(entry);

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
			entry->numElements = 0;

		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
		{
			CheatEntry	tempEntry;

			memset(&tempEntry, 0, sizeof(CheatEntry));

			SetupCheatFromWatchAsWatch(&tempEntry, entry);
			SaveCheat(machine, &tempEntry, 0, 0);
			DisposeCheat(&tempEntry);
		}
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*----------------------------------------------
  SelectOptions - management for options menu
----------------------------------------------*/

static int SelectOptions(running_machine *machine, int selection)
{
	enum
	{
		kMenu_SelectSearch = 0,
		kMenu_SearchDialogStyle,
		kMenu_ShowSearchLabels,
		kMenu_AutoSaveCheats,
		kMenu_ShowActivationKeyMessage,
		kMenu_LoadOldFormat,
		kMenu_Debug,
		kMenu_Return,

		kMenu_Max
	};

	INT32			sel;
	static INT32	submenuChoice = 0;
	const char		* menuItem[kMenu_Max + 1];
	const char		* menuSubItem[kMenu_Max + 1];
	int				total = 0;

	sel = selection - 1;

	/********** SUB MENU **********/
	if(submenuChoice)
	{
		switch(sel)
		{
			case kMenu_SelectSearch:
				submenuChoice = SelectSearch(machine, submenuChoice);
				break;

			default:
				submenuChoice = 0;
				sel = -1;
				break;
		}

		if(submenuChoice == -1)
			submenuChoice = 0;

		return sel + 1;
	}

	/********** MENU CONSTRUCTION **********/
	menuItem[total] =		"Select Search";
	menuSubItem[total++] =	NULL;

	menuItem[total] =		"Search Dialog Style";
	switch(EXTRACT_FIELD(cheatOptions, SearchBox))
	{
		case kSearchBox_Minimum:
			menuSubItem[total++] = "Minimum";
			break;

		case kSearchBox_Classic:
			menuSubItem[total++] = "Classic";
			break;

		case kSearchBox_Advanced:
			menuSubItem[total++] = "Advanced";
			break;
	}

	menuItem[total] =		"Show Search Labels";
	menuSubItem[total++] =	TEST_FIELD(cheatOptions, DontPrintNewLabels) ? "Off" : "On";

	menuItem[total] =		"Auto Save Cheats";
	menuSubItem[total++] =	TEST_FIELD(cheatOptions, AutoSaveEnabled) ? "On" : "Off";

	menuItem[total] =		"Show Activation Key Message";
	menuSubItem[total++] =	TEST_FIELD(cheatOptions, ActivationKeyMessage) ? "On" : "Off";

	menuItem[total] =		"Load Old Format Code";
	menuSubItem[total++] =	TEST_FIELD(cheatOptions, LoadOldFormat) ? "On" : "Off";

	menuItem[total] =		"Debug";
	menuSubItem[total++] =	TEST_FIELD(cheatOptions, Debug) ? "On" : "Off";

	menuItem[total] =		"Return to Prior Menu";		// return
	menuSubItem[total++] =	NULL;

	menuItem[total] =		NULL;								// terminate array
	menuSubItem[total] =	NULL;

	/* ----- draw it ----- */
	old_style_menu(menuItem, menuSubItem, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_SearchDialogStyle:
				{
					UINT8	length = (EXTRACT_FIELD(cheatOptions, SearchBox) + 1) & 3;

					if(length > kSearchBox_Advanced)
						length = kSearchBox_Minimum;

					SET_FIELD(cheatOptions, SearchBox, length);
				}
				break;

			case kMenu_ShowSearchLabels:
				TOGGLE_MASK_FIELD(cheatOptions, DontPrintNewLabels);
				break;

			case kMenu_AutoSaveCheats:
				TOGGLE_MASK_FIELD(cheatOptions, AutoSaveEnabled);
				break;

			case kMenu_ShowActivationKeyMessage:
				TOGGLE_MASK_FIELD(cheatOptions, ActivationKeyMessage);
				break;

			case kMenu_LoadOldFormat:
				TOGGLE_MASK_FIELD(cheatOptions, LoadOldFormat);
				break;

			case kMenu_Debug:
				TOGGLE_MASK_FIELD(cheatOptions, Debug);
				break;
		}
	}

	 if(UIPressedRepeatThrottle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		switch(sel)
		{
			case kMenu_SearchDialogStyle:
				{
					UINT8	length = (EXTRACT_FIELD(cheatOptions, SearchBox) - 1) & 3;

					if(length > kSearchBox_Advanced)
						length = kSearchBox_Advanced;

					SET_FIELD(cheatOptions, SearchBox, length);
				}
				break;

			case kMenu_ShowSearchLabels:
				TOGGLE_MASK_FIELD(cheatOptions, DontPrintNewLabels);
				break;

			case kMenu_AutoSaveCheats:
				TOGGLE_MASK_FIELD(cheatOptions, AutoSaveEnabled);
				break;

			case kMenu_ShowActivationKeyMessage:
				TOGGLE_MASK_FIELD(cheatOptions, ActivationKeyMessage);
				break;

			case kMenu_LoadOldFormat:
				TOGGLE_MASK_FIELD(cheatOptions, LoadOldFormat);
				break;

			case kMenu_Debug:
				TOGGLE_MASK_FIELD(cheatOptions, Debug);
				break;
		}
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(sel < (kMenu_Max - 1))
			sel++;
		else
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(sel > 0)
			sel--;
		else
			sel = kMenu_Max - 1;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(sel)
		{
			case kMenu_Return:
				submenuChoice = 0;
				sel = -1;
				break;

			case kMenu_SelectSearch:
				submenuChoice = 1;
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		SaveCheat(machine, NULL, 0, 2);

		ui_popup_time(1, "command code saved");
	}

	if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
		ReloadCheatDatabase(machine);

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*------------------------------------------------------
  SelectSearch - management for search selection menu
------------------------------------------------------*/

static int SelectSearch(running_machine *machine, int selection)
{
	INT32			sel;
	const char		** menuItem;
	char			** buf;
	INT32			i;
	INT32			total = 0;

	sel = selection - 1;

	RequestStrings(searchListLength + 2, searchListLength, 300, 0);

	menuItem =	menuStrings.mainList;
	buf =		menuStrings.mainStrings;

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < searchListLength; i++)
	{
		SearchInfo	* info = &searchList[i];

		if(i == currentSearchIdx)
		{
			if(info->name)
				sprintf(buf[total], "[#%d: %s]", i, info->name);
			else
				sprintf(buf[total], "[#%d]", i);
		}
		else
		{
			if(info->name)
				sprintf(buf[total], "#%d: %s", i, info->name);
			else
				sprintf(buf[total], "#%d", i);
		}

		menuItem[total] = buf[total];
		total++;
	}

	menuItem[total++] = "Return to Prior Menu";		// return

	menuItem[total] = NULL;									// terminate array

	/* ----- adjust current cursor position ----- */
	if(sel < 0)
		sel = 0;
	if(sel > (total - 1))
		sel = total - 1;

	/* ----- draw it ----- */
	old_style_menu(menuItem, NULL, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(UIPressedRepeatThrottle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(UIPressedRepeatThrottle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
	{
		AddSearchBefore(sel);

		BuildSearchRegions(machine, &searchList[sel]);
		AllocateSearchRegions(&searchList[sel]);
	}

	if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(searchListLength > 1)
			DeleteSearchAt(sel);
	}

	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(sel < total - 1)
			currentSearchIdx = sel;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(sel >= total - 1)
			sel = -1;
		else
			currentSearchIdx = sel;
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	return sel + 1;
}

/*------------------
  cheate_periodic
------------------*/

static TIMER_CALLBACK( cheat_periodic )
{
	int	i;

	if(input_ui_pressed(machine, IPT_UI_TOGGLE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			/* ------ toggle watchpoint display if shift + toggle cheat ----- */
			watchesDisabled ^= 1;

			ui_popup_time(1, "Watchpoints %s", watchesDisabled ? "Off" : "On");
		}
		else
		{
			/* ------ toggle cheat ----- */
			cheatsDisabled ^= 1;

			ui_popup_time(1, "Cheats %s", cheatsDisabled ? "Off" : "On");

			if(cheatsDisabled)
			{
				for(i = 0; i < cheatListLength; i++)
					TempDeactivateCheat(&cheatList[i]);
			}
		}
	}

	if(cheatsDisabled)
		return;

	for(i = 0; i < cheatListLength; i++)
		cheat_periodicEntry(machine, &cheatList[i]);
}

/*--------------
  PrintBinary
--------------*/

static UINT32 PrintBinary(char * buf, UINT32 data, UINT32 mask)
{
	UINT32	traverse = 0x80000000;
	UINT32	written = 0;

	while(traverse)
	{
		if(mask & traverse)
		{
			*buf++ = (data & traverse) ? '1' : '0';
			written++;
		}

		traverse >>= 1;
	}

	*buf++ = 0;

	return written;
}

/*-------------
  PrintASCII
-------------*/

static UINT32 PrintASCII(char * buf, UINT32 data, UINT8 size)
{
	switch(size)
	{
		case kSearchSize_16Bit:
			buf[0] = (data >> 8) & 0xFF;
			buf[1] = (data >> 0) & 0xFF;
			buf[2] = 0;

			return 2;

		case kSearchSize_24Bit:
			buf[0] = (data >> 16)& 0xFF;
			buf[1] = (data >> 8) & 0xFF;
			buf[2] = (data >> 0) & 0xFF;
			buf[3] = 0;

			return 3;

		case kSearchSize_32Bit:
			buf[0] = (data >> 24) & 0xFF;
			buf[1] = (data >> 16) & 0xFF;
			buf[2] = (data >>  8) & 0xFF;
			buf[3] = (data >>  0) & 0xFF;
			buf[4] = 0;

			return 4;

		case kSearchSize_8Bit:
		case kSearchSize_1Bit:
		default:
			buf[0] = (data >> 0) & 0xFF;
			buf[1] = 0;

			return 1;

	}

}

/*---------------------------------------------
  cheat_display_watches - display watchpoint
---------------------------------------------*/

void cheat_display_watches(void)
{
	int		i;

	if(watchesDisabled)
		return;

	for(i = 0; i < watchListLength; i++)
	{
		int			j;
		WatchInfo	* info = &watchList[i];
		char		buf[1024];
		UINT32		address = info->address;
		int			xOffset = 0, yOffset = 0;
		int			numChars;
		int			lineElements = 0;

		/* ----- if the length of the watchpoint is not 0, display it ----- */
		if(info->numElements)
		{
			/* ----- label display ----- */
			switch(info->labelType)
			{
				case kWatchLabel_Address:
					numChars = sprintf(buf, "%.8X: ", info->address);

					ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
					xOffset += numChars;
					break;

				case kWatchLabel_String:
					numChars = sprintf(buf, "%s: ", info->label);

					ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
					xOffset += numChars;
					break;
			}

			/* ----- value display ----- */
			for(j = 0; j < info->numElements; j++)
			{
				UINT32	data = 0;

				switch(info->locationType)
				{
					case kLocation_Standard:
						data =	(DoCPURead(info->cpu, address, kSearchByteIncrementTable[info->elementBytes], CPUNeedsSwap(info->cpu)) + info->addValue) &
								kSearchByteMaskTable[info->elementBytes];
						break;

					case kLocation_MemoryRegion:
					{
						int		region = REGION_CPU1 + info->cpu;
						UINT8	* buf = memory_region(region);

						data =	DoMemoryRead(buf, address, kSearchByteIncrementTable[info->elementBytes], RegionNeedsSwap(region) + info->addValue, GetRegionCPUInfo(region)) &
								kSearchByteMaskTable[info->elementBytes];
					}
					break;

					case kLocation_IndirectIndexed:
						return; // under construction...

					case kLocation_ProgramSpace:
					{
						UINT8	* buf;

						memory_set_opbase(address);

						buf = memory_get_op_ptr(info->cpu, address, 0);

						buf -= address;

						if(buf)
							data =	DoMemoryRead(buf, address, kSearchByteIncrementTable[info->elementBytes], CPUNeedsSwap(info->cpu) + info->addValue, GetCPUInfo(info->cpu)) &
									kSearchByteMaskTable[info->elementBytes];
					}
					break;

					default:
						logerror("invalid location type : %x\n", info->locationType);
						return;

				}

				data = DoShift(data, info->dataShift);
				data ^= info->xor;

				if(	(lineElements >= info->elementsPerLine) && info->elementsPerLine)
				{
					lineElements = 0;

					xOffset = 0;
					yOffset++;
				}

				switch(info->displayType)
				{
					case kWatchDisplayType_Hex:
						numChars = sprintf(buf, "%.*X", kSearchByteDigitsTable[info->elementBytes], data);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_Decimal:
						numChars = sprintf(buf, "%.*d", kSearchByteDecDigitsTable[info->elementBytes], data);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_Binary:
						numChars = PrintBinary(buf, data, kSearchByteMaskTable[info->elementBytes]);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_ASCII:
						numChars = PrintASCII(buf, data, info->elementBytes);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						break;
				}

				address += kSearchByteIncrementTable[info->elementBytes] + info->skip;
				lineElements++;
			}
		}
	}
}

/*------------------
  ResizeCheatList
------------------*/

static void ResizeCheatList(UINT32 newLength)
{
	if(newLength != cheatListLength)
	{
		if(newLength < cheatListLength)
		{
			int	i;

			for(i = newLength; i < cheatListLength; i++)
				DisposeCheat(&cheatList[i]);
		}

		cheatList = realloc(cheatList, newLength * sizeof(CheatEntry));

		if(!cheatList && (newLength != 0))
		{
			logerror("ResizeCheatList: out of memory resizing cheat list\n");
			ui_popup_time(2, "out of memory while loading cheat database");

			cheatListLength = 0;

			return;
		}

		if(newLength > cheatListLength)
		{
			int	i;

			memset(&cheatList[cheatListLength], 0, (newLength - cheatListLength) * sizeof(CheatEntry));

			for(i = cheatListLength; i < newLength; i++)
				cheatList[i].flags |= kCheatFlag_Dirty;
		}

		cheatListLength = newLength;
	}
}

/*---------------------------
  ResizeCheatListNoDispose
---------------------------*/

static void ResizeCheatListNoDispose(UINT32 newLength)
{
	if(newLength != cheatListLength)
	{
		cheatList = realloc(cheatList, newLength * sizeof(CheatEntry));

		if(!cheatList && (newLength != 0))
		{
			logerror("ResizeCheatListNoDispose: out of memory resizing cheat list\n");
			ui_popup_time(2, "out of memory while loading cheat database");

			cheatListLength = 0;

			return;
		}

		if(newLength > cheatListLength)
		{
			int	i;

			memset(&cheatList[cheatListLength], 0, (newLength - cheatListLength) * sizeof(CheatEntry));

			for(i = cheatListLength; i < newLength; i++)
				cheatList[i].flags |= kCheatFlag_Dirty;
		}

		cheatListLength = newLength;
	}
}

/*---------------------------------------------------------------
  AddCheatBefore - insert new cheat entry before selected code
---------------------------------------------------------------*/

static void AddCheatBefore(UINT32 idx)
{
	/* ----- memory reallocation ----- */
	ResizeCheatList(cheatListLength + 1);

	if(idx < (cheatListLength - 1))
		memmove(&cheatList[idx + 1], &cheatList[idx], sizeof(CheatEntry) * (cheatListLength - 1 - idx));

	if(idx >= cheatListLength)
		idx = cheatListLength - 1;

	/* ----- insert new entry ----- */
	memset(&cheatList[idx], 0, sizeof(CheatEntry));

	cheatList[idx].flags |= kCheatFlag_Dirty;

	ResizeCheatActionList(&cheatList[idx], 1);
}

/*----------------
  DeleteCheatAt
----------------*/

static void DeleteCheatAt(UINT32 idx)
{
	if(idx >= cheatListLength)
		return;

	DisposeCheat(&cheatList[idx]);

	if(idx < (cheatListLength - 1))
	{
		memmove(&cheatList[idx], &cheatList[idx + 1], sizeof(CheatEntry) * (cheatListLength - 1 - idx));
	}

	ResizeCheatListNoDispose(cheatListLength - 1);
}

/*------------------------------------------------
  DisposeCheat - free memory for selected entry
------------------------------------------------*/

static void DisposeCheat(CheatEntry * entry)
{
	if(entry)
	{
		int	i;

		free(entry->name);
		free(entry->comment);

		for(i = 0; i < entry->actionListLength; i++)
		{
			CheatAction	* action = &entry->actionList[i];

			DisposeAction(action);
		}

		free(entry->actionList);

		memset(entry, 0, sizeof(CheatEntry));
	}
}

/*------------------------------------
  GetNewCheat - get new cheat entry
------------------------------------*/

static CheatEntry *	GetNewCheat(void)
{
	/* ----- insert new cheat entry ----- */
	AddCheatBefore(cheatListLength);

	/* ----- return inserted entry ----- */
	return &cheatList[cheatListLength - 1];
}

/*------------------------
  ResizeCheatActionList
------------------------*/

static void ResizeCheatActionList(CheatEntry * entry, UINT32 newLength)
{
	if(newLength != entry->actionListLength)
	{
		if(newLength < entry->actionListLength)
		{
			int	i;

			for(i = newLength; i < entry->actionListLength; i++)
				DisposeAction(&entry->actionList[i]);
		}

		entry->actionList = realloc(entry->actionList, newLength * sizeof(CheatAction));
		if(!entry->actionList && (newLength != 0))
		{
			logerror("ResizeCheatActionList: out of memory resizing cheat action list\n");
			ui_popup_time(2, "out of memory while loading cheat database");

			entry->actionListLength = 0;

			return;
		}

		if(newLength > entry->actionListLength)
		{
			memset(&entry->actionList[entry->actionListLength], 0, (newLength - entry->actionListLength) * sizeof(CheatAction));
		}

		entry->actionListLength = newLength;
	}
}

/*---------------------------------
  ResizeCheatActionListNoDispose
---------------------------------*/

static void ResizeCheatActionListNoDispose(CheatEntry * entry, UINT32 newLength)
{
	if(newLength != entry->actionListLength)
	{
		entry->actionList = realloc(entry->actionList, newLength * sizeof(CheatAction));
		if(!entry->actionList && (newLength != 0))
		{
			logerror("ResizeCheatActionList: out of memory resizing cheat action list\n");
			ui_popup_time(2, "out of memory while loading cheat database");

			entry->actionListLength = 0;

			return;
		}

		if(newLength > entry->actionListLength)
		{
			memset(&entry->actionList[entry->actionListLength], 0, (newLength - entry->actionListLength) * sizeof(CheatAction));
		}

		entry->actionListLength = newLength;
	}
}

/*------------------
  AddActionBefore
------------------*/

static void AddActionBefore(CheatEntry * entry, UINT32 idx)
{
	ResizeCheatActionList(entry, entry->actionListLength + 1);

	if(idx < (entry->actionListLength - 1))
		memmove(&entry->actionList[idx + 1], &entry->actionList[idx], sizeof(CheatAction) * (entry->actionListLength - 1 - idx));

	if(idx >= entry->actionListLength)
		idx = entry->actionListLength - 1;

	memset(&entry->actionList[idx], 0, sizeof(CheatAction));
}

/*-----------------
  DeleteActionAt
-----------------*/

static void DeleteActionAt(CheatEntry * entry, UINT32 idx)
{
	if(idx >= entry->actionListLength)
		return;

	DisposeAction(&entry->actionList[idx]);

	if(idx < (entry->actionListLength - 1))
		memmove(&entry->actionList[idx], &entry->actionList[idx + 1], sizeof(CheatAction) * (entry->actionListLength - 1 - idx));

	ResizeCheatActionListNoDispose(entry, entry->actionListLength - 1);
}

/*----------------
  DisposeAction
----------------*/

static void DisposeAction(CheatAction * action)
{
	if(action)
	{
		free(action->optionalName);

		memset(action, 0, sizeof(CheatAction));
	}
}

/*----------------------------------------------------
  InitWatch - initialize y-position for watchpoints
----------------------------------------------------*/

static void InitWatch(WatchInfo * info, UINT32 idx)
{
	if(idx > 0)
		/* ----- set 2nd or later y-position ----- */
		info->y = watchList[idx - 1].y + ui_get_line_height();
	else
		/* ----- set 1st y-position (always Y=0) ----- */
		info->y = 0;
}

/*------------------
  ResizeWatchList
------------------*/

static void ResizeWatchList(UINT32 newLength)
{
	if(newLength != watchListLength)
	{
		if(newLength < watchListLength)
		{
			int	i;

			for(i = newLength; i < watchListLength; i++)
				DisposeWatch(&watchList[i]);
		}

		watchList = realloc(watchList, newLength * sizeof(WatchInfo));
		if(!watchList && (newLength != 0))
		{
			logerror("ResizeWatchList: out of memory resizing watch list\n");
			ui_popup_time(2, "out of memory while adding watch");

			watchListLength = 0;

			return;
		}

		if(newLength > watchListLength)
		{
			int	i;

			memset(&watchList[watchListLength], 0, (newLength - watchListLength) * sizeof(WatchInfo));

			for(i = watchListLength; i < newLength; i++)
				InitWatch(&watchList[i], i);
		}

		watchListLength = newLength;
	}
}

/*---------------------------
  ResizeWatchListNoDispose
---------------------------*/

static void ResizeWatchListNoDispose(UINT32 newLength)
{
	if(newLength != watchListLength)
	{
		watchList = realloc(watchList, newLength * sizeof(WatchInfo));
		if(!watchList && (newLength != 0))
		{
			logerror("ResizeWatchList: out of memory resizing watch list\n");
			ui_popup_time(2, "out of memory while adding watch");

			watchListLength = 0;

			return;
		}

		if(newLength > watchListLength)
		{
			int	i;

			memset(&watchList[watchListLength], 0, (newLength - watchListLength) * sizeof(WatchInfo));

			for(i = watchListLength; i < newLength; i++)
				InitWatch(&watchList[i], i);
		}

		watchListLength = newLength;
	}
}

/*-----------------
  AddWatchBefore
-----------------*/

static void AddWatchBefore(UINT32 idx)
{
	ResizeWatchList(watchListLength + 1);

	if(idx < (watchListLength - 1))
		memmove(&watchList[idx + 1], &watchList[idx], sizeof(WatchInfo) * (watchListLength - 1 - idx));

	if(idx >= watchListLength)
		idx = watchListLength - 1;

	memset(&watchList[idx], 0, sizeof(WatchInfo));

	InitWatch(&watchList[idx], idx);
}

/*----------------
  DeleteWatchAt
----------------*/

static void DeleteWatchAt(UINT32 idx)
{
	if(idx >= watchListLength)
		return;

	DisposeWatch(&watchList[idx]);

	if(idx < (watchListLength - 1))
		memmove(&watchList[idx], &watchList[idx + 1], sizeof(WatchInfo) * (watchListLength - 1 - idx));

	ResizeWatchListNoDispose(watchListLength - 1);
}

/*---------------
  DisposeWatch
---------------*/

static void DisposeWatch(WatchInfo * watch)
{
	if(watch)
		memset(watch, 0, sizeof(WatchInfo));
}

/*----------------------------------------------------------
  GetUnusedWatch - search unused watchpoint and return it
----------------------------------------------------------*/

static WatchInfo * GetUnusedWatch(void)
{
	int			i;
	WatchInfo	* info;
	WatchInfo	* theWatch = NULL;

	/* ----- search unused watchpoint ----- */
	for(i = 0; i < watchListLength; i++)
	{
		info = &watchList[i];

		if(info->numElements == 0)
		{
			theWatch = info;
			break;
		}
	}

	/* ----- if unused watchpoint is not found, insert new watchpoint as unused ----- */
	if(!theWatch)
	{
		AddWatchBefore(watchListLength);

		theWatch = &watchList[watchListLength - 1];
	}

	/* ----- return unused watchpoint ----- */
	return theWatch;
}

/*----------------------------------------------------------------------------
  AddCheatFromWatch - add new cheat code from watchpoint list to cheat list
----------------------------------------------------------------------------*/

static void AddCheatFromWatch(WatchInfo * watch)
{
	if(watch)
	{
		CheatEntry	* entry = GetNewCheat();
		CheatAction	* action = &entry->actionList[0];
		char		tempString[1024];
		int			tempStringLength;
		UINT32		data = DoCPURead(watch->cpu, watch->address, kSearchByteIncrementTable[watch->elementBytes], 0);

		tempStringLength = sprintf(tempString, "%.8X (%d) = %.*X", watch->address, watch->cpu, kSearchByteDigitsTable[watch->elementBytes], data);

		entry->name = realloc(entry->name, tempStringLength + 1);
		memcpy(entry->name, tempString, tempStringLength + 1);

		SET_FIELD(action->type, LocationParameter, watch->cpu);
		SET_FIELD(action->type, BytesUsed, kSearchByteIncrementTable[watch->elementBytes] - 1);
		action->address = watch->address;
		action->data = data;
		action->extendData = 0xFFFFFFFF;
		action->originalDataField = data;

		UpdateCheatInfo(entry, 0);
	}
}

/*-------------------------------------------------------------------------------------
  SetupCheatFromWatchAsWatch - add new watch code from watchpoint list to cheat list
-------------------------------------------------------------------------------------*/

static void SetupCheatFromWatchAsWatch(CheatEntry * entry, WatchInfo * watch)
{
	if(watch && entry && watch->numElements)
	{
		CheatAction	* action;
		char		tempString[1024];
		size_t		tempStringLength;

		DisposeCheat(entry);
		ResizeCheatActionList(entry, 1);

		action = &entry->actionList[0];

		tempStringLength = sprintf(tempString, "Watch %.8X (%d)", watch->address, watch->cpu);

		entry->name = realloc(entry->name, tempStringLength + 1);
		memcpy(entry->name, tempString, tempStringLength + 1);

		action->type = 0;
		SET_FIELD(action->type, LocationParameter, watch->cpu);
		SET_FIELD(action->type, Type, kType_Watch);
		SET_FIELD(action->type, BytesUsed, kSearchByteIncrementTable[watch->elementBytes] - 1);
		SET_FIELD(action->type, TypeParameter, watch->displayType | ((watch->labelType == kWatchLabel_String) ? 0x04 : 0));

		action->address = watch->address;
		action->data  =	((watch->numElements - 1) & 0xFF) |
						((watch->skip & 0xFF) << 8) |
						((watch->elementsPerLine & 0xFF) << 16) |
						((watch->addValue & 0xFF) << 24);
		action->originalDataField = action->data;
		action->extendData = 0xFFFFFFFF;

		tempStringLength = strlen(watch->label);
		entry->comment = realloc(entry->comment, tempStringLength + 1);
		memcpy(entry->comment, watch->label, tempStringLength + 1);

		UpdateCheatInfo(entry, 0);
	}
}

/*-------------------
  ResizeSearchList
-------------------*/

static void ResizeSearchList(UINT32 newLength)
{
	if(newLength != searchListLength)
	{
		if(newLength < searchListLength)
		{
			int	i;

			for(i = newLength; i < searchListLength; i++)
				DisposeSearch(i);
		}

		searchList = realloc(searchList, newLength * sizeof(SearchInfo));
		if(!searchList && (newLength != 0))
		{
			logerror("ResizeSearchList: out of memory resizing search list\n");
			ui_popup_time(2, "out of memory while adding search");

			searchListLength = 0;

			return;
		}

		if(newLength > searchListLength)
		{
			int	i;

			memset(&searchList[searchListLength], 0, (newLength - searchListLength) * sizeof(SearchInfo));

			for(i = searchListLength; i < newLength; i++)
			{
				InitSearch(&searchList[i]);
			}
		}

		searchListLength = newLength;
	}
}

/*----------------------------
  ResizeSearchListNoDispose
----------------------------*/

static void ResizeSearchListNoDispose(UINT32 newLength)
{
	if(newLength != searchListLength)
	{
		searchList = realloc(searchList, newLength * sizeof(SearchInfo));
		if(!searchList && (newLength != 0))
		{
			logerror("ResizeSearchList: out of memory resizing search list\n");
			ui_popup_time(2, "out of memory while adding search");

			searchListLength = 0;

			return;
		}

		if(newLength > searchListLength)
		{
			memset(&searchList[searchListLength], 0, (newLength - searchListLength) * sizeof(SearchInfo));
		}

		searchListLength = newLength;
	}
}

/*------------------
  AddSearchBefore
------------------*/

static void AddSearchBefore(UINT32 idx)
{
	ResizeSearchListNoDispose(searchListLength + 1);

	if(idx < (searchListLength - 1))
		memmove(&searchList[idx + 1], &searchList[idx], sizeof(SearchInfo) * (searchListLength - 1 - idx));

	if(idx >= searchListLength)
		idx = searchListLength - 1;

	memset(&searchList[idx], 0, sizeof(SearchInfo));
	InitSearch(&searchList[idx]);
}

/*-----------------
  DeleteSearchAt
-----------------*/

static void DeleteSearchAt(UINT32 idx)
{
	if(idx >= searchListLength)
		return;

	DisposeSearch(idx);

	if(idx < (searchListLength - 1))
	{
		memmove(&searchList[idx], &searchList[idx + 1], sizeof(SearchInfo) * (searchListLength - 1 - idx));
	}

	ResizeSearchListNoDispose(searchListLength - 1);
}

/*--------------------------------------
  InitSearch - initialize search info
--------------------------------------*/

static void InitSearch(SearchInfo * info)
{
	if(info)
	{
		info->searchSpeed = kSearchSpeed_Medium;
	}
}

/*-----------------------
  DisposeSearchRegions
-----------------------*/

static void DisposeSearchRegions(SearchInfo * info)
{
	if(info->regionList)
	{
		int	i;

		for(i = 0; i < info->regionListLength; i++)
		{
			SearchRegion	* region = &info->regionList[i];

			free(region->first);
			free(region->last);
			free(region->status);
			free(region->backupLast);
			free(region->backupStatus);
		}

		free(info->regionList);

		info->regionList = NULL;
	}

	info->regionListLength = 0;
}

/*------------------
  DisposeSearch
  ----------------*/

static void DisposeSearch(UINT32 idx)
{
	SearchInfo	* info;

	if(idx >= searchListLength)
		return;

	info = &searchList[idx];

	DisposeSearchRegions(info);

	free(info->name);
	info->name = NULL;
}

/*----------------------------------------
  GetCurrentSearch - return search info
----------------------------------------*/

static SearchInfo *	GetCurrentSearch(void)
{
	if(currentSearchIdx >= searchListLength)
		currentSearchIdx = searchListLength - 1;
	if(currentSearchIdx < 0)
		currentSearchIdx = 0;

	return &searchList[currentSearchIdx];
}

/*-------------------------------------------------------
  FillBufferFromRegion - set a data into search region
-------------------------------------------------------*/

static void FillBufferFromRegion(SearchRegion * region, UINT8 * buf)
{
	UINT32	offset;

	/* ### optimize if needed ### */
	for(offset = 0; offset < region->length; offset++)
		buf[offset] = ReadRegionData(region, offset, 1, 0);
}

/*-------------------------------------------------------
  ReadRegionData - read a data from region when search
-------------------------------------------------------*/

static UINT32 ReadRegionData(SearchRegion * region, UINT32 offset, UINT8 size, UINT8 swap)
{
	UINT32	address = region->address + offset;

	switch(region->targetType)
	{
		case kRegionType_CPU:
			return DoCPURead(region->targetIdx, address, size, CPUNeedsSwap(region->targetIdx) ^ swap);

		case kRegionType_Memory:
			if(region->cachedPointer)
				return DoMemoryRead(region->cachedPointer, address, size, swap, &rawCPUInfo);
			else
				return 0;
	}

	return 0;
}
/*-----------------------------------------------
  BackupSearch - back up current search region
-----------------------------------------------*/

static void BackupSearch(SearchInfo * info)
{
	int	i;

	for(i = 0; i < info->regionListLength; i++)
		BackupRegion(&info->regionList[i]);

	info->oldNumResults = info->numResults;
	info->backupValid = 1;
}

/*-------------------------------------------------------
  RestoreSearchBackup - restore previous search region
-------------------------------------------------------*/

static void RestoreSearchBackup(SearchInfo * info)
{
	int i;

	if(info && info->backupValid)
	{
		for(i = 0; i < info->regionListLength; i++)
			RestoreRegionBackup(&info->regionList[i]);

		info->numResults = info->oldNumResults;
		info->backupValid = 0;

		ui_popup_time(1, "values restored");
	}
	else
		ui_popup_time(1, "there are no old values");
}

/*------------------------------------------------
  BackupRegion - back up current search results
------------------------------------------------*/

static void BackupRegion(SearchRegion * region)
{
	if(region->flags & kRegionFlag_Enabled)
	{
		memcpy(region->backupLast,		region->last,	region->length);
		memcpy(region->backupStatus,	region->status,	region->length);
		region->oldNumResults =			region->numResults;
	}
}

/*--------------------------------------------------------
  RestoreSearchBackup - restore previous search results
--------------------------------------------------------*/

static void RestoreRegionBackup(SearchRegion * region)
{
	if(region->flags & kRegionFlag_Enabled)
	{
		memcpy(region->last,	region->backupLast,		region->length);
		memcpy(region->status,	region->backupStatus,	region->length);
		region->numResults =	region->oldNumResults;
	}
}

/*-----------------------------------------------------------
  DefaultEnableRegion - get default regions you can search
-----------------------------------------------------------*/

static UINT8 DefaultEnableRegion(running_machine *machine, SearchRegion * region, SearchInfo * info)
{
	write8_machine_func	handler = region->writeHandler->write.mhandler8;
	FPTR				handlerAddress = (FPTR)handler;

	switch(info->searchSpeed)
	{
		case kSearchSpeed_Fast:

#if HAS_SH2
			if(machine->config->cpu[0].type == CPU_SH2)
			{
				if(	(info->targetType == kRegionType_CPU) && (info->targetIdx == 0) && (region->address == 0x06000000))
					return 1;

				return 0;
			}
#endif

			if(	(handler == SMH_RAM) && (!region->writeHandler->baseptr))
				return 1;

#ifndef MESS

			{
				/* ----- for neogeo, search bank one ----- */
				if(	(!strcmp(machine->gamedrv->parent, "neogeo")) && (info->targetType == kRegionType_CPU) &&
					(info->targetIdx == 0) && (handler == SMH_BANK1))
					return 1;
			}

#endif

#if HAS_TMS34010

			/* ----- for exterminator, search bank one ----- */
			if(	(machine->config->cpu[1].type == CPU_TMS34010) && (info->targetType == kRegionType_CPU) &&
				(info->targetIdx == 1) && (handler == SMH_BANK1))
				return 1;

			/* ----- for smashtv, search bank two ----- */
			if(	(machine->config->cpu[0].type == CPU_TMS34010) && (info->targetType == kRegionType_CPU) &&
				(info->targetIdx == 0) && (handler == SMH_BANK2))
				return 1;

#endif
			return 0;

		case kSearchSpeed_Medium:
			if(	(handlerAddress >= ((FPTR)SMH_BANK1)) && (handlerAddress <= ((FPTR)SMH_BANK24)))
				return 1;

			if(handler == SMH_RAM)
				return 1;

			return 0;

		case kSearchSpeed_Slow:
			if(	(handler == SMH_NOP) || (handler == SMH_ROM))
				return 0;

			if(	(handlerAddress > STATIC_COUNT) && (!region->writeHandler->baseptr))
				return 0;

			return 1;

		case kSearchSpeed_VerySlow:
			if(	(handler == SMH_NOP) || (handler == SMH_ROM))
				return 0;

			return 1;
	}

	return 0;
}

/*-----------------------------
  SetSearchRegionDefaultName
-----------------------------*/

static void SetSearchRegionDefaultName(SearchRegion * region)
{
	switch(region->targetType)
	{
		case kRegionType_CPU:
		{
			char	desc[16];

			if(region->writeHandler)
			{
				genf *				handler = region->writeHandler->write.generic;
				FPTR				handlerAddress = (FPTR)handler;

				if(	(handlerAddress >= ((FPTR)SMH_BANK1)) && (handlerAddress <= ((FPTR)SMH_BANK24)))
					sprintf(desc, "BANK%.2d", (int)(handlerAddress - (FPTR)SMH_BANK1) + 1);
				else
				{
					switch(handlerAddress)
					{
						case (FPTR)SMH_NOP:		strcpy(desc, "NOP   ");	break;
						case (FPTR)SMH_RAM:		strcpy(desc, "RAM   ");	break;
						case (FPTR)SMH_ROM:		strcpy(desc, "ROM   ");	break;
						default:					strcpy(desc, "CUSTOM");	break;
					}
				}
			}
			else
				sprintf(desc, "CPU%.2d ", region->targetIdx);

			sprintf(region->name,	"%.*X-%.*X %s",
									cpuInfoList[region->targetIdx].addressCharsNeeded,
									region->address,
									cpuInfoList[region->targetIdx].addressCharsNeeded,
									region->address + region->length - 1,
									desc);
		}
		break;

		case kRegionType_Memory:
			sprintf(region->name, "%.8X-%.8X MEMORY", region->address, region->address + region->length - 1);
			break;

		default:
			sprintf(region->name, "UNKNOWN");
			break;
	}
}

/*-----------------------------------------------------------------------------------
  AllocateSearchRegions - free memory for search region then realloc if searchable
-----------------------------------------------------------------------------------*/

static void AllocateSearchRegions(SearchInfo * info)
{
	int	i;

	info->backupValid = 0;
	info->numResults = 0;

	for(i = 0; i < info->regionListLength; i++)
	{
		SearchRegion	* region;

		region = &info->regionList[i];

		region->numResults = 0;

		free(region->first);
		free(region->last);
		free(region->status);
		free(region->backupLast);
		free(region->backupStatus);

		if(region->flags & kRegionFlag_Enabled)
		{
			region->first =			malloc(region->length);
			region->last =			malloc(region->length);
			region->status =		malloc(region->length);
			region->backupLast =	malloc(region->length);
			region->backupStatus =	malloc(region->length);

			if(!region->first || !region->last || !region->status || !region->backupLast || !region->backupStatus)
			{
				free(region->first);
				free(region->last);
				free(region->status);
				free(region->backupLast);
				free(region->backupStatus);

				region->first =			NULL;
				region->last =			NULL;
				region->status =		NULL;
				region->backupLast =	NULL;
				region->backupStatus =	NULL;

				region->flags &= ~kRegionFlag_Enabled;
			}
		}
		else
		{
			region->first =			NULL;
			region->last =			NULL;
			region->status =		NULL;
			region->backupLast =	NULL;
			region->backupStatus =	NULL;
		}
	}
}

/*---------------------
  BuildSearchRegions
---------------------*/

static void BuildSearchRegions(running_machine *machine, SearchInfo * info)
{
	info->comparison = kSearchComparison_EqualTo;

	DisposeSearchRegions(info);

	switch(info->targetType)
	{
		case kRegionType_CPU:
		{
			if(info->searchSpeed == kSearchSpeed_AllMemory)
			{
				UINT32			length = cpuInfoList[info->targetIdx].addressMask + 1;
				SearchRegion	* region;

				info->regionList = calloc(sizeof(SearchRegion), 1);
				info->regionListLength = 1;
				region = info->regionList;

				region->address = 0;
				region->length = length;

				region->targetIdx = info->targetIdx;
				region->targetType = info->targetType;
				region->writeHandler = NULL;

				region->first = NULL;
				region->last = NULL;
				region->status = NULL;

				region->backupLast = NULL;
				region->backupStatus = NULL;

				region->flags = kRegionFlag_Enabled;

				SetSearchRegionDefaultName(region);
			}
			else
			{
				if(info->targetIdx < cpu_gettotalcpu())
				{
					const address_map			* map = NULL;
					const address_map_entry			* entry;
					SearchRegion						* traverse;
					int									count = 0;

					map = memory_get_address_map(info->targetIdx, ADDRESS_SPACE_PROGRAM);
					for (entry = map->entrylist; entry != NULL; entry = entry->next)
						if (entry->write.generic)
							count++;

					info->regionList = calloc(sizeof(SearchRegion), count);
					info->regionListLength = count;
					traverse = info->regionList;

					for (entry = map->entrylist; entry != NULL; entry = entry->next)
					{
						if (entry->write.generic)
						{
							UINT32	length = (entry->addrend - entry->addrstart) + 1;

							traverse->address = entry->addrstart;
							traverse->length = length;

							traverse->targetIdx = info->targetIdx;
							traverse->targetType = info->targetType;
							traverse->writeHandler = entry;

							traverse->first = NULL;
							traverse->last = NULL;
							traverse->status = NULL;

							traverse->backupLast = NULL;
							traverse->backupStatus = NULL;

							traverse->flags = DefaultEnableRegion(machine, traverse, info) ? kRegionFlag_Enabled : 0;

							SetSearchRegionDefaultName(traverse);

							traverse++;
						}
					}
				}
			}
		}
		break;

		case kRegionType_Memory:
			break;
	}
}

/*---------------------------------------------------------------------
  ConvertOldCode - convert old format code to new when load database
---------------------------------------------------------------------*/

static int ConvertOldCode(int code, int cpu, int * data, int * extendData)
{
	enum
	{
		kCustomField_None =					0,
		kCustomField_DontApplyCPUField =	1 << 0,
		kCustomField_SetBit =				1 << 1,
		kCustomField_ClearBit =				1 << 2,
		kCustomField_SubtractOne =			1 << 3,

		kCustomField_BitMask =				kCustomField_SetBit |
											kCustomField_ClearBit,
		kCustomField_End =					0xFF
	};

	struct ConversionTable
	{
		int		oldCode;
		UINT32	newCode;
		UINT8	customField;
	};

	static const struct ConversionTable kConversionTable[] =
	{
		{	0,		0x00000000,	kCustomField_None },
		{	1,		0x00000001,	kCustomField_None },
		{	2,		0x00000020,	kCustomField_None },
		{	3,		0x00000040,	kCustomField_None },
		{	4,		0x000000A0,	kCustomField_None },
		{	5,		0x00000022,	kCustomField_None },
		{	6,		0x00000042,	kCustomField_None },
		{	7,		0x000000A2,	kCustomField_None },
		{	8,		0x00000024,	kCustomField_None },
		{	9,		0x00000044,	kCustomField_None },
		{	10,		0x00000064,	kCustomField_None },
		{	11,		0x00000084,	kCustomField_None },
		{	15,		0x00000023,	kCustomField_None },
		{	16,		0x00000043,	kCustomField_None },
		{	17,		0x000000A3,	kCustomField_None },
		{	20,		0x00000000,	kCustomField_SetBit },
		{	21,		0x00000001,	kCustomField_SetBit },
		{	22,		0x00000020,	kCustomField_SetBit },
		{	23,		0x00000040,	kCustomField_SetBit },
		{	24,		0x000000A0,	kCustomField_SetBit },
		{	40,		0x00000000,	kCustomField_ClearBit },
		{	41,		0x00000001,	kCustomField_ClearBit },
		{	42,		0x00000020,	kCustomField_ClearBit },
		{	43,		0x00000040,	kCustomField_ClearBit },
		{	44,		0x000000A0,	kCustomField_ClearBit },
		{	60,		0x00000103,	kCustomField_None },
		{	61,		0x00000303,	kCustomField_None },
		{	62,		0x00000503,	kCustomField_SubtractOne },
		{	63,		0x00000903,	kCustomField_None },
		{	64,		0x00000B03,	kCustomField_None },
		{	65,		0x00000D03,	kCustomField_SubtractOne },
		{	70,		0x00000101,	kCustomField_None },
		{	71,		0x00000301,	kCustomField_None },
		{	72,		0x00000501,	kCustomField_SubtractOne },
		{	73,		0x00000901,	kCustomField_None },
		{	74,		0x00000B01,	kCustomField_None },
		{	75,		0x00000D01,	kCustomField_SubtractOne },
		{	80,		0x00000102,	kCustomField_None },
		{	81,		0x00000302,	kCustomField_None },
		{	82,		0x00000502,	kCustomField_SubtractOne },
		{	83,		0x00000902,	kCustomField_None },
		{	84,		0x00000B02,	kCustomField_None },
		{	85,		0x00000D02,	kCustomField_SubtractOne },
		{	90,		0x00000100,	kCustomField_None },
		{	91,		0x00000300,	kCustomField_None },
		{	92,		0x00000500,	kCustomField_SubtractOne },
		{	93,		0x00000900,	kCustomField_None },
		{	94,		0x00000B00,	kCustomField_None },
		{	95,		0x00000D00,	kCustomField_SubtractOne },
		{	100,	0x20800000,	kCustomField_None },
		{	101,	0x20000001,	kCustomField_None },
		{	102,	0x20800000,	kCustomField_None },
		{	103,	0x20000001,	kCustomField_None },
		{	110,	0x40800000,	kCustomField_None },
		{	111,	0x40000001,	kCustomField_None },
		{	112,	0x40800000,	kCustomField_None },
		{	113,	0x40000001,	kCustomField_None },
		{	120,	0x63000001,	kCustomField_None },
		{	121,	0x63000001,	kCustomField_DontApplyCPUField | kCustomField_SetBit },
		{	122,	0x63000001,	kCustomField_DontApplyCPUField | kCustomField_ClearBit },
		{	998,	0x00000006,	kCustomField_None },
		{	999,	0x60000000,	kCustomField_DontApplyCPUField },
		{	-1,		0x00000000,	kCustomField_End }
	};

	const struct ConversionTable	* traverse = kConversionTable;
	UINT32					newCode;
	UINT8					linkCheat = 0;

	/* ----- convert link cheats ----- */
	if((code >= 500) && (code <= 699))
	{
		linkCheat = 1;
		code -= 500;
	}

	/* ----- look up code ----- */
	while(traverse->oldCode >= 0)
	{
		if(code == traverse->oldCode)
			goto foundCode;
		traverse++;
	}

	logerror("ConvertOldCode: %d not found\n", code);

	/* ----- not found ----- */
	*extendData = 0;
	return 0;

	/* ----- found ----- */
	foundCode:

	newCode = traverse->newCode;

	/* ----- add in the CPU field ----- */
	if(!(traverse->customField & kCustomField_DontApplyCPUField))
		newCode = (newCode & ~0x1F000000) | ((cpu << 24) & 0x1F000000);

	/* ----- hack-ish, subtract one from data field for x5 user select ----- */
	if(traverse->customField & kCustomField_SubtractOne)
		(*data)--;	// yaay for C operator precedence

	/* ----- set up the extend data ----- */
	if(traverse->customField & kCustomField_BitMask)
		*extendData = *data;
	else
		*extendData = 0xFFFFFFFF;

	if(traverse->customField & kCustomField_ClearBit)
		*data = 0;

	if(linkCheat)
		SET_MASK_FIELD(newCode, LinkEnable);

	return newCode;
}

/*--------------------------------------------------------------------------------
  HandleLocalCommandCheat - get special code which is not added into cheat list
--------------------------------------------------------------------------------*/

static void HandleLocalCommandCheat(running_machine *machine, UINT32 type, UINT32 address, UINT32 data, UINT32 extendData, char * name, char * description)
{
	switch(EXTRACT_FIELD(type, LocationType))
	{
		case kLocation_Custom:
			switch(EXTRACT_FIELD(type, LocationParameter))
			{
				/* ----- activation key ----- */
				case kCustomLocation_AssignActivationKey:
				{
					if(address < cheatListLength)
					{
						CheatEntry	* entry = &cheatList[address];

						if(!TEST_FIELD(type, OneShot))
						{
							/* ----- 1st activation key ----- */
							entry->activationKey1 = data;
							entry->flags |= kCheatFlag_HasActivationKey1;
						}
						else
						{
							/* ----- 2nd activation key ----- */
							entry->activationKey2 = data;
							entry->flags |= kCheatFlag_HasActivationKey2;
						}
					}
				}
				break;

				/* ----- pre-enable ----- */
				case kCustomLocation_Enable:
				{
					if(address < cheatListLength)
					{
						CheatEntry	* entry = &cheatList[address];

						ActivateCheat(entry);
					}
				}
				break;

				/* ----- over-clock ----- */
				case kCustomLocation_Overclock:
				{
					if(address < cpu_gettotalcpu())
					{
						double	overclock = data;

						overclock /= 65536.0;

						cpunum_set_clockscale(machine, address, overclock);
					}
				}
				break;

				/* ----- refresh rate ----- */
				case kCustomLocation_RefreshRate:
				{
					int width  = video_screen_get_width(machine->primary_screen);
					int height = video_screen_get_height(machine->primary_screen);
					const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);
					double refresh = data / 65536.0;

					video_screen_configure(machine->primary_screen, width, height, visarea, HZ_TO_ATTOSECONDS(refresh));
				}
				break;
			}
			break;
	}
}

/*------------------------------------------------
  LoadCheatFile - load cheat code from database
------------------------------------------------*/

static void LoadCheatFile(running_machine *machine, char * fileName)
{
	mame_file	* theFile;
	file_error filerr;
	char		formatString[256];
	char		oldFormatString[256];
	char		buf[2048];
	int			recordNames = 0;

	/* ----- open the database ----- */
	filerr = mame_fopen(SEARCHPATH_CHEAT, fileName, OPEN_FLAG_READ, &theFile);

	if(filerr != FILERR_NONE)
		return;

	foundCheatDatabase = 1;

	/* ----- make the format strings ----- */
#ifdef MESS
	sprintf(formatString, ":%s:%s", machine->gamedrv->name, "%x:%x:%x:%x:%x:%[^:\n\r]:%[^:\n\r]");
	sprintf(oldFormatString, "%s:%s", machine->gamedrv->name, "%x:%d:%x:%x:%d:%[^:\n\r]:%[^:\n\r]");
#else
	sprintf(formatString, ":%s:%s", machine->gamedrv->name, "%x:%x:%x:%x:%[^:\n\r]:%[^:\n\r]");
	sprintf(oldFormatString, "%s:%s", machine->gamedrv->name, "%d:%x:%x:%d:%[^:\n\r]:%[^:\n\r]");
#endif

	/* ----- get a line from database ----- */
	while(mame_fgets(buf, 2048, theFile))
	{
		int			type;
		int			address;
		int			data;
		int			extendData;
		char		name[256];
		char		description[256];
#ifdef MESS
		int			crc;
#endif

		int			argumentsMatched;
		UINT32		command;

		CheatEntry	* entry;
		CheatAction	* action;

		/* ----- get command line ----- */
		argumentsMatched = sscanf(buf, ":_command:%8X", &command);

		if(argumentsMatched == 1)
		{
			/* ----- set cheatOptions from command line ----- */
			cheatOptions = command;

			continue;
		}

		/* ----- get cheat code ----- */
		name[0] = 0;
		description[0] = 0;

#ifdef MESS
		argumentsMatched = sscanf(buf, formatString, &crc, &type, &address, &data, &extendData, name, description);
#else
		argumentsMatched = sscanf(buf, formatString, &type, &address, &data, &extendData, name, description);
#endif

#ifdef MESS
		if(argumentsMatched < 5)
#else
		if(argumentsMatched < 4)
#endif
		{
			int	oldCPU;
			int	oldCode;

#ifdef MESS
			argumentsMatched = sscanf(buf, oldFormatString, &crc, &oldCPU, &address, &data, &oldCode, name, description);

			if(argumentsMatched < 5)
#else
			argumentsMatched = sscanf(buf, oldFormatString, &oldCPU, &address, &data, &oldCode, name, description);

			if(argumentsMatched < 4)
#endif
				/* ----- if it is not a cheat code, continue ----- */
				continue;
			else
			{
#ifdef MESS
				if(!MatchesCRCTable(crc))
					continue;
#endif
				/* ----- convert the old code to the new format ----- */
				if(TEST_FIELD(cheatOptions, LoadOldFormat))
					type = ConvertOldCode(oldCode, oldCPU, &data, &extendData);
//                  continue;
			}
		}
		else
		{
#ifdef MESS
			if(!MatchesCRCTable(crc))
				continue;
#endif
		}

		//logerror("cheat: processing %s\n", buf);

		if(TEST_FIELD(type, RemoveFromList))
		{
			//logerror("cheat: cheat line removed\n", buf);

			HandleLocalCommandCheat(machine, type, address, data, extendData, name, description);
		}
		else
		{
			if(TEST_FIELD(type, LinkEnable))
			{
				/* ----- if 1st code is the link code, it is invalid ----- */
				if(cheatListLength == 0)
				{
					logerror("LoadCheatFile: first cheat found was link cheat; bailing\n");

					goto bail;
				}

				//logerror("cheat: doing link cheat\n");

				entry = &cheatList[cheatListLength - 1];
			}
			else
			{
				/* ----- go to the next cheat entry ----- */
				ResizeCheatList(cheatListLength + 1);

				//logerror("cheat: doing normal cheat\n");

				if(cheatListLength == 0)
				{
					logerror("LoadCheatFile: cheat list resize failed; bailing\n");

					goto bail;
				}

				entry = &cheatList[cheatListLength - 1];

				/* ----- copy the code name ----- */
				entry->name = CreateStringCopy(name);

				/* ----- copy the description if we got it ----- */
				if(argumentsMatched == 6)
					entry->comment = CreateStringCopy(description);

				recordNames = 0;

				/* ----- try to copy the label if label-selection ------ */
				if(	(EXTRACT_FIELD(type, LocationType) == kLocation_Custom) &&
					(EXTRACT_FIELD(type, LocationParameter) == kCustomLocation_Select))
						recordNames = 1;
			}

			ResizeCheatActionList(&cheatList[cheatListLength - 1], entry->actionListLength + 1);

			if(entry->actionListLength == 0)
			{
				logerror("LoadCheatFile: action list resize failed; bailing\n");

				goto bail;
			}

			action = &entry->actionList[entry->actionListLength - 1];

			action->type = type;
			action->address = address;
			action->data = data;
			action->originalDataField = data;
			action->extendData = extendData;

			/* ----- copy the label if label-selection ------ */
			if(recordNames)
				action->optionalName = CreateStringCopy(name);
		}
	}

	bail:

	/* ----- close the database ----- */
	mame_fclose(theFile);
}

/*---------------------------------------------------------
  LoadCheatDatabase - get the database name then load it
---------------------------------------------------------*/

static void LoadCheatDatabase(running_machine *machine)
{
	char		buf[4096];
	const char	* inTraverse;
	char		* outTraverse;
	char		* mainTraverse;
	int			first = 1;
	char		data;

	cheatfile = options_get_string(mame_options(), OPTION_CHEAT_FILE);

	/* ----- set default database name as "cheat.dat" ----- */
	if (cheatfile[0] == 0)
		cheatfile = "cheat.dat";

	inTraverse = cheatfile;
	outTraverse = buf;
	mainTraverse = mainDatabaseName;

	buf[0] = 0;

	/* ----- get database name ----- */
	do
	{
		data = *inTraverse;

		/* ----- check separator or line end ----- */
		if((data == ';') || (data == 0))
		{
			*outTraverse++ = 0;

			if(first)
				*mainTraverse++ = 0;

			if(buf[0])
			{
				/* ----- load database based on the name we gotten ----- */
				LoadCheatFile(machine, buf);

				outTraverse =buf;
				buf[0] = 0;
				first = 0;
			}
		}
		else
		{
			*outTraverse++ = data;

			if(first)
				*mainTraverse++ = data;
		}

		inTraverse++;
	}
	while(data);

	UpdateAllCheatInfo();
}

/*-----------------------------------------------------------------------------------------
  DisposeCheatDatabase - deactivate all cheats when reload database or exit cheat system
-----------------------------------------------------------------------------------------*/

static void DisposeCheatDatabase(void)
{
	int	i;

	/* ----- first, turn all cheats "OFF" ----- */
	for(i = 0; i < cheatListLength; i++)
		TempDeactivateCheat(&cheatList[i]);

	/* ----- next, free memory for all cheat entries ----- */
	if(cheatList)
	{
		for(i = 0; i < cheatListLength; i++)
			DisposeCheat(&cheatList[i]);

		free(cheatList);

		cheatList = NULL;
		cheatListLength = 0;
	}
}

/*-------------------------------------------------------------------------
  ReloadCheatDatabase - reload cheat database directly on the cheat menu
-------------------------------------------------------------------------*/

static void ReloadCheatDatabase(running_machine *machine)
{
	DisposeCheatDatabase();
	LoadCheatDatabase(machine);

	ui_popup_time(1, "Cheat Database reloaded");
}

/*---------------------------------------------------------------
  SaveCheat - save a code (normal code, activation key, option)
---------------------------------------------------------------*/

static void SaveCheat(running_machine *machine, CheatEntry * entry, int selection, int saveCode)
{
	enum{
			normalCode = 0,
			activationKey,
			command,
		};

	mame_file * theFile;
	file_error filerr;
	UINT32	i;
	char	buf[4096];

	/* ----- check selected entry ----- */
	if((saveCode == normalCode) || (saveCode == activationKey))
	{
		if(!entry || !entry->actionList)
			return;

		if((saveCode == activationKey) && (entry->flags & kCheatFlag_Select) && !entry->actionListLength)
			return;
	}

	/* ----- open the database ----- */
	filerr = mame_fopen(SEARCHPATH_CHEAT, mainDatabaseName, OPEN_FLAG_WRITE, &theFile);

	/* ----- if fails to create, no save ----- */
	if(!theFile)
		return;
	if (filerr != FILERR_NONE)
	{
		filerr = mame_fopen(SEARCHPATH_CHEAT, mainDatabaseName, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &theFile);

		/* ----- if fails to create, no save ----- */
		if(filerr != FILERR_NONE)
			return;
	}

	mame_fseek(theFile, 0, SEEK_END);

	switch(saveCode)
	{
		case normalCode:
			for(i = 0; i < entry->actionListLength; i++)
			{
				CheatAction	* action = &entry->actionList[i];
				char		* name = entry->name;
				UINT32		type = action->type;
				char		* bufTraverse = buf;
				int			addressLength = 8;

				/* ----- set link enable and label if label-selection ----- */
				if(i)
				{
					SET_MASK_FIELD(type, LinkEnable);

					if(entry->flags & kCheatFlag_Select)
						name = action->optionalName;
				}

				/* ----- get address length for current CPU ----- */
				switch(EXTRACT_FIELD(type, LocationType))
				{
					case kLocation_Standard:
					case kLocation_HandlerMemory:
					case kLocation_ProgramSpace:
						addressLength = cpuInfoList[EXTRACT_FIELD(type, LocationParameter)].addressCharsNeeded;
						break;

					case kLocation_IndirectIndexed:
						addressLength = cpuInfoList[(EXTRACT_FIELD(type, LocationParameter) >> 2) & 0x7].addressCharsNeeded;
						break;

					case kLocation_MemoryRegion:
						{
							int	idx = EXTRACT_FIELD(type, LocationParameter) + REGION_CPU1 - REGION_INVALID;

							if(idx < kRegionListLength)
								addressLength = regionInfoList[idx].addressCharsNeeded;
						}
						break;
				}
#ifdef MESS
				bufTraverse += sprintf(bufTraverse, ":%s:%.8X:%.8X:%.*X:%.8X:%.8X", machine->gamedrv->name, thisGameCRC, type, addressLength, action->address, action->originalDataField, action->extendData);
#else
				bufTraverse += sprintf(bufTraverse, ":%s:%.8X:%.*X:%.8X:%.8X", machine->gamedrv->name, type, addressLength, action->address, action->originalDataField, action->extendData);
#endif
				/* ----- set description and comment ----- */
				if(name)
				{
					bufTraverse += sprintf(bufTraverse, ":%s", name);

					if(!i && (entry->comment))
						bufTraverse += sprintf(bufTraverse, ":%s", entry->comment);
				}
				else
				{
					if(!i && (entry->comment))
						bufTraverse += sprintf(bufTraverse, ":(none):%s", entry->comment);
				}

				bufTraverse += sprintf(bufTraverse, "\n");

				/* ----- write the normal cheat code ----- */
				mame_fwrite(theFile, buf, (UINT32)strlen(buf));
			}
			break;

		case activationKey:
			for(i = 0; i < 2; i++)
			{
				char		* bufTraverse = buf;
				int			addressLength;

				/* ----- if not find 1st activation key at saving 1st key, try to save 2nd key ----- */
				if(!i && !(entry->flags & kCheatFlag_HasActivationKey1))
					continue;

				/* ----- if not find 2nd key after 1st key saved, finish ------ */
				if(i && !(entry->flags & kCheatFlag_HasActivationKey2))
					break;

				/* ----- get address length for current CPU ----- */
				if(!(entry->flags & kCheatFlag_Select))
					addressLength = cpuInfoList[EXTRACT_FIELD(entry->actionList[0].type, LocationParameter)].addressCharsNeeded;
				else
					addressLength = cpuInfoList[EXTRACT_FIELD(entry->actionList[1].type, LocationParameter)].addressCharsNeeded;
#ifdef MESS
				if(!i)
					bufTraverse += sprintf(bufTraverse, ":%s:%.8X:63004000:%.*X:%.8X:00000000", machine->gamedrv->name, thisGameCRC, addressLength, selection, entry->activationKey1);
				else
					bufTraverse += sprintf(bufTraverse, ":%s:%.8X:63004001:%.*X:%.8X:00000000", machine->gamedrv->name, thisGameCRC, addressLength, selection, entry->activationKey2);
#else
				if(!i)
					bufTraverse += sprintf(bufTraverse, ":%s:63004000:%.*X:%.8X:00000000", machine->gamedrv->name, addressLength, selection, entry->activationKey1);
				else
					bufTraverse += sprintf(bufTraverse, ":%s:63004001:%.*X:%.8X:00000000", machine->gamedrv->name, addressLength, selection, entry->activationKey2);
#endif
				/* ----- set description and button index ----- */
				if(!i)
				{
					/* ----- 1st key ----- */
					astring *name = input_code_name(astring_alloc(), entry->activationKey1);
					if (entry->name)
						bufTraverse += sprintf(bufTraverse, ":1st Activation Key for %s (%s)\n", entry->name, astring_c(name));
					else
						bufTraverse += sprintf(bufTraverse, ":1st Activation Key (%s)\n", astring_c(name));
					astring_free(name);
				}
				else
				{
					/* ----- 2nd key ----- */
					astring *name = input_code_name(astring_alloc(), entry->activationKey2);
					if (entry->name)
						bufTraverse += sprintf(bufTraverse, ":2nd Activation Key for %s (%s)\n", entry->name, astring_c(name));
					else
						bufTraverse += sprintf(bufTraverse, ":2nd Activation Key (%s)\n", astring_c(name));
					astring_free(name);
				}

				/* ----- write the activation key code ----- */
				mame_fwrite(theFile, buf, (UINT32)strlen(buf));
			}
			break;

		case command:
			sprintf(buf, ":_command:%.8X\n", cheatOptions);

			/* ----- write the command code ----- */
			mame_fwrite(theFile, buf, (UINT32)strlen(buf));
			break;

		default:
			break;
	}

	/* ----- close the database ----- */
	mame_fclose(theFile);

	/* ----- clear dirty flag ----- */
	if((saveCode == normalCode) || (saveCode == activationKey))
		entry->flags &= ~kCheatFlag_Dirty;
}

/*--------------------------------------------------------------------------
  DoAutoSaveCheat - save normal code automatically when exit cheat system
--------------------------------------------------------------------------*/

static void DoAutoSaveCheats(running_machine *machine)
{
	int	i;

	for(i = 0; i < cheatListLength; i++)
	{
		CheatEntry	* entry = &cheatList[i];

		if(entry->flags & kCheatFlag_Dirty)
			SaveCheat(machine, entry, 0, 0);
	}
}

/*-------------------------------------------------------------------
  AddCheatFromResult - add a code from result viewer to cheat list
-------------------------------------------------------------------*/

static void AddCheatFromResult(SearchInfo * search, SearchRegion * region, UINT32 address)
{
	if(region->targetType == kRegionType_CPU)
	{
		CheatEntry	* entry = GetNewCheat();
		CheatAction	* action = &entry->actionList[0];
		char		tempString[1024];
		int			tempStringLength;
		UINT32		data = ReadSearchOperand(kSearchOperand_First, search, region, address);

		tempStringLength = sprintf(tempString, "%.8X (%d) = %.*X", address, region->targetIdx, kSearchByteDigitsTable[search->bytes], data);

		entry->name = realloc(entry->name, tempStringLength + 1);
		memcpy(entry->name, tempString, tempStringLength + 1);

		SET_FIELD(action->type, LocationParameter, region->targetIdx);
		SET_FIELD(action->type, BytesUsed, kSearchByteIncrementTable[search->bytes] - 1);
		action->address = address;
		action->data = data;
		action->extendData = 0xFFFFFFFF;
		action->originalDataField = data;

		UpdateCheatInfo(entry, 0);
	}
}

/*--------------------------------------------------------------------------------------------
  AddCheatFromFirstResult - add a code from search box to cheat list if found result is one
--------------------------------------------------------------------------------------------*/

static void AddCheatFromFirstResult(SearchInfo * search)
{
	int	i;

	for(i = 0; i < search->regionListLength; i++)
	{
		SearchRegion	* region = &search->regionList[i];

		if(region->numResults)
		{
			UINT32	traverse;

			for(traverse = 0; traverse < region->length; traverse++)
			{
				UINT32	address = region->address + traverse;

				if(IsRegionOffsetValid(search, region, traverse))
				{
					AddCheatFromResult(search, region, address);

					return;
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------
  AddWatchFromResult - add a watch code from result viewer to cheat list
-------------------------------------------------------------------------*/

static void AddWatchFromResult(SearchInfo * search, SearchRegion * region, UINT32 address)
{
	if(region->targetType == kRegionType_CPU)
	{
		WatchInfo	* info = GetUnusedWatch();

		info->address =			address;
		info->cpu =				region->targetIdx;
		info->numElements =		1;
		info->elementBytes =	kWatchSizeConversionTable[search->bytes];
		info->labelType =		kWatchLabel_None;
		info->displayType =		kWatchDisplayType_Hex;
		info->skip =			0;
		info->elementsPerLine =	0;
		info->addValue =		0;

		info->linkedCheat =		NULL;

		info->label[0] =		0;
	}
}

/*--------------------
  SearchSignExtend
--------------------*/

static UINT32 SearchSignExtend(SearchInfo * search, UINT32 value)
{
	if(search->sign)
	{
		if(value & kSearchByteSignBitTable[search->bytes])
			value |= ~kSearchByteUnsignedMaskTable[search->bytes];
	}

	return value;
}

/*-------------------------------------------------------------------
  ReadSearchOperand - read a data from search region and return it
-------------------------------------------------------------------*/

static UINT32 ReadSearchOperand(UINT8 type, SearchInfo * search, SearchRegion * region, UINT32 address)
{
	UINT32	value = 0;

	switch(type)
	{
		case kSearchOperand_Current:
			value = ReadRegionData(region, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap);
			break;

		case kSearchOperand_Previous:
			value = DoMemoryRead(region->last, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_First:
			value = DoMemoryRead(region->first, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_Value:
			value = search->value;
			break;
	}

	value = SearchSignExtend(search, value);

	return value;
}

/*------------------------------------------------------------------------------------------
  ReadSearchOperandBit - read a bit data from search region and return it when bit search
------------------------------------------------------------------------------------------*/

static UINT32 ReadSearchOperandBit(UINT8 type, SearchInfo * search, SearchRegion * region, UINT32 address)
{
	UINT32	value = 0;

	switch(type)
	{
		case kSearchOperand_Current:
			value = ReadRegionData(region, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap);
			break;

		case kSearchOperand_Previous:
			value = DoMemoryRead(region->last, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_First:
			value = DoMemoryRead(region->first, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_Value:
			if(search->value)
				value = 0xFFFFFFFF;
			else
				value = 0x00000000;
			break;
	}

	value = SearchSignExtend(search, value);

	return value;
}

/*----------------------------------------------------------
  DoSearchComparison - compare data and return it matched
----------------------------------------------------------*/

static UINT8 DoSearchComparison(SearchInfo * search, UINT32 lhs, UINT32 rhs)
{
	INT32	svalue;

	if(search->sign)
	{
		/* ----- signed ----- */
		INT32	slhs = lhs;
		INT32	srhs = rhs;

		switch(search->comparison)
		{
			case kSearchComparison_LessThan:
				return slhs < srhs;

			case kSearchComparison_GreaterThan:
				return slhs > srhs;

			case kSearchComparison_EqualTo:
				return slhs == srhs;

			case kSearchComparison_LessThanOrEqualTo:
				return slhs <= srhs;

			case kSearchComparison_GreaterThanOrEqualTo:
				return slhs >= srhs;

			case kSearchComparison_NotEqual:
				return slhs != srhs;

			case kSearchComparison_IncreasedBy:
				svalue = search->value;
				if(search->value & kSearchByteSignBitTable[search->bytes])
					svalue |= ~kSearchByteUnsignedMaskTable[search->bytes];

				return slhs == (srhs + svalue);

			case kSearchComparison_NearTo:
				return (slhs == srhs) || ((slhs + 1) == srhs);
		}
	}
	else
	{
		/* ----- unsigned ----- */
		switch(search->comparison)
		{
			case kSearchComparison_LessThan:
				return lhs < rhs;

			case kSearchComparison_GreaterThan:
				return lhs > rhs;

			case kSearchComparison_EqualTo:
				return lhs == rhs;

			case kSearchComparison_LessThanOrEqualTo:
				return lhs <= rhs;

			case kSearchComparison_GreaterThanOrEqualTo:
				return lhs >= rhs;

			case kSearchComparison_NotEqual:
				return lhs != rhs;

			case kSearchComparison_IncreasedBy:
				svalue = search->value;
				if(search->value & kSearchByteSignBitTable[search->bytes])
					svalue |= ~kSearchByteUnsignedMaskTable[search->bytes];

				return lhs == (rhs + svalue);

			case kSearchComparison_NearTo:
				return (lhs == rhs) || ((lhs + 1) == rhs);
		}
	}

	return 0;
}

/*-----------------------------------------------------------------
  DoSearchComparisonBit - compare bit data and return it matched
-----------------------------------------------------------------*/

static UINT32 DoSearchComparisonBit(SearchInfo * search, UINT32 lhs, UINT32 rhs)
{
	switch(search->comparison)
	{
		case kSearchComparison_LessThan:
		case kSearchComparison_NotEqual:
		case kSearchComparison_GreaterThan:
		case kSearchComparison_LessThanOrEqualTo:
		case kSearchComparison_GreaterThanOrEqualTo:
		case kSearchComparison_IncreasedBy:
			return lhs ^ rhs;

		case kSearchComparison_EqualTo:
		case kSearchComparison_NearTo:
			return ~(lhs ^ rhs);
	}

	return 0;
}

/*----------------------
  IsRegionOffsetValid
----------------------*/

/*
static UINT8 IsRegionOffsetValid(SearchInfo * search, SearchRegion * region, UINT32 offset)
{
    switch(kSearchByteIncrementTable[search->bytes])
    {
        case 1:
            return *((UINT8  *)&region->status[offset]) == 0xFF;
            break;

        case 2:
            return *((UINT16 *)&region->status[offset]) == 0xFFFF;
            break;

        case 4:
            return *((UINT32 *)&region->status[offset]) == 0xFFFFFFFF;
            break;
    }

    return 0;
}
*/

/*----------------------------------------------------------
  IsRegionOffsetValidBit - check selected offset is valid
----------------------------------------------------------*/

static UINT8 IsRegionOffsetValidBit(SearchInfo * search, SearchRegion * region, UINT32 offset)
{
	switch(kSearchByteStep[search->bytes])
	{
		case 1:
			return *((UINT8  *)&region->status[offset]) != 0;
			break;

		case 2:
			return *((UINT16 *)&region->status[offset]) != 0;
			break;

		case 3:
			return ((*((UINT16 *)&region->status[offset]) != 0) | (*((UINT8 *)&region->status[offset+2]) != 0));
			break;

		case 4:
			return *((UINT32 *)&region->status[offset]) != 0;
			break;
	}

	return 0;
}

/*----------------------------------------------------------------
  InvalidateRegionOffset - remove unmatched offset after search
----------------------------------------------------------------*/

static void InvalidateRegionOffset(SearchInfo * search, SearchRegion * region, UINT32 offset)
{
	switch(kSearchByteStep[search->bytes])
	{
		case 1:
			*((UINT8  *)&region->status[offset]) = 0;
			break;

		case 2:
			*((UINT16 *)&region->status[offset]) = 0;
			break;

		case 3:
			*((UINT16 *)&region->status[offset]) = 0;
			*((UINT8  *)&region->status[offset+2]) = 0;
			break;

		case 4:
			*((UINT32 *)&region->status[offset]) = 0;
			break;
	}
}

/*-----------------------------------------------------------------------
  InvalidateRegionOffsetBit - remove unmatched offset after bit search
-----------------------------------------------------------------------*/

static void InvalidateRegionOffsetBit(SearchInfo * search, SearchRegion * region, UINT32 offset, UINT32 invalidate)
{
	switch(kSearchByteStep[search->bytes])
	{
		case 1:
			*((UINT8  *)&region->status[offset]) &= ~invalidate;
			break;

		case 2:
			*((UINT16 *)&region->status[offset]) &= ~invalidate;
			break;

		case 3:
			*((UINT16 *)&region->status[offset]) &= ~invalidate;
			*((UINT8  *)&region->status[offset+2]) &= ~invalidate;
			break;

		case 4:
			*((UINT32 *)&region->status[offset]) &= ~invalidate;
			break;
	}
}

/*--------------------------------------------------------------
  InvalidateEntireRegions - invalidate selected search region
--------------------------------------------------------------*/

static void InvalidateEntireRegion(SearchInfo * search, SearchRegion * region)
{
	memset(region->status, 0, region->length);

	search->numResults -= region->numResults;
	region->numResults = 0;
}

/*-------------------------------------------------------------------
  InitializeNewSearch - initialize search region to start a search
-------------------------------------------------------------------*/

static void InitializeNewSearch(SearchInfo * search)
{
	int	i;

	search->numResults = 0;

	for(i = 0; i < search->regionListLength; i++)
	{
		SearchRegion	* region = &search->regionList[i];

		if(region->flags & kRegionFlag_Enabled)
		{
			region->numResults = 0;

			memset(region->status, 0xFF, region->length);

			FillBufferFromRegion(region, region->first);

			memcpy(region->last, region->first, region->length);
		}
	}
}

/*---------------------------------------------------------------------------
  UpdateSearch - updata a data in search region after initialize or search
---------------------------------------------------------------------------*/

static void UpdateSearch(SearchInfo * search)
{
	int	i;

	for(i = 0; i < search->regionListLength; i++)
	{
		SearchRegion	* region = &search->regionList[i];

		if(region->flags & kRegionFlag_Enabled)
			FillBufferFromRegion(region, region->last);
	}
}

/*-------------------------------------
  DoSearch - management for a search
-------------------------------------*/

static void DoSearch(SearchInfo * search)
{
	int	i, j;

	search->numResults = 0;

	if(search->bytes == kSearchSize_1Bit)
	{
		/* ----- bit search ----- */
		for(i = 0; i < search->regionListLength; i++)
		{
			SearchRegion	* region = &search->regionList[i];
			UINT32			lastAddress = region->length - kSearchByteIncrementTable[search->bytes] + 1;
			UINT32			increment = kSearchByteStep[search->bytes];

			region->numResults = 0;

			if((region->length < kSearchByteIncrementTable[search->bytes]) || !region->flags & kRegionFlag_Enabled)
				continue;

			for(j = 0; j < lastAddress; j += increment)
			{
				UINT32	address;
				UINT32	lhs, rhs;

				address = region->address + j;

				if(IsRegionOffsetValidBit(search, region, j))
				{
					UINT32	validBits;

					lhs = ReadSearchOperandBit(search->lhs, search, region, address);
					rhs = ReadSearchOperandBit(search->rhs, search, region, address);

					/* ----- do search ----- */
					validBits = DoSearchComparisonBit(search, lhs, rhs);

					InvalidateRegionOffsetBit(search, region, j, ~validBits);

					if(IsRegionOffsetValidBit(search, region, j))
					{
						search->numResults++;
						region->numResults++;
					}
				}
			}
		}
	}
	else
	{
		/* ----- normal search ----- */
		for(i = 0; i < search->regionListLength; i++)
		{
			SearchRegion	* region = &search->regionList[i];
			UINT32			lastAddress = region->length - kSearchByteIncrementTable[search->bytes] + 1;
			UINT32			increment = kSearchByteStep[search->bytes];

			region->numResults = 0;

			if((region->length < kSearchByteIncrementTable[search->bytes]) || !region->flags & kRegionFlag_Enabled)
				continue;

			for(j = 0; j < lastAddress; j += increment)
			{
				UINT32	address;
				UINT32	lhs, rhs;

				address = region->address + j;

				if(IsRegionOffsetValid(search, region, j))
				{
					lhs = ReadSearchOperand(search->lhs, search, region, address);
					rhs = ReadSearchOperand(search->rhs, search, region, address);

					/* ----- do search ----- */
					if(!DoSearchComparison(search, lhs, rhs))
						/* ----- unmatched ----- */
						InvalidateRegionOffset(search, region, j);
					else
					{
						/* ----- matched ----- */
						search->numResults++;
						region->numResults++;
					}
				}
			}
		}
	}
}

/*----------------------
  LookupHandlerMemory
----------------------*/

static UINT8 ** LookupHandlerMemory(UINT8 cpu, UINT32 address, UINT32 * outRelativeAddress)
{
	const address_map	* map = memory_get_address_map(cpu, ADDRESS_SPACE_PROGRAM);
	const address_map_entry *entry;

	for (entry = map->entrylist; entry != NULL; entry = entry->next)
		if (entry->write.generic != NULL && (address >= entry->addrstart) && (address <= entry->addrend))
		{
			if(outRelativeAddress)
				*outRelativeAddress = address - entry->addrstart;

			return (UINT8 **)entry->baseptr;
		}

	return NULL;
}

/*------------------------------------------
  DoCPURead - read a data from RAM region
------------------------------------------*/

static UINT32 DoCPURead(UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap)
{
	switch(bytes)
	{
		case 1:
				return	(cpunum_read_byte(cpu, address + 0) <<  0);

		case 2:
			if(swap)
				return	(cpunum_read_byte(cpu, address + 0) <<  0) | (cpunum_read_byte(cpu, address + 1) <<  8);
			else
				return	(cpunum_read_byte(cpu, address + 0) <<  8) | (cpunum_read_byte(cpu, address + 1) <<  0);
			break;

		case 3:
			if(swap)
				return	(cpunum_read_byte(cpu, address + 0) <<  0) | (cpunum_read_byte(cpu, address + 1) <<  8) |
						(cpunum_read_byte(cpu, address + 2) << 16);
			else
				return	(cpunum_read_byte(cpu, address + 0) << 16) | (cpunum_read_byte(cpu, address + 1) <<  8) |
						(cpunum_read_byte(cpu, address + 2) <<  0);
			break;

		case 4:
			if(swap)
				return	(cpunum_read_byte(cpu, address + 0) <<  0) | (cpunum_read_byte(cpu, address + 1) <<  8) |
						(cpunum_read_byte(cpu, address + 2) << 16) | (cpunum_read_byte(cpu, address + 3) << 24);
			else
				return	(cpunum_read_byte(cpu, address + 0) << 24) | (cpunum_read_byte(cpu, address + 1) << 16) |
						(cpunum_read_byte(cpu, address + 2) <<  8) | (cpunum_read_byte(cpu, address + 3) <<  0);
			break;
	}

	return 0;
}

/*-----------------------------------------------------
  DoMemoryRead - read a data from ROM or User region
-----------------------------------------------------*/

static UINT32 DoMemoryRead(UINT8 * buf, UINT32 address, UINT8 bytes, UINT8 swap, CPUInfo * info)
{
	UINT32	data = 0;

	if(!info)
	{
		switch(bytes)
		{
			case 1:
				data = buf[address];
				break;

			case 2:
				data = *((UINT16 *)&buf[address]);

				if(swap)
					data =	((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);
				break;

			case 4:
				data = *((UINT32 *)&buf[address]);

				if(swap)
					data =	((data >> 24) & 0x000000FF) | ((data >>  8) & 0x0000FF00) |
							((data <<  8) & 0x00FF0000) | ((data << 24) & 0xFF000000);
				break;

			default:
				info = &rawCPUInfo;
				goto generic;
		}

		return data;
	}

generic:

	if(swap)
	{
		UINT32	i;

		for(i = 0; i < bytes; i++)
			data |= buf[SwapAddress(address + i, bytes, info)] << (i * 8);
	}
	else
	{
		UINT32	i;

		for(i = 0; i < bytes; i++)
			data |= buf[SwapAddress(address + i, bytes, info)] << ((bytes - i - 1) * 8);
	}

	return data;
}

/*--------------------------------------------
  DoCPUWrite - write a data into RAM region
--------------------------------------------*/

static void DoCPUWrite(UINT32 data, UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap)
{
	switch(bytes)
	{
		case 1:
				cpunum_write_byte(cpu, address + 0, data & 0xFF);
			break;

		case 2:
			if(swap)
			{
				cpunum_write_byte(cpu, address + 0, (data >> 0) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >> 8) & 0xFF);
			}
			else
			{
				cpunum_write_byte(cpu, address + 0, (data >> 8) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >> 0) & 0xFF);
			}
			break;

		case 3:
			if(swap)
			{
				cpunum_write_byte(cpu, address + 0, (data >>  0) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >>  8) & 0xFF);
				cpunum_write_byte(cpu, address + 2, (data >> 16) & 0xFF);
			}
			else
			{
				cpunum_write_byte(cpu, address + 0, (data >> 16) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >>  8) & 0xFF);
				cpunum_write_byte(cpu, address + 2, (data >>  0) & 0xFF);
			}
			break;

		case 4:
			if(swap)
			{
				cpunum_write_byte(cpu, address + 0, (data >>  0) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >>  8) & 0xFF);
				cpunum_write_byte(cpu, address + 2, (data >> 16) & 0xFF);
				cpunum_write_byte(cpu, address + 3, (data >> 24) & 0xFF);
			}
			else
			{
				cpunum_write_byte(cpu, address + 0, (data >> 24) & 0xFF);
				cpunum_write_byte(cpu, address + 1, (data >> 16) & 0xFF);
				cpunum_write_byte(cpu, address + 2, (data >>  8) & 0xFF);
				cpunum_write_byte(cpu, address + 3, (data >>  0) & 0xFF);
			}
			break;

		default:
			logerror("DoCPUWrite: bad size (%d)\n", bytes);
			break;
	}
}

/*-------------------------------------------------------
  DoMemoryWrite - write a data into ROM or User region
-------------------------------------------------------*/

static void DoMemoryWrite(UINT32 data, UINT8 * buf, UINT32 address, UINT8 bytes, UINT8 swap, CPUInfo * info)
{
	if(!info)
	{
		switch(bytes)
		{
			case 1:
				buf[address] = data;
				break;

			case 2:
				if(swap)
					data =	((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);

				*((UINT16 *)&buf[address]) = data;
				break;

			case 4:
				if(swap)
					data =	((data >> 24) & 0x000000FF) | ((data >>  8) & 0x0000FF00) |
							((data <<  8) & 0x00FF0000) | ((data << 24) & 0xFF000000);

				*((UINT32 *)&buf[address]) = data;
				break;

			default:
				info = &rawCPUInfo;
				goto generic;
		}

		return;
	}

generic:

	if(swap)
	{
		UINT32	i;

		for(i = 0; i < bytes; i++)
			buf[SwapAddress(address + i, bytes, info)] = data >> (i * 8);
	}
	else
	{
		UINT32	i;

		for(i = 0; i < bytes; i++)
			buf[SwapAddress(address + i, bytes, info)] = data >> ((bytes - i - 1) * 8);
	}
}

/*--------------------------------
  CPUNeedsSwap - endian checker
--------------------------------*/

static UINT8 CPUNeedsSwap(UINT8 cpu)
{
	return cpuInfoList[cpu].endianness ^ 1;
}

/*-----------------------------------
  RegionNeedsSwap - endian checker
-----------------------------------*/

static UINT8 RegionNeedsSwap(UINT8 region)
{
	CPUInfo	* temp = GetRegionCPUInfo(region);

	if(temp)
		return temp->endianness ^ 1;

	return 0;
}

/*----------------------------------------------------
  GetCPUInfo - return CPU info for standard region
----------------------------------------------------*/

static CPUInfo * GetCPUInfo(UINT8 cpu)
{
		return &cpuInfoList[cpu];
}

/*-------------------------------------------------
  GetRegionCPUInfo - return CPU info for regions
-------------------------------------------------*/

static CPUInfo * GetRegionCPUInfo(UINT8 region)
{
	if((region >= REGION_INVALID) && (region < REGION_MAX))
		return &regionInfoList[region - REGION_INVALID];

	return NULL;
}

/*--------------
  SwapAddress
--------------*/

static UINT32 SwapAddress(UINT32 address, UINT8 dataSize, CPUInfo * info)
{
	switch(info->dataBits)
	{
		case 16:
			if(info->endianness == CPU_IS_BE)
				return BYTE_XOR_BE(address);
			else
				return BYTE_XOR_LE(address);

		case 32:
			if(info->endianness == CPU_IS_BE)
				return BYTE4_XOR_BE(address);
			else
				return BYTE4_XOR_LE(address);
	}

	return address;
}

/*------------------------------------
  ReadData - read a data per region
------------------------------------*/

static UINT32 ReadData(CheatAction * action)
{
	UINT8	parameter = EXTRACT_FIELD(action->type, LocationParameter);
	UINT8	bytes = EXTRACT_FIELD(action->type, BytesUsed) + 1;
	UINT8	swapBytes = EXTRACT_FIELD(action->type, Endianness);

	switch(EXTRACT_FIELD(action->type, LocationType))
	{
		case kLocation_Standard:
			return DoCPURead(parameter, action->address, bytes, CPUNeedsSwap(parameter) ^ swapBytes);
			break;

		case kLocation_MemoryRegion:
		{
			int		region = REGION_CPU1 + parameter;
			UINT8	* buf = memory_region(region);

			if(buf)
			{
				if(TEST_FIELD(cheatOptions, Debug))
				{
					if(ControlKeyPressed())
						ui_popup_time(1, "MR : %x : %p", region, buf);
				}

				if(IsAddressInRange(action, memory_region_length(region)))
					return DoMemoryRead(buf, action->address, bytes, RegionNeedsSwap(region) ^ swapBytes, GetRegionCPUInfo(region));
			}
		}
		break;

		case kLocation_HandlerMemory:
		{
			UINT32	relativeAddress;
			UINT8	** buf;

			if(!action->cachedPointer)
				action->cachedPointer = LookupHandlerMemory(parameter, action->address, &action->cachedOffset);

			buf = action->cachedPointer;
			relativeAddress = action->cachedOffset;

			if(buf && *buf)
				return DoMemoryRead(*buf, relativeAddress, bytes, CPUNeedsSwap(parameter) ^ swapBytes, GetCPUInfo(parameter));
		}
		break;

		case kLocation_IndirectIndexed:
		{
			UINT32	address;
			INT32	offset = action->extendData;
			UINT8	cpu = (parameter >> 2) & 0x7;
			UINT8	addressBytes = (parameter & 0x3) + 1;
			CPUInfo	* info = GetCPUInfo(cpu);

			address = DoCPURead(cpu, action->address, addressBytes, CPUNeedsSwap(cpu) ^ swapBytes);

			if(info)
				address = DoShift(address, info->addressShift);

			if(address)
			{
				address += offset;

				return DoCPURead(cpu, address, bytes, CPUNeedsSwap(cpu) ^ swapBytes);
			}
		}
		break;

		case kLocation_ProgramSpace:
		{
			UINT8	* buf;

			memory_set_opbase(action->address);

			buf = memory_get_op_ptr(parameter, action->address, 0);

			buf -= action->address;

			if(buf)
			{
				if(TEST_FIELD(cheatOptions, Debug))
				{
					if(ControlKeyPressed())
						ui_popup_time(1,"PG : %x : %p", parameter, buf);
				}

				return DoMemoryRead(buf, action->address, bytes, CPUNeedsSwap(parameter) ^ swapBytes, GetCPUInfo(parameter));
			}
		}
		break;

		case kLocation_Custom:
		{
			switch(parameter)
			{
				case kCustomLocation_Comment:
					break;

				case kCustomLocation_EEPROM:
				{
					int		length;
					UINT8	* buf;

					buf = EEPROM_get_data_pointer(&length);

					if(IsAddressInRange(action, length))
						return DoMemoryRead(buf, action->address, bytes, swapBytes, &rawCPUInfo);
				}
				break;
			}
		}
		break;

		default:
			break;
	}

	return 0;
}

/*--------------------------------------
  WriteData - write a data per region
--------------------------------------*/

static void WriteData(CheatAction * action, UINT32 data)
{
	UINT8	parameter = EXTRACT_FIELD(action->type, LocationParameter);
	UINT8	bytes = EXTRACT_FIELD(action->type, BytesUsed) + 1;
	UINT8	swapBytes = EXTRACT_FIELD(action->type, Endianness);

	switch(EXTRACT_FIELD(action->type, LocationType))
	{
		case kLocation_Standard:
			DoCPUWrite(data, parameter, action->address, bytes, CPUNeedsSwap(parameter) ^ swapBytes);

			break;

		case kLocation_MemoryRegion:
		{
			int		region = REGION_CPU1 + parameter;
			UINT8	* buf = memory_region(region);

			if(buf)
			{
				if(IsAddressInRange(action, memory_region_length(region)))
					DoMemoryWrite(data, buf, action->address, bytes, RegionNeedsSwap(region) ^ swapBytes, GetRegionCPUInfo(region));
			}
		}
		break;

		case kLocation_HandlerMemory:
		{
			UINT32	relativeAddress;
			UINT8	** buf;

			if(!action->cachedPointer)
			{
				action->cachedPointer = LookupHandlerMemory(parameter, action->address, &action->cachedOffset);
			}

			buf = action->cachedPointer;
			relativeAddress = action->cachedOffset;

			if(buf && *buf)
				DoMemoryWrite(data, *buf, relativeAddress, bytes, CPUNeedsSwap(parameter) ^ swapBytes, GetCPUInfo(parameter));
		}
		break;

		case kLocation_IndirectIndexed:
		{
			UINT32	address;
			INT32	offset = action->extendData;
			UINT8	cpu = (parameter >> 2) & 0x7;
			UINT8	addressBytes = (parameter & 0x3) + 1;
			CPUInfo	* info = GetCPUInfo(cpu);

			address = DoCPURead(cpu, action->address, addressBytes, CPUNeedsSwap(cpu) ^ swapBytes);

			if(info)
				address = DoShift(address, info->addressShift);

			if(address)
			{
				address += offset;

				DoCPUWrite(data, cpu, address, bytes, CPUNeedsSwap(cpu) ^ swapBytes);
			}
		}
		break;

		case kLocation_ProgramSpace:
		{
			UINT8	* buf;

			memory_set_opbase(action->address);

			buf = memory_get_op_ptr(parameter, action->address, 0);

			buf -= action->address;

			if(buf)
				DoMemoryWrite(data, buf, action->address, bytes, CPUNeedsSwap(parameter) ^ swapBytes, GetCPUInfo(parameter));
		}
		break;

		case kLocation_Custom:
		{
			switch(parameter)
			{
				case kCustomLocation_Comment:
					break;

				case kCustomLocation_EEPROM:
				{
					int		length;
					UINT8	* buf;

					buf = EEPROM_get_data_pointer(&length);

					if(IsAddressInRange(action, length))
						DoMemoryWrite(data, buf, action->address, bytes, swapBytes, &rawCPUInfo);
				}
				break;
			}
		}
		break;

		default:
			break;
	}
}

/*-----------------------------------------------------
  WatchCheatEntry - set watchpoint from action entry
-----------------------------------------------------*/

static void WatchCheatEntry(CheatEntry * entry, UINT8 associate)
{
	UINT32		i, j;
	CheatEntry	* associateEntry = NULL;

	if(associate)
		associateEntry = entry;

	if(!entry)
		return;

	for(i = 0; i < entry->actionListLength; i++)
	{
		if(!i)
			/* ----- first action entry ----- */
			AddActionWatch(&entry->actionList[i], associateEntry);
		else
		{
			/* ----- 2nd or later action entry ----- */
			UINT8	sameAddress = 0;

			for(j = 0; j < i; j++)
			{
				/* ----- if we find the same address action entry, skip it ----- */
				if(entry->actionList[j].address == entry->actionList[i].address)
					sameAddress = 1;
			}

			if(!sameAddress)
				AddActionWatch(&entry->actionList[i], associateEntry);
		}
	}
}

/*--------------------------------------------------
  AddActionWatch - add watchpoint into watch list
--------------------------------------------------*/

static void AddActionWatch(CheatAction * action, CheatEntry * entry)
{
	if(	(EXTRACT_FIELD(action->type, LocationType) == kLocation_Standard) ||
		(EXTRACT_FIELD(action->type, LocationType) == kLocation_MemoryRegion) ||
		(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed) ||
		(EXTRACT_FIELD(action->type, LocationType) == kLocation_ProgramSpace))

	{
		WatchInfo	* info = GetUnusedWatch();

		info->address =			action->address;
		info->cpu =				EXTRACT_FIELD(action->type, LocationParameter);
		info->displayType =		kWatchDisplayType_Hex;
		info->elementBytes =	kByteConversionTable[EXTRACT_FIELD(action->type, BytesUsed)];
		info->label[0] =		0;
		info->labelType =		kWatchLabel_None;
		info->linkedCheat =		entry;
		info->numElements =		1;
		info->skip =			0;

		info->locationType =	EXTRACT_FIELD(action->type, LocationType);

		if(EXTRACT_FIELD(action->type, Type) == kType_Watch)
		{
			/* ----- set watchpoint from watch code ----- */
			UINT32	typeParameter = EXTRACT_FIELD(action->type, TypeParameter);

			info->numElements = (action->data & 0xFF) + 1;

			info->skip = (action->data >> 8) & 0xFF;
			info->elementsPerLine = (action->data >> 16) & 0xFF;
			info->addValue = (action->data >> 24) & 0xFF;

			if(info->addValue & 0x80)
				info->addValue |= ~0xFF;

			if(action->extendData != 0xFFFFFFFF)
			{
				/* ### fix me... ### */
				info->x = (float)((action->extendData >> 16) & 0xFFFF)/100;
				info->y = (float)((action->extendData >>  0) & 0xFFFF)/100;
			}

			/* ----- if label is needed, copy the description from comment field ----- */
			if((typeParameter & 0x04) && (entry->comment) && (strlen(entry->comment) < 256))
			{
				info->labelType = kWatchLabel_String;
				strcpy(info->label, entry->comment);
			}

			info->displayType = typeParameter & 0x03;
		}
	}
}

/*-----------------------------------------------------------------
  RemoveAssociatedWatches - remove watchpoint when CODE is "OFF"
-----------------------------------------------------------------*/

static void RemoveAssociatedWatches(CheatEntry * entry)
{
	int	i;

	for(i = watchListLength - 1; i >= 0; i--)
	{
		WatchInfo	* info = &watchList[i];

		if(info->linkedCheat == entry)
			DeleteWatchAt(i);
	}
}

/*----------------------------------------------------------------------
  ResetAction - back up data and set action flags when turn CODE "ON"
----------------------------------------------------------------------*/

static void ResetAction(CheatAction * action)
{
	action->frameTimer = 0;

	/* ----- backup value ----- */
	if(!(action->flags & kActionFlag_LastValueGood))
		action->lastValue = ReadData(action);

	action->flags &= ~kActionFlag_StateMask;
	action->flags |= kActionFlag_LastValueGood;
}

/*-------------------------------------------------------------------------------------
  ActivateCheat - reset action entry and set activate entry flag when turn CODE "ON"
-------------------------------------------------------------------------------------*/

static void ActivateCheat(CheatEntry * entry)
{
	int	i;

	for(i = 0; i < entry->actionListLength; i++)
	{
		CheatAction	* action = &entry->actionList[i];

		ResetAction(action);

		/* ----- if watchpoint code, add watchpoint ----- */
		if(EXTRACT_FIELD(action->type, Type) == kType_Watch)
			AddActionWatch(action, entry);
	}

	entry->flags |= kCheatFlag_Active;
}

/*--------------------------------------------------------------------------------------
  DeactivateCheat - restore previous value and remove watchpoint when turn CODE "OFF"
--------------------------------------------------------------------------------------*/

static void DeactivateCheat(CheatEntry * entry)
{
	int	i;

	for(i = 0; i < entry->actionListLength; i++)
	{
		CheatAction	* action = &entry->actionList[i];

		/* ----- restore previous value and clear backup flag if needed ----- */
		if(EXTRACT_FIELD(action->type, RestorePreviousValue) && (action->flags & kActionFlag_LastValueGood))
		{
			WriteData(action, action->lastValue);

			action->flags &= ~kActionFlag_LastValueGood;
		}
	}

	/* ----- remove watchpoint ----- */
	RemoveAssociatedWatches(entry);

	entry->flags &= ~kCheatFlag_StateMask;
}

/*--------------------------------------------------------------------
  TempDeactivateCheat - restore previous value when turn CHEAT "OFF"
--------------------------------------------------------------------*/

static void TempDeactivateCheat(CheatEntry * entry)
{
	if(entry->flags & kCheatFlag_Active)
	{
		int	i;

		for(i = 0; i < entry->actionListLength; i++)
		{
			CheatAction	* action = &entry->actionList[i];

			/* ----- restore previous value if needed ----- */
			if(EXTRACT_FIELD(action->type, RestorePreviousValue) && (action->flags & kActionFlag_LastValueGood))
				WriteData(action, action->lastValue);
		}
	}
}

/*------------------------------------------------------------
  cheat_periodicOperation - management for cheat operations
------------------------------------------------------------*/

static void cheat_periodicOperation(CheatAction * action)
{
	UINT8	operation = EXTRACT_FIELD(action->type, Operation) | (EXTRACT_FIELD(action->type, OperationExtend) << 2);

	switch(operation)
	{
		case kOperation_WriteMask:
		{
			UINT32	temp;

			if(action->flags & kActionFlag_IgnoreMask)
				WriteData(action, action->data);
			else
			{
				temp = ReadData(action);

				temp = (action->data & action->extendData) | (temp & ~action->extendData);

				WriteData(action, temp);
			}
		}
		break;

		case kOperation_AddSubtract:
		{
			//INT32 temp, bound;

			/* ----- if extend data field is invalid, direct return ----- */
			//if(action->flags & kActionFlag_IgnoreMask)
				return;

			/* FIXME: AddSubstract looks seriously broken
             *        and being worked on. */
#if 0
			temp = ReadData(action);

			/* ----- OperationParameter field stores add/subtract ----- */
			if(TEST_FIELD(action->type, OperationParameter))
			{
				/* ----- subtract ----- */
				bound = action->extendData + action->data;

				if(temp > bound)
					temp -= action->data;
			}
			else
			{
				/* ----- add ----- */
				bound = action->extendData - action->data;

				if(temp < bound)
					temp += action->data;
			}

			WriteData(action, temp);
#endif
		}
		break;

		case kOperation_ForceRange:
		{
			UINT32	temp;

			/* ----- if extend data field is invalid, direct return ----- */
			if(action->flags & kActionFlag_IgnoreMask)
				return;

			temp = ReadData(action);

			if(!TEST_FIELD(action->type, BytesUsed))
			{
				/* ----- 8 bit ----- */
				if(	(temp < ((action->extendData >> 8) & 0xFF)) || (temp > ((action->extendData >> 0) & 0xFF)))
				{
					temp = action->data;

					WriteData(action, temp);
				}
			}
			else
			{
				/* ----- 16 bit (or 24, 32 bit) ----- */
				if(	(temp < ((action->extendData >> 16) & 0xFFFF)) || (temp > ((action->extendData >> 0) & 0xFFFF)))
				{
					temp = action->data;

					WriteData(action, temp);
				}
			}
		}
		break;

		case kOperation_SetOrClearBits:
		{
			UINT32	temp;

			temp = ReadData(action);

			if(TEST_FIELD(action->type, OperationParameter))
				/* ----- clear bit ----- */
				temp &= ~action->data;
			else
				/* ----- set bit ----- */
				temp |= action->data;

			WriteData(action, temp);
		}
		break;

		case kOperation_WriteMatch:
		{
			UINT32	temp;

			/* ----- if extend data field is invalid, direct return ----- */
			if(action->flags & kActionFlag_IgnoreMask)
				return;

			temp = ReadData(action);

			/* ----- if current value matches appointed value, write ----- */
			if(temp == action->extendData)
			{
				temp = action->data;

				WriteData(action, temp);
			}
		}
		break;

		case kOperation_None:
			break;

		default:
			break;
	}
}

/*------------------------------------------------------
  cheat_periodicAction - management for cheat actions
------------------------------------------------------*/

static void cheat_periodicAction(running_machine *machine, CheatAction * action)
{
	UINT8	parameter = EXTRACT_FIELD(action->type, TypeParameter);

	/* ----- if operation is done in case of one shot, no action ----- */
	if(action->flags & kActionFlag_OperationDone)
		return;

	if(TEST_FIELD(action->type, Prefill) && (!(action->flags & kActionFlag_PrefillDone)))
	{
		/* ----- pre-fill ----- */
		UINT32	prefillValue = kPrefillValueTable[EXTRACT_FIELD(action->type, Prefill)];

		if(!(action->flags & kActionFlag_PrefillWritten))
		{
			/* ----- set pre-fill value ---- */
			WriteData(action, prefillValue);

			action->flags |= kActionFlag_PrefillWritten;

			return;
		}
		else
		{
			/* ----- if value is changed, try to write new value ----- */
			if(ReadData(action) == prefillValue)
				return;

			action->flags |= kActionFlag_PrefillDone;
		}
	}

	switch(EXTRACT_FIELD(action->type, Type))
	{
		case kType_NormalOrDelay:
		{
			if(TEST_FIELD(action->type, OneShot) && TEST_FIELD(action->type, RestorePreviousValue) && parameter)
			{
				/* ----- keep if one shot + restore prevous value + delay !=0 ----- */
				cheat_periodicOperation(action);

				if(action->frameTimer >= (parameter * ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds)))
				{
					action->frameTimer = 0;

					action->flags |= kActionFlag_OperationDone;
				}
				else
					action->frameTimer++;
			}
			else
			{
				/* ----- otherwise, delay ----- */
				if(action->frameTimer >= (parameter * ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds)))
				{
					action->frameTimer = 0;

					cheat_periodicOperation(action);

					if(TEST_FIELD(action->type, OneShot))
						action->flags |= kActionFlag_OperationDone;
				}
				else
					action->frameTimer++;
			}
		}
		break;

		case kType_WaitForModification:
		{
			if(action->flags & kActionFlag_WasModified)
			{
				if(action->frameTimer <= 0)
				{
					cheat_periodicOperation(action);

					action->flags &= ~kActionFlag_WasModified;

					if(TEST_FIELD(action->type, OneShot))
						action->flags |= kActionFlag_OperationDone;
				}
				else
					action->frameTimer--;

				action->lastValue = ReadData(action);
			}
			else
			{
				UINT32	currentValue = ReadData(action);

				if(currentValue != action->lastValue)
				{
					action->frameTimer = parameter * ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds);

					action->flags |= kActionFlag_WasModified;
				}

				action->lastValue = currentValue;
			}
		}
		break;

		case kType_IgnoreIfDecrementing:
		{
			UINT32	currentValue = ReadData(action);

			/* ### is "NotEqual" correct ? "LessOrEqual" seems to be good... ### */
			if(currentValue != (action->lastValue - parameter))
			{
				cheat_periodicOperation(action);

				if(TEST_FIELD(action->type, OneShot))
					action->flags |= kActionFlag_OperationDone;
			}

			action->lastValue = currentValue;
		}
		break;

		default:
			break;
	}
}

/*-----------------------------------------------------
  cheat_periodicEntry - management for cheat entries
-----------------------------------------------------*/

static void cheat_periodicEntry(running_machine *machine, CheatEntry * entry)
{
	int	i;

	/* ----- special handling for label-selection ----- */
	if(entry->flags & kCheatFlag_Select)
	{
		/* ----- special handling for activation key ----- */
		if((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2))
		{
			if(input_code_pressed(entry->activationKey1) || input_code_pressed(entry->activationKey2))
			{
				if(!(entry->flags & kCheatFlag_ActivationKeyPressed))
				{
					/* ----- move current position until find OFF or no link extension code ----- */
					do{
						/* ----- 1st activation key pressed ----- */
						if(input_code_pressed(entry->activationKey1))
						{
							entry->selection++;

							if(entry->flags & kCheatFlag_OneShot)
							{
								if(entry->selection >= entry->actionListLength)
								{
									entry->selection = 1;

									if(entry->selection >= entry->actionListLength)
										entry->selection = 0;
								}

								if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
								{
									ActivateCheat(entry);

									entry->flags |= kCheatFlag_DoOneShot;
								}
							}
							else
							{
								if(entry->selection >= entry->actionListLength)
								{
									entry->selection = 0;

									DeactivateCheat(entry);
								}
								else
								{
									if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
										ActivateCheat(entry);
								}
							}
						}
						/* ----- 2nd activation key pressed ----- */
						else
						{
							entry->selection--;

							if(entry->flags & kCheatFlag_OneShot)
							{
								if(entry->selection <= 0)
									entry->selection = entry->actionListLength - 1;

								if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
								{
									ActivateCheat(entry);

									entry->flags |= kCheatFlag_DoOneShot;
								}
							}
							else
							{
								if(!entry->selection)
									DeactivateCheat(entry);
								else
								{
									if(entry->selection < 0)
										entry->selection = entry->actionListLength - 1;

									if(!TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension))
										ActivateCheat(entry);
								}
							}
						}
					}while(entry->selection && TEST_FIELD(entry->actionList[entry->selection].type, LinkExtension));

					/* ----- display selected label name if needed ----- */
					if(TEST_FIELD(cheatOptions, ActivationKeyMessage))
					{
						if(!entry->selection)
							ui_popup_time(1,"%s disabled", entry->name);
						else
							ui_popup_time(1,"%s : %s selected", entry->name, entry->actionList[entry->selection].optionalName);
					}

					entry->flags |= kCheatFlag_ActivationKeyPressed;		// set flag
				}
			}
			else
				entry->flags &= ~kCheatFlag_ActivationKeyPressed;			// clear flag
		}

		/* ----- if a subcheat is selected, handle it ----- */
		if(entry->selection)
		{
			i = entry->selection;

			do{
				if(!(entry->flags & kCheatFlag_OneShot) || (entry->flags & kCheatFlag_DoOneShot))
					cheat_periodicAction(machine, &entry->actionList[i]);

				i++;

				/* ----- if illegal index, finish ----- */
				if(i >= entry->actionListLength)
					break;

			/* ----- if next code has link extension, handle it too ----- */
			}while(TEST_FIELD(entry->actionList[i].type, LinkExtension));
		}
	}
	else
	{
		/* ----- NOTE : activatinon key for value-selection is disabled -----*/
		if(((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2)) &&
			!(entry->flags & kCheatFlag_UserSelect))
		{
			/* ----- special handling for activation key ----- */
			if(input_code_pressed(entry->activationKey1) || input_code_pressed(entry->activationKey2))
			{
				if(!(entry->flags & kCheatFlag_ActivationKeyPressed))
				{
					if(entry->flags & kCheatFlag_OneShot)
					{
						ActivateCheat(entry);

						/* ----- display description for one shot if needed ----- */
						if(TEST_FIELD(cheatOptions, ActivationKeyMessage))
							ui_popup_time(1,"set %s", entry->name);
					}
					else
					{
						if(entry->flags & kCheatFlag_Active)
						{
							DeactivateCheat(entry);

							/* ----- display description for OFF if needed ----- */
							if(TEST_FIELD(cheatOptions, ActivationKeyMessage))
								ui_popup_time(1,"%s disabled", entry->name);
						}
						else
						{
							ActivateCheat(entry);

							/* ----- display description for ON if needed ----- */
							if(TEST_FIELD(cheatOptions, ActivationKeyMessage))
								ui_popup_time(1,"%s enabled", entry->name);
						}
					}

					entry->flags |= kCheatFlag_ActivationKeyPressed;
				}
			}
			else
				entry->flags &= ~kCheatFlag_ActivationKeyPressed;
		}

		if(!(entry->flags & kCheatFlag_Active))
			return;

		/* ----- update all actions ----- */
		for(i = 0; i < entry->actionListLength; i++)
			cheat_periodicAction(machine, &entry->actionList[i]);

		/* ----- if all actions are done, deactivate the cheat if oneshot entry ----- */
		{
			UINT8	done = 1;

			for(i = 0 ; (i < entry->actionListLength && done) ; i++)
				if(!(entry->actionList[i].flags & kActionFlag_OperationDone))
					done = 0;

			if(done)
				DeactivateCheat(entry);
		}
	}
}

/*------------------------------------------------------------------
  UpdateAllCheatInfo - update all cheat info when database loaded
------------------------------------------------------------------*/

static void UpdateAllCheatInfo(void)
{
	int	i;

	for(i = 0; i < cheatListLength; i++)
		UpdateCheatInfo(&cheatList[i], 1);
}

/*--------------------------------------------------
  UpdateCheatInfo - update entry, action and flag
--------------------------------------------------*/

static void UpdateCheatInfo(CheatEntry * entry, UINT8 isLoadTime)
{
	int		isOneShot =	1;
	int		isNull =	1;
	int		flags =		0;
	int		i;

	flags = entry->flags & kCheatFlag_PersistentMask;

	/* ---- is it label-selection ? ----- */
	if(	(EXTRACT_FIELD(entry->actionList[0].type, LocationType) == kLocation_Custom) &&
		(EXTRACT_FIELD(entry->actionList[0].type, LocationParameter) == kCustomLocation_Select))
			flags |= kCheatFlag_Select;

	for(i = 0; i < entry->actionListLength; i++)
	{
		CheatAction	* action = &entry->actionList[i];

		int			isActionNull =	0;
		UINT32		size;
		UINT32		operation;
		UINT32		actionFlags = action->flags & kActionFlag_PersistentMask;

		size = EXTRACT_FIELD(action->type, BytesUsed);

		operation = EXTRACT_FIELD(action->type, Operation) | EXTRACT_FIELD(action->type, OperationExtend) << 2;

		/* ---- is it comment ? ----- */
		if(	(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom) &&
			(EXTRACT_FIELD(action->type, LocationParameter) == kCustomLocation_Comment))
				isActionNull = 1;
		else
			isNull = 0;

		/* ---- is it one shot ? ----- */
		if(!TEST_FIELD(action->type, OneShot))
			isOneShot = 0;

		/* ---- is it value-selection ? ----- */
		if(TEST_FIELD(action->type, UserSelectEnable))
			flags |= kCheatFlag_UserSelect;

		/* ---- is it relative address ? ----- */
		if(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed)
			actionFlags |= kActionFlag_IgnoreMask;
		else
		{
			if(isLoadTime)
			{
				/* ----- check for mask == 0 and fix when database loaded ----- */
				if((operation == kOperation_WriteMask) && (action->extendData == 0))
					action->extendData = ~0;
			}
		}

		action->flags = actionFlags;
	}

	/* ---- set one shot flag ----- */
	if(isOneShot)
		flags |= kCheatFlag_OneShot;

	/* ---- set null flag ----- */
	if(isNull)
		flags |= kCheatFlag_Null;

	entry->flags = (flags & kCheatFlag_InfoMask) | (entry->flags & ~kCheatFlag_InfoMask);

	if(isLoadTime)
		entry->flags &= ~kCheatFlag_Dirty;
}

/*--------------------------------------------------------------------------------
  IsAddressInRange - check an address is in range (1 = in range, 0 = out range)
--------------------------------------------------------------------------------*/

static int IsAddressInRange(CheatAction * action, UINT32 length)
{
	UINT8	bytes = EXTRACT_FIELD(action->type, BytesUsed) + 1;

	return ((action->address + bytes) <= length);
}

/*---------------------------------------------------------------
  BuildCPUInfoList - get CPU info when initialize cheat system
---------------------------------------------------------------*/

static void BuildCPUInfoList(running_machine *machine)
{
	int	i;

	/* ----- do regions ----- */
	{
		const rom_entry	 * traverse = rom_first_region(machine->gamedrv);

		memset(regionInfoList, 0, sizeof(CPUInfo) * kRegionListLength);

		while(traverse)
		{
			if(ROMENTRY_ISREGION(traverse))
			{
				UINT8	regionType = ROMREGION_GETTYPE(traverse);

				/* ----- non-cpu region ? ----- */
				if((regionType >= REGION_GFX1) && (regionType <= REGION_USER8))
				{
					CPUInfo	* info = &regionInfoList[regionType - REGION_INVALID];
					UINT32	length = memory_region_length(regionType);
					int		bitState = 0;

					info->type = regionType;
					info->dataBits = ROMREGION_GETWIDTH(traverse);

					info->addressBits = 0;
					info->addressMask = length;

					/* ----- build address mask ----- */
					for(i = 0; i < 32; i++)
					{
						UINT32	mask = 1 << (31 - i);

						if(bitState)
							info->addressMask |= mask;
						else
						{
							if(info->addressMask & mask)
							{
								info->addressBits = 32 - i;
								bitState = 1;
							}
						}
					}

					info->addressCharsNeeded = info->addressBits >> 2;

					if(info->addressBits & 3)
						info->addressCharsNeeded++;

					info->endianness = ROMREGION_ISBIGENDIAN(traverse);
				}
			}

			traverse = rom_next_region(traverse);
		}
	}

	/* ----- do CPUs ----- */
	{
		memset(cpuInfoList, 0, sizeof(CPUInfo) * MAX_CPU);

		for(i = 0; i < cpu_gettotalcpu(); i++)
		{
			CPUInfo	* info = &cpuInfoList[i];
			CPUInfo	* regionInfo = &regionInfoList[REGION_CPU1 + i - REGION_INVALID];

			cpu_type type = machine->config->cpu[i].type;

			info->type = type;
			info->dataBits = cputype_databus_width(type, ADDRESS_SPACE_PROGRAM);
			info->addressBits = cputype_addrbus_width(type, ADDRESS_SPACE_PROGRAM);
			info->addressMask = 0xFFFFFFFF >> (32 - cputype_addrbus_width(type, ADDRESS_SPACE_PROGRAM));

			info->addressCharsNeeded = info->addressBits >> 2;
			if(info->addressBits & 0x3)
				info->addressCharsNeeded++;

			info->endianness = (cputype_endianness(type) == CPU_IS_BE);

			switch(type)
			{
#if HAS_TMS34010
				case CPU_TMS34010:
					info->addressShift = 3;
					break;
#endif
#if HAS_TMS34020
				case CPU_TMS34020:
					info->addressShift = 3;
					break;
#endif
				default:
					info->addressShift = 0;
					break;
			}

			/* ----- copy to region list ----- */
			memcpy(regionInfo, info, sizeof(CPUInfo));
		}
	}
}

