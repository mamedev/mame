// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    ATMEL AT29 family of Flash EEPROM

    References:
    [1] ATMEL: 4-megabit (512K x 8) 5-volt Only 256-byte sector Flash Memory
    [2] ATMEL: Programming Atmel's AT29 Flash Family


    AT29 family

    Device          Memory      ID      Sectors     Sector Size     Write Cycle Time    Comments
    ------------------------------------------------------------------------------------------
    AT29C256       32K x 8     DC        512        64 bytes            10 ms
    AT29LV256      32K x 8     BC        512        64 bytes            20 ms
    AT29C257       32K x 8     DC        512        64 bytes            10 ms
    AT29C512       64K x 8     5D        512       128 bytes            10 ms
    AT29LV512      64K x 8     3D        512       128 bytes            20 ms
    AT29C010A     128K x 8     D5       1024       128 bytes            10 ms
    AT29LV010A    128K x 8     35       1024       128 bytes            20 ms
    AT29BV010A    128K x 8     35       1024       128 bytes            20 ms
    AT29C1024      64K x 16    25        512       128 words            10 ms
    AT29LV1024     64K x 16    26        512       128 words            20 ms
    AT29C020      256K x 8     DA       1024       256 bytes            10 ms
    AT29LV020     256K x 8     BA       1024       256 bytes            20 ms
    AT29BV020     256K x 8     BA       1024       256 bytes            20 ms
    AT29C040      512K x 8     5B       1024       512 bytes            10 ms        Use AT29C040A for new designs
    AT29LV040     512K x 8     3B       1024       512 bytes            20 ms        Use AT29LV040A for new designs
    AT29BV040     512K x 8     3B       1024       512 bytes            20 ms        Use AT29BV040A for new designs
    AT29C040A     512K x 8     A4       2048       256 bytes            10 ms
    AT29LV040A    512K x 8     C4       2048       256 bytes            20 ms
    AT29BV040A    512K x 8     C4       2048       256 bytes            20 ms

    TODO: Implement remaining variants

    MZ, Aug 2015
*/

#include "emu.h"
#include "at29x.h"

#define LOG_DETAIL      (1U<<1)     // More detail
#define LOG_WARN        (1U<<2)     // Warning
#define LOG_PRG         (1U<<3)     // Programming
#define LOG_READ        (1U<<4)     // Reading
#define LOG_WRITE       (1U<<5)     // Writing
#define LOG_CONFIG      (1U<<6)     // Configuration
#define LOG_STATE       (1U<<7)     // State machine

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

enum
{
	PRGTIMER = 1
};

/*
    Constructor for all variants
*/

at29x_device::at29x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int memory_size, int device_id, int sector_size)
	: device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_memory_size(memory_size),   // bytes
	m_word_width(8),
	m_device_id(device_id),
	m_sector_size(sector_size),
	m_cycle_time(10),    // ms
	m_boot_block_size(16*1024),
	m_version(0)
{
}

/*
    Constructor for AT29C020
*/
at29c020_device::at29c020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at29x_device(mconfig, AT29C020, tag, owner, clock, 256*1024, 0xda, 256)
{
}

/*
    Constructor for AT29C040
*/
at29c040_device::at29c040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at29x_device(mconfig, AT29C040, tag, owner, clock, 512*1024, 0x5b, 512)
{
}

