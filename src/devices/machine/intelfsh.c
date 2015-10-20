// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
    Flash ROM emulation

    Explicitly supports:
    Intel 28F016S5 (byte-wide)
    AMD/Fujitsu 29F016 (byte-wide)
    Sharp LH28F400 (word-wide)

    Flash ROMs use a standardized command set across manufacturers,
    so this emulation should work even for non-Intel and non-Sharp chips
    as long as the game doesn't query the maker ID.
*/

#include "emu.h"
#include "intelfsh.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	FM_NORMAL,  // normal read/write
	FM_READID,  // read ID
	FM_READSTATUS,  // read status
	FM_WRITEPART1,  // first half of programming, awaiting second
	FM_CLEARPART1,  // first half of clear, awaiting second
	FM_SETMASTER,   // first half of set master lock, awaiting on/off
	FM_READAMDID1,  // part 1 of alt ID sequence
	FM_READAMDID2,  // part 2 of alt ID sequence
	FM_READAMDID3,  // part 3 of alt ID sequence
	FM_ERASEAMD1,   // part 1 of AMD erase sequence
	FM_ERASEAMD2,   // part 2 of AMD erase sequence
	FM_ERASEAMD3,   // part 3 of AMD erase sequence
	FM_ERASEAMD4,   // part 4 of AMD erase sequence
	FM_BYTEPROGRAM,
	FM_BANKSELECT,
	FM_WRITEPAGEATMEL
};


