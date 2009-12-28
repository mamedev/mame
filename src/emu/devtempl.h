/***************************************************************************

    devtempl.h

    Template include for defining devices.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Typical usage is as follows:

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

// for a primary device....
#define DEVTEMPLATE_ID(p,s)             p##devicenameprefix##s
#define DEVTEMPLATE_FEATURES            DT_HAS_xxx | DT_HAS_yyy | ...
#define DEVTEMPLATE_NAME                "Device Name String"
#define DEVTEMPLATE_FAMILY              "Device Family String"
#define DEVTEMPLATE_CLASS               DEVICE_CLASS_xxxx
#include "devtempl.h"

// for a derived device....
#define DEVTEMPLATE_DERIVED_ID(p,s)     p##derivednameprefix##s
#define DEVTEMPLATE_DERIVED_FEATURES    DT_HAS_xxx | DT_HAS_yyy | ...
#define DEVTEMPLATE_NAME                "Derived Name String"
#include "devtempl.h"

****************************************************************************

    Parameters are as follows:

    DEVTEMPLATE_ID(p,s) - required - macro to produce device function and
        type names with a prefix of 'p' and a suffix of 's'

    DEVTEMPLATE_FEATURES - required - bitmask consisting of one of the
        DT_HAS_* flags, indicating which standard-named callbacks or
        pointers are specified by this device (everything else is assumed
        to be NULL, which is the default)

    DEVTEMPLATE_NAME - required - a string describing the device

    DEVTEMPLATE_FAMILY - required - a string describing the device family
        name

    DEVTEMPLATE_STATE - optional - the name of the device's state
        structure; by default, this is assumed to be
        DEVTEMPLATE_ID(,_state)

    DEVTEMPLATE_CLASS - optional - the device's class (default is
        DEVICE_CLASS_PERIPHERAL)

    DEVTEMPLATE_VERSION - optional - the device's version string (default
        is "1.0")

    DEVTEMPLATE_CREDITS - optional - the device's credit string (default
        is "Copyright Nicola Salmoria and the MAME Team")

***************************************************************************/


#define DEVTEMPLATE_ID1(x) DEVTEMPLATE_ID(x,)
#define DEVTEMPLATE_DERIVED_ID1(x) DEVTEMPLATE_DERIVED_ID(x,)

/* flag bits for DEVTEMPLATE_FEATURES */
#define DT_HAS_START			0x0001
#define DT_HAS_RESET			0x0002
#define DT_HAS_STOP				0x0004
#define DT_HAS_EXECUTE			0x0008
#define DT_HAS_NVRAM			0x0010
#define DT_HAS_VALIDITY_CHECK	0x0020
#define DT_HAS_CUSTOM_CONFIG	0x0040
#define DT_HAS_ROM_REGION		0x0080
#define DT_HAS_MACHINE_CONFIG	0x0100
#define DT_HAS_INLINE_CONFIG	0x0200
#define DT_HAS_CONTRACT_LIST	0x0400
#define DT_HAS_PROGRAM_SPACE	0x1000
#define DT_HAS_DATA_SPACE		0x2000
#define DT_HAS_IO_SPACE			0x4000


/* verify core stuff is specified */
#ifndef DEVTEMPLATE_ID
#error DEVTEMPLATE_ID must be specified!
#endif

#ifndef DEVTEMPLATE_FEATURES
#error DEVTEMPLATE_FEATURES must be specified!
#endif

#if (((DEVTEMPLATE_FEATURES) & DT_HAS_START) == 0)
#error Device start routine is required!
#endif

#ifndef DEVTEMPLATE_NAME
#error DEVTEMPLATE_NAME must be specified!
#endif

#ifndef DEVTEMPLATE_FAMILY
#error DEVTEMPLATE_FAMILY must be specified!
#endif