/*
    Constructor for AT29C040A
*/
at29c040a_device::at29c040a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at29x_device(mconfig, AT29C040A, tag, owner, clock, 512*1024, 0xa4, 256)
{
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void at29x_device::nvram_default()
{
	memset(m_eememory.get(), 0, m_memory_size+2);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool at29x_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_eememory.get(), m_memory_size+2, actual) && actual == m_memory_size+2;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool at29x_device::nvram_write(util::write_stream &file)
{
	// If we don't write (because there were no changes), the file will be wiped
	LOGMASKED(LOG_PRG, "Write to NVRAM file\n");
	m_eememory[0] = m_version;

	size_t actual;
	return !file.write(m_eememory.get(), m_memory_size+2, actual) && actual == m_memory_size+2;
}

/*
    Programming timer callback
*/
void at29x_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (m_pgm)
	{
	case PGM_1:
		// Programming cycle timeout
		LOGMASKED(LOG_WARN, "Programming cycle timeout\n", tag());
		m_pgm = PGM_0;
		break;

	case PGM_2:
		// Programming cycle start
		LOGMASKED(LOG_PRG, "Sector write start\n", tag());
		m_pgm = PGM_3;
		// We assume a typical delay of 70% of the max value
		m_programming_timer->adjust(attotime::from_msec(m_cycle_time*7/10));
		break;

	case PGM_3:
		// Programming cycle end; now burn the buffer into the flash EEPROM
		memcpy(m_eememory.get() + 2 + get_sector_number(m_programming_last_offset) * m_sector_size, m_programming_buffer.get(), m_sector_size);

		LOGMASKED(LOG_PRG, "Sector write completed at location %04x\n", m_programming_last_offset);

		// Data protect state will be activated at the end of the program cycle [1]
		if (m_enabling_sdb)  m_sdp = true;

		// Data protect state will be deactivated at the end of the program period [1]
		if (m_disabling_sdb) m_sdp = false;

		LOGMASKED(LOG_PRG, "Software data protection = %d\n", m_sdp);

		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;
		sync_flags();
		break;

	default:
		LOGMASKED(LOG_WARN, "Invalid state %d during programming\n", m_pgm);
		m_pgm = PGM_0;
		break;
	}
}

void at29x_device::sync_flags()
{
	if (m_lower_bbl) m_eememory[1] |= 0x04;
	else m_eememory[1] &= ~0x04;

	if (m_higher_bbl) m_eememory[1] |= 0x02;
	else m_eememory[1] &= ~0x02;

	if (m_sdp) m_eememory[1] |= 0x01;
	else m_eememory[1] &= ~0x01;
}

/*
    read a byte from FEEPROM
*/
uint8_t at29x_device::read(offs_t offset)
{
	int reply;

	offset &= m_address_mask;

	// Reading in the midst of any command sequence cancels it (not verified)
	m_cmd = CMD_0;
	m_long_sequence = false;

	sync_flags();

	// Reading before the start of a programming cycle cancels it (not verified)
	if (m_pgm == PGM_1)
	{
		// Attempt to access a locked out boot block: cancel programming command if necessary
		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;
		m_programming_timer->adjust(attotime::never);
	}

	if (m_id_mode)
	{
		// Experiments showed that the manufacturer code and device code
		// are returned for every address 0 and 1 modulo sector_size.
		//
		if ((offset % m_sector_size)==0) reply = 0x1f; // Manufacturer code
		else
		{
			if ((offset % m_sector_size)==1) reply = m_device_id; // Device code
			else
			{
				// Boot block lockout detection [1]
				if (offset == 0x00002) reply = m_lower_bbl? 0xff : 0xfe;
				else
				{
					if (offset == 0x7fff2) reply = m_higher_bbl? 0xff : 0xfe;
					else reply = 0;
				}
			}
		}
	}
	else if ((m_pgm == PGM_2) || (m_pgm == PGM_3))
	{
		if (m_pgm == PGM_2)
		{
			// DATA* polling starts the programming cycle (not verified)
			m_pgm = PGM_3;

			// We assume a typical delay of 70% of the max value
			m_programming_timer->adjust(attotime::from_msec(m_cycle_time*7/10));
		}

		LOGMASKED(LOG_READ, "DATA poll; toggle bit 1\n", tag());
		reply = m_toggle_bit? 0x02 : 0x00;
		m_toggle_bit = !m_toggle_bit;

		// When we read the byte on the last position, we get the inverse of the last bit [1]
		if (offset == m_programming_last_offset)
		{
			reply |= ((~m_programming_buffer[m_programming_last_offset & m_sector_mask]) & 0x01);
		}
	}
	else
		// Simple case: just read the memory contents
		reply = m_eememory[offset+2];

	LOGMASKED(LOG_READ, "%05x -> %02x (PGM=%d)\n", offset, reply, m_pgm);

	return reply;
}

