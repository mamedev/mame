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

#define REGION_LIST_LENGTH						(REGION_MAX - REGION_INVALID)
#define VALID_CPU(cpu)							(cpu < cpu_gettotalcpu())

#define ADD_MENU_2_ITEMS(name, sub_name)		do { menu_item[total] = name; menu_sub_item[total] = sub_name; total++; } while(0)
#define TERMINATE_MENU_2_ITEMS					do { menu_item[total] = NULL; menu_sub_item[total] = NULL; } while(0)

#define ADD_MENU_3_ITEMS(name, sub_name, enum)	do { menu_item[total] = name; menu_sub_item[total] = sub_name; menu_index[total] = enum; total++; } while(0)

#define ADJUST_CURSOR(sel, total)				do { if(sel < 0) sel = 0; else if(sel > (total - 1)) sel = (total - 1); } while(0);
#define CURSOR_TO_NEXT(sel, total)				do { if(++sel > (total - 1)) sel = 0; } while(0)
#define CURSOR_TO_PREVIOUS(sel, total)			do { if(--sel < 0) sel = (total - 1); } while(0)
#define CURSOR_PAGE_UP(sel)						do { sel -= visible_items; if(sel < 0) sel = 0; } while (0)
#define CURSOR_PAGE_DOWN(sel, total)			do { sel += visible_items; if(sel >= total) sel = (total - 1); } while (0)
#define SET_MESSAGE(type)						do { message_timer = DEFAULT_MESSAGE_TIME; message_type = type; } while(0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CHEAT_FILENAME_MAX_LEN					(255)
#define CHEAT_MENU_DEPTH						(8)
#ifdef MESS
#define DEFAULT_CHEAT_OPTIONS					(0x4F)
#else
#define DEFAULT_CHEAT_OPTIONS					(0xF)
#endif
#define DEFAULT_MESSAGE_TIME					(50)
#define VARIABLE_MAX_ARRAY						(0x0F)		/* NOTE : Destination in Move code can only accept variable[0]-[3] only */
#define CHEAT_RETURN_VALUE						(0xFF)
#define SAFE_SEARCH_REGION_RANGE				(0xFFFF)

/********** BIT FIELD **********/

enum
{
	/* type field */
	DEFINE_BITFIELD_ENUM(OneShot,					0,	0),
	DEFINE_BITFIELD_ENUM(DelayEnable,				1,	3),
	DEFINE_BITFIELD_ENUM(PrefillEnable,				4,	5),
	DEFINE_BITFIELD_ENUM(Return,					6,	6),
	DEFINE_BITFIELD_ENUM(RestoreValue,				7,	7),
	DEFINE_BITFIELD_ENUM(ValueSelectEnable,			8,	8),		/* = CopyValueEnable */
	DEFINE_BITFIELD_ENUM(ValueSelectMinimumDisplay,	9,	9),
	DEFINE_BITFIELD_ENUM(ValueSelectMinimum,		10,	10),
	DEFINE_BITFIELD_ENUM(ValueSelectBCD,			11,	11),	/* = CopyValueBCD */
	DEFINE_BITFIELD_ENUM(ValueSelectNegative,		12,	12),	/* = CopyValueNegative */
	DEFINE_BITFIELD_ENUM(PopupParameter,			13,	14),
	DEFINE_BITFIELD_ENUM(DataRead,					15,	15),	/* 0 = field, 1 = variable */
	DEFINE_BITFIELD_ENUM(Link,						16,	17),	/* 0 = master, 1 = link, 2 = sub link, 3 = EOL */
	DEFINE_BITFIELD_ENUM(Endianness,				18,	18),
	DEFINE_BITFIELD_ENUM(AddressSize,				20,	21),
	DEFINE_BITFIELD_ENUM(AddressRead,				22,	23),	/* 0 = field, 1 = indirect address from variable, 2 = variable */
	DEFINE_BITFIELD_ENUM(CodeParameter,				24,	27),
	DEFINE_BITFIELD_ENUM(CodeParameterLower,		24,	25),
	DEFINE_BITFIELD_ENUM(CodeParameterUpper,		26,	27),
	DEFINE_BITFIELD_ENUM(CodeType,					28,	31),

	/* extend field */
	DEFINE_BITFIELD_ENUM(LSB16,						0,	15),	/* lower 4 bytes */
	DEFINE_BITFIELD_ENUM(MSB16,						16,	31),	/* upper 4 bytes */

	/* watch code */
	DEFINE_BITFIELD_ENUM(WatchDisplayFormat,		24,	25),	/* from type field */
	DEFINE_BITFIELD_ENUM(WatchShowLabel,			26,	27),	/* from type field */
	DEFINE_BITFIELD_ENUM(WatchNumElements,			0,	7),		/* from data field */
	DEFINE_BITFIELD_ENUM(WatchSkip,					8,	15),	/* from data field */
	DEFINE_BITFIELD_ENUM(WatchElementsPerLine,		16,	23),	/* from data field */
	DEFINE_BITFIELD_ENUM(WatchAddValue,				24,	31),	/* from data field */

	/* label-selection */
	DEFINE_BITFIELD_ENUM(LabelSelectUseSelector,	4,	4),
	DEFINE_BITFIELD_ENUM(LabelSelectQuickClose,		5,	5),

	/* cpu/region field */
	DEFINE_BITFIELD_ENUM(CPUIndex,					0,	3),
	DEFINE_BITFIELD_ENUM(AddressSpace,				4,	6),
//  DEFINE_BITFIELD_ENUM(RegionIndex,               0,  6),     /* unused ??? */

	/* old code */
//  DEFINE_BITFIELD_ENUM(OneShot,                   0,  0),     /* = OneShot in new format */
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
	DEFINE_BITFIELD_ENUM(EndiannessOld,				22,	22),
	DEFINE_BITFIELD_ENUM(RestorePreviousValue,		23, 23),
	DEFINE_BITFIELD_ENUM(LocationParameter,			24,	28),
	DEFINE_BITFIELD_ENUM(LocationType,				29,	31),

	/* location parameter extention (relative address, memory accessor) */
	DEFINE_BITFIELD_ENUM(LocationParameterOption,	24,	25),
	DEFINE_BITFIELD_ENUM(LocationParameterCPU,		26,	28),

	/* command */
	DEFINE_BITFIELD_ENUM(SearchBox,					0,	1),
	DEFINE_BITFIELD_ENUM(DontPrintNewLabels,		2,	2),		/* in options menu, it is reversed display */
	DEFINE_BITFIELD_ENUM(AutoSaveEnabled,			3,	3),
	DEFINE_BITFIELD_ENUM(ActivationKeyMessage,		4,	4),
	DEFINE_BITFIELD_ENUM(LoadOldFormat,				5,	5),
#ifdef MESS
	DEFINE_BITFIELD_ENUM(SharedCode,				6,	6)
#endif
};

/********** OPERATION **********/

enum{		// address space for RAM region
	kAddressSpace_Program = 0,
	kAddressSpace_DataSpace,
	kAddressSpace_IOSpace,
	kAddressSpace_OpcodeRAM,
	kAddressSpace_MappedMemory,
	kAddressSpace_DirectMemory,
	kAddressSpace_EEPROM };

enum{		// code types
	kCodeType_Write = 0,		// EF = Mask
	kCodeType_IWrite,			// EF = Index
	kCodeType_RWrite,			// EF = Count/Skip
	kCodeType_CWrite,			// EF = Condition
	kCodeType_CBit,				// EF = Condition
	kCodeType_PDWWrite,			// EF = 2nd Data
	kCodeType_Move,				// DF = Add Value, EF = Destination
	kCodeType_Branch,			// DF = Jump Code, EF = Condition
	kCodeType_Loop,
	kCodeType_Popup,			// EF = Condition
	kCodeType_Watch,			// EF = position
	kCodeType_Unused };

enum{		/* parameter for IWrite */
	IWRITE_WRITE = 0,
	IWRITE_BIT_SET,
	IWRITE_BIT_CLEAR,
	IWRITE_LIMITED_MASK };

enum{		/* parameter for CBit */
	CBIT_BIT_SET = 0,
	CBIT_BIT_CLEAR,
	CBIT_LIMITED_MASK };

enum{		/* conditions for CWrite, Branch, Popup */
	kCondition_Equal = 0,
	kCondition_NotEqual,
	kCondition_Less,
	kCondition_LessOrEqual,
	kCondition_Greater,
	kCondition_GreaterOrEqual,
	kCondition_BitSet,
	kCondition_BitClear,
	kCondition_Unused1,
	kCondition_Unused2,
	kCondition_Unused3,
	kCondition_Unused4,
	kCondition_PreviousValue,
	kCondition_KeyPressedOnce,
	kCondition_KeyPressedRepeat,
	kCondition_True };

enum{		/* conditions for CBit */
	CONDITION_CBIT_BIT_SET = 0,
	CONDITION_CBIT_BIT_CLEAR };

enum{		// popup parameter
	kPopup_Label = 0,
	kPopup_Value,
	kPopup_LabelValue,
	kPopup_ValueLabel };

enum{		/* custom code */
	CUSTOM_CODE = 0xF0,
	CUSTOM_CODE_COMMENT,
	CUSTOM_CODE_SEPARATOR,
	CUSTOM_CODE_LABEL_SELECT,
	CUSTOM_CODE_LAYER_TAG,
	CUSTOM_CODE_UNUSED_1,
	CUSTOM_CODE_UNUSED_2,
	CUSTOM_CODE_UNUSED_3,
	CUSTOM_CODE_ACTIVATION_KEY,
	CUSTOM_CODE_PRE_ENABLE,
	CUSTOM_CODE_OVER_CLOCK,
	CUSTOM_CODE_REFRESH_RATE };

enum{		// link level
	kLink_Master = 0,
	kLink_Link,
	kLink_SubLink };

enum{		// address read from
	kReadFrom_Address = 0,			// $xxxx
	kReadFrom_Index,				// $Vx
	kReadFrom_Variable };			// Vx

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
	kOperation_Unused4,
	kOperation_Unused5,
	kOperation_Unused6,
	kOperation_None
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
	kLocation_Unused5,
	kLocation_Unused6,
	kLocation_Unused7
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

enum		// memory accessor parameters
{
	kMemoryLocation_DirectAccess = 0,
	kMemoryLocation_DataSpace,
	kMemoryLocation_IOSpace,
	kMemoryLocation_OpbaseRAM
};

enum		// action flags
{
	/* set if this code is not the newest format */
	kActionFlag_OldFormat =			1 << 0,

	/* set if this code doesn't need to read/write a value */
	kActionFlag_NoAction =			1 << 1,

	/* set if this code is custom cheat */
	kActionFlag_Custom =			1 << 2,

	/* set for one shot cheats after the operation is performed */
	kActionFlag_OperationDone =		1 << 3,

	/* set if the code is memory writing type (Write, IWrite, RWrite, CWrite, CBit, PDWWrite) */
	kActionFlag_MemoryWrite =		1 << 4,

	/* set if the extend data field is being used by index adress reading (IWrite, Move) */
	kActionFlag_IndexAddress =		1 << 5,

	/* set if the code uses limited mask which divids data field to 16-bit data and 16-bit mask (IWrite, CBit) */
	kActionFlag_LimitedMask =		1 << 6,

	/* set if the code needs to check the condition (CWrite, CBit, Branch, Popup) */
	kActionFlag_CheckCondition =	1 << 7,

	/* set if the code needs to repeat writing (RWrite) */
	kActionFlag_Repeat =			1 << 8,

	/* set if the extendData field is being used by 2nd data (PDWWrite) */
	kActionFlag_PDWWrite =			1 << 9,

	/* set if the lastValue field contains valid data and can be restored if needed */
	kActionFlag_LastValueGood =		1 << 10,

	/* set after value changes from prefill value */
	kActionFlag_PrefillDone =		1 << 11,

	/* set after prefill value written */
	kActionFlag_PrefillWritten =	1 << 12,

	/* set if the code is a label for label-selection code */
	kActionFlag_IsLabel =			1 << 13,

	/* set when read/write a data after 1st read/write and clear after 2nd read/write (PDWWrite) */
	kActionFlag_IsFirst =			1 << 14,

	/* masks */
	kActionFlag_StateMask =			kActionFlag_OperationDone |		/* used in reset_action() to clear specified flags */
									kActionFlag_LastValueGood |
									kActionFlag_PrefillDone |
									kActionFlag_PrefillWritten,
	kActionFlag_InfoMask =			kActionFlag_IndexAddress,		/* ??? unused ??? */
	kActionFlag_PersistentMask =	kActionFlag_OldFormat |			/* used in update_cheat_info() to clear other flags */
									kActionFlag_LastValueGood
};

enum		// entry flags
{
	/* true when the cheat is active */
	kCheatFlag_Active =					1 << 0,

	/* true if the cheat is entirely one shot */
	kCheatFlag_OneShot =				1 << 1,

	/* true if the cheat is entirely null (ex. a comment) */
	kCheatFlag_Null =					1 << 2,

	/* true if the cheat is multi-comment code */
	kCheatFlag_ExtendComment =			1 << 3,

	/* true if the cheat is specified separator */
	kCheatFlag_Separator =				1 << 4,

	/* true if the cheat contains a user-select element */
	kCheatFlag_UserSelect =				1 << 5,

	/* true if the cheat is a select cheat */
	kCheatFlag_Select =					1 << 6,

	/* true if the cheat is layer index cheat */
	kCheatFlag_LayerIndex =				1 << 7,

	/* true if selected code is layer index cheat */
	kCheatFlag_LayerSelected =			1 << 8,

	/* true if selected code requests left/right arrow for sub item */
	kCheatFlag_RequestArrow =			1 << 9,

	/* true if the cheat has been assigned an 1st activation key */
	kCheatFlag_HasActivationKey1 =		1 << 10,

	/* true if the cheat has been assigned an 2nd activation key */
	kCheatFlag_HasActivationKey2 =		1 << 11,

	/* true if the cheat is not the newest format */
	kCheatFlag_OldFormat =				1 << 12,

	/* true if wrong cheat code */
	kCheatFlag_HasWrongCode =			1 << 13,

	/* true if the cheat has been edited or is a new cheat
       checked at auto-save then save the code if true */

	kCheatFlag_Dirty =					1 << 14,

	/* masks */
	kCheatFlag_StateMask =			kCheatFlag_Active,					// used in reset_action() and deactivate_cheat() to clear specified flag
	kCheatFlag_InfoMask =			kCheatFlag_OneShot |				// used in update_cheat_info() to detect specified flags
									kCheatFlag_Null |
									kCheatFlag_ExtendComment |
									kCheatFlag_Separator |
									kCheatFlag_UserSelect |
									kCheatFlag_Select |
									kCheatFlag_LayerIndex |
									kCheatFlag_OldFormat,
	kCheatFlag_PersistentMask =		kCheatFlag_Active |					// used in update_cheat_info() to extract specified flags
									kCheatFlag_HasActivationKey1 |
									kCheatFlag_HasActivationKey2 |
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

/********** SEARCH REGION **********/

enum		// flags
{
	/* true if enabled for search */
	kRegionFlag_Enabled =			1 << 0,

	/* true if this region has an error */
	kRegionFlag_HasError =			1 << 1,

	/* true if the memory region has no mapped memory and uses a memory handler (unused?) */
/*  kRegionFlag_UsesHandler = */
};

enum		// types
{
	kRegionType_CPU = 0,
	kRegionType_Memory
};

enum		/* search speed */
{
	SEARCH_SPEED_FAST = 0,		/* RAM + some banks */
	SEARCH_SPEED_MEDIUM,		/* RAM + BANKx */
	SEARCH_SPEED_SLOW,			/* all memory areas except ROM, NOP, and custom handlers */
	SEARCH_SPEED_VERY_SLOW,		/* all memory areas except ROM and NOP */
	SEARCH_SPEED_ALL_MEMORY,	/* entire CPU address space */
	SEARCH_SPEED_USER_DEFINED,	/* user defined region */

	SEARCH_SPEED_MAX
};

/********** SEARCH INFO **********/

enum		// operands
{
	kSearchOperand_Current = 0,
	kSearchOperand_Previous,
	kSearchOperand_First,
	kSearchOperand_Value,

	kSearchOperand_Max = kSearchOperand_Value
};

enum		// length
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

enum		// search menu
{
	kSearchBox_Minimum = 0,
	kSearchBox_Standard,
	kSearchBox_Advanced
};

enum		/* format level */
{
	FORMAT_NEW = 1,
	FORMAT_STANDARD,
	FORMAT_OLD
};

enum		/* disposition for resizing a list */
{
	REQUEST_DISPOSE = 0,
	NO_DISPOSE
};

enum		/* database open flags */
{
	DATABASE_LOAD = 0,
	DATABASE_SAVE
};

enum		/* database load flags */
{
	LOAD_CHEAT_OPTION	= 1,
	LOAD_CHEAT_CODE		= 1 << 1,
	LOAD_USER_REGION	= 1 << 2
};

enum		/* message type */
{
	CHEAT_MESSAGE_NONE = 0,				/* this message should be unused */
	CHEAT_MESSAGE_RELOAD_CHEAT_OPTION,
	CHEAT_MESSAGE_RELOAD_CHEAT_CODE,
	CHEAT_MESSAGE_RELOAD_USER_REGION,
	CHEAT_MESSAGE_FAILED_TO_LOAD_DATABASE,
	CHEAT_MESSAGE_CHEAT_FOUND,
	CHEAT_MESSAGE_ONE_CHEAT_FOUND,
	CHEAT_MESSAGE_SUCCEEDED_TO_SAVE,
	CHEAT_MESSAGE_FAILED_TO_SAVE,
	CHEAT_MESSAGE_NO_SUPPORTED_OLD_FORMAT,
	CHEAT_MESSAGE_ALL_CHEATS_SAVED,
	CHEAT_MESSAGE_ACTIVATION_KEY_SAVED,
	CHEAT_MESSAGE_NO_ACTIVATION_KEY,
	CHEAT_MESSAGE_PRE_ENABLE_SAVED,
	CHEAT_MESSAGE_SUCCEEDED_TO_ADD,
	CHEAT_MESSAGE_FAILED_TO_ADD,
	CHEAT_MESSAGE_SUCCEEDED_TO_DELETE,
	CHEAT_MESSAGE_FAILED_TO_DELETE,
	CHEAT_MESSAGE_NO_SEARCH_REGION,
	CHEAT_MESSAGE_RESTORE_VALUE,
	CHEAT_MESSAGE_NO_OLD_VALUE,
	CHEAT_MESSAGE_INITIALIZE_MEMORY,
	CHEAT_MESSAGE_INVALIDATE_REGION,
	CHEAT_MESSAGE_FAILED_TO_ALLOCATE,
	CHEAT_MESSAGE_WRONG_CODE,

	CHEAT_MESSAGE_MAX
};

enum{		// error flags
	kErrorFlag_InvalidLocationType		= 1,		// old
	kErrorFlag_InvalidOperation			= 1 << 1,	// old
	kErrorFlag_InvalidCodeType			= 1 << 2,	// new
	kErrorFlag_InvalidCondition			= 1 << 3,	// new
	kErrorFlag_InvalidCodeOption		= 1 << 4,	// new
	kErrorFlag_ConflictedExtendField	= 1 << 5,	// old
	kErrorFlag_InvalidRange				= 1 << 6,
	kErrorFlag_NoRestorePreviousValue	= 1 << 7,
	kErrorFlag_NoLabel					= 1 << 8,
	kErrorFlag_ConflictedSelects		= 1 << 9,
	kErrorFlag_InvalidDataField			= 1 << 10,
	kErrorFlag_InvalidExtendDataField	= 1 << 11,
	kErrorFlag_ForbittenVariable		= 1 << 12,	// new
	kErrorFlag_NoSelectableValue		= 1 << 13,
	kErrorFlag_InvalidLimitedMaskSize	= 1 << 14,	// new
	kErrorFlag_NoRepeatCount			= 1 << 15,	// new
	kErrorFlag_UndefinedAddressRead		= 1 << 16,	// new
	kErrorFlag_AddressVariableOutRange	= 1 << 17,	// new
	kErrorFlag_DataVariableOutRange		= 1 << 18,	// new
	kErrorFlag_OutOfCPURegion			= 1 << 19,
	kErrorFlag_InvalidCPURegion			= 1 << 20,
	kErrorFlag_InvalidAddressSpace		= 1 << 21,	// new
	kErrorFlag_RegionOutOfRange			= 1 << 22,
	kErrorFlag_InvalidCustomCode		= 1 << 23,	// new

	kErrorFlag_Max						= 24 };

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/********** ACTION **********/

typedef struct _cheat_action cheat_action;
struct _cheat_action
{
	UINT32	type;					/* packed several information (cpu/region, address size, operation and parameters and so on) */
	UINT8	region;					/* the number of cpu or region. in case of cpu, it has a kind of an address space together */
	UINT32	address;				/* address. it is changeable by RWrite in the cheat core */
	UINT32	original_address;		/* address. it is for edit/view/save */
	UINT32	data;					/* data. it is changeable by valuse selector in the cheat core */
	UINT32	original_data;			/* data. it is for edit/view/save */
	UINT32	extend_data;			/* extend parameter. it is different using by each operations */

	INT32	frame_timer;			/* timer for delay/keep */
	UINT32	*last_value;			/* back up value before cheating to restore value. PDWWrite uses 2 and RWrite uses about a counter */

	UINT32	flags;					/* internal flags */

	UINT8	**cached_pointer;		/* pointer to specified address space or mapped memory */
	UINT32	cached_offset;			/* offset for mapped memory */

	char	*optional_name;			/* label for label selection or popup */
};

/********** ENTRY **********/

typedef struct _cheat_entry cheat_entry;
struct _cheat_entry
{
	char			*name;					/* name to display in list menu */
	char			*comment;				/* simple comment */

	INT32			action_list_length;		/* total codes of cheat action. NOTE : minimum length is 1 and 0 is ERROR */
	cheat_action	*action_list;			/* pointer to cheat action */

	int				activation_key_1;		/* activation key index for 1st key */
	int				activation_key_2;		/* activation key index for 2nd key */

	int				selection;				/* for label-select to set selected label number */
	int				label_index_length;		/* total tables of index table. NOTE : minimum length is 1 and 0 is ERROR */
	int				*label_index;			/* pointer of label index table */

	int				layer_index;			/* layer index for enable/disable menu */
	UINT32			flags;					/* internal flags */
};

/********** WATCH **********/

typedef struct _watch_info watch_info;
struct _watch_info
{
	UINT32		address;			/* address */
	UINT8		cpu;				/* target cpu or region */
	UINT8		num_elements;		/* length of this watchpoint */
	UINT8		element_bytes;		/* address size of this watchpoint */
	UINT8		label_type;			/* header label */
	UINT8		display_type;		/* type of value display */
	UINT8		skip;				/* address skip */
	UINT8		elements_per_line;	/* max watchpoints in the same line */
	INT8		add_value;			/* add value */
	INT8		address_shift;		/* address shift */
	INT8		data_shift;			/* data shift */
	UINT32		xor;				/* xor */

	float		x, y;				/* position */

	cheat_entry	*linked_cheat;		/* relative cheat_entry */

	char		label[256];			/* name of label */
};

/********** REGION **********/

typedef struct _search_region search_region;
struct _search_region
{
	UINT32	address;			/* address */
	UINT32	length;				/* total length */

	UINT8	target_type;		/* flag for cpu or region */
	UINT8	target_idx;			/* target cpu/region */

	UINT8	flags;				/* internal flags */

	UINT8	**cached_pointer;	/* pointer to read from memory */
	const address_map_entry
			*write_handler;		/* pointer of write handler in address map */

	UINT8	*first;				/* first value in searching */
	UINT8	*last;				/* current value in searching */

	UINT8	*status;

	UINT8	*backup_last;		/* previous value in searching */
	UINT8	*backup_status;		/* flag of backup */

	char	name[64];			/* name of this region */

	UINT32	num_results;		/* total results */
	UINT32	old_num_results;	/* previous total results */
};

/********** SEARCH **********/

typedef struct _old_search_options old_search_options;
struct _old_search_options
{
	INT8	comparison;
	UINT32	value;
	INT8	sign;
	UINT32	delta;
};

typedef struct _search_info search_info;
struct _search_info
{
	INT32				region_list_length;
	search_region		*region_list;

	char				*name;			/* search info name used in select_region() */

	INT8				bytes;			/* 0 = 1, 1 = 2, 2 = 3, 3 = 4, 4 = bit */
	UINT8				swap;
	UINT8				sign;
	INT8				lhs;
	INT8				rhs;
	INT8				comparison;

	UINT8				target_type;		/* is cpu or region? */
	UINT8				target_idx;			/* index for cpu or region */

	UINT32				value;

	INT8				search_speed;

	UINT32				num_results;
	UINT32				old_num_results;

	INT32				current_region_idx;
	INT32				current_results_page;

	UINT8				backup_valid;

	old_search_options	old_options;
};

/********** CPU **********/

typedef struct _cpu_region_info cpu_region_info;
struct _cpu_region_info
{
	UINT8	type;					/* cpu or region type */
	UINT8	data_bits;				/* data bits */
	UINT8	address_bits;			/* address bits */
	UINT8	address_chars_needed;	/* length of address */
	UINT32	address_mask;			/* address mask */
	UINT8	endianness;				/* endianness */
	UINT8	address_shift;			/* address shift */
};

/********** STRINGS **********/

typedef struct _menu_string_list menu_string_list;
struct _menu_string_list
{
	const char	**main_list;	/* main item field in menu items */
	const char	**sub_list;		/* sub item field in menu items */
	char		*flag_list;		/* flag of highlight for sub menu field */

	char	**main_strings;		/* strings buffer 1 */
	char	**sub_strings;		/* strings buffer 2 */

	char	*buf;				/* internal buffer? (only used in rebuild_string_tables()) */

	UINT32	length;				/* number of total menu items */
	UINT32	num_strings;		/* number of strings buffer */
	UINT32	main_string_length;	/* length of string in strings buffer 1 */
	UINT32	sub_string_length;	/* length of string in strings buffer 2 */
};

/********** MENUS **********/

/* NOTE : _cheat_menu_item_info is used in the edit menu right now */
typedef struct _cheat_menu_item_info cheat_menu_item_info;
struct _cheat_menu_item_info
{
	UINT32	sub_cheat;		/* index of an item */
	UINT32	field_type;		/* field type of an item */
	UINT32	extra_data;		/* it stores an extra data but NOT read (so I don't know how to use)*/
};

typedef struct _cheat_menu_stack cheat_menu_stack;
typedef int (*cheat_menu_handler)(running_machine *machine, cheat_menu_stack *menu);
struct _cheat_menu_stack
{
	cheat_menu_handler	handler;		/* menu handler for cheat */
	cheat_menu_handler	return_handler;	/* menu handler to return menu */
	int					sel;			/* current cursor position */
	int					pre_sel;		/* selected item in previous menu */
	UINT8				first_time;		/* flags to first setting (1 = first, 0 = not first) */
};

/********** FORMAT **********/
typedef const struct _cheat_format cheat_format;
struct _cheat_format
{
	const char		*format_string;			/* string templete of a format */
	UINT8			arguments_matched;		/* valid arguments of a format */
	UINT8			type_matched;			/* valid length of a type field */
	UINT8			data_matched;			/* valid length of a data/extend data field */
	UINT8			comment_matched;		/* valid arguments to add comment */
};

static const struct _cheat_format cheat_format_table[] =
{
		/* option - :_command:TYPE */
	{	":_command:%[^:\n\r]",																1,	8,	0,	0	},
#ifdef MESS
		/* code (new) - :MACHINE_NAME::CRC::TYPE::ADDRESS::DATA::EXTEND_DATA:(DESCRIPTION:COMMENT) */
	{	":%8[^:\n\r]::%8X::%10[^:\n\r]::%X::%8[^:\n\r]::%8[^:\n\r]:%[^:\n\r]:%[^:\n\r]",	6,	10,	8,	8	},
		/* code (old) - :MACHINE_NAME:CRC:TYPE:ADDRESS:DATA:EXTEND_DATA:(DESCRIPTION:COMMENT) */
	{	":%8s:8X:%8[^:\n\r]:%X:%8[^:\n\r]:%8[^:\n\r]:%[^:\n\r]:%[^:\n\r]",					6,	8,	8,	0	},
		/* code (older) - :MACHINE_NAME:CRC:CPU:ADDRESS:DATA:TYPE:(DESCRIPTION:COMMENT) */
	{	"%8[^:\n\r]:%8X:%d:%X:%X:%d:%[^:\n\r]:%[^:\n\r]",									5,	0,	0,	8	},
		/* user region - :MACHINE_NAME:CRC:CPU:ADDRESS_SPACE:START_ADDRESS:END_ADDRESS:STATUS:(DESCRIPTION) */
	{	":%8[^:\n\r]:%8X:%2X:%2X:%X:%X:%1X:%[^:\n\r]",										7,	0,	0,	0	},
#else
		/* code (new) - :GAME_NAME::TYPE::ADDRESS::DATA::EXTEND_DATA:(DESCRIPTION:COMMENT) */
	{	":%8[^:\n\r]::%10[^:\n\r]::%X::%8[^:\n\r]::%8[^:\n\r]:%[^:\n\r]:%[^:\n\r]",			5,	10,	8,	7	},
		/* code (old) - :GAME_NAME:TYPE:ADDRESS:DATA:EXTEND_DATA:(DESCRIPTION:COMMENT) */
	{	":%8s:%8[^:\n\r]:%X:%8[^:\n\r]:%8[^:\n\r]:%[^:\n\r]:%[^:\n\r]",						5,	8,	8,	0	},
		/* code (older) - :GAME_NAME:CPU:ADDRESS:DATA:TYPE:(DESCRIPTION:COMMENT) */
	{	"%8[^:\n\r]:%d:%X:%X:%d:%[^:\n\r]:%[^:\n\r]",										4,	0,	0,	7	},
		/* user region - :GAME_NAME:CPU:ADDRESS_SPACE:START_ADDRESS:END_ADDRESS:STATUS:(DESCRIPTION) */
	{	":%8[^:\n\r]:%2X:%2X:%X:%X:%1X:%[^:\n\r]",											6,	0,	0,	0	},
#endif	/* MESS */
	/* end of table */
	{	0	}
};

/***************************************************************************
    EXPORTED VARIABLES
***************************************************************************/

static const char			*cheat_file = NULL;

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static emu_timer			*periodic_timer;

static cheat_entry			*cheat_list;
static INT32				cheat_list_length;

static watch_info			*watch_list;
static INT32				watch_list_length;

static search_info			*search_list;
static INT32				search_list_length;
static INT32				current_search_idx;

static cpu_region_info		cpu_info_list[MAX_CPU];
static cpu_region_info		region_info_list[REGION_LIST_LENGTH];

static int					found_database;
static int					cheats_disabled;
static int					watches_disabled;

static int					fullMenuPageHeight;
static int					visible_items;

static char					main_database_name[CHEAT_FILENAME_MAX_LEN + 1];

static menu_string_list		menu_strings;

static cheat_menu_item_info	* menu_item_info;
static INT32				menuItemInfoLength = 0;

static UINT8				editActive;
static INT8					editCursor;

static INT8					stack_index;
static cheat_menu_stack		menu_stack[CHEAT_MENU_DEPTH];

static int					cheat_variable[VARIABLE_MAX_ARRAY] = { 0 };

static UINT32				cheat_options;
static UINT32				driverSpecifiedFlag;
static UINT8				message_type;
static UINT8				message_timer;

static cpu_region_info raw_cpu_info =
{
	0,			/* type */
	8,			/* data_bits */
	8,			/* address_bits */
	1,			/* address_chars_needed */
	CPU_IS_BE,	/* endianness */
	0,			/* address_shift */
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

static const UINT32 kIncrementValueTable[] =
{
	0x00000001,
	0x00000010,
	0x00000100,
	0x00001000,
	0x00010000,
	0x00100000,
	0x01000000,
	0x10000000
};

static const UINT32 kIncrementMaskTable[] =
{
	0x0000000F,
	0x000000F0,
	0x00000F00,
	0x0000F000,
	0x000F0000,
	0x00F00000,
	0x0F000000,
	0xF0000000
};

static const UINT32 kIncrementDecTable[] =
{
	0x00000009,
	0x00000090,
	0x00000900,
	0x00009000,
	0x00090000,
	0x00900000,
	0x09000000,
	0x90000000
};

static const char *const kRegionNames[] = {
	"INVALID",
	"CPU1",		"CPU2",		"CPU3",		"CPU4",		"CPU5",		"CPU6",		"CPU7",		"CPU8",		/* 01-08    [01-08] : CPU */
	"GFX1",		"GFX2",		"GFX3",		"GFX4",		"GFX5",		"GFX6",		"GFX7",		"GFX8",		/* 09-16    [08-10] : GFX */
	"PROMS",																						/* 17       [11]    : PROMS */
	"SOUND1",	"SOUND2",	"SOUND3",	"SOUND4",	"SOUND5",	"SOUND6",	"SOUND7",	"SOUND8",	/* 18-25    [12-19] : SOUND */
	"USER1",	"USER2",	"USER3",	"USER4",	"USER5",	"USER6",	"USER7",	"USER8",	/* 26-45    [1A-2D] : USER */
	/* USER9 - PLDS are undefined in old format */
	"USER9",	"USER10",	"USER11",	"USER12",	"USER13",	"USER14",	"USER15",	"USER16",
	"USER17",	"USER18",	"USER19",	"USER20",
	"DISKS",																						/* 46       [2E]    : DISKS */
	"PLDS" };																						/* 47       [2F]    : PLDS */

static const char *const kNumbersTable[] = {
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",
	"8",	"9",	"10",	"11",	"12",	"13",	"14",	"15",
	"16",	"17",	"18",	"19",	"20",	"21",	"22",	"23",
	"24",	"25",	"26",	"27",	"28",	"29",	"30",	"31" };

static const char *const kByteSizeStringList[] =
{
	"8 Bit",
	"16 Bit",
	"24 Bit",
	"32 Bit"
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

static const char *const CHEAT_MESSAGE_TABLE[] =
{
	"INVALID MESSAGE!",							/* this message should be unused */
	"cheat option reloaded",					/* RELOAD_CHEAT_OPTION */
	"cheat code reloaded",						/* RELOAD_CHEAT_CODE */
	"user defined search region reloaded",		/* RELOAD_USER_REGION */
	"failed to load database!",					/* FAILED_TO_LOAD_DATABASE */
	"cheats found",								/* CHEAT_FOUND */
	"1 result found, added to list",			/* ONE_CHEAT_FOUND */
	"succeeded to save",						/* SUCCEEDED_TO_SAVE */
	"failed to save!",							/* FAILED_TO_SAVE */
	"unsupported old/older format!",			/* NO_SUPPORTED_OLD_FORMAT */
	"cheats saved",								/* ALL_CHEATS_SAVED */
	"activation key saved",						/* ACTIVATION_KEY_SAVED */
	"no activation key!",						/* NO_ACTIVATION_KEY */
	"pre-enable saved",							/* PRE_ENABLE_SAVED */
	"succeeded to add",							/* SUCCEEDED_TO_ADD */
	"failed to add!",							/* FAILED_TO_ADD */
	"succeeded to delete",						/* SUCCEEDED_TO_DELETE */
	"failed to delete!",						/* FAILED_TO_DELETE */
	"no search region!",						/* NO_SEARCH_REGION */
	"values restored",							/* RESTORE_VALUE */
	"there are no old values!",					/* NO_OLD_VALUE */
	"saved all memory regions",					/* INITIALIZE_MEMOY */
	"region invalidated remains results are",	/* INVALIDATE_REGION */
	"failed to allocate memory!",				/* FAILED_TO_ALLOCATE */
	"found wrong code!"							/* WRONG_CODE */
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( cheat_periodic );
static void 	cheat_exit(running_machine *machine);

/********** SPECIAL KEY HANDLING **********/
static int		ShiftKeyPressed(void);
static int		ControlKeyPressed(void);
static int		AltKeyPressed(void);

static int		ui_pressed_repeat_throttle(running_machine *machine, int code, int base_speed);

/********** KEY INPUT **********/
static int		read_hex_input(void);
#if 0
static char		*DoDynamicEditTextField(char * buf);
#endif
static void		DoStaticEditTextField(char * buf, int size);
static UINT32	do_edit_hex_field(running_machine *machine, UINT32 data);
static UINT32	DoEditHexFieldSigned(UINT32 data, UINT32 mask);
static INT32	DoEditDecField(INT32 data, INT32 min, INT32 max);
static UINT32	DoIncrementHexField(UINT32 data, UINT8 digits);
static UINT32	DoDecrementHexField(UINT32 data, UINT8 digits);
static UINT32	DoIncrementHexFieldSigned(UINT32 data, UINT8 digits, UINT8 bytes);
static UINT32	DoDecrementHexFieldSigned(UINT32 data, UINT8 digits, UINT8 bytes);
static UINT32	DoIncrementDecField(UINT32 data, UINT8 digits);
static UINT32	DoDecrementDecField(UINT32 data, UINT8 digits);

/********** VALUE CONVERTER **********/
static UINT32	BCDToDecimal(UINT32 value);
static UINT32	DecimalToBCD(UINT32 value);

/********** STRINGS **********/
static void		rebuild_string_tables(void);
static void		request_strings(UINT32 length, UINT32 num_strings, UINT32 main_string_length, UINT32 sub_string_length);
static void		init_string_table(void);
static void		free_string_table(void);

static char		*create_string_copy(char *buf);

/********** MENU EXPORTER **********/
static void		old_style_menu(const char **items, const char **sub_items, char *flag, int selected, int arrowize_subitem);

/********** MENU STACK **********/
static void		init_cheat_menu_stack(void);
static void		cheat_menu_stack_push(cheat_menu_handler new_handler, cheat_menu_handler ret_handler, int selection);
static void		cheat_menu_stack_pop(void);
static void		free_cheat_menu_stack(void);

/********** ADDITIONAL MENU FOR CHEAT **********/
static int		user_select_value_menu(running_machine *machine, cheat_menu_stack *menu);
static int		user_select_label_menu(running_machine *machine, cheat_menu_stack *menu);
static int		comment_menu(running_machine *machine, cheat_menu_stack *menu);
static int		extend_comment_menu(running_machine *machine, cheat_menu_stack *menu);

/********** CHEAT MENU **********/
static int		cheat_main_menu(running_machine *machine, cheat_menu_stack *menu);
static int		enable_disable_cheat_menu(running_machine *machine, cheat_menu_stack *menu);
static int		add_edit_cheat_menu(running_machine *machine, cheat_menu_stack *menu);
static int		command_add_edit_menu(running_machine *machine, cheat_menu_stack *menu);
static int		edit_cheat_menu(running_machine *machine, cheat_menu_stack *menu);
static int		view_cheat_menu(running_machine *machine, cheat_menu_stack *menu);
static int		analyse_cheat_menu(running_machine *machine, cheat_menu_stack *menu);

static int		search_minimum_menu(running_machine *machine, cheat_menu_stack *menu);
static int		search_standard_menu(running_machine *machine, cheat_menu_stack *menu);
static int		search_advanced_menu(running_machine *machine, cheat_menu_stack *menu);
static int		select_search_region_menu(running_machine *machine, cheat_menu_stack *menu);
static int		view_search_result_menu(running_machine *machine, cheat_menu_stack *menu);

static int		choose_watch_menu(running_machine *machine, cheat_menu_stack *menu);
static int		command_watch_menu(running_machine *machine, cheat_menu_stack *menu);
static int		edit_watch_menu(running_machine *machine, cheat_menu_stack *menu);

static int		select_option_menu(running_machine *machine, cheat_menu_stack *menu);
static int		select_search_menu(running_machine *machine, cheat_menu_stack *menu);

static int		command_cheat_menu(running_machine *machine, cheat_menu_stack *menu);

#ifdef MAME_DEBUG
static int		check_activation_key_code_menu(running_machine *machine, cheat_menu_stack *menu);
static int		view_cpu_region_info_list_menu(running_machine *machine, cheat_menu_stack *menu);
static int		debug_cheat_menu(running_machine *machine, cheat_menu_stack *menu);
#endif

/********** ENTRY LIST **********/
static void		resize_cheat_list(UINT32 new_length, UINT8 dispose);

static void		add_cheat_before(UINT32 idx);
static void		delete_cheat_at(UINT32 idx);

static void		dispose_cheat(cheat_entry *entry);
static cheat_entry
				*get_new_cheat(void);

/********** ACTION LIST **********/
static void		resize_cheat_action_list(cheat_entry *entry, UINT32 new_length, UINT8 dispose);
#if 0
static void		AddActionBefore(CheatEntry * entry, UINT32 idx);
static void		DeleteActionAt(CheatEntry * entry, UINT32 idx);
#endif
static void		dispose_action(cheat_action *action);

/********** WATCH LIST **********/
static void		init_watch(watch_info *info, UINT32 idx);
static void		resize_watch_list(UINT32 new_length, UINT8 dispose);

static void		add_watch_before(UINT32 idx);
static void		delete_watch_at(UINT32 idx);
static void		dispose_watch(watch_info *watch);

static watch_info
				*get_unused_watch(void);

static void		add_cheat_from_watch(running_machine *machine, watch_info *watch);
static void		add_cheat_from_watch_as_watch(running_machine *machine, cheat_entry *entry, watch_info *watch);

static void		reset_watch(watch_info *watch);

/********** SEARCH LIST **********/
static void		resize_search_list(UINT32 newLength);
#if 0
static void		resize_search_list_no_dispose(UINT32 new_length);
static void		add_search_before(UINT32 idx);
static void		delete_search_at(UINT32 idx);
#endif
static void		init_search(search_info *info);

static void		dispose_search_reigons(search_info *info);
static void		dispose_search(search_info *info);
static search_info
				*get_current_search(void);

static void		fill_buffer_from_region(search_region *region, UINT8 *buf);
static UINT32	read_region_data(search_region *region, UINT32 offset, UINT8 size, UINT8 swap);

static void		backup_search(search_info *info);
static void		restore_search_backup(search_info *info);
static void		backup_region(search_region *region);
static void		restore_region_backup(search_region *region);
static UINT8	default_enable_region(running_machine *machine, search_region *region, search_info *info);
static void		set_search_region_default_name(search_region *region);
static UINT8	is_search_region_in_range(UINT8 cpu, UINT32 length);
static void		allocate_search_regions(search_info *info);
static void		build_search_regions(running_machine *machine, search_info *info);

/********** LOADER **********/
static int		convert_older_code(int code, int cpu, int *data, int *extend_data);
static int		convert_to_new_code(cheat_action *action);
static void		handle_local_command_cheat(running_machine *machine, int cpu, int type, int address, int data, int extend_data, UINT8 format);

static UINT8	open_cheat_database(mame_file **the_file, char *file_name, UINT8 flag);

static void		load_cheat_option(char *file_name);
static void		load_cheat_code(running_machine *machine, char *file_name);
static void		load_user_defined_search_region(running_machine *machine, char *file_name);
static void		load_cheat_database(running_machine *machine, UINT8 flags);
static void		reload_cheat_database(running_machine *machine);

static void		dispose_cheat_database(running_machine *machine);

/********** SAVER **********/
static void		save_cheat_code(running_machine *machine, cheat_entry *entry);
static void		save_activation_key(running_machine *machine, cheat_entry *entry, int entryIndex);
static void		save_pre_enable(running_machine *machine, cheat_entry *entry, int entry_index);
static void		save_cheat_options(void);
static void		save_description(running_machine *machine);
static void		save_raw_code(running_machine *machine);
static void		do_auto_save_cheats(running_machine *machine);

/********** CODE ADDITION **********/
static void		add_cheat_from_result(running_machine *machine, search_info *search, search_region *region, UINT32 address);
static void		add_cheat_from_first_result(running_machine *machine, search_info *search);
static void		add_watch_from_result(search_info *search, search_region *region, UINT32 address);

/********** SEARCH **********/
static UINT32	search_sign_extend(search_info *search, UINT32 value);
static UINT32	read_search_operand(UINT8 type, search_info *search, search_region *region, UINT32 address);
static UINT32	read_search_operand_bit(UINT8 type, search_info *search, search_region *region, UINT32 address);
static UINT8	do_search_comparison(search_info *search, UINT32 lhs, UINT32 rhs);
static UINT32	do_search_comparison_bit(search_info *search, UINT32 lhs, UINT32 rhs);
#if 0
static UINT8  is_region_offset_valid(search_info *search, search_region *region, UINT32 offset);
#else
#define is_region_offset_valid is_region_offset_valid_bit /* ????? */
#endif
static UINT8	is_region_offset_valid_bit(search_info *search, search_region *region, UINT32 offset);
static void		invalidate_region_offset(search_info *search, search_region *region, UINT32 offset);
static void		invalidate_region_offset_bit(search_info *search, search_region *region, UINT32 offset, UINT32 invalidate);
static void		invalidate_entire_region(search_info *search, search_region *region);

static void		init_new_search(search_info *search);
static void		update_search(search_info *search);

static void		do_search(search_info *search);

/********** MEMORY ACCESSOR **********/
static UINT8	**look_up_handler_memory(UINT8 cpu, UINT32 address, UINT32 *out_relative_address);
static UINT8	**get_memory_region_base_pointer(UINT8 cpu, UINT8 space, UINT32 address);

static UINT32	do_cpu_read(UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap);
static UINT32	do_memory_read(UINT8 *buf, UINT32 address, UINT8 bytes, UINT8 swap, cpu_region_info *info);
static void		do_cpu_write(UINT32 data, UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap);
static void		do_memory_write(UINT32 data, UINT8 *buf, UINT32 address, UINT8 bytes, UINT8 swap, cpu_region_info *info);

static UINT32	read_data(running_machine *machine, cheat_action *action);
static void		write_data(running_machine *machine, cheat_action *action, UINT32 data);

/********** WATCH **********/
static void		watch_cheat_entry(cheat_entry *entry, UINT8 associate);
static void		add_action_watch(cheat_action *action, cheat_entry *entry);
static void		remove_associated_watches(cheat_entry *entry);

/********** ACTIVE/DEACTIVE ENTRY **********/
static void		reset_action(running_machine *machine, cheat_action *action);
static void		activate_cheat(running_machine *machine, cheat_entry *entry);
static void		restore_last_value(running_machine *machine, cheat_action *action);
static void		deactivate_cheat(running_machine *machine, cheat_entry *entry);
static void		temp_deactivate_cheat(running_machine *machine, cheat_entry *entry);

/********** OPERATION CORE **********/
static void		cheat_periodicOperation(running_machine *machine, cheat_action *action);
static UINT8	cheat_periodicCondition(running_machine *machine, cheat_action *action);
static int		cheat_periodicAction(running_machine *machine, cheat_action *action, int selection);
static void		cheat_periodicEntry(running_machine *machine, cheat_entry *entry);

/********** CONFIGURE ENTRY **********/
static void		update_all_cheat_info(running_machine *machine);
static void		update_cheat_info(running_machine *machine, cheat_entry *entry, UINT8 is_load_time);
static UINT32	analyse_code_format(running_machine *machine, cheat_entry *entry, cheat_action *action);
static void		check_code_format(running_machine *machine, cheat_entry *entry);
static void		build_label_index_table(cheat_entry *entry);
static void		set_layer_index(void);

/********** OTHER STUFF **********/
static void		display_cheat_message(void);
static UINT8	get_address_length(UINT8 region);
static char		*get_region_name(UINT8 region);
static void		build_cpu_region_info_list(running_machine *machine);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*---------------
  get_cpu_info
---------------*/

INLINE cpu_region_info *get_cpu_info(UINT8 cpu)
{
	if(VALID_CPU(EXTRACT_FIELD(cpu, CPUIndex)))
		return &cpu_info_list[cpu];

	return NULL;
}

/*------------------
  get_region_info
------------------*/

INLINE cpu_region_info *get_region_info(UINT8 region)
{
	if(region >= REGION_CPU1 && region < REGION_MAX)
		return &region_info_list[region - REGION_INVALID];

	return NULL;
}

/*-------------------------
  get_cpu_or_region_info
-------------------------*/

INLINE cpu_region_info *get_cpu_or_region_info(UINT8 cpu_region)
{
	if(cpu_region < REGION_INVALID)
		return get_cpu_info(cpu_region);
	else
		return get_region_info(cpu_region);
}

/*-----------------
  cpu_needs_swap
-----------------*/

INLINE UINT8 cpu_needs_swap(UINT8 cpu)
{
	return cpu_info_list[cpu].endianness ^ 1;
}

/*--------------------
  region_needs_swap
--------------------*/

INLINE UINT8 region_needs_swap(UINT8 region)
{
	cpu_region_info *temp = get_region_info(region);

	if(temp)
		return temp->endianness ^ 1;

	return 0;
}

/*--------------
  SwapAddress
--------------*/

INLINE UINT32 SwapAddress(UINT32 address, UINT8 dataSize, cpu_region_info *info)
{
	switch(info->data_bits)
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

/*---------------------------------------------------------------------------------
  is_address_in_range - check the range of address for eeprom and non-cpu region
---------------------------------------------------------------------------------*/

INLINE int is_address_in_range(cheat_action *action, UINT32 length)
{
	return ((action->address + EXTRACT_FIELD(action->type, AddressSize) + 1) <= length);
}

/*----------
  DoShift
----------*/

INLINE UINT32 DoShift(UINT32 input, INT8 shift)
{
	if(shift > 0)
		return input >> shift;
	else
		return input << -shift;
}

/***************************************************************************
    KEY HANDLER
***************************************************************************/

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

#if OSD_READKEY_KLUDGE

static int ReadKeyAsync(int flush)
{
	int	code;

	if(flush)
	{
		/* check key input */
		while(input_code_poll_keyboard_switches(TRUE) != INPUT_CODE_INVALID) ;

		return 0;
	}

	while(1)
	{
		/* check pressed key */
		code = input_code_poll_keyboard_switches(FALSE);

		if(code == INPUT_CODE_INVALID)
		{
			return 0;
		}
		else if(code >= KEYCODE_A && code <= KEYCODE_Z)
		{
			if(ShiftKeyPressed())		return 'A' + (code - KEYCODE_A);
			else						return 'a' + (code - KEYCODE_A);
		}
		else if(code >= KEYCODE_0 && code <= KEYCODE_9)
		{
			if(ShiftKeyPressed())		return ")!@#$%^&*("[code - KEYCODE_0];
			else						return '0' + (code - KEYCODE_0);
		}
		else if(code >= KEYCODE_0_PAD && code <= KEYCODE_9_PAD)
		{
										return '0' + (code - KEYCODE_0_PAD);
		}
		else if(code == KEYCODE_TILDE)
		{
			if(ShiftKeyPressed())		return '~';
			else						return '`';
		}
		else if(code == KEYCODE_MINUS)
		{
			if(ShiftKeyPressed())		return '_';
			else						return '-';
		}
		else if(code == KEYCODE_EQUALS)
		{
			if(ShiftKeyPressed())		return '+';
			else						return '=';
		}
		else if(code == KEYCODE_BACKSPACE)
		{
										return 0x08;
		}
		else if(code == KEYCODE_OPENBRACE)
		{
			if(ShiftKeyPressed())		return '{';
			else						return '[';
		}
		else if(code == KEYCODE_CLOSEBRACE)
		{
			if(ShiftKeyPressed())		return '}';
			else						return ']';
		}
		else if(code == KEYCODE_COLON)
		{
			if(ShiftKeyPressed())		return ':';
			else						return ';';
		}
		else if(code == KEYCODE_QUOTE)
		{
			if(ShiftKeyPressed())		return '\"';
			else						return '\'';
		}
		else if(code == KEYCODE_BACKSLASH)
		{
			if(ShiftKeyPressed())		return '|';
			else						return '\\';
		}
		else if(code == KEYCODE_COMMA)
		{
			if(ShiftKeyPressed())		return '<';
			else						return ',';
		}
		else if(code == KEYCODE_STOP)
		{
			if(ShiftKeyPressed())		return '>';
			else						return '.';
		}
		else if(code == KEYCODE_SLASH)
		{
			if(ShiftKeyPressed())		return '?';
			else						return '/';
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

#endif /* OSD_READKEY_KLUDGE */

/*---------------------------------------------------
  ui_pressed_repeat_throttle - key repeat handling
---------------------------------------------------*/

static int ui_pressed_repeat_throttle(running_machine *machine, int code, int base_speed)
{
	int pressed = 0;
	const int delay_ramp_timer = 10;
	static int last_code = -1;
	static int last_speed = -1;
	static int increment_timer = 0;

	if(input_type_pressed(machine, code, 0))
	{
		if(last_code != code)
		{
			/* pressed different key */
			last_code = code;
			last_speed = base_speed;
			increment_timer = delay_ramp_timer * last_speed;
		}
		else
		{
			/* hold the same key */
			increment_timer--;

			if(increment_timer <= 0)
			{
				increment_timer = delay_ramp_timer * last_speed;

				last_speed /= 2;

				if(last_speed < 1)
					last_speed = 1;

				pressed = 1;
			}
		}
	}
	else
	{
		if(last_code == code)
			last_code = -1;
	}

	return input_ui_pressed_repeat(machine, code, last_speed);
}

/*-----------------------------------
  read_hex_input - check hex input
-----------------------------------*/

static int read_hex_input(void)
{
	int i;

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
#if 0
static char * DoDynamicEditTextField(char * buf)
{
	char code = osd_readkey_unicode(0) & 0xFF;

	if(code == 0x08)
	{
		/* backspace */
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
				size_t length = strlen(buf);

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
#endif
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
		/* back space */
		if(length > 0)
			buf[length - 1] = 0;
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

/*----------------------------------------------------------------------
  do_edit_hex_field - edit hex field with direct key input (unsigned)
----------------------------------------------------------------------*/

static UINT32 do_edit_hex_field(running_machine *machine, UINT32 data)
{
	INT8 key;

	if(input_code_pressed_once(KEYCODE_BACKSPACE))
	{
		/* back space */
		data >>= 4;
		return data;
	}
	else if(input_ui_pressed(machine, IPT_UI_CLEAR))
	{
		/* data clear */
		data = 0;
		return data;
	}

	key = read_hex_input();

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

	key = read_hex_input();

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
	char code = osd_readkey_unicode(0) & 0xFF;

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
			/* backspace */
			if(code == 0x08)
				data /= 10;
		}
	}

	/* adjust value */
	if(data < min)
		data = min;
	if(data > max)
		data = max;

	return data;
}

/*--------------------------------------------------------------------------------
  DoIncrementHexField - increment a specified value in hex field with arrow key
--------------------------------------------------------------------------------*/

static UINT32 DoIncrementHexField(UINT32 data, UINT8 digits)
{
	data =	((data & ~kIncrementMaskTable[digits]) |
			(((data & kIncrementMaskTable[digits]) + kIncrementValueTable[digits]) &
			kIncrementMaskTable[digits]));

	return data;
}

/*--------------------------------------------------------------------------------
  DoDecrementHexField - decrement a specified value in hex field with arrow key
--------------------------------------------------------------------------------*/

static UINT32 DoDecrementHexField(UINT32 data, UINT8 digits)
{
	data =	((data & ~kIncrementMaskTable[digits]) |
			(((data & kIncrementMaskTable[digits]) - kIncrementValueTable[digits]) &
			kIncrementMaskTable[digits]));

	return data;
}

/*--------------------------------------------------------------------------------------
  DoIncrementHexFieldSigned - increment a specified value in hex field with arrow key
--------------------------------------------------------------------------------------*/

static UINT32 DoIncrementHexFieldSigned(UINT32 data, UINT8 digits, UINT8 bytes)
{
	UINT32 newData = data;

	if(data & kSearchByteSignBitTable[bytes])
	{
		/* MINUS */
		newData =	((newData & ~kIncrementMaskTable[digits]) |
					(((newData & kIncrementMaskTable[digits]) - kIncrementValueTable[digits]) &
					kIncrementMaskTable[digits]));

		if(newData & kSearchByteSignBitTable[bytes])
			newData = ((data & ~kIncrementMaskTable[digits]) | (newData & kIncrementMaskTable[digits]));
	}
	else
	{
		/* PLUS */
		newData =	((newData & ~kIncrementMaskTable[digits]) |
					(((newData & kIncrementMaskTable[digits]) + kIncrementValueTable[digits]) &
					kIncrementMaskTable[digits]));

		if(!(newData & kSearchByteSignBitTable[bytes]))
			newData = ((data & ~kIncrementMaskTable[digits]) | (newData & kIncrementMaskTable[digits]));
	}

	return newData;
}

/*--------------------------------------------------------------------------------------
  DoDecrementHexFieldSigned - decrement a specified value in hex field with arrow key
--------------------------------------------------------------------------------------*/

static UINT32 DoDecrementHexFieldSigned(UINT32 data, UINT8 digits, UINT8 bytes)
{
	UINT32 newData = data;

	if(data & kSearchByteSignBitTable[bytes])
	{
		/* MINUS */
		newData =	((newData & ~kIncrementMaskTable[digits]) |
					(((newData & kIncrementMaskTable[digits]) + kIncrementValueTable[digits]) &
					kIncrementMaskTable[digits]));

		if(newData & kSearchByteSignBitTable[bytes])
			newData = ((data & ~kIncrementMaskTable[digits]) | (newData & kIncrementMaskTable[digits]));
	}
	else
	{
		/* PLUS */
		newData =	((newData & ~kIncrementMaskTable[digits]) |
					(((newData & kIncrementMaskTable[digits]) - kIncrementValueTable[digits]) &
					kIncrementMaskTable[digits]));

		if(!(newData & kSearchByteSignBitTable[bytes]))
			newData = ((data & ~kIncrementMaskTable[digits]) | (newData & kIncrementMaskTable[digits]));
	}

	return newData;
}

/*--------------------------------------------------------------------------------
  DoIncrementDecField - increment a specified value in dec field with arrow key
--------------------------------------------------------------------------------*/

static UINT32 DoIncrementDecField(UINT32 data, UINT8 digits)
{
	UINT32 value = data & kIncrementMaskTable[digits];

	if(value >= kIncrementDecTable[digits])
		value = 0;
	else
		value += kIncrementValueTable[digits];

	return ((data & ~kIncrementMaskTable[digits]) | value);
}

/*--------------------------------------------------------------------------------
  DoDecrementDecField - increment a specified value in dec field with arrow key
--------------------------------------------------------------------------------*/

static UINT32 DoDecrementDecField(UINT32 data, UINT8 digits)
{
	UINT32 value = data & kIncrementMaskTable[digits];

	if(!value)
		value = kIncrementDecTable[digits];
	else
		value -= kIncrementValueTable[digits];

	return ((data & ~kIncrementMaskTable[digits]) | value);
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*---------------------------------------
  cheat_init - initialize cheat system
---------------------------------------*/

void cheat_init(running_machine *machine)
{
	/* initialize lists */
	cheat_list			= NULL;
	cheat_list_length	= 0;

	watch_list			= NULL;
	watch_list_length	= 0;

	search_list = NULL;
	search_list_length = 0;

#ifdef MESS
	InitMessCheats(machine);
#endif

	/* initialize flags/parameters */
	current_search_idx	= 0;
	found_database		= 0;
	cheats_disabled		= 0;
	watches_disabled	= 0;
	editActive			= 0;
	editCursor			= 0;
	cheat_options		= DEFAULT_CHEAT_OPTIONS;
	driverSpecifiedFlag	= 0;

	fullMenuPageHeight = visible_items = floor(1.0f / ui_get_line_height()) - 1;

	/* initialize CPU/Region info for cheat system */
	build_cpu_region_info_list(machine);

	/* set cheat list from database */
	load_cheat_database(machine, LOAD_CHEAT_OPTION | LOAD_CHEAT_CODE);

	/* set default search and watch lists */
	resize_search_list(1);
	resize_watch_list(fullMenuPageHeight - 1, 0);

	/* build search region */
	{
		search_info *info = get_current_search();

		/* attemp to load user region */
		load_cheat_database(machine, LOAD_USER_REGION);

		/* if no user region or fail to load the database, attempt to build default region */
		if(info->region_list_length)
			info->search_speed = SEARCH_SPEED_USER_DEFINED;
		else
		{
			info->search_speed = SEARCH_SPEED_VERY_SLOW;
			build_search_regions(machine, info);
		}

		/* memory allocation for search region */
		allocate_search_regions(info);
	}

	/* initialize string table */
	init_string_table();

	/* initialize cheat menu stack */
	init_cheat_menu_stack();

	{
		astring * sourceName;

		sourceName = core_filename_extract_base(astring_alloc(), machine->gamedrv->source_file, TRUE);

		if(!strcmp(astring_c(sourceName), "neodrvr"))
			driverSpecifiedFlag |= 1;
		else if(!strcmp(astring_c(sourceName), "cps2"))
			driverSpecifiedFlag |= 2;
		else if(!strcmp(astring_c(sourceName), "cps3"))
			driverSpecifiedFlag |= 4;

		astring_free(sourceName);
	}

	/* NOTE : disable all cheat messages in initializing so that message parameters should be initialized here */
	SET_MESSAGE(0);

	periodic_timer = timer_alloc(cheat_periodic, NULL);
	timer_adjust_periodic(periodic_timer, video_screen_get_frame_period(machine->primary_screen), 0, video_screen_get_frame_period(machine->primary_screen));

	add_exit_callback(machine, cheat_exit);
}

/*---------------------------------
  cheat_exit - free cheat system
---------------------------------*/

static void cheat_exit(running_machine *machine)
{
	int i;

	/* save all cheats automatically if needed */
	if(TEST_FIELD(cheat_options, AutoSaveEnabled))
		do_auto_save_cheats(machine);

	/* free cheat list */
	dispose_cheat_database(machine);

	/* free watch lists */
	if(watch_list)
	{
		for(i = 0; i < watch_list_length; i++)
			dispose_watch(&watch_list[i]);

		free(watch_list);

		watch_list = NULL;
	}

	/* free search lists */
	if(search_list)
	{
		for(i = 0; i < search_list_length; i++)
			dispose_search(&search_list[i]);

		free(search_list);

		search_list = NULL;
	}

	/* free string table */
	free_string_table();

	/* free cheat menu stack */
	free_cheat_menu_stack();

	free(menu_item_info);
	menu_item_info = NULL;

#ifdef MESS
	StopMessCheats();
#endif

	/* free flags */
	cheat_list_length		= 0;
	watch_list_length		= 0;
	search_list_length		= 0;
	current_search_idx		= 0;
	found_database			= 0;
	cheats_disabled			= 0;
	watches_disabled		= 0;
	main_database_name[0]	= 0;
	menuItemInfoLength		= 0;
	cheat_variable[0]		= 0;
	cheat_options			= 0;
	driverSpecifiedFlag		= 0;
	SET_MESSAGE(0);
}

/*----------------------------------
  cheat_menu - handle cheat menus
----------------------------------*/

int cheat_menu(running_machine *machine, int selection)
{
	cheat_menu_stack *menu = &menu_stack[stack_index];

	/* handle cheat message */
	if(message_type)
		display_cheat_message();

	/* handle cheat menu */
	selection = (*menu->handler)(machine, menu);

	if(selection == 0)
	{
		if(stack_index)
			/* return previous "cheat" menu */
			cheat_menu_stack_pop();
		else
			/* return MAME general menu */
			return selection;
	}

	return selection + 1;
}

/*----------------------------------------
  BCDToDecimal - convert a value to hex
----------------------------------------*/

static UINT32 BCDToDecimal(UINT32 value)
{
	int		i;
	UINT32	accumulator	= 0;
	UINT32	multiplier	= 1;

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
	int		i;
	UINT32	accumulator	= 0;
	UINT32	divisor		= 10;

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

/*-----------------------------------------------------------------
  rebuild_string_tables - memory allocation for menu string list
-----------------------------------------------------------------*/

static void rebuild_string_tables(void)
{
	UINT32 i;
	UINT32 storage_needed;
	char *traverse;

	/* calculate length of total buffer */
	storage_needed = (menu_strings.main_string_length + menu_strings.sub_string_length) * menu_strings.num_strings;

	/* allocate memory for all items */
	menu_strings.main_list =	(const char **)	realloc((char *)	menu_strings.main_list,		sizeof(char *)	* menu_strings.length);
	menu_strings.sub_list =		(const char **)	realloc((char *)	menu_strings.sub_list,		sizeof(char *)	* menu_strings.length);
	menu_strings.flag_list =					realloc(			menu_strings.flag_list,		sizeof(char)	* menu_strings.length);
	menu_strings.main_strings =					realloc(			menu_strings.main_strings,	sizeof(char *)	* menu_strings.num_strings);
	menu_strings.sub_strings =					realloc(			menu_strings.sub_strings,	sizeof(char *)	* menu_strings.num_strings);
	menu_strings.buf =							realloc(			menu_strings.buf,			sizeof(char)	* storage_needed);

	if(	(menu_strings.main_list == NULL && menu_strings.length) ||
		(menu_strings.sub_list == NULL && menu_strings.length) ||
		(menu_strings.flag_list == NULL && menu_strings.length) ||
		(menu_strings.main_strings == NULL && menu_strings.num_strings) ||
		(menu_strings.sub_strings == NULL && menu_strings.num_strings) ||
		(menu_strings.buf == NULL && storage_needed))
	{
		fatalerror(	"cheat: [string table] memory allocation error\n"
					"	length =			%.8X\n"
					"	num_strings =		%.8X\n"
					"	main_stringLength =	%.8X\n"
					"	sub_string_length =	%.8X\n"
					"%p %p %p %p %p %p\n",
					menu_strings.length,
					menu_strings.num_strings,
					menu_strings.main_string_length,
					menu_strings.sub_string_length,

					menu_strings.main_list,
					menu_strings.sub_list,
					menu_strings.flag_list,
					menu_strings.main_strings,
					menu_strings.sub_strings,
					menu_strings.buf);
	}

	traverse = menu_strings.buf;

	/* allocate string buffer to list */
	for(i = 0; i < menu_strings.num_strings; i++)
	{
		menu_strings.main_strings[i] = traverse;
		traverse += menu_strings.main_string_length;
		menu_strings.sub_strings[i] = traverse;
		traverse += menu_strings.sub_string_length;
	}
}

/*------------------------------------------------------------
  request_strings - check string list and rebuild if needed
------------------------------------------------------------*/

static void request_strings(UINT32 length, UINT32 num_strings, UINT32 main_string_length, UINT32 sub_string_length)
{
	UINT8 changed = 0;

	/* total items */
	if(menu_strings.length < length)
	{
		menu_strings.length = length;
		changed = 1;
	}

	/* total buffer items */
	if(menu_strings.num_strings < num_strings)
	{
		menu_strings.num_strings = num_strings;
		changed = 1;
	}

	/* length of buffer 1 */
	if(menu_strings.main_string_length < main_string_length)
	{
		menu_strings.main_string_length = main_string_length;
		changed = 1;
	}

	/* length of buffer 2 */
	if(menu_strings.sub_string_length < sub_string_length)
	{
		menu_strings.sub_string_length = sub_string_length;
		changed = 1;
	}

	if(changed)
		rebuild_string_tables();
}

/*----------------------------------------------
  init_string_table - initialize string table
----------------------------------------------*/

static void init_string_table(void)
{
	memset(&menu_strings, 0, sizeof(menu_string_list));
}

/*----------------------------------------
  free_string_table - free string table
----------------------------------------*/

static void free_string_table(void)
{
	free((char *)	menu_strings.main_list);
	free((char *)	menu_strings.sub_list);
	free(			menu_strings.flag_list);
	free(			menu_strings.main_strings);
	free(			menu_strings.sub_strings);
	free(			menu_strings.buf);

	memset(&menu_strings, 0, sizeof(menu_string_list));
}

/*-----------------------------------
  create_string_copy - copy stirng
-----------------------------------*/

static char *create_string_copy(char *buf)
{
	char *temp = NULL;

	if(buf && buf[0] != 0)
	{
		/* allocate memory */
		size_t length = strlen(buf) + 1;
		temp = malloc_or_die(length);

		/* copy memory for string */
		if(temp)
			memcpy(temp, buf, length);
	}

	return temp;
}

/*--------------------------------------------------
  old_style_menu - export menu items to draw menu
--------------------------------------------------*/

static void old_style_menu(const char **items, const char **sub_items, char *flag, int selected, int arrowize_subitem)
{
	int menu_items;
	static ui_menu_item item_list[1000];

	for(menu_items = 0; items[menu_items]; menu_items++)
	{
		item_list[menu_items].text = items[menu_items];
		item_list[menu_items].subtext = sub_items ? sub_items[menu_items] : NULL;
		item_list[menu_items].flags = 0;

		/* set a highlight for sub-item */
		if(flag && flag[menu_items])
			item_list[menu_items].flags |= MENU_FLAG_INVERT;

		/* set an arrow for sub-item */
		if(menu_items == selected)
		{
			if (arrowize_subitem & 1)
				item_list[menu_items].flags |= MENU_FLAG_LEFT_ARROW;
			if (arrowize_subitem & 2)
				item_list[menu_items].flags |= MENU_FLAG_RIGHT_ARROW;
		}
	}

	visible_items = ui_menu_draw(item_list, menu_items, selected, NULL);
}

/*-------------------------------------------------------
  init_cheat_menu_stack - initilalize cheat menu stack
-------------------------------------------------------*/

static void init_cheat_menu_stack(void)
{
	cheat_menu_stack *menu = &menu_stack[stack_index = 0];

	menu->handler = cheat_main_menu;
	menu->sel = 0;
	menu->pre_sel = 0;
	menu->first_time = 1;
}

/*-----------------------------------------------------
  cheat_menu_stack_push - push a menu onto the stack
-----------------------------------------------------*/

static void cheat_menu_stack_push(cheat_menu_handler new_handler, cheat_menu_handler ret_handler, int selection)
{
	cheat_menu_stack *menu = &menu_stack[++stack_index];

	assert(stack_index < CHEAT_MENU_DEPTH);

	menu->handler = new_handler;
	menu->return_handler = ret_handler;
	menu->sel = 0;
	menu->pre_sel = selection;
	menu->first_time = 1;
	editActive = 0;
}

/*---------------------------------------------------
  cheat_menu_stack_pop - pop a menu from the stack
---------------------------------------------------*/

static void cheat_menu_stack_pop(void)
{
	int i;

	assert(stack_index > 0);

	/* NOTE : cheat menu supports deep-return */
	for(i = stack_index; i > 0; i--)
		if(menu_stack[stack_index].return_handler == menu_stack[i].handler)
			break;

	stack_index = i;
}

/*-----------------------------------------------------
  free_cheat_menu_stack - reset the cheat menu stack
-----------------------------------------------------*/

static void free_cheat_menu_stack(void)
{
	int i;
	cheat_menu_stack *menu;

	for(i = 0; i < CHEAT_MENU_DEPTH; i++)
	{
		menu = &menu_stack[i];

		menu->handler = NULL;
		menu->return_handler = NULL;
		menu->pre_sel = 0;
		menu->first_time = 0;
	}

	stack_index = 0;
}

/*---------------------------------------------------------------
  user_select_value_menu - management for value-selection menu
---------------------------------------------------------------*/

static int user_select_value_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				i;
	UINT8			is_bcd;
	UINT32			min;
	UINT32			max;
	static int		master			= 0;
	static int		display_value	= -1;
	char			buf[256];
	char			*strings_buf	= buf;
	cheat_entry		*entry			= &cheat_list[menu->pre_sel];
	cheat_action	*action			= NULL;

	/* first setting 1 : search master code of value-selection */
	if(menu->first_time)
	{
		for(i = 0; i < entry->action_list_length; i++)
		{
			if(entry->action_list[0].flags & kActionFlag_OldFormat) {
				/* NOTE : old format should be always master = 0 */
				master = 0; break; }

			/* search value selection master code */
			if(TEST_FIELD(entry->action_list[i].type, ValueSelectEnable)) {
				master = i; break; }
		}
	}

	/* set value selection master code */
	action = &entry->action_list[master];

	if(TEST_FIELD(action->type, ValueSelectMinimumDisplay) || TEST_FIELD(action->type, ValueSelectMinimum))
		min = 1;
	else
		min = 0;

	max		= action->original_data + min;
	is_bcd	= EXTRACT_FIELD(action->type, ValueSelectBCD);

	/* first setting 2 : save the value */
	if(menu->first_time)
	{
		display_value = is_bcd ? DecimalToBCD(BCDToDecimal(read_data(machine, action))) : read_data(machine, action);

		if(display_value < min)
			display_value = min;
		else if(display_value > max)
			display_value = max;
		else if(TEST_FIELD(action->type, ValueSelectMinimumDisplay))
			display_value++;

		menu->first_time = 0;
	}

	/* ##### MENU CONSTRACTION ##### */
	strings_buf += sprintf(strings_buf, "\t%s\n\t", entry->name);

	if(editActive)
	{
		for(i = kSearchByteDigitsTable[EXTRACT_FIELD(action->type, AddressSize)] - 1; i >= 0; i--)
		{
			if(i == editCursor)
				strings_buf += sprintf(strings_buf, "[%X]", (display_value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
			else
				strings_buf += sprintf(strings_buf, "%X", (display_value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
		}
	}
	else
		strings_buf += sprintf(strings_buf, "%.*X", kSearchByteDigitsTable[EXTRACT_FIELD(action->type, AddressSize)], display_value);

	if(TEST_FIELD(action->type, ValueSelectNegative))
		strings_buf += sprintf(	strings_buf, " <%.*X>",
								kSearchByteDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
								(~display_value + 1) & kSearchByteMaskTable[EXTRACT_FIELD(action->type, AddressSize)]);

	if(is_bcd == 0)
		strings_buf += sprintf(strings_buf, " (%d)", display_value);

	strings_buf += sprintf(strings_buf, "\n\t OK ");

	/* print it */
	ui_draw_message_window(buf);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			if(display_value == max)
				display_value = min;
			else
			{
				display_value = is_bcd ? DoIncrementDecField(display_value, editCursor) : DoIncrementHexField(display_value, editCursor);

				if(display_value > max)
					display_value = max;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			if(display_value == min)
				display_value = max;

			else
			{
				display_value = is_bcd ? DoDecrementDecField(display_value, editCursor) : DoDecrementHexField(display_value, editCursor);

				if(display_value > max)
					display_value = max;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			if(++editCursor > kSearchByteDigitsTable[EXTRACT_FIELD(action->type, AddressSize)] - 1)
				editCursor = 0;
		}
		else
		{
			if(display_value-- == min)
				display_value = max;

			if(is_bcd)
				display_value = DecimalToBCD(BCDToDecimal(display_value));
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			if(--editCursor < 0)
				editCursor = kSearchByteDigitsTable[EXTRACT_FIELD(action->type, AddressSize)] - 1;
		}
		else
		{
			if(++display_value > max)
				display_value = min;

			if(is_bcd)
				display_value = DecimalToBCD(BCDToDecimal(display_value));
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		int i;

		if(editActive)
			editActive = 0;

		/* adjust a value */
		if(TEST_FIELD(action->type, ValueSelectMinimumDisplay))
			display_value--;

		if(TEST_FIELD(action->type, ValueSelectMinimum) && display_value == 0)
			display_value = 1;

		if(TEST_FIELD(action->type, ValueSelectNegative) && display_value)
			display_value = (~display_value + 1) & kSearchByteMaskTable[EXTRACT_FIELD(action->type, AddressSize)];

		/* set/copy value */
		for(i = 0; i < entry->action_list_length; i++)
		{
			cheat_action *destination = &entry->action_list[i];

			int copy_value = display_value;

			if(i == master)
			{
				/* set value */
				destination->data = display_value;
			}
			else if(TEST_FIELD(destination->type, ValueSelectEnable))
			{
				/* copy value */
				int new_data = destination->original_data;

				if(TEST_FIELD(destination->type, ValueSelectBCD))
				{
					copy_value	= BCDToDecimal(DecimalToBCD(copy_value));
					new_data	= BCDToDecimal(DecimalToBCD(new_data));
				}

				copy_value += new_data;

				if(TEST_FIELD(destination->type, ValueSelectBCD))
					copy_value = DecimalToBCD(BCDToDecimal(copy_value));

				if(TEST_FIELD(destination->type, ValueSelectNegative))
					copy_value = (~copy_value + 1) & kSearchByteMaskTable[EXTRACT_FIELD(destination->type, AddressSize)];

				destination->data = copy_value;
			}
		}

		activate_cheat(machine, entry);

		menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		editActive ^= 1;
		editCursor = 0;
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)		editActive = 0;
		else				menu->sel = -1;
	}

	/* ##### EDIT ##### */
	if(editActive == 0)
	{
		UINT32 temp;

		temp			= display_value;
		display_value	= do_edit_hex_field(machine, display_value) & kCheatSizeMaskTable[EXTRACT_FIELD(action->type, AddressSize)];

		if(display_value != temp)
		{
			if(is_bcd)
				display_value = DecimalToBCD(BCDToDecimal(display_value));

			if(display_value < min)	display_value = max;
			if(display_value > max)	display_value = min;
		}
	}

	return menu->sel + 1;
}

/*------------------------------------------------------------
  user_select_label_menu - management for label-select menu
------------------------------------------------------------*/

static int user_select_label_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				i;
	UINT8			total	= 0;
	const char		**menu_item;
	char			**buf;
	cheat_entry		*entry	= &cheat_list[menu->pre_sel];

	/* required items = (total items + return + terminator) & (strings buf * total items [max chars = 100]) */
	request_strings(entry->action_list_length + 2, entry->action_list_length, 100, 0);
	menu_item	= menu_strings.main_list;
	buf			= menu_strings.main_strings;

	/* first setting : compute cursor position from previous menu */
	if(menu->first_time)
	{
		menu->sel = entry->flags & kCheatFlag_OneShot ? entry->selection - 1 : entry->selection;
		menu->first_time = 0;
	}

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < entry->label_index_length; i++)
	{
		if(i == 0)
		{
			if((entry->flags & kCheatFlag_OneShot) == 0)
			{
				/* add "OFF" item if not one shot at the first time */
				if(!entry->selection)
					sprintf(buf[total], "[ Off ]");
				else
					sprintf(buf[total], "Off");

				menu_item[total] = buf[total];
				total++;
			}

			continue;
		}
		else if(i == entry->selection)
			sprintf(buf[total], "[ %s ]", entry->action_list[entry->label_index[i]].optional_name);
		else
			sprintf(buf[total], "%s", entry->action_list[entry->label_index[i]].optional_name);

		menu_item[total] = buf[total];
		total++;
	}

	/* ##### RETURN ##### */
	menu_item[total++] = "Return to Prior Menu";

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total] = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menu_item, NULL, NULL, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(menu->sel != total - 1)
		{
			entry->selection = entry->flags & kCheatFlag_OneShot ? menu->sel + 1 : menu->sel;

			/* set new label index */
			if(entry->selection)
				activate_cheat(machine, entry);
			else
				deactivate_cheat(machine, entry);

			/* NOTE : the index number of master code should be stored into 1st table */
			if(TEST_FIELD(entry->action_list[entry->label_index[0]].type, LabelSelectQuickClose))
				menu->sel = -1;
		}
		else
			menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	return menu->sel + 1;
}

/*------------------------------------------------
  comment_menu - management simple comment menu
------------------------------------------------*/

static int comment_menu(running_machine *machine, cheat_menu_stack *menu)
{
	char		buf[2048];
	const char	*comment;
	cheat_entry	*entry = &cheat_list[menu->pre_sel];

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/********** MENU CONSTRUCTION **********/
	comment = entry->comment;
	sprintf(buf, "%s\n\t OK ", comment);

	/* print it */
	ui_draw_message_window(buf);

	/********** KEY HANDLING **********/
	if(input_ui_pressed(machine, IPT_UI_SELECT) || input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	return menu->sel + 1;
}

/*-----------------------------------------------------------
  extend_comment_menu - management multi-line comment menu
-----------------------------------------------------------*/

static int extend_comment_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int			i;
	UINT8		total	= 0;
	const char	**menu_item;
	char		**buf;
	cheat_entry	*entry	= &cheat_list[menu->pre_sel];

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = (display comment lines + return + terminator) & (strings buf * display comment lines [max char = 100]) */
	request_strings(entry->action_list_length + 1, entry->action_list_length - 1, 100, 0);
	menu_item 	= menu_strings.main_list;
	buf			= menu_strings.main_strings;

	/********** MENU CONSTRUCTION **********/
	for(i = 1; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		/* NOTE : don't display the comment in master code */
		if(i != 0)
		{
			sprintf(buf[total], "%s", action->optional_name);
			menu_item[total] = buf[total];
			total++;
		}
	}

	/* ##### OK ##### */
	menu_item[total++] = "OK";

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total] = NULL;

	/* adjust cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menu_item, NULL, NULL, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT) || input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	return menu->sel + 1;
}

/*------------------------------------------------------
  cheat_main_menu - management for cheat general menu
------------------------------------------------------*/

static int cheat_main_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum
	{
		MENU_ENABLE_DISABLE = 0,
		MENU_ADD_EDIT,
		MENU_SEARCH,
		MENU_VIEW_RESULT,
		MENU_CHOOSE_WATCH,
		MENU_OPTIONS,
		MENU_COMMANDS,
#ifdef MAME_DEBUG
		MENU_DEBUG,
#endif
		MENU_RETURN,
		MENU_MAX
	};

	int				total = 0;
	ui_menu_item	menu_item[MENU_MAX + 1];

	memset(menu_item, 0, sizeof(menu_item));

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/********** MENU CONSTRUCION **********/
	menu_item[total++].text = "Enable/Disable a Cheat";
	menu_item[total++].text = "Add/Edit a Cheat";
	switch(EXTRACT_FIELD(cheat_options, SearchBox))
	{
		case kSearchBox_Minimum:
			menu_item[total++].text = "Search a Cheat (Minimum Mode)";
			break;

		case kSearchBox_Standard:
			menu_item[total++].text = "Search a Cheat (Standard Mode)";
			break;

		case kSearchBox_Advanced:
			menu_item[total++].text = "Search a Cheat (Advanced Mode)";
			break;

		default:
			menu_item[total++].text = "Unknown Search Menu";
	}

	menu_item[total++].text = "View Last Results";
	menu_item[total++].text = "Configure Watchpoints";
	menu_item[total++].text = "Cheat Options";
	menu_item[total++].text = "Cheat Commands";
#ifdef MAME_DEBUG
	menu_item[total++].text = "Debug Viewer";
#endif
	menu_item[total++].text = "Return to Main Menu";
	menu_item[total].text = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* print it */
	ui_menu_draw(menu_item, total, menu->sel, NULL);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case MENU_ENABLE_DISABLE:
				cheat_menu_stack_push(enable_disable_cheat_menu, cheat_main_menu, menu->sel);
				break;

			case MENU_ADD_EDIT:
				cheat_menu_stack_push(add_edit_cheat_menu, cheat_main_menu, 0);
				break;

			case MENU_SEARCH:
				switch(EXTRACT_FIELD(cheat_options, SearchBox))
				{
					case kSearchBox_Minimum:
						cheat_menu_stack_push(search_minimum_menu, menu->handler, menu->sel);
						break;

					case kSearchBox_Standard:
						cheat_menu_stack_push(search_standard_menu, menu->handler, menu->sel);
						break;

					case kSearchBox_Advanced:
						cheat_menu_stack_push(search_advanced_menu, menu->handler, menu->sel);
				}
				break;

			case MENU_VIEW_RESULT:
				cheat_menu_stack_push(view_search_result_menu, menu->handler, menu->sel);
				break;

			case MENU_CHOOSE_WATCH:
				cheat_menu_stack_push(choose_watch_menu, menu->handler, menu->sel);
				break;

			case MENU_OPTIONS:
				cheat_menu_stack_push(select_option_menu, menu->handler, menu->sel);
				break;

			case MENU_COMMANDS:
				cheat_menu_stack_push(command_cheat_menu, menu->handler, menu->sel);
				break;

#ifdef MAME_DEBUG
			case MENU_DEBUG:
				cheat_menu_stack_push(debug_cheat_menu, menu->handler, menu->sel);
				break;
#endif
			case MENU_RETURN:
				menu->sel = -1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		reload_cheat_database(machine);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*-----------------------------------------------------------------
  enable_disable_cheat_menu - management for Enable/Disable menu
-----------------------------------------------------------------*/

static int enable_disable_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				i;
	UINT8			total			= 0;
	UINT8			length			= cheat_list_length ? cheat_list_length : 2;
	UINT8			is_empty		= 0;
	UINT8			request_arrow	= 0;
	UINT8			*menu_index;
	static INT8		current_layer;
	const char		**menu_item;
	const char		**menu_sub_item;
	char			*flag_buf;
	cheat_entry		*entry;

	/* first setting : always starts with root layer level */
	if(menu->first_time)
	{
		current_layer = 0;
		menu->first_time = 0;
	}

	/* required items = total codes + return + terminator */
	request_strings(length + 2, 0, 0, 0);

	menu_item		= menu_strings.main_list;
	menu_sub_item	= menu_strings.sub_list;
	flag_buf		= menu_strings.flag_list;

	menu_index = malloc_or_die(sizeof(INT8) * (length + 1));
	memset(menu_index, 0, sizeof(INT8) * (length + 1));

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < cheat_list_length; i++)
	{
		cheat_entry *traverse = &cheat_list[i];

		if(total >= fullMenuPageHeight) break;

		/* when return to previous layer, set cursor position to previous layer label code */
		if(traverse->flags & kCheatFlag_LayerSelected)
		{
			menu->sel = total;
			traverse->flags &= ~kCheatFlag_LayerSelected;
		}

		if(traverse->layer_index == current_layer)
		{
			/* ##### SEPARAOTR ##### */
			if(traverse->flags & kCheatFlag_Separator)
			{
				menu_item[total] = MENU_SEPARATOR_ITEM;
				menu_sub_item[total] = NULL;
				menu_index[total++] = i;
				traverse->flags &= ~kCheatFlag_RequestArrow;
			}
			else
			{
				/* ##### NAME ##### */
				if(traverse->name)
					menu_item[total] = traverse->name;
				else
					menu_item[total] = "null name";

				/* ##### SUB ITEM ##### */
				if(traverse->flags & kCheatFlag_HasWrongCode)
				{
					/* "LOCKED" if wrong code */
					menu_sub_item[total] = "Locked";
					traverse->selection = 0;
					traverse->flags &= ~kCheatFlag_RequestArrow;
				}
				else if(traverse->flags & kCheatFlag_Select)
				{
					/* label or "OFF" if label-selection though one-shot should ignore "OFF" */
					if(traverse->selection)
						menu_sub_item[total] = traverse->action_list[traverse->label_index[traverse->selection]].optional_name;
					else
						menu_sub_item[total] = "Off";

					traverse->flags |= kCheatFlag_RequestArrow;
				}
				else if(traverse->flags & kCheatFlag_ExtendComment)
				{
					/* "READ" if extend comment code */
					menu_sub_item[total] = "Read";
					traverse->flags &= ~kCheatFlag_RequestArrow;
				}
				else if(traverse->flags & kCheatFlag_LayerIndex)
				{
					/* ">>>" (next layer) if layer label code */
					menu_sub_item[total] = ">>>";
					traverse->flags &= ~kCheatFlag_RequestArrow;
				}
				else if((traverse->flags & kCheatFlag_Null) == 0)
				{
					/* otherwise, add sub-items */
					if(traverse->flags & kCheatFlag_OneShot)
					{
						menu_sub_item[total] = "Set";
						traverse->flags &= ~kCheatFlag_RequestArrow;
					}
					else
					{
						if(traverse->flags & kCheatFlag_Active)
							menu_sub_item[total] = "On";
						else
							menu_sub_item[total] = "Off";

						traverse->flags |= kCheatFlag_RequestArrow;
					}
				}

				if(traverse->flags & kCheatFlag_OldFormat)
					traverse->flags &= ~kCheatFlag_RequestArrow;

				/* set highlight flag if comment is set */
				if(traverse->comment && traverse->comment[0])
					flag_buf[total] = 1;
				else
					flag_buf[total] = 0;

				menu_index[total++] = i;
			}
		}
		else if(traverse->flags & kCheatFlag_LayerIndex)
		{
			if(current_layer == traverse->action_list[0].data)
			{
				/* specified handling previous layer label code with "<<<" (previous layer) */
				if(traverse->comment)
					menu_item[total] = traverse->comment;
				else
					menu_item[total] = "Return to Prior Layer";
				menu_sub_item[total] = "<<<";
				flag_buf[total] = 0;
				menu_index[total++] = i;
				traverse->flags &= ~kCheatFlag_RequestArrow;
			}
		}
	}

	/* if no code, set special message */
	if(cheat_list_length == 0)
	{
		if(found_database)
		{
			/* the database is found but no code */
			menu_item[total]		= "there are no cheats for this game";
			menu_sub_item[total]	= NULL;
			menu_index[total]		= total;
			flag_buf[total++]		= 0;
			is_empty				= 1;
		}
		else
		{
			/* the database itself is not found */
			menu_item[total]		= "cheat database not found";
			menu_sub_item[total]	= NULL;
			menu_index[total]		= total;
			flag_buf[total++]		= 0;

			menu_item[total]		= "unzip it and place it in the MAME directory";
			menu_sub_item[total]	= NULL;
			menu_index[total]		= total;
			flag_buf[total++]		= 0;
			is_empty				= 2;
		}
	}
	else if(current_layer && total == 1)
	{
		/* selected layer doesn't have code */
		menu_item[total]		= "selected layer doesn't have sub code";
		menu_sub_item[total]	= NULL;
		menu_index[total]		= menu_index[total - 1];
		flag_buf[total++]		= 0;
		is_empty				= 3;
	}

	/* ##### RETURN ##### */
	if(current_layer)
		menu_item[total]	= "Return to Root Layer";
	else
		menu_item[total]	= "Return to Prior Menu";
	menu_sub_item[total]	= NULL;
	flag_buf[total++]		= 0;

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total]		= NULL;
	menu_sub_item[total]	= NULL;
	flag_buf[total]			= 0;

	/* adjust cursor position */
	switch(is_empty)
	{
		default:
			ADJUST_CURSOR(menu->sel, total);

			/* if cursor is on comment code, skip it */
			while(menu->sel < total - 1 && (cheat_list[menu_index[menu->sel]].flags & kCheatFlag_Null))
				menu->sel++;
			break;

		case 1:
		case 2:
			/* no database or code, unselectable message line */
			menu->sel = total - 1;
			break;

		case 3:
			/* no code in sub layer, unselectable message line */
			if(menu->sel == 1)
				menu->sel = 0;
	}

	/* set selected item's cheat entry */
	if(is_empty == 0 && menu->sel < total - 1)
	{
		entry = &cheat_list[menu_index[menu->sel]];

		if(entry->flags & kCheatFlag_Null)
			entry = NULL;

		if(entry->flags & kCheatFlag_Separator)
			entry = NULL;

		if(entry->flags & kCheatFlag_RequestArrow)
			request_arrow = 3;
	}
	else
		entry = NULL;

	/* draw it */
	old_style_menu(menu_item, menu_sub_item, flag_buf, menu->sel, request_arrow);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		switch(is_empty)
		{
			default:
				CURSOR_TO_PREVIOUS(menu->sel, total);

				if(cheat_list_length)
				{
					/* if cursor is on comment code, skip it */
					for(i = 0; (i < fullMenuPageHeight / 2) && menu->sel != total - 1 && (cheat_list[menu_index[menu->sel]].flags & kCheatFlag_Null); i++)
					{
						if(--menu->sel < 0)
							menu->sel = total - 1;
					}
				}
				break;

			case 1:
				break;

			case 2:
				if(menu->sel)
					menu->sel = 0;
				else
					menu->sel = 2;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		switch(is_empty)
		{
			default:
				CURSOR_TO_NEXT(menu->sel, total);

				if(cheat_list_length)
				{
					/* if cursor is on comment code, skip it */
					for(i = 0; (i < fullMenuPageHeight / 2) && menu->sel < total - 1 && (cheat_list[menu_index[menu->sel]].flags & kCheatFlag_Null); i++)
						menu->sel++;
				}

			case 1:
				break;

			case 2:
				if(menu->sel)
					menu->sel = 0;
				else
					menu->sel = 2;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}

	if(entry && (entry->flags & kCheatFlag_Null) == 0 && (entry->flags & kCheatFlag_ExtendComment) == 0&& (entry->flags & kCheatFlag_HasWrongCode) == 0)
	{
		UINT8 toggle = 0;

		if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
		{
			if(entry->flags & kCheatFlag_Select)
			{
				if(TEST_FIELD(entry->action_list[entry->label_index[0]].type, LabelSelectUseSelector))
				{
					/* activate label selector */
					cheat_menu_stack_push(user_select_label_menu, menu->handler, menu->sel);
				}
				else
				{
					/* select previous label */
					if(--entry->selection < 0)
						entry->selection = entry->label_index_length - 1;

					/* NOTE : one shot cheat should not be activated by changing label */
					if(entry->label_index[entry->selection] == 0)
						deactivate_cheat(machine, entry);
					else if((entry->flags & kCheatFlag_OneShot) == 0 && (entry->flags & kCheatFlag_Active) == 0)
						activate_cheat(machine, entry);
				}
			}
			else if(entry->flags & kCheatFlag_LayerIndex)
			{
				/* change layer level */
				toggle = 2;
			}
			else if(!(entry->flags & kCheatFlag_OneShot))
			{
				/* toggle ON/OFF */
				toggle = 1;
			}
		}
		else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
		{
			if(entry->flags & kCheatFlag_Select)
			{
				/* label-selection special handling */
				if(TEST_FIELD(entry->action_list[entry->label_index[0]].type, LabelSelectUseSelector))
				{
					/* activate label selector */
					cheat_menu_stack_push(user_select_label_menu, menu->handler, menu->sel);
				}
				else
				{
					/* select next label index */
					if(++entry->selection >= entry->label_index_length)
						entry->selection = 0;

					/* NOTE : one shot cheat should not be activated by changing label */
					if(entry->label_index[entry->selection] == 0)
						deactivate_cheat(machine, entry);
					else if((entry->flags & kCheatFlag_OneShot) == 0 && (entry->flags & kCheatFlag_Active) == 0)
						activate_cheat(machine, entry);
				}
			}
			else if(entry->flags & kCheatFlag_LayerIndex)
			{
				/* change layer level */
				toggle = 2;
			}
			else if(!(entry->flags & kCheatFlag_OneShot))
			{
				/* toggle ON/OFF */
				toggle = 1;
			}
		}

		switch(toggle)
		{
			case 1:
				{
					int active = (entry->flags & kCheatFlag_Active) ^ 1;

					if((entry->flags & kCheatFlag_UserSelect) && active)
					{
						/* activate value-selection menu */
						cheat_menu_stack_push(user_select_value_menu, menu->handler, menu->sel);
					}
					else
					{
						if(active)
							activate_cheat(machine, entry);
						else
							deactivate_cheat(machine, entry);
					}
				}
				break;

			case 2:
				{
					/* go to next/previous layer level */
					if(entry->action_list[0].data == current_layer)
						current_layer = entry->action_list[0].address;
					else
						current_layer = entry->action_list[0].data;

					entry->flags |= kCheatFlag_LayerSelected;
				}
				break;

			default:
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(entry == NULL)
		{
			if(current_layer)
				current_layer = menu->sel = 0;
			else
				menu->sel = -1;
		}
		else if((entry->flags & kCheatFlag_Null) == 0)
		{
			if(ShiftKeyPressed() && (entry->comment && entry->comment[0]))
			{
				/* display comment */
				cheat_menu_stack_push(comment_menu, menu->handler, menu->sel);
			}
			else if(entry->flags & kCheatFlag_ExtendComment)
			{
				/* display extend comment */
				cheat_menu_stack_push(extend_comment_menu, menu->handler, menu->sel);
			}
			else if(entry->flags & kCheatFlag_UserSelect)
			{
				/* activate value-selection menu */
				cheat_menu_stack_push(user_select_value_menu, menu->handler, menu->sel);
			}
			else if((entry->flags & kCheatFlag_Select) && TEST_FIELD(entry->action_list[entry->label_index[0]].type, LabelSelectUseSelector))
			{
				/* activate label selector */
				cheat_menu_stack_push(user_select_label_menu, menu->handler, menu->sel);
			}
			else if(entry->flags & kCheatFlag_LayerIndex)
			{
				/* go to next/previous layer level */
				if(entry->action_list[0].data == current_layer)
					current_layer = entry->action_list[0].address;
				else
					current_layer = entry->action_list[0].data;

				entry->flags |= kCheatFlag_LayerSelected;
			}
			else if(entry->flags & kCheatFlag_HasWrongCode)
			{
				/* analyse wrong format code */
				cheat_menu_stack_push(analyse_cheat_menu, menu->handler, menu->sel);
			}
			else
			{
				/* activate selected code */
				activate_cheat(machine, entry);
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			/* shift + save : save all codes */
			for(i = 0; i < cheat_list_length; i++)
				save_cheat_code(machine, &cheat_list[i]);

			SET_MESSAGE(CHEAT_MESSAGE_ALL_CHEATS_SAVED);
		}
		else if(ControlKeyPressed())
		{
			/* ctrl + save : save activation key */
			save_activation_key(machine, entry, menu_index[menu->sel]);
		}
		else if(AltKeyPressed())
		{
			/* alt + save : save pre-enable */
			save_pre_enable(machine, entry, menu_index[menu->sel]);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
	{
		add_cheat_before(menu_index[menu->sel]);
	}
	else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(entry)
			delete_cheat_at(menu_index[menu->sel]);
		else
			SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(entry)
		{
			cheat_menu_stack_push(command_add_edit_menu, menu->handler, menu->sel);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
	{
		watch_cheat_entry(entry, 0);
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		reload_cheat_database(machine);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		/* NOTE : cancel button return cheat main menu directly */
		menu->sel = -1;
	}

	free(menu_index);

	return menu->sel + 1;
}

/*-----------------------------------------------------------
  add_edit_cheat_menu - management for Add/Edit cheat menu
-----------------------------------------------------------*/

static int add_edit_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				i;
	UINT8			total = 0;
	const char		**menu_item;
	cheat_entry		*entry;

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = total codes + return + terminator */
	request_strings(cheat_list_length + 2, 0, 0, 0);
	menu_item = menu_strings.main_list;

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < cheat_list_length; i++)
	{
		cheat_entry *traverse = &cheat_list[i];

		/* ##### NAME ##### */
		if(traverse->name)
			menu_item[total++] = traverse->name;
		else
			menu_item[total++] = "(none)";
	}

	/* ##### RETURN ##### */
	menu_item[total++] = "Return to Prior Menu";

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total] = NULL;

	/* adjust cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menu_item, NULL, NULL, menu->sel, 0);

	/* set entry for selected item if valid, otherwise null (return item) */
	if(menu->sel < total - 1)
		entry = &cheat_list[menu->sel];
	else
		entry = NULL;

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(entry)
		{
			if(entry->flags & kCheatFlag_HasWrongCode)
			{
				cheat_menu_stack_push(analyse_cheat_menu, menu->handler, menu->sel);
			}
			else
				cheat_menu_stack_push(command_add_edit_menu, menu->handler, menu->sel);
		}
		else
			menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		reload_cheat_database(machine);
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(entry)
		{
			if(entry->flags & kCheatFlag_HasWrongCode)
			{
				cheat_menu_stack_push(analyse_cheat_menu, menu->handler, menu->sel);
			}
			else
				cheat_menu_stack_push(command_add_edit_menu, menu->handler, menu->sel);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
	{
		add_cheat_before(menu->sel);
	}
	else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		delete_cheat_at(menu->sel);
	}
	else if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			/* shift + save = save all codes */
			for(i = 0; i < cheat_list_length; i++)
				save_cheat_code(machine, &cheat_list[i]);

			SET_MESSAGE(CHEAT_MESSAGE_ALL_CHEATS_SAVED);
		}
		else if(ControlKeyPressed())
		{
			if((entry->flags & kCheatFlag_HasActivationKey1) || (entry->flags & kCheatFlag_HasActivationKey2))
				/* ctrl + save = save activation key */
				save_activation_key(machine, entry, menu->sel);
		}
		else if(AltKeyPressed())
		{
			/* alt + save = save pre-enable code */
			save_pre_enable(machine, entry, menu->sel);
		}
		else
		{
			save_cheat_code(machine, entry);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
	{
		watch_cheat_entry(entry, 0);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*-----------------------------------------------------------
  command_add_edit_menu - management for code command menu
-----------------------------------------------------------*/

static int command_add_edit_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_EditCheat = 0,
		kMenu_ReloadDatabase,
		kMenu_WatchCheat,
		kMenu_SaveCheat,
		kMenu_save_activation_key,
		kMenu_save_pre_enable,
		kMenu_SaveAllCodes,
		kMenu_AddCode,
		kMenu_DeleteCode,
#ifdef MAME_DEBUG
		kMenu_ConvertFormat,
		kMenu_AnalyseCode,
#endif
		kMenu_Return,
		kMenu_Max };

	int				i;
	UINT8			total = 0;
	ui_menu_item	menu_item[kMenu_Max + 1];
	cheat_entry		*entry = &cheat_list[menu->pre_sel];

	memset(menu_item, 0, sizeof(menu_item));

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/********** MENU CONSTRUCION **********/
	if(entry->flags & kCheatFlag_OldFormat)
		menu_item[total++].text = "Edit Code";
	else
		menu_item[total++].text = "View Code";
	menu_item[total++].text = "Reload Database";
	menu_item[total++].text = "Watch Code";
	menu_item[total++].text = "Save Code";
	menu_item[total++].text = "Save Activation Key";
	menu_item[total++].text = "Save PreEnable";
	menu_item[total++].text = "Save All Codes";
	menu_item[total++].text = "Add New Code";
	menu_item[total++].text = "Delete Code";
#ifdef MAME_DEBUG
	if(entry->flags & kCheatFlag_OldFormat)
		menu_item[total++].text = "Convert To New Format";
	else
		menu_item[total++].text = "Convert To Old Format";
	menu_item[total++].text = "Analyse Code";
#endif
	menu_item[total++].text = "Return to Prior Menu";
	menu_item[total].text = NULL;

	/* adjust cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* print it */
	ui_menu_draw(menu_item, total, menu->sel, NULL);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case kMenu_EditCheat:
				if(entry->flags & kCheatFlag_OldFormat)
					cheat_menu_stack_push(edit_cheat_menu, menu->return_handler, menu->pre_sel);
				else
					cheat_menu_stack_push(view_cheat_menu, menu->return_handler, menu->pre_sel);
				break;

			case kMenu_ReloadDatabase:
				reload_cheat_database(machine);
				cheat_menu_stack_pop();
				break;

			case kMenu_WatchCheat:
				watch_cheat_entry(entry, 0);
				break;

			case kMenu_SaveCheat:
				save_cheat_code(machine, entry);
				break;

			case kMenu_save_activation_key:
				save_activation_key(machine, entry, menu->pre_sel);
				break;

			case kMenu_save_pre_enable:
				save_pre_enable(machine, entry, menu->pre_sel);
				break;

			case kMenu_SaveAllCodes:
				{
					for(i = 0; i < cheat_list_length; i++)
						save_cheat_code(machine, &cheat_list[i]);

					SET_MESSAGE(CHEAT_MESSAGE_ALL_CHEATS_SAVED);
				}
				break;

			case kMenu_AddCode:
				add_cheat_before(menu->pre_sel);
				menu->sel = -1;
				break;

			case kMenu_DeleteCode:
				delete_cheat_at(menu->pre_sel);
				menu->sel = -1;
				break;

#ifdef MAME_DEBUG
			case kMenu_ConvertFormat:
				if((entry->flags & kCheatFlag_OldFormat) == 0)
				{
					/* new -> old (UNDERCONSTRUCTION) */
					for(i = 0; i < entry->action_list_length; i++)
					{
						entry->action_list[i].flags |= kActionFlag_OldFormat;
						update_cheat_info(machine, entry, 0);
					}
				}
				else
				{
					/* old -> new (UNDERCONSTRUCTION) */
					for(i = 0; i < entry->action_list_length; i++)
					{
						entry->action_list[i].flags &= ~kActionFlag_OldFormat;
						update_cheat_info(machine, entry, 0);
					}
				}
				break;

			case kMenu_AnalyseCode:
				cheat_menu_stack_push(analyse_cheat_menu, menu->return_handler, menu->pre_sel);
				break;
#endif
			case kMenu_Return:
				menu->sel = -1;
				break;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		reload_cheat_database(machine);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL) || input_ui_pressed(machine, IPT_UI_LEFT) || input_ui_pressed(machine, IPT_UI_RIGHT))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*--------------------------------------------------
  edit_cheat_menu - management for edit code menu
--------------------------------------------------*/

static int edit_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
#if 0
	static const char *const kTypeNames[] =
	{
		"Normal/Delay",
		"Wait",
		"Ignore Decrement",
		"Watch",
		"Comment",
		"Select"
	};

	static const char *const kOperationNames[] =
	{
		"Write",
		"Add/Subtract",
		"Force Range",
		"Set/Clear Bits",
		"Unused (4)",
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

	static const char *const kLocationNames[] =
	{
		"Normal",
		"Region",
		"Mapped Memory",
		"Custom",
		"Relative Address",
		"Unused (5)", "Unused (6)", "Unused (7)"
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
		kType_Code,						//  value       Type Field
										//  NOTE:   it's debug display so that must not modify directly
		kType_Link,						//  select      Link                  Off - On
		kType_LinkExtension,			//  select      Link Extension        Off - On
		kType_Type,						//  select      Type                  Normal/Delay - Watch - Comment - (label) Select
		kType_OneShot,					//  select      OneShot               Off - On
		kType_RestorePreviousValue,		//  select      RestorePreviousValue  Off - On
			// if(Type != Ignore Decrement)
			kType_Delay,				//  value       TypeParameter         0 - 7
			// else
			kType_IgnoreDecrementBy,	//  value       TypeParameter         0 - 7
			// if(Type == Watch)
			kType_WatchSize,			//  value       Data                  0x01 - 0xFF (stored as 0x00 - 0xFE)
										//  NOTE: value is packed in to 0x000000FF
			kType_WatchSkip,			//  value       Data                  0x00 - 0xFF
										//  NOTE: value is packed in to 0x0000FF00
			kType_WatchPerLine,			//  value       Data                  0x00 - 0xFF
										//  NOTE: value is packed in to 0x00FF0000
			kType_WatchAddValue,		//  value       Data                  -0x80 - 0x7F
										//  NOTE: value is packed in to 0xFF000000
			kType_WatchFormat,			//  select      TypeParameter         Hex - Decimal - Binary - ASCII
										//  NOTE: value is packed in to 0x03
			kType_WatchLabel,			//  select      TypeParameter         Off - On
										//  NOTE: value is packed in to 0x04
		kType_Operation,				//  select      Operation             Write - Add/Subtract - Force Range - Set/Clear Bits
		kType_OperationExtend,			//  select      Operation Extend      Off - On
										//  NOTE: Operation Extend is unused right now
			// if(LocationType != Relative Address && Operation == Write)
			kType_WriteMask,			//  value       extendData            0x00000000 - 0xFFFFFFFF
			// if(LocationType != Relative Address && Operation == Add/Subtract)
			kType_AddSubtract,			//  select      OperationParameter    Add - Subtract
				// if(OperationParameter == Add)
				kType_AddMaximum,		//  value       extendData            0x00000000 - 0xFFFFFFFF
				// else
				kType_SubtractMinimum,	//  value       extendData            0x00000000 - 0xFFFFFFFF
			// if(LocationType != Relative Address && Operation == Force Range)
			kType_RangeMinimum,			//  value       extendData            0x00 - 0xFF / 0xFFFF
										//  NOTE: value is packed in to upper byte of extendData (as a word)
			kType_RangeMaximum,			//  value       extendData            0x00 - 0xFF / 0xFFFF
										//  NOTE: value is packed in to lower byte of extendData (as a word)
			// if(Operation == Set/Clear)
			kType_SetClear,				//  select      OperationParameter    Set - Clear
		kType_Data,
		kType_UserSelect,				//  select      UserSelectEnable      Off - On
		kType_UserSelectMinimumDisp,	//  value       UserSelectMinimumDisplay  Off - On
		kType_UserSelectMinimum,		//  value       UserSelectMinimum     0 - 1
		kType_UserSelectBCD,			//  select      UserSelectBCD         Off - On
		kType_CopyPrevious,				//  select      LinkCopyPreviousValue Off - On
		kType_Prefill,					//  select      UserSelectPrefill     None - FF - 00 - 01
		kType_ByteLength,				//  value       BytesUsed             1 - 4
		kType_Endianness,				//  select      Endianness            Normal - Swap
		kType_LocationType,				//  select      LocationType          Normal - Region - Mapped Memory - EEPROM - Relative Address
			// if(LocationType != Region && LocationType != Relative Address)
			kType_CPU,					//  value       LocationParameter     0 - 31
			// if(LocationType == Region)
			kType_Region,				//  select      LocationParameter     CPU1 - CPU2 - CPU3 - CPU4 - CPU5 - CPU6 - CPU7 -
										//                                    CPU8 - GFX1 - GFX2 - GFX3 - GFX4 - GFX5 - GFX6 -
										//                                    GFX7 - GFX8 - PROMS - SOUND1 - SOUND2 - SOUND3 -
										//                                    SOUND4 - SOUND5 - SOUND6 - SOUND7 - SOUND8 -
										//                                    USER1 - USER2 - USER3 - USER4 - USER5 - USER6 -
										//                                    USER7
										//  NOTE: old format doesn't support USER8 - PLDS
			// if(LocationType == Relative Address)
			kType_PackedCPU,			//  value       LocationParameter     0 - 7
										//  NOTE: packed in to upper three bits of LocationParameter
			kType_PackedSize,			//  value       LocationParameter     1 - 4
										//  NOTE: packed in to lower two bits of LocationParameter
			kType_AddressIndex,			//  value       extendData            -0x80000000 - 0x7FFFFFFF
		kType_Address,

		kType_Return,
		kType_Divider,

		kType_Max
	};

	int					i;
	int					sel				= selection - 1;
	int					total			= 0;
	UINT8				dirty			= 0;
	static UINT8		editActive		= 0;
	const char			** menuItem;
	const char			** menuSubItem;
	char				* flagBuf;
	char				** extendDataBuf;		// FFFFFFFF (-80000000)
	char				** addressBuf;			// FFFFFFFF
	char				** dataBuf;				// 80000000 (-2147483648)
	char				** typeBuf;				// FFFFFFFF
	char				** watchSizeBuf;		// FF
	char				** watchSkipBuf;		// FF
	char				** watchPerLineBuf;		// FF
	char				** watchAddValueBuf;	// FF

	menu_item_info		* info = NULL;
	CheatAction			* action		= NULL;
	astring				* tempString1	= astring_alloc();		// activation key 1
	astring				* tempString2	= astring_alloc();		// activation key 2

	if(!entry)
		return 0;

	if(menuItemInfoLength < (kType_Max * entry->actionListLength) + 2)
	{
		menuItemInfoLength = (kType_Max * entry->actionListLength) + 2;

		menuItemInfo = realloc(menuItemInfo, menuItemInfoLength * sizeof(menu_item_info));
	}

	request_strings((kType_Max * entry->actionListLength) + 2, 8 * entry->actionListLength, 32, 0);

	menuItem			= menu_strings.main_list;
	menuSubItem			= menu_strings.sub_list;
	flagBuf				= menu_strings.flag_list;
	extendDataBuf		= &menu_strings.main_strings[entry->actionListLength * 0];
	addressBuf			= &menu_strings.main_strings[entry->actionListLength * 1];
	dataBuf				= &menu_strings.main_strings[entry->actionListLength * 2];
	typeBuf				= &menu_strings.main_strings[entry->actionListLength * 3];
	watchSizeBuf		= &menu_strings.main_strings[entry->actionListLength * 4];	// these fields are wasteful
	watchSkipBuf		= &menu_strings.main_strings[entry->actionListLength * 5];	// but the alternative is even more ugly
	watchPerLineBuf		= &menu_strings.main_strings[entry->actionListLength * 6];	//
	watchAddValueBuf	= &menu_strings.main_strings[entry->actionListLength * 7];	//

	memset(flagBuf, 0, (kType_Max * entry->actionListLength) + 2);

	/********** MENU CONSTRUCTION **********/
	/* create menu items */
	for(i = 0; i < entry->actionListLength; i++)
	{
		CheatAction	* traverse = &entry->actionList[i];

		UINT32	type					= EXTRACT_FIELD(traverse->type, Type);
		UINT32	typeParameter			= EXTRACT_FIELD(traverse->type, TypeParameter);
		UINT32	operation				= EXTRACT_FIELD(traverse->type, Operation) | EXTRACT_FIELD(traverse->type, OperationExtend) << 2;
		UINT32	operationParameter		= EXTRACT_FIELD(traverse->type, OperationParameter);
		UINT32	locationType			= EXTRACT_FIELD(traverse->type, LocationType);
		UINT32	locationParameter		= EXTRACT_FIELD(traverse->type, LocationParameter);

		/* create item if label-select, extend comment or condition */
		if(!i)
		{
			/* do name field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Name;
			menuItem[total] = "Name";

			if(entry->name)
				menuSubItem[total++] = entry->name;
			else
				menuSubItem[total++] = "(none)";
		}
		else
		{
			/* do label name field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_ExtendName;
			menuItem[total] = "Name";

			if(traverse->optionalName)
				menuSubItem[total++] = traverse->optionalName;
			else
				menuSubItem[total++] = "(none)";
		}

		if(!i)
		{
			/* do comment field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Comment;
			menuItem[total] = "Comment";

			if(entry->comment)
				menuSubItem[total++] = entry->comment;
			else
				menuSubItem[total++] = "(none)";
		}

		{
			/* do 1st (previous-selection) activation key field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_ActivationKey1;

			if(locationType == kLocation_Custom && locationParameter == kCustomLocation_Select)
				menuItem[total] = "Activation Key - Prev";
			else
				menuItem[total] = "Activation Key - 1st";

			if(entry->flags & kCheatFlag_HasActivationKey1)
				menuSubItem[total++] = astring_c(input_code_name(tempString1, entry->activationKey1));
			else
				menuSubItem[total++] = "(none)";
		}

		{
			/* do 2nd (next-selection) activation key field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_ActivationKey2;

			if(locationType == kLocation_Custom && locationParameter == kCustomLocation_Select)
				menuItem[total] = "Activation Key - Next";
			else
				menuItem[total] = "Activation Key - 2nd";

			if(entry->flags & kCheatFlag_HasActivationKey2)
				menuSubItem[total++] = astring_c(input_code_name(tempString2, entry->activationKey2));
			else
				menuSubItem[total++] = "(none)";
		}

		{
			/* do type code field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Code;
			menuItem[total] = "Code";

			sprintf(typeBuf[i], "%8.8X", traverse->type);
			menuSubItem[total++] = typeBuf[i];
		}

		{
			/* do link field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Link;
			menuItem[total] = "Link";
			menuSubItem[total++] = TEST_FIELD(traverse->type, LinkEnable) ? "On" : "Off";
		}

		{
			/* do link extension field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_LinkExtension;
			menuItem[total] = "Link Extension";
			menuSubItem[total++] = TEST_FIELD(traverse->type, LinkExtension) ? "On" : "Off";
		}

		{
			/* do type field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Type;
			menuItem[total] = "Type";

			if(locationType == kLocation_Custom && locationParameter == kCustomLocation_Comment)
				menuSubItem[total++] = kTypeNames[4];
			else if(locationType == kLocation_Custom && locationParameter == kCustomLocation_Select)
				menuSubItem[total++] = kTypeNames[5];
			else
				menuSubItem[total++] = kTypeNames[type & 3];
		}

		{
			/* do one shot field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_OneShot;
			menuItem[total] = "One Shot";
			menuSubItem[total++] = TEST_FIELD(traverse->type, OneShot) ? "On" : "Off";
		}

		{
			/* do restore previous value field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_RestorePreviousValue;
			menuItem[total] = "Restore Previous Value";
			menuSubItem[total++] = TEST_FIELD(traverse->type, RestorePreviousValue) ? "On" : "Off";
		}

		switch(type)
		{
			default:
			{
				/* do delay field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_Delay;

				if(TEST_FIELD(traverse->type, OneShot) && TEST_FIELD(traverse->type, RestorePreviousValue) && EXTRACT_FIELD(traverse->type, TypeParameter))
					menuItem[total] = "Keep";
				else
					menuItem[total] = "Delay";

				menuSubItem[total++] = kNumbersTable[typeParameter];
			}
			break;

			case kType_IgnoreIfDecrementing:
			{
				/* do ignore decrement by field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_IgnoreDecrementBy;
				menuItem[total] = "Ignore Decrement By";
				menuSubItem[total++] = kNumbersTable[typeParameter];
			}
			break;

			case kType_Watch:
			{
				/* do watch size field */
				sprintf(watchSizeBuf[i], "%d", (traverse->originalDataField & 0xFF) + 1);

				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchSize;
				menuItem[total] = "Watch Size";
				menuSubItem[total++] = watchSizeBuf[i];
			}

			{
				/* do watch skip field */
				sprintf(watchSkipBuf[i], "%d", (traverse->data >> 8) & 0xFF);

				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchSkip;
				menuItem[total] = "Watch Skip";
				menuSubItem[total++] = watchSkipBuf[i];
			}

			{
				/* do watch per line field */
				sprintf(watchPerLineBuf[i], "%d", (traverse->data >> 16) & 0xFF);

				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchPerLine;
				menuItem[total] = "Watch Per Line";
				menuSubItem[total++] = watchPerLineBuf[i];
			}

			{
				/* do watch add value field */
				INT8 temp = (traverse->data >> 24) & 0xFF;

				if(temp < 0)
					sprintf(watchAddValueBuf[i], "-%.2X", -temp);
				else
					sprintf(watchAddValueBuf[i], "%.2X", temp);

				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchAddValue;
				menuItem[total] = "Watch Add Value";
				menuSubItem[total++] = watchAddValueBuf[i];
			}

			{
				/* do watch format field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchFormat;
				menuItem[total] = "Watch Format";
				menuSubItem[total++] = kWatchDisplayTypeStringList[(typeParameter >> 0) & 0x03];
			}

			{
				/* do watch label field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_WatchLabel;
				menuItem[total] = "Watch Label";
				menuSubItem[total++] = typeParameter >> 2 & 0x01 ? "On" : "Off";
			}
			break;
		}

		{
			/* do operation field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Operation;
			menuItem[total] = "Operation";

			menuSubItem[total++] = kOperationNames[operation];
		}

		{
			/* do operation extend field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_OperationExtend;
			menuItem[total] = "Operation Extend";
			menuSubItem[total++] = TEST_FIELD(traverse->type, OperationExtend) ? "On" : "Off";
		}

		if(locationType != kLocation_IndirectIndexed)
		{
			switch(operation)
			{
				default:
				{
					/* do mask field */
					int numChars;

					if(traverse->flags & kActionFlag_IndexAddress)
					{
						menuItemInfo[total].extra_data = 0xFFFFFFFF;
						numChars = 8;
					}
					else
					{
						menuItemInfo[total].extra_data = kCheatSizeMaskTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
						numChars = kCheatSizeDigitsTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
					}

					sprintf(extendDataBuf[i], "%.*X", numChars, traverse->extendData & kCheatSizeMaskTable[EXTRACT_FIELD(traverse->type, BytesUsed)]);

					menuItemInfo[total].sub_cheat = i;
					menuItemInfo[total].field_type = kType_WriteMask;
					menuItem[total] = "Mask";
					menuSubItem[total++] = extendDataBuf[i];
				}
				break;

				case kOperation_AddSubtract:
				{
					{
						/* do add/subtract field */
						menuItemInfo[total].sub_cheat = i;
						menuItemInfo[total].field_type = kType_AddSubtract;
						menuItem[total] = "Add/Subtract";
						menuSubItem[total++] = kAddSubtractNames[operationParameter];
					}

					if(operationParameter)
					{
						/* do subtract minimum field */
						sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

						menuItemInfo[total].sub_cheat = i;
						menuItemInfo[total].field_type = kType_SubtractMinimum;
						menuItem[total] = "Minimum Boundary";
						menuSubItem[total++] = extendDataBuf[i];
					}
					else
					{
						/* do add maximum field */
						sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

						menuItemInfo[total].sub_cheat = i;
						menuItemInfo[total].field_type = kType_AddMaximum;
						menuItem[total] = "Maximum Boundary";
						menuSubItem[total++] = extendDataBuf[i];
					}
				}
				break;

				case kOperation_ForceRange:
				{
					{
						/* do range minimum field */
						if(!EXTRACT_FIELD(traverse->type, BytesUsed))
							sprintf(extendDataBuf[i], "%.2X", (traverse->extendData >> 8) & 0xFF);
						else
							sprintf(extendDataBuf[i], "%.4X", (traverse->extendData >> 16) & 0xFFFF);

						menuItemInfo[total].sub_cheat = i;
						menuItemInfo[total].field_type = kType_RangeMinimum;
						menuItem[total] = "Range Minimum";
						menuSubItem[total++] = extendDataBuf[i];
					}

					{
						/* do range maximum field */
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

						menuItemInfo[total].sub_cheat = i;
						menuItemInfo[total].field_type = kType_RangeMaximum;
						menuItem[total++] = "Range Maximum";
					}
				}
				break;

				case kOperation_SetOrClearBits:
				{
					/* do set/clear field */
					menuItemInfo[total].sub_cheat = i;
					menuItemInfo[total].field_type = kType_SetClear;
					menuItem[total] = "Set/Clear";
					menuSubItem[total++] = kSetClearNames[operationParameter];
				}
				break;
			}
		}

		{
			/* do data field */
			sprintf(dataBuf[i], "%.*X (%d)",
					(int)kCheatSizeDigitsTable[EXTRACT_FIELD(traverse->type, BytesUsed)],
					traverse->originalDataField,
					traverse->originalDataField);

			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Data;
			menuItemInfo[total].extra_data = kCheatSizeMaskTable[EXTRACT_FIELD(traverse->type, BytesUsed)];
			menuItem[total] = "Data";
			menuSubItem[total++] = dataBuf[i];
		}

		{
			/* do user select field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_UserSelect;
			menuItem[total] = "User Select";
			menuSubItem[total++] = EXTRACT_FIELD(traverse->type, UserSelectEnable) ? "On" : "Off";
		}

		{
			/* do user select minimum displayed value field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_UserSelectMinimumDisp;
			menuItem[total] = "Minimum Displayed Value";
			menuSubItem[total++] = kNumbersTable[EXTRACT_FIELD(traverse->type, UserSelectMinimumDisplay)];
		}

		{
			/* do user select minimum value field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_UserSelectMinimum;
			menuItem[total] = "Minimum Value";
			menuSubItem[total++] = kNumbersTable[EXTRACT_FIELD(traverse->type, UserSelectMinimum)];
		}

		{
			/* do user select BCD field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_UserSelectBCD;
			menuItem[total] = "BCD";
			menuSubItem[total++] = TEST_FIELD(traverse->type, UserSelectBCD) ? "On" : "Off";
		}

		{
			/* do copy previous value field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_CopyPrevious;
			menuItem[total] = "Copy Previous Value";
			menuSubItem[total++] = TEST_FIELD(traverse->type, LinkCopyPreviousValue) ? "On" : "Off";
		}

		{
			/* do prefill field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Prefill;
			menuItem[total] = "Prefill";
			menuSubItem[total++] = kPrefillNames[EXTRACT_FIELD(traverse->type, Prefill)];
		}

		{
			/* do byte length field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_ByteLength;
			menuItem[total] = "Byte Length";
			menuSubItem[total++] = kByteSizeStringList[EXTRACT_FIELD(traverse->type, BytesUsed)];
		}

		{
			/* do endianness field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Endianness;
			menuItem[total] = "Endianness";
			menuSubItem[total++] = kEndiannessNames[EXTRACT_FIELD(traverse->type, Endianness)];
		}

		{
			/* do location type field */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_LocationType;
			menuItem[total] = "Location";

			if(locationType == kLocation_Custom)
				/* "Comment", "EEPROM", "Select" */
				menuSubItem[total++] = kCustomLocationNames[locationParameter];
			else
				/* "Normal", "Region", "Mapped Memory", "Relative Address" */
				menuSubItem[total++] = kLocationNames[locationType];
		}

		if(locationType != kLocation_IndirectIndexed)
		{
			if(locationType != kLocation_MemoryRegion)
			{
				/* do cpu field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_CPU;
				menuItem[total] = "CPU";
				menuSubItem[total++] = kNumbersTable[locationParameter];
			}
			else
			{
				/* do region field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_Region;
				menuItem[total] = "Region";
				menuSubItem[total++] = kRegionNames[locationParameter];
			}
		}
		else
		{
			{
				/* do packed CPU field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_PackedCPU;
				menuItem[total] = "CPU";
				menuSubItem[total++] = kNumbersTable[EXTRACT_FIELD(traverse->type, LocationParameterCPU)];
			}

			{
				/* do packed size field */
				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_PackedSize;
				menuItem[total] = "Address Size";
				menuSubItem[total++] = kNumbersTable[(locationParameter & 3) + 1];
			}

			{
				/* do address index field */
				if(traverse->extendData & 0x80000000)
				{
					/* swap if negative */
					int temp = traverse->extendData;

					temp = -temp;

					sprintf(extendDataBuf[i], "-%.8X", temp);
				}
				else
					sprintf(extendDataBuf[i], "%.8X", traverse->extendData);

				menuItemInfo[total].sub_cheat = i;
				menuItemInfo[total].field_type = kType_AddressIndex;
				menuItem[total] = "Address Index";
				menuSubItem[total++] = extendDataBuf[i];
			}
		}

		{
			/* do address field */
			cpu_region_info *info = get_cpu_info(0);

			switch(EXTRACT_FIELD(traverse->type, LocationType))
			{
				case kLocation_Standard:
				case kLocation_HandlerMemory:
				case kLocation_MemoryRegion:
				case kLocation_IndirectIndexed:
					menuItemInfo[total].extra_data = info->addressMask;
					break;

				default:
					menuItemInfo[total].extra_data = 0xFFFFFFFF;
			}

			sprintf(addressBuf[i], "%.*X", get_address_length(EXTRACT_FIELD(traverse->type, LocationParameter)), traverse->address);

			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Address;
			menuItem[total] = "Address";
			menuSubItem[total++] = addressBuf[i];
		}

		if(i < entry->actionListLength - 1)
		{
			/* do divider */
			menuItemInfo[total].sub_cheat = i;
			menuItemInfo[total].field_type = kType_Divider;
			menuItem[total] = MENU_SEPARATOR_ITEM;
			menuSubItem[total++] = NULL;
		}
	}

	/* ##### RETURN ##### */
	menuItemInfo[total].sub_cheat = 0;
	menuItemInfo[total].field_type = kType_Return;
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARREY ##### */
	menuItemInfo[total].sub_cheat = 0;
	menuItemInfo[total].field_type = kType_Return;
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* adjust cursor position */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	info = &menuItemInfo[sel];
	action = &entry->actionList[info->sub_cheat];

	/* higlighted sub-item if edit mode */
	if(editActive)
		flagBuf[sel] = 1;

	/* draw it */
	old_style_menu(menuItem, menuSubItem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;
		dirty = 1;

		switch(info->fieldType)
		{
/*          case kType_ActivationKey1:
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
			case kType_Link:
				TOGGLE_MASK_FIELD(action->type, LinkEnable);
				break;

			case kType_LinkExtension:
				TOGGLE_MASK_FIELD(action->type, LinkExtension);
				break;

			case kType_Type:
			{
				UINT8 handled = 0;

				if(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom)
				{
					UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

					if(locationParameter == kCustomLocation_Comment)
					{
						/* "Comment" -> "Watch" */
						SET_FIELD(action->type, LocationParameter, 0);
						SET_FIELD(action->type, LocationType, kLocation_Standard);
						SET_FIELD(action->type, Type, kType_Watch);
						SET_FIELD(action->type, Operation, kOperation_None);

						handled = 1;
					}
					else if(locationParameter == kCustomLocation_Select)
					{
						/* "Select" -> "Comment" */
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Comment);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);

						handled = 1;
					}
				}

				if(!handled)
				{
					UINT32 type = EXTRACT_FIELD(action->type, Type);

					if(type == kType_NormalOrDelay)
					{
						/* "Normal/Delay" -> "Select" */
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Select);
						SET_FIELD(action->type, Type, 0);
					}
					else
						SET_FIELD(action->type, Type, type - 1);
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
			{
				UINT32 delay = (EXTRACT_FIELD(action->type, TypeParameter) - 1) & 7;

				SET_FIELD(action->type, TypeParameter, delay);
			}
			break;

			case kType_WatchSize:
				action->originalData = (action->originalData & 0xFFFFFF00) | ((action->originalData - 0x00000001) & 0x000000FF);
				action->data = action->originalData;
				break;

			case kType_WatchSkip:
				action->original_data = (action->originalData & 0xFFFF00FF) | ((action->originalData - 0x00000100) & 0x0000FF00);
				action->data = action->originalData;
				break;

			case kType_WatchPerLine:
				action->originalData = (action->originalData & 0xFF00FFFF) | ((action->originalData - 0x00010000) & 0x00FF0000);
				action->data = action->originalData;
				break;

			case kType_WatchAddValue:
				action->original_data = (action->originalData & 0x00FFFFFF) | ((action->originalData - 0x01000000) & 0xFF000000);
				action->data = action->originalData;
				break;

			case kType_WatchFormat:
			{
				UINT32 typeParameter = EXTRACT_FIELD(action->type, TypeParameter);

				typeParameter = (typeParameter & 0xFFFFFFFC) | ((typeParameter - 0x00000001) & 0x0000003);
				SET_FIELD(action->type, TypeParameter, typeParameter);
			}
			break;

			case kType_WatchLabel:
				SET_FIELD(action->type, TypeParameter, EXTRACT_FIELD(action->type, TypeParameter) ^ 0x00000004);
				break;

			case kType_Operation:
				if(EXTRACT_FIELD(action->type, Operation) == kOperation_WriteMask)
				{
					if(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed)
						SET_FIELD(action->type, Operation, kOperation_SetOrClearBits);
					else
						SET_FIELD(action->type, Operation, kOperation_ForceRange);
				}
				else
					SET_FIELD(action->type, Operation, EXTRACT_FIELD(action->type, Operation) - 1);
				break;

			case kType_OperationExtend:
				TOGGLE_MASK_FIELD(action->type, OperationExtend);
				break;

			case kType_WriteMask:
				action->extendData -= 1;
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

			case kType_SubtractMinimum:
			case kType_AddMaximum:
			case kType_AddressIndex:
				action->extendData -= 1;
				break;

			case kType_AddSubtract:
			case kType_SetClear:
				TOGGLE_MASK_FIELD(action->type, OperationParameter);
				break;

			case kType_Data:
				action->originalData -= 1;
				action->originalData &= info->extraData;
				action->data = action->originalData;
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

			case kType_CopyPrevious:
				TOGGLE_MASK_FIELD(action->type, LinkCopyPreviousValue);
				break;

			case kType_Prefill:
			{
				UINT32 prefill = (EXTRACT_FIELD(action->type, Prefill) - 1) & 3;

				SET_FIELD(action->type, Prefill, prefill);
			}
			break;

			case kType_ByteLength:
			{
				UINT32 length = (EXTRACT_FIELD(action->type, BytesUsed) - 1) & 3;

				SET_FIELD(action->type, BytesUsed, length);
			}
			break;

			case kType_Endianness:
				TOGGLE_MASK_FIELD(action->type, Endianness);
				break;

			case kType_LocationType:
			{
				UINT32 locationType = EXTRACT_FIELD(action->type, LocationType);

				switch(locationType)
				{
					case kLocation_Standard:
						/* "Normal" -> "Relative Address" */
						SET_FIELD(action->type, LocationType, kLocation_IndirectIndexed);
						SET_FIELD(action->type, LocationParameter, 0);
						break;

					case kLocation_Custom:
						/* "EEPROM" -> "Mapped Memory" */
						SET_FIELD(action->type, LocationType, kLocation_HandlerMemory);
						SET_FIELD(action->type, LocationParameter, 0);
						break;

					case kLocation_IndirectIndexed:
						/* "Relative Address" -> "EEPROM" */
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, LocationParameter, kCustomLocation_EEPROM);
						break;

					default:
						locationType--;
						SET_FIELD(action->type, LocationType, locationType);
				}
			}
			break;

			case kType_CPU:
			case kType_Region:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = (locationParameter - 1) & 31;

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedCPU:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameterCPU);

				locationParameter = (locationParameter - 1) & 7;

				SET_FIELD(action->type, LocationParameterCPU, locationParameter);
			}
			break;

			case kType_PackedSize:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameterOption);

				locationParameter = (locationParameter - 1) & 3;

				SET_FIELD(action->type, LocationParameterOption, locationParameter);
			}
			break;

			case kType_Address:
				action->address -= 1;
				action->address &= info->extraData;
				break;

			case kType_Return:
			case kType_Divider:
				break;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		editActive = 0;
		dirty = 1;

		switch(info->fieldType)
		{
/*          case kType_ActivationKey1:
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
			case kType_Link:
				TOGGLE_MASK_FIELD(action->type, LinkEnable);
				break;

			case kType_LinkExtension:
				TOGGLE_MASK_FIELD(action->type, LinkExtension);
				break;

			case kType_Type:
			{
				UINT8 handled = 0;

				if(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom)
				{
					UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

					if(locationParameter == kCustomLocation_Comment)
					{
						/* "Comment" -> "Select" */
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Select);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);

						handled = 1;
					}
				}
				else if((EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom) &&
						(EXTRACT_FIELD(action->type, LocationParameter) == kCustomLocation_Select))
				{
					/* "Select" -> "Normal/Delay" */
					SET_FIELD(action->type, LocationParameter, 0);
					SET_FIELD(action->type, LocationType, kLocation_Standard);
					SET_FIELD(action->type, Type, 0);

					handled = 1;
				}

				if(!handled)
				{
					UINT32 type = EXTRACT_FIELD(action->type, Type);

					if(type == kType_Watch)
					{
						/* "Watch" -> "Comment" */
						SET_FIELD(action->type, LocationParameter, kCustomLocation_Comment);
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, Type, 0);
					}
					else
						SET_FIELD(action->type, Type, type + 1);
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
			{
				UINT32 delay = (EXTRACT_FIELD(action->type, TypeParameter) + 1) & 7;

				SET_FIELD(action->type, TypeParameter, delay);
			}
			break;

			case kType_WatchSize:
				action->originalData = (action->originalData & 0xFFFFFF00) | ((action->originalData + 0x00000001) & 0x000000FF);
				action->data = action->originalData;
				break;

			case kType_WatchSkip:
				action->originalData = (action->originalData & 0xFFFF00FF) | ((action->originalData + 0x00000100) & 0x0000FF00);
				action->data = action->originalData;
				break;

			case kType_WatchPerLine:
				action->originalData = (action->originalData & 0xFF00FFFF) | ((action->originalData + 0x00010000) & 0x00FF0000);
				action->data = action->originalData;
				break;

			case kType_WatchAddValue:
				action->originalData = (action->originalData & 0x00FFFFFF) | ((action->originalData + 0x01000000) & 0xFF000000);
				action->data = action->originalData;
				break;

			case kType_WatchFormat:
			{
				UINT32 typeParameter = EXTRACT_FIELD(action->type, TypeParameter);

				typeParameter = (typeParameter & 0xFFFFFFFC) | ((typeParameter + 0x00000001) & 0x0000003);
				SET_FIELD(action->type, TypeParameter, typeParameter);
			}
			break;

			case kType_WatchLabel:
				SET_FIELD(action->type, TypeParameter, EXTRACT_FIELD(action->type, TypeParameter) ^ 0x00000004);
				break;

			case kType_Operation:
			{
				CLEAR_MASK_FIELD(action->type, OperationExtend);

				if(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed)
				{
					if(EXTRACT_FIELD(action->type, Type) >= kOperation_SetOrClearBits)
						SET_FIELD(action->type, Operation, 0);
					else
						SET_FIELD(action->type, Operation, EXTRACT_FIELD(action->type, Operation) + 1);
				}
				else if(EXTRACT_FIELD(action->type, Operation) >= kOperation_ForceRange)
					SET_FIELD(action->type, Operation, 0);
				else
					SET_FIELD(action->type, Operation, EXTRACT_FIELD(action->type, Operation) + 1);
			}
			break;

			case kType_OperationExtend:
				TOGGLE_MASK_FIELD(action->type, OperationExtend);
				break;

			case kType_WriteMask:
				action->extendData += 1;
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
				action->extendData += 1;
				break;

			case kType_AddSubtract:
			case kType_SetClear:
				TOGGLE_MASK_FIELD(action->type, OperationParameter);
				break;

			case kType_Data:
				action->originalData += 1;
				action->originalData &= info->extraData;
				action->data = action->originalData;
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

			case kType_CopyPrevious:
				TOGGLE_MASK_FIELD(action->type, LinkCopyPreviousValue);
				break;

			case kType_Prefill:
			{
				UINT32 prefill = (EXTRACT_FIELD(action->type, Prefill) + 1) & 3;

				SET_FIELD(action->type, Prefill, prefill);
			}
				break;

			case kType_ByteLength:
			{
				UINT32 length = (EXTRACT_FIELD(action->type, BytesUsed) + 1) & 3;

				SET_FIELD(action->type, BytesUsed, length);
			}
			break;

			case kType_Endianness:
				TOGGLE_MASK_FIELD(action->type, Endianness);
				break;

			case kType_LocationType:
			{
				UINT32 locationType = EXTRACT_FIELD(action->type, LocationType);

				switch(locationType)
				{
					case kLocation_HandlerMemory:
						/* "Mapped Memory" -> "EEPROM" */
						SET_FIELD(action->type, LocationType, kLocation_Custom);
						SET_FIELD(action->type, LocationParameter, kCustomLocation_EEPROM);
						break;

					case kLocation_Custom:
						/* "EEPROM" -> "Relative Address" */
						SET_FIELD(action->type, LocationType, kLocation_IndirectIndexed);
						SET_FIELD(action->type, LocationParameter, 0);
						break;

					case kLocation_IndirectIndexed:
						/* "Relative Address" -> "Normal" */
						SET_FIELD(action->type, LocationType, kLocation_Standard);
						SET_FIELD(action->type, LocationParameter, 0);
						break;

					default:
						locationType++;
						SET_FIELD(action->type, LocationType, locationType);
				}
			}
			break;

			case kType_CPU:
			case kType_Region:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameter);

				locationParameter = (locationParameter + 1) & 31;

				SET_FIELD(action->type, LocationParameter, locationParameter);
			}
			break;

			case kType_PackedCPU:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameterCPU);

				locationParameter = (locationParameter + 1) & 7;

				SET_FIELD(action->type, LocationParameterCPU, locationParameter);
			}
			break;

			case kType_PackedSize:
			{
				UINT32 locationParameter = EXTRACT_FIELD(action->type, LocationParameterOption);

				locationParameter = (locationParameter + 1) & 3;

				SET_FIELD(action->type, LocationParameterOption, locationParameter);
			}
			break;

			case kType_Address:
				action->address += 1;
				action->address &= info->extraData;
				break;

			case kType_Return:
			case kType_Divider:
				break;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(--sel < 0)
			sel = total - 1;

		editActive = 0;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(++sel >= total)
			sel = 0;

		editActive = 0;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;

		editActive = 0;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;

		editActive = 0;
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
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
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)
			editActive = 0;
		else
			sel = -1;
	}

	/********** EDIT MODE **********/
	if(editActive)
	{
		/* do edit text */
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
					int code = input_code_poll_switches(FALSE);

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
				UINT32 temp = (action->originalData >> 0) & 0xFF;

				temp++;
				temp = do_edit_hex_field(machine, temp) & 0xFF;
				temp--;

				action->originalData = (action->originalData & 0xFFFFFF00) | ((temp << 0) & 0x000000FF);
				action->data = action->originalData;
			}
			break;

			case kType_WatchSkip:
			{
				UINT32 temp = (action->originalData >> 8) & 0xFF;

				temp = do_edit_hex_field(machine, temp) & 0xFF;

				action->originalData = (action->originalData & 0xFFFF00FF) | ((temp << 8) & 0x0000FF00);
				action->data = action->originalData;
			}
			break;

			case kType_WatchPerLine:
			{
				UINT32 temp = (action->originalData >> 16) & 0xFF;

				temp = do_edit_hex_field(machine, temp) & 0xFF;

				action->originalData = (action->originalData & 0xFF00FFFF) | ((temp << 16) & 0x00FF0000);
				action->data = action->originalData;
			}
			break;

			case kType_WatchAddValue:
			{
				UINT32 temp = (action->originalData >> 24) & 0xFF;

				temp = DoEditHexFieldSigned(temp, 0xFFFFFF80) & 0xFF;

				action->originalData = (action->originalData & 0x00FFFFFF) | ((temp << 24) & 0xFF000000);
				action->data = action->originalData;
			}
			break;

			case kType_WriteMask:
				action->extendData = do_edit_hex_field(machine, action->extendData);
				action->extendData &= info->extraData;
				break;

			case kType_AddMaximum:
			case kType_SubtractMinimum:
				action->extendData = do_edit_hex_field(machine, action->extendData);
				break;

			case kType_RangeMinimum:
			{
				UINT32 temp;

				if(!TEST_FIELD(action->type, BytesUsed))
				{
					temp = (action->extendData >> 8) & 0xFF;
					temp = do_edit_hex_field(machine, temp) & 0xFF;

					action->extendData = (action->extendData & 0xFF) | ((temp << 8) & 0xFF00);
				}
				else
				{
					temp = (action->extendData >> 16) & 0xFFFF;
					temp = do_edit_hex_field(machine, temp) & 0xFFFF;

					action->extendData = (action->extendData & 0x0000FFFF) | ((temp << 16) & 0xFFFF0000);
				}
			}
			break;

			case kType_RangeMaximum:
			{
				UINT32 temp;

				if(!TEST_FIELD(action->type, BytesUsed))
				{
					temp = action->extendData & 0xFF;
					temp = do_edit_hex_field(machine, temp) & 0xFF;

					action->extendData = (action->extendData & 0xFF00) | (temp & 0x00FF);
				}
				else
				{
					temp = action->extendData & 0xFFFF;
					temp = do_edit_hex_field(machine, temp) & 0xFFFF;

					action->extendData = (action->extendData & 0xFFFF0000) | (temp & 0x0000FFFF);
				}
			}
			break;

			case kType_Data:
				action->originalData = do_edit_hex_field(machine, action->originalData);
				action->originalData &= info->extraData;
				action->data = action->originalData;
				break;

			case kType_Address:
				action->address = do_edit_hex_field(machine, action->address);
				action->address &= info->extraData;
				break;
		}
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
		{
			save_cheat_code(machine, entry);
		}
		else if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
		{
			watch_cheat_entry(entry, 0);
		}
		else if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
		{
			AddActionBefore(entry, info->sub_cheat);

			entry->actionList[info->sub_cheat].extendData = ~0;

			for(i = 0; i < entry->action_list_length; i++)
			{
				cheat_action *action = &entry->action_list[i];

				if(i)
					SET_MASK_FIELD(action->type, LinkEnable);
				else
					CLEAR_MASK_FIELD(action->type, LinkEnable);
			}
		}
		else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
		{
			if(!info->sub_cheat)
			{
				/* don't delete MASTER Action due to crash */
				SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
			}
			else
				DeleteActionAt(entry, info->sub_cheat);
		}
	}

	if(dirty)
	{
		UpdateCheatInfo(entry, 0);

		entry->flags |= kCheatFlag_Dirty;
	}

	if(sel == -1)
	{
		/* NOTE : building label index table should be done when exit the edit menu */
		if(entry->flags & kCheatFlag_Select)
			build_label_index_table(entry);

		check_code_format(entry);

		editActive = 0;
		dirty = 1;
	}

	/* free astrings used by displaying the name of activation key */
	astring_free(tempString1);
	astring_free(tempString2);
#endif
	return menu->sel;
}

/*--------------------------------------------------------------
  view_cheat_menu - management for code viewer for new format
--------------------------------------------------------------*/

static int view_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	static const char *const kAddressSpaceNames[] = {
		"Program", "Data Space", "I/O Space", "Opcode RAM", "Mapped Memory", "Direct Memory", "EEPROM" };

	static const char *const kCodeTypeNames[] = {
		"Write", "IWrite", "RWrite", "CWrite", "CBit", "PDWWrite", "Move", "Branch", "Loop", "Popup", "Watch" };

	static const char *const kCustomCodeTypeNames[] = {
		"NOP", "Comment", "Separator", "Label Select", "Layer Tag",
		"Unused 1", "Unused2", "Unused 3",
		/* NOTE : the following command codes should be removed when database is loaded */
		"Activation Key 1", "Activation Key 2", "Pre-enable", "Overclock", "Refresh Rate" };

	static const char *const kIWriteParameterNames[] = {
		"Write", "Bit Set", "Bit Clear", "Limited Mask" };

	static const char *const kCBitParameterNames[] = {
		"Bit Set", "Bit Clear", "Limited Mask" };

	static const char *const kConditionNames[] = {
		"Equal", "Not Equal", "Less", "Less or Equal", "Greater", "Greater or Equal", "Bit Set", "Bit Clear",
		"Unused 1", "Unused 2", "Unused 3", "Unused 4",
		"Previous Value", "Key Pressed", "Key Hold", "TRUE" };

	static const char *const kCBitConditionNames[] = {
		"Bis Set", "Bit Clear" };

	static const char *const kDataNames[] = {
		"Data", "Data", "Data", "Data", "Bit", "1st Data", "Add Value", "Jump Index", "-----", "-----", "-----" };

	static const char *const kPopupOptionNames[] = {
		"Label", "Value", "Label + Value", "Value + Label" };

	enum{
		kViewMenu_Header = 1,						/* format : [total index / current index] */
		kViewMenu_Layer,							/* internal value of code and layer index */
		kViewMenu_Name,								/* master code = entry->name, linked code = action->optionalName */
			kViewMenu_Comment,
			kViewMenu_ActivationKey1,
			kViewMenu_ActivationKey2,
		/* if(CustomCode == 0) */
			kViewMenu_CPURegion,					/* add address space if CPU region */
			kViewMenu_CodeType,
			/* if(CodeType == IWrite) */
				kViewMenu_IWriteParameter,
			/* if(CodeType == CWrite, CBit, Branch, Popup) */
				kViewMenu_Condition,
			/* if(CodeType == CBit) */
				kViewMenu_CBitParameter,
				kViewMenu_CBitCondition,
			/* if(CodeType == Watch) */
				kViewMenu_WatchParameter,
			/* if(CodeType == Popup) */
				kViewMenu_PopupParameter,
			/* if(CodeType == RWrite, CWrite, CBit, Branch, Popup */
				kViewMenu_Extend,
			kViewMenu_Address,
			kViewMenu_AddressSize,
			/* if(CodeType == IWrite, Move) */
				kViewMenu_IndexOffset,
				kViewMenu_IndexAddressSize,
			/* if(CodeType == Move) */
				kViewMenu_MoveParameter,			/* variable 0-3 */
			kViewMenu_Data,
			/* if(LimitedMask) */
				kViewMenu_LimitedMask,				/* for IWrite, CBit */
			/* if(CodeType == PDWWRite) */
				kViewMenu_SubData,
				kViewMenu_SubDataSize,
			/* if(ValueSelect) */
				kViewMenu_ValueSelectOptions,
			kViewMenu_Options,						/* One-shot, Delay, Return, Prefill, RestoreValue */
		/* if(CustomCode) */
			/* if(LayerIndex) */
				kViewMenu_LayerDisplayLevel,
				kViewMenu_LayerSetLevel,
				kViewMenu_LayerSetLength,
		kViewMenu_Return,
		kViewMenu_Max };

	int				total = 0;
	int				menu_index[kViewMenu_Max] = { 0 };
	static int		page_index = 0;
	INT8			code_type = 0;
	INT8			address_length = 0;
	INT8			address_size = 0;
	INT8			request_arrow = 0;
	const char		**menu_item;
	const char		**menu_sub_item;
	char			*flag_buf;
	char			*header;
	char			*buf_code;
	char			*buf_type;
	char			*buf_cpu;
	char			*buf_address;
	char			*buf_size;
	char			*buf_sub_size;
	char			*buf_data;
	char			*buf_extend;
	char			*buf_limited_mask;
	char			*buf_options;
	char			*buf_code_type_options;
	cheat_entry		*entry = &cheat_list[menu->pre_sel];
	cheat_action	*action = NULL;
	astring			*activation_key_string1 = NULL;
	astring			*activation_key_string2 = NULL;

	/* first setting : forced set page as 0 when first open */
	if(menu->first_time)
	{
		page_index = 0;
		menu->first_time = 0;
	}

	action			= &entry->action_list[page_index];
	code_type		= EXTRACT_FIELD(action->type, CodeType);
	address_length	= get_address_length(action->region);
	address_size	= EXTRACT_FIELD(action->type, AddressSize);

	/* required items = (total items + return + terminator) + (strings buf * 12) */
	request_strings(kViewMenu_Max + 2, 12, 32, 0);
	menu_item				= menu_strings.main_list;
	menu_sub_item			= menu_strings.sub_list;
	flag_buf				= menu_strings.flag_list;
	header					= menu_strings.main_strings[0];
	buf_code				= menu_strings.main_strings[1];
	buf_type				= menu_strings.main_strings[1];
	buf_cpu					= menu_strings.main_strings[3];
	buf_address				= menu_strings.main_strings[4];
	buf_size				= menu_strings.main_strings[5];
	buf_sub_size			= menu_strings.main_strings[6];
	buf_data				= menu_strings.main_strings[7];
	buf_extend				= menu_strings.main_strings[8];
	buf_limited_mask		= menu_strings.main_strings[9];
	buf_options				= menu_strings.main_strings[10];
	buf_code_type_options	= menu_strings.main_strings[11];

	memset(flag_buf, 0, kViewMenu_Max + 2);

	/********** MENU CONSTRUCTION **********/
	/* ##### HEADER ##### */
	sprintf(header, "%2.2d/%2.2d", page_index + 1, entry->action_list_length);
	ADD_MENU_3_ITEMS("Page", header, kViewMenu_Header);

	/* ##### LAYER INDEX ##### */
	sprintf(buf_code, "%2.2X", entry->layer_index);
	ADD_MENU_3_ITEMS("Layer Index", buf_code, kViewMenu_Layer);

	/* ##### NAME ##### */
	if(entry->name)
	{
		/* NOTE : master code is from cheat entry and others are from cheat action */
		ADD_MENU_3_ITEMS("Name", page_index ? action->optional_name : entry->name, kViewMenu_Name);
	}
	else
		ADD_MENU_3_ITEMS("Name", "< none >", kViewMenu_Name);

	/* ##### COMMENT ##### */
	if(page_index == 0 && entry->comment)
		ADD_MENU_3_ITEMS("Comment", entry->comment, kViewMenu_Comment);

	/* ##### ACTIVATION KEY ##### */
	if(entry->flags & kCheatFlag_Select)
	{
		if(entry->flags & kCheatFlag_HasActivationKey1)
		{
			activation_key_string1 = astring_alloc();
			ADD_MENU_3_ITEMS("Activation Key - Prev", astring_c(input_code_name(activation_key_string1, entry->activation_key_1)), kViewMenu_ActivationKey1);
		}

		if(entry->flags & kCheatFlag_HasActivationKey2)
		{
			activation_key_string2 = astring_alloc();
			ADD_MENU_3_ITEMS("Activation Key - Next", astring_c(input_code_name(activation_key_string2, entry->activation_key_2)), kViewMenu_ActivationKey2);
		}
	}
	else
	{
		if(entry->flags & kCheatFlag_HasActivationKey1)
		{
			activation_key_string1 = astring_alloc();
			ADD_MENU_3_ITEMS("Activation Key", astring_c(input_code_name(activation_key_string1, entry->activation_key_1)), kViewMenu_ActivationKey1);
		}
	}

	/* ##### CPU/REGION ##### */
	if((action->flags & kActionFlag_Custom) == 0)
	{
		if(action->region < REGION_INVALID)
		{
			/* cpu region */
			sprintf(buf_cpu, "%s (%s)", kRegionNames[EXTRACT_FIELD(action->region, CPUIndex) + 1], kAddressSpaceNames[EXTRACT_FIELD(action->region, AddressSpace)]);
			ADD_MENU_3_ITEMS("CPU", buf_cpu, kViewMenu_CPURegion);
		}
		else
			/* non-cpu region */
			ADD_MENU_3_ITEMS("Region", kRegionNames[action->region - REGION_INVALID], kViewMenu_CPURegion);
	}

	/* ##### CODE TYPE ##### */
	if(action->flags & kActionFlag_Custom)
	{
		ADD_MENU_3_ITEMS("Custom Type", kCustomCodeTypeNames[action->region - CUSTOM_CODE], kViewMenu_CodeType);
	}
	else
	{
		ADD_MENU_3_ITEMS("Code Type", kCodeTypeNames[code_type], kViewMenu_CodeType);
	}

	if((action->flags & kActionFlag_Custom) == 0)
	{
		/* ##### CODE PARAMETER ##### */
		if(code_type == kCodeType_IWrite)
		{
			/* writing parameter */
			ADD_MENU_3_ITEMS("Parameter", kIWriteParameterNames[EXTRACT_FIELD(action->type, CodeParameterUpper)], kViewMenu_IWriteParameter);
		}

		if(code_type == kCodeType_RWrite)
		{
			/* repeat parameter */
			sprintf(buf_extend, "%4.4X / %4.4X",
						EXTRACT_FIELD(action->extend_data, LSB16),
						EXTRACT_FIELD(action->extend_data, MSB16));
			ADD_MENU_3_ITEMS("Count / Skip", buf_extend, kViewMenu_Extend);
		}

		if(action->flags & kActionFlag_CheckCondition)
		{
			if(code_type == kCodeType_CBit)
			{
				/* bit parameter for CBit */
				ADD_MENU_3_ITEMS("Parameter", kCBitParameterNames[EXTRACT_FIELD(action->type, CodeParameterUpper)], kViewMenu_CBitParameter);

				/* bit condition for CBit */
				ADD_MENU_3_ITEMS("Condition", kCBitConditionNames[EXTRACT_FIELD(action->type, CodeParameterLower)], kViewMenu_CBitCondition);

			}
			else
			{
				/* condition for CWrite, Branch, Popup */
				ADD_MENU_3_ITEMS("Condition", kConditionNames[EXTRACT_FIELD(action->type, CodeParameter)], kViewMenu_Condition);
			}

			if(EXTRACT_FIELD(action->type, CodeParameter) != kCondition_PreviousValue)
			{
				sprintf(buf_extend, "%*.*X", kCheatSizeDigitsTable[address_size], kCheatSizeDigitsTable[address_size], action->extend_data);
				ADD_MENU_3_ITEMS("Comparison", buf_extend, kViewMenu_Extend);
			}
		}

		if(code_type == kCodeType_Popup)
		{
			/* popup parameter */
			ADD_MENU_3_ITEMS("Parameter", kPopupOptionNames[EXTRACT_FIELD(action->type, PopupParameter)], kViewMenu_PopupParameter);
		}

		/* ##### ADDRESS ##### */
		switch(EXTRACT_FIELD(action->type, AddressRead))
		{
			case kReadFrom_Address:
				sprintf(buf_address, "%*.*X", address_length, address_length, action->original_address);
				break;

			case kReadFrom_Index:
				sprintf(buf_address, "(V%s)", kNumbersTable[action->original_address]);
				break;

			case kReadFrom_Variable:
				sprintf(buf_address, "V%s", kNumbersTable[action->original_address]);
		}

		ADD_MENU_3_ITEMS("Address", buf_address, kViewMenu_Address);

		/* ##### SIZE ##### */
		sprintf(buf_size, "%s", kByteSizeStringList[address_size]);
		ADD_MENU_3_ITEMS("Size", buf_size, kViewMenu_AddressSize);

		/* ##### INDEX ##### */
		if(action->flags & kActionFlag_IndexAddress)
		{
			/* index offset for IWrite, Move */
			sprintf(buf_extend, "%*.*X", address_length, address_length, action->extend_data);
			ADD_MENU_3_ITEMS("Index Offset", buf_extend, kViewMenu_IndexOffset);

			/* index size for IWrite, Move */
			sprintf(buf_sub_size, "%s", kByteSizeStringList[EXTRACT_FIELD(action->type, CodeParameterLower)]);
			ADD_MENU_3_ITEMS("Index Size", buf_sub_size, kViewMenu_IndexAddressSize);
		}

		/* ##### DESTINATION ##### */
		if(code_type == kCodeType_Move)
		{
			sprintf(buf_type, "%s", kNumbersTable[EXTRACT_FIELD(action->type, CodeParameterUpper)]);
			ADD_MENU_3_ITEMS("Destination", buf_type, kViewMenu_MoveParameter);
		}

		/* ##### DATA ##### */
		if(action->flags & kActionFlag_MemoryWrite)
		{
			if(action->flags & kActionFlag_LimitedMask)
			{
				/* NOTE : data is upper 2 bytes from data field in case of limited mask */
				if(TEST_FIELD(action->type, DataRead))
					sprintf(buf_data, "V%s", kNumbersTable[EXTRACT_FIELD(action->original_data, MSB16)]);
				else
					sprintf(buf_data, "%*.*X", kCheatSizeDigitsTable[address_size], kCheatSizeDigitsTable[address_size], EXTRACT_FIELD(action->original_data, MSB16));
			}
			else
			{
				if(TEST_FIELD(action->type, DataRead))
					sprintf(buf_data, "V%s", kNumbersTable[action->original_data]);
				else
					sprintf(buf_data, "%*.*X", kCheatSizeDigitsTable[address_size], kCheatSizeDigitsTable[address_size], action->original_data);
			}

			ADD_MENU_3_ITEMS(kDataNames[EXTRACT_FIELD(action->type, CodeType)], buf_data, kViewMenu_Data);
		}

		/* ##### MASK ##### */
		if(code_type == kCodeType_Write)
		{
			sprintf(buf_extend, "%*.*X", address_size, address_size, action->extend_data);
			ADD_MENU_3_ITEMS("Mask", buf_extend, kViewMenu_Extend);
		}
		else if(action->flags & kActionFlag_LimitedMask)
		{
			/* NOTE : mask is lower 2 bytes from data field in case of limited mask */
			sprintf(buf_limited_mask, "%*.*X", address_size, address_size, EXTRACT_FIELD(action->original_data, LSB16));
			ADD_MENU_3_ITEMS("Mask", buf_limited_mask, kViewMenu_LimitedMask);
		}

		/* ##### SUB DATA ##### */
		if(code_type == kCodeType_PDWWrite)
		{
			/* 2nd data */
			sprintf(buf_extend, "%*.*X",
				kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, CodeParameterLower)],
				kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, CodeParameterLower)],
				action->extend_data);
			ADD_MENU_3_ITEMS("2nd Data", buf_extend, kViewMenu_Extend);

			/* 2nd data size */
			sprintf(buf_sub_size, "%s", kByteSizeStringList[EXTRACT_FIELD(action->type, CodeParameterLower)]);
			ADD_MENU_3_ITEMS("2nd Data Size", buf_sub_size, kViewMenu_SubDataSize);
		}

		/* ##### JUMP INDEX ##### */
		if(code_type == kCodeType_Branch)
		{
			/* jump index for branch */
			sprintf(buf_extend, "%2.2d", action->original_data + 1);
			ADD_MENU_3_ITEMS("Jump Index", buf_extend, kViewMenu_Extend);
		}

		/* ##### OPTIONS ##### */
		sprintf(buf_options, "%s %s %s %s %s",
					TEST_FIELD(action->type, OneShot) ? "One" : "-",
					TEST_FIELD(action->type, DelayEnable) ? "Del" : "-",
					TEST_FIELD(action->type, PrefillEnable) ? "Pre" : "-",
					TEST_FIELD(action->type, Return) ? "Ret" : "-",
					TEST_FIELD(action->type, RestoreValue) ? "Rst" : "-");
		ADD_MENU_3_ITEMS("Options", buf_options, kViewMenu_Options);

		/* ##### CODE TYPE OPTIONS ##### */
		if(TEST_FIELD(action->type, ValueSelectEnable))
		{
			sprintf(buf_code_type_options, "%s %s %s %s",
						TEST_FIELD(action->type, ValueSelectMinimumDisplay) ? "MinD" : "-",
						TEST_FIELD(action->type, ValueSelectMinimum) ? "Min" : "-",
						TEST_FIELD(action->type, ValueSelectBCD) ? "BCD" : "-",
						TEST_FIELD(action->type, ValueSelectNegative) ? "Neg" : "-");
			ADD_MENU_3_ITEMS("Value Select Options", buf_code_type_options, kViewMenu_ValueSelectOptions);
		}
	}
	else
	{
		/* ##### CUSTOM CODE PARAMETER ##### */
		if(action->region == CUSTOM_CODE_LAYER_TAG)
		{
			/* ##### LAYER TAG ##### */
			sprintf(buf_address, "%2.2X", action->original_address);
			ADD_MENU_3_ITEMS("Display Level", buf_address, kViewMenu_LayerDisplayLevel);

			sprintf(buf_data, "%2.2X", action->original_data);
			ADD_MENU_3_ITEMS("Set Level", buf_data, kViewMenu_LayerSetLevel);

			sprintf(buf_extend, "%2.2X", action->extend_data);
			ADD_MENU_3_ITEMS("Set Length", buf_extend, kViewMenu_LayerSetLength);
		}
	}

	/* ##### RETURN ##### */
	ADD_MENU_3_ITEMS("Return to Prior Menu", NULL, kViewMenu_Return);

	/* ##### TERMINATE ARRAY ##### */
	TERMINATE_MENU_2_ITEMS;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	if(menu_index[menu->sel] == kViewMenu_Header)
	{
		if(entry->action_list_length > 1)
			request_arrow = 3;
	}

	/* draw it */
	old_style_menu(menu_item, menu_sub_item, flag_buf, menu->sel, request_arrow);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		if(--page_index < 0)
			page_index = entry->action_list_length - 1;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		if(++page_index > entry->action_list_length - 1)
			page_index = 0;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT) || input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		save_cheat_code(machine, entry);
	}

	if(activation_key_string1)
		astring_free(activation_key_string1);
	if(activation_key_string2)
		astring_free(activation_key_string2);

	return menu->sel + 1;
}

/*---------------------------------------------------------
  analyse_cheat_menu - management for code analizer menu
---------------------------------------------------------*/

static int analyse_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	static const char *const FORMAT_ERROR_MESSAGE_TABLE[] = {
		"Invalid Location Type",
		"Invalid Operation",
		"Invalid Code Type",
		"Invalid Condition",
		"Invalid Code Option",
		"Relative Address conflicts other operations",
		"Maximum range is smaller than minimum",
		"Missing Restore Previous Value flag",
		"Missing label link code",
		"Both Value and Label Select are defined",
		"Invalid Data Field",
		"Invalid Extend Data Field",
		"Don't set Variable in this type",
		"Value Select can't select any value",
		"Limited Mask is over length",
		"RWrite doesn't have counter",
		"Invalid Address Read",
		"Invalid Variable in address field",
		"Invalid Variable in extend data field",
		"CPU/Region is out of range",
		"Invalid CPU/Region number",
		"Invalid Address Space",
		"An address is out of range",
		"Invalid Custom Code" };

	int				i, j;
	int				total			= 0;
	UINT8			is_edit			= 0;
	const char		**menu_item;
	cheat_entry		*entry			= &cheat_list[menu->pre_sel];

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = ((items + header + 2 separators) * total_actions) + edit + return + terminator) */
	request_strings(((kErrorFlag_Max + 3) * entry->action_list_length) + 3, 0, 0, 0);
	menu_item = menu_strings.main_list;

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		UINT32 flags = analyse_code_format(machine, entry, action);

		menu_item[total++] = action->optional_name ? action->optional_name : "(Null)";
		menu_item[total++] = MENU_SEPARATOR_ITEM;

		if(flags)
		{
			for(j = 0; j < kErrorFlag_Max; j++)
			{
				if((flags >> j) & 1)
					menu_item[total++] = FORMAT_ERROR_MESSAGE_TABLE[j];
			}
		}
		else
			menu_item[total++] = "No Problem";

		menu_item[total++] = MENU_SEPARATOR_ITEM;
	}

	menu_item[total++] = "Edit This Entry";
	menu_item[total++] = "Return to Prior Menu";
	menu_item[total] = NULL;

	/* adjust cursor position */
	ADJUST_CURSOR(menu->sel, total);

	old_style_menu(menu_item, NULL, NULL, menu->sel, 0);

	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		is_edit = 1;
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(menu->sel == total - 2)
			is_edit = 1;
		else
			menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	if(is_edit)
	{
		if(entry->flags & kCheatFlag_OldFormat)
			cheat_menu_stack_push(edit_cheat_menu, menu->return_handler, menu->pre_sel);
		else
			cheat_menu_stack_push(view_cheat_menu, menu->return_handler, menu->pre_sel);
	}

	return menu->sel + 1;
}

/*-----------------------------------------------------------
  search_minimum_menu - management for minimum search menu
                        limited size, operand, comparison
-----------------------------------------------------------*/

static int search_minimum_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_CPU = 0,
		kMenu_Memory,
		kMenu_Divider1,
		kMenu_Value,
		kMenu_Timer,
		kMenu_Greater,
		kMenu_Less,
		kMenu_Equal,
		kMenu_NotEqual,
		kMenu_Divider2,
		kMenu_Result,
		kMenu_RestoreSearch,
		kMenu_Return,
		kMenu_Max };

	INT8			i;
	UINT8			total			= 0;
	UINT8			doSearch		= 0;
	static UINT8	doneSaveMemory	= 0;
	const char		* menuItem[kMenu_Max + 1]		= { 0 };
	const char		* menuSubItem[kMenu_Max + 1]	= { 0 };
	char			flagBuf[kMenu_Max + 1]			= { 0 };
	char			cpuBuf[4];
	char			valueBuf[32];		/* "FFFFFFF[F] (4294967295)"   23 chars */
	char			timerBuf[32];		/* "-FFFFFFF[F] (4294967295)"  24 chars */
	char			numResultsBuf[16];
	char			* stringsBuf;
	search_info		*search = get_current_search();

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* NOTE : in this mode, search bytes is always fixed with 1 byte right now */
	search->old_options.value &= kSearchByteMaskTable[search->bytes];
	search->old_options.delta &= kSearchByteMaskTable[search->bytes];

	/********** MENU CONSTRUCTION **********/
	/* ##### CPU ##### */
	menuItem[total] = "CPU";
	sprintf(cpuBuf, "%d", search->target_idx);
	menuSubItem[total++] = cpuBuf;

	/* ##### MEMORY ##### */
	if(doneSaveMemory)
		menuItem[total] = "Initialize Memory";
	else
		menuItem[total] = "Save Memory";
	menuSubItem[total++] = NULL;

	/* ##### DIVIDER ##### */
	menuItem[total] = MENU_SEPARATOR_ITEM;
	menuSubItem[total++] = NULL;

	/* ##### VALUE ##### */
	menuItem[total] = "Value";

	if(editActive && menu->sel == kMenu_Value)
	{
		stringsBuf = valueBuf;

		for(i = 1; i >= 0; i--)
		{
			if(i == editCursor)
				stringsBuf += sprintf(stringsBuf, "[%X]", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
			else
				stringsBuf += sprintf(stringsBuf, "%X", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
		}

		stringsBuf += sprintf(stringsBuf, " (%d)", search->old_options.value);
	}
	else
		sprintf(valueBuf, "%.*X (%d)",
			kSearchByteDigitsTable[search->bytes], search->old_options.value, search->old_options.value);

	menuSubItem[total++] = valueBuf;

	/* ##### TIMER ##### */
	menuItem[total] = "Timer";

	if(!(search->old_options.delta & kSearchByteSignBitTable[search->bytes]))
	{
		/* PLUS */
		if(editActive && menu->sel == kMenu_Timer)
		{
			stringsBuf = timerBuf;

			for(i = 2; i >= 0; i--)
			{
				if(i == editCursor)
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "[+]");
					else
						stringsBuf += sprintf(stringsBuf, "[%X]", (search->old_options.delta & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
				else
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "+");
					else
						stringsBuf += sprintf(stringsBuf, "%X", (search->old_options.delta & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
			}

			stringsBuf += sprintf(stringsBuf, " (+%d)", search->old_options.delta);
		}
		else
			sprintf(timerBuf, "+%.2X (+%d)", search->old_options.delta, search->old_options.delta);
	}
	else
	{
		UINT32 deltaDisp = (~search->old_options.delta + 1) & 0x7F;

		/* MINUS */
		if(editActive && menu->sel == kMenu_Timer)
		{
			stringsBuf = timerBuf;

			for(i = 2; i >= 0; i--)
			{
				if(i == editCursor)
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "[-]");
					else
						stringsBuf += sprintf(stringsBuf, "[%X]", (deltaDisp & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
				else
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "-");
					else
						stringsBuf += sprintf(stringsBuf, "%X", (deltaDisp & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
			}

#ifdef MAME_DEBUG
			stringsBuf += sprintf(stringsBuf, " = %.2X (-%d)", search->old_options.delta, deltaDisp);
#else
			stringsBuf += sprintf(stringsBuf, " (-%d)", deltaDisp);
#endif
		}
		else

#ifdef MAME_DEBUG
		sprintf(timerBuf, "-%.2X = %.2X (-%d)", deltaDisp, search->old_options.delta, deltaDisp);
#else
		sprintf(timerBuf, "-%.2X (-%d)", deltaDisp, deltaDisp);
#endif
	}
	menuSubItem[total++] = timerBuf;

	/* ##### GREATER ##### */
	menuItem[total] = "Greater";
	menuSubItem[total++] = "A > B";

	/* ##### LESS ##### */
	menuItem[total] = "Less";
	menuSubItem[total++] = "A < B";

	/* ##### EQUAL ##### */
	menuItem[total] = "Equal";
	menuSubItem[total++] = "A = B";

	/* ##### NOT EQUAL ##### */
	menuItem[total] = "Not Equal";
	menuSubItem[total++] = "A != B";

	/* ##### DIVIDER ##### */
	menuItem[total] = MENU_SEPARATOR_ITEM;
	menuSubItem[total++] = NULL;

	/* ##### RESULT ##### */
	menuItem[total] = "Results";
	if(search->num_results)
	{
		sprintf(numResultsBuf, "%d", search->num_results);
	}
	else
	{
		if(!doneSaveMemory)
			sprintf(numResultsBuf, "No Memory Save");
		else
			sprintf(numResultsBuf, "No Result");
	}
	menuSubItem[total++] = numResultsBuf;

	/* ##### RESULT RESTORE ##### */
	menuItem[total] = "Restore Previous Results";
	menuSubItem[total++] = NULL;

	/* ##### RETURN ##### */
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* adjust current cursor position */
	if(menu->sel < 0 || menu->sel >= total)
		menu->sel = kMenu_Memory;
	if(!doneSaveMemory)
	{
		if(menu->sel >= kMenu_Timer && menu->sel <= kMenu_NotEqual)
			menu->sel = kMenu_Memory;
	}

	if(editActive)
		flagBuf[menu->sel] = 1;

	/* draw it */
	old_style_menu(menuItem, menuSubItem, flagBuf, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
					search->old_options.value = DoIncrementHexField(search->old_options.value, editCursor);
					break;

				case kMenu_Timer:
					switch(editCursor)
					{
						default:
							search->old_options.delta = DoIncrementHexFieldSigned(search->old_options.delta, editCursor, search->bytes);
							break;

						case 2:
							if(!search->old_options.delta || search->old_options.delta == 0x80)
								search->old_options.delta ^= 0x80;
							else
								search->old_options.delta = ~search->old_options.delta + 1;
							break;
					}
					break;
			}
		}
		else
		{
			CURSOR_TO_PREVIOUS(menu->sel, total);

			if(menu->sel == kMenu_Divider1 || menu->sel == kMenu_Divider2)
			{
				if(--menu->sel == kMenu_NotEqual && !doneSaveMemory)
					menu->sel = kMenu_Value;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
					search->old_options.value = DoDecrementHexField(search->old_options.value, editCursor);
					break;

				case kMenu_Timer:
					switch(editCursor)
					{
						default:
							search->old_options.delta = DoDecrementHexFieldSigned(search->old_options.delta, editCursor, search->bytes);
							break;

						case 2:
							if(!search->old_options.delta || search->old_options.delta == 0x80)
								search->old_options.delta ^= 0x80;
							else
								search->old_options.delta = ~search->old_options.delta + 1;
							break;
					}
					break;
			}
		}
		else
		{
			CURSOR_TO_NEXT(menu->sel, total);

			if(menu->sel == kMenu_Divider1 || menu->sel == kMenu_Divider2)
				menu->sel++;
			if(!doneSaveMemory && menu->sel == kMenu_Timer)
				menu->sel = kMenu_Result;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
					editCursor ^= 1;
					break;

				case kMenu_Timer:
					if(++editCursor > 2)
						editCursor = 0;
					break;
			}
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_CPU:
					if(search->target_idx > 0)
					{
						search->target_idx--;

						build_search_regions(machine, search);
						allocate_search_regions(search);

						doneSaveMemory = 0;
					}
					break;

				case kMenu_Value:
					search->old_options.value = (search->old_options.value - 1) & 0xFF;
					break;

				case kMenu_Timer:
					search->old_options.delta = (search->old_options.delta - 1) & 0xFF;
					break;

				default:
					menu->sel = kMenu_Memory;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
					editCursor ^= 1;
					break;

				case kMenu_Timer:
					if(--editCursor < 0)
						editCursor = 2;
					break;
			}
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_CPU:
					if(search->target_idx < cpu_gettotalcpu() - 1)
					{
						search->target_idx++;

						build_search_regions(machine, search);
						allocate_search_regions(search);

						doneSaveMemory = 0;
					}
					break;

				case kMenu_Value:
					search->old_options.value = (search->old_options.value + 1) & 0xFF;
					break;

				case kMenu_Timer:
					search->old_options.delta = (search->old_options.delta + 1) & 0xFF;
					break;

				default:
					menu->sel = kMenu_Result;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Value;
					search->comparison =	kSearchComparison_EqualTo;
					search->value =			search->old_options.value;
					doSearch = 1;
					break;

				case kMenu_Timer:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_IncreasedBy;
					search->value =			search->old_options.delta;
					doSearch = 1;
					break;
			}
			editActive = 0;
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_CPU:
					cheat_menu_stack_push(select_search_region_menu, menu->handler, menu->sel);
					break;

				case kMenu_Memory:
					doneSaveMemory = 0;
					doSearch = 1;
					break;

				case kMenu_Value:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Value;
					search->comparison =	kSearchComparison_EqualTo;
					search->value =			search->old_options.value;
					doSearch = 1;
					break;

				case kMenu_Timer:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_IncreasedBy;
					search->value =			search->old_options.delta;
					doSearch = 1;
					break;

				case kMenu_Equal:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_EqualTo;
					doSearch = 1;
					break;

				case kMenu_NotEqual:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_NotEqual;
					doSearch = 1;
					break;

				case kMenu_Less:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_LessThan;
					doSearch = 1;
					break;

				case kMenu_Greater:
					search->bytes =			kSearchSize_8Bit;
					search->lhs =			kSearchOperand_Current;
					search->rhs =			kSearchOperand_Previous;
					search->comparison =	kSearchComparison_GreaterThan;
					doSearch = 1;
					break;

				case kMenu_Result:
					if(search->region_list_length)
						cheat_menu_stack_push(view_search_result_menu, menu->handler, menu->sel);
					else
					{
						/* if no search region (eg, sms.c in HazeMD), don't open result viewer to avoid the crash */
						SET_MESSAGE(CHEAT_MESSAGE_NO_SEARCH_REGION);
					}
					break;

				case kMenu_RestoreSearch:
					restore_search_backup(search);
					break;

				case kMenu_Return:
					menu->sel = -1;
					break;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			if(menu->sel == kMenu_Value || menu->sel == kMenu_Timer)
			{
				editActive = 1;
				editCursor = 0;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		restore_search_backup(search);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)
			editActive = 0;
		else
			menu->sel = -1;
	}

	/********** SEARCH **********/
	if(doSearch)
	{
		if(!doneSaveMemory)
			init_new_search(search);

		if(doneSaveMemory || menu->sel == kMenu_Value)
		{
			backup_search(search);
			do_search(search);
		}

		update_search(search);

		if(doneSaveMemory || menu->sel == kMenu_Value)
			SET_MESSAGE(CHEAT_MESSAGE_CHEAT_FOUND);
		else
			SET_MESSAGE(CHEAT_MESSAGE_INITIALIZE_MEMORY);

		doneSaveMemory = 1;

		if(search->num_results == 1)
		{
			add_cheat_from_first_result(machine, search);

			SET_MESSAGE(CHEAT_MESSAGE_ONE_CHEAT_FOUND);
		}
	}

	/********** EDIT **********/
	if(editActive)
	{
		switch(menu->sel)
		{
			case kMenu_Value:
				search->old_options.value = do_edit_hex_field(machine, search->old_options.value);
				search->old_options.value &= 0xFF;
				break;

			case kMenu_Timer:
				search->old_options.delta = do_edit_hex_field(machine, search->old_options.delta);
				search->old_options.delta &= 0xFF;
				break;
		}
	}

	return menu->sel + 1;
}

/*-------------------------------------------------------------
  search_standard_menu - management for standard search menu
                         fixed bit and operand
-------------------------------------------------------------*/

static int search_standard_menu(running_machine *machine, cheat_menu_stack *menu)
{
	static const char *const kComparisonNameTable[] = {
		"Equal",
		"Not Equal",
		"Less",
		"Less or Equal",
		"Greater",
		"Greater or Equal" };

	static const char *const kIncrementNameTable[] = {
		"Increment",
		"Decrement" };

	static const int kComparisonConversionTable[] = {
		kSearchComparison_EqualTo,
		kSearchComparison_NotEqual,
		kSearchComparison_LessThan,
		kSearchComparison_LessThanOrEqualTo,
		kSearchComparison_GreaterThan,
		kSearchComparison_GreaterThanOrEqualTo };

	enum{
		kMenu_Comparison,
		kMenu_Value,
		kMenu_NearTo,
		kMenu_Increment,
		kMenu_Bit,
		kMenu_Separator,
		kMenu_CPU,
		kMenu_Memory,
		kMenu_ViewResult,
		kMenu_RestoreResult,
		kMenu_Return,

		kMenu_Max };

	enum{
		kComparison_Equal = 0,
		kComparison_NotEqual,
		kComparison_Less,
		kComparison_LessOrEqual,
		kComparison_Greater,
		kComparison_GreaterOrEqual,
		kComparison_Max = kComparison_GreaterOrEqual };

	INT8			total			= 0;
	INT8			doSearch		= 0;
	static INT8		doneSaveMemory	= 0;
	const char		* menuItem[kMenu_Max + 1]		= { 0 };
	const char		* menuSubItem[kMenu_Max + 1]	= { 0 };
	char			valueBuf[32];
	char			incrementBuf[16];
	search_info		*search = get_current_search();

	/* first setting : adjust search items */
	if(menu->first_time)
	{
		search->lhs = kSearchOperand_Current;

		if(!search->old_options.delta)
			search->old_options.delta = 1;

		menu->first_time = 0;
	}

	/********** MENU CONSTRUCTION **********/
	/* ##### COMPARISON ##### */
	menuItem[total] = "Comparison";
	if(menu->sel == kMenu_Comparison || menu->sel == kMenu_Value)
		menuSubItem[total++] = kComparisonNameTable[search->old_options.comparison];
	else
		menuSubItem[total++] = " ";

	/* ##### VALUE ##### */
	menuItem[total] = "Value";
	if(menu->sel == kMenu_Value)
	{
		if(editActive)
		{
			INT8	i;
			char	* stringsBuf = valueBuf;

			for(i = 1; i >= 0; i--)
			{
				if(i == editCursor)
					stringsBuf += sprintf(stringsBuf, "[%X]", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				else
					stringsBuf += sprintf(stringsBuf, "%X", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
			}
			stringsBuf += sprintf(stringsBuf, " (%d)", search->old_options.value);
		}
		else
			sprintf(valueBuf,	"%2.2X (%2.2d)", search->old_options.value, search->old_options.value);

		menuSubItem[total++] = valueBuf;
	}
	else
		menuSubItem[total++] = " ";

	/* ##### VALUE - NEAR TO ##### */
	menuItem[total] = "Near To";
	if(menu->sel == kMenu_NearTo)
	{
		if(editActive)
		{
			INT8	i;
			char	* stringsBuf = valueBuf;

			for(i = 1; i >=0; i--)
			{
				if(i == editCursor)
					stringsBuf += sprintf(stringsBuf, "[%X]", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				else
					stringsBuf += sprintf(stringsBuf, "%X", (search->old_options.value & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
			}
			stringsBuf += sprintf(stringsBuf,	" or %2.2X (%2.2d or %2.2d)",
												(search->old_options.value - 1) & kSearchByteMaskTable[search->bytes],
												search->old_options.value,
												search->old_options.value - 1);
		}
		else
			sprintf(valueBuf,	"%2.2X or %2.2X (%2.2d or %2.2d)",
								search->old_options.value, (search->old_options.value - 1) & kSearchByteMaskTable[search->bytes],
								search->old_options.value, search->old_options.value - 1);
		menuSubItem[total++] = valueBuf;
	}
	else
		menuSubItem[total++] = " ";

	/* ##### INCREMENT/DECREMENT ##### */
	menuItem[total] = kIncrementNameTable[search->old_options.sign];
	if(menu->sel == kMenu_Increment)
	{
		if(editActive)
		{
			INT8	i;
			char	* stringsBuf = incrementBuf;

			for(i = 2; i >=0; i--)
			{
				if(i == editCursor)
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "[%s]", search->old_options.sign ? "-" : "+");
					else
						stringsBuf += sprintf(stringsBuf, "[%X]", (search->old_options.delta & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
				else
				{
					if(i == 2)
						stringsBuf += sprintf(stringsBuf, "%s", search->old_options.sign ? "-" : "+");
					else
						stringsBuf += sprintf(stringsBuf, "%X", (search->old_options.delta & kIncrementMaskTable[i]) / kIncrementValueTable[i]);
				}
			}
			stringsBuf += sprintf(stringsBuf, " (%s%2.2d)", search->old_options.sign ? "-" : "+", search->old_options.delta);
		}
		else
		{
			sprintf(incrementBuf,	"%s%2.2X (%s%2.2d)",
									search->old_options.sign ? "-" : "+",
									search->old_options.delta,
									search->old_options.sign ? "-" : "+",
									search->old_options.delta);
		}
		menuSubItem[total++] = incrementBuf;
	}
	else
		menuSubItem[total++] = " ";

	/* ##### BIT ##### */
	menuItem[total] = "Bit";
	if(menu->sel == kMenu_Bit)
		menuSubItem[total++] = kComparisonNameTable[search->old_options.comparison];
	else
		menuSubItem[total++] = " ";

	/* ##### SEPARATOR ##### */
	menuItem[total] = MENU_SEPARATOR_ITEM;
	menuSubItem[total++] = NULL;

	/* ##### CPU ##### */
	menuItem[total] = "CPU";
	menuSubItem[total++] = kNumbersTable[search->target_idx];

	/* ##### MEMORY ##### */
	if(doneSaveMemory)
		menuItem[total] = "Initialize Memory";
	else
		menuItem[total] = "Save Memory";
	menuSubItem[total++] = NULL;

	/* ##### RESULT VIEWER ##### */
	menuItem[total] = "View Last Results";
	menuSubItem[total++] = NULL;

	/* ##### RESULT RESTORE ##### */
	menuItem[total] = "Restore Previous Results";
	menuSubItem[total++] = NULL;

	/* ##### RETURN ##### */
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menuItem, menuSubItem, 0, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
				case kMenu_NearTo:
					search->old_options.value = DoIncrementHexField(search->old_options.value, editCursor);
					break;

				case kMenu_Increment:
					if(editCursor == 2)
						search->old_options.sign ^= 1;
					else
						search->old_options.delta = DoIncrementHexField(search->old_options.delta, editCursor);
					break;
			}
		}
		else
		{
			CURSOR_TO_PREVIOUS(menu->sel, total);

			if(menu->sel == kMenu_Separator) menu->sel--;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
				case kMenu_NearTo:
					search->old_options.value = DoDecrementHexField(search->old_options.value, editCursor);
					break;

				case kMenu_Increment:
					if(editCursor == 2)
						search->old_options.sign ^= 1;
					else
						search->old_options.delta = DoDecrementHexField(search->old_options.delta, editCursor);
					break;
			}
		}
		else
		{
			CURSOR_TO_NEXT(menu->sel, total);

			if(menu->sel == kMenu_Separator) menu->sel++;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		if(!editActive)
			CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		if(!editActive)
			CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
				case kMenu_NearTo:
					if(++editCursor > 1) editCursor = 0;
					break;

				case kMenu_Increment:
					if(++editCursor > 2) editCursor = 0;
					break;
			}
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_Comparison:
				case kMenu_Bit:
					if(--search->old_options.comparison < kComparison_Equal)
						search->old_options.comparison = kComparison_Max;
					break;

				case kMenu_Value:
				case kMenu_NearTo:
					search->old_options.value = (search->old_options.value - 1) & kCheatSizeMaskTable[search->bytes];
					break;

				case kMenu_Increment:
					if(search->old_options.sign)
					{
						if(++search->old_options.delta > kSearchByteUnsignedMaskTable[search->bytes])
							search->old_options.delta = kSearchByteUnsignedMaskTable[search->bytes];
					}
					else if(--search->old_options.delta <= 0)
					{
						search->old_options.delta = 1;
						search->old_options.sign ^= 0x01;
					}
					break;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(editActive)
		{
			switch(menu->sel)
			{
				case kMenu_Value:
				case kMenu_NearTo:
					if(--editCursor < 0) editCursor = 1;
					break;

				case kMenu_Increment:
					if(--editCursor < 0) editCursor = 2;
					break;
			}
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_Comparison:
				case kMenu_Bit:
					if(++search->old_options.comparison > kComparison_GreaterOrEqual)
						search->old_options.comparison = 0;
					break;

				case kMenu_Value:
				case kMenu_NearTo:
					search->old_options.value = (search->old_options.value + 1) & kCheatSizeMaskTable[search->bytes];
					break;

				case kMenu_Increment:
					if(search->old_options.sign)
					{
						if(--search->old_options.delta <= 0)
						{
							search->old_options.delta = 1;
							search->old_options.sign ^= 0x01;
						}
					}
					else if(++search->old_options.delta > kSearchByteUnsignedMaskTable[search->bytes])
					{
						search->old_options.delta = kSearchByteUnsignedMaskTable[search->bytes];
					}
					break;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			switch(menu->sel)
			{
				case kMenu_Comparison:
					search->bytes		= kSearchSize_8Bit;
					search->rhs			= kSearchOperand_First;
					search->comparison	= kComparisonConversionTable[search->old_options.comparison];
					doSearch			= 1;
					break;

				case kMenu_Value:
					search->bytes		= kSearchSize_8Bit;
					search->rhs			= kSearchOperand_Value;
					search->comparison	= kComparisonConversionTable[search->old_options.comparison];
					search->value		= search->old_options.value;
					doSearch			= 1;
					break;

				case kMenu_NearTo:
					search->bytes		= kSearchSize_8Bit;
					search->rhs			= kSearchOperand_Value;
					search->comparison	= kSearchComparison_NearTo;
					search->value		= search->old_options.value;
					doSearch			= 1;
					break;

				case kMenu_Increment:
					if(search->old_options.delta)
					{
						search->bytes		= kSearchSize_8Bit;
						search->rhs			= kSearchOperand_First;
						search->comparison	= kSearchComparison_IncreasedBy;

						if(search->old_options.sign)
							search->value	= ~search->old_options.delta + 1;
						else
							search->value	= search->old_options.delta;

						doSearch			= 1;
					}
					break;

				case kMenu_Bit:
					search->bytes		= kSearchSize_1Bit;
					search->rhs			= kSearchOperand_First;
					search->comparison	= kComparisonConversionTable[search->old_options.comparison];
					doSearch			= 1;
					break;

				case kMenu_Memory:
					doneSaveMemory	= 0;
					doSearch		= 1;
					break;

				case kMenu_CPU:
					cheat_menu_stack_pop();
					break;

				case kMenu_ViewResult:
					if(search->region_list_length)
						cheat_menu_stack_push(view_search_result_menu, menu->handler, menu->sel);
					break;

				case kMenu_RestoreResult:
					restore_search_backup(search);
					break;

				case kMenu_Return:
					menu->sel = -1;
					break;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(menu->sel == kMenu_Value || menu->sel == kMenu_NearTo || menu->sel == kMenu_Increment)
		{
			editActive ^= 1;
			editCursor = 0;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)	editActive = 0;
		else			menu->sel = -1;
	}

	/***** SEARCH *****/
	if(doSearch)
	{
		if(!doneSaveMemory)
			init_new_search(search);

		if(search->rhs == kSearchOperand_Value || doneSaveMemory)
		{
			backup_search(search);
			do_search(search);
		}

		update_search(search);
		doneSaveMemory = 1;

		if(search->num_results == 1)
			add_cheat_from_first_result(machine, search);
	}

	/********** EDIT **********/
	switch(menu->sel)
	{
		case kMenu_Value:
			search->old_options.value = do_edit_hex_field(machine, search->old_options.value);
			search->old_options.value &= 0xFF;
			break;

		case kMenu_Increment:
			search->old_options.delta = do_edit_hex_field(machine, search->old_options.delta);
			search->old_options.delta &= 0x7F;
			break;
	}

	return menu->sel + 1;
}

/*-------------------------------------------------------------
  search_advanced_menu - management for advanced search menu
-------------------------------------------------------------*/

static int search_advanced_menu(running_machine *machine, cheat_menu_stack *menu)
{
#if 0
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

	search_info		*search = get_current_search();

	INT32			sel = selection - 1;
	INT32			total = 0;
	UINT32			increment = 1;

	static int		submenuChoice = 0;
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
				submenuChoice = SelectSearchRegions(machine, submenuChoice, get_current_search());
				break;

			case kMenu_ViewResult:
				submenuChoice = ViewSearchResults(submenuChoice, firstEntry);
				break;
		}

		firstEntry = 0;

		/* meaningless ? because no longer return with sel = -1 (pressed UI_CONFIG in submenu) */
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

	if(TEST_FIELD(cheat_options, DontPrintNewLabels))
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

	/* adjust current cursor position */
	if(sel < 0)
		sel = 0;
	if(sel >= total)
		sel = total - 1;

	/* higlighted sub-item if edit mode */
	if(editActive)
		flagBuf[sel] = 1;

	/* draw it */
	old_style_menu(menu_item, menu_subitem, flagBuf, sel, 0);

	/********** KEY HANDLING **********/
	if(AltKeyPressed())
		increment <<= 4;
	if(ControlKeyPressed())
		increment <<= 8;
	if(ShiftKeyPressed())
		increment <<= 16;

	if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		/* if divider, skip it */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2) || (sel == kMenu_Divider3))
			sel++;

		if(sel >= total)
			sel = 0;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		/* if divider, skip it */
		if((sel == kMenu_Divider) || (sel == kMenu_Divider2) || (sel == kMenu_Divider3))
			sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		sel += fullMenuPageHeight;

		if(sel >= total)
			sel = total - 1;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
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

	if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
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
						/* initialize search */
						init_new_search(search);

					if((!kSearchOperandNeedsInit[search->lhs] && !kSearchOperandNeedsInit[search->rhs]) || doneMemorySave)
					{
						backup_search(search);

						DoSearch(search);
					}

					update_search(search);

					if((!kSearchOperandNeedsInit[search->lhs] && !kSearchOperandNeedsInit[search->rhs]) || doneMemorySave)
						popmessage("%d results found", search->num_results);
					else
						popmessage("saved all memory regions");

					doneMemorySave = 1;

					if(search->num_results == 1)
					{
						add_cheat_from_first_result(machine, search);

						popmessage("1 result found, added to list");
					}
					break;

				case kMenu_SaveMemory:
					init_new_search(search);

					update_search(search);

					popmessage("saved all memory regions");

					doneMemorySave = 1;
					break;

				case kMenu_ViewResult:
					if(search->regionListLength)
					{
						/* go to result viewer */
						firstEntry = 1;
						submenuChoice = 1;
					}
					else
						/* if no search region (eg, sms.c in HazeMD), don't open result viewer to avoid the crash */
						ui_popup_time(1, "no search regions");
					break;

				case kMenu_RestoreSearch:
					/* restore previous results */
					restore_search_backup(search);
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

	/* edit selected field  */
	if(editActive)
	{
		switch(sel)
		{
			case kMenu_Value:
				search->value = do_edit_hex_field(machine, search->value);

				search->value &= kSearchByteMaskTable[search->bytes];
				break;

			case kMenu_Name:
				search->name = DoDynamicEditTextField(search->name);
				break;
		}
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;

	/* keep current cursor position */
	if(!(sel == -1))
		lastSel = sel;
#endif
	return menu->sel;
}

/*---------------------------------------------------------------------------
  select_search_region_menu - management for search regions selection menu
---------------------------------------------------------------------------*/

static int select_search_region_menu(running_machine *machine, cheat_menu_stack *menu)
{
	static const char *const ksearch_speedList[] =
	{
		"Fast",
		"Medium",
		"Slow",
		"Very Slow",
		"All Memory",
		"User Defined"
	};

	int				total			= 0;
	UINT8			do_rebuild		= 0;
	UINT8			do_reallocate	= 0;
	const char		**menu_item;
	const char		**menu_sub_item;
	search_info		*search = get_current_search();
	search_region	*region;

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = speed + total regions + return + terminator */
	request_strings(search->region_list_length + 4, 0, 0, 0);
	menu_item		= menu_strings.main_list;
	menu_sub_item	= menu_strings.sub_list;

	/********** MENU CONSTRUCTION **********/
	/* ##### SEARCH SPEED ##### */
	menu_item[total] = "Search Speed";
	menu_sub_item[total++] = ksearch_speedList[search->search_speed];

	/* ##### REGIONS ##### */
	if(search->region_list_length)
	{
		int i;

		for(i = 0; i < search->region_list_length; i++)
		{
			search_region *region = &search->region_list[i];

			menu_item[total] = region->name;

			if(region->flags & kRegionFlag_HasError)
				menu_sub_item[total++] = "Locked";
			else
				menu_sub_item[total++] = region->flags & kRegionFlag_Enabled ? "On" : "Off";
		}
	}
	else
	{

		/* in case of no search region */
		menu_item[total] = "No Search Region";
		menu_sub_item[total++] = NULL;
	}

	/* ##### RETURN ##### */
	menu_item[total] = "Return to Prior Menu";
	menu_sub_item[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total] = NULL;
	menu_sub_item[total] = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menu_item, menu_sub_item, NULL, menu->sel, 0);

	if(search->region_list_length && menu->sel && menu->sel < total - 1)
	{
		region = &search->region_list[menu->sel - 1];

		if(region->flags & kRegionFlag_HasError)
			region = NULL;
	}
	else
		region = NULL;

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		if(region)
		{
			/* toggle ON/OFF */
			region->flags ^= kRegionFlag_Enabled;

			do_reallocate = 1;
		}
		else if(menu->sel == 0)
		{
			if(--search->search_speed < 0)
				/* "Fast" -> "User Defined" */
				search->search_speed = SEARCH_SPEED_MAX - 1;

			do_rebuild = 1;
			do_reallocate = 1;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		if(region)
		{
			/* toggle ON/OFF */
			region->flags ^= kRegionFlag_Enabled;

			do_reallocate = 1;
		}
		else if(menu->sel == 0)
		{
			if(++search->search_speed >= SEARCH_SPEED_MAX)
				/* "User Defined" -> "Fast" */
				search->search_speed = SEARCH_SPEED_FAST;

			do_rebuild = 1;
			do_reallocate = 1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(ShiftKeyPressed() && region && search)
		{
			/* SHIFT + CHEAT DELETE = invalidate selected region */
			invalidate_entire_region(search, region);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		if(search->search_speed == SEARCH_SPEED_USER_DEFINED)
		/* reload user region */
			do_rebuild = 1;
			do_reallocate = 1;
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(region)
		{
			region->flags ^= kRegionFlag_Enabled;
			do_reallocate = 1;
		}
		else if(menu->sel == 0)
		{
			do_rebuild = 1;
			do_reallocate = 1;
		}
		else if(menu->sel == total - 1)
			menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	/********** REBUILD SEARCH REGION **********/
	if(do_rebuild)
	{
		if(search->search_speed != SEARCH_SPEED_USER_DEFINED)
			/* rebuild search region from memory map */
			build_search_regions(machine, search);
		else
			/* rebuild search region from user-defined map */
			load_cheat_database(machine, LOAD_USER_REGION);
	}

	if(do_reallocate)
		/* reallocate search region */
		allocate_search_regions(search);

	return menu->sel + 1;
}

/*--------------------------------------------------------------
  view_search_result_menu - management for search result menu
--------------------------------------------------------------*/

static int view_search_result_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_Header = 0,
		kMenu_FirstResult,

		kMaxResultsPerPage = 100 };

	int				i;
	int				total				= 0;
	int				numPages;
	int				resultsPerPage;
	int				numToSkip;
	UINT8			hadResults			= 0;
	UINT8			pageSwitch			= 0;
	UINT8			resultsFound		= 0;
	UINT8			selectedAddressGood	= 0;
	UINT32			selectedAddress		= 0;
	UINT32			selectedOffset		= 0;
	UINT32			traverse;
	const char		** menu_item;
	char			** buf;
	char			* header;
	search_info		*search = get_current_search();
	search_region	*region;

	/* first setting : initialize region index and page number */
	if(menu->first_time)
	{
		search->current_region_idx = 0;
		search->current_results_page = 0;

		/* set current REGION for first display */
		for(traverse = 0; traverse < search->region_list_length; traverse++)
		{
			region = &search->region_list[traverse];

			if(region->num_results)
			{
				search->current_region_idx = traverse;
				break;
			}
		}

		menu->first_time = 0;
	}

	/* required items = (header + defined max items + return + terminator) & (strings buf * (header + defined max items) [max chars = 32]) */
	request_strings(kMaxResultsPerPage + 3, kMaxResultsPerPage, 32, 0);
	menu_item	= menu_strings.main_list;
	header		= menu_strings.main_strings[0];
	buf			= &menu_strings.main_strings[1];

	/* adjust current REGION */
	if(search->current_region_idx >= search->region_list_length)
		search->current_region_idx = search->region_list_length - 1;
	if(search->current_region_idx < 0)
		search->current_region_idx = 0;

	region = &search->region_list[search->current_region_idx];

	/* set the number of items per PAGE */
	resultsPerPage = fullMenuPageHeight - 3;

	/* adjust total items per PAGE */
	if(resultsPerPage <= 0)
		resultsPerPage = 1;
	else if(resultsPerPage > kMaxResultsPerPage)
		resultsPerPage = kMaxResultsPerPage;

	/* get the number of total PAGEs */
	if(region->flags & kRegionFlag_Enabled)
		numPages = (region->num_results / kSearchByteStep[search->bytes] + resultsPerPage - 1) / resultsPerPage;
	else
		numPages = 0;

	if(numPages > fullMenuPageHeight)
		numPages = fullMenuPageHeight;

	/* adjust current PAGE */
	if(search->current_results_page >= numPages)
		search->current_results_page = numPages - 1;
	if(search->current_results_page < 0)
		search->current_results_page = 0;

	/* set the number of skipping results to undisplay */
	numToSkip = resultsPerPage * search->current_results_page;

	/********** MENU CONSTRUCTION **********/
	/* ##### HEADER ##### */
	sprintf(header, "%s %d/%d", region->name, search->current_results_page + 1, numPages);

	menu_item[total++] = header;

	traverse = 0;

	if((region->length < kSearchByteIncrementTable[search->bytes]) || !(region->flags & kRegionFlag_Enabled))
	{
		; // no results...
	}
	else
	{
		/* ##### RESULT ##### */
		for(i = 0; i < resultsPerPage && traverse < region->length && resultsFound < region->num_results;)
		{
			while(is_region_offset_valid(search, region, traverse) == 0 && traverse < region->length)
				traverse += kSearchByteStep[search->bytes];

			if(traverse < region->length)
			{
				if(numToSkip > 0)
					numToSkip--;
				else
				{
					if(total == menu->sel)
					{
						selectedAddress		= region->address + traverse;
						selectedOffset		= traverse;
						selectedAddressGood	= 1;
					}

					sprintf(	buf[total],
								"%.8X (%.*X %.*X %.*X)",
								region->address + traverse,
								kSearchByteDigitsTable[search->bytes],
								read_search_operand(kSearchOperand_First, search, region, region->address + traverse),
								kSearchByteDigitsTable[search->bytes],
								read_search_operand(kSearchOperand_Previous, search, region, region->address + traverse),
								kSearchByteDigitsTable[search->bytes],
								read_search_operand(kSearchOperand_Current, search, region, region->address + traverse));

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

	/* set special message if empty REGION */
	if(!hadResults)
	{
		if(search->num_results)
			menu_item[total++] = "no results for this region";
		else
			menu_item[total++] = "no results found";
	}

	/* ##### RETURN ##### */
	menu_item[total++] = "Return to Prior Menu";

	/* ##### TERMINATE ARRAY ##### */
	menu_item[total] = NULL;

	/* adjust current cursor position */
	if(menu->sel <= kMenu_Header)
		menu->sel = kMenu_FirstResult;
	if(menu->sel > total - 1 || !hadResults)
		menu->sel = total - 1;

	/* draw it */
	old_style_menu(menu_item, NULL, NULL, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(--menu->sel < 1)
			menu->sel = total - 1;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(++menu->sel >= total)
			menu->sel = kMenu_FirstResult;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalFastKeyRepeatRate))
	{
		if(ShiftKeyPressed())
		{
			/* shift + right = go to last PAGE */
			search->current_results_page = numPages - 1;
		}
		else if(ControlKeyPressed())
		{
			/* ctrl + right = go to next REGION */
			search->current_region_idx++;

			if(search->current_region_idx >= search->region_list_length)
				search->current_region_idx = 0;
		}
		else
		{
			/* otherwise, go to next PAGE */
			pageSwitch = 1;
		}

		menu->sel = kMenu_FirstResult;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalFastKeyRepeatRate))
	{
		if(ShiftKeyPressed())
		{
			/* shift + left = go to first PAGE */
			search->current_results_page = 0;
		}
		else if(ControlKeyPressed())
		{
			/* ctrl + left = go to previous REGION */
			search->current_region_idx--;

			if(search->current_region_idx < 0)
				search->current_region_idx = search->region_list_length - 1;
		}
		else
		{
			/* otherwise, go to previous PAGE */
			pageSwitch = 2;
		}

		menu->sel = kMenu_FirstResult;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kHorizontalFastKeyRepeatRate))
	{
		menu->sel -=fullMenuPageHeight;

		if(menu->sel <= kMenu_Header)
			menu->sel = kMenu_FirstResult;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kHorizontalFastKeyRepeatRate))
	{
		menu->sel +=fullMenuPageHeight;

		if(menu->sel >= total)
			menu->sel = total - 1;
	}
	else if(input_code_pressed_once(KEYCODE_HOME))
	{
		/* go to first PAGE */
		search->current_results_page = 0;
	}
	else if(input_code_pressed_once(KEYCODE_END))
	{
		/* go to last PAGE */
		search->current_results_page = numPages - 1;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PREV_GROUP, kVerticalKeyRepeatRate))
	{
		/* go to previous REGION */
		search->current_region_idx--;

		if(search->current_region_idx < 0)
			search->current_region_idx = search->region_list_length - 1;

		menu->sel = kMenu_FirstResult;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_NEXT_GROUP, kVerticalKeyRepeatRate))
	{
		/* go to next REGION */
		search->current_region_idx++;

		if(search->current_region_idx >= search->region_list_length)
			search->current_region_idx = 0;

		menu->sel = kMenu_FirstResult;
	}
	else if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
	{
		if(selectedAddressGood)
			add_cheat_from_result(machine, search, region, selectedAddress);
	}
	else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			if(region && search)
				/* shift + delete = invalidate all results in current REGION */
				invalidate_entire_region(search, region);
		}
		else if(selectedAddressGood)
		{
			/* otherwise, delete selected result */
			invalidate_region_offset(search, region, selectedOffset);
			search->num_results--;
			region->num_results--;

			SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_DELETE);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_WATCH_VALUE))
	{
		if(selectedAddressGood)
			add_watch_from_result(search, region, selectedAddress);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT) || input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	switch(pageSwitch)
	{
		case 1:		// go to next PAGE
			search->current_results_page++;

			/* if current PAGE is the last, go to first PAGE in next REGION */
			if(search->current_results_page >= numPages)
			{
				search->current_results_page = 0;
				search->current_region_idx++;

				/* if current REGION is the last, go to first REGION */
				if(search->current_region_idx >= search->region_list_length)
					search->current_region_idx = 0;

				/* if next REGION is empty, search next "non-empty" REGION.
                     but incomplete because last REGION is displayed even if empty */
				for(traverse = search->current_region_idx; traverse < search->region_list_length; traverse++)
				{
					search->current_region_idx = traverse;

					if(search->region_list[traverse].num_results)
						break;
				}
			}

			menu->sel = kMenu_FirstResult;
			break;

		case 2:		// go to previous PAGE
			search->current_results_page--;

			/* if current PAGE is first, go to previous REGION */
			if(search->current_results_page < 0)
			{
				search->current_results_page = 0;
				search->current_region_idx--;

				/* if current REGION is first, go to last REGION */
				if(search->current_region_idx < 0)
					search->current_region_idx = search->region_list_length - 1;

				/* if previous REGION is empty, search previous "non-empty" REGION.
                     but incomplete because first REGION is displayed even if empty */
				for(i = search->current_region_idx; i >= 0; i--)
				{
					search->current_region_idx = i;

					if(search->region_list[i].num_results)
						break;
				}

				/* go to last PAGE in previous REGION */
				{
					/* get the number of total PAGEs for previous REGION */
					search_region	*new_region		= &search->region_list[search->current_region_idx];
					UINT32			nextNumPages	= (new_region->num_results / kSearchByteStep[search->bytes] + resultsPerPage - 1) / resultsPerPage;

					if(nextNumPages <= 0)
						nextNumPages = 1;

					search->current_results_page = nextNumPages - 1;
				}
			}

			menu->sel = kMenu_FirstResult;
			break;
	}

	return menu->sel + 1;
}

/*----------------------------------------------------------
  choose_watch_menu - management for watchpoint list menu
----------------------------------------------------------*/

static int choose_watch_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				i;
	UINT8			total			= 0;
	const char		** menuItem;
	char			** buf;
	char			*stringsBuf;	/* "USER20 FFFFFFFF (99:32 Bit)" 27 chars */
	watch_info		*watch;

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = (total watchpoints + return + terminator) & (strings buf * total watchpoints [max chars = 32]) */
	request_strings(watch_list_length + 2, watch_list_length, 32, 0);
	menuItem	= menu_strings.main_list;
	buf			= menu_strings.main_strings;

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < watch_list_length; i++)
	{
		watch_info		*traverse	= &watch_list[i];
		cpu_region_info	*info		= get_cpu_or_region_info(traverse->cpu);

		/* FORMAT : "CPU/Region : Address (Length : Bytes)" */
		if(!editActive || menu->sel != i)
		{
			sprintf(buf[i], "%s:%.*X (%d:%s)",
				get_region_name(traverse->cpu),
				info->address_chars_needed,
				traverse->address,
				traverse->num_elements,
				kByteSizeStringList[traverse->element_bytes]);
		}
		else
		{
			/* insert edit cursor in quick edit mode */
			switch(editCursor)
			{
				default:
				{
					INT8 j;

					stringsBuf = buf[i];

					stringsBuf += sprintf(stringsBuf, "%s ", get_region_name(traverse->cpu));

					for(j = info->address_chars_needed + 1; j > 1; j--)
					{
						INT8 digits = j - 2;

						if(j == editCursor)
							stringsBuf += sprintf(stringsBuf, "[%X]", (traverse->address & kIncrementMaskTable[digits]) / kIncrementValueTable[digits]);
						else
							stringsBuf += sprintf(stringsBuf, "%X", (traverse->address & kIncrementMaskTable[digits]) / kIncrementValueTable[digits]);
					}

					stringsBuf += sprintf(stringsBuf, " (%d:%s)", traverse->num_elements, kByteSizeStringList[traverse->element_bytes]);
				}
				break;

				case 1:
					sprintf(buf[i], "%s:%.*X ([%d]:%s)",
						get_region_name(traverse->cpu),
						info->address_chars_needed,
						traverse->address,
						traverse->num_elements,
						kByteSizeStringList[traverse->element_bytes]);
					break;

				case 0:
					sprintf(buf[i], "%s:%.*X (%d:[%s])",
						get_region_name(traverse->cpu),
						info->address_chars_needed,
						traverse->address,
						traverse->num_elements,
						kByteSizeStringList[traverse->element_bytes]);
					break;
			}
		}

		menuItem[total] = buf[i];
		total++;
	}

	/* ##### RETURN ##### */
	menuItem[total++] = "Return to Prior Menu";

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menuItem, NULL, NULL, menu->sel, 0);

	if(menu->sel < watch_list_length)
		watch = &watch_list[menu->sel];
	else
		watch = NULL;

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		if(!editActive)
		{
			CURSOR_TO_PREVIOUS(menu->sel, total);
		}
		else
		{
			switch(editCursor)
			{
				default:
					watch->address = DoIncrementHexField(watch->address, editCursor - 2);
					break;

				case 1:
					if(watch->num_elements < 99)
						watch->num_elements++;
					break;

				case 0:
					watch->element_bytes = (watch->element_bytes + 1) & 0x03;
					break;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		if(!editActive)
		{
			CURSOR_TO_NEXT(menu->sel, total);
		}
		else
		{
			switch(editCursor)
			{
				default:
					watch->address = DoDecrementHexField(watch->address, editCursor - 2);
					break;

				case 1:
					if(watch->num_elements > 0)
						watch->num_elements--;
					break;

				case 0:
					watch->element_bytes = (watch->element_bytes - 1) & 0x03;
					break;
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		if(!editActive)
			CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		if(!editActive)
			CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			cpu_region_info *info = get_cpu_or_region_info(watch->cpu);

			if(++editCursor > info->address_chars_needed + 1)
				editCursor = 0;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kVerticalKeyRepeatRate))
	{
		if(editActive)
		{
			cpu_region_info *info = get_cpu_or_region_info(watch->cpu);

			if(--editCursor < 0)
				editCursor = info->address_chars_needed + 1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			if(watch)
				cheat_menu_stack_push(command_watch_menu, menu->handler, menu->sel);
			else
				menu->sel = -1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
	{
		if(!editActive && watch)
		{
			if(ShiftKeyPressed())
				add_cheat_from_watch(machine, watch);
			else if(ControlKeyPressed())
			{
				cheat_entry *entry = get_new_cheat();

				add_cheat_from_watch_as_watch(machine, entry, watch);

				/* when fails to add, delete this entry because it causes the crash */
				if(message_type == CHEAT_MESSAGE_FAILED_TO_ADD)
				{
					delete_cheat_at(cheat_list_length - 1);
					SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
				}
			}
			else
				add_watch_before(menu->sel);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
	{
		if(!editActive && watch)
		{
			if(ShiftKeyPressed())
			{
				delete_watch_at(menu->sel);
			}
			else if(ControlKeyPressed())
			{
				for(i = 0; i < watch_list_length; i++)
					watch_list[i].num_elements = 0;
			}
			else
			{
				if(watch)
					watch->num_elements = 0;
			}
		}
		else if(editActive)
		{
			switch(editCursor)
			{
				default:
					watch->address = 0;
					break;

				case 1:
					watch->num_elements = 0;
					break;

				case 0:
					watch->element_bytes = 0;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		if(!editActive && watch)
		{
			cheat_entry entry;

			memset(&entry, 0, sizeof(cheat_entry));

			add_cheat_from_watch_as_watch(machine, &entry, watch);
			save_cheat_code(machine, &entry);
			dispose_cheat(&entry);
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(watch)
		{
			editCursor = 2;
			editActive ^= 1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CLEAR))
	{
		if(!editActive)
		{
			if(ShiftKeyPressed())
			{
				reset_watch(watch);
			}
			else
			{
				for(i = 0; i < watch_list_length; i++)
					reset_watch(&watch_list[i]);
			}
		}
		else
		{
			watch->address = 0;
			watch->num_elements = 0;
			watch->element_bytes = 0;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)
			editActive = 0;
		else
			menu->sel = -1;
	}

	return menu->sel + 1;
}

/*--------------------------------------------------------------
  command_watch_menu - management for wathcpoint command menu
--------------------------------------------------------------*/

static int command_watch_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_EditWatch = 0,
		kMenu_DisableWatch,
		kMenu_DisableAllWatch,
		kMenu_ResetWatch,
		kMenu_ResetAllWatch,
		kMenu_AddAsCheatCode,
		kMenu_AddAsWatchCode,
		kMenu_SaveAsCheatCode,
		kMenu_SaveAsWatchCode,
		kMenu_AddWatch,
		kMenu_DeleteWatch,
		kMenu_Return,
		kMenu_Max };

	int				i;
	UINT8			total	= 0;
	ui_menu_item	menuItem[kMenu_Max + 1];
	watch_info		*entry	= &watch_list[menu->pre_sel];

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	memset(menuItem, 0, sizeof(menuItem));

	/********** MENU CONSTRUCION **********/
	menuItem[total++].text = "Edit Watchpoint";
	menuItem[total++].text = "Disable Watchpoint";
	menuItem[total++].text = "Disable All Watchpoints";
	menuItem[total++].text = "Reset Watchpoint";
	menuItem[total++].text = "Reset All Watchpoints";
	menuItem[total++].text = "Add as Cheat Code";
	menuItem[total++].text = "Add as Watch Code";
	menuItem[total++].text = "Save as Cheat Code";
	menuItem[total++].text = "Save as Watch Code";
	menuItem[total++].text = "Add New Watchpoint";
	menuItem[total++].text = "Delete Watchpoint";
	menuItem[total++].text = "Return to Prior Menu";
	menuItem[total].text = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* print it */
	ui_menu_draw(menuItem, total, menu->sel, NULL);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case kMenu_EditWatch:
				cheat_menu_stack_push(edit_watch_menu, menu->return_handler, menu->pre_sel);
				break;

			case kMenu_DisableWatch:
				entry->num_elements = 0;
				break;

			case kMenu_DisableAllWatch:
				for(i = 0; i < watch_list_length; i++)
					watch_list[i].num_elements = 0;
				break;

			case kMenu_ResetWatch:
				reset_watch(entry);
				break;

			case kMenu_ResetAllWatch:
				for(i = 0; i < watch_list_length; i++)
					reset_watch(&watch_list[i]);
				break;

			case kMenu_AddAsCheatCode:
				add_cheat_from_watch(machine, entry);
				break;

			case kMenu_AddAsWatchCode:
			{
				cheat_entry *new_entry = get_new_cheat();

				add_cheat_from_watch_as_watch(machine, new_entry, entry);

				/* when fails to add, delete this entry because it causes the crash */
				if(message_type == CHEAT_MESSAGE_FAILED_TO_ADD)
				{
					delete_cheat_at(cheat_list_length - 1);
					SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
				}
			}
			break;

			case kMenu_SaveAsCheatCode:
				// underconstruction...
				break;

			case kMenu_SaveAsWatchCode:
			{
				cheat_entry temp_entry;

				memset(&temp_entry, 0, sizeof(cheat_entry));

				add_cheat_from_watch_as_watch(machine, &temp_entry, entry);
				save_cheat_code(machine, &temp_entry);
				dispose_cheat(&temp_entry);
			}
			break;

			case kMenu_Return:
				menu->sel = -1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		reload_cheat_database(machine);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL) || input_ui_pressed(machine, IPT_UI_LEFT) || input_ui_pressed(machine, IPT_UI_RIGHT))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*--------------------------------------------------------
  edit_watch_menu - management for watchpoint edit menu
--------------------------------------------------------*/

static int edit_watch_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
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

		kMenu_Return,
		kMenu_Max };

	UINT8			total		= 0;
	UINT32			increment	= 1;
	const char		** menuItem;
	const char		** menuSubItem;
	char			** buf;
	char			* flagBuf;
	watch_info		*entry		= &watch_list[menu->pre_sel];
	cpu_region_info *info		= get_cpu_or_region_info(entry->cpu);

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	request_strings(kMenu_Max + 1, kMenu_Return, 0, 20);
	menuItem		= menu_strings.main_list;
	menuSubItem		= menu_strings.sub_list;
	buf				= menu_strings.sub_strings;
	flagBuf			= menu_strings.flag_list;

	memset(flagBuf, 0, kMenu_Max + 1);

	/********** MENU CONSTRUCTION **********/
	/* ##### ADDRESS ##### */
	sprintf(buf[total], "%.*X", info->address_chars_needed, entry->address >> entry->address_shift);
	menuItem[total] = "Address";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### CPU/REGION ##### */
	sprintf(buf[total], "%s", get_region_name(entry->cpu));
	menuItem[total] = entry->cpu < REGION_INVALID ? "CPU" : "Region";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### LENGTH ##### */
	sprintf(buf[total], "%d", entry->num_elements);
	menuItem[total] = "Length";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### ELEMENT SIZE ##### */
	menuItem[total] = "Element Size";
	menuSubItem[total++] = kByteSizeStringList[entry->element_bytes];

	/* ##### LABEL TYPE ##### */
	menuItem[total] = "Label Type";
	menuSubItem[total++] = kWatchLabelStringList[entry->label_type];

	/* ##### TEXT LABEL ##### */
	menuItem[total] = "Text Label";
	if(entry->label[0])
		menuSubItem[total++] = entry->label;
	else
		menuSubItem[total++] = "(none)";

	/* ##### DISPLAY TYPE ##### */
	menuItem[total] = "Display Type";
	menuSubItem[total++] = kWatchDisplayTypeStringList[entry->display_type];

	/* ##### X POSITION ##### */
	sprintf(buf[total], "%f", entry->x);
	menuItem[total] = "X";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### Y POSITION ##### */
	sprintf(buf[total], "%f", entry->y);
	menuItem[total] = "Y";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### SKIP BYTES ##### */
	sprintf(buf[total], "%d", entry->skip);
	menuItem[total] = "Skip Bytes";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### ELEMENTS PER LINE ##### */
	sprintf(buf[total], "%d", entry->elements_per_line);
	menuItem[total] = "Elements Per Line";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### ADD VALUE ##### */
	menuItem[total] = "Add Value";
	if(entry->add_value < 0)
		sprintf(buf[total], "-%.2X", -entry->add_value);
	else
		sprintf(buf[total], "%.2X", entry->add_value);
	menuSubItem[total] = buf[total];
	total++;

	/* ##### ADDRESS SHIFT ##### */
	sprintf(buf[total], "%d", entry->address_shift);
	menuItem[total] = "Address Shift";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### DATA SHIFT ##### */
	sprintf(buf[total], "%d", entry->data_shift);
	menuItem[total] = "Data Shift";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### XOR ##### */
	sprintf(buf[total], "%.*X", kSearchByteDigitsTable[kWatchSizeConversionTable[entry->element_bytes]], entry->xor);
	menuItem[total] = "XOR";
	menuSubItem[total] = buf[total];
	total++;

	/* ##### RETURN ##### */
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* higlighted sub-item if edit mode */
	if(editActive)
		flagBuf[menu->sel] = 1;

	/* draw it */
	old_style_menu(menuItem, menuSubItem, flagBuf, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		if(editActive)
		{
			// underconstruction...
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_Address:
					entry->address = DoShift(entry->address, entry->address_shift);
					entry->address -= increment;
					entry->address = DoShift(entry->address, -entry->address_shift);

					if(entry->cpu < REGION_INVALID)
						entry->address &= info->address_mask;
					break;

				case kMenu_CPU:
					entry->cpu--;

					if(entry->cpu >= cpu_gettotalcpu())
						entry->cpu = cpu_gettotalcpu() - 1;

					entry->address &= info->address_mask;
					break;

				case kMenu_NumElements:
					if(entry->num_elements > 0)
						entry->num_elements--;
					else
						entry->num_elements = 0;
					break;

				case kMenu_ElementSize:
					if(entry->element_bytes > 0)
						entry->element_bytes--;
					else
						entry->element_bytes = 0;

					entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->element_bytes]];
					break;

				case kMenu_LabelType:
					if(entry->label_type > 0)
						entry->label_type--;
					else
						entry->label_type = 0;
					break;

				case kMenu_TextLabel:
					break;

				case kMenu_DisplayType:
					if(entry->display_type > 0)
						entry->display_type--;
					else
						entry->display_type = 0;
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
					if(entry->elements_per_line > 0)
						entry->elements_per_line--;
					break;

				case kMenu_AddValue:
					entry->add_value = (entry->add_value - 1) & 0xFF;
					break;

				case kMenu_AddressShift:
					if(entry->address_shift > -31)
						entry->address_shift--;
					else
						entry->address_shift = 31;
					break;

				case kMenu_DataShift:
					if(entry->data_shift > -31)
						entry->data_shift--;
					else
						entry->data_shift = 31;
					break;

				case kMenu_XOR:
					entry->xor -= increment;
					entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->element_bytes]];
			}
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		if(editActive)
		{
			// underconstruction...
		}
		else
		{
			switch(menu->sel)
			{
				case kMenu_Address:
					entry->address = DoShift(entry->address, entry->address_shift);
					entry->address += increment;
					entry->address = DoShift(entry->address, -entry->address_shift);

					if(entry->cpu < REGION_INVALID)
						entry->address &= info->address_mask;
					break;

				case kMenu_CPU:
					entry->cpu++;

					if(entry->cpu >= cpu_gettotalcpu())
						entry->cpu = 0;

					entry->address &= info->address_mask;
					break;

				case kMenu_NumElements:
					entry->num_elements++;
					break;

				case kMenu_ElementSize:
					if(entry->element_bytes < kSearchSize_32Bit)
						entry->element_bytes++;
					else
						entry->element_bytes = kSearchSize_32Bit;

					entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->element_bytes]];
					break;

				case kMenu_LabelType:
					if(entry->label_type < kWatchLabel_MaxPlusOne - 1)
						entry->label_type++;
					else
						entry->label_type = kWatchLabel_MaxPlusOne - 1;
					break;

				case kMenu_TextLabel:
					break;

				case kMenu_DisplayType:
					if(entry->display_type < kWatchDisplayType_MaxPlusOne - 1)
						entry->display_type++;
					else
						entry->display_type = kWatchDisplayType_MaxPlusOne - 1;
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
					entry->elements_per_line++;
					break;

				case kMenu_AddValue:
					entry->add_value = (entry->add_value + 1) & 0xFF;
					break;

				case kMenu_AddressShift:
					if(entry->address_shift < 31)
						entry->address_shift++;
					else
						entry->address_shift = -31;
					break;

				case kMenu_DataShift:
					if(entry->data_shift < 31)
						entry->data_shift++;
					else
						entry->data_shift = -31;
					break;

				case kMenu_XOR:
					entry->xor += increment;
					entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->element_bytes]];
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(editActive)
			editActive = 0;
		else
		{
			switch(menu->sel)
			{
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
					break;

				case kMenu_Return:
					menu->sel = -1;
			}
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CLEAR))
	{
		reset_watch(entry);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		if(editActive)
			editActive = 0;
		else
			menu->sel = -1;
	}

	if(editActive)
	{
		switch(menu->sel)
		{
			case kMenu_Address:
				entry->address = DoShift(entry->address, entry->address_shift);
				entry->address = do_edit_hex_field(machine, entry->address);
				entry->address = DoShift(entry->address, -entry->address_shift);
				entry->address &= info->address_mask;
				break;

			case kMenu_CPU:
				entry->cpu = DoEditDecField(entry->cpu, 0, cpu_gettotalcpu() - 1);
				entry->address &= info->address_mask;
				break;

			case kMenu_NumElements:
				entry->num_elements = DoEditDecField(entry->num_elements, 0, 99);
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
				entry->add_value = DoEditHexFieldSigned(entry->add_value, 0xFFFFFF80) & 0xFF;
				break;

			case kMenu_AddressShift:
				entry->address_shift = DoEditDecField(entry->address_shift, -31, 31);
				break;

			case kMenu_DataShift:
				entry->data_shift = DoEditDecField(entry->data_shift, -31, 31);
				break;

			case kMenu_XOR:
				entry->xor = do_edit_hex_field(machine, entry->xor);
				entry->xor &= kSearchByteMaskTable[kWatchSizeConversionTable[entry->element_bytes]];
				break;
		}
	}
	else
	{
		if(input_ui_pressed(machine, IPT_UI_ADD_CHEAT))
			add_cheat_from_watch(machine, entry);

		if(input_ui_pressed(machine, IPT_UI_DELETE_CHEAT))
			entry->num_elements = 0;

		if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
		{
			cheat_entry temp_entry;

			memset(&temp_entry, 0, sizeof(cheat_entry));

			add_cheat_from_watch_as_watch(machine, &temp_entry, entry);
			save_cheat_code(machine, &temp_entry);
			dispose_cheat(&temp_entry);
		}
	}

	return menu->sel + 1;
}

/*---------------------------------------------------
  select_option_menu - management for options menu
---------------------------------------------------*/

static int select_option_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_SelectSearch = 0,
		kMenu_Loadcheat_options,
		kMenu_save_cheat_options,
		kMenu_Resetcheat_options,
		kMenu_Separator,
		kMenu_SearchDialogStyle,
		kMenu_ShowSearchLabels,
		kMenu_AutoSaveCheats,
		kMenu_ShowActivationKeyMessage,
		kMenu_LoadOldFormat,
#ifdef MESS
		kMenu_SharedCode,
#endif
		kMenu_Return,

		kMenu_Max };

	UINT8			total		= 0;
	const char		* menu_item[kMenu_Max + 1];
	const char		* menu_sub_item[kMenu_Max + 1];

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/********** MENU CONSTRUCTION **********/
	/* ##### SEARCH REGION ##### */
	ADD_MENU_2_ITEMS("Select Search", NULL);

	/* ##### LOAD CHEAT OPTIONS ##### */
	ADD_MENU_2_ITEMS("Load Cheat Options", NULL);

	/* ##### SAVE CHEAT OPTIONS ##### */
	ADD_MENU_2_ITEMS("Save Cheat Options", NULL);

	/* ##### RESET CHEAT OPTIONS ##### */
	ADD_MENU_2_ITEMS("Reset Cheat Options", NULL);

	/* ##### SEPARATOR ##### */
	ADD_MENU_2_ITEMS(MENU_SEPARATOR_ITEM, NULL);

	/* ##### SEARCH MENU ##### */
	menu_item[total] = "Search Dialog Style";
	switch(EXTRACT_FIELD(cheat_options, SearchBox))
	{
		case kSearchBox_Minimum:
			menu_sub_item[total++] = "Minimum";
			break;

		case kSearchBox_Standard:
			menu_sub_item[total++] = "Standard";
			break;

		case kSearchBox_Advanced:
			menu_sub_item[total++] = "Advanced";
			break;

		default:
			menu_sub_item[total++] = "<< No Search Box >>";
			break;
	}

	/* ##### SEARCH LABEL ##### */
	ADD_MENU_2_ITEMS("Show Search Labels", TEST_FIELD(cheat_options, DontPrintNewLabels) ? "Off" : "On");

	/* ##### AUTO SAVE ##### */
	ADD_MENU_2_ITEMS("Auto Save Cheats", TEST_FIELD(cheat_options, AutoSaveEnabled) ? "On" : "Off");

	/* ##### ACTIVATION KEY MESSAGE ##### */
	ADD_MENU_2_ITEMS("Show Activation Key Message", TEST_FIELD(cheat_options, ActivationKeyMessage) ? "On" : "Off");

	/* ##### OLD FORMAT LOADING ##### */
	ADD_MENU_2_ITEMS("Load Old Format Code", TEST_FIELD(cheat_options, LoadOldFormat) ? "On" : "Off");

#ifdef MESS
	/* ##### SHARED CODE ##### */
	ADD_MENU_2_ITEMS("Shared Code", TEST_FIELD(cheat_options, SharedCode) ? "On" : "Off");
#endif

	/* ##### RETURN ##### */
	ADD_MENU_2_ITEMS("Return to Prior Menu", NULL);

	/* ##### TERMINATE ALLAY ##### */
	TERMINATE_MENU_2_ITEMS;

	/* draw it */
	old_style_menu(menu_item, menu_sub_item, NULL, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);

		if(menu->sel == kMenu_Separator)
			menu->sel--;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);

		if(menu->sel == kMenu_Separator)
			menu->sel++;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		switch(menu->sel)
		{
			case kMenu_SearchDialogStyle:
			{
				UINT8 length = (EXTRACT_FIELD(cheat_options, SearchBox) - 1) & 3;

				if(length > kSearchBox_Advanced)
					length = kSearchBox_Advanced;

				SET_FIELD(cheat_options, SearchBox, length);
			}
			break;

			case kMenu_ShowSearchLabels:
				TOGGLE_MASK_FIELD(cheat_options, DontPrintNewLabels);
				break;

			case kMenu_AutoSaveCheats:
				TOGGLE_MASK_FIELD(cheat_options, AutoSaveEnabled);
				break;

			case kMenu_ShowActivationKeyMessage:
				TOGGLE_MASK_FIELD(cheat_options, ActivationKeyMessage);
				break;

			case kMenu_LoadOldFormat:
				TOGGLE_MASK_FIELD(cheat_options, LoadOldFormat);
				break;
#ifdef MESS
			case kMenu_SharedCode:
				TOGGLE_MASK_FIELD(cheat_options, SharedCode);
#endif
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		switch(menu->sel)
		{
			case kMenu_SearchDialogStyle:
			{
				UINT8 length = (EXTRACT_FIELD(cheat_options, SearchBox) + 1) & 3;

				if(length > kSearchBox_Advanced)
					length = kSearchBox_Minimum;

				SET_FIELD(cheat_options, SearchBox, length);
			}
			break;

			case kMenu_ShowSearchLabels:
				TOGGLE_MASK_FIELD(cheat_options, DontPrintNewLabels);
				break;

			case kMenu_AutoSaveCheats:
				TOGGLE_MASK_FIELD(cheat_options, AutoSaveEnabled);
				break;

			case kMenu_ShowActivationKeyMessage:
				TOGGLE_MASK_FIELD(cheat_options, ActivationKeyMessage);
				break;

			case kMenu_LoadOldFormat:
				TOGGLE_MASK_FIELD(cheat_options, LoadOldFormat);

#ifdef MESS
			case kMenu_SharedCode:
				TOGGLE_MASK_FIELD(cheat_options, SharedCode);
#endif
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case kMenu_SelectSearch:
				cheat_menu_stack_push(select_search_menu, menu->handler, menu->sel);
				break;

			case kMenu_Loadcheat_options:
				load_cheat_database(machine, LOAD_CHEAT_OPTION);
				break;

			case kMenu_save_cheat_options:
				save_cheat_options();
				break;

			case kMenu_Resetcheat_options:
				cheat_options = DEFAULT_CHEAT_OPTIONS;
				break;

			case kMenu_Return:
				menu->sel = -1;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SAVE_CHEAT))
	{
		save_cheat_options();
	}
	else if(input_ui_pressed(machine, IPT_UI_RELOAD_CHEAT))
	{
		load_cheat_database(machine, LOAD_CHEAT_OPTION);
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*------------------------------------------------------------
  select_search_menu - management for search selection menu
------------------------------------------------------------*/

static int select_search_menu(running_machine *machine, cheat_menu_stack *menu)
{
#if 0
	INT32			sel;
	const char		** menuItem;
	char			** buf;
	INT32			i;
	INT32			total = 0;

	sel = selection - 1;

	request_strings(search_list_length + 2, search_list_length, 300, 0);

	menuItem =	menu_strings.main_list;
	buf =		menu_strings.main_strings;

	/********** MENU CONSTRUCTION **********/
	for(i = 0; i < search_list_length; i++)
	{
		search_info *info = &search_list[i];

		if(i == current_search_idx)
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

	/* adjust current cursor position */
	if(sel < 0)
		sel = 0;
	if(sel > (total - 1))
		sel = total - 1;

	/* draw it */
	old_style_menu(menuItem, NULL, NULL, sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		sel++;

		if(sel >= total)
			sel = 0;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		sel--;

		if(sel < 0)
			sel = total - 1;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		sel -= fullMenuPageHeight;

		if(sel < 0)
			sel = 0;
	}

	if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
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
		if(search_list_length > 1)
			DeleteSearchAt(sel);
	}

	if(input_ui_pressed(machine, IPT_UI_EDIT_CHEAT))
	{
		if(sel < total - 1)
			current_search_idx = sel;
	}

	if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(sel >= total - 1)
			sel = -1;
		else
			current_search_idx = sel;
	}

	if(input_ui_pressed(machine, IPT_UI_CANCEL))
		sel = -1;
#endif
	return menu->sel;
}

/*-----------------------------------------------------------------
  command_cheat_menu - management for cheat general command menu
-----------------------------------------------------------------*/

static int command_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		MENU_RELOAD_CHEAT_CODE = 0,
		MENU_SEPARATOR_1,
		MENU_TOGGLE_CHEAT,
		MENU_TOGGLE_WATCHPOINT,
		MENU_SEPARATOR_2,
		MENU_SAVE_DESCRIPTION,
		MENU_SAVE_RAW_CODE,
		MENU_SEPARATOR_3,
		MENU_RETURN,
		MENU_MAX };

	int				total	= 0;
	int				toggle	= 0;
	ui_menu_item	menu_item[MENU_MAX + 1];

	memset(menu_item, 0 , sizeof(menu_item));

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/********** MENU CONSTRUCION **********/
	menu_item[total++].text = "Reload Cheat Codes";
	menu_item[total++].text = MENU_SEPARATOR_ITEM;
	if(cheats_disabled)	menu_item[total++].text = "Cheat OFF";
	else				menu_item[total++].text = "Cheat ON";
	if(watches_disabled)	menu_item[total++].text = "Watchpoints OFF";
	else					menu_item[total++].text = "Watchpoints ON";
	menu_item[total++].text = MENU_SEPARATOR_ITEM;
	menu_item[total++].text = "Save Description";
	menu_item[total++].text = "Save Raw Code";
	menu_item[total++].text = MENU_SEPARATOR_ITEM;
	menu_item[total++].text = "Return to Prior Menu";
	menu_item[total].text = NULL;

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* print it */
	ui_menu_draw(menu_item, total, menu->sel, NULL);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);

		if(menu->sel == MENU_SEPARATOR_1 || menu->sel == MENU_SEPARATOR_2 || menu->sel == MENU_SEPARATOR_3)
			menu->sel--;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);

		if(menu->sel == MENU_SEPARATOR_1 || menu->sel == MENU_SEPARATOR_2 || menu->sel == MENU_SEPARATOR_3)
			menu->sel++;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		toggle = 1;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		toggle = 1;
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case MENU_RELOAD_CHEAT_CODE:
				reload_cheat_database(machine);
				break;

			case MENU_TOGGLE_CHEAT:
			case MENU_TOGGLE_WATCHPOINT:
				toggle = 1;
				break;

			case MENU_SAVE_DESCRIPTION:
				save_description(machine);
				break;

			case MENU_SAVE_RAW_CODE:
				save_raw_code(machine);
				break;

			case MENU_RETURN:
				menu->sel = -1;
				break;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	if(toggle)
	{
		if(menu->sel == MENU_TOGGLE_CHEAT)				cheats_disabled ^= 1;
		else if(menu->sel == MENU_TOGGLE_WATCHPOINT)	watches_disabled ^= 1;
	}

	return menu->sel + 1;
}

#ifdef MAME_DEBUG
/*-----------------------------------------------------------------------
  check_activation_key_code_menu - key code checker for activation key
-----------------------------------------------------------------------*/

static int check_activation_key_code_menu(running_machine *machine, cheat_menu_stack *menu)
{
	int				code		= input_code_poll_switches(FALSE);
	static int		index		= INPUT_CODE_INVALID;
	char			stringsBuf[64];
	char			* buf		= stringsBuf;
	astring			* keyIndex	= astring_alloc();

	if(code != INPUT_CODE_INVALID)
	{
		/* NOTE : if first, no action to prevent from wrong display */
		if(menu->first_time)
			menu->first_time = 0;
		else
			index = code;
	}

	/********** MENU CONSTRUCTION **********/
	/* ##### HEADER ##### */
	buf += sprintf(buf, "\tPress a key\n");

	/* ##### KEY NAME / INDEX ##### */
	if(index != INPUT_CODE_INVALID && input_code_pressed(index))
		buf += sprintf(buf, "\t%s\n\t%X\n", astring_c(input_code_name(keyIndex, index)), index);
	else
		buf += sprintf(buf, "\n\n");

	/* ##### RETURN ##### */
	buf += sprintf(buf, "\t Return = Shift + Cancel ");

	/* print it */
	ui_draw_message_window(stringsBuf);

	/* NOTE : shift + cancel is only key to return because normal cancel prevents from diplaying this key */
	if(ShiftKeyPressed() && input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		index = INPUT_CODE_INVALID;
		menu->sel = -1;
	}

	astring_free(keyIndex);

	return menu->sel + 1;
}

/*------------------------------------------------------------------------------------------------
  view_cpu_region_info_list_menu - view internal CPU or Region info list called from debug menu
------------------------------------------------------------------------------------------------*/

static int view_cpu_region_info_list_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_Header = 0,
		kMenu_Type,
		kMenu_DataBits,
		kMenu_AddressBites,
		kMenu_AddressMask,
		kMenu_Endianness,
		kMenu_AddressShift,
		kMenu_Return,
		kMenu_Max };

	UINT8		total	= 0;
	static INT8	index;
	const char	** menuItem;
	const char	** menuSubItem;
	char		* headerBuf;
	char		* regionNameBuf;
	char		* dataBitsBuf;
	char		* addressBitsBuf;
	char		* addressMaskBuf;
	char		* addressShiftBuf;
	cpu_region_info *info;

	/* required items = total items + (strings buf * 6 [max chars : 32]) */
	request_strings(kMenu_Max, kMenu_Max - 2, 32, 0);
	menuItem 		= menu_strings.main_list;
	menuSubItem		= menu_strings.sub_list;
	headerBuf		= menu_strings.main_strings[0];
	regionNameBuf	= menu_strings.main_strings[1];
	dataBitsBuf		= menu_strings.main_strings[2];
	addressBitsBuf	= menu_strings.main_strings[3];
	addressMaskBuf	= menu_strings.main_strings[4];
	addressShiftBuf	= menu_strings.main_strings[5];

	/* first setting : set index as 1st */
	if(menu->first_time)
	{
		index = REGION_CPU1 - REGION_INVALID;
		menu->first_time = 0;
	}

	info = get_region_info(index + REGION_INVALID);

	/********** MENU CONSTRUCTION **********/
	/* ##### HEADER ##### */
	menuItem[total] = "Index";
	sprintf(headerBuf, "%2.2d", index);
	menuSubItem[total++] = headerBuf;

	/* ##### CPU/REGION TYPE ##### */
	menuItem[total] = "Region";

	if(index < REGION_CPU8 - REGION_INVALID)
		sprintf(regionNameBuf, "%s (%s)", kRegionNames[index], cputype_name(info->type));
	else
		sprintf(regionNameBuf, "%s (non-cpu)", kRegionNames[info->type - REGION_CPU1 + 1]);
	menuSubItem[total++] = regionNameBuf;

	/* ##### DATA BITS ##### */
	menuItem[total] = "Data Bits";
	sprintf(dataBitsBuf, "%d", info->data_bits);
	menuSubItem[total++] = dataBitsBuf;

	/* ##### ADDRESS BITS ##### */
	menuItem[total] = "Address Bits";
	sprintf(addressBitsBuf, "%d", info->address_bits);
	menuSubItem[total++] = addressBitsBuf;

	/* ##### ADDRESS MASK/LENGTH ##### */
	menuItem[total] = "Address Mask";
	sprintf(addressMaskBuf, "%X (%d)", info->address_mask, info->address_chars_needed);
	menuSubItem[total++] = addressMaskBuf;

	/* ##### ENDIANNESS ##### */
	menuItem[total] = "Endianness";
	menuSubItem[total++] = info->endianness ? "Big" : "Little";

	/* ##### ADDRESS SHIFT ##### */
	menuItem[total] = "AddressShift";
	sprintf(addressShiftBuf, "%d", info->address_shift);
	menuSubItem[total++] = addressShiftBuf;

	/* ##### RETURN ##### */
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* draw it */
	old_style_menu(menuItem, menuSubItem, NULL, menu->sel, 0);

	/* adjust current cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_RIGHT, kHorizontalSlowKeyRepeatRate))
	{
		int i = index + 1;

		/* search next valid info list */
		while(1)
		{
			if(i >= REGION_LIST_LENGTH - 1)
			{
				i = 0;
				continue;
			}

			if(region_info_list[i].type)
			{
				index = i;
				break;
			}
			else
				i++;
		}
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_LEFT, kHorizontalSlowKeyRepeatRate))
	{
		int i = index - 1;

		/* search previous valid info list */
		while(1)
		{
			if(i < 0)
			{
				i = REGION_LIST_LENGTH - 1;
				continue;
			}

			if(region_info_list[i].type)
			{
				index = i;
				break;
			}
			else
				i--;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		if(menu->sel == kMenu_Return)
			menu->sel = -1;
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
	{
		menu->sel = -1;
	}

	return menu->sel + 1;
}

/*-----------------------------------------------------
  debug_cheat_menu - view internal cheat engine data
-----------------------------------------------------*/

static int debug_cheat_menu(running_machine *machine, cheat_menu_stack *menu)
{
	enum{
		kMenu_DriverName = 0,
		kMenu_GameName,
#ifdef MESS
		kMenu_CRC,
#endif
		kMenu_FullMenuPageHeight,
		kMenu_cheat_list_length,
		kMenu_Separator,
		kMenu_CPURegionInfo,
		kMenu_KeyCodeChecker,
		kMenu_Return,
		kMenu_Max };

	UINT8		total		= 0;
	const char	** menuItem;
	const char	** menuSubItem;
	char		* driverNameBuf;
#ifdef MESS
	char		* crcBuf;
#endif
	char		* heightBuf;
	char		* numCodeBuf;

	/* first setting : NONE */
	if(menu->first_time)
		menu->first_time = 0;

	/* required items = (total items + return + terminator) & (strings buf * 3 [MAME]/4 [MESS]) */
	request_strings(kMenu_Max + 2, kMenu_Max, 64, 0);
	menuItem = menu_strings.main_list;
	menuSubItem = menu_strings.sub_list;
	driverNameBuf = menu_strings.main_strings[0];
	heightBuf = menu_strings.main_strings[1];
	numCodeBuf = menu_strings.main_strings[2];
#ifdef MESS
	crcBuf = menu_strings.main_strings[3];
#endif

	/********** MENU CONSTRUCTION **********/
	/* ##### DRIVER NAME ##### */
	menuItem[total] = "Driver";
	{
		astring * driverName = astring_alloc();

		sprintf(driverNameBuf, "%s", astring_c(core_filename_extract_base(driverName, machine->gamedrv->source_file, TRUE)));
		menuSubItem[total++] = driverNameBuf;
		astring_free(driverName);
	}

	/* ##### GAME / MACHINE NAME ##### */
#ifdef MESS
	menuItem[total]			= "Machine";
#else
	menuItem[total] 		= "Game";
#endif
	menuSubItem[total++]	= machine->gamedrv->name;

#ifdef MESS
	/* ##### CRC (MESS ONLY) ##### */
	menuItem[total] = "CRC";
	sprintf(crcBuf, "%X", thisGameCRC);
	menuSubItem[total++] = crcBuf;
#endif

	/* ##### FULL PAGE MENU HEIGHT ##### */
	menuItem[total] = "Default Menu Height";
	sprintf(heightBuf, "%d", fullMenuPageHeight);
	menuSubItem[total++] = heightBuf;

	/* ##### CHEAT LIST LENGTH ##### */
	menuItem[total] = "Total Code Entries";
	sprintf(numCodeBuf, "%d", cheat_list_length);
	menuSubItem[total++] = numCodeBuf;

	/* ##### SEPARATOR ##### */
	menuItem[total] = MENU_SEPARATOR_ITEM;
	menuSubItem[total++] = NULL;

	/* ##### CPU/REGION INFO ##### */
	menuItem[total] = "View CPU/Region Info";
	menuSubItem[total++] = NULL;

	/* ##### ACTIVATION KEY CODE CHEAKCER ##### */
	menuItem[total] = "Check Activation Key Code";
	menuSubItem[total++] = NULL;

	/* ##### RETURN ##### */
	menuItem[total] = "Return to Prior Menu";
	menuSubItem[total++] = NULL;

	/* ##### TERMINATE ARRAY ##### */
	menuItem[total] = NULL;
	menuSubItem[total] = NULL;

	/* adjust cursor position */
	ADJUST_CURSOR(menu->sel, total);

	/* draw it */
	old_style_menu(menuItem, menuSubItem, NULL, menu->sel, 0);

	/********** KEY HANDLING **********/
	if(ui_pressed_repeat_throttle(machine, IPT_UI_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_PREVIOUS(menu->sel, total);

		if(menu->sel == kMenu_Separator)
			menu->sel--;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_TO_NEXT(menu->sel, total);

		if(menu->sel == kMenu_Separator)
			menu->sel++;
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_UP, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_UP(menu->sel);
	}
	else if(ui_pressed_repeat_throttle(machine, IPT_UI_PAGE_DOWN, kVerticalKeyRepeatRate))
	{
		CURSOR_PAGE_DOWN(menu->sel, total);
	}
	else if(input_ui_pressed(machine, IPT_UI_SELECT))
	{
		switch(menu->sel)
		{
			case kMenu_CPURegionInfo:
				cheat_menu_stack_push(view_cpu_region_info_list_menu, menu->handler, menu->sel);
				break;

			case kMenu_KeyCodeChecker:
				cheat_menu_stack_push(check_activation_key_code_menu, menu->handler, menu->sel);
				break;

			case kMenu_Return:
				menu->sel = -1;
				break;
		}
	}
	else if(input_ui_pressed(machine, IPT_UI_CANCEL))
		menu->sel = -1;

	return menu->sel + 1;
}
#endif
/*------------------
  cheate_periodic
------------------*/

static TIMER_CALLBACK( cheat_periodic )
{
	int i;

	if(input_ui_pressed(machine, IPT_UI_TOGGLE_CHEAT))
	{
		if(ShiftKeyPressed())
		{
			/* ##### WATCHPOINT ##### */
			watches_disabled ^= 1;

			ui_popup_time(1, "Watchpoints %s", watches_disabled ? "Off" : "On");
		}
		else
		{
			/* ##### CHEAT ##### */
			cheats_disabled ^= 1;

			ui_popup_time(1, "Cheats %s", cheats_disabled ? "Off" : "On");

			if(cheats_disabled)
			{
				for(i = 0; i < cheat_list_length; i++)
					temp_deactivate_cheat(machine, &cheat_list[i]);
			}
		}
	}

	if(cheats_disabled)
		return;

	for(i = 0; i < cheat_list_length; i++)
		cheat_periodicEntry(machine, &cheat_list[i]);

	if(EXTRACT_FIELD(cheat_options, SearchBox) == kSearchBox_Minimum)
		SET_FIELD(cheat_options, SearchBox, kSearchBox_Standard);
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
		case kSearchSize_8Bit:
		case kSearchSize_1Bit:
		default:
			buf[0] = (data >> 0) & 0xFF;
			buf[1] = 0;

			return 1;

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
	}
}

/*---------------------------------------------
  cheat_display_watches - display watchpoint
---------------------------------------------*/

void cheat_display_watches(running_machine *machine)
{
	int i;

	if(watches_disabled)
		return;

	for(i = 0; i < watch_list_length; i++)
	{
		watch_info *info = &watch_list[i];

		int			j;
		int			numChars;
		int			xOffset			= 0;
		int			yOffset			= 0;
		int			lineElements	= 0;
		UINT32		address			= info->address;

		char		buf[1024];

		if(info->num_elements)
		{
			/* ##### LABEL ##### */
			switch(info->label_type)
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

			/* ##### VALUE ##### */
			for(j = 0; j < info->num_elements; j++)
			{
				UINT32 data = 0;

				if(info->cpu < REGION_INVALID)
				{
					UINT8 cpu = EXTRACT_FIELD(info->cpu, CPUIndex);

					switch(EXTRACT_FIELD(info->cpu, AddressSpace))
					{
						case kAddressSpace_Program:
							data = (do_cpu_read(cpu, address, kSearchByteIncrementTable[info->element_bytes], cpu_needs_swap(cpu)) + info->add_value) & kSearchByteMaskTable[info->element_bytes];
							break;

						case kAddressSpace_DataSpace:
						case kAddressSpace_IOSpace:
						case kAddressSpace_OpcodeRAM:
						{
							UINT8 *buf = (UINT8 *)get_memory_region_base_pointer(cpu, EXTRACT_FIELD(info->cpu, AddressSpace), address);

							if(buf)
								data =	do_memory_read(buf, address, kSearchByteIncrementTable[info->element_bytes], cpu_needs_swap(cpu) + info->add_value, get_cpu_info(cpu)) &
										kSearchByteMaskTable[info->element_bytes];
						}
						break;

						default:
							return;
					}
				}
				else
				{
					UINT8 * buf = memory_region(machine, info->cpu);

					if(buf)
						data =	do_memory_read(buf, address, kSearchByteIncrementTable[info->element_bytes],
								region_needs_swap(info->cpu) + info->add_value,
								get_region_info(info->cpu)) & kSearchByteMaskTable[info->element_bytes];
				}

				data = DoShift(data, info->data_shift);
				data ^= info->xor;

				if(	(lineElements >= info->elements_per_line) && info->elements_per_line)
				{
					lineElements = 0;
					xOffset = 0;
					yOffset++;
				}

				switch(info->display_type)
				{
					case kWatchDisplayType_Hex:
						numChars = sprintf(buf, "%.*X", kSearchByteDigitsTable[info->element_bytes], data);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_Decimal:
						numChars = sprintf(buf, "%.*d", kSearchByteDecDigitsTable[info->element_bytes], data);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_Binary:
						numChars = PrintBinary(buf, data, kSearchByteMaskTable[info->element_bytes]);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						xOffset++;
						break;

					case kWatchDisplayType_ASCII:
						numChars = PrintASCII(buf, data, info->element_bytes);

						ui_draw_text(buf, xOffset * ui_get_char_width('0') + info->x, yOffset * ui_get_line_height() + info->y);
						xOffset += numChars;
						break;
				}

				address += kSearchByteIncrementTable[info->element_bytes] + info->skip;
				lineElements++;
			}
		}
	}
}

/*--------------------------------------------------------
  resize_cheat_list - memory allocation for cheat entry
--------------------------------------------------------*/

static void resize_cheat_list(UINT32 new_length, UINT8 dispose)
{
	if(new_length != cheat_list_length)
	{
		/* ??? REQUEST_DISPOSE is only when INSERT right now so that it's meaningless ??? */
		if(dispose == REQUEST_DISPOSE)
		{
			/* free cheat entry if delete */
			if(new_length < cheat_list_length)
			{
				int i;

				for(i = new_length; i < cheat_list_length; i++)
					dispose_cheat(&cheat_list[i]);
			}
		}

		/* reallocate cheat entry */
		cheat_list = realloc(cheat_list, new_length * sizeof(cheat_entry));

		if(cheat_list == NULL && new_length != 0)
		{
			fatalerror("cheat: [cheat entry] memory allocation error\n"
						"	length =		%.8X\n"
						"	new_length =	%.8X\n"
						"	cheat_list =	%p\n",
						cheat_list_length, new_length, cheat_list);
		}

		/* add new entry if insert */
		if(new_length > cheat_list_length)
		{
			int i;

			memset(&cheat_list[cheat_list_length], 0, (new_length - cheat_list_length) * sizeof(cheat_entry));

			/* set dirty flag to inserted entry */
			for(i = cheat_list_length; i < new_length; i++)
				cheat_list[i].flags |= kCheatFlag_Dirty;
		}

		cheat_list_length = new_length;
	}
}

/*-----------------------------------------------------------------
  add_cheat_before - insert new cheat entry before selected code
-----------------------------------------------------------------*/

static void add_cheat_before(UINT32 idx)
{
	/* insert new cheat entry */
	resize_cheat_list(cheat_list_length + 1, REQUEST_DISPOSE);

	/* pack later entry if inserting point is not last */
	if(idx < cheat_list_length - 1)
		memmove(&cheat_list[idx + 1], &cheat_list[idx], sizeof(cheat_entry) * (cheat_list_length - 1 - idx));

	if(idx >= cheat_list_length)
		idx = cheat_list_length - 1;

	/* initialize inserted entry */
	memset(&cheat_list[idx], 0, sizeof(cheat_entry));

	/* set dirty flag (meaningelss?) */
	cheat_list[idx].flags |= kCheatFlag_Dirty;

	/* insert new cheat action together */
	resize_cheat_action_list(&cheat_list[idx], 1, REQUEST_DISPOSE);

	cheat_list[idx].action_list[0].extend_data = ~0;
	cheat_list[idx].action_list[0].last_value = NULL;

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_ADD);
}

/*------------------------------------------------
  delete_cheat_at - delete selected cheat entry
------------------------------------------------*/

static void delete_cheat_at(UINT32 idx)
{
	/* if selected point is not cheat entry, no action */
	if(idx >= cheat_list_length)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
		return;
	}

	/* free selected cheat entry */
	dispose_cheat(&cheat_list[idx]);

	/* pack later cheat entry if selected point is not last */
	if(idx < cheat_list_length - 1)
	{
		memmove(&cheat_list[idx], &cheat_list[idx + 1], sizeof(cheat_entry) * (cheat_list_length - 1 - idx));
	}

	/* realloc cheat entry */
	resize_cheat_list(cheat_list_length - 1, NO_DISPOSE);

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_DELETE);
}

/*--------------------------------------------------------------------------------
  dispose_cheat - free selected cheat entry and all cheat actions in this entry
--------------------------------------------------------------------------------*/

static void dispose_cheat(cheat_entry *entry)
{
	if(entry)
	{
		int i;

		free(entry->name);
		free(entry->comment);
		free(entry->label_index);

		/* free all cheat actions in selected entry */
		for(i = 0; i < entry->action_list_length; i++)
			dispose_action(&entry->action_list[i]);

		free(entry->action_list);

		memset(entry, 0, sizeof(cheat_entry));
	}
}

/*-------------------------------------------------------------------------------
  get_new_cheat - return pointer to new cheat entry inserted at last position
-------------------------------------------------------------------------------*/

static cheat_entry *get_new_cheat(void)
{
	/* insert new cheat entry at last position */
	add_cheat_before(cheat_list_length);

	return &cheat_list[cheat_list_length - 1];
}

/*-----------------------------------------------------------------------
  resize_cheat_action_list - reallocate cheat action in selected entry
-----------------------------------------------------------------------*/

static void resize_cheat_action_list(cheat_entry *entry, UINT32 new_length, UINT8 dispose)
{
	if(new_length != entry->action_list_length)
	{
		if(dispose == REQUEST_DISPOSE)
		{
			/* free cheat action if delete */
			if(new_length < entry->action_list_length)
			{
				int i;

				for(i = new_length; i < entry->action_list_length; i++)
					dispose_action(&entry->action_list[i]);
			}
		}

		/* reallocate cheat action */
		entry->action_list = realloc(entry->action_list, new_length * sizeof(cheat_action));

		if(entry->action_list == NULL && new_length != 0)
		{
			fatalerror("cheat: [cheat action] memory allocation error\n"
						"	length =		%.8X\n"
						"	new_length =	%.8X\n"
						"	action_list =	%p\n",
						entry->action_list_length, new_length, entry->action_list);
		}

		/* add new action if insert */
		if(new_length > entry->action_list_length)
		{
			memset(&entry->action_list[entry->action_list_length], 0, (new_length - entry->action_list_length) * sizeof(cheat_action));
		}

		entry->action_list_length = new_length;
	}
}

#if 0
/*----------------------------------------------------------------
  AddActionBefore - Insert empty Action
                    This function is only called in EditCheat()
----------------------------------------------------------------*/

static void AddActionBefore(CheatEntry * entry, UINT32 idx)
{
	ResizeCheatActionList(entry, entry->actionListLength + 1);

	if(idx < (entry->actionListLength - 1))
		memmove(&entry->actionList[idx + 1], &entry->actionList[idx], sizeof(CheatAction) * (entry->actionListLength - 1 - idx));

	if(idx >= entry->actionListLength)
		idx = entry->actionListLength - 1;

	memset(&entry->actionList[idx], 0, sizeof(CheatAction));

	cheat_list[idx].actionList[0].extendData = ~0;
	cheat_list[idx].actionList[0].lastValue = NULL;
}

/*-----------------
  DeleteActionAt
-----------------*/

static void DeleteActionAt(CheatEntry * entry, UINT32 idx)
{
	if(idx >= entry->actionListLength)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
		return;
	}

	DisposeAction(&entry->actionList[idx]);

	if(idx < (entry->actionListLength - 1))
		memmove(&entry->actionList[idx], &entry->actionList[idx + 1], sizeof(CheatAction) * (entry->actionListLength - 1 - idx));

	ResizeCheatActionListNoDispose(entry, entry->actionListLength - 1);

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_DELETE);
}
#endif
/*----------------------------------------------
  dispose_action - free selected cheat action
----------------------------------------------*/

static void dispose_action(cheat_action *action)
{
	if(action != NULL)
	{
		free(action->optional_name);

		if(action->last_value)
			free(action->last_value);

		memset(action, 0, sizeof(cheat_action));
	}
}

/*-------------------------------------------------
  init_watch - initialize a value for watchpoint
-------------------------------------------------*/

static void init_watch(watch_info *info, UINT32 idx)
{
	/* NOTE : 1st watchpoint should be always Y = 0 */
	if(idx > 0)
		info->y = watch_list[idx - 1].y + ui_get_line_height();
	else
		info->y = 0;
}

/*--------------------------------------------------------------
  resize_watch_list - reallocate watch list if different size
--------------------------------------------------------------*/

static void resize_watch_list(UINT32 new_length, UINT8 dispose)
{
	if(new_length != watch_list_length)
	{
		/* NOTE : dispose flag is only set when called from delete_watch_at() */
		if(dispose)
		{
			/* if delete, free deleted or later watch list */
			if(new_length < watch_list_length)
			{
				int i;

				for(i = new_length; i < watch_list_length; i++)
					dispose_watch(&watch_list[i]);
			}
		}

		/* reallocate watch list */
		watch_list = realloc(watch_list, new_length * sizeof(watch_info));

		if(watch_list == NULL && new_length != 0)
		{
			/* memory allocation error */
			fatalerror(	"cheat : [watch list] memory allocation error\n"
						"	length =		%.8X\n"
						"	new_length =	%.8X\n"
						"	watch_list =	%p\n",
						watch_list_length, new_length, watch_list);
		}

		/* if insert, move later watch list */
		if(new_length > watch_list_length)
		{
			int i;

			memset(&watch_list[watch_list_length], 0, (new_length - watch_list_length) * sizeof(watch_info));

			for(i = watch_list_length; i < new_length; i++)
				init_watch(&watch_list[i], i);
		}

		watch_list_length = new_length;
	}
}

/*-------------------------------------------
  add_watch_before - insert new watch list
-------------------------------------------*/

static void add_watch_before(UINT32 idx)
{
	/* reallocate watch list */
	resize_watch_list(watch_list_length + 1, 0);

	/* if insert position is not last, move later watch list */
	if(idx < watch_list_length - 1)
		memmove(&watch_list[idx + 1], &watch_list[idx], sizeof(watch_info) * (watch_list_length - 1 - idx));
	else
		idx = watch_list_length - 1;

	/* memory initialize */
	memset(&watch_list[idx], 0, sizeof(watch_info));

	init_watch(&watch_list[idx], idx);
}

/*------------------------------------------------
  delete_watch_at - delete selected watch point
------------------------------------------------*/

static void delete_watch_at(UINT32 idx)
{
	/* if selected item is not watchpoint, no delete */
	if(idx >= watch_list_length)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
		return;
	}

	/* free selected watch list */
	dispose_watch(&watch_list[idx]);

	/* move later watch list */
	memmove(&watch_list[idx], &watch_list[idx + 1], sizeof(watch_info) * (watch_list_length - 1 - idx));

	/* reallocate watch list */
	resize_watch_list(watch_list_length - 1, 1);
}

/*-------------------------------------------------
  dispose_watch - initialize selected watch list
-------------------------------------------------*/

static void dispose_watch(watch_info *watch)
{
	memset(watch, 0, sizeof(watch_info));
}

/*--------------------------------------------
  get_unused_watch - find unused watchpoint
--------------------------------------------*/

static watch_info *get_unused_watch(void)
{
	int			i;
	watch_info	*info;
	watch_info	*the_watch = NULL;

	/* search unused watchpoint */
	for(i = 0; i < watch_list_length; i++)
	{
		info = &watch_list[i];

		/* NOTE : "unused" means "undisplayed" */
		if(info->num_elements == 0)
		{
			the_watch = info;
			break;
		}
	}

	/* if all watchpoints are used, insert new watchpoint */
	if(the_watch == NULL)
	{
		add_watch_before(watch_list_length);

		the_watch = &watch_list[watch_list_length - 1];
	}

	return the_watch;
}

/*------------------------------------------------------------
  add_cheat_from_watch - add new cheat code from watchpoint
------------------------------------------------------------*/

static void add_cheat_from_watch(running_machine *machine, watch_info *watch)
{
	if(watch)
	{
		int				temp_string_length;
		char			temp_string[1024];
		cheat_entry		*entry		= get_new_cheat();
		cheat_action	*action		= &entry->action_list[0];

		/* set parameters */
		action->region				= watch->cpu;
		action->address				= watch->address;
		action->original_address	= watch->address;
		action->extend_data			= ~0;
		action->last_value			= NULL;
		action->data 				= read_data(machine, action);
		SET_FIELD(action->type, AddressSize, watch->element_bytes);

		/* set name */
		temp_string_length = sprintf(temp_string, "%.8X (%d) = %.*X", watch->address, watch->cpu, kSearchByteDigitsTable[watch->element_bytes], action->data);
		entry->name = create_string_copy(temp_string);

		update_cheat_info(machine, entry, 0);

		SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_ADD);
	}
	else
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
}

/*---------------------------------------------------------------------------
  add_cheat_from_watch_as_watch - add new watchpoint cheat from watchpoint
---------------------------------------------------------------------------*/

static void add_cheat_from_watch_as_watch(running_machine *machine, cheat_entry *entry, watch_info *watch)
{
	/* NOTE : don't add in case of undisplayed watchpoint */
	if(watch && entry && watch->num_elements)
	{
		char			temp_string[256];
		cheat_action	*action;

		/* free then add new cheat entry */
		dispose_cheat(entry);
		resize_cheat_action_list(entry, 1, REQUEST_DISPOSE);
		action = &entry->action_list[0];

		/* set name and comments */
		sprintf(temp_string, "Watch %.8X (%d)", watch->address, watch->cpu);

		entry->name		= create_string_copy(temp_string);
		entry->comment	= create_string_copy(watch->label);

		/* set parameters */
		action->region				= watch->cpu;
		action->address				= watch->address;
		action->original_address	= watch->address;
		action->original_data		= action->data;
		action->extend_data			= ~0;

		SET_FIELD(action->type, CodeType, kCodeType_Watch);
		SET_FIELD(action->type, AddressSize, kSearchByteIncrementTable[watch->element_bytes] - 1);
		SET_FIELD(action->data, WatchNumElements, watch->num_elements - 1);
		SET_FIELD(action->data, WatchSkip, watch->skip);
		SET_FIELD(action->data, WatchElementsPerLine, watch->elements_per_line);
		SET_FIELD(action->data, WatchAddValue, watch->add_value);

		update_cheat_info(machine, entry, 0);

		SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_ADD);
	}
	else
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
}

/*----------------------------------------------------------------------
  reset_watch - clear data in selected watch list except the position
----------------------------------------------------------------------*/

static void reset_watch(watch_info *watch)
{
	if(watch)
	{
		watch->address				= 0;
		watch->cpu					= 0;
		watch->num_elements			= 0;
		watch->element_bytes		= kWatchSizeConversionTable[0];
		watch->label_type			= kWatchLabel_None;
		watch->display_type			= kWatchDisplayType_Hex;
		watch->skip					= 0;
		watch->elements_per_line	= 0;
		watch->add_value			= 0;
		watch->address_shift		= 0;
		watch->data_shift			= 0;
		watch->xor					= 0;
		watch->linked_cheat			= NULL;
		watch->label[0]				= 0;
	}
	else
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_DELETE);
}

/*------------------------------------------------------------------
  resize_search_list - reallocate search list if different length
------------------------------------------------------------------*/

static void resize_search_list(UINT32 new_length)
{
	if(new_length != search_list_length)
	{
		/* free search list if delete */
		if(new_length < search_list_length)
		{
			int i;

			for(i = new_length; i < search_list_length; i++)
				dispose_search(&search_list[i]);
		}

		/* reallocate search list */
		search_list = realloc(search_list, new_length * sizeof(search_info));

		if(search_list == NULL && new_length != 0)
		{
			fatalerror("cheat: [search info] memory allocation error\n"
						"	length =		%.8X\n"
						"	new_length =	%.8X\n"
						"	search_list =	%p\n",
						search_list_length, new_length, search_list);
		}

		/* add new search list if insert */
		if(new_length > search_list_length)
		{
			int i;

			memset(&search_list[search_list_length], 0, (new_length - search_list_length) * sizeof(search_info));

			for(i = search_list_length; i < new_length; i++)
				init_search(&search_list[i]);
		}

		search_list_length = new_length;
	}
}

/*--------------------------------
  resize_search_list_no_dispose
--------------------------------*/
#if 0
static void ResizeSearchListNoDispose(UINT32 newLength)
{
	if(newLength != search_list_length)
	{
		searchList = realloc(searchList, newLength * sizeof(search_info));

		if(!searchList && (newLength != 0))
		{
			logerror("ResizeSearchList: out of memory resizing search list\n");
			ui_popup_time(2, "out of memory while adding search");

			search_list_length = 0;

			return;
		}

		if(newLength > search_list_length)
			memset(&searchList[search_list_length], 0, (newLength - search_list_length) * sizeof(search_info));

		search_list_length = newLength;
	}
}

/*--------------------
  add_search_before
--------------------*/

static void AddSearchBefore(UINT32 idx)
{
	ResizeSearchListNoDispose(search_list_length + 1);

	if(idx < (search_list_length - 1))
		memmove(&search_list[idx + 1], &search_list[idx], sizeof(search_info) * (search_list_length - 1 - idx));

	if(idx >= search_list_length)
		idx = search_list_length - 1;

	memset(&search_list[idx], 0, sizeof(search_info));

	init_search(&search_list[idx]);
}

/*-------------------
  delete_search_at
-------------------*/

static void DeleteSearchAt(UINT32 idx)
{
	if(idx >= search_list_length)
		return;

	DisposeSearch(idx);

	if(idx < (search_list_length - 1))
		memmove(&search_list[idx], &search_list[idx + 1], sizeof(search_info) * (search_list_length - 1 - idx));

	ResizeSearchListNoDispose(search_list_length - 1);
}
#endif
/*---------------------------------------
  init_search - initialize search info
---------------------------------------*/

static void init_search(search_info *info)
{
	if(info)
		info->search_speed = SEARCH_SPEED_MEDIUM;
}

/*---------------------------------------------------------------------------
  dispose_search_reigons - free all search regions in selected search info
---------------------------------------------------------------------------*/

static void dispose_search_reigons(search_info *info)
{
	if(info->region_list)
	{
		int i;

		for(i = 0; i < info->region_list_length; i++)
		{
			search_region *region = &info->region_list[i];

			free(region->first);
			free(region->last);
			free(region->status);
			free(region->backup_last);
			free(region->backup_status);
		}

		free(info->region_list);

		info->region_list = NULL;
	}

	info->region_list_length = 0;
}

/*-------------------------------------------------------------------------
  dispose_search - free selected search info and all search region in it
-------------------------------------------------------------------------*/

static void dispose_search(search_info *info)
{
	dispose_search_reigons(info);

	free(info->name);

	info->name = NULL;
}

/*--------------------------------------------------
  get_current_search - return working search info
--------------------------------------------------*/

static search_info *get_current_search(void)
{
	if(current_search_idx >= search_list_length)
		current_search_idx = search_list_length - 1;

	if(current_search_idx < 0)
		current_search_idx = 0;

	return &search_list[current_search_idx];
}

/*----------------------------------------------------------------------------------------------
  fill_buffer_from_region - fill selected search region with a value read from search address
----------------------------------------------------------------------------------------------*/

static void fill_buffer_from_region(search_region *region, UINT8 *buf)
{
	UINT32 offset;

	/* ##### optimize if needed ##### */
	for(offset = 0; offset < region->length; offset++)
		buf[offset] = read_region_data(region, offset, 1, 0);
}

/*---------------------------------------------------------------------------
  read_region_data - read a data from memory or search region in searching
---------------------------------------------------------------------------*/

static UINT32 read_region_data(search_region *region, UINT32 offset, UINT8 size, UINT8 swap)
{
	UINT32 address = region->address + offset;

	if(region->cached_pointer != NULL)
	{
		UINT8 *buf = (UINT8 *)region->cached_pointer;

		if(buf)
//          return do_memory_read(region->cached_pointer, address, size, swap, &raw_cpu_info);
			return do_memory_read(buf, address, size, cpu_needs_swap(region->target_idx) ^ swap, get_cpu_info(region->target_idx));
		else
			return 0;
	}

	if(region->target_type == kRegionType_CPU)
	{
		/* NOTE : it conflicts cpu_spinutil() and causes the crash... */
		return do_cpu_read(region->target_idx, address, size, cpu_needs_swap(region->target_idx) ^ swap);
	}

	return 0;
}
/*------------------------------------------------
  backup_search - back up current search region
------------------------------------------------*/

static void backup_search(search_info *info)
{
	int i;

	for(i = 0; i < info->region_list_length; i++)
		backup_region(&info->region_list[i]);

	info->old_num_results = info->num_results;
	info->backup_valid = 1;
}

/*---------------------------------------------------------
  restore_search_backup - restore previous search region
---------------------------------------------------------*/

static void restore_search_backup(search_info *info)
{
	int i;

	if(info && info->backup_valid)
	{
		for(i = 0; i < info->region_list_length; i++)
			restore_region_backup(&info->region_list[i]);

		info->num_results	= info->old_num_results;
		info->backup_valid	= 0;

#if 1
		SET_MESSAGE(CHEAT_MESSAGE_RESTORE_VALUE);
#else
		ui_popup_time(1, "values restored");	/* not displayed when ui is opend */
#endif
	}
	else
	{
#if 1
		SET_MESSAGE(CHEAT_MESSAGE_NO_OLD_VALUE);
#else
		ui_popup_time(1, "there are no old values");	/* not displayed when ui is opend */
#endif
	}
}

/*-------------------------------------------------
  backup_region - back up current search results
-------------------------------------------------*/

static void backup_region(search_region *region)
{
	if(region->flags & kRegionFlag_Enabled)
	{
		memcpy(region->backup_last,		region->last,	region->length);
		memcpy(region->backup_status,	region->status,	region->length);

		region->old_num_results = region->num_results;
	}
}

/*----------------------------------------------------------
  restore_region_backup - restore previous search results
----------------------------------------------------------*/

static void restore_region_backup(search_region *region)
{
	if(region->flags & kRegionFlag_Enabled)
	{
		memcpy(region->last,	region->backup_last,	region->length);
		memcpy(region->status,	region->backup_status,	region->length);

		region->num_results = region->old_num_results;
	}
}

/*--------------------------------------------------------
  default_enable_region - get default regions to search
--------------------------------------------------------*/

static UINT8 default_enable_region(running_machine *machine, search_region *region, search_info *info)
{
	write8_machine_func	handler			= region->write_handler->write.mhandler8;
	FPTR				handler_address	= (FPTR)handler;

	switch(info->search_speed)
	{
		case SEARCH_SPEED_FAST:
			if(handler == SMH_RAM && region->write_handler->baseptr != NULL)
				return 1;

		case SEARCH_SPEED_MEDIUM:
			if(handler_address >= (FPTR)SMH_BANK1 && handler_address <= (FPTR)SMH_BANK32)
				return 1;

			if(handler == SMH_RAM)
				return 1;

			return 0;

		case SEARCH_SPEED_SLOW:
			if(handler == SMH_NOP || handler == SMH_ROM)
				return 0;

			if(handler_address > STATIC_COUNT && region->write_handler->baseptr == NULL)
				return 0;

			return 1;

		case SEARCH_SPEED_VERY_SLOW:
			if(handler == SMH_NOP || handler == SMH_ROM)
				return 0;

			return 1;
	}

	return 0;
}

/*---------------------------------------------------------------------
  set_search_region_default_name - set name of region to region list
---------------------------------------------------------------------*/

static void set_search_region_default_name(search_region *region)
{
	switch(region->target_type)
	{
		case kRegionType_CPU:
		{
			char desc[16];

			if(region->write_handler)
			{
				genf	*handler		= region->write_handler->write.generic;
				FPTR	handler_address	= (FPTR)handler;

				if(handler_address >= (FPTR)SMH_BANK1 && handler_address <= (FPTR)SMH_BANK32)
				{
					sprintf(desc, "BANK%.2d", (int)(handler_address - (FPTR)SMH_BANK1) + 1);
				}
				else
				{
					switch(handler_address)
					{
						case (FPTR)SMH_NOP:		strcpy(desc, "NOP   ");	break;
						case (FPTR)SMH_RAM:		strcpy(desc, "RAM   ");	break;
						case (FPTR)SMH_ROM:		strcpy(desc, "ROM   ");	break;
						default:				strcpy(desc, "CUSTOM");	break;
					}
				}
			}
			else
				sprintf(desc, "CPU%.2d ", region->target_idx);

			sprintf(region->name,	"%.*X-%.*X %s",
									cpu_info_list[region->target_idx].address_chars_needed, region->address,
									cpu_info_list[region->target_idx].address_chars_needed, region->address + region->length - 1,
									desc);
		}
		break;

		case kRegionType_Memory:	/* unused? */
			sprintf(region->name, "%.8X-%.8X MEMORY", region->address, region->address + region->length - 1);
			break;

		default:
			sprintf(region->name, "UNKNOWN");
	}
}

/*------------------------------------------------------------------
  is_search_region_in_range - check valid range for search region
------------------------------------------------------------------*/

static UINT8 is_search_region_in_range(UINT8 cpu, UINT32 length)
{
	if(length > cpu_info_list[cpu].address_mask)
		return 0;

	/* all memory in 32-bit cpu causes zero length in several cases */
	if(length == 0)
		return 0;

	if(length > SAFE_SEARCH_REGION_RANGE)
		return 0;

	return 1;
}

/*----------------------------------------------------------------
  allocate_search_regions - memory allocation for search region
----------------------------------------------------------------*/

static void allocate_search_regions(search_info *info)
{
	int i;

	info->backup_valid	= 0;
	info->num_results	= 0;

	for(i = 0; i < info->region_list_length; i++)
	{
		search_region *region = &info->region_list[i];

		region->num_results = 0;

		free(region->first);
		free(region->last);
		free(region->status);
		free(region->backup_last);
		free(region->backup_status);

		if(region->flags & kRegionFlag_Enabled)
		{
			region->first			= malloc(region->length);
			region->last			= malloc(region->length);
			region->status			= malloc(region->length);
			region->backup_last		= malloc(region->length);
			region->backup_status	= malloc(region->length);

			if(region->first == NULL || region->last == NULL || region->status == NULL || region->backup_last == NULL || region->backup_status == NULL)
			{
				/* NOTE : ignored memory allocation error but locked this region */
				free(region->first);
				free(region->last);
				free(region->status);
				free(region->backup_last);
				free(region->backup_status);

				region->first =			NULL;
				region->last =			NULL;
				region->status =		NULL;
				region->backup_last =	NULL;
				region->backup_status =	NULL;
				region->flags &= 		~kRegionFlag_Enabled;
				region->flags |=		kRegionFlag_HasError;

				SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ALLOCATE);
			}
		}
		else
		{
			region->first			= NULL;
			region->last			= NULL;
			region->status			= NULL;
			region->backup_last		= NULL;
			region->backup_status	= NULL;
		}
	}
}

/*--------------------------------------------------------------
  build_search_regions - build serach regions from memory map
--------------------------------------------------------------*/

static void build_search_regions(running_machine *machine, search_info *info)
{
	info->comparison = kSearchComparison_EqualTo;

	/* 1st, free all search regions in current search_info */
	dispose_search_reigons(info);

	/* 2nd, set parameters from memory map */
	switch(info->target_type)
	{
		case kRegionType_CPU:
		{
			if(info->search_speed == SEARCH_SPEED_ALL_MEMORY)
			{
				/* All Memory = 1 search region */
				UINT32			length = cpu_info_list[info->target_idx].address_mask + 1;
				search_region	*region;

				info->region_list			= calloc(sizeof(search_region), 1);
				info->region_list_length	= 1;

				region					= info->region_list;
				region->address			= 0;
				region->length			= length;
				region->target_idx		= info->target_idx;
				region->target_type		= info->target_type;
				region->write_handler	= NULL;
				region->first			= NULL;
				region->last			= NULL;
				region->status			= NULL;
				region->backup_last		= NULL;
				region->backup_status	= NULL;

				set_search_region_default_name(region);

				if(is_search_region_in_range(info->target_idx, length) == 0)
				{
					region->flags &= ~kRegionFlag_Enabled;
					region->flags |= kRegionFlag_HasError;
				}
				else
					region->flags = kRegionFlag_Enabled;
			}
			else
			{
				if(VALID_CPU(info->target_idx))
				{
					int							count = 0;
					const address_map			*map = NULL;
					const address_map_entry		*entry;
					search_region				*traverse;

					map = memory_get_address_map(info->target_idx, ADDRESS_SPACE_PROGRAM);

					/* calculate total search entries */
					for(entry = map->entrylist; entry != NULL; entry = entry->next)
						if(entry->write.generic)
							count++;

					info->region_list			= calloc(sizeof(search_region), count);
					info->region_list_length	= count;
					traverse					= info->region_list;

					for(entry = map->entrylist; entry != NULL; entry = entry->next)
					{
						if(entry->write.generic)
						{
							UINT32 length = (entry->addrend - entry->addrstart) + 1;

							traverse->address			= entry->addrstart;
							traverse->length			= length;
							traverse->target_idx		= info->target_idx;
							traverse->target_type		= info->target_type;
							traverse->cached_pointer	= get_memory_region_base_pointer(info->target_idx, kAddressSpace_DirectMemory, entry->addrstart);
							traverse->write_handler		= entry;
							traverse->first				= NULL;
							traverse->last				= NULL;
							traverse->status			= NULL;
							traverse->backup_last		= NULL;
							traverse->backup_status		= NULL;

							if(is_search_region_in_range(info->target_idx, length) == 0)
							{
								traverse->flags &= ~kRegionFlag_Enabled;
								traverse->flags |= kRegionFlag_HasError;
							}
							else
								traverse->flags = default_enable_region(machine, traverse, info) ? kRegionFlag_Enabled : 0;

							set_search_region_default_name(traverse);

							traverse++;
						}
					}
				}
			}
		}
		break;

		case kRegionType_Memory:	/* unused? */
			break;
	}
}

/*-------------------------------------------------------------------------
  convert_older_code - convert old format code to new when load database
-------------------------------------------------------------------------*/

static int convert_older_code(int code, int cpu, int *data, int *extend_data)
{
	enum
	{
		CUSTOM_FIELD_NONE =					0,
		CUSTOM_FIELD_DONT_APPLY_CPU_FIELD =	1 << 0,
		CUSTOM_FIELD_SET_BIT =				1 << 1,
		CUSTOM_FIELD_CLEAR_BIT =			1 << 2,
		CUSTOM_FIELD_SUBTRACT_ONE =			1 << 3,
		CUSTOM_FIELD_BIT_MASK =				CUSTOM_FIELD_SET_BIT | CUSTOM_FIELD_CLEAR_BIT,

		CUSTOM_FIELD_END =					0xFF
	};

	struct _conversion_table
	{
		int		old_code;
		UINT32	new_code;
		UINT8	custom_field;
	};

	static const struct _conversion_table conversion_table[] =
	{
		{	0,		0x00000000,	CUSTOM_FIELD_NONE },
		{	1,		0x00000001,	CUSTOM_FIELD_NONE },
		{	2,		0x00000020,	CUSTOM_FIELD_NONE },
		{	3,		0x00000040,	CUSTOM_FIELD_NONE },
		{	4,		0x000000A0,	CUSTOM_FIELD_NONE },
		{	5,		0x00000022,	CUSTOM_FIELD_NONE },
		{	6,		0x00000042,	CUSTOM_FIELD_NONE },
		{	7,		0x000000A2,	CUSTOM_FIELD_NONE },
		{	8,		0x00000024,	CUSTOM_FIELD_NONE },
		{	9,		0x00000044,	CUSTOM_FIELD_NONE },
		{	10,		0x00000064,	CUSTOM_FIELD_NONE },
		{	11,		0x00000084,	CUSTOM_FIELD_NONE },
		{	15,		0x00000023,	CUSTOM_FIELD_NONE },
		{	16,		0x00000043,	CUSTOM_FIELD_NONE },
		{	17,		0x000000A3,	CUSTOM_FIELD_NONE },
		{	20,		0x00000000,	CUSTOM_FIELD_SET_BIT },
		{	21,		0x00000001,	CUSTOM_FIELD_SET_BIT },
		{	22,		0x00000020,	CUSTOM_FIELD_SET_BIT },
		{	23,		0x00000040,	CUSTOM_FIELD_SET_BIT },
		{	24,		0x000000A0,	CUSTOM_FIELD_SET_BIT },
		{	40,		0x00000000,	CUSTOM_FIELD_CLEAR_BIT },
		{	41,		0x00000001,	CUSTOM_FIELD_CLEAR_BIT },
		{	42,		0x00000020,	CUSTOM_FIELD_CLEAR_BIT },
		{	43,		0x00000040,	CUSTOM_FIELD_CLEAR_BIT },
		{	44,		0x000000A0,	CUSTOM_FIELD_CLEAR_BIT },
		{	60,		0x00000103,	CUSTOM_FIELD_NONE },
		{	61,		0x00000303,	CUSTOM_FIELD_NONE },
		{	62,		0x00000503,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	63,		0x00000903,	CUSTOM_FIELD_NONE },
		{	64,		0x00000B03,	CUSTOM_FIELD_NONE },
		{	65,		0x00000D03,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	70,		0x00000101,	CUSTOM_FIELD_NONE },
		{	71,		0x00000301,	CUSTOM_FIELD_NONE },
		{	72,		0x00000501,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	73,		0x00000901,	CUSTOM_FIELD_NONE },
		{	74,		0x00000B01,	CUSTOM_FIELD_NONE },
		{	75,		0x00000D01,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	80,		0x00000102,	CUSTOM_FIELD_NONE },
		{	81,		0x00000302,	CUSTOM_FIELD_NONE },
		{	82,		0x00000502,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	83,		0x00000902,	CUSTOM_FIELD_NONE },
		{	84,		0x00000B02,	CUSTOM_FIELD_NONE },
		{	85,		0x00000D02,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	90,		0x00000100,	CUSTOM_FIELD_NONE },
		{	91,		0x00000300,	CUSTOM_FIELD_NONE },
		{	92,		0x00000500,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	93,		0x00000900,	CUSTOM_FIELD_NONE },
		{	94,		0x00000B00,	CUSTOM_FIELD_NONE },
		{	95,		0x00000D00,	CUSTOM_FIELD_SUBTRACT_ONE },
		{	100,	0x20800000,	CUSTOM_FIELD_NONE },
		{	101,	0x20800001,	CUSTOM_FIELD_NONE },
		{	102,	0x20800000,	CUSTOM_FIELD_NONE },
		{	103,	0x20800001,	CUSTOM_FIELD_NONE },
		{	110,	0x40800000,	CUSTOM_FIELD_NONE },
		{	111,	0x40800001,	CUSTOM_FIELD_NONE },
		{	112,	0x40800000,	CUSTOM_FIELD_NONE },
		{	113,	0x40800001,	CUSTOM_FIELD_NONE },
		{	120,	0x63000001,	CUSTOM_FIELD_NONE },
		{	121,	0x63000001,	CUSTOM_FIELD_DONT_APPLY_CPU_FIELD | CUSTOM_FIELD_SET_BIT },
		{	122,	0x63000001,	CUSTOM_FIELD_DONT_APPLY_CPU_FIELD | CUSTOM_FIELD_CLEAR_BIT },
		{	998,	0x00000006,	CUSTOM_FIELD_NONE },
		{	999,	0x60000000,	CUSTOM_FIELD_DONT_APPLY_CPU_FIELD },
		{	-1,		0x00000000,	CUSTOM_FIELD_END }
	};

	const struct _conversion_table *traverse = conversion_table;

	UINT8	link_cheat = 0;
	UINT32	new_code;

	/* convert link cheats */
	if((code >= 500) && (code <= 699))
	{
		link_cheat = 1;
		code -= 500;
	}

	/* look up code */
	while(traverse->old_code >= 0)
	{
		if(code == traverse->old_code)
			goto found_code;
		traverse++;
	}

	logerror("cheat: [older code conversion] %d not found\n", code);

	/* not found */
	*extend_data = 0;
	return 0;

	/* found */
	found_code:

	new_code = traverse->new_code;

	/* add in the CPU field */
	if((traverse->custom_field & CUSTOM_FIELD_DONT_APPLY_CPU_FIELD) == 0)
		new_code = (new_code & ~0x1F000000) | ((cpu << 24) & 0x1F000000);

	/* hack-ish, subtract one from data field for x5 user select */
	if(traverse->custom_field & CUSTOM_FIELD_SUBTRACT_ONE)
		/* yaay for C operator precedence */
		(*data)--;

	/* set up the extend data */
	if(traverse->custom_field & CUSTOM_FIELD_BIT_MASK)
		*extend_data = *data;
	else
		*extend_data = 0xFFFFFFFF;

	if(traverse->custom_field & CUSTOM_FIELD_CLEAR_BIT)
		*data = 0;

	if(link_cheat)
		SET_MASK_FIELD(new_code, LinkEnable);

	return new_code;
}

/*----------------------
  convert_to_new_code
----------------------*/

static int convert_to_new_code(cheat_action *action)
{
	int new_type = 0;

	if(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed) {
		SET_FIELD(new_type, CodeType, kCodeType_IWrite);
		SET_FIELD(new_type, CodeParameter, EXTRACT_FIELD(action->type, LocationParameterOption)); }
	SET_FIELD(new_type, AddressSize, EXTRACT_FIELD(action->type, BytesUsed));
	SET_MASK_FIELD(new_type, Endianness);
	if(TEST_FIELD(action->type, RestorePreviousValue))
		SET_MASK_FIELD(new_type, RestoreValue);
	if(EXTRACT_FIELD(action->type, Prefill))
		SET_FIELD(new_type, PrefillEnable, 0);
	if(EXTRACT_FIELD(action->type, Type) == kLocation_Standard && EXTRACT_FIELD(action->type, TypeParameter))
		SET_MASK_FIELD(new_type, DelayEnable);

	return new_type;
}

/*-------------------------------------------------------------------
  handle_local_command_cheat - handle specified cheat command code
-------------------------------------------------------------------*/

static void handle_local_command_cheat(running_machine *machine, int cpu, int type, int address, int data, int extend_data, UINT8 format)
{
	int command = 0;

	if(format != FORMAT_NEW)
	{
		switch(EXTRACT_FIELD(type, LocationParameter))
		{
			case kCustomLocation_AssignActivationKey:
				command = CUSTOM_CODE_ACTIVATION_KEY;
				break;

			case kCustomLocation_Enable:
				command = CUSTOM_CODE_PRE_ENABLE;
				break;

			case kCustomLocation_Overclock:
				command = CUSTOM_CODE_OVER_CLOCK;
				break;

			case kCustomLocation_RefreshRate:
				command = CUSTOM_CODE_REFRESH_RATE;
				break;

			default:
				return;
		}
	}
	else
		command = cpu;

	switch(command)
	{
		case CUSTOM_CODE_ACTIVATION_KEY:
			if(address < cheat_list_length)
			{
				cheat_entry *entry = &cheat_list[address];

				if(entry->flags & kCheatFlag_Select)
				{
					if(data)
					{
						entry->activation_key_1 = data;
						entry->flags |= kCheatFlag_HasActivationKey1;
					}
					else if(extend_data)
					{
						entry->activation_key_2 = extend_data;
						entry->flags |= kCheatFlag_HasActivationKey2;
					}
				}
				else
				{
					entry->activation_key_1 = data;
					entry->flags |= kCheatFlag_HasActivationKey1;
				}
			}
			break;

		case CUSTOM_CODE_PRE_ENABLE:
			if(address < cheat_list_length)
			{
				cheat_entry *entry = &cheat_list[address];

				activate_cheat(machine, entry);

				if(data && data < entry->action_list_length)
					entry->selection = data;
			}
			break;

		case CUSTOM_CODE_OVER_CLOCK:
			if(VALID_CPU(address))
			{
				double	over_clock = data / 65536.0;

				cpunum_set_clockscale(machine, address, over_clock);
			}
			break;

		case CUSTOM_CODE_REFRESH_RATE:
			{
				int width		= video_screen_get_width(machine->primary_screen);
				int height		= video_screen_get_height(machine->primary_screen);

				const rectangle	*visarea	= video_screen_get_visible_area(machine->primary_screen);
				double			refresh		= data / 65536.0;

				video_screen_configure(machine->primary_screen, width, height, visarea, HZ_TO_ATTOSECONDS(refresh));
			}
			break;
	}
}

/*-------------------------------------------------------------------
  open_cheat_database - open the cheat database in loading/saveing
-------------------------------------------------------------------*/

static UINT8 open_cheat_database(mame_file **the_file, char *file_name, UINT8 flag)
{
	file_error filerr;

	if(flag == DATABASE_LOAD)
	{
		/* load */
		filerr = mame_fopen(SEARCHPATH_CHEAT, file_name, OPEN_FLAG_READ, the_file);

		if(filerr != FILERR_NONE)
			return 0;
	}
	else
	{
		/* save */
		filerr = mame_fopen(SEARCHPATH_CHEAT, file_name, OPEN_FLAG_WRITE, the_file);

		if(the_file == NULL)
		{
			return 0;
		}
		else if(filerr != FILERR_NONE)
		{
			/* if the database is not found, create new database */
			filerr = mame_fopen(SEARCHPATH_CHEAT, file_name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, the_file);

			if(filerr != FILERR_NONE)
				return 0;
		}
	}

	return 1;
}

/*------------------------------------------------------
  load_cheat_option - load cheat option from database
------------------------------------------------------*/

static void load_cheat_option(char *file_name)
{
	static char		buf[2048];
	mame_file		*the_file;
	cheat_format	*format = &cheat_format_table[0];

	/* open the database */
	if(open_cheat_database(&the_file, file_name, DATABASE_LOAD) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_LOAD_DATABASE);
		return;
	}

	while(mame_fgets(buf, 2048, the_file))
	{
		char pre_command[9];

		if(sscanf(buf, format->format_string, pre_command) == format->arguments_matched)
		{
			if(strlen(pre_command) == format->type_matched)
			{
				sscanf(pre_command, "%X", &cheat_options);
				SET_MESSAGE(CHEAT_MESSAGE_RELOAD_CHEAT_OPTION);
			}
		}
	}

	mame_fclose(the_file);
}

/*--------------------------------------------------
  load_cheat_code - load cheat code from database
--------------------------------------------------*/

static void load_cheat_code(running_machine *machine, char *file_name)
{
	char		buf[2048];
	mame_file	*the_file;

	/* open the database */
	if(open_cheat_database(&the_file, file_name, DATABASE_LOAD) == 0)
		return;

	found_database = 1;

	/* get a line from database */
	while(mame_fgets(buf, 2048, the_file))
	{
		int		i;
#ifdef MESS
		int		crc =					0;
#endif
		int		type =					0;
		int		address =				0;
		int		data =					0;
		int		extend_data =			0;
		int		arguments_matched =		0;
		int		format_level =			0;
		int		cpu =					0;
		int		code =					0;
		char	name[16] =				{ 0 };
		char	pre_type[16] =			{ 0 };
		char	pre_data[16] =			{ 0 };
		char	pre_extend_data[16] =	{ 0 };
		char	description[256] =		{ 0 };
		char	comment[256] =			{ 0 };
		cheat_entry		*entry;
		cheat_action	*action;

		/* scan and check format */
		for(i = 1; i < 4; i++)
		{
			cheat_format *format = &cheat_format_table[i];

			if(TEST_FIELD(cheat_options, LoadOldFormat) == 0)
			{
				if(i != 1)
					break;
			}

			/* scan a parameter */
			switch(i)
			{
				case FORMAT_NEW:
				case FORMAT_STANDARD:
#ifdef MESS
					arguments_matched = sscanf(buf, format->format_string, name, &crc, pre_type, &address, pre_data, pre_extend_data, description, comment);
#else
					arguments_matched = sscanf(buf, format->format_string, name, pre_type, &address, pre_data, pre_extend_data, description, comment);
#endif
					break;

				case FORMAT_OLD:
#ifdef MESS
					arguments_matched = sscanf(buf, format->format_string, &crc, name, &cpu, &address, &data, &code, description, comment);
#else
					arguments_matched = sscanf(buf, format->format_string, name, &cpu, &address, &data, &code, description, comment);
#endif
			}

			/* NOTE : description and comment are not always needed */
			if(arguments_matched < format->arguments_matched)
				continue;

#ifdef MESS
			/* check name */
			if(TEST_FIELD(cheat_options, SharedCode) && strcmp(machine->gamedrv->parent, "0"))
			{
				if(strcmp(name, machine->gamedrv->parent))
					break;
			}
			else
#endif
			if(strcmp(name, machine->gamedrv->name))
				break;
#ifdef MESS
			/* check crc */
			if(MatchesCRCTable(crc) == 0)
				break;
#endif
			if(i != FORMAT_OLD)
			{
				UINT8 is_error = 0;

				/* check length */
				if(strlen(pre_type) != format->type_matched)
				{
					logerror("cheat: [load code] %s has invalid type field length (%X)\n", description, (int)strlen(pre_type));
					is_error++;
				}

				if(strlen(pre_data) != format->data_matched)
				{
					logerror("cheat: [load code] %s has invalid data field length (%X)\n", description, (int)strlen(pre_type));
					is_error++;
				}

				if(strlen(pre_extend_data) != format->data_matched)
				{
					logerror("cheat: [load code] %s has invalid extend data field length (%X)\n", description, (int)strlen(pre_extend_data));
					is_error++;
				}

				if(is_error)
				{
					SET_MESSAGE(CHEAT_MESSAGE_WRONG_CODE);
					continue;
				}
				else
				{
					if(i == FORMAT_NEW)
						sscanf(pre_type, "%2X%8X", &cpu, &type);
					else
					{
						sscanf(pre_type, "%X", &type);
						cpu = EXTRACT_FIELD(type, LocationParameter) & 0x0F;
					}

					sscanf(pre_data, "%X", &data);
					sscanf(pre_extend_data, "%X", &extend_data);
				}
			}
			else
				type = convert_older_code(code, cpu, &data, &extend_data);

			/* matched! */
			format_level = i;
			break;
		}

		/* attemp to get next line if unmatched */
		if(format_level == 0 || format_level >= 4)
			continue;

		/* logerror("cheat: processing %s\n", buf); */

		/* handle command code */
		if(cpu >= CUSTOM_CODE_ACTIVATION_KEY)
		{
			/* logerror("cheat: cheat line removed\n", buf); */
			handle_local_command_cheat(machine, cpu, type, address, data, extend_data, format_level);
			continue;
		}

		/***** ENTRY *****/
		if(EXTRACT_FIELD(type, Link) == kLink_Master)
		{
			/* 1st (master) code in an entry */
			resize_cheat_list(cheat_list_length + 1, REQUEST_DISPOSE);

			if(cheat_list_length == 0)
			{
				logerror("cheat: [load code] cheat list resize failed. bailing\n");
				goto bail;
			}

			entry = &cheat_list[cheat_list_length - 1];

			entry->name = create_string_copy(description);

			if(arguments_matched == cheat_format_table[format_level].comment_matched)
				entry->comment = create_string_copy(comment);
		}
		else
		{
			/* 2nd or later (link) code in an entry */
			if(cheat_list_length == 0)
			{
				logerror("cheat: [load code] first cheat found was link cheat. bailing\n");
				goto bail;
			}

			entry = &cheat_list[cheat_list_length - 1];
		}

		/***** ACTION *****/
		resize_cheat_action_list(&cheat_list[cheat_list_length - 1], entry->action_list_length + 1, REQUEST_DISPOSE);

		if(entry->action_list_length == 0)
		{
			logerror("cheat: [load code] action list resize failed. bailing\n");
			goto bail;
		}

		action						= &entry->action_list[entry->action_list_length - 1];
		action->flags				= 0;
		action->type				= type;
		action->region				= cpu;
		action->address				= address;
		action->original_address	= address;
		action->data				= data;
		action->original_data		= data;
		action->extend_data			= extend_data;
		action->optional_name		= create_string_copy(description);
		action->last_value			= NULL;

		if(format_level != FORMAT_NEW)
			action->flags |= kActionFlag_OldFormat;
	}

	bail:

	mame_fclose(the_file);
}

/*------------------------------------------------------------------------------------
  load_user_defined_search_region - load user region code and rebuild search region
------------------------------------------------------------------------------------*/

static void load_user_defined_search_region(running_machine *machine, char *file_name)
{
	int		count = 0;
	char	buf[2048];

	search_info 	*info	= get_current_search();
	mame_file 		*the_file;
	cheat_format 	*format	= &cheat_format_table[4];

	/* 1st, free all search regions in current search_info */
	dispose_search_reigons(info);

	/* 2nd, attempt to open the memorymap file */
	if(open_cheat_database(&the_file, file_name, DATABASE_LOAD) == 0)
	{
		info->region_list_length = 0;
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_LOAD_DATABASE);
		return;
	}

	/* 3rd, scan parameters from the file */
	while(mame_fgets(buf, 2048, the_file))
	{
#ifdef MESS
		int				crc;
#endif
		int				cpu;
		int				space;
		int				start;
		int				end;
		int				status;
		char			name[16]		= { 0 };
		char			description[64]	= { 0 };
		cpu_region_info	*cpu_info;
		search_region	*region;

#ifdef MESS
		if(sscanf(buf, format->format_string, name, &crc, &cpu, &space, &start, &end, &status, description) < format->arguments_matched)
#else
		if(sscanf(buf, format->format_string, name, &cpu, &space, &start, &end, &status, description) < format->arguments_matched)
#endif
			continue;
		else
		{
#ifdef MESS
			if(MatchesCRCTable(crc) == 0)
				continue;
#endif
			if(cpu != info->target_idx)
				continue;
		}

		cpu_info = get_cpu_info(cpu);

		/* check parameters */
		switch(space)
		{
			case kAddressSpace_Program:
			case kAddressSpace_DirectMemory:
				/* check length */
				if(start >= end)
				{
					logerror("cheat: [user region] invalid length (start : %.8X >= end : %.8X)\n", start, end);
					continue;
				}

				/* check bound */
				if(is_search_region_in_range(info->target_idx, start + end + 1) == 0)
				{
					logerror("cheat: [user region] invalid length (%.8X)\n", start - end);
					continue;
				}
				break;

			case kAddressSpace_DataSpace:
			case kAddressSpace_IOSpace:
				/* underconstruction... */
				break;

			default:
				logerror("cheat: [user region] %X is invalid space\n", space);
				continue;
		}

		/* check status */
		if(status > kRegionFlag_Enabled)
		{
			logerror("cheat: [user region] %X is invalid status\n", status);
			continue;
		}

		/* allocate memory for new search region */
		info->region_list = realloc(info->region_list, (count + 1) * sizeof(search_region));

		if(info->region_list == NULL)
		{
			mame_fclose(the_file);
			fatalerror(	"cheat: [user region] memory allocation error\n"
						"	length =		%.8X\n"
						"	region_list =	%p\n",
						count + 1, info->region_list);
		}

		region = &info->region_list[count];

		/* 4th, set region parameters from scaned data */
		region->address		= start;
		region->length		= end - start + 1;
		region->target_idx	= cpu;

		if(space == 0)
		{
			region->target_type		= kRegionType_CPU;
			region->cached_pointer	= NULL;
		}
		else
		{
			region->target_type		= kRegionType_Memory;
			region->cached_pointer	= get_memory_region_base_pointer(cpu, space, start);

			//logerror("pointer for %s : %p\n", description, region->cachedPointer);
		}

		region->write_handler	= NULL;
		region->first			= NULL;
		region->last			= NULL;
		region->status			= NULL;
		region->backup_last		= NULL;
		region->backup_status	= NULL;
		region->flags			= status;

		sprintf(region->name,	"%.*X-%.*X %s",
								cpu_info->address_chars_needed, region->address,
								cpu_info->address_chars_needed, region->address + region->length - 1,
								description);

		if(is_search_region_in_range(cpu, region->length) == 0)
		{
			region->flags &= ~kRegionFlag_Enabled;
			region->flags |= kRegionFlag_HasError;
		}
		else
			region->flags = status;

		count++;
	}

	info->region_list_length = count;

	if(info->region_list_length)
		SET_MESSAGE(CHEAT_MESSAGE_RELOAD_USER_REGION);

	mame_fclose(the_file);
}

/*-----------------------------------------------------------
  load_cheat_database - get the database name then load it
-----------------------------------------------------------*/

static void load_cheat_database(running_machine *machine, UINT8 flags)
{
	UINT8		first = 1;
	char		buf[256] = { 0 };
	char		data;
	const char	*in_traverse;
	char		*out_traverse;
	char		*main_traverse;

	cheat_file = options_get_string(mame_options(), OPTION_CHEAT_FILE);

	if(cheat_file[0] == 0)
		cheat_file = "cheat.dat";

	in_traverse		= cheat_file;
	out_traverse	= buf;
	main_traverse	= main_database_name;

	do
	{
		data = *in_traverse;

		/* check separator or end */
		if(data == ';' || data == 0)
		{
			*out_traverse++ = 0;

			if(first)
				*main_traverse++ = 0;

			if(buf[0])
			{
				/* load database based on the name we gotten */
				if(flags & LOAD_CHEAT_OPTION)
					load_cheat_option(buf);
				if(flags & LOAD_CHEAT_CODE)
					load_cheat_code(machine, buf);
				if(flags & LOAD_USER_REGION)
					load_user_defined_search_region(machine, buf);

				out_traverse	= buf;
				buf[0]			= 0;
				first			= 0;
				break;
			}
		}
		else
		{
			*out_traverse++ = data;

			if(first)
				*main_traverse++ = data;
		}

		in_traverse++;
	}
	while(data);

	if(flags & LOAD_CHEAT_CODE)
		update_all_cheat_info(machine);
}

/*---------------------------------------------------------------------------
  reload_cheat_database - reload cheat database directly on the cheat menu
---------------------------------------------------------------------------*/

static void reload_cheat_database(running_machine *machine)
{
	dispose_cheat_database(machine);
	load_cheat_database(machine, LOAD_CHEAT_CODE);

	SET_MESSAGE(found_database ? CHEAT_MESSAGE_RELOAD_CHEAT_CODE : CHEAT_MESSAGE_FAILED_TO_LOAD_DATABASE);
}

/*--------------------------------------------------
  dispose_cheat_database - free all cheat entries
--------------------------------------------------*/

static void dispose_cheat_database(running_machine *machine)
{
	int i;

	/* first, turn all cheats "OFF" */
	for(i = 0; i < cheat_list_length; i++)
		temp_deactivate_cheat(machine, &cheat_list[i]);

	/* next, free memory for all cheat entries */
	if(cheat_list)
	{
		for(i = 0; i < cheat_list_length; i++)
			dispose_cheat(&cheat_list[i]);

		free(cheat_list);

		cheat_list = NULL;
		cheat_list_length = 0;
	}
}

/*--------------------------------------------------------
  save_cheat_code - save a cheat code into the database
--------------------------------------------------------*/

static void save_cheat_code(running_machine *machine, cheat_entry *entry)
{
	int			i;
	char		buf[4096];
	mame_file	*the_file;

	/* check entry */
	if(entry == NULL || entry->action_list == NULL)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	/* ##### fix me... ##### */
	if(entry->flags & kCheatFlag_OldFormat)
	{
		SET_MESSAGE(CHEAT_MESSAGE_NO_SUPPORTED_OLD_FORMAT);
		return;
	}

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	/* write the cheat code */
	for(i = 0; i < entry->action_list_length; i++)
	{
		UINT8			address_length	= 8;
		char			*name;
		char			*buf_strings	= buf;
		cheat_action	*action			= &entry->action_list[i];

		/* get name */
		name = action->optional_name;

		/* get address length of CPU/Region */
		address_length = get_address_length(action->region);

		/* print format */
#ifdef MESS
		buf_strings +=	sprintf(buf_strings,":%s::%.8X::%.2X%.8X::%.*X::%.8X::%.8X",
								machine->gamedrv->name,							/* name */
								thisGameCRC,									/* CRC */
								action->region, action->type,					/* type */
								address_length, action->original_address,		/* address */
								action->original_data,							/* data */
								action->extend_data);							/* extend data */
#else /* MAME */
		buf_strings +=	sprintf(buf_strings, ":%s::%.2X%.8X::%.*X::%.8X::%.8X",
								machine->gamedrv->name,							/* name */
								action->region, action->type,					/* type */
								address_length, action->original_address,		/* address */
								action->original_data,							/* data */
								action->extend_data);							/* extend data */
#endif

		/* set description */
		if(name)
			buf_strings += sprintf(buf_strings, ":%s", name);
		else
			buf_strings += sprintf(buf_strings, ":(none)");

		/* set comment */
		if(i == 0 && entry->comment)
			buf_strings += sprintf(buf_strings, ":%s", entry->comment);

		buf_strings += sprintf(buf_strings, "\n");

		/* write the cheat code */
		mame_fwrite(the_file, buf, (UINT32)strlen(buf));
	}

	/* close the database */
	mame_fclose(the_file);

	/* clear dirty flag as "this entry has been saved" */
	entry->flags &= ~kCheatFlag_Dirty;

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_SAVE);
}

/*-----------------------------------------------------------------
  save_activation_key - save activation key code into the database
-----------------------------------------------------------------*/

static void save_activation_key(running_machine *machine, cheat_entry *entry, int entry_index)
{
	int			type			= 0;
	int			key_1			= 0;
	int			key_2			= 0;
	UINT8		region			= CUSTOM_CODE_ACTIVATION_KEY;
	INT8		address_length	= 8;
	char		buf[256];
	char		*buf_strings	= buf;
	mame_file	*the_file;
	astring		*name_1			= NULL;
	astring		*name_2			= NULL;

	/* check activation keys */
	if((entry->flags & kCheatFlag_HasActivationKey1) == 0 && (entry->flags & kCheatFlag_HasActivationKey2) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_NO_ACTIVATION_KEY);
		return;
	}

	/* check entry */
	if(entry == NULL || entry->action_list == NULL)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	/* get address length of CPU/Region */
	address_length = get_address_length(entry->action_list[0].region);

	/* detect activation key */
	key_1 = entry->activation_key_1;
	key_2 = entry->activation_key_2;

	/* set code */
#ifdef MESS
	buf_strings += sprintf(	buf_strings, ":%s::%.8X::%.2X%.8X::%.*X::%.8X::%.8X::Activation Key",
							machine->gamedrv->name,			/* name */
							thisGameCRC,					/* CRC */
							region, type,					/* type */
							address_length, entry_index,	/* address */
							key_1,							/* data */
							key_2);							/* extend data */
#else /* MAME */
	buf_strings += sprintf(	buf_strings, ":%s::%.2X%.8X::%.*X::%.8X::%.8X:Activation Key",
							machine->gamedrv->name,			/* name */
							region, type,					/* type */
							address_length, entry_index,	/* address */
							key_1,							/* data */
							key_2);							/* extend data */
#endif

	/* set description */
	if(entry->name && entry->name[0] != 0)
		buf_strings += sprintf(buf_strings, " for %s", entry->name);

	/* set label of key */
	if(entry->flags & kCheatFlag_Select)
	{
		if(entry->flags & kCheatFlag_HasActivationKey1)
			name_1 = input_code_name(astring_alloc(), entry->activation_key_1);

		if(entry->flags & kCheatFlag_HasActivationKey2)
			name_2 = input_code_name(astring_alloc(), entry->activation_key_2);

		buf_strings += sprintf(buf_strings, " (prev = %s / next = %s)\n", astring_c(name_1), astring_c(name_2));
	}
	else
	{
		name_1 = input_code_name(astring_alloc(), entry->activation_key_1);

		buf_strings += sprintf(buf_strings, " (%s)\n", astring_c(name_1));
	}


	/* write the activation key code */
	mame_fwrite(the_file, buf, (UINT32)strlen(buf));

	if(name_1)	astring_free(name_1);
	if(name_2)	astring_free(name_2);

	/* close the database */
	mame_fclose(the_file);

	SET_MESSAGE(CHEAT_MESSAGE_ACTIVATION_KEY_SAVED);
}

/*-----------------------------------------------------------
  save_pre_enable - save a pre-enable code into the database
-----------------------------------------------------------*/

static void save_pre_enable(running_machine *machine, cheat_entry *entry, int entry_index)
{
	int				type			= 0;
	UINT8			region			= CUSTOM_CODE_PRE_ENABLE;
	INT8			address_length	= 8;
	char			buf[256];
	char			*buf_strings	= buf;
	mame_file		*the_file;

	/* if invalid cheat code, no action */
	if(entry == NULL || entry->action_list == NULL)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	/* get address length of CPU/Region */
	address_length = get_address_length(entry->action_list[0].region);

#ifdef MESS
	buf_strings += sprintf(	buf_strings, ":%s::%.8X::%.2X%.8X::%.*X::%.8X:00000000",
							machine->gamedrv->name,			/* name */
							thisGameCRC,					/* CRC */
							region, type,					/* type */
							address_length, entry_index,	/* address */
							entry->selection);				/* data */
#else /* MAME */
	buf_strings += sprintf(	buf_strings, ":%s::%.2X%.8X::%.*X::%.8X::00000000",
							machine->gamedrv->name,			/* name */
							region ,type,					/* type */
							address_length, entry_index,	/* address */
							entry->selection);				/* data */
#endif

	/* set description */
	buf_strings += sprintf(buf_strings, ":Pre-enable");

	if(entry->name)
		buf_strings += sprintf(buf_strings, " for %s", entry->name);

	/* set label name */
	if(entry->selection)
		buf_strings += sprintf(buf_strings, " Label - %s", entry->action_list[entry->label_index[entry->selection]].optional_name);

	buf_strings += sprintf(buf_strings, "\n");

	/* write the pre-enable code */
	mame_fwrite(the_file, buf, (UINT32)strlen(buf));

	/* close the database */
	mame_fclose(the_file);

	SET_MESSAGE(CHEAT_MESSAGE_PRE_ENABLE_SAVED);
}

/*--------------------------------------------------------------
  save_cheat_options - save cheat option code into the database
--------------------------------------------------------------*/

static void save_cheat_options(void)
{
	char		buf[32];
	mame_file	*the_file;

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	sprintf(buf, ":_command:%.8X\n", cheat_options);

	/* write the cheat option */
	mame_fwrite(the_file, buf, (UINT32)strlen(buf));

	/* close the database */
	mame_fclose(the_file);

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_SAVE);
}

/*------------------
  save_description
------------------*/

static void save_description(running_machine *machine)
{
	char		buf[256];
	mame_file	*the_file;

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	sprintf(buf, "; [ %s ]\n", machine->gamedrv->description);

	/* write the description of game/machine */
	mame_fwrite(the_file, buf, (UINT32)strlen(buf));

	/* close the database */
	mame_fclose(the_file);

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_SAVE);
}

/*----------------
  save_raw_code
----------------*/

static void save_raw_code(running_machine *machine)
{
	int			address_length = 8;
	int			address = 0;
	char		buf[256];
	mame_file	*the_file;

	/* open the database */
	if(open_cheat_database(&the_file, main_database_name, DATABASE_SAVE) == 0)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_SAVE);
		return;
	}

	mame_fseek(the_file, 0, SEEK_END);

	address_length = get_address_length(0);

#ifdef MESS
	sprintf(buf, ":%s::%.8X::0000000000::%.*X::00000000::FFFFFFFF:Raw Code\n", machine->gamedrv->name, thisGameCRC, address_length, address);
#else /* MAME */
	sprintf(buf, ":%s::0000000000::%.*X::00000000::FFFFFFFF:Raw Code\n", machine->gamedrv->name, address_length, address);
#endif

	/* write the raw code */
	mame_fwrite(the_file, buf, (UINT32)strlen(buf));

	/* close the database */
	mame_fclose(the_file);

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_SAVE);
}

/*------------------------------------------------------------------------------
  do_auto_save_cheats - save normal code automatically when exit cheat system
------------------------------------------------------------------------------*/

static void do_auto_save_cheats(running_machine *machine)
{
	int i;

	save_description(machine);

	for(i = 0; i < cheat_list_length; i++)
	{
		cheat_entry *entry = &cheat_list[i];

		/* if the entry is not edited/added newly or has not been already saved, save it */
		if(entry->flags & kCheatFlag_Dirty)
			save_cheat_code(machine, entry);
	}
}

/*----------------------------------------------------------------------
  add_cheat_from_result - add a code from result viewer to cheat list
----------------------------------------------------------------------*/

static void add_cheat_from_result(running_machine *machine, search_info *search, search_region *region, UINT32 address)
{
	if(region->target_type == kRegionType_CPU || region->target_type == kRegionType_Memory)
	{
		int				temp_string_length;
		UINT32			data		= read_search_operand(kSearchOperand_First, search, region, address);
		char			temp_string[1024];
		cheat_entry		*entry		= get_new_cheat();
		cheat_action	*action		= &entry->action_list[0];

		temp_string_length = sprintf(temp_string, "%.8X (%d) = %.*X", address, region->target_idx, kSearchByteDigitsTable[search->bytes], data);

		entry->name = realloc(entry->name, temp_string_length + 1);
		if(entry->name == NULL)
			fatalerror(	"cheat: [cheat entry] memory allocation error"
						"	temp_length	=		%.8X"
						"	entry->name =		%p",
						temp_string_length, entry->name);

		memcpy(entry->name, temp_string, temp_string_length + 1);
		action->region			= region->target_idx;
		action->address			= address;
		action->data			= data;
		action->original_data	= data;
		action->extend_data		= ~0;
		action->last_value		= NULL;
		SET_FIELD(action->type, AddressSize, kSearchByteIncrementTable[search->bytes] - 1);

		update_cheat_info(machine, entry, 0);
	}
}

/*------------------------------------------------------------------------------------------------
  add_cheat_from_first_result - add a code from search box to cheat list if found result is one
------------------------------------------------------------------------------------------------*/

static void add_cheat_from_first_result(running_machine *machine, search_info *search)
{
	int i;

	for(i = 0; i < search->region_list_length; i++)
	{
		search_region *region = &search->region_list[i];

		if(region->num_results)
		{
			UINT32 traverse;

			for(traverse = 0; traverse < region->length; traverse++)
			{
				UINT32 address = region->address + traverse;

				if(is_region_offset_valid(search, region, traverse))
				{
					add_cheat_from_result(machine, search, region, address);
					return;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
  add_watch_from_result - add a watchpoint from result viewer to cheat list
----------------------------------------------------------------------------*/

static void add_watch_from_result(search_info *search, search_region *region, UINT32 address)
{
	if(region->target_type == kRegionType_CPU || region->target_type == kRegionType_Memory)
	{
		watch_info *info = get_unused_watch();

		info->address			= address;
		info->cpu				= region->target_idx;
		info->num_elements		= 1;
		info->element_bytes		= kWatchSizeConversionTable[search->bytes];
		info->label_type		= kWatchLabel_None;
		info->display_type		= kWatchDisplayType_Hex;
		info->skip				= 0;
		info->elements_per_line	= 0;
		info->add_value			= 0;
		info->linked_cheat		= NULL;
		info->label[0]			= 0;

		SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_ADD);
	}
	else
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
}

/*---------------------
  search_sign_extend
---------------------*/

static UINT32 search_sign_extend(search_info *search, UINT32 value)
{
	if(search->sign)
		if(value & kSearchByteSignBitTable[search->bytes])
			value |= ~kSearchByteUnsignedMaskTable[search->bytes];

	return value;
}

/*---------------------------------------------------------------------
  read_search_operand - read a data from search region and return it
---------------------------------------------------------------------*/

static UINT32 read_search_operand(UINT8 type, search_info *search, search_region *region, UINT32 address)
{
	UINT32 value = 0;

	switch(type)
	{
		case kSearchOperand_Current:
			value = read_region_data(region, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap);
			break;

		case kSearchOperand_Previous:
			value = do_memory_read(region->last, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_First:
			value = do_memory_read(region->first, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_Value:
			value = search->value;
	}

	value = search_sign_extend(search, value);

	return value;
}

/*---------------------------------------------------------------------------------------------
  read_search_operand_bit - read a bit data from search region and return it when bit search
---------------------------------------------------------------------------------------------*/

static UINT32 read_search_operand_bit(UINT8 type, search_info *search, search_region *region, UINT32 address)
{
	UINT32 value = 0;

	switch(type)
	{
		case kSearchOperand_Current:
			value = read_region_data(region, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap);
			break;

		case kSearchOperand_Previous:
			value = do_memory_read(region->last, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_First:
			value = do_memory_read(region->first, address - region->address, kSearchByteIncrementTable[search->bytes], search->swap, NULL);
			break;

		case kSearchOperand_Value:
			if(search->value)
				value = 0xFFFFFFFF;
			else
				value = 0x00000000;
	}

	value = search_sign_extend(search, value);

	return value;
}

/*------------------------------------------------------------
  do_search_comparison - compare data and return it matched
------------------------------------------------------------*/

static UINT8 do_search_comparison(search_info *search, UINT32 lhs, UINT32 rhs)
{
	INT32 svalue;

	if(search->sign)
	{
		/* signed */
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
		/* unsigned */
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

/*--------------------------------------------------------------------
  do_search_comparison_bit - compare bit data and return it matched
--------------------------------------------------------------------*/

static UINT32 do_search_comparison_bit(search_info *search, UINT32 lhs, UINT32 rhs)
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

/*----------------------------------------------
  is_region_offset_valid - ????? unused ?????
----------------------------------------------*/

#if 0
static UINT8 is_region_offset_valid(search_info *search, search_region *region, UINT32 offset)
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
#endif

/*-----------------------------------------------------
  is_region_offset_valid_bit - check selected offset
-----------------------------------------------------*/

static UINT8 is_region_offset_valid_bit(search_info *search, search_region *region, UINT32 offset)
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

/*------------------------------------------------------------------
  invalidate_region_offset - remove unmatched offset after search
------------------------------------------------------------------*/

static void invalidate_region_offset(search_info *search, search_region *region, UINT32 offset)
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

/*--------------------------------------------------------------------------
  invalidate_region_offset_bit - remove unmatched offset after bit search
--------------------------------------------------------------------------*/

static void invalidate_region_offset_bit(search_info *search, search_region *region, UINT32 offset, UINT32 invalidate)
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

/*----------------------------------------------------------------
  invalidate_entire_regions - invalidate selected search region
----------------------------------------------------------------*/

static void invalidate_entire_region(search_info *search, search_region *region)
{
	memset(region->status, 0, region->length);

	search->num_results -=	region->num_results;
	region->num_results =	0;
}

/*---------------------------------------------------------------
  init_new_search - initialize search region to start a search
---------------------------------------------------------------*/

static void init_new_search(search_info *search)
{
	int i;

	search->num_results = 0;

	for(i = 0; i < search->region_list_length; i++)
	{
		search_region *region = &search->region_list[i];

		if(region->flags & kRegionFlag_Enabled)
		{
			region->num_results = 0;

			memset(region->status, 0xFF, region->length);

			fill_buffer_from_region(region, region->first);

			memcpy(region->last, region->first, region->length);
		}
	}
}

/*----------------------------------------------------------------------------
  update_search - update a data in search region after initialize or search
----------------------------------------------------------------------------*/

static void update_search(search_info *search)
{
	int i;

	for(i = 0; i < search->region_list_length; i++)
	{
		search_region *region = &search->region_list[i];

		if(region->flags & kRegionFlag_Enabled)
			fill_buffer_from_region(region, region->last);
	}
}

/*---------------------------------------
  do_search - cheat engine search core
---------------------------------------*/

static void do_search(search_info *search)
{
	int i, j;

	search->num_results = 0;

	if(search->bytes == kSearchSize_1Bit)
	{
		/* bit search */
		for(i = 0; i < search->region_list_length; i++)
		{
			search_region	*region		= &search->region_list[i];
			UINT32			lastAddress	= region->length - kSearchByteIncrementTable[search->bytes] + 1;
			UINT32			increment	= kSearchByteStep[search->bytes];

			region->num_results = 0;

			if(region->length < kSearchByteIncrementTable[search->bytes] || (region->flags & kRegionFlag_Enabled) == 0)
				continue;

			for(j = 0; j < lastAddress; j += increment)
			{
				UINT32	address;
				UINT32	lhs, rhs;

				address = region->address + j;

				if(is_region_offset_valid_bit(search, region, j))
				{
					UINT32	validBits;

					lhs = read_search_operand_bit(search->lhs, search, region, address);
					rhs = read_search_operand_bit(search->rhs, search, region, address);

					/* do bit search */
					validBits = do_search_comparison_bit(search, lhs, rhs);

					invalidate_region_offset_bit(search, region, j, ~validBits);

					if(is_region_offset_valid_bit(search, region, j))
					{
						search->num_results++;
						region->num_results++;
					}
				}
			}
		}
	}
	else
	{
		/* normal search */
		for(i = 0; i < search->region_list_length; i++)
		{
			search_region	*region		= &search->region_list[i];
			UINT32			lastAddress	= region->length - kSearchByteIncrementTable[search->bytes] + 1;
			UINT32			increment	= kSearchByteStep[search->bytes];

			region->num_results = 0;

			if(region->length < kSearchByteIncrementTable[search->bytes] || (region->flags & kRegionFlag_Enabled) == 0)
				continue;

			for(j = 0; j < lastAddress; j += increment)
			{
				UINT32	address;
				UINT32	lhs, rhs;

				address = region->address + j;

				if(is_region_offset_valid(search, region, j))
				{
					lhs = read_search_operand(search->lhs, search, region, address);
					rhs = read_search_operand(search->rhs, search, region, address);

					/* do value search */
					if(do_search_comparison(search, lhs, rhs) == 0)
					{
						/* unmatched */
						invalidate_region_offset(search, region, j);
					}
					else
					{
						/* matched */
						search->num_results++;
						region->num_results++;
					}
				}
			}
		}
	}
}

/*----------------------------------------------------------
  look_up_handler_memory - search specified write handler
----------------------------------------------------------*/

static UINT8 **look_up_handler_memory(UINT8 cpu, UINT32 address, UINT32 *out_relative_address)
{
	const address_map			*map = memory_get_address_map(cpu, ADDRESS_SPACE_PROGRAM);
	const address_map_entry		*entry;

	for(entry = map->entrylist; entry != NULL; entry = entry->next)
	{
		if(entry->write.generic != NULL && address >= entry->addrstart && address <= entry->addrend)
		{
			if(out_relative_address)
			{
				*out_relative_address = address - entry->addrstart;
				return (UINT8 **)entry->baseptr;
			}
		}
	}

	return NULL;
}

/*----------------------------------------------------------------------------------------------------------------
  get_memory_region_base_pointer - return base pointer to the base of RAM associated with given CPU and address
----------------------------------------------------------------------------------------------------------------*/

static UINT8 **get_memory_region_base_pointer(UINT8 cpu, UINT8 space, UINT32 address)
{
	UINT8 *buf = NULL;

	switch(space)
	{
		case kAddressSpace_DataSpace:
			buf = memory_get_read_ptr(cpu, ADDRESS_SPACE_DATA, address);
			break;

		case kAddressSpace_IOSpace:
			buf = memory_get_read_ptr(cpu, ADDRESS_SPACE_IO, address);
			break;

		case kAddressSpace_OpcodeRAM:
			buf = memory_get_op_ptr(cpu, address, 0);
			break;

		case kAddressSpace_DirectMemory:
			buf = memory_get_read_ptr(cpu, ADDRESS_SPACE_PROGRAM, address);
			break;

		default:
			logerror("invalid address space (%x)\n", space);
	}

	if(buf)
		buf -= address;

	//logerror("pointer (return) : %p\n", buf);

	return (UINT8 **)buf;
}

/*----------------------------------------------------------------------------------------------------------------
  do_cpu_read - read a data from standard CPU region
                NOTE : if a driver has cpu_spinutil(), reading a data via cpunum_read_byte may conflict with it
----------------------------------------------------------------------------------------------------------------*/

static UINT32 do_cpu_read(UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap)
{
	cpu_region_info *info = get_cpu_info(cpu);

	address = DoShift(address, info->address_shift);

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

/*----------------------------------------------------
  do_memory_read - read a data from memory directly
----------------------------------------------------*/

static UINT32 do_memory_read(UINT8 *buf, UINT32 address, UINT8 bytes, UINT8 swap, cpu_region_info *info)
{
	int i;
	int data = 0;

	if(info)
		address = DoShift(address, info->address_shift);

	if(info == NULL)
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
				info = &raw_cpu_info;
				goto generic;
		}

		return data;
	}

	generic:

	for(i = 0; i < bytes; i++)
		data |= swap ?	buf[SwapAddress(address + i, bytes, info)] << (i * 8) :
						buf[SwapAddress(address + i, bytes, info)] << ((bytes - i - 1) * 8);

	return data;
}

/*-------------------------------------------------------
  do_cpu_write - write a data into standard CPU region
-------------------------------------------------------*/

static void do_cpu_write(UINT32 data, UINT8 cpu, UINT32 address, UINT8 bytes, UINT8 swap)
{
	cpu_region_info *info = get_cpu_info(cpu);

	address = DoShift(address, info->address_shift);

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
			logerror("do_cpu_write: bad size (%d)\n", bytes);
			break;
	}
}

/*------------------------------------------------------
  do_memory_write - write a data into memory directly
------------------------------------------------------*/

static void do_memory_write(UINT32 data, UINT8 *buf, UINT32 address, UINT8 bytes, UINT8 swap, cpu_region_info *info)
{
	UINT32 i;

	if(info)
		address = DoShift(address, info->address_shift);

	if(info == NULL)
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
				info = &raw_cpu_info;
				goto generic;
		}

		return;
	}

	generic:

	for(i = 0; i < bytes; i++)
	{
		if(swap)	buf[SwapAddress(address + i, bytes, info)] = data >> (i * 8);
		else		buf[SwapAddress(address + i, bytes, info)] = data >> ((bytes - i - 1) * 8);
	}
}

/*------------
  read_data
------------*/

static UINT32 read_data(running_machine *machine, cheat_action *action)
{
	UINT8	read_from	= EXTRACT_FIELD(action->type, AddressRead);
	UINT8	bytes		= EXTRACT_FIELD(action->type, AddressSize);

	if(read_from != kReadFrom_Variable)
	{
		/* read a data from memory */
		int		address			= ~0;
		UINT8	region			= action->region < REGION_INVALID ? EXTRACT_FIELD(action->region, CPUIndex) : action->region;

		/* set address */
		if(read_from == kReadFrom_Address)		address = action->address;
		else if(read_from == kReadFrom_Index)	address = cheat_variable[action->address];

		/* NOTE : data size "+1" */
		bytes++;

		/* 1st, adjust address */
		if(action->flags & kActionFlag_IndexAddress)
		{
			int					index_address	= 0;
			cpu_region_info		*info			= get_cpu_info(region);

			index_address = do_cpu_read(region, address, EXTRACT_FIELD(action->type, CodeParameterLower) + 1,
										cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness));

			if(info)
				index_address = DoShift(index_address, info->address_shift);

			if(index_address)
				address = index_address + action->extend_data;
		}
		else if(action->flags & kActionFlag_PDWWrite)
		{
			if(action->flags & kActionFlag_IsFirst)
			{
				address += bytes;
				bytes = EXTRACT_FIELD(action->type, CodeParameterLower) + 1;
				action->flags &= ~kActionFlag_IsFirst;
			}
			else
				action->flags |= kActionFlag_IsFirst;
		}

		/* 2nd, read a data from specified region */
		if(action->region < REGION_INVALID)
		{
			/* CPU region */
			switch(EXTRACT_FIELD(action->region, AddressSpace))
			{
				case kAddressSpace_Program:
					return do_cpu_read(region, address, bytes, cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness));

				case kAddressSpace_DataSpace:
				case kAddressSpace_IOSpace:
				case kAddressSpace_OpcodeRAM:
				case kAddressSpace_DirectMemory:
					{
						UINT8 *buf;

						action->cached_pointer = get_memory_region_base_pointer(region, EXTRACT_FIELD(action->type, AddressSpace), address);
						buf = (UINT8 *)action->cached_pointer;

						if(buf)
							return do_memory_read(	buf, address, bytes,
													cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
													get_cpu_info(region));
					}
					break;

				case kAddressSpace_MappedMemory:
					{
						int		relative_address;
						UINT8	**buf;

						action->cached_pointer = look_up_handler_memory(region, address, &action->cached_offset);
						buf = action->cached_pointer;
						relative_address = action->cached_offset;

						if(buf && *buf)
							return do_memory_read(	*buf, relative_address, bytes,
													cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
													get_cpu_info(region));
					}
					break;

				case kAddressSpace_EEPROM:
					{
						int		length;
						UINT8	*buf;

						buf = eeprom_get_data_pointer(&length);

						if(is_address_in_range(action, length))
							return do_memory_read(buf, address, bytes, 0, &raw_cpu_info);
					}
					break;
			}
		}
		else
		{
			/* non-CPU region */
			UINT8 * buf = memory_region(machine, region);

			if(buf)
			{
				if(is_address_in_range(action, memory_region_length(machine, region)))
					return	do_memory_read(	buf, address, bytes,
											region_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
											get_region_info(region));
			}
		}
	}
	else
	{
		/* read a data from variable */
		return (cheat_variable[action->address] & kCheatSizeMaskTable[bytes]);
	}

	return 0;
}

/*--------------------------------------
  write_data - write a data to memory
--------------------------------------*/

static void write_data(running_machine *machine, cheat_action *action, UINT32 data)
{
	UINT8	read_from	= EXTRACT_FIELD(action->type, AddressRead);
	UINT8	bytes		= EXTRACT_FIELD(action->type, AddressSize);

	if(read_from != kReadFrom_Variable)
	{
		/* write a data to memory */
		int		address			= ~0;
		UINT8	region			= action->region < REGION_INVALID ? EXTRACT_FIELD(action->region, CPUIndex) : action->region;

		/* set address */
		if(read_from == kReadFrom_Address)		address = action->address;
		else if(read_from == kReadFrom_Index)	address = cheat_variable[action->address];

		/* NOTE : data size "+1" */
		bytes++;

		/* 1st, adjust address */
		if(action->flags & kActionFlag_IndexAddress)
		{
			int					index_address	= 0;
			cpu_region_info		*info			= get_cpu_info(region);

			index_address = do_cpu_read(region, address, EXTRACT_FIELD(action->type, CodeParameterLower) + 1,
										cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness));

			if(info)
				index_address = DoShift(index_address, info->address_shift);

			if(index_address)
				address = index_address + action->extend_data;
		}
		else if(action->flags & kActionFlag_PDWWrite)
		{
			if(action->flags & kActionFlag_IsFirst)
			{
				address += bytes;
				bytes = EXTRACT_FIELD(action->type, CodeParameterLower) + 1;
				action->flags &= ~kActionFlag_IsFirst;
			}
			else
				action->flags |= kActionFlag_IsFirst;
		}

		/* 2nd, write a data to specified region */
		if(action->region < REGION_INVALID)
		{
			/* CPU region */
			switch(EXTRACT_FIELD(action->region, AddressSpace))
			{
				case kAddressSpace_Program:
					do_cpu_write(data, region, address, bytes, cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness));
					break;

				case kAddressSpace_DataSpace:
				case kAddressSpace_IOSpace:
				case kAddressSpace_OpcodeRAM:
				case kAddressSpace_DirectMemory:
					{
						UINT8 *buf;

						action->cached_pointer = get_memory_region_base_pointer(region, EXTRACT_FIELD(action->type, AddressSpace), address);
						buf = (UINT8 *)action->cached_pointer;

						if(buf)
							do_memory_write(data, buf, address, bytes,
											cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
											get_cpu_info(region));
					}
					break;

				case kAddressSpace_MappedMemory:
					{
						int		relative_address;
						UINT8	**buf;

						action->cached_pointer = look_up_handler_memory(region, address, &action->cached_offset);
						buf = action->cached_pointer;
						relative_address = action->cached_offset;

						if(buf && *buf)
							do_memory_write(data, *buf, relative_address, bytes,
											cpu_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
											get_cpu_info(region));
					}
					break;

				case kAddressSpace_EEPROM:
					{
						int		length;
						UINT8	*buf;

						buf = eeprom_get_data_pointer(&length);

						if(is_address_in_range(action, length))
							do_memory_write(data, buf, address, bytes, 0, &raw_cpu_info);
					}
					break;
			}
		}
		else
		{
			/* non-CPU region */
			UINT8 * buf = memory_region(machine, region);

			if(buf)
			{
				if(is_address_in_range(action, memory_region_length(machine, region)))
					do_memory_write(data, buf, address, bytes,
									region_needs_swap(region) ^ EXTRACT_FIELD(action->type, Endianness),
									get_region_info(action->region));
			}
		}
	}
	else
	{
		/* write a data to variable */
		cheat_variable[action->address] = data & kCheatSizeMaskTable[bytes];
	}
}

/*--------------------------------------------------------------------------------------
  watch_cheat_entry - set watchpoint when watch value key is pressed in several menus
--------------------------------------------------------------------------------------*/

static void watch_cheat_entry(cheat_entry *entry, UINT8 associate)
{
	int			i, j;
	cheat_entry	*associate_entry = NULL;

	/* NOTE : calling with associate = 1 doesn't exist right now */
	if(associate)
		associate_entry = entry;

	if(entry == NULL)
	{
		SET_MESSAGE(CHEAT_MESSAGE_FAILED_TO_ADD);
		return;
	}

	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		if((action->flags & kActionFlag_NoAction) == 0 && EXTRACT_FIELD(action->type, AddressRead) == kReadFrom_Address)
		{
			/* NOTE : unsupported the watchpoint for indirect index right now */
			if(action->flags & kActionFlag_IndexAddress)
				continue;

			if(i == 0)
			{
				/* first cheat action */
				add_action_watch(&entry->action_list[i], associate_entry);
			}
			else
			{
				/* 2nd or later cheat action */
				UINT8 different_address = 1;

				for(j = 0; j < i; j++)
				{
					/* if we find the same address cheat action, skip it */
					if(entry->action_list[j].address == entry->action_list[i].address)
						different_address = 0;
				}

				if(different_address)
					add_action_watch(&entry->action_list[i], associate_entry);
			}
		}
	}

	SET_MESSAGE(CHEAT_MESSAGE_SUCCEEDED_TO_ADD);
}

/*--------------------------------------------------------------------------
  add_action_watch - set watchpoint parameters into watch info
                     called from watch_cheat_entry() when watch value key
                     or activate_cheat() via watchpoint code
--------------------------------------------------------------------------*/

static void add_action_watch(cheat_action *action, cheat_entry *entry)
{
	switch(EXTRACT_FIELD(action->type, CodeType))
	{
		case kCodeType_Write:
		case kCodeType_IWrite:
		case kCodeType_RWrite:
		case kCodeType_CWrite:
		case kCodeType_CBit:
		case kCodeType_PDWWrite:
		case kCodeType_Move:
		case kCodeType_Branch:
		case kCodeType_Loop:
		case kCodeType_Popup:
			{
				watch_info *info = get_unused_watch();

				info->address		= action->address;
				info->cpu			= action->region;
				info->display_type	= kWatchDisplayType_Hex;
				info->element_bytes	= kSearchSize_8Bit;
				info->label[0]		= 0;
				info->label_type	= kWatchLabel_None;
				info->num_elements	= EXTRACT_FIELD(action->type, AddressSize) + 1;
				info->linked_cheat	= entry;
				info->skip			= 0;

				if(action->flags & kActionFlag_PDWWrite)
				{
					info->num_elements += (EXTRACT_FIELD(action->type, CodeParameterLower) + 1);
				}
				else if(action->flags & kActionFlag_Repeat)
				{
					info->num_elements	= EXTRACT_FIELD(action->extend_data, LSB16);
					info->skip			= EXTRACT_FIELD(action->extend_data, MSB16) - 1;
				}
			}
			break;

		case kCodeType_Watch:
			{
				watch_info *info = get_unused_watch();

				info->address			= action->address;
				info->cpu				= action->region;
				info->display_type		= EXTRACT_FIELD(action->type, WatchDisplayFormat);
				info->element_bytes		= kByteConversionTable[EXTRACT_FIELD(action->type, AddressSize)];
				info->label[0]			= 0;
				info->linked_cheat		= entry;
				info->num_elements		= EXTRACT_FIELD(action->data, WatchNumElements) + 1;
				info->skip				= EXTRACT_FIELD(action->data, WatchSkip);
				info->elements_per_line	= EXTRACT_FIELD(action->data, WatchElementsPerLine);
				info->add_value			= EXTRACT_FIELD(action->data, WatchAddValue);

				if(info->add_value & 0x80)
					info->add_value |= ~0xFF;

				if(action->extend_data != ~0)
				{
					/* ### fix me... ### */
					info->x = (float)(EXTRACT_FIELD(action->extend_data, LSB16)) / 100;
					info->y = (float)(EXTRACT_FIELD(action->extend_data, MSB16)) / 100;
				}

				if(TEST_FIELD(action->type, WatchShowLabel) && entry->comment && strlen(entry->comment) < 256)
				{
					info->label_type = kWatchLabel_String;
					strcpy(info->label, entry->comment);
				}
				else
					info->label_type = kWatchLabel_None;
			}
			break;
	}
}

/*-------------------------------------------------------------------------------
  remove_associated_watches - delete watchpoints when watchpoint CODE is "OFF"
                              called from deactivate_cheat().
-------------------------------------------------------------------------------*/

static void remove_associated_watches(cheat_entry *entry)
{
	int i;

	for(i = watch_list_length - 1; i >= 0; i--)
	{
		watch_info *info = &watch_list[i];

		if(info->linked_cheat == entry)
			delete_watch_at(i);
	}
}

/*---------------------------------------------------
  reset_action - back up data and set action flags
---------------------------------------------------*/

static void reset_action(running_machine *machine, cheat_action *action)
{
	/* back up a value */
	if(action->flags & kActionFlag_OldFormat)
	{
		UINT32 type = action->type;

		action->last_value = malloc(sizeof(action->last_value));
		if(action->last_value == NULL) goto reset_action_error;
		action->type = convert_to_new_code(action);
		action->last_value[0] = read_data(machine, action);
		action->type = type;
	}
	else
	{
		if(action->flags & kActionFlag_MemoryWrite)
		{
			if((action->flags & kActionFlag_LastValueGood) == 0)
			{
				switch(EXTRACT_FIELD(action->type, CodeType))
				{
					default:
						/* Write, IWrite, CWrite, CBit */
						action->last_value = malloc(sizeof(action->last_value));
						if(action->last_value == NULL) goto reset_action_error;
						action->last_value[0] = read_data(machine, action);
						break;

					case kCodeType_PDWWrite:
						action->last_value = malloc(sizeof(action->last_value) * 2);
						if(action->last_value == NULL) goto reset_action_error;
						action->flags &= ~kActionFlag_IsFirst;
						action->last_value[0] = read_data(machine, action);
						action->last_value[1] = read_data(machine, action);
						break;

					case kCodeType_RWrite:
						{
							int i;

							for(i = 0; i < EXTRACT_FIELD(action->extend_data, LSB16); i++)
							{
								action->last_value = realloc(action->last_value, sizeof(action->last_value) * EXTRACT_FIELD(action->extend_data, LSB16));
								if(action->last_value == NULL) goto reset_action_error;
								action->last_value[i] = read_data(machine, action);
								action->address +=	EXTRACT_FIELD(action->extend_data, MSB16) ?
													EXTRACT_FIELD(action->extend_data, MSB16) :
													kSearchByteIncrementTable[EXTRACT_FIELD(action->type, AddressSize)];
							}

							action->address = action->original_address;
						}
						break;
				}
			}
		}
		else
			action->last_value = NULL;
	}

	action->frame_timer = 0;
	action->flags &= ~kActionFlag_StateMask;
	action->flags |= kActionFlag_LastValueGood;

	if(action->flags & kActionFlag_CheckCondition)
	{
		if(EXTRACT_FIELD(action->type, CodeParameter) == kCondition_PreviousValue)
			action->extend_data = read_data(machine, action);
	}

	return;

	reset_action_error:

	fatalerror("cheat:[last value] memory allocation error\n"
				"	----- %s -----\n"
				"	type =			%.8X\n"
				"	last_value =	%p\n",
				action->optional_name, action->type, action->last_value);
}

/*--------------------------------------------------------------------------------------
  activate_cheat - reset action entry and set activate entry flag when turn CODE "ON"
--------------------------------------------------------------------------------------*/

static void activate_cheat(running_machine *machine, cheat_entry *entry)
{
	int i;

	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		reset_action(machine, action);

		/* if watchpoint code, add watchpoint */
		if(EXTRACT_FIELD(action->type, CodeType) == kCodeType_Watch)
			add_action_watch(action, entry);
	}

	/* set activate flag */
	entry->flags |= kCheatFlag_Active;
}

/*--------------------------------------------------------
  restore_last_value - restore previous value if needed
--------------------------------------------------------*/

static void restore_last_value(running_machine *machine, cheat_action *action)
{
	if(action->flags & kActionFlag_MemoryWrite)
	{
		if(TEST_FIELD(action->type, RestoreValue))
		{
			switch(EXTRACT_FIELD(action->type, CodeType))
			{
				default:
					/* Write, IWrite, CWrite, CBit */
					write_data(machine, action, (UINT32)action->last_value[0]);
					break;

				case kCodeType_PDWWrite:
					action->flags &= ~kActionFlag_IsFirst;
					write_data(machine, action, (UINT32)action->last_value[0]);
					write_data(machine, action, (UINT32)action->last_value[1]);
					break;

				case kCodeType_RWrite:
					{
						int j;

						for(j = 0; j < EXTRACT_FIELD(action->extend_data, LSB16); j++)
						{
							write_data(machine, action, (UINT32)action->last_value[j]);
							action->address +=	EXTRACT_FIELD(action->extend_data, MSB16) ?
												EXTRACT_FIELD(action->extend_data, MSB16) :
												kSearchByteIncrementTable[EXTRACT_FIELD(action->type, AddressSize)];
						}
						action->address = action->original_address;
					}
					break;
			}
		}
	}
}

/*-------------------------------------------------------------------------
  deactivate_cheat - deactivate selecte cheat entry when turn CODE "OFF"
-------------------------------------------------------------------------*/

static void deactivate_cheat(running_machine *machine, cheat_entry *entry)
{
	int i;

	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		action->frame_timer = 0;
		action->flags &= ~kActionFlag_OperationDone;

		/* restore previous value if needed */
		if(action->last_value)
		{
			restore_last_value(machine, action);
			action->flags &= ~kActionFlag_LastValueGood;
			free(action->last_value);
		}

		action->last_value = NULL;
	}

	/* remove watchpoint */
	remove_associated_watches(entry);

	/* clear active flag */
	entry->flags &= ~kCheatFlag_StateMask;
}

/*-----------------------------------------------------------------
  temp_deactivate_cheat - deactivate cheat when turn CHEAT "OFF"
-----------------------------------------------------------------*/

static void temp_deactivate_cheat(running_machine *machine, cheat_entry *entry)
{
	if(entry->flags & kCheatFlag_Active)
	{
		int i;

		for(i = 0; i < entry->action_list_length; i++)
		{
			cheat_action *action = &entry->action_list[i];

			action->frame_timer = 0;
			action->flags &= ~kActionFlag_OperationDone;

			/* restore previous value if needed */
			if(action->last_value)
				restore_last_value(machine, action);
		}
	}
}

/*------------------------------------------------------------
  cheat_periodicOperation - management for cheat operations
------------------------------------------------------------*/

static void cheat_periodicOperation(running_machine *machine, cheat_action *action)
{
	int data = TEST_FIELD(action->type, DataRead) ? cheat_variable[action->data] : action->data;

	if(action->flags & kActionFlag_PDWWrite)
	{
		action->flags &= ~kActionFlag_IsFirst;
		write_data(machine, action, data);
		write_data(machine, action, action->extend_data);
	}
	else if(action->flags & kActionFlag_Repeat)
	{
		int i;

		for(i = 0; i < EXTRACT_FIELD(action->extend_data, LSB16); i++)
		{
			write_data(machine, action, data);
			action->address +=	EXTRACT_FIELD(action->extend_data, MSB16) ?
								EXTRACT_FIELD(action->extend_data, MSB16) :
								kSearchByteIncrementTable[EXTRACT_FIELD(action->type, AddressSize)];
		}
		action->address = action->original_address;
	}
	else
	{
		switch(EXTRACT_FIELD(action->type, CodeType))
		{
			case kCodeType_Write:
				write_data(machine, action, (data & action->extend_data) | (read_data(machine, action) & ~action->extend_data));
				break;

			case kCodeType_IWrite:
				switch(EXTRACT_FIELD(action->type, CodeParameter))
				{
					case IWRITE_WRITE:
						write_data(machine, action, data);
						break;

					case IWRITE_BIT_SET:
						write_data(machine, action, read_data(machine, action) | data);
						break;

					case IWRITE_BIT_CLEAR:
						write_data(machine, action, read_data(machine, action) & ~data);
						break;

					case IWRITE_LIMITED_MASK:
						write_data(machine, action, (EXTRACT_FIELD(data, MSB16) & EXTRACT_FIELD(data, LSB16)) | (read_data(machine, action) & ~EXTRACT_FIELD(data, LSB16)));
						break;
				}
				break;

			case kCodeType_CWrite:
				write_data(machine, action, data);
				break;

			case kCodeType_CBit:
				switch(EXTRACT_FIELD(action->type, CodeParameterUpper))
				{
					case CBIT_BIT_SET:
						write_data(machine, action, read_data(machine, action) | data);
						break;

					case CBIT_BIT_CLEAR:
						write_data(machine, action, read_data(machine, action) & ~data);
						break;

					case CBIT_LIMITED_MASK:
						write_data(machine, action, (EXTRACT_FIELD(data, MSB16) & EXTRACT_FIELD(data, LSB16)) | (read_data(machine, action) & ~EXTRACT_FIELD(data, LSB16)));
						break;
				}
				break;

			case kCodeType_Move:
				cheat_variable[EXTRACT_FIELD(action->type, CodeParameter)] = read_data(machine, action) + data;
				break;

			case kCodeType_Popup:
				switch(EXTRACT_FIELD(action->type, PopupParameter))
				{
					case kPopup_Label:
						ui_popup_time(1, "%s", action->optional_name);
						break;

					case kPopup_Value:
						ui_popup_time(1, "%*.*X",
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										read_data(machine, action));
						break;

					case kPopup_LabelValue:
						ui_popup_time(1, "%s %*.*X",
										action->optional_name,
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										read_data(machine, action));
						break;

					case kPopup_ValueLabel:
						ui_popup_time(1, "%*.*X %s",
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										kCheatSizeDigitsTable[EXTRACT_FIELD(action->type, AddressSize)],
										read_data(machine, action),
										action->optional_name);
						break;
				}
				break;
#ifdef MAME_DEBUG
			default:
				ui_popup_time(1, "Invalid CodeType : %X", EXTRACT_FIELD(action->type, CodeType));
#endif
		}
	}
}

/*------------------------------------------------------------
  cheat_periodicCondition - management for cheat conditions
------------------------------------------------------------*/

static UINT8 cheat_periodicCondition(running_machine *machine, cheat_action *action)
{
	int	data	= read_data(machine, action);
	int	value	= action->extend_data;

	if(EXTRACT_FIELD(action->type, CodeType) != kCodeType_CBit)
	{
		/* CWrite, Branch, Popup */
		switch(EXTRACT_FIELD(action->type, CodeParameter))
		{
			case kCondition_Equal:
				return (data == value);

			case kCondition_NotEqual:
				return (data != value);

			case kCondition_Less:
				return (data < value);

			case kCondition_LessOrEqual:
				return (data <= value);

			case kCondition_Greater:
				return (data > value);

			case kCondition_GreaterOrEqual:
				return (data >= value);

			case kCondition_BitSet:
				return (data & value);

			case kCondition_BitClear:
				return (!(data & value));

			case kCondition_PreviousValue:
				if(data != value)
				{
					action->extend_data = data;
					return 1;
				}
				break;

			case kCondition_KeyPressedOnce:
				return (input_code_pressed_once(value));

			case kCondition_KeyPressedRepeat:
				return (input_code_pressed(value));

			case kCondition_True:
				return 1;
		}
	}
	else
	{
		/* CBit */
		switch(EXTRACT_FIELD(action->type, CodeParameterLower))
		{
			case CONDITION_CBIT_BIT_SET:
				return (data & value);

			case CONDITION_CBIT_BIT_CLEAR:
				return (!(data & value));
		}
	}

	return 0;
}

/*------------------------------------------------------
  cheat_periodicAction - management for cheat actions
------------------------------------------------------*/

static int cheat_periodicAction(running_machine *machine, cheat_action *action, int selection)
{
	UINT8 execute_operation = 0;

	if(TEST_FIELD(action->type, PrefillEnable))
	{
		if((action->flags & kActionFlag_PrefillDone) == 0)
		{
			UINT8 prefillValue = kPrefillValueTable[EXTRACT_FIELD(action->type, PrefillEnable)];

			if((action->flags & kActionFlag_PrefillWritten) == 0)
			{
				/* set prefill */
				write_data(machine, action, prefillValue);
				action->flags |= kActionFlag_PrefillWritten;
				return (TEST_FIELD(action->type, Return) ? CHEAT_RETURN_VALUE : selection + 1);
			}
			else
			{
				/* do re-write */
				if(read_data(machine, action) == prefillValue)
					return (TEST_FIELD(action->type, Return) ? CHEAT_RETURN_VALUE : selection + 1);

				action->flags |= kActionFlag_PrefillDone;
			}
		}
	}

	if(EXTRACT_FIELD(action->type, DelayEnable))
	{
		if(TEST_FIELD(action->type, OneShot) && TEST_FIELD(action->type, RestoreValue))
		{
			/* Keep */
			execute_operation = 1;

			if(action->frame_timer++ >= EXTRACT_FIELD(action->type, DelayEnable) * ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds))
				action->flags |= kActionFlag_OperationDone;
		}
		else
		{
			if(action->frame_timer++ >= EXTRACT_FIELD(action->type, DelayEnable) * ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds))
			{
				/* Delay */
				execute_operation = 1;
				action->frame_timer = 0;

				if(TEST_FIELD(action->type, OneShot))
					action->flags |= kActionFlag_OperationDone;
			}
		}
	}
	else
	{
		if(action->frame_timer++)
		{
			if(action->frame_timer >= EXTRACT_FIELD(action->type, DelayEnable) + ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds))
				action->frame_timer = 0;

			return (TEST_FIELD(action->type, Return) ? CHEAT_RETURN_VALUE : selection + 1);
		}
		else
			execute_operation = 1;
	}

	if(action->flags & kActionFlag_CheckCondition)
	{
		if(cheat_periodicCondition(machine, action))
			execute_operation = 1;
		else
			execute_operation = 0;
	}

	if(execute_operation)
	{
		/* do cheat operation */
		switch(EXTRACT_FIELD(action->type, CodeType))
		{
			case kCodeType_Write:
			case kCodeType_IWrite:
			case kCodeType_RWrite:
			case kCodeType_CWrite:
			case kCodeType_CBit:
			case kCodeType_PDWWrite:
			case kCodeType_Move:
			case kCodeType_Popup:
				cheat_periodicOperation(machine, action);
				break;

			case kCodeType_Branch:
				return action->data;

			case kCodeType_Loop:
				{
					int counter = read_data(machine, action);

					if(counter != 0)
					{
						write_data(machine, action, counter - 1);

						return (TEST_FIELD(action->type, DataRead) ? cheat_variable[action->data] : action->data);
					}
				}
		}
	}

	return (TEST_FIELD(action->type, Return) ? CHEAT_RETURN_VALUE : selection + 1);
}

/*-----------------------------------------------------
  cheat_periodicEntry - management for cheat entries
-----------------------------------------------------*/

static void cheat_periodicEntry(running_machine *machine, cheat_entry *entry)
{
	int		i			= 0;
	UINT8	pressedKey	= 0;
	UINT8	isSelect	= 0;

	/* ***** 1st, handle activation key ***** */

	if(!ui_is_menu_active())
	{
		/* NOTE : activation key should be not checked in activating UI menu because it conflicts activation key checker */
		if((entry->flags & kCheatFlag_HasActivationKey1) && input_code_pressed_once(entry->activation_key_1))
			pressedKey = 1;
		else if((entry->flags & kCheatFlag_HasActivationKey2) && input_code_pressed_once(entry->activation_key_2))
			pressedKey = 2;
	}

	if(entry->flags & kCheatFlag_Select)
	{
		if(pressedKey)
		{
			if(pressedKey == 1)
			{
				if(--entry->selection < 0)
					entry->selection = entry->label_index_length - 1;
			}
			else
			{
				if(++entry->selection >= entry->label_index_length)
					entry->selection = 0;
			}

			/* NOTE : in handling activatio key, forced to activate a cheat even if one shot */
			if(!(entry->flags & kCheatFlag_OneShot) && !(entry->label_index[entry->selection]))
				deactivate_cheat(machine, entry);
			else
				activate_cheat(machine, entry);

			if(TEST_FIELD(cheat_options, ActivationKeyMessage))
			{
				if(!entry->selection)
					ui_popup_time(1,"%s disabled", entry->name);
				else
					ui_popup_time(1,"%s : %s selected", entry->name, entry->action_list[entry->label_index[entry->selection]].optional_name);
			}
		}
	}
	else
	{
		/* NOTE : if value-selection has activatinon key, no handling -----*/
		if(!(entry->flags & kCheatFlag_UserSelect) && pressedKey)
		{
			if(entry->flags & kCheatFlag_OneShot)
			{
				activate_cheat(machine, entry);

				if(TEST_FIELD(cheat_options, ActivationKeyMessage))
					ui_popup_time(1,"set %s", entry->name);
			}
			else if(entry->flags & kCheatFlag_Active)
			{
				deactivate_cheat(machine, entry);

				if(TEST_FIELD(cheat_options, ActivationKeyMessage))
					ui_popup_time(1,"%s disabled", entry->name);
			}
			else
			{
				activate_cheat(machine, entry);

				if(TEST_FIELD(cheat_options, ActivationKeyMessage))
					ui_popup_time(1,"%s enabled", entry->name);
			}
		}
	}

	/* ***** 2nd, do cheat actions ***** */

	/* if "OFF", no action */
	if(driverSpecifiedFlag != 0 || (entry->flags & kCheatFlag_Active) == 0)
		return;

	while(1)
	{
		if(entry->action_list[i].flags & kActionFlag_Custom)
		{
			if(entry->action_list[i].region == CUSTOM_CODE_LABEL_SELECT)
			{
				i = entry->label_index[entry->selection];
				isSelect = 1;
				continue;
			}
			else
				i++;
		}
		else if((entry->action_list[i].flags & kActionFlag_OperationDone) || (entry->action_list[i].flags & kActionFlag_NoAction))
		{
			i++;
		}
		else if(entry->action_list[i].flags & kActionFlag_OldFormat)
		{
			int	tempType	= entry->action_list[i].type;
			int	j			= i;

			entry->action_list[i].type = convert_to_new_code(&entry->action_list[i]);
			i = cheat_periodicAction(machine, &entry->action_list[i], i);
			entry->action_list[j].type = tempType;
		}
		else
			i = cheat_periodicAction(machine, &entry->action_list[i], i);

		if(i >= entry->action_list_length)
			break;

		if(isSelect && EXTRACT_FIELD(entry->action_list[i].type, Link) != kLink_SubLink)
			break;
	}

	/* ***** 3rd, deactive a cheat if one shot ***** */

	if(entry->flags & kCheatFlag_OneShot)
	{
		UINT8 done = 1;

		i = 0;
		isSelect = 0;

		while(1)
		{
			if(entry->action_list[i].flags & kActionFlag_Custom)
			{
				if(entry->action_list[i].region == CUSTOM_CODE_LABEL_SELECT)
				{
					i = entry->label_index[entry->selection];
					isSelect = 1;
					continue;
				}
			}
			else if((entry->action_list[i].flags & kActionFlag_OperationDone) == 0)
			{
				done = 0;
				i++;
			}
			else
				i++;

			if(i >= entry->action_list_length)
				break;

			if(isSelect && EXTRACT_FIELD(entry->action_list[i].type, Link) != kLink_SubLink)
				break;
		}

		if(done)
			deactivate_cheat(machine, entry);
	}
}

/*---------------------------------------------------------------------
  update_all_cheat_info - update all cheat info when database loaded
---------------------------------------------------------------------*/

static void update_all_cheat_info(running_machine *machine)
{
	int i;

	/* update flags for all cheat entry */
	for(i = 0; i < cheat_list_length; i++)
	{
		update_cheat_info(machine, &cheat_list[i], 1);

		if(cheat_list[i].flags & kCheatFlag_Select)
			build_label_index_table(&cheat_list[i]);
	}

	set_layer_index();
}

/*----------------------------------------------------------------------------------------------------
  update_cheat_info - check several fields on cheat entry and cheat action then set flags
                      "is_load_time" parameter is set when called update_all_cheat_info() right now
----------------------------------------------------------------------------------------------------*/

static void update_cheat_info(running_machine *machine, cheat_entry *entry, UINT8 is_load_time)
{
	int		i;
	int		flags = 0;
	UINT8	is_one_shot = 1;
	UINT8	is_null = 1;
	UINT8	is_separator = 1;
	UINT8	is_new_format = 1;

	flags = entry->flags & kCheatFlag_PersistentMask;

	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		int action_flags = action->flags & kActionFlag_PersistentMask;

		if(TEST_FIELD(action->type, OneShot) == 0)
			is_one_shot = 0;

		if(action_flags & kActionFlag_OldFormat)
		{
			if(	(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom) &&
				(EXTRACT_FIELD(action->type, LocationParameter) == kCustomLocation_Select))
					flags |= kCheatFlag_Select;

			if(is_null && EXTRACT_FIELD(action->type, LocationType) != kLocation_Custom)
				is_null = 0;

			if(TEST_FIELD(action->type, UserSelectEnable))
				flags |= kCheatFlag_UserSelect;

			if(	(EXTRACT_FIELD(action->type, LocationType) == kLocation_Custom) &&
				(EXTRACT_FIELD(action->type, LocationParameter) == kCustomLocation_Select))
					action_flags |= kActionFlag_NoAction;

			if(	(EXTRACT_FIELD(action->type, LocationType) == kLocation_IndirectIndexed) ||
				((EXTRACT_FIELD(action->type, LocationType) != kLocation_Custom) &&
				((EXTRACT_FIELD(action->type, Operation) | EXTRACT_FIELD(action->type, OperationExtend) << 2) == 1)))
					action_flags |= kActionFlag_IndexAddress;

			if(i && entry->action_list[i-1].type == entry->action_list[i].type)
			{
				if(	entry->action_list[i].address >= entry->action_list[i-1].address &&
					entry->action_list[i].address <= entry->action_list[i-1].address + 3)
						action_flags |= kActionFlag_NoAction;
			}

			is_new_format = 0;
		}
		else
		{
			if(action->region < CUSTOM_CODE)
			{
				switch(EXTRACT_FIELD(action->type, CodeType))
				{
					case kCodeType_Write:
						action_flags |= kActionFlag_MemoryWrite;
						break;

					case kCodeType_IWrite:
						action_flags |= kActionFlag_MemoryWrite;
						if(action->extend_data != ~0)
						{
							action_flags |= kActionFlag_IndexAddress;
							if(EXTRACT_FIELD(action->type, CodeParameterUpper) == IWRITE_LIMITED_MASK)
								action_flags |= kActionFlag_LimitedMask;
						}
						break;

					case kCodeType_RWrite:
						action_flags |= kActionFlag_MemoryWrite;
						action_flags |= kActionFlag_Repeat;
						break;

					case kCodeType_CWrite:
						action_flags |= kActionFlag_MemoryWrite;
						action_flags |= kActionFlag_CheckCondition;
						break;

					case kCodeType_CBit:
						action_flags |= kActionFlag_MemoryWrite;
						action_flags |= kActionFlag_CheckCondition;
						if(EXTRACT_FIELD(action->type, CodeParameterUpper) == CBIT_LIMITED_MASK)
							action_flags |= kActionFlag_LimitedMask;
						break;

					case kCodeType_PDWWrite:
						action_flags |= kActionFlag_MemoryWrite;
						action_flags |= kActionFlag_PDWWrite;
						break;

					case kCodeType_Move:
						if(action->extend_data != ~0)
							action_flags |= kActionFlag_IndexAddress;
						break;

					case kCodeType_Branch:
						action_flags |= kActionFlag_CheckCondition;
						break;

					case kCodeType_Popup:
						action_flags |= kActionFlag_CheckCondition;
						break;

					case kCodeType_Watch:
						action_flags |= kActionFlag_NoAction;
						break;
				}

				if(TEST_FIELD(action->type, ValueSelectEnable))
					flags |= kCheatFlag_UserSelect;

				is_null = 0;
				is_separator = 0;
			}
			else
			{
				/* is label-select? */
				if(action->region == CUSTOM_CODE_LABEL_SELECT)
					flags |= kCheatFlag_Select;

				/* is comment? */
				if(is_null && action->region != CUSTOM_CODE_COMMENT)
					is_null = 0;

				/* is separator? */
				if(is_separator && action->region != CUSTOM_CODE_SEPARATOR)
					is_separator = 0;

				/* is layer index? */
				if(action->region == CUSTOM_CODE_LAYER_TAG)
					flags |= kCheatFlag_LayerIndex;

				action_flags |= kActionFlag_NoAction;
				action_flags |= kActionFlag_Custom;
			}
		}

		action->flags = action_flags;
	}

	if(is_one_shot)
		flags |= kCheatFlag_OneShot;

	if(is_new_format == 0)
	{
		flags |= kCheatFlag_OldFormat;

		if(is_null)
			flags |= kCheatFlag_Null;
	}
	else
	{
		if(is_null)
		{
			/* NOTE : auto detection if multi-comment code */
			if(entry->action_list_length > 1)
				flags |= kCheatFlag_ExtendComment;
			else
				flags |= kCheatFlag_Null;
		}

		if(is_separator)
		{
			flags |= kCheatFlag_Null;
			flags |= kCheatFlag_Separator;
		}
	}

	entry->flags = (flags & kCheatFlag_InfoMask) | (entry->flags & ~kCheatFlag_InfoMask);

	/* clear dirty flag */
	if(is_load_time)
		entry->flags &= ~kCheatFlag_Dirty;

	check_code_format(machine, entry);
}

/*----------------------
  analyse_code_format
----------------------*/

static UINT32 analyse_code_format(running_machine *machine, cheat_entry *entry, cheat_action *action)
{
	UINT32 errorFlag = 0;

	/* check type field */
	if(action->flags & kActionFlag_OldFormat)
	{
		UINT8	type		= EXTRACT_FIELD(action->type, LocationType);
		UINT8	parameter	= EXTRACT_FIELD(action->type, LocationParameter);
		UINT8	operation	= EXTRACT_FIELD(action->type, Operation);

		if(type >= kLocation_Unused5)
			errorFlag |= kErrorFlag_InvalidLocationType;

		if(TEST_FIELD(action->type, OperationExtend))
			errorFlag |= kErrorFlag_InvalidOperation;

		if(type == kLocation_IndirectIndexed)
		{
			if(operation != kOperation_WriteMask && operation != kOperation_SetOrClearBits)
				errorFlag |= kErrorFlag_ConflictedExtendField;
		}

		if(operation == kOperation_ForceRange)
		{
			if(EXTRACT_FIELD(action->type, MSB16) <= EXTRACT_FIELD(action->type, LSB16))
				errorFlag |= kErrorFlag_InvalidRange;
		}

		if(type == kLocation_MemoryRegion)
		{
			if(!TEST_FIELD(action->type, RestorePreviousValue))
				errorFlag |= kErrorFlag_NoRestorePreviousValue;
		}

		if((entry->flags & kCheatFlag_UserSelect) && (entry->flags & kCheatFlag_Select))
			errorFlag |= kErrorFlag_ConflictedSelects;

		if(TEST_FIELD(action->type, UserSelectEnable))
		{
			if(!action->original_data)
				errorFlag |= kErrorFlag_NoSelectableValue;
		}

		if(type == kLocation_Custom && parameter == kCustomLocation_Select)
		{
			if(entry->action_list_length == 1)
				errorFlag |= kErrorFlag_NoLabel;

			if(action->original_data)
				errorFlag |= kErrorFlag_InvalidDataField;

			if(action->extend_data)
				errorFlag |= kErrorFlag_InvalidExtendDataField;
		}

		if(operation == kOperation_WriteMask)
		{
			if(type == kLocation_Standard)
			{
				if(!action->extend_data || action->extend_data == 0xFF || action->extend_data == 0xFFFF || action->extend_data == 0xFFFFFF)
					errorFlag |= kErrorFlag_InvalidExtendDataField;
			}
			else if(type == kLocation_MemoryRegion)
			{
				if(action->extend_data != ~0)
					errorFlag |= kErrorFlag_InvalidExtendDataField;
			}
		}
	}
	else
	{
		if((action->flags & kActionFlag_Custom) == 0)
		{
			UINT8	codeType		= EXTRACT_FIELD(action->type, CodeType);
			UINT8	codeParameter	= EXTRACT_FIELD(action->type, CodeParameter);

			if(codeType >= kCodeType_Unused)
				errorFlag |= kErrorFlag_InvalidCodeType;

			if(action->flags & kActionFlag_CheckCondition)
			{
				if(codeParameter >= kCondition_Unused1 && codeParameter <= kCondition_Unused4)
					errorFlag |= kErrorFlag_InvalidCondition;
			}

			if(codeType == kCodeType_PDWWrite)
			{
				if(EXTRACT_FIELD(action->type, AddressRead) == kReadFrom_Variable)
					errorFlag |= kErrorFlag_ForbittenVariable;
			}

			if(action->region >= REGION_CPU1 || EXTRACT_FIELD(action->type, AddressSpace) == kAddressSpace_OpcodeRAM)
			{
				if(!TEST_FIELD(action->type, RestoreValue))
					errorFlag |= kErrorFlag_NoRestorePreviousValue;
			}

			if(action->flags & kActionFlag_LimitedMask)
			{
				if(EXTRACT_FIELD(action->type, AddressSize) > kSearchSize_16Bit)
					errorFlag |= kErrorFlag_InvalidLimitedMaskSize;
			}

			if(action->flags & kActionFlag_Repeat)
			{
				if(!EXTRACT_FIELD(action->extend_data, LSB16))
					errorFlag |= kErrorFlag_NoRepeatCount;
			}

			if(EXTRACT_FIELD(action->type, AddressRead))
			{
				if(EXTRACT_FIELD(action->type, AddressRead) > kReadFrom_Variable)
					errorFlag |= kErrorFlag_UndefinedAddressRead;

				if(action->address >= VARIABLE_MAX_ARRAY)
					errorFlag |= kErrorFlag_AddressVariableOutRange;
			}

			if(TEST_FIELD(action->type, DataRead))
			{
				if(action->data >= VARIABLE_MAX_ARRAY)
					errorFlag |= kErrorFlag_DataVariableOutRange;
			}

			if(codeType == kCodeType_Write)
			{
				if(!action->extend_data || action->extend_data == 0xFF || action->extend_data == 0xFFFF || action->extend_data == 0xFFFFFF)
					errorFlag |= kErrorFlag_InvalidExtendDataField;
			}

			if((action->flags & kActionFlag_Custom) == 0)
			{
				if(action->region < REGION_INVALID)
				{
					UINT8 cpu = EXTRACT_FIELD(action->region, CPUIndex);

					if(VALID_CPU(cpu) == 0)
						errorFlag |= kErrorFlag_OutOfCPURegion;
					else if(cpu_info_list[cpu].type == 0)
						errorFlag |= kErrorFlag_InvalidCPURegion;

					if(EXTRACT_FIELD(action->type, AddressSpace) > kAddressSpace_EEPROM)
						errorFlag |= kErrorFlag_InvalidAddressSpace;
				}
				else
				{
					UINT8 region = action->region - REGION_INVALID;

					if(region >= REGION_MAX)				errorFlag |= kErrorFlag_OutOfCPURegion;
					else if(!region_info_list[region].type)	errorFlag |= kErrorFlag_InvalidCPURegion;
					else if(!is_address_in_range(action, memory_region_length(machine, action->region)))
															errorFlag |= kErrorFlag_RegionOutOfRange;
				}
			}
		}
		else
		{
			if(action->region >= CUSTOM_CODE_UNUSED_1)
				errorFlag |= kErrorFlag_InvalidCustomCode;
		}
	}

	return errorFlag;
}

/*------------------------------------------
  check_code_format - code format checker
------------------------------------------*/

static void check_code_format(running_machine *machine, cheat_entry *entry)
{
	int		i;
	UINT8	is_error = 0;

	for(i = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		if(analyse_code_format(machine, entry, action))
			is_error = 1;
	}

	if(is_error)
	{
		entry->flags |= kCheatFlag_HasWrongCode;
		SET_MESSAGE(CHEAT_MESSAGE_WRONG_CODE);
	}
}

/*----------------------------------------------------------------------------------------------------
  build_label_index_table - create index table for label-selection and calculate index table length
----------------------------------------------------------------------------------------------------*/

static void build_label_index_table(cheat_entry *entry)
{
	int i, j, length = 0;

	entry->label_index_length = 0;

	for(i = 0, j = 0; i < entry->action_list_length; i++)
	{
		cheat_action *action = &entry->action_list[i];

		if((action->flags & kActionFlag_Custom) && action->region == CUSTOM_CODE_LABEL_SELECT)
		{
			/* label selection master code */
			entry->label_index = malloc(sizeof(entry->label_index) * (++entry->label_index_length + 1));

			if(entry->label_index == NULL)
				goto label_index_error;

			entry->label_index[j++] = i;
			length = action->data ? action->data + i : entry->action_list_length;
		}
		else if(EXTRACT_FIELD(action->type, Link) == kLink_Link)
		{
			/* link or sub-link code */
			if(length)
			{
				entry->label_index = realloc(entry->label_index, sizeof(entry->label_index) * (++entry->label_index_length + 1));

				if(entry->label_index == NULL)
					goto label_index_error;

				entry->label_index[j++] = i;
			}
		}
	}

	if(entry->label_index_length <= 1)
	{
		logerror("cheat: [label index table] %s fails to build due to invalid or no link\n", entry->name);
		return;
	}
	else
		/* set terminator */
		entry->label_index[entry->label_index_length] = ~0;

	if(entry->flags & kCheatFlag_OneShot)
		entry->selection = 1;

	/* logerror("Cheat - Finish building index table for %s (length = %x)\n", entry->name, entry->label_index_length); */
	/* for(i = 0; i < entry->label_index_length; i++) */
	/*  logerror("IndexTable[%x] = %x\n",i,entry->label_index[i]); */

	return;

	label_index_error:

	fatalerror("cheat:[label index table] memory allocation error\n"
				"	name =					%s\n"
				"	label_index_length =	%.8X\n"
				"	label_index_length =	%p\n",
				entry->name, entry->label_index_length, entry->label_index);
}

/*------------------
  set_layer_index
------------------*/

static void set_layer_index(void)
{
	int i, j;

	for(i = 0; i < cheat_list_length; i++)
	{
		cheat_entry *entry = &cheat_list[i];

		if(entry->flags & kCheatFlag_LayerIndex)
		{
			int length = entry->action_list[0].extend_data;

			if(length < cheat_list_length && (i + length) <= cheat_list_length)
			{
				entry->layer_index = entry->action_list[0].address;

				for(j = i + 1; j <= i + length; j++)
				{
					cheat_entry *traverse = &cheat_list[j];

					if((traverse->flags & kCheatFlag_LayerIndex) == 0)
						traverse->layer_index = entry->action_list[0].data;
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------
  display_cheat_message - display cheat message via ui_draw_text_box instead of popup menu
-------------------------------------------------------------------------------------------*/

static void display_cheat_message(void)
{
	char buf[64];

	switch(message_type)
	{
		/* simple message */
		default:
			sprintf(buf, "%s", CHEAT_MESSAGE_TABLE[message_type]);
			break;

		/* message with data */
		case CHEAT_MESSAGE_CHEAT_FOUND:
		case CHEAT_MESSAGE_INVALIDATE_REGION:
			{
				search_info *search = get_current_search();

				sprintf(buf, "%d %s", search->num_results, CHEAT_MESSAGE_TABLE[message_type]);
			}
			break;

		case CHEAT_MESSAGE_ALL_CHEATS_SAVED:
			sprintf(buf, "%d %s", cheat_list_length, CHEAT_MESSAGE_TABLE[message_type]);
			break;
	}

	/* draw it */
	switch(message_type)
	{
		default:
			ui_draw_text_box(buf, JUSTIFY_CENTER, 0.5f, 0.9f, UI_FILLCOLOR);
			break;

		case CHEAT_MESSAGE_NONE:
		case CHEAT_MESSAGE_FAILED_TO_LOAD_DATABASE:
		case CHEAT_MESSAGE_FAILED_TO_SAVE:
		case CHEAT_MESSAGE_NO_SUPPORTED_OLD_FORMAT:
		case CHEAT_MESSAGE_NO_ACTIVATION_KEY:
		case CHEAT_MESSAGE_FAILED_TO_ADD:
		case CHEAT_MESSAGE_FAILED_TO_DELETE:
		case CHEAT_MESSAGE_NO_SEARCH_REGION:
		case CHEAT_MESSAGE_NO_OLD_VALUE:
		case CHEAT_MESSAGE_FAILED_TO_ALLOCATE:
		case CHEAT_MESSAGE_WRONG_CODE:
		case CHEAT_MESSAGE_MAX:
			/* warning message has red background color */
			ui_draw_text_box(buf, JUSTIFY_CENTER, 0.5f, 0.9f, UI_REDCOLOR);
			break;
	}

	/* decrement message timer */
	if(--message_timer == 0)
		message_type = 0;
}

/*---------------------
  get_address_length
---------------------*/

static UINT8 get_address_length(UINT8 region)
{
	if(region < CUSTOM_CODE)
	{
		cpu_region_info *info = get_cpu_or_region_info(region);

		if(info && info->type)
			return info->address_chars_needed;
	}

	return cpu_info_list[0].address_chars_needed;
}

/*------------------
  get_region_name
------------------*/

static char *get_region_name(UINT8 region)
{
	if(region < REGION_INVALID)
	{
		if(VALID_CPU(region))
			return (char *)kRegionNames[EXTRACT_FIELD(region, CPUIndex) + 1];
	}
	else
	{
		if(region < REGION_MAX)
			return (char *)kRegionNames[region - REGION_INVALID];
	}

	return (char *)kRegionNames[0];
}

/*------------------------------------------------------------------------------------
  build_cpu_region_info_list - get CPU and region info when initialize cheat system
------------------------------------------------------------------------------------*/

static void build_cpu_region_info_list(running_machine *machine)
{
	int i;

	/* do regions */
	{
		const rom_entry *traverse = rom_first_region(machine->gamedrv);

		memset(region_info_list, 0, sizeof(cpu_region_info) * REGION_LIST_LENGTH);

		while(traverse)
		{
			if(ROMENTRY_ISREGION(traverse))
			{
				UINT8 region_type = ROMREGION_GETTYPE(traverse);

				/* non-cpu region */
				if(region_type >= REGION_GFX1 && region_type <= REGION_PLDS)
				{
					UINT8	bit_state		= 0;
					UINT32	length			= memory_region_length(machine, region_type);
					cpu_region_info *info	= &region_info_list[region_type - REGION_INVALID];

					info->type						= region_type;
					info->data_bits					= ROMREGION_GETWIDTH(traverse);
					info->address_bits				= 0;
					info->address_mask				= length;
					info->address_chars_needed		= info->address_bits >> 2;
					info->endianness				= ROMREGION_ISBIGENDIAN(traverse);

					if(info->address_bits & 3)
						info->address_chars_needed++;

					/* build address mask */
					for(i = 0; i < 32; i++)
					{
						UINT32 mask = 1 << (31 - i);

						if(bit_state)
							info->address_mask |= mask;
						else
						{
							if(info->address_mask & mask)
							{
								info->address_bits = 32 - i;
								bit_state = 1;
							}
						}
					}
				}
			}

			traverse = rom_next_region(traverse);
		}
	}

	/* do CPUs */
	{
		memset(cpu_info_list, 0, sizeof(cpu_region_info) * MAX_CPU);

		for(i = 0; i < cpu_gettotalcpu(); i++)
		{
			cpu_region_info	*cpu_info		= &cpu_info_list[i];
			cpu_region_info	*region_info	= &region_info_list[REGION_CPU1 + i - REGION_INVALID];
			cpu_type		type			= machine->config->cpu[i].type;

			cpu_info->type						= type;
			cpu_info->data_bits					= cputype_databus_width(type, ADDRESS_SPACE_PROGRAM);
			cpu_info->address_bits				= cputype_addrbus_width(type, ADDRESS_SPACE_PROGRAM);
			cpu_info->address_mask				= 0xFFFFFFFF >> (32 - cpu_info->address_bits);
			cpu_info->address_chars_needed		= cpu_info->address_bits >> 2;
			cpu_info->endianness				= (cputype_endianness(type) == CPU_IS_BE);
			cpu_info->address_shift				= cputype_addrbus_shift(type, ADDRESS_SPACE_PROGRAM);

			if(cpu_info->address_bits & 0x3)
				cpu_info->address_chars_needed++;

			/* copy CPU info to region info */
			memcpy(region_info, cpu_info, sizeof(cpu_region_info));
		}
	}
}
