/**********************************************************************

    MOS Technology 8722 Memory Management Unit emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "mos8722.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// registers
enum
{
	CR = 0,
	PCRA, LCRA = PCRA,
	PCRB, LCRB = PCRB,
	PCRC, LCRC = PCRC, 
	PCRD, LCRD = PCRD,
	MCR,
	RCR,
	P0L,
	P0H,
	P1L,
	P1H,
	VR
};


// configuration register
enum
{
	CR_IO_SYSTEM_IO = 0,
	CR_IO_HI_ROM
};

enum
{
	CR_ROM_SYSTEM_ROM = 0,
	CR_ROM_INT_FUNC_ROM,
	CR_ROM_EXT_FUNC_ROM,
	CR_ROM_RAM
};

#define CR_IO			BIT(m_reg[CR], 0)
#define CR_ROM_LO		BIT(m_reg[CR], 1)
#define CR_ROM_MID		((m_reg[CR] >> 2) & 0x03)
#define CR_ROM_HI		((m_reg[CR] >> 4) & 0x03)
#define CR_A16			BIT(m_reg[CR], 6)


// mode configuration register
#define MCR_8500		BIT(m_reg[MCR], 0)
#define MCR_FSDIR		BIT(m_reg[MCR], 3)
#define MCR_GAME		BIT(m_reg[MCR], 4)
#define MCR_EXROM		BIT(m_reg[MCR], 5)
#define MCR_C64			BIT(m_reg[MCR], 6)
#define MCR_40_80		BIT(m_reg[MCR], 7)


// RAM configuration register
enum
{
	RCR_STATUS_NO = 0,
	RCR_STATUS_BOTTOM,
	RCR_STATUS_TOP,
	RCR_STATUS_BOTH
};

#define RCR_SHARE		(m_reg[RCR] & 0x03)
#define RCR_STATUS		((m_reg[RCR] >> 2) & 0x03)
#define RCR_A16			BIT(m_reg[RCR], 6)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MOS8722 = &device_creator<mos8722_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8722_device - constructor
//-------------------------------------------------

mos8722_device::mos8722_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, MOS8722, "MOS8722", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos8722_device::device_config_complete()
{
	// inherit a copy of the static data
	const mos8722_interface *intf = reinterpret_cast<const mos8722_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos8722_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_z80en_cb, 0, sizeof(m_out_z80en_cb));
		memset(&m_out_fsdir_cb, 0, sizeof(m_out_fsdir_cb));
		memset(&m_in_game_cb, 0, sizeof(m_in_game_cb));
		memset(&m_in_exrom_cb, 0, sizeof(m_in_exrom_cb));
		memset(&m_in_sense40_cb, 0, sizeof(m_in_sense40_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8722_device::device_start()
{
	// resolve callbacks
	m_out_z80en_func.resolve(m_out_z80en_cb, *this);
	m_out_fsdir_func.resolve(m_out_fsdir_cb, *this);
	m_in_game_func.resolve(m_in_game_cb, *this);
	m_in_exrom_func.resolve(m_in_exrom_cb, *this);
	m_in_sense40_func.resolve(m_in_sense40_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8722_device::device_reset()
{
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

READ8_MEMBER( mos8722_device::read )
{
	return 0;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( mos8722_device::write )
{
}


//-------------------------------------------------
//  fsdir_r - fast serial direction read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::fsdir_r )
{
	return 1;
}


//-------------------------------------------------
//  ms0_r - memory status 0 read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::ms0_r )
{
	return 1;
}


//-------------------------------------------------
//  ms1_r - memory status 1 read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::ms1_r )
{
	return 1;
}


//-------------------------------------------------
//  ms2_r - memory status 2 read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::ms2_r )
{
	return 1;
}


//-------------------------------------------------
//  ms3_r - memory status 3 read
//-------------------------------------------------

READ_LINE_MEMBER( mos8722_device::ms3_r )
{
	return 1;
}


//-------------------------------------------------
//  ta_r - translated address read
//-------------------------------------------------

offs_t mos8722_device::ta_r(offs_t offset, int aec)
{
	return offset;
}