#if (((DEVTEMPLATE_FEATURES) & (DT_HAS_PROGRAM_SPACE | DT_HAS_DATA_SPACE | DT_HAS_IO_SPACE)) != 0)
#ifndef DEVTEMPLATE_ENDIANNESS
#error DEVTEMPLATE_ENDIANNESS must be specified if an address space is present!
#endif
#endif

#ifdef DEVTEMPLATE_DERIVED_FEATURES
#ifndef DEVTEMPLATE_DERIVED_NAME
#error DEVTEMPLATE_DERIVED_NAME must be specified!
#endif
#endif


/* primary device case */
#ifndef DEVTEMPLATE_DERIVED_FEATURES

/* derive standard state name (unless explicitly provided) */
#ifndef DEVTEMPLATE_STATE
#define DEVTEMPLATE_STATE		DEVTEMPLATE_ID(,_state)
#endif

/* default to DEVICE_CLASS_PERIPHERAL */
#ifndef DEVTEMPLATE_CLASS
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#endif

/* default to version 1.0 */
#ifndef DEVTEMPLATE_VERSION
#define DEVTEMPLATE_VERSION		"1.0"
#endif

/* default to the standard copyright attribution */
#ifndef DEVTEMPLATE_CREDITS
#define DEVTEMPLATE_CREDITS 	"Copyright Nicola Salmoria and the MAME Team"
#endif

/* declare callback functions */
static DEVICE_START( DEVTEMPLATE_ID(,) );
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_RESET)
static DEVICE_RESET( DEVTEMPLATE_ID(,) );
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_STOP)
static DEVICE_STOP( DEVTEMPLATE_ID(,) );
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_EXECUTE)
static DEVICE_EXECUTE( DEVTEMPLATE_ID(,) );
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_NVRAM)
static DEVICE_NVRAM( DEVTEMPLATE_ID(,) );
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_VALIDITY_CHECK)
static DEVICE_VALIDITY_CHECK( DEVTEMPLATE_ID(,) );
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_CUSTOM_CONFIG)
static DEVICE_CUSTOM_CONFIG( DEVTEMPLATE_ID(,) );
#endif

/* the actual get_info function */
DEVICE_GET_INFO( DEVTEMPLATE_ID(,) )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(DEVTEMPLATE_STATE);							break;
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_INLINE_CONFIG)
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(DEVTEMPLATE_ID(,_config));						break;
#endif
		case DEVINFO_INT_CLASS:					info->i = DEVTEMPLATE_CLASS;									break;
#ifdef DEVTEMPLATE_ENDIANNESS
		case DEVINFO_INT_ENDIANNESS:			info->i = DEVTEMPLATE_ENDIANNESS;								break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_PROGRAM_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_0:		info->i = DEVTEMPLATE_PGM_DATAWIDTH;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:		info->i = DEVTEMPLATE_PGM_ADDRWIDTH;							break;
#ifdef DEVTEMPLATE_PGM_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_0:		info->i = DEVTEMPLATE_PGM_ADDRSHIFT;							break;
#endif
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_DATA_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_1:		info->i = DEVTEMPLATE_DATA_DATAWIDTH;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH_1:		info->i = DEVTEMPLATE_DATA_ADDRWIDTH;							break;
#ifdef DEVTEMPLATE_DATA_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_1:		info->i = DEVTEMPLATE_DATA_ADDRSHIFT;							break;
#endif
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_IO_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_2:		info->i = DEVTEMPLATE_IO_DATAWIDTH;								break;
		case DEVINFO_INT_ADDRBUS_WIDTH_2:		info->i = DEVTEMPLATE_IO_ADDRWIDTH;								break;
#ifdef DEVTEMPLATE_IO_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_2:		info->i = DEVTEMPLATE_IO_ADDRSHIFT;								break;
#endif
#endif

		/* --- the following bits of info are returned as pointers --- */
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_ROM_REGION)
		case DEVINFO_PTR_ROM_REGION:			info->romregion = DEVTEMPLATE_ID1(ROM_NAME());						break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_MACHINE_CONFIG)
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = DEVTEMPLATE_ID1(MACHINE_DRIVER_NAME());		break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_CONTRACT_LIST)
		case DEVINFO_PTR_CONTRACT_LIST:			info->contract_list = DEVTEMPLATE_ID1(DEVICE_CONTRACT_LIST_NAME());	break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_PROGRAM_SPACE)