enum
{
	MFG_ALLIANCE = 0x52,
	MFG_AMD = 0x01,
	MFG_AMIC = 0x37,
	MFG_ATMEL = 0x1f,
	MFG_BRIGHT = 0xad,
	MFG_CATALYST = 0x31,
	MFG_EON = 0x1c,
	MFG_FUJITSU = 0x04,
	MFG_GIGADEVICE = 0xc8,
	MFG_HYUNDAI = 0xad,
	MFG_INTEL = 0x89,
	MFG_ISSI = 0xd5,
	MFG_MACRONIX = 0xc2,
	MFG_PANASONIC = 0x32,
	MFG_PMC = 0x9d,
	MFG_SANYO = 0x62,
	MFG_SHARP = 0xb0,
	MFG_SPANSION = 0x01,
	MFG_SST = 0xbf,
	MFG_ST = 0x20,
	MFG_SYNCMOS = 0x40,
	MFG_TI = 0x97,
	MFG_TI_OLD = 0x01,
	MFG_WINBOND_NEX = 0xef,
	MFG_WINBOND = 0xda
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type INTEL_28F016S5 = &device_creator<intel_28f016s5_device>;
const device_type SHARP_LH28F016S = &device_creator<sharp_lh28f016s_device>;
const device_type ATMEL_29C010 = &device_creator<atmel_29c010_device>;
const device_type AMD_29F010 = &device_creator<amd_29f010_device>;
const device_type AMD_29F040 = &device_creator<amd_29f040_device>;
const device_type AMD_29F080 = &device_creator<amd_29f080_device>;
const device_type AMD_29F400T = &device_creator<amd_29f400t_device>;
const device_type AMD_29F800T = &device_creator<amd_29f800t_device>;
const device_type AMD_29LV200T = &device_creator<amd_29lv200t_device>;
const device_type FUJITSU_29F160T = &device_creator<fujitsu_29f160t_device>;
const device_type FUJITSU_29F016A = &device_creator<fujitsu_29f016a_device>;
const device_type FUJITSU_29DL16X = &device_creator<fujitsu_29dl16x_device>;
const device_type INTEL_E28F400B = &device_creator<intel_e28f400b_device>;
const device_type MACRONIX_29L001MC = &device_creator<macronix_29l001mc_device>;
const device_type MACRONIX_29LV160TMC = &device_creator<macronix_29lv160tmc_device>;
const device_type TMS_29F040 = &device_creator<tms_29f040_device>;

const device_type PANASONIC_MN63F805MNP = &device_creator<panasonic_mn63f805mnp_device>;
const device_type SANYO_LE26FV10N1TS = &device_creator<sanyo_le26fv10n1ts_device>;
const device_type SST_28SF040 = &device_creator<sst_28sf040_device>;
const device_type SST_39VF020 = &device_creator<sst_39vf020_device>;

const device_type SHARP_LH28F400 = &device_creator<sharp_lh28f400_device>;
const device_type INTEL_E28F008SA = &device_creator<intel_e28f008sa_device>;
const device_type INTEL_TE28F160 = &device_creator<intel_te28f160_device>;
const device_type SHARP_UNK128MBIT = &device_creator<sharp_unk128mbit_device>;
const device_type INTEL_28F320J3D = &device_creator<intel_28f320j3d_device>;
const device_type INTEL_28F320J5 = &device_creator<intel_28f320j5_device>;

const device_type SST_39VF400A = &device_creator<sst_39vf400a_device>;

static ADDRESS_MAP_START( memory_map8_512Kb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x00ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_1Mb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x01ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_2Mb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x03ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_4Mb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x07ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_8Mb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x0fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_16Mb, AS_PROGRAM, 8, intelfsh_device )
	AM_RANGE(0x00000, 0x1fffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( memory_map16_4Mb, AS_PROGRAM, 16, intelfsh_device )
	AM_RANGE(0x00000, 0x03ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map16_16Mb, AS_PROGRAM, 16, intelfsh_device )
	AM_RANGE(0x00000, 0x0fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map16_32Mb, AS_PROGRAM, 16, intelfsh_device )
	AM_RANGE(0x00000, 0x1fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map16_64Mb, AS_PROGRAM, 16, intelfsh_device )
	AM_RANGE(0x00000, 0x3fffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intelfsh_device - constructor
//-------------------------------------------------

intelfsh_device::intelfsh_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_type(variant),
		m_size(0),
		m_bits(8),
		m_addrmask(0),
		m_device_id(0),
		m_maker_id(0),
		m_sector_is_4k(false),
		m_sector_is_16k(false),
		m_top_boot_sector(false),
		m_status(0x80),
		m_erase_sector(0),
		m_flash_mode(FM_NORMAL),
		m_flash_master_lock(false),
		m_timer(NULL),
		m_bank(0)
{
	address_map_constructor map = NULL;

	switch( variant )
	{
	case FLASH_INTEL_28F016S5:
	case FLASH_SHARP_LH28F016S:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = MFG_INTEL;
		m_device_id = 0xaa;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_ATMEL_29C010:
		m_bits = 8;
		m_size = 0x20000;
		m_page_size = 0x80;
		m_maker_id = MFG_ATMEL;
		m_device_id = 0xd5;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_AMD_29F010:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = MFG_AMD;
		m_device_id = 0x20;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_AMD_29F040:
		m_bits = 8;
		m_size = 0x80000;
		m_maker_id = MFG_AMD;
		m_device_id = 0xa4;
		map = ADDRESS_MAP_NAME( memory_map8_4Mb );
		break;
	case FLASH_AMD_29F080:
		m_bits = 8;
		m_size = 0x100000;
		m_addrmask = 0x7ff;
		m_maker_id = MFG_AMD;
		m_device_id = 0xd5;
		map = ADDRESS_MAP_NAME( memory_map8_8Mb );
		break;
	case FLASH_AMD_29F400T:
		m_bits = 8;
		m_size = 0x80000;
		m_maker_id = MFG_AMD;
		m_device_id = 0x23;
		m_top_boot_sector = true;
		map = ADDRESS_MAP_NAME( memory_map8_4Mb );
		break;
	case FLASH_AMD_29F800T:
		m_bits = 8;
		m_size = 0x100000;
		m_maker_id = MFG_AMD;
		m_device_id = 0xda;
		m_top_boot_sector = true;
		map = ADDRESS_MAP_NAME( memory_map8_8Mb );
		break;
	case FLASH_AMD_29LV200T:
		m_bits = 8;
		m_size = 0x40000;
		m_maker_id = MFG_AMD;
		m_device_id = 0x3b;
		map = ADDRESS_MAP_NAME( memory_map8_2Mb );
		break;
	case FLASH_INTEL_28F320J3D:
		m_bits = 16;
		m_size = 0x400000;
		m_maker_id = MFG_INTEL;
		m_device_id = 0x16;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map16_32Mb );
		break;
	case FLASH_INTEL_28F320J5: // funkball
		m_bits = 16;
		m_size = 0x400000;
		m_maker_id = MFG_INTEL;
		m_device_id = 0x14;
//      m_sector_is_4k = true; 128kb?
		map = ADDRESS_MAP_NAME( memory_map16_32Mb );
		break;
	case FLASH_SST_39VF020:
		m_bits = 8;
		m_size = 0x40000;
		m_maker_id = MFG_SST;
		m_device_id = 0xd6;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map8_2Mb );
		break;
	case FLASH_SST_39VF400A:
		m_bits = 16;
		m_size = 0x80000;
		m_maker_id = MFG_SST;
		m_device_id = 0xd6;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map16_4Mb );
		break;
	case FLASH_SHARP_LH28F400:
		m_bits = 16;
		m_size = 0x80000;
		m_maker_id = MFG_SHARP;
		m_device_id = 0xed;
		map = ADDRESS_MAP_NAME( memory_map16_4Mb );
		break;
	case FLASH_INTEL_E28F400B:
		m_bits = 16;
		m_size = 0x80000;
		m_maker_id = MFG_INTEL;
		m_device_id = 0x4471;
		map = ADDRESS_MAP_NAME( memory_map16_4Mb );
		break;
	case FLASH_FUJITSU_29F160T:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = MFG_FUJITSU;
		m_device_id = 0xad;
		m_top_boot_sector = true;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_FUJITSU_29F016A:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = MFG_FUJITSU;
		m_device_id = 0xad;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_FUJITSU_29DL16X:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = MFG_FUJITSU;
		m_device_id = 0x35;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_INTEL_E28F008SA:
		m_bits = 8;
		m_size = 0x100000;
		m_maker_id = MFG_INTEL;
		m_device_id = 0xa2;
		map = ADDRESS_MAP_NAME( memory_map8_8Mb );
		break;
	case FLASH_INTEL_TE28F160:
		m_bits = 16;
		m_size = 0x200000;
		m_maker_id = MFG_SHARP;
		m_device_id = 0xd0;
		map = ADDRESS_MAP_NAME( memory_map16_16Mb );
		break;
	case FLASH_SHARP_UNK128MBIT:
		m_bits = 16;
		m_size = 0x800000;
		m_maker_id = MFG_SHARP;
		m_device_id = 0xb0;
		map = ADDRESS_MAP_NAME( memory_map16_64Mb );
		break;
	case FLASH_MACRONIX_29L001MC:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = MFG_MACRONIX;
		m_device_id = 0x51;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_MACRONIX_29LV160TMC:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = MFG_MACRONIX;
		m_device_id = 0x49;
		m_sector_is_16k = true;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_PANASONIC_MN63F805MNP:
		m_bits = 8;
		m_size = 0x10000;
		m_maker_id = MFG_PANASONIC;
		m_device_id = 0x1b;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map8_512Kb );
		break;
	case FLASH_SANYO_LE26FV10N1TS:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = MFG_SANYO;
		m_device_id = 0x13;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_SST_28SF040:
		m_bits = 8;
		m_size = 0x80000;
		m_maker_id = MFG_SST;
		m_device_id = 0x04;
		map = ADDRESS_MAP_NAME( memory_map8_4Mb );
		break;
	case FLASH_TMS_29F040:
		m_bits = 8;
		m_addrmask = 0x7fff;
		m_size = 0x80000;
		m_maker_id = MFG_AMD;
		m_device_id = 0xa4;
		map = ADDRESS_MAP_NAME( memory_map8_4Mb );
		break;
	}

	int addrbits;
	for (addrbits = 24; addrbits > 0; addrbits--)
		if ((m_size & (1 << addrbits)) != 0)
			break;

	m_space_config = address_space_config("flash", ENDIANNESS_BIG, m_bits, addrbits, (m_bits == 8) ? 0 : -1, map);
}