/*
    Write a byte to FEEPROM
*/
void at29x_device::write(offs_t offset, uint8_t data)
{
	offset &= m_address_mask;
	LOGMASKED(LOG_WRITE, "%05x <- %02x\n", offset, data);

	// The special CFI commands assume a smaller address space according
	// to the specification ("address format A14-A0")
	offs_t cfi_offset = offset & 0x7fff;

	if (m_enabling_bbl)
	{
		// Determine whether we lock the upper or lower boot block
		LOGMASKED(LOG_STATE, "Enabling boot block lockout\n", tag());
		m_enabling_bbl = false;

		if ((offset == 0x00000) && (data == 0x00))
		{
			LOGMASKED(LOG_STATE, "Enabling lower boot block lockout\n", tag());
			m_lower_bbl = true;
			sync_flags();
			return;
		}
		else
		{
			if ((offset == 0x7ffff) && (data == 0xff))
			{
				LOGMASKED(LOG_STATE, "Enabling higher boot block lockout\n", tag());
				m_higher_bbl = true;
				sync_flags();
				return;
			}
			else
			{
				LOGMASKED(LOG_WARN, "Invalid boot block specification: %05x/%02x\n", offset, data);
			}
		}
	}

	switch (m_cmd)
	{
	case CMD_0:
		//  CMD_0: start state
		if ((cfi_offset == 0x5555) && (data == 0xaa))
		{
			LOGMASKED(LOG_STATE, "Command sequence started (aa)\n", tag());
			m_cmd = CMD_1;
			return;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
		}
		break;

	case CMD_1:
		//  CMD_1: state after writing aa to 5555
		if ((cfi_offset == 0x2aaa) && (data == 0x55))
		{
			LOGMASKED(LOG_STATE, "Command sequence continued (55)\n", tag());
			m_cmd = CMD_2;
			return;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
			LOGMASKED(LOG_STATE, "Command sequence aborted\n", tag());
		}
		break;

	case CMD_2:
		//  CMD_2: state after writing 55 to 2aaa
		if (cfi_offset == 0x5555)
		{
			m_pgm = PGM_0;
			m_enabling_sdb = false;
			m_disabling_sdb = false;
			m_programming_timer->adjust(attotime::never);

			// Process command
			LOGMASKED(LOG_STATE, "Command sequence continued (%2x)\n", data);
			switch (data)
			{
			case 0x10:
				//  Software chip erase (optional feature, see [1])
				if (m_long_sequence)
				{
					if (m_lower_bbl || m_higher_bbl)
						LOGMASKED(LOG_WARN, "Boot block lockout active; chip cannot be erased.\n", tag());
					else
					{
						LOGMASKED(LOG_STATE, "Erase chip\n", tag());
						memset(m_eememory.get()+2, 0xff, m_memory_size);
					}
				}
				break;

			case 0x20:
				// Software data protection disable
				// The complete sequence is aa-55-80-aa-55-20
				// so we need a 80 before, else the sequence is invalid
				if (m_long_sequence)
				{
					LOGMASKED(LOG_STATE, "Software data protection disable\n", tag());
					m_pgm = PGM_1;
					m_disabling_sdb = true;
					// It is not clear from the specification whether the byte cycle timer
					// is already started here or when the first data byte is written
				}
				break;

			case 0x40:
				// Boot block lockout enable
				// Complete sequence is aa-55-80-aa-55-40
				LOGMASKED(LOG_STATE, "Boot block lockout enable\n", tag());
				if (m_long_sequence) m_enabling_bbl = true;
				// We'll know which boot block is affected on the next write
				break;

			case 0x80:
				// Long sequences are those that contain aa55 twice
				m_long_sequence = true;
				break;

			case 0x90:
				// Software product identification entry
				LOGMASKED(LOG_STATE, "Entering Identification mode\n", tag());
				m_id_mode = true;
				break;

			case 0xa0:
				// Software data protection enable
				LOGMASKED(LOG_STATE, "Software data protection enable\n", tag());
				m_pgm = PGM_1;
				m_enabling_sdb = true;
				// It is not clear from the specification whether the byte cycle timer
				// is already started here or when the first data byte is written
				break;

			case 0xf0:
				// Software product identification exit
				LOGMASKED(LOG_STATE, "Exiting Identification mode\n", tag());
				m_id_mode = false;
				break;
			}
			m_cmd = CMD_0;
			if (data != 0x80) m_long_sequence = false;

			// Return, because we don't want to write the EEPROM with the command byte
			return;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
		}
	}

	if ((m_pgm == PGM_2) && (get_sector_number(offset) != get_sector_number(m_programming_last_offset)))
	{
		// cancel current programming cycle
		LOGMASKED(LOG_WRITE, "Invalid sector change (from sector 0x%04x to 0x%04x); cancel programming cycle\n", get_sector_number(m_programming_last_offset), get_sector_number(offset));
		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;
		m_programming_timer->adjust(attotime::never);
	}

	if (((m_pgm == PGM_0) && !m_sdp)  // write directly
		|| (m_pgm == PGM_1))          // write after unlocking
	{
		if (((offset < m_boot_block_size) && m_lower_bbl)
			|| ((offset >= m_memory_size-m_boot_block_size) && m_higher_bbl))
		{
			// attempt to access a locked out boot block: cancel programming
			// command if necessary
			LOGMASKED(LOG_WRITE, "Attempt to access a locked out boot block: offset = %05x, lowblock=%d, highblock=%d\n", offset, m_lower_bbl, m_higher_bbl);

			m_pgm = PGM_0;
			m_enabling_sdb = false;
			m_disabling_sdb = false;
		}
		else
		{   // enter programming mode
			LOGMASKED(LOG_STATE, "Enter programming mode (m_pgm=%d, m_sdp=%d)\n", m_pgm, m_sdp);
			// Clear the programming buffer
			memset(m_programming_buffer.get(), 0xff, m_sector_size);
			m_pgm = PGM_2;
		}
	}
	// TODO: If data protection is active and bytes are written, the device
	// enters a dummy write mode

	if (m_pgm == PGM_2)
	{
		// write data to programming buffer
		LOGMASKED(LOG_PRG, "Write data to programming buffer: buf[%x] = %02x\n", offset & m_sector_mask, data);
		m_programming_buffer[offset & m_sector_mask] = data;
		m_programming_last_offset = offset;
		m_programming_timer->adjust(attotime::from_usec(150));  // next byte must be written before the timer expires
	}
}

