// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 8722 Memory Management Unit emulation

**********************************************************************/

#include "mos8722.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MOS8722 = &device_creator<mos8722_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define CR_IO           BIT(m_reg[CR], 0)
#define CR_ROM_LO       BIT(m_reg[CR], 1)
#define CR_ROM_MID      ((m_reg[CR] >> 2) & 0x03)
#define CR_ROM_HI       ((m_reg[CR] >> 4) & 0x03)
#define CR_A16          BIT(m_reg[CR], 6)


// mode configuration register
#define MCR_8500        BIT(m_reg[MCR], 0)
#define MCR_FSDIR       BIT(m_reg[MCR], 3)
#define MCR_GAME        BIT(m_reg[MCR], 4)
#define MCR_EXROM       BIT(m_reg[MCR], 5)
#define MCR_C64         BIT(m_reg[MCR], 6)
#define MCR_40_80       BIT(m_reg[MCR], 7)


// RAM configuration register
static const offs_t RCR_BOTTOM_ADDRESS[4] = { 0x0400, 0x1000, 0x0400, 0x1000 };
static const offs_t RCR_TOP_ADDRESS[4] =    { 0xf000, 0xf000, 0xe000, 0xc000 };

#define RCR_SHARE       (m_reg[RCR] & 0x03)
#define RCR_BOTTOM      BIT(m_reg[RCR], 2)
#define RCR_TOP         BIT(m_reg[RCR], 3)
#define RCR_VA16        BIT(m_reg[RCR], 6)


// page 0 pointer register
#define P0H_A16         BIT(m_reg[P0H], 0)


// page 1 pointer register
#define P1H_A16         BIT(m_reg[P1H], 0)



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8722_device - constructor
//-------------------------------------------------

