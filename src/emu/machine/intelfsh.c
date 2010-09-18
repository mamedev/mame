/*
    Flash ROM emulation

    Explicitly supports:
    Intel 28F016S5 (byte-wide)
    AMD/Fujitsu 29F016 (byte-wide)
    Sharp LH28F400 (word-wide)

    Flash ROMs use a standardized command set accross manufacturers,
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
	FM_NORMAL,	// normal read/write
	FM_READID,	// read ID
	FM_READSTATUS,	// read status
	FM_WRITEPART1,	// first half of programming, awaiting second
	FM_CLEARPART1,	// first half of clear, awaiting second
	FM_SETMASTER,	// first half of set master lock, awaiting on/off
	FM_READAMDID1,	// part 1 of alt ID sequence
	FM_READAMDID2,	// part 2 of alt ID sequence
	FM_READAMDID3,	// part 3 of alt ID sequence
	FM_ERASEAMD1,	// part 1 of AMD erase sequence
	FM_ERASEAMD2,	// part 2 of AMD erase sequence
	FM_ERASEAMD3,	// part 3 of AMD erase sequence
	FM_ERASEAMD4,	// part 4 of AMD erase sequence
	FM_BYTEPROGRAM,
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// 8-bit variants
DEFINE_TRIVIAL_DERIVED_DEVICE(intel_28f016s5_device_config, intelfsh8_device_config, intel_28f016s5_device, intelfsh8_device, "Sharp LH28F400 Flash", intelfsh_device_config::FLASH_SHARP_LH28F400)
DEFINE_TRIVIAL_DERIVED_DEVICE(fujitsu_29f016a_device_config, intelfsh8_device_config, fujitsu_29f016a_device, intelfsh8_device, "Fujitsu 29F016A Flash", intelfsh_device_config::FLASH_FUJITSU_29F016A)
DEFINE_TRIVIAL_DERIVED_DEVICE(sharp_lh28f016s_device_config, intelfsh8_device_config, sharp_lh28f016s_device, intelfsh8_device, "Sharp LH28F016S Flash", intelfsh_device_config::FLASH_SHARP_LH28F016S)
DEFINE_TRIVIAL_DERIVED_DEVICE(intel_e28f008sa_device_config, intelfsh8_device_config, intel_e28f008sa_device, intelfsh8_device, "Intel E28F008SA Flash", intelfsh_device_config::FLASH_INTEL_E28F008SA)
DEFINE_TRIVIAL_DERIVED_DEVICE(macronix_29l001mc_device_config, intelfsh8_device_config, macronix_29l001mc_device, intelfsh8_device, "Macronix 29L001MC Flash", intelfsh_device_config::FLASH_MACRONIX_29L001MC)
DEFINE_TRIVIAL_DERIVED_DEVICE(panasonic_mn63f805mnp_device_config, intelfsh8_device_config, panasonic_mn63f805mnp_device, intelfsh8_device, "Panasonic MN63F805MNP Flash", intelfsh_device_config::FLASH_PANASONIC_MN63F805MNP)
DEFINE_TRIVIAL_DERIVED_DEVICE(sanyo_le26fv10n1ts_device_config, intelfsh8_device_config, sanyo_le26fv10n1ts_device, intelfsh8_device, "Sanyo LE26FV10N1TS Flash", intelfsh_device_config::FLASH_SANYO_LE26FV10N1TS)

const device_type INTEL_28F016S5 = intel_28f016s5_device_config::static_alloc_device_config;
const device_type SHARP_LH28F016S = sharp_lh28f016s_device_config::static_alloc_device_config;
const device_type FUJITSU_29F016A = fujitsu_29f016a_device_config::static_alloc_device_config;
const device_type INTEL_E28F008SA = intel_e28f008sa_device_config::static_alloc_device_config;
const device_type MACRONIX_29L001MC = macronix_29l001mc_device_config::static_alloc_device_config;
const device_type PANASONIC_MN63F805MNP = panasonic_mn63f805mnp_device_config::static_alloc_device_config;
const device_type SANYO_LE26FV10N1TS = sanyo_le26fv10n1ts_device_config::static_alloc_device_config;


// 16-bit variants
DEFINE_TRIVIAL_DERIVED_DEVICE(sharp_lh28f400_device_config, intelfsh16_device_config, sharp_lh28f400_device, intelfsh16_device, "Sharp LH28F400 Flash", intelfsh_device_config::FLASH_SHARP_LH28F400)
DEFINE_TRIVIAL_DERIVED_DEVICE(intel_te28f160_device_config, intelfsh16_device_config, intel_te28f160_device, intelfsh16_device, "Intel TE28F160 Flash", intelfsh_device_config::FLASH_INTEL_TE28F160)
DEFINE_TRIVIAL_DERIVED_DEVICE(intel_e28f400_device_config, intelfsh16_device_config, intel_e28f400_device, intelfsh16_device, "Intel E28F400 Flash", intelfsh_device_config::FLASH_INTEL_E28F400)
DEFINE_TRIVIAL_DERIVED_DEVICE(sharp_unk128mbit_device_config, intelfsh16_device_config, sharp_unk128mbit_device, intelfsh16_device, "Sharp Unknown 128Mb Flash", intelfsh_device_config::FLASH_SHARP_UNK128MBIT)

const device_type SHARP_LH28F400 = sharp_lh28f400_device_config::static_alloc_device_config;
const device_type INTEL_TE28F160 = intel_te28f160_device_config::static_alloc_device_config;
const device_type INTEL_E28F400 = intel_e28f400_device_config::static_alloc_device_config;
const device_type SHARP_UNK128MBIT = sharp_unk128mbit_device_config::static_alloc_device_config;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static ADDRESS_MAP_START( memory_map8_512Kb, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x00ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_1Mb, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x01ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_8Mb, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map8_16Mb, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x1fffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( memory_map16_4Mb, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x07ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map16_16Mb, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x1fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memory_map16_64Mb, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x7fffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  intelfsh_device_config - constructor
//-------------------------------------------------

intelfsh_device_config::intelfsh_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: device_config(mconfig, type, name, tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  device_config_nvram_interface(mconfig, *this),
	  m_type(variant),
	  m_size(0),
	  m_bits(8),
	  m_device_id(0),
	  m_maker_id(0),
	  m_sector_is_4k(false)
{
	address_map_constructor map = NULL;

	switch( variant )
	{
	case FLASH_INTEL_28F016S5:
	case FLASH_SHARP_LH28F016S:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = 0x89;
		m_device_id = 0xaa;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_SHARP_LH28F400:
	case FLASH_INTEL_E28F400:
		m_bits = 16;
		m_size = 0x80000;
		m_maker_id = 0xb0;
		m_device_id = 0xed;
		map = ADDRESS_MAP_NAME( memory_map16_4Mb );
		break;
	case FLASH_FUJITSU_29F016A:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = 0x04;
		m_device_id = 0xad;
		map = ADDRESS_MAP_NAME( memory_map8_16Mb );
		break;
	case FLASH_INTEL_E28F008SA:
		m_bits = 8;
		m_size = 0x100000;
		m_maker_id = 0x89;
		m_device_id = 0xa2;
		map = ADDRESS_MAP_NAME( memory_map8_8Mb );
		break;
	case FLASH_INTEL_TE28F160:
		m_bits = 16;
		m_size = 0x200000;
		m_maker_id = 0xb0;
		m_device_id = 0xd0;
		map = ADDRESS_MAP_NAME( memory_map16_16Mb );
		break;
	case FLASH_SHARP_UNK128MBIT:
		m_bits = 16;
		m_size = 0x800000;
		m_maker_id = 0xb0;
		m_device_id = 0xb0;
		map = ADDRESS_MAP_NAME( memory_map16_64Mb );
		break;
	case FLASH_MACRONIX_29L001MC:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = 0xc2;
		m_device_id = 0x51;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	case FLASH_PANASONIC_MN63F805MNP:
		m_bits = 8;
		m_size = 0x10000;
		m_maker_id = 0x32;
		m_device_id = 0x1b;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map8_512Kb );
		break;
	case FLASH_SANYO_LE26FV10N1TS:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = 0x62;
		m_device_id = 0x13;
		m_sector_is_4k = true;
		map = ADDRESS_MAP_NAME( memory_map8_1Mb );
		break;
	}
	
	int addrbits;
	for (addrbits = 24; addrbits > 0; addrbits--)
		if ((m_size & (1 << addrbits)) != 0)
			break;

	m_space_config = address_space_config("flash", ENDIANNESS_BIG, m_bits, addrbits, 0, map);
}

intelfsh8_device_config::intelfsh8_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: intelfsh_device_config(mconfig, type, name, tag, owner, clock, variant)
{
}

intelfsh16_device_config::intelfsh16_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: intelfsh_device_config(mconfig, type, name, tag, owner, clock, variant)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *intelfsh_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intelfsh_device - constructor
//-------------------------------------------------

intelfsh_device::intelfsh_device(running_machine &_machine, const intelfsh_device_config &config)
	: device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_status(0x80),
	  m_erase_sector(0),
	  m_flash_mode(FM_NORMAL),
	  m_flash_master_lock(false),
	  m_timer(NULL)
{
}

intelfsh8_device::intelfsh8_device(running_machine &_machine, const intelfsh_device_config &config)
	: intelfsh_device(_machine, config) { }

intelfsh16_device::intelfsh16_device(running_machine &_machine, const intelfsh_device_config &config)
	: intelfsh_device(_machine, config) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intelfsh_device::device_start()
{
	m_timer = device_timer_alloc(*this);

	state_save_register_device_item( this, 0, m_status );
	state_save_register_device_item( this, 0, m_flash_mode );
	state_save_register_device_item( this, 0, m_flash_master_lock );
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
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void intelfsh_device::nvram_default()
{
	// region always wins
	if (m_region != NULL)
	{
		UINT32 bytes = m_region->bytes();
		if (bytes > m_config.m_size)
			bytes = m_config.m_size;

		if (m_config.m_bits == 8)
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
	for (offs_t offs = 0; offs < m_config.m_size; offs++)
		m_addrspace[0]->write_byte(offs, 0xff);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_read(mame_file &file)
{
	UINT8 *buffer = global_alloc_array(UINT8, m_config.m_size);
	mame_fread(&file, buffer, m_config.m_size);
	for (int byte = 0; byte < m_config.m_size; byte++)
		m_addrspace[0]->write_byte(byte, buffer[byte]);
	global_free(buffer);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_write(mame_file &file)
{
	UINT8 *buffer = global_alloc_array(UINT8, m_config.m_size);
	for (int byte = 0; byte < m_config.m_size; byte++)
		buffer[byte] = m_addrspace[0]->read_byte(byte);
	mame_fwrite(&file, buffer, m_config.m_size);
	global_free(buffer);
}


//-------------------------------------------------
//  read_full - generic read, called by the
//  bit-width-specific readers
//-------------------------------------------------

UINT32 intelfsh_device::read_full(UINT32 address)
{
	UINT32 data = 0;
	switch( m_flash_mode )
	{
	default:
	case FM_NORMAL:
		switch( m_config.m_bits )
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
		switch (address)
		{
			case 0:	data = m_config.m_maker_id; break;
			case 1: data = m_config.m_device_id; break;
			case 2: data = 0; break;
		}
		break;
	case FM_READID:
		switch (address)
		{
		case 0:	// maker ID
			data = m_config.m_maker_id;
			break;
		case 1:	// chip ID
			data = m_config.m_device_id;
			break;
		case 2:	// block lock config
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
		break;
	case FM_ERASEAMD4:
		// reads outside of the erasing sector return normal data
		if ((address < m_erase_sector) || (address >= m_erase_sector+(64*1024)))
		{
			switch( m_config.m_bits )
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

//  logerror( "intelflash_read( %d, %08x ) %08x\n", chip, address, data );

	return data;
}


//-------------------------------------------------
//  write_full - generic write, called by the
//  bit-width-specific writers
//-------------------------------------------------

void intelfsh_device::write_full(UINT32 address, UINT32 data)
{
//  logerror( "intelflash_write( %d, %08x, %08x )\n", chip, address, data );

	switch( m_flash_mode )
	{
	case FM_NORMAL:
	case FM_READSTATUS:
	case FM_READID:
	case FM_READAMDID3:
		switch( data & 0xff )
		{
		case 0xf0:
		case 0xff:	// reset chip mode
			m_flash_mode = FM_NORMAL;
			break;
		case 0x90:	// read ID
			m_flash_mode = FM_READID;
			break;
		case 0x40:
		case 0x10:	// program
			m_flash_mode = FM_WRITEPART1;
			break;
		case 0x50:	// clear status reg
			m_status = 0x80;
			m_flash_mode = FM_READSTATUS;
			break;
		case 0x20:	// block erase
			m_flash_mode = FM_CLEARPART1;
			break;
		case 0x60:	// set master lock
			m_flash_mode = FM_SETMASTER;
			break;
		case 0x70:	// read status
			m_flash_mode = FM_READSTATUS;
			break;
		case 0xaa:	// AMD ID select part 1
			if( ( address & 0xfff ) == 0x555 )
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
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0x80 )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0x80 )
		{
			m_flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0xa0 )
		{
			m_flash_mode = FM_BYTEPROGRAM;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xa0 )
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
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD2\n", address, data & 0xff );
		}
		break;
	case FM_ERASEAMD3:
		if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0x10 )
		{
			// chip erase
			for (offs_t offs = 0; offs < m_config.m_size; offs++)
				m_addrspace[0]->write_byte(offs, 0xff);

			m_status = 1 << 3;
			m_flash_mode = FM_ERASEAMD4;

			if (m_config.m_sector_is_4k)
			{
				timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 1 ), 0 );
			}
			else
			{
				timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 16 ), 0 );
			}
		}
		else if( ( data & 0xff ) == 0x30 )
		{
			// sector erase
			// clear the 4k/64k block containing the current address to all 0xffs
			if (m_config.m_sector_is_4k)
			{
				for (offs_t offs = 0; offs < 4 * 1024; offs++)
					m_addrspace[0]->write_byte((address & ~0xfff) + offs, 0xff);
				m_erase_sector = address & ~0xfff;
				timer_adjust_oneshot( m_timer, ATTOTIME_IN_MSEC( 125 ), 0 );
			}
			else
			{
				for (offs_t offs = 0; offs < 64 * 1024; offs++)
					m_addrspace[0]->write_byte((address & ~0xffff) + offs, 0xff);
				m_erase_sector = address & ~0xffff;
				timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 1 ), 0 );
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
		switch( m_config.m_bits )
		{
		case 8:
			{
				m_addrspace[0]->write_byte(address, data);
			}
			break;
		default:
			logerror( "FM_BYTEPROGRAM not supported when m_bits == %d\n", m_config.m_bits );
			break;
		}
		m_flash_mode = FM_NORMAL;
		break;
	case FM_WRITEPART1:
		switch( m_config.m_bits )
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
			logerror( "FM_WRITEPART1 not supported when m_bits == %d\n", m_config.m_bits );
			break;
		}
		m_status = 0x80;
		m_flash_mode = FM_READSTATUS;
		break;
	case FM_CLEARPART1:
		if( ( data & 0xff ) == 0xd0 )
		{
			// clear the 64k block containing the current address to all 0xffs
			for (offs_t offs = 0; offs < 64 * 1024; offs++)
				m_addrspace[0]->write_byte((address & ~0xffff) + offs, 0xff);

			m_status = 0x00;
			m_flash_mode = FM_READSTATUS;

			timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 1 ), 0 );
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
	}
}
