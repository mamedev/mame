// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/*
    Intel 28F640J5 Flash ROM emulation (could also handle 28F320J5 with minor
    changes, and possibly 28F256J3, 28F128J3, 28F640J3 and 28F320J3)

    The 28F640J5 is a 64Mbit FEEPROM that can be accessed either on an 8-bit or
    a 16-bit bus.

    References:
    Datasheets were found on Intel's site (www.intel.com)

    Raphael Nabet 2004, based on MAME's intelfsh.c core

    Device is currently only used in ti99/tn_usbsm

    Rewritten as class
    Michael Zapf, 2014

    TODO: Make it work
*/

#include "emu.h"
#include "strata.h"


#define FEEPROM_SIZE        0x800000    // 64Mbit
#define BLOCK_SIZE          0x020000

#define BLOCKLOCK_SIZE ((FEEPROM_SIZE/BLOCK_SIZE+7)/8)
#define WRBUF_SIZE 32
#define PROT_REGS_SIZE 18

#define COMPLETE_SIZE FEEPROM_SIZE + WRBUF_SIZE + PROT_REGS_SIZE + BLOCKLOCK_SIZE

#define ADDRESS_MASK        0x7fffff
#define BLOCK_ADDRESS_MASK  0x7e0000
#define BLOCK_ADDRESS_SHIFT 17
#define BYTE_ADDRESS_MASK   0x01ffff

/* accessors for individual block lock flags */
#define READ_BLOCKLOCK(block) ((m_blocklock[(block) >> 3] >> ((block) & 7)) & 1)
#define SET_BLOCKLOCK(block) (m_blocklock[(block) >> 3] |= 1 << ((block) & 7))
#define CLEAR_BLOCKLOCK(block) (m_blocklock[(block) >> 3] &= ~(1 << ((block) & 7)))

DEFINE_DEVICE_TYPE(STRATAFLASH, strataflash_device, "strataflash", "Intel 28F640J5")