intelfsh8_device::intelfsh8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: intelfsh_device(mconfig, type, name, tag, owner, clock, variant, shortname, source) { }

intelfsh16_device::intelfsh16_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: intelfsh_device(mconfig, type, name, tag, owner, clock, variant, shortname, source) { }


intel_28f016s5_device::intel_28f016s5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, INTEL_28F016S5, "Intel 28F016S5 Flash", tag, owner, clock, FLASH_INTEL_28F016S5, "intel_28f016s5", __FILE__) { }

fujitsu_29f160t_device::fujitsu_29f160t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, FUJITSU_29F160T, "Fujitsu 29F160 Flash", tag, owner, clock, FLASH_FUJITSU_29F160T, "fujitsu_29f160t", __FILE__) { }

fujitsu_29f016a_device::fujitsu_29f016a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, FUJITSU_29F016A, "Fujitsu 29F016A Flash", tag, owner, clock, FLASH_FUJITSU_29F016A, "fujitsu_29f016a", __FILE__) { }

fujitsu_29dl16x_device::fujitsu_29dl16x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, FUJITSU_29DL16X, "Fujitsu 29DL16X Flash", tag, owner, clock, FLASH_FUJITSU_29DL16X, "fujitsu_29dl16x", __FILE__) { }

sharp_lh28f016s_device::sharp_lh28f016s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, SHARP_LH28F016S, "Sharp LH28F016S Flash", tag, owner, clock, FLASH_SHARP_LH28F016S, "sharp_lh28f016s", __FILE__) { }

