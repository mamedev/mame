// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC8801-31 CD-ROM I/F

    TODO:
    - Interface with PC8801-30 (the actual CD drive);
    - Make it a slot option for PC-8801MA (does it have same ROM as the internal MC version?)

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
	, m_scsibus(*this, "scsi")
	, m_rom_bank_cb(*this)
	, m_sel_off_timer(nullptr)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

static void pc8801_scsi_devices(device_slot_interface &device)
{
	// TODO: PC8801-30
	device.option_add("cdrom", NSCSI_CDROM);
	// TODO: at very least HxC Floppy emulator option
}

void pc8801_31_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", pc8801_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsi:1", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", pc8801_scsi_devices, nullptr);

	SOFTWARE_LIST(config, "cd_list").set_original("pc8801_cdrom");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc8801_31_device::device_resolve_objects()
{
	m_rom_bank_cb.resolve();
}


void pc8801_31_device::device_start()
{
	m_sel_off_timer = timer_alloc(FUNC(pc8801_31_device::select_off), this);

	save_item(NAME(m_clock_hb));
	save_item(NAME(m_cddrive_enable));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void pc8801_31_device::device_reset()
{
	m_clock_hb = false;
	m_cddrive_enable = false;

	m_sel_off_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(pc8801_31_device::select_off)
{
	m_scsibus->ctrl_w(0, 0, nscsi_device::S_SEL);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


void pc8801_31_device::amap(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(pc8801_31_device::scsi_status_r), FUNC(pc8801_31_device::scsi_sel_w));
	map(0x01, 0x01).lrw8(
		NAME([this]() { return m_scsibus->data_r(); }),
		NAME([this](u8 data) { m_scsibus->data_w(0, data); })
	);
	map(0x04, 0x04).w(FUNC(pc8801_31_device::scsi_reset_w));
	map(0x08, 0x08).rw(FUNC(pc8801_31_device::clock_r), FUNC(pc8801_31_device::volume_control_w));
	map(0x09, 0x09).rw(FUNC(pc8801_31_device::id_r), FUNC(pc8801_31_device::rom_bank_w));
	map(0x0b, 0x0b).r(FUNC(pc8801_31_device::volume_meter_r));
	map(0x0d, 0x0d).r(FUNC(pc8801_31_device::volume_meter_r));
	map(0x0f, 0x0f).lw8(
		NAME([this](u8 data) { m_cddrive_enable = bool(BIT(data, 0)); })
	);
}

/*
 * I/O Port $90
 *
 * x--- ---- BSY
 * -x-- ---- REQ
 * --x- ---- MSG
 * ---x ---- CD
 * ---- x--- IO
 * ---- ---x CD-ROM drive status (from $9f)
 *
 * BIOS expects a bit 6 to go high then checks if (MSG | CD | IO) == CD.
 * It eventually moves on floppy/BASIC fallback if this check fails.
 *
 */
u8 pc8801_31_device::scsi_status_r()
{
	u32 ctrl = m_scsibus->ctrl_r();
	return (
		(ctrl & nscsi_device::S_BSY ? 0x80 : 0x00) |
		(ctrl & nscsi_device::S_REQ ? 0x40 : 0x00) |
		(ctrl & nscsi_device::S_MSG ? 0x20 : 0x00) |
		(ctrl & nscsi_device::S_CTL ? 0x10 : 0x00) |
		(ctrl & nscsi_device::S_INP ? 0x08 : 0x00) |
		m_cddrive_enable
	);
}

void pc8801_31_device::scsi_sel_w(u8 data)
{
	bool sel_on = bool(BIT(data, 0));
	m_scsibus->ctrl_w(
		0,
		sel_on ? nscsi_device::S_SEL : 0,
		nscsi_device::S_SEL
	);

	// TODO: timing
	if (sel_on)
		m_sel_off_timer->adjust(attotime::from_usec(10));
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
 * I/O Port $94
 *
 * x--- ---- to SCSI RST
 *
 */
void pc8801_31_device::scsi_reset_w(u8 data)
{
	m_scsibus->ctrl_w(
		0,
		BIT(data, 7) ? nscsi_device::S_RST : 0,
		nscsi_device::S_RST
	);
}

u8 pc8801_31_device::id_r()
{
	// PC=A9AA CD-Player checks this against 0xcd, branches with $71 bit 0 set (N88 extended ROM bank)
	// Identifier for a 8801MC?
	logerror("%s: id_r\n", machine().describe_context());
	return 0xcd;
}

/*
 * I/O Port $99
 *
 * ---x ---- CD-ROM BIOS bank
 * ---- ---x CD-ROM E-ROM bank (?)
 *
 */
void pc8801_31_device::rom_bank_w(u8 data)
{
	m_rom_bank_cb(bool(BIT(data, 4)));

	if (data & 0xef)
		logerror("%s: rom_bank_w %02x\n", machine().describe_context(), data);
}

/*
 * I/O Port $9b / $9d CDDA Left/Right output meter
 *
 * ?xxx xx-- L/R metering
 *
 * M88 hardwires this as 0x3c, CD player shows arbitrary clamp with
 * timed flicker on ticks depending on the value read.
 * i.e. a value of 0x70 will make the uppermost tick to flicker more than 0x74,
 * while 0x7c won't flicker at all
 *
 */
// TODO: templatìze, measure via real HW tests
u8 pc8801_31_device::volume_meter_r()
{
	return 0;
}
