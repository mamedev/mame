// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/*
    Atmel at29c040a flash EEPROM

    512k*8 FEEPROM, organized in pages of 256 bytes.

    References:
    Datasheets were found on Atmel's site (www.atmel.com)

    Raphael Nabet 2003

    September 2010: Rewritten as device
    February 2012: Rewritten as class
*/

#include "at29040a.h"

#define VERBOSE 2
#define LOG logerror

#define FEEPROM_SIZE        0x80000
#define SECTOR_SIZE         0x00100
#define BOOT_BLOCK_SIZE     0x04000

#define ADDRESS_MASK        0x7ffff
#define SECTOR_ADDRESS_MASK 0x7ff00
#define BYTE_ADDRESS_MASK   0x000ff

#define PRG_TIMER 1

#define VERSION 0

/*
    Constructor.
*/
at29040a_device::at29040a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, AT29040A, "ATMEL 29040A 512K*8 FEEPROM", tag, owner, clock, "at29040a", __FILE__),
	device_nvram_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void at29040a_device::nvram_default()
{
	memset(m_eememory, 0, FEEPROM_SIZE+2);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void at29040a_device::nvram_read(emu_file &file)
{
	file.read(m_eememory, FEEPROM_SIZE+2);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void at29040a_device::nvram_write(emu_file &file)
{
	m_eememory[0] = VERSION;
	file.write(m_eememory, FEEPROM_SIZE+2);
}

/*
    programming timer callback
*/
void at29040a_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (m_pgm)
	{
	case PGM_1:
		/* programming cycle timeout */
		if (VERBOSE>7) LOG("at29040a: Programming cycle timeout\n");
		m_pgm = PGM_0;
		break;

	case PGM_2:
		/* programming cycle start */
		if (VERBOSE>7) LOG("at29040a: Sector write start\n");
		m_pgm = PGM_3;
		/* max delay 10ms, typical delay 5 to 7 ms */
		m_programming_timer->adjust(attotime::from_msec(5));
		break;

	case PGM_3:
		/* programming cycle end */
		memcpy(m_eememory + 2 + (m_programming_last_offset & ~0xff), m_programming_buffer, SECTOR_SIZE);
		if (VERBOSE>7) LOG("at29040a: Sector write completed at location %04x + 2\n", (m_programming_last_offset & ~0xff));
		if (m_enabling_sdb)
		{
			m_sdp = true;
		}
		if (m_disabling_sdb)
		{
			m_sdp = false;
		}
		if (VERBOSE>7) LOG("at29040a: Software data protection = %d\n", m_sdp);

		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;

		break;

	default:
		if (VERBOSE>1) LOG("internal error in %s %d\n", __FILE__, __LINE__);
		break;
	}
}

void at29040a_device::sync_flags()
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
READ8_MEMBER( at29040a_device::read )
{
	int reply;

	offset &= ADDRESS_MASK;

	/* reading in the midst of any command sequence cancels it (right???) */
	m_cmd = CMD_0;
	m_long_sequence = false;
	// m_higher_bbl = true;         // who says that?

	sync_flags();

	/* reading before the start of a programming cycle cancels it (right???) */
	if (m_pgm == PGM_1)
	{
		// attempt to access a locked out boot block: cancel programming
		// command if necessary
		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;
		m_programming_timer->adjust(attotime::never);
	}

	if (m_id_mode)
	{
		switch (offset)
		{
		case 0x00000:
			reply = 0x1f;       // Manufacturer code
			break;

		case 0x00001:
			reply = 0xa4;       // Device code
			break;

		case 0x00002:
			reply = m_lower_bbl? 0xff : 0xfe;
			break;

		case 0x7fff2:
			reply = m_higher_bbl? 0xff : 0xfe;
			break;

		default:
			reply = 0;
			break;
		}
	}
	else if ((m_pgm == PGM_2) || (m_pgm == PGM_3))
	{
		if (m_pgm == PGM_2)
		{   // DATA* polling starts the programming cycle (right???)
			m_pgm = PGM_3;
			/* max delay 10ms, typical delay 5 to 7 ms */
			m_programming_timer->adjust(attotime::from_msec(5));
		}

		reply = m_toggle_bit? 0x02 : 0x00;
		m_toggle_bit = !m_toggle_bit;

		if ((offset == m_programming_last_offset) && (! (m_programming_buffer[m_programming_last_offset & 0xff] & 0x01)))
			reply |= 0x01;
	}
	else
		reply = m_eememory[offset+2];

	if (VERBOSE>7) LOG("at29040a: %05x -> %02x\n", offset, reply);

	return reply;
}

/*
    Write a byte to FEEPROM
*/
WRITE8_MEMBER( at29040a_device::write )
{
	offset &= ADDRESS_MASK;
	if (VERBOSE>7) LOG("at29040a: %05x <- %02x\n", offset, data);

	/* The special CFI commands assume a smaller address space according */
	/* to the specification ("address format A14-A0") */
	offs_t cfi_offset = offset & 0x7fff;

	if (m_enabling_bbl)
	{
		if (VERBOSE>7) LOG("at29040a: Enabling boot block lockout\n");
		m_enabling_bbl = false;

		if ((offset == 0x00000) && (data == 0x00))
		{
			if (VERBOSE>7) LOG("at29040a: Enabling lower boot block lockout\n");
			m_lower_bbl = true;
			sync_flags();
			return;
		}
		else
		{
			if ((offset == 0x7ffff) && (data == 0xff))
			{
				if (VERBOSE>7) LOG("at29040a: Enabling higher boot block lockout\n");
				m_higher_bbl = true;
				sync_flags();
				return;
			}
			else
			{
				if (VERBOSE>1) LOG("at29040a: Invalid boot block specification: %05x/%02x\n", offset, data);
			}
		}
	}

	switch (m_cmd)
	{
	case CMD_0:
		if ((cfi_offset == 0x5555) && (data == 0xaa))
		{
			if (VERBOSE>7) LOG("at29040a: Command sequence started\n");
			m_cmd = CMD_1;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
		}
		break;

	case CMD_1:
		if ((cfi_offset == 0x2aaa) && (data == 0x55))
		{
			m_cmd = CMD_2;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
			if (VERBOSE>7) LOG("at29040a: Command sequence aborted\n");
		}
		break;

	case CMD_2:
		if (cfi_offset == 0x5555)
		{
			if (!m_long_sequence)
				if (VERBOSE>7) LOG("at29040a: Command sequence completed\n");

			m_pgm = PGM_0;
			m_enabling_sdb = false;
			m_disabling_sdb = false;
			m_programming_timer->adjust(attotime::never);

			/* process command */
			switch (data)
			{
			case 0x10:
				/*  Software chip erase */
				if (m_long_sequence)
				{
					if (m_lower_bbl || m_higher_bbl)
					{
						if (VERBOSE>1) LOG("at29040a: Chip erase sequence deactivated due to previous boot block lockout.\n");
					}
					else
					{
						if (VERBOSE>7) LOG("at29040a: Erase chip\n");
						memset(m_eememory+2, 0xff, FEEPROM_SIZE);
					}
				}
				break;

			case 0x20:
				/* Software data protection disable */
				if (VERBOSE>7) LOG("at29040a: Software data protection disable\n");
				// The complete sequence is aa-55-80-aa-55-20
				// so we need a 80 before, else the sequence is invalid
				if (m_long_sequence)
				{
					m_pgm = PGM_1;
					m_disabling_sdb = true;
					/* set command timeout (right???) */
					//m_programming_timer->adjust(attotime::from_usec(150), id, 0.);
				}
				break;

			case 0x40:
				/* Boot block lockout enable */
				// Complete sequence is aa-55-80-aa-55-40
				if (VERBOSE>7) LOG("at29040a: Boot block lockout enable\n");
				if (m_long_sequence) m_enabling_bbl = true;
				break;

			case 0x80:
				m_long_sequence = true;
				break;

			case 0x90:
				/* Software product identification entry */
				if (VERBOSE>7) LOG("at29040a: Identification mode (start)\n");
				m_id_mode = true;
				break;

			case 0xa0:
				/* Software data protection enable */
				if (VERBOSE>7) LOG("at29040a: Software data protection enable\n");
				m_pgm = PGM_1;
				m_enabling_sdb = true;
				/* set command timeout (right???) */
				//m_programming_timer->adjust(attotime::from_usec(150), id, 0.);
				break;

			case 0xf0:
				/* Software product identification exit */
				if (VERBOSE>7) LOG("at29040a: Identification mode (end)\n");
				m_id_mode = false;
				break;
			}
			m_cmd = CMD_0;
			if (data != 0x80) m_long_sequence = false;

			/* return, because we don't want to write the EEPROM with the command byte */
			return;
		}
		else
		{
			m_cmd = CMD_0;
			m_long_sequence = false;
		}
	}
	if ((m_pgm == PGM_2)
			&& ((offset & ~0xff) != (m_programming_last_offset & ~0xff)))
	{
		/* cancel current programming cycle */
		if (VERBOSE>7) LOG("at29040a: invalid sector change (from %05x to %05x); cancel programming cycle\n",(offset & ~0xff), (m_programming_last_offset & ~0xff));
		m_pgm = PGM_0;
		m_enabling_sdb = false;
		m_disabling_sdb = false;
		m_programming_timer->adjust(attotime::never);
	}

	if (((m_pgm == PGM_0) && !m_sdp)  // write directly
		|| (m_pgm == PGM_1))          // write after unlocking
	{
		if (((offset < BOOT_BLOCK_SIZE) && m_lower_bbl)
			|| ((offset >= FEEPROM_SIZE-BOOT_BLOCK_SIZE) && m_higher_bbl))
		{
			// attempt to access a locked out boot block: cancel programming
			// command if necessary
			if (VERBOSE>7) LOG("at29040a: attempt to access a locked out boot block: offset = %05x, lowblock=%d, highblock=%d\n", offset, m_lower_bbl, m_higher_bbl);

			m_pgm = PGM_0;
			m_enabling_sdb = false;
			m_disabling_sdb = false;
		}
		else
		{   /* enter programming mode */
			if (VERBOSE>7) LOG("at29040a: enter programming mode (m_pgm=%d)\n", m_pgm);
			memset(m_programming_buffer, 0xff, SECTOR_SIZE);
			m_pgm = PGM_2;
		}
	}
	if (m_pgm == PGM_2)
	{
		/* write data to programming buffer */
		if (VERBOSE>7) LOG("at29040a: Write data to programming buffer\n");
		m_programming_buffer[offset & 0xff] = data;
		m_programming_last_offset = offset;
		m_programming_timer->adjust(attotime::from_usec(150));  // next byte must be written before the timer expires
	}
}

void at29040a_device::device_start(void)
{
	m_programming_buffer = global_alloc_array(UINT8, SECTOR_SIZE);
	m_programming_timer = timer_alloc(PRG_TIMER);

	m_eememory = global_alloc_array(UINT8, FEEPROM_SIZE+2);
}

void at29040a_device::device_stop(void)
{
	global_free_array(m_programming_buffer);
	global_free_array(m_eememory);
}

void at29040a_device::device_reset(void)
{
	if (m_eememory[0] != VERSION)
	{
		if (VERBOSE>1) LOG("AT29040A: Warning: Version mismatch; expected %d but found %d for %s. Resetting.\n", VERSION, m_eememory[0], tag());
		m_eememory[0] = 0;
		m_eememory[1] = 0;
	}

	m_lower_bbl =   ((m_eememory[1] & 0x04)!=0);
	m_higher_bbl =  ((m_eememory[1] & 0x02)!=0);
	m_sdp =         ((m_eememory[1] & 0x01)!=0);

	if (VERBOSE>7) LOG("at29040a (%s): LowerBBL = %d, HigherBBL = %d, SoftDataProt = %d\n", tag(), m_lower_bbl, m_higher_bbl, m_sdp);

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

const device_type AT29040A = &device_creator<at29040a_device>;