#ifdef DEVTEMPLATE_PGM_INTMAP
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_PGM_INTMAP;							break;
#endif
#ifdef DEVTEMPLATE_PGM_DEFMAP
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_PGM_DEFMAP;							break;
#endif
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_DATA_SPACE)
#ifdef DEVTEMPLATE_DATA_INTMAP
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_DATA_INTMAP;							break;
#endif
#ifdef DEVTEMPLATE_DATA_DEFMAP
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_DATA_DEFMAP;							break;
#endif
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_IO_SPACE)
#ifdef DEVTEMPLATE_IO_INTMAP
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_IO_INTMAP;							break;
#endif
#ifdef DEVTEMPLATE_IO_DEFMAP
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:	info->p = (void *)DEVTEMPLATE_IO_DEFMAP;							break;
#endif
#endif


		/* --- the following bits of info are returned as pointers to data or functions --- */
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_START)
		case DEVINFO_FCT_START:					info->start = DEVTEMPLATE_ID1(DEVICE_START_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_RESET)
		case DEVINFO_FCT_RESET:					info->reset = DEVTEMPLATE_ID1(DEVICE_RESET_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_STOP)
		case DEVINFO_FCT_STOP:					info->stop = DEVTEMPLATE_ID1(DEVICE_STOP_NAME());					break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_EXECUTE)
		case DEVINFO_FCT_EXECUTE:				info->execute = DEVTEMPLATE_ID1(DEVICE_EXECUTE_NAME()); 				break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_NVRAM)
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVTEMPLATE_ID1(DEVICE_NVRAM_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_VALIDITY_CHECK)
		case DEVINFO_FCT_VALIDITY_CHECK:		info->validity_check = DEVTEMPLATE_ID1(DEVICE_VALIDITY_CHECK_NAME());	break;
#endif
#if ((DEVTEMPLATE_FEATURES) & DT_HAS_CUSTOM_CONFIG)
		case DEVINFO_FCT_CUSTOM_CONFIG:			info->custom_config = DEVTEMPLATE_ID1(DEVICE_CUSTOM_CONFIG_NAME());	break;
#endif

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, DEVTEMPLATE_NAME);								break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, DEVTEMPLATE_FAMILY);							break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, DEVTEMPLATE_VERSION);							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, DEVTEMPLATE_SOURCE);							break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, DEVTEMPLATE_CREDITS);							break;
	}
}


/* derived device case */
#else

/* declare callback functions */
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_START)
static DEVICE_START( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_RESET)
static DEVICE_RESET( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_STOP)
static DEVICE_STOP( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_EXECUTE)
static DEVICE_EXECUTE( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_NVRAM)
static DEVICE_NVRAM( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_VALIDITY_CHECK)
static DEVICE_VALIDITY_CHECK( DEVTEMPLATE_DERIVED_ID(,) );
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_CUSTOM_CONFIG)
static DEVICE_CUSTOM_CONFIG( DEVTEMPLATE_DERIVED_ID(,) );
#endif

