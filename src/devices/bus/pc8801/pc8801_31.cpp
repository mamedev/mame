// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

	NEC PC8801-31 CD-ROM I/F

	TODO:
	- Interface with PC8801-30 (the actual CD drive);

**************************************************************************************************/

#include "emu.h"
#include "pc8801_31.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(PC8801_31, pc8801_31_device, "pc8801_31", "NEC PC8801-31 CD-ROM I/F")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  pc8801_31_device - constructor
//-------------------------------------------------


pc8801_31_device::pc8801_31_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC8801_31, tag, owner, clock)
	, m_rom_bank_cb(*this)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------


void pc8801_31_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc8801_31_device::device_resolve_objects()
{
	// resolve callbacks
	m_rom_bank_cb.resolve();
}


void pc8801_31_device::device_start()
{
	save_item(NAME(m_clock_hb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void pc8801_31_device::device_reset()
{
	m_clock_hb = false;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


void pc8801_31_device::amap(address_map &map)
{
	map(0x08, 0x08).rw(FUNC(pc8801_31_device::clock_r), FUNC(pc8801_31_device::volume_control_w));
	map(0x09, 0x09).w(FUNC(pc8801_31_device::rom_bank_w));
}

/*
 * I/O Port $98
 *
 * x--- ---- (r/o?) device clock heartbeat?
 * ---- -xxx (w/o?) CD-DA volume control (not unlike PCE)
 * ---- -00x enable
 * ---- -01x disable
 * ---- -100 fade-in short (100 msec)
 * ---- -101 fade-in long (1500 msec)
 * ---- -110 fade-out short (100 msec)
 * ---- -111 fade-out long (1500 msec)
 *
 */
u8 pc8801_31_device::clock_r()
{
	// Checked 11 times on POST before giving up, definitely some kind of timing-based CD-ROM or board identification.
    // If check passes the BIOS goes on with the "CD-System initialize\n[Space]->CD player" screen.
	// A similar PCE pattern is mapped as "CDDA data select".
	// There's no way to load floppies with this always on unless STOP key is held on boot (which doesn't look convenient).
	// TODO: identify source and verify how much fast this really is.
	m_clock_hb ^= 1;
	return m_clock_hb << 7;
}

void pc8801_31_device::volume_control_w(u8 data)
{
	logerror("%s: volume_w %02x\n", machine().describe_context(), data);
}

/*
 * I/O Port $99
 *
 * ---x ---- CD-ROM BIOS bank
 * ---- ---x CD-ROM E-ROM bank (?)
 */
void pc8801_31_device::rom_bank_w(u8 data)
{
	m_rom_bank_cb(bool(BIT(data, 4)));
	
	if (data & 0xef)
		logerror("%s: rom_bank_w %02x\n", machine().describe_context(), data);
}