strataflash_device::strataflash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, STRATAFLASH, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void strataflash_device::nvram_default()
{
	memset(m_flashmemory.get(), 0, COMPLETE_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool strataflash_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (file.read(m_flashmemory.get(), COMPLETE_SIZE, actual) || actual != COMPLETE_SIZE)
		return false;

	// TODO

	/*
	uint8_t buf;
	int i;

	// version flag
	if (file->read(& buf, 1) != 1)
	    return 1;
	if (buf != 0)
	    return 1;

	// chip state: master lock
	if (file->read(& buf, 1) != 1)
	    return 1;
	m_master_lock = buf & 1;

	// main FEEPROM area
	if (file->read(m_flashmemory, FEEPROM_SIZE) != FEEPROM_SIZE)
	    return 1;
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_flashmemory[i]);
	    *ptr = little_endianize_int16(*ptr);
	}

	// protection registers
	if (file->read(m_prot_regs, PROT_REGS_SIZE) != PROT_REGS_SIZE)
	    return 1;
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_prot_regs[i]);
	    *ptr = little_endianize_int16(*ptr);
	}

	// block lock flags
	if (file->read(m_blocklock, BLOCKLOCK_SIZE) != BLOCKLOCK_SIZE)
	    return 1;

	return 0;
	*/

	return true;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool strataflash_device::nvram_write(util::write_stream &file)
{
	// TODO

	/*
	uint8_t buf;
	int i;

	// version flag
	buf = 0;
	if (file->write(& buf, 1) != 1)
	    return 1;

	// chip state: lower boot block lockout, higher boot block lockout,
	software data protect
	buf = m_master_lock;
	if (file->write(& buf, 1) != 1)
	    return 1;

	// main FEEPROM area
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_flashmemory[i]);
	    *ptr = little_endianize_int16(*ptr);
	}
	if (file->write(m_flashmemory, FEEPROM_SIZE) != FEEPROM_SIZE)
	    return 1;
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_flashmemory[i]);
	    *ptr = little_endianize_int16(*ptr);
	}

	// protection registers
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_prot_regs[i]);
	    *ptr = little_endianize_int16(*ptr);
	}
	if (file->write(m_prot_regs, PROT_REGS_SIZE) != PROT_REGS_SIZE)
	    return 1;
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
	    uint16_t *ptr = (uint16_t *) (&m_prot_regs[i]);
	    *ptr = little_endianize_int16(*ptr);
	}

	// block lock flags
	if (file->write(m_blocklock, BLOCKLOCK_SIZE) != BLOCKLOCK_SIZE)
	    return 1;

	return 0;
	*/

	size_t actual;
	return !file.write(m_flashmemory.get(), COMPLETE_SIZE, actual) && actual == COMPLETE_SIZE;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void strataflash_device::device_start()
{
	m_mode = FM_NORMAL;
	m_status = 0x80;
	m_master_lock = 0;
	m_flashmemory = std::make_unique<uint8_t[]>(COMPLETE_SIZE);

	m_wrbuf = m_flashmemory.get() + FEEPROM_SIZE;
	m_prot_regs = m_wrbuf + WRBUF_SIZE;
	m_blocklock = m_prot_regs + PROT_REGS_SIZE;

	// clear various FEEPROM areas
	memset(m_prot_regs, 0xff, 18);
	memset(m_flashmemory.get(), 0xff, FEEPROM_SIZE);
	memset(m_blocklock, 0x00, BLOCKLOCK_SIZE);

	// set-up factory-programmed protection register segment
	m_prot_regs[BYTE_XOR_LE(0)] &= 0xfe;
	for (int i=2; i<10; i++)
		m_prot_regs[i] = machine().rand();
}

/*
    read a 8/16-bit word from FEEPROM
*/
uint16_t strataflash_device::read8_16(offs_t offset, bus_width_t bus_width)
{
	switch (bus_width)
	{
	case bw_8:
		offset &= ADDRESS_MASK;
		break;
	case bw_16:
		offset &= ADDRESS_MASK & ~1;
	}

	switch (m_mode)
	{
	default:
	case FM_NORMAL:
		switch (bus_width)
		{
		case bw_8:
			return m_flashmemory[BYTE_XOR_LE(offset)];
		case bw_16:
			return *(uint16_t*)(m_flashmemory.get()+offset);
		}
		break;
	case FM_READSTATUS:
		return m_status;
	case FM_WRBUFPART1:
		return 0x80;
	case FM_READID:
		if ((offset >= 0x100) && (offset < 0x112))
		{   /* protection registers */
			switch (bus_width)
			{
			case bw_8:
				return m_prot_regs[BYTE_XOR_LE(offset)];
			case bw_16:
				return *(uint16_t*)(m_prot_regs+offset);
			}
		}
		else
			switch (offset >> 1)
			{
			case 0: // maker ID
				return 0x89;    // Intel
			case 1: // chip ID
				return 0x15;    // 64 Mbit
			default:
				if (((offset & BYTE_ADDRESS_MASK) >> 1) == 2)
				{   // block lock config
					return READ_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT);
				}
				return 0;   // default case
			case 3: // master lock config
				if (m_master_lock)
					return 1;
				else
					return 0;
			}
		break;
	case FM_READQUERY:
		switch (offset >> 1)
		{
		case 0x00:  // maker ID
			return 0x89;    // Intel
		case 0x01:  // chip ID
			return 0x15;    // 64 Mbit
		default:
			if (((offset & BYTE_ADDRESS_MASK) >> 1) == 2)
			{   // block lock config
				return READ_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT);
			}
			return 0;   // default case
#if 0
		case 0x03: // master lock config
			if (m_flash_master_lock)
				return 1;
			else
				return 0;
#endif

		/* CFI query identification string */
		case 0x10:
			return 'Q';
		case 0x11:
			return 'R';
		case 0x12:
			return 'Y';
		case 0x13:
			return 0x01;
		case 0x14:
			return 0x00;
		case 0x15:
			return 0x31;
		case 0x16:
			return 0x00;
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
			return 0x00;

		/* system interface information: voltage */
		case 0x1b:
			return 0x45;
		case 0x1c:
			return 0x55;
		case 0x1d:
			return 0x00;
		case 0x1e:
			return 0x00;

		/* system interface information: timings */
		case 0x1f:
			return /*0x07*/0x00;
		case 0x20:
			return /*0x07*/0x00;
		case 0x21:
			return /*0x0a*/0x00;
		case 0x22:
			return 0x00;
		case 0x23:
			return /*0x04*/0x00;
		case 0x24:
			return /*0x04*/0x00;
		case 0x25:
			return /*0x04*/0x00;
		case 0x26:
			return 0x00;

		/* device geometry definition */
		case 0x27:
			return 0x17;
		case 0x28:
			return 0x02;
		case 0x29:
			return 0x00;
		case 0x2a:
			return 0x05;
		case 0x2b:
			return 0x00;
		case 0x2c:
			return 0x01;
		case 0x2d:
			return 0x3f;
		case 0x2e:
			return 0x00;
		case 0x2f:
			return 0x00;
		case 0x30:
			return 0x02;

		/* primary vendor-specific extended query */
		case 0x31:
			return 'P';
		case 0x32:
			return 'R';
		case 0x33:
			return 'I';
		case 0x34:
			return '1';
		case 0x35:
			return '1';
		case 0x36:
			return 0x0a;
		case 0x37:
			return 0x00;
		case 0x38:
			return 0x00;
		case 0x39:
			return 0x00;
		case 0x3a:
			return 0x01;
		case 0x3b:
			return 0x01;
		case 0x3c:
			return 0x00;
		case 0x3d:
			return 0x50;
		case 0x3e:
			return 0x00;
		case 0x3f:
			return 0x00;
		}
	}

	return 0;
}

/*
    write a 8/16-bit word to FEEPROM
*/
void strataflash_device::write8_16(offs_t offset, uint16_t data, bus_width_t bus_width)
{
	switch (bus_width)
	{
	case bw_8:
		offset &= ADDRESS_MASK;
		break;
	case bw_16:
		offset &= ADDRESS_MASK & ~1;
		break;
	}

	switch (m_mode)
	{
	case FM_NORMAL:
	case FM_READID:
	case FM_READQUERY:
	case FM_READSTATUS:
		switch (data)
		{
		case 0xff:  // read array
			m_mode = FM_NORMAL;
			break;
		case 0x90:  // read identifier codes
			m_mode = FM_READID;
			break;
		case 0x98:  // read query
			m_mode = FM_READQUERY;
			break;
		case 0x70:  // read status register
			m_mode = FM_READSTATUS;
			break;
		case 0x50:  // clear status register
			m_mode = FM_READSTATUS;
			m_status &= 0xC5;
			break;
		case 0xe8:  // write to buffer
			m_mode = FM_WRBUFPART1;
			m_wrbuf_base = offset & BLOCK_ADDRESS_MASK;
			/*m_status &= 0xC5;*/
			break;
		case 0x40:
		case 0x10:  // program
			m_mode = FM_WRITEPART1;
			m_status &= 0xC5;
			break;
		case 0x20:  // block erase
			m_mode = FM_CLEARPART1;
			m_status &= 0xC5;
			break;
		case 0xb0:  // block erase, program suspend
			/* not emulated (erase is instantaneous) */
			break;
		case 0xd0:  // block erase, program resume
			/* not emulated (erase is instantaneous) */
			break;
		case 0xb8:  // configuration
			m_mode = FM_CONFPART1;
			m_status &= 0xC5;
			break;
		case 0x60:  // set master lock
			m_mode = FM_SETLOCK;
			m_status &= 0xC5;
			break;
		case 0xc0:  // protection program
			m_mode = FM_WRPROTPART1;
			m_status &= 0xC5;
			break;
		default:
			logerror("Unknown flash mode byte %x\n", data);
			break;
		}
		break;
	case FM_WRBUFPART1:
		m_mode = FM_WRBUFPART2;
		if (((offset & BLOCK_ADDRESS_MASK) != m_wrbuf_base) || (data >= 0x20))
		{
			m_status |= 0x30;
			m_wrbuf_len = 0;
			m_wrbuf_count = data;
		}
		else
		{
			switch (bus_width)
			{
			case bw_8:
				m_wrbuf_len = data+1;
				break;
			case bw_16:
				m_wrbuf_len = (data+1) << 1;
				break;
			}
			m_wrbuf_count = data;
		}
		break;
	case FM_WRBUFPART2:
		m_mode = FM_WRBUFPART3;
		if (((offset & BLOCK_ADDRESS_MASK) != m_wrbuf_base)
			|| (((offset & BYTE_ADDRESS_MASK) + m_wrbuf_len) > BLOCK_SIZE))
		{
			m_status |= 0x30;
			m_wrbuf_len = 0;
			m_wrbuf_base = 0;
		}
		else
			m_wrbuf_base = offset;
		memset(m_wrbuf, 0xff, m_wrbuf_len); /* right??? */
		[[fallthrough]];
	case FM_WRBUFPART3:
		if ((offset < m_wrbuf_base) || (offset >= (m_wrbuf_base + m_wrbuf_len)))
			m_status |= 0x30;
		else
		{
			switch (bus_width)
			{
			case bw_8:
				m_wrbuf[offset-m_wrbuf_base] = data;
				break;
			case bw_16:
				m_wrbuf[offset-m_wrbuf_base] = data & 0xff;
				m_wrbuf[offset-m_wrbuf_base+1] = data >> 8;
				break;
			}
		}
		if (m_wrbuf_count == 0)
			m_mode = FM_WRBUFPART4;
		else
			m_wrbuf_count--;
		break;
	case FM_WRBUFPART4:
		if (((offset & BLOCK_ADDRESS_MASK) != (m_wrbuf_base & BLOCK_ADDRESS_MASK)) || (data != 0xd0))
		{
			m_status |= 0x30;
		}
		else if (READ_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT) && !m_hard_unlock)
		{
			m_status |= 0x12;
		}
		else if (!(m_status & 0x30))
		{
			int i;
			for (i=0; i<m_wrbuf_len; i++)
				m_flashmemory[BYTE_XOR_LE(m_wrbuf_base+i)] &= m_wrbuf[i];
			m_mode = FM_READSTATUS;
		}
		break;
	case FM_WRITEPART1:
		if (READ_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT) && !m_hard_unlock)
		{
			m_status |= 0x12;
		}
		else
		{
			switch (bus_width)
			{
			case bw_8:
				m_flashmemory[BYTE_XOR_LE(offset)] &= data;
				break;
			case bw_16:
				*(uint16_t*)(m_flashmemory.get()+offset) &= data;
				break;
			}
		}
		m_mode = FM_READSTATUS;
		break;
	case FM_CLEARPART1:
		if (data == 0xd0)
		{
			// clear the 128k block containing the current address
			// to all 0xffs
			if (READ_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT) && !m_hard_unlock)
			{
				m_status |= 0x22;
			}
			else
			{
				offset &= BLOCK_ADDRESS_MASK;
				memset(&m_flashmemory[offset], 0xff, BLOCK_SIZE);
			}
			m_mode = FM_READSTATUS;
		}
		break;
	case FM_SETLOCK:
		switch (data)
		{
		case 0xf1:
			if (!m_hard_unlock)
				m_status |= 0x12;
			else
				m_master_lock = 1;
			break;
		case 0x01:
			if (m_master_lock && !m_hard_unlock)
				m_status |= 0x12;
			else
				SET_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT);
			break;
		case 0xd0:
			if (m_master_lock && !m_hard_unlock)
				m_status |= 0x22;
			else
				CLEAR_BLOCKLOCK(offset >> BLOCK_ADDRESS_SHIFT);
			break;
		case 0x03:  // Set Read configuration
			/* ignore command */
			break;
		default:
			m_status |= 0x30;
			break;
		}
		m_mode = FM_READSTATUS;
		break;
	case FM_CONFPART1:
		/* configuration register is not emulated because the sts pin is not */
		//m_configuration = data;
		m_mode = FM_READSTATUS;   /* right??? */
		break;
	case FM_WRPROTPART1:
		if ((offset < 0x100) || (offset >= 0x112))
			m_status |= 0x10;
		else if ((offset >= 0x102) && !((m_prot_regs[BYTE_XOR_LE(0)] >> ((offset - 0x102) >> 3)) & 1))
			m_status |= 0x12;
		else
		{
			switch (bus_width)
			{
			case bw_8:
				m_prot_regs[BYTE_XOR_LE(offset-0x100)] &= data;
				break;
			case bw_16:
				*(uint16_t*)(m_prot_regs+(offset-0x100)) &= data;
				break;
			}
		}
		m_mode = FM_READSTATUS;   /* right??? */
		break;
	}
}

/*
    read a byte from FEEPROM
*/
uint8_t strataflash_device::read8(offs_t offset)
{
	return read8_16(offset, bw_8);
}

/*
    Write a byte to FEEPROM
*/
void strataflash_device::write8(offs_t offset, uint8_t data)
{
	write8_16(offset, data, bw_8);
}

/*
    read a 16-bit word from FEEPROM
*/
uint16_t strataflash_device::read16(offs_t offset)
{
	return read8_16(offset, bw_16);
}

/*
    Write a byte to FEEPROM
*/
void strataflash_device::write16(offs_t offset, uint16_t data)
{
	write8_16(offset, data, bw_16);
}