void at29x_device::device_start()
{
	m_programming_buffer = std::make_unique<uint8_t[]>(m_sector_size);
	m_eememory = std::make_unique<uint8_t[]>(m_memory_size+2);
	m_programming_timer = timer_alloc(PRGTIMER);

	// TODO: Complete 16-bit handling
	m_address_mask = m_memory_size/(m_word_width/8) - 1;
	m_sector_mask = m_sector_size - 1;
}

void at29x_device::device_stop(void)
{
	m_programming_buffer = nullptr;
	m_eememory = nullptr;
}

void at29x_device::device_reset(void)
{
	if (m_eememory[0] != m_version)
	{
		LOGMASKED(LOG_WARN, "Warning: Version mismatch; expected %d but found %d in file. Resetting.\n", m_version, m_eememory[0]);
		m_eememory[0] = 0;
		m_eememory[1] = 0;
	}

	m_lower_bbl  =  ((m_eememory[1] & 0x04)!=0);
	m_higher_bbl =  ((m_eememory[1] & 0x02)!=0);
	m_sdp        =  ((m_eememory[1] & 0x01)!=0);

	LOGMASKED(LOG_CONFIG, "LowerBBL = %d, HigherBBL = %d, SoftDataProt = %d\n", m_lower_bbl, m_higher_bbl, m_sdp);

	m_id_mode = false;
	m_cmd = CMD_0;
	m_enabling_bbl = false;
	m_long_sequence = false;
	m_pgm = PGM_0;
	m_enabling_sdb = false;
	m_disabling_sdb = false;
	m_toggle_bit = false;
	m_programming_last_offset = 0;
}

DEFINE_DEVICE_TYPE(AT29C020,  at29c020_device,  "at29c020",  "ATMEL 29C020 256Kx8 FEEPROM")
DEFINE_DEVICE_TYPE(AT29C040,  at29c040_device,  "at29c040",  "ATMEL 29C040 512Kx8 FEEPROM")
DEFINE_DEVICE_TYPE(AT29C040A, at29c040a_device, "at29c040a", "ATMEL 29C040A 512Kx8 FEEPROM")