atmel_29c010_device::atmel_29c010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, ATMEL_29C010, "Atmel 29C010 Flash", tag, owner, clock, FLASH_ATMEL_29C010, "atmel_29c010", __FILE__) { }

amd_29f010_device::amd_29f010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29F010, "AMD 29F010 Flash", tag, owner, clock, FLASH_AMD_29F010, "amd_29f010", __FILE__) { }

amd_29f040_device::amd_29f040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29F040, "AMD 29F040 Flash", tag, owner, clock, FLASH_AMD_29F040, "amd_29f040", __FILE__) { }

amd_29f080_device::amd_29f080_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29F080, "AMD 29F080 Flash", tag, owner, clock, FLASH_AMD_29F080, "amd_29f080", __FILE__) { }

amd_29f400t_device::amd_29f400t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29F400T, "AMD 29F400 Flash", tag, owner, clock, FLASH_AMD_29F400T, "amd_29f400t", __FILE__) { }

amd_29f800t_device::amd_29f800t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29F800T, "AMD 29F800 Flash", tag, owner, clock, FLASH_AMD_29F800T, "amd_29f800t", __FILE__) { }

amd_29lv200t_device::amd_29lv200t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, AMD_29LV200T, "AMD 29LV200T Flash", tag, owner, clock, FLASH_AMD_29LV200T, "amd_29lv200t", __FILE__) { }

intel_e28f008sa_device::intel_e28f008sa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, INTEL_E28F008SA, "Intel E28F008SA Flash", tag, owner, clock, FLASH_INTEL_E28F008SA, "intel_e28f008sa", __FILE__) { }

macronix_29l001mc_device::macronix_29l001mc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, MACRONIX_29L001MC, "Macronix 29L001MC Flash", tag, owner, clock, FLASH_MACRONIX_29L001MC, "macronix_29l001mc", __FILE__) { }

macronix_29lv160tmc_device::macronix_29lv160tmc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, MACRONIX_29LV160TMC, "Macronix 29LV160TMC Flash", tag, owner, clock, FLASH_MACRONIX_29LV160TMC, "macronix_29lv160tmc", __FILE__) { }

panasonic_mn63f805mnp_device::panasonic_mn63f805mnp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, PANASONIC_MN63F805MNP, "Panasonic MN63F805MNP Flash", tag, owner, clock, FLASH_PANASONIC_MN63F805MNP, "panasonic_mn63f805mnp", __FILE__) { }

sanyo_le26fv10n1ts_device::sanyo_le26fv10n1ts_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, SANYO_LE26FV10N1TS, "Sanyo LE26FV10N1TS Flash", tag, owner, clock, FLASH_SANYO_LE26FV10N1TS, "sanyo_le26fv10n1ts", __FILE__) { }

sst_28sf040_device::sst_28sf040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, SST_28SF040, "SST 28SF040 Flash", tag, owner, clock, FLASH_SST_28SF040, "sst_28sf040", __FILE__) { }

sst_39vf020_device::sst_39vf020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, SST_39VF020, "SST 39VF020 Flash", tag, owner, clock, FLASH_SST_39VF020, "sst_39vf020", __FILE__) { }

sharp_lh28f400_device::sharp_lh28f400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, SHARP_LH28F400, "Sharp LH28F400 Flash", tag, owner, clock, FLASH_SHARP_LH28F400, "sharp_lh28f400", __FILE__) { }

intel_te28f160_device::intel_te28f160_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, INTEL_TE28F160, "Intel TE28F160 Flash", tag, owner, clock, FLASH_INTEL_TE28F160, "intel_te28f160", __FILE__) { }

intel_e28f400b_device::intel_e28f400b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, INTEL_E28F400B, "Intel E28F400B Flash", tag, owner, clock, FLASH_INTEL_E28F400B, "intel_e28f400b", __FILE__) { }

sharp_unk128mbit_device::sharp_unk128mbit_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, SHARP_UNK128MBIT, "Sharp Unknown 128Mbit Flash", tag, owner, clock, FLASH_SHARP_UNK128MBIT, "sharp_unk128mbit", __FILE__) { }

intel_28f320j3d_device::intel_28f320j3d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, INTEL_28F320J3D, "Intel 28F320J3D Flash", tag, owner, clock, FLASH_INTEL_28F320J3D, "intel_28f320j3d", __FILE__) { }

