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
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  intelfsh_device_config - constructor
//-------------------------------------------------

intelfsh_device_config::intelfsh_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: device_config(mconfig, type, name, tag, owner, clock),
	  device_config_nvram_interface(mconfig, *this),
	  m_type(variant)
{
}

intelfsh8_device_config::intelfsh8_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: intelfsh_device_config(mconfig, type, name, tag, owner, clock, variant)
{
}

intelfsh16_device_config::intelfsh16_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant)
	: intelfsh_device_config(mconfig, type, name, tag, owner, clock, variant)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intelfsh_device - constructor
//-------------------------------------------------

intelfsh_device::intelfsh_device(running_machine &_machine, const intelfsh_device_config &config)
	: device_t(_machine, config),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_size(0),
	  m_bits(config.m_type >> 8),
	  m_status(0x80),
	  m_erase_sector(0),
	  m_sector_is_4k(false),
	  m_flash_mode(FM_NORMAL),
	  m_flash_master_lock(false),
	  m_device_id(0),
	  m_maker_id(0),
	  m_timer(NULL),
	  m_flash_memory(NULL)
{
	switch( config.m_type )
	{
	case intelfsh_device_config::FLASH_INTEL_28F016S5:
	case intelfsh_device_config::FLASH_SHARP_LH28F016S:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = 0x89;
		m_device_id = 0xaa;
		break;
	case intelfsh_device_config::FLASH_SHARP_LH28F400:
	case intelfsh_device_config::FLASH_INTEL_E28F400:
		m_bits = 16;
		m_size = 0x80000;
		m_maker_id = 0xb0;
		m_device_id = 0xed;
		break;
	case intelfsh_device_config::FLASH_FUJITSU_29F016A:
		m_bits = 8;
		m_size = 0x200000;
		m_maker_id = 0x04;
		m_device_id = 0xad;
		break;
	case intelfsh_device_config::FLASH_INTEL_E28F008SA:
		m_bits = 8;
		m_size = 0x100000;
		m_maker_id = 0x89;
		m_device_id = 0xa2;
		break;
	case intelfsh_device_config::FLASH_INTEL_TE28F160:
		m_bits = 16;
		m_size = 0x200000;
		m_maker_id = 0xb0;
		m_device_id = 0xd0;
		break;
	case intelfsh_device_config::FLASH_SHARP_UNK128MBIT:
		m_bits = 16;
		m_size = 0x800000;
		m_maker_id = 0xb0;
		m_device_id = 0xb0;
		break;
	case intelfsh_device_config::FLASH_MACRONIX_29L001MC:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = 0xc2;
		m_device_id = 0x51;
		break;

	case intelfsh_device_config::FLASH_PANASONIC_MN63F805MNP:
		m_bits = 8;
		m_size = 0x10000;
		m_maker_id = 0x32;
		m_device_id = 0x1b;
		m_sector_is_4k = true;
		break;

	case intelfsh_device_config::FLASH_SANYO_LE26FV10N1TS:
		m_bits = 8;
		m_size = 0x20000;
		m_maker_id = 0x62;
		m_device_id = 0x13;
		m_sector_is_4k = true;
		break;
	}
	m_flash_memory = auto_alloc_array( &m_machine, UINT8, m_size );
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
	state_save_register_memory( machine, name(), tag(), 0, "m_flash_memory", m_flash_memory, m_bits/8, m_size / (m_bits/8), __FILE__, __LINE__ );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void intelfsh_device::device_timer(emu_timer &timer, int param, void *ptr)
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
		if (bytes > m_size)
			bytes = m_size;
		memcpy(m_flash_memory, *m_region, bytes);
		return;
	}

	memset( m_flash_memory, 0xff, m_size );
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_read(mame_file &file)
{
	mame_fread(&file, m_flash_memory, m_size);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void intelfsh_device::nvram_write(mame_file &file)
{
	mame_fwrite(&file, m_flash_memory, m_size);
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
		switch( m_bits )
		{
		case 8:
			{
				UINT8 *flash_memory = (UINT8 *)m_flash_memory;
				data = flash_memory[ address ];
			}
			break;
		case 16:
			{
				UINT16 *flash_memory = (UINT16 *)m_flash_memory;
				data = flash_memory[ address ];
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
			case 0:	data = m_maker_id; break;
			case 1: data = m_device_id; break;
			case 2: data = 0; break;
		}
		break;
	case FM_READID:
		switch (address)
		{
		case 0:	// maker ID
			data = m_maker_id;
			break;
		case 1:	// chip ID
			data = m_device_id;
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
			switch( m_bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)m_flash_memory;
					data = flash_memory[ address ];
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)m_flash_memory;
					data = flash_memory[ address ];
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
			memset( m_flash_memory, 0xff, m_size);

			m_status = 1 << 3;
			m_flash_mode = FM_ERASEAMD4;

			timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 17 ), 0 );
		}
		else if( ( data & 0xff ) == 0x30 )
		{
			// sector erase
			// clear the 4k/64k block containing the current address to all 0xffs
			switch( m_bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)m_flash_memory;
					if (m_sector_is_4k)
					{
						memset( &flash_memory[ address & ~0xfff ], 0xff, 4 * 1024 );
						m_erase_sector = address & ~0xfff;
						timer_adjust_oneshot( m_timer, ATTOTIME_IN_MSEC( 125 ), 0 );
					}
					else
					{
						memset( &flash_memory[ address & ~0xffff ], 0xff, 64 * 1024 );
						m_erase_sector = address & ~0xffff;
						timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 1 ), 0 );
					}
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)m_flash_memory;
					if (m_sector_is_4k)
					{
						memset( &flash_memory[ address & ~0x7ff ], 0xff, 4 * 1024 );
						m_erase_sector = address & ~0x7ff;
						timer_adjust_oneshot( m_timer, ATTOTIME_IN_MSEC( 125 ), 0 );
					}
					else
					{
						memset( &flash_memory[ address & ~0x7fff ], 0xff, 64 * 1024 );
						m_erase_sector = address & ~0x7fff;
						timer_adjust_oneshot( m_timer, ATTOTIME_IN_SEC( 1 ), 0 );
					}
				}
				break;
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
				UINT8 *flash_memory = (UINT8 *)m_flash_memory;
				flash_memory[ address ] = data;
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
				UINT8 *flash_memory = (UINT8 *)m_flash_memory;
				flash_memory[ address ] = data;
			}
			break;
		case 16:
			{
				UINT16 *flash_memory = (UINT16 *)m_flash_memory;
				flash_memory[ address ] = data;
			}
			break;
		default:
			logerror( "FM_WRITEPART1 not supported when m_bits == %d\n", m_bits );
			break;
		}
		m_status = 0x80;
		m_flash_mode = FM_READSTATUS;
		break;
	case FM_CLEARPART1:
		if( ( data & 0xff ) == 0xd0 )
		{
			// clear the 64k block containing the current address to all 0xffs
			switch( m_bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)m_flash_memory;
					memset( &flash_memory[ address & ~0xffff ], 0xff, 64 * 1024 );
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)m_flash_memory;
					memset( &flash_memory[ address & ~0x7fff ], 0xff, 64 * 1024 );
				}
				break;
			default:
				logerror( "FM_CLEARPART1 not supported when m_bits == %d\n", m_bits );
				break;
			}
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