/* the actual get_info function */
DEVICE_GET_INFO( DEVTEMPLATE_DERIVED_ID(,) )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_PROGRAM_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_0:		info->i = DEVTEMPLATE_DERIVED_PGM_DATAWIDTH;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:		info->i = DEVTEMPLATE_DERIVED_PGM_ADDRWIDTH;					break;
#ifdef DEVTEMPLATE_PGM_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_0:		info->i = DEVTEMPLATE_DERIVED_PGM_ADDRSHIFT;					break;
#endif
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_DATA_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_1:		info->i = DEVTEMPLATE_DERIVED_DATA_DATAWIDTH;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH_1:		info->i = DEVTEMPLATE_DERIVED_DATA_ADDRWIDTH;					break;
#ifdef DEVTEMPLATE_DATA_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_1:		info->i = DEVTEMPLATE_DERIVED_DATA_ADDRSHIFT;					break;
#endif
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_IO_SPACE)
		case DEVINFO_INT_DATABUS_WIDTH_2:		info->i = DEVTEMPLATE_DERIVED_IO_DATAWIDTH;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH_2:		info->i = DEVTEMPLATE_DERIVED_IO_ADDRWIDTH;					break;
#ifdef DEVTEMPLATE_IO_ADDRSHIFT
		case DEVINFO_INT_ADDRBUS_SHIFT_2:		info->i = DEVTEMPLATE_DERIVED_IO_ADDRSHIFT;					break;
#endif
#endif

		/* --- the following bits of info are returned as pointers --- */
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_ROM_REGION)
		case DEVINFO_PTR_ROM_REGION:			info->romregion = DEVTEMPLATE_DERIVED_ID1(ROM_NAME());						break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_MACHINE_CONFIG)
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = DEVTEMPLATE_DERIVED_ID1(MACHINE_DRIVER_NAME());		break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_CONTRACT_LIST)
		case DEVINFO_PTR_CONTRACT_LIST:			info->contract_list = DEVTEMPLATE_DERIVED_ID1(DEVICE_CONTRACT_LIST_NAME()); 			break;
#endif

		/* --- the following bits of info are returned as pointers to data or functions --- */
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_START)
		case DEVINFO_FCT_START:					info->start = DEVTEMPLATE_DERIVED_ID1(DEVICE_START_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_RESET)
		case DEVINFO_FCT_RESET:					info->reset = DEVTEMPLATE_DERIVED_ID1(DEVICE_RESET_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_STOP)
		case DEVINFO_FCT_STOP:					info->stop = DEVTEMPLATE_DERIVED_ID1(DEVICE_STORE_NAME());					break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_EXECUTE)
		case DEVINFO_FCT_EXECUTE:				info->execute = DEVTEMPLATE_DERIVED_ID1(DEVICE_EXECUTE_NAME()); 				break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_NVRAM)
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVTEMPLATE_DERIVED_ID1(DEVICE_NVRAM_NAME()); 					break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_VALIDITY_CHECK)
		case DEVINFO_FCT_VALIDITY_CHECK:		info->validity_check = DEVTEMPLATE_DERIVED_ID1(DEVICE_VALIDITY_CHECK_NAME()); break;
#endif
#if ((DEVTEMPLATE_DERIVED_FEATURES) & DT_HAS_CUSTOM_CONFIG)
		case DEVINFO_FCT_CUSTOM_CONFIG:			info->custom_config = DEVTEMPLATE_DERIVED_ID1(DEVICE_CUSTOM_CONFIG_NAME()); 	break;
#endif

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, DEVTEMPLATE_DERIVED_NAME);								break;
		default:								DEVICE_GET_INFO_CALL(DEVTEMPLATE_ID(,));								break;
	}
}

#endif



#undef DT_HAS_RESET
#undef DT_HAS_STOP
#undef DT_HAS_EXECUTE
#undef DT_HAS_NVRAM
#undef DT_HAS_VALIDITY_CHECK
#undef DT_HAS_CUSTOM_CONFIG
#undef DT_HAS_ROM_REGION
#undef DT_HAS_MACHINE_CONFIG
#undef DT_HAS_INLINE_CONFIG
#undef DT_HAS_CONTRACT_LIST
#undef DT_HAS_PROGRAM_SPACE
#undef DT_HAS_DATA_SPACE
#undef DT_HAS_IO_SPACE

#undef DEVTEMPLATE_DERIVED_ID
#undef DEVTEMPLATE_DERIVED_FEATURES
#undef DEVTEMPLATE_DERIVED_NAME