intel_28f320j5_device::intel_28f320j5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, INTEL_28F320J5, "Intel 28F320J3D_a Flash", tag, owner, clock, FLASH_INTEL_28F320J5, "intel_28f320j5", __FILE__) { }


sst_39vf400a_device::sst_39vf400a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh16_device(mconfig, SST_39VF400A, "SST 39VF400A Flash", tag, owner, clock, FLASH_SST_39VF400A, "sst_39vf400a", __FILE__) { }


tms_29f040_device::tms_29f040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: intelfsh8_device(mconfig, TMS_29F040, "Texas Instruments 29F040 Flash", tag, owner, clock, FLASH_TMS_29F040, "tms_29f040", __FILE__) { }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intelfsh_device::device_start()
{
	m_timer = timer_alloc();

	save_item( NAME(m_status) );
	save_item( NAME(m_flash_mode) );
	save_item( NAME(m_flash_master_lock) );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void intelfsh_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch( m_flash_mode )
	{
	case FM_READSTATUS:
		m_status = 0x80;
		break;

	case FM_ERASEAMD4:
		m_flash_mode = FM_NORMAL;
		break;
	}
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *intelfsh_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void intelfsh_device::nvram_default()
{
	// region always wins
	if (m_region != NULL)
	{
		UINT32 bytes = m_region->bytes();
		if (bytes > m_size)
			bytes = m_size;

		if (m_bits == 8)
		{
			for (offs_t offs = 0; offs < bytes; offs++)
				m_addrspace[0]->write_byte(offs, m_region->u8(offs));
		}
		else
		{
			for (offs_t offs = 0; offs < bytes; offs += 2)
				m_addrspace[0]->write_word(offs, m_region->u16(offs / 2));
		}
		return;
	}

	// otherwise, default to 0xff
	for (offs_t offs = 0; offs < m_size; offs++)
		m_addrspace[0]->write_byte(offs, 0xff);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_read(emu_file &file)
{
	dynamic_buffer buffer(m_size);
	file.read(&buffer[0], m_size);
	for (int byte = 0; byte < m_size; byte++)
		m_addrspace[0]->write_byte(byte, buffer[byte]);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_write(emu_file &file)
{
	dynamic_buffer buffer(m_size);
	for (int byte = 0; byte < m_size; byte++)
		buffer[byte] = m_addrspace[0]->read_byte(byte);
	file.write(&buffer[0], m_size);
}


//-------------------------------------------------
//  read_full - generic read, called by the
//  bit-width-specific readers
//-------------------------------------------------

UINT32 intelfsh_device::read_full(UINT32 address)
{
	UINT32 data = 0;
	address += m_bank << 16;
	switch( m_flash_mode )
	{
	default:
	case FM_NORMAL:
		switch( m_bits )
		{
		case 8:
			{
				data = m_addrspace[0]->read_byte(address);
			}
			break;
		case 16:
			{
				data = m_addrspace[0]->read_word(address * 2);
			}
			break;
		}
		break;
	case FM_READSTATUS:
		data = m_status;
		break;
	case FM_READAMDID3:
		if ((m_maker_id == MFG_FUJITSU && m_device_id == 0x35) || (m_maker_id == MFG_AMD && m_device_id == 0x3b))
		{
			// used in Fujitsu 29DL16X 8bits mode
			// used in AMD 29LV200 8bits mode
			switch (address)
			{
				case 0: data = m_maker_id; break;
				case 2: data = m_device_id; break;
				case 4: data = 0; break;
			}
		}
		else
		{
			switch (address)
			{
				case 0: data = m_maker_id; break;
				case 1: data = m_device_id; break;
				case 2: data = 0; break;
			}
		}
		break;
	case FM_READID:
		if (m_maker_id == MFG_INTEL && m_device_id == 0x16)
		{
			switch (address)
			{
				case 0: data = m_maker_id; break;
				case 2: data = m_device_id; break;
				case 4: data = 0; break;
			}
		}
		else
		{
			switch (address)
			{
			case 0: // maker ID
				data = m_maker_id;
				break;
			case 1: // chip ID
				data = m_device_id;
				break;
			case 2: // block lock config
				data = 0; // we don't support this yet
				break;
			case 3: // master lock config
				if (m_flash_master_lock)
				{
					data = 1;
				}
				else
				{
					data = 0;
				}
				break;
			}
		}
		break;
	case FM_ERASEAMD4:
		// reads outside of the erasing sector return normal data
		if ((address < m_erase_sector) || (address >= m_erase_sector+(64*1024)))
		{
			switch( m_bits )
			{
			case 8:
				{
					data = m_addrspace[0]->read_byte(address);
				}
				break;
			case 16:
				{
					data = m_addrspace[0]->read_word(address * 2);
				}
				break;
			}
		}
		else
		{
			m_status ^= ( 1 << 6 ) | ( 1 << 2 );
			data = m_status;
		}
		break;
	}

	//logerror( "intelflash_read( %08x ) %08x\n", address, data );

	return data;
}


//-------------------------------------------------
//  write_full - generic write, called by the
//  bit-width-specific writers
//-------------------------------------------------

void intelfsh_device::write_full(UINT32 address, UINT32 data)
{
	//logerror( "intelflash_write( %u : %08x, %08x )\n", m_flash_mode, address, data );

	address += m_bank << 16;

	switch( m_flash_mode )
	{
	case FM_NORMAL:
	case FM_READSTATUS:
	case FM_READID:
	case FM_READAMDID3:
		switch( data & 0xff )
		{
		case 0xf0:
		case 0xff:  // reset chip mode
			m_flash_mode = FM_NORMAL;
			break;
		case 0x90:  // read ID
			m_flash_mode = FM_READID;
			break;
		case 0x40:
		case 0x10:  // program
			m_flash_mode = FM_WRITEPART1;
			break;
		case 0x50:  // clear status reg
			m_status = 0x80;
			m_flash_mode = FM_READSTATUS;
			break;
		case 0x20:  // block erase
			m_flash_mode = FM_CLEARPART1;
			break;
		case 0x60:  // set master lock
			m_flash_mode = FM_SETMASTER;
			break;
		case 0x70:  // read status
			m_flash_mode = FM_READSTATUS;
			break;
		case 0xaa:  // AMD ID select part 1
			if( ( address & 0xfff ) == 0x555 )
			{
				m_flash_mode = FM_READAMDID1;
			}
			else if( ( address & 0xfff ) == 0xaaa )
			{
				m_flash_mode = FM_READAMDID1;
			}
			break;
		default:
			logerror( "Unknown flash mode byte %x\n", data & 0xff );
			break;
		}
		break;
	case FM_READAMDID1:
		if( ( address & 0xffff ) == 0x2aa && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_READAMDID2;
		}
		else if( ( address & 0xffff ) == 0x2aaa && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_READAMDID2;
		}
		else if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_READAMDID2;
		}
		// for AMD 29F080 address bits A11-A19 don't care, for TMS 29F040 address bits A15-A18 don't care
		else if( ( address & m_addrmask ) == ( 0xaaaa & m_addrmask ) && ( data & 0xff ) == 0x55 && m_addrmask )
		{
			m_flash_mode = FM_READAMDID2;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_READAMDID1\n", address, data & 0xff );
			m_flash_mode = FM_NORMAL;
		}
		break;
	case FM_READAMDID2:
		if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0x90 )
		{
			m_flash_mode = FM_READAMDID3;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0x90 )
		{
			m_flash_mode = FM_READAMDID3;
		}
		else if( ( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0x90 )
		{
			m_flash_mode = FM_READAMDID3;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0x80 )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0x80 )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0x80 )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0xa0 )
		{
			m_flash_mode = FM_BYTEPROGRAM;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xa0 )
		{
			if (m_type == FLASH_ATMEL_29C010)
			{
				m_flash_mode = FM_WRITEPAGEATMEL;
				m_byte_count = 0;
			}
			else
			{
				m_flash_mode = FM_BYTEPROGRAM;
			}
		}
		else if( ( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0xa0 )
		{
			m_flash_mode = FM_BYTEPROGRAM;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0xf0 )
		{
			m_flash_mode = FM_NORMAL;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xf0 )
		{
			m_flash_mode = FM_NORMAL;
		}
		else if( ( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0xf0 )
		{
			m_flash_mode = FM_NORMAL;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xb0 && m_maker_id == 0x62 && m_device_id == 0x13 )
		{
			m_flash_mode = FM_BANKSELECT;
		}

		// for AMD 29F080 address bits A11-A19 don't care, for TMS 29F040 address bits A15-A18 don't care
		else if(( address & m_addrmask ) == ( 0x5555 & m_addrmask ) && ( data & 0xff ) == 0x80 && m_addrmask )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if(( address & m_addrmask ) == ( 0x5555 & m_addrmask ) && ( data & 0xff ) == 0x90 && m_addrmask )
		{
			m_flash_mode = FM_READAMDID3;
		}
		else if(( address & m_addrmask ) == ( 0x5555 & m_addrmask ) && ( data & 0xff ) == 0xa0 && m_addrmask )
		{
			m_flash_mode = FM_BYTEPROGRAM;
		}
		else if(( address & m_addrmask ) == ( 0x5555 & m_addrmask ) && ( data & 0xff ) == 0xf0 && m_addrmask )
		{
			m_flash_mode = FM_NORMAL;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_READAMDID2\n", address, data & 0xff );
			m_flash_mode = FM_NORMAL;
		}
		break;
	case FM_ERASEAMD1:
		if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0xaa )
		{
			m_flash_mode = FM_ERASEAMD2;
		}
		else if( ( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0xaa )
		{
			m_flash_mode = FM_ERASEAMD2;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD1\n", address, data & 0xff );
		}
		break;
	case FM_ERASEAMD2:
		if( ( address & 0xffff ) == 0x2aa && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_ERASEAMD3;
		}
		else if( ( address & 0xffff ) == 0x2aaa && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_ERASEAMD3;
		}
		else if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0x55 )
		{
			m_flash_mode = FM_ERASEAMD3;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD2\n", address, data & 0xff );
		}
		break;
	case FM_ERASEAMD3:
		if( (( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0x10 ) ||
			(( address & 0xfff ) == 0xaaa && ( data & 0xff ) == 0x10 ) )
		{
			// chip erase
			for (offs_t offs = 0; offs < m_size; offs++)
				m_addrspace[0]->write_byte(offs, 0xff);

			m_status = 1 << 3;
			m_flash_mode = FM_ERASEAMD4;

			if (m_sector_is_4k)
			{
				m_timer->adjust( attotime::from_seconds( 1 ) );
			}
			else if(m_sector_is_16k)
			{
				m_timer->adjust( attotime::from_seconds( 4 ) );
			}
			else
			{
				m_timer->adjust( attotime::from_seconds( 16 ) );
			}
		}
		else if( ( data & 0xff ) == 0x30 )
		{
			// sector erase
			// clear the 4k/64k block containing the current address to all 0xffs
			UINT32 base = address * ((m_bits == 16) ? 2 : 1);
			if (m_sector_is_4k)
			{
				for (offs_t offs = 0; offs < 4 * 1024; offs++)
					m_addrspace[0]->write_byte((base & ~0xfff) + offs, 0xff);
				m_erase_sector = address & ((m_bits == 16) ? ~0x7ff : ~0xfff);
				m_timer->adjust( attotime::from_msec( 125 ) );
			}
			else if(m_sector_is_16k)
			{
				for (offs_t offs = 0; offs < 16 * 1024; offs++)
					m_addrspace[0]->write_byte((base & ~0x3fff) + offs, 0xff);
				m_erase_sector = address & ((m_bits == 16) ? ~0x1fff : ~0x3fff);
				m_timer->adjust( attotime::from_msec( 500 ) );
			}
			else if(m_top_boot_sector && address >= (m_size - 64*1024))
			{
				if (address >= (m_size - (16*1024)))
				{
					for (offs_t offs = 0; offs < 16 * 1024; offs++)
						m_addrspace[0]->write_byte((base & ~0x3fff) + offs, 0xff);
					m_erase_sector = address & ((m_bits == 16) ? ~0x1fff : ~0x3fff);
					m_timer->adjust( attotime::from_msec( 500 ) );
				}
				else if (address >= (m_size - (32*1024)))
				{
					for (offs_t offs = 0; offs < 8 * 1024; offs++)
						m_addrspace[0]->write_byte((base & ~0x1fff) + offs, 0xff);
					m_erase_sector = address & ((m_bits == 16) ? ~0xfff : ~0x1fff);
					m_timer->adjust( attotime::from_msec( 250 ) );
				}
				else
				{
					for (offs_t offs = 0; offs < 32 * 1024; offs++)
						m_addrspace[0]->write_byte((base & ~0x7fff) + offs, 0xff);
					m_erase_sector = address & ((m_bits == 16) ? ~0x3fff : ~0x7fff);
					m_timer->adjust( attotime::from_msec( 500 ) );
				}
			}
			else
			{
				for (offs_t offs = 0; offs < 64 * 1024; offs++)
					m_addrspace[0]->write_byte((base & ~0xffff) + offs, 0xff);
				m_erase_sector = address & ((m_bits == 16) ? ~0x7fff : ~0xffff);
				m_timer->adjust( attotime::from_seconds( 1 ) );
			}

			m_status = 1 << 3;
			m_flash_mode = FM_ERASEAMD4;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD3\n", address, data & 0xff );
		}
		break;
	case FM_BYTEPROGRAM:
		switch( m_bits )
		{
		case 8:
			{
				m_addrspace[0]->write_byte(address, data);
			}
			break;
		default:
			logerror( "FM_BYTEPROGRAM not supported when m_bits == %d\n", m_bits );
			break;
		}
		m_flash_mode = FM_NORMAL;
		break;
	case FM_WRITEPART1:
		switch( m_bits )
		{
		case 8:
			{
				m_addrspace[0]->write_byte(address, data);
			}
			break;
		case 16:
			{
				m_addrspace[0]->write_word(address * 2, data);
			}
			break;
		default:
			logerror( "FM_WRITEPART1 not supported when m_bits == %d\n", m_bits );
			break;
		}
		m_status = 0x80;
		if (m_type == FLASH_SST_28SF040)
			m_flash_mode = FM_NORMAL;
		else
			m_flash_mode = FM_READSTATUS;
		break;
	case FM_WRITEPAGEATMEL:
		switch( m_bits )
		{
		case 8:
			{
				m_addrspace[0]->write_byte(address, data);
			}
			break;
		case 16:
			{
				m_addrspace[0]->write_word(address * 2, data);
			}
			break;
		default:
			logerror( "FM_WRITEPAGEATMEL not supported when m_bits == %d\n", m_bits );
			break;
		}

		m_byte_count++;

		if (m_byte_count == m_page_size)
		{
			m_flash_mode = FM_NORMAL;
		}
		break;
	case FM_CLEARPART1:
		if( ( data & 0xff ) == 0xd0 )
		{
			if (m_type == FLASH_SST_28SF040)
			{
				// clear the 256 bytes block containing the current address to all 0xffs
				UINT32 base = address * ((m_bits == 16) ? 2 : 1);
				for (offs_t offs = 0; offs < 256; offs++)
					m_addrspace[0]->write_byte((base & ~0xff) + offs, 0xff);

				m_timer->adjust( attotime::from_msec( 4 ) );
			}
			else if (m_type == FLASH_INTEL_E28F400B)
			{
				// 00000-03fff -  16KB boot block (may be write protected via external pins)
				// 04000-05fff -   8KB parameter block
				// 06000-07fff -   8KB parameter block
				// 08000-1ffff -  96KB main block
				// 20000-3ffff - 128KB main block
				// 40000-5ffff - 128KB main block
				// 60000-7ffff - 128KB main block
				// erase duration is 0.3s for boot and parameter blocks, and 0.6s for main blocks
				UINT32 base = (address & 0x3ffff) * 2;
				int size, duration;
				if (base < 0x4000)
				{
					base = 0;
					size = 0x4000;
					duration = 300;
				}
				else if (base < 0x8000)
				{
					base &= 0x6000;
					size = 0x2000;
					duration = 300;
				}
				else if (base < 0x20000)
				{
					base = 0x8000;
					size = 0x18000;
					duration = 600;
				}
				else
				{
					base &= 0x60000;
					size = 0x20000;
					duration = 600;
				}

				// clear the block containing the current address to all 0xffffs
				for (offs_t offs = 0; offs < size / 2; offs += 2)
					m_addrspace[0]->write_word(base | offs, 0xffff);

				m_timer->adjust( attotime::from_msec( duration ) );
			}
			else
			{
				// clear the 64k block containing the current address to all 0xffs
				UINT32 base = address * ((m_bits == 16) ? 2 : 1);
				for (offs_t offs = 0; offs < 64 * 1024; offs++)
					m_addrspace[0]->write_byte((base & ~0xffff) + offs, 0xff);

				m_timer->adjust( attotime::from_seconds( 1 ) );
			}

			m_status = 0x00;
			m_flash_mode = FM_READSTATUS;
			break;
		}
		else
		{
			logerror( "unexpected %02x in FM_CLEARPART1\n", data & 0xff );
		}
		break;
	case FM_SETMASTER:
		switch( data & 0xff )
		{
		case 0xf1:
			m_flash_master_lock = true;
			break;
		case 0xd0:
			m_flash_master_lock = false;
			break;
		default:
			logerror( "unexpected %08x=%02x in FM_SETMASTER:\n", address, data & 0xff );
			break;
		}
		m_flash_mode = FM_NORMAL;
		break;
	case FM_BANKSELECT:
		m_bank = data & 0xff;
		m_flash_mode = FM_NORMAL;
		break;
	}
}