mos8722_device::mos8722_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MOS8722, "MOS8722", tag, owner, clock, "mos8722", __FILE__),
	m_write_z80en(*this),
	m_write_fsdir(*this),
	m_read_game(*this),
	m_read_exrom(*this),
	m_read_sense40(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8722_device::device_start()
{
	// resolve callbacks
	m_write_z80en.resolve_safe();
	m_write_fsdir.resolve_safe();
	m_read_game.resolve_safe(1);
	m_read_exrom.resolve_safe(1);
	m_read_sense40.resolve_safe(1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8722_device::device_reset()
{
	for (auto & elem : m_reg)
	{
		elem = 0;
	}

	m_reg[P1L] = 0x01;

	m_p0h_latch = 0;
	m_p1h_latch = 0;

	m_write_z80en(MCR_8500);
	m_write_fsdir(MCR_FSDIR);
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

UINT8 mos8722_device::read(offs_t offset, UINT8 data)
{
	if (MCR_C64) return data;

	if (!CR_IO && offset >= 0xd500 && offset < 0xd50c)
	{
		switch (offset & 0x0f)
		{
		case CR:
			data = m_reg[CR] | 0x80;
			break;

		case MCR:
			data = m_reg[MCR] | 0x06;

			data &= ((m_read_game() << 4) | ~0x10);
			data &= ((m_read_exrom() << 5) | ~0x20);
			data &= ((m_read_sense40() << 7) | ~0x80);
			break;

		case VR:
			data = 0x20;
			break;

		default:
			data = m_reg[offset & 0x0f];
			break;
		}
	}
	else if (offset >= 0xff00 && offset < 0xff05)
	{
		switch (offset & 0x0f)
		{
		case CR:
			data = m_reg[CR] | 0x80;
			break;

		default:
			data = m_reg[offset & 0x0f];
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( mos8722_device::write )
{
	if (MCR_C64) return;

	if (!CR_IO && offset >= 0xd500 && offset < 0xd50c)
	{
		if (LOG) logerror("MOS8722 '%s' Write %01x : %02x\n", tag().c_str(), offset & 0x0f, data);

		switch (offset & 0x0f)
		{
		case CR:
			m_reg[CR] = data & 0x7f;
			break;

		case PCRA:
		case PCRB:
		case PCRC:
		case PCRD:
			m_reg[offset & 0x0f] = data & 0x7f;
			break;

		case MCR:
		{
			int _8500 = MCR_8500;
			int fsdir = MCR_FSDIR;

			m_reg[MCR] = data;

			if (_8500 != MCR_8500) m_write_z80en(MCR_8500);
			if (fsdir != MCR_FSDIR) m_write_fsdir(MCR_FSDIR);
			break;
		}

		case RCR:
			m_reg[RCR] = data & 0x4f;
			break;

		case P0L:
			m_reg[P0L] = data;
			m_reg[P0H] = m_p0h_latch;
			break;

		case P0H:
			m_p0h_latch = data & 0x01;
			break;

		case P1L:
			m_reg[P1L] = data;
			m_reg[P1H] = m_p1h_latch;
			break;

		case P1H:
			m_p1h_latch = data & 0x01;
			break;

		default:
			m_reg[offset & 0x0f] = data;
		}
	}
	else if (offset >= 0xff00 && offset < 0xff05)
	{
		if (LOG) logerror("MOS8722 '%s' Write %01x : %02x\n", tag().c_str(), offset & 0x0f, data);

		switch (offset & 0x0f)
		{
		case CR:
			m_reg[CR] = data & 0x7f;
			break;

		default:
			m_reg[CR] = m_reg[offset & 0x0f];
			break;
		}
	}
}


//-------------------------------------------------
//  fsdir_r - fast serial direction read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::fsdir_r )
{
	return MCR_FSDIR;
}


//-------------------------------------------------
//  ta_r - translated address read
//-------------------------------------------------

offs_t mos8722_device::ta_r(offs_t offset, int aec, int *ms0, int *ms1, int *ms2, int *ms3, int *cas0, int *cas1)
{
	offs_t ta;

	*ms0 = 1;
	*ms1 = 1;
	*ms2 = CR_IO;
	*ms3 = !MCR_C64;

	if (aec)
	{
		// CPU access
		ta = offset & 0xff00;

		*cas0 = CR_A16;
		*cas1 = !*cas0;

		if (!MCR_C64)
		{
			if (offset >= 0xff00 && offset < 0xff05)
			{
				// MMU registers
				*cas0 = 1;
				*cas1 = 1;
			}
			else if (!MCR_8500 && !CR_A16 && offset < 0x1000)
			{
				// Z80 ROM
				ta = 0xd000 | (offset & 0xf00);

				*ms0 = 0;
				*ms1 = 0;
			}
			else
			{
				if (offset < 0x0100)
				{
					// page 0 pointer
					ta = m_reg[P0L] << 8;

					*cas0 = P0H_A16;
					*cas1 = !*cas0;
				}
				else if (offset < 0x0200)
				{
					// page 1 pointer
					ta = m_reg[P1L] << 8;

					*cas0 = P1H_A16;
					*cas1 = !*cas0;
				}
				else if (offset >= 0x4000 && offset < 0x8000)
				{
					// low ROM
					*ms0 = CR_ROM_LO;
					*ms1 = CR_ROM_LO;
				}
				else if (offset >= 0x8000 && offset < 0xc000)
				{
					// middle ROM
					*ms0 = BIT(CR_ROM_MID, 1);
					*ms1 = BIT(CR_ROM_MID, 0);
				}
				else if (offset >= 0xc000)
				{
					// high ROM
					*ms0 = BIT(CR_ROM_HI, 1);
					*ms1 = BIT(CR_ROM_HI, 0);
				}

				if (*ms0 && *ms1)
				{
					if ((offset >> 8) == m_reg[P0L])
					{
						ta = 0x0000;
					}
					else if ((offset >> 8) == m_reg[P1L])
					{
						ta = 0x0100;
					}
				}

				if ((RCR_BOTTOM && offset < RCR_BOTTOM_ADDRESS[RCR_SHARE]) ||
					(RCR_TOP && offset >= RCR_TOP_ADDRESS[RCR_SHARE]))
				{
					// RAM sharing
					*cas0 = 0;
					*cas1 = !*cas0;
				}
			}
		}
	}
	else
	{
		// VIC access
		ta = 0xf000 | (offset & 0xf00);

		*cas0 = RCR_VA16;
		*cas1 = !*cas0;
	}

	return ta;
}
