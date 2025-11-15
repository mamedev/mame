// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC8801-31 CD-ROM I/F

TODO:
- Make it a slot option for PC-8801MA (does it have same ROM as the internal MC version?);
- Document BIOS program flow (PC=1000);

**************************************************************************************************/

#include "emu.h"
#include "pc8801_31.h"

#include "bus/nscsi/pc8801_30.h"

#include "speaker.h"

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
	, m_sasibus(*this, "sasi")
	, m_cddrive(*this, "sasi:0:cdrom")
	, m_sasi(*this, "sasi:7:sasicb")
	, m_rom_bank_cb(*this)
	, m_drq_cb(*this)
	, m_sel_off_timer(nullptr)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

static void pc8801_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_PC8801_30);
	// TODO: at very least HxC Floppy emulator option
}

void pc8801_31_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "headphone", 2).front_center();

	NSCSI_BUS(config, m_sasibus);
	NSCSI_CONNECTOR(config, "sasi:0").option_set("cdrom", NSCSI_CDROM_PC8801_30).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^headphone", 0.5, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^headphone", 0.5, 1);
		});
	NSCSI_CONNECTOR(config, "sasi:1", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:2", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:3", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:4", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:5", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:6", pc8801_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "sasicb", true)
		.option_add_internal("sasicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(pc8801_31_device::sasi_req_w));
			downcast<nscsi_callback_device&>(*device).sel_callback().set(*this, FUNC(pc8801_31_device::sasi_sel_w));
		});


	SOFTWARE_LIST(config, "cd_list").set_original("pc8801_cdrom");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc8801_31_device::device_start()
{
	m_sel_off_timer = timer_alloc(FUNC(pc8801_31_device::select_off_cb), this);

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

TIMER_CALLBACK_MEMBER(pc8801_31_device::select_off_cb)
{
	m_sasi->sel_w(0);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// base +$90
void pc8801_31_device::amap(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(pc8801_31_device::status_r), FUNC(pc8801_31_device::select_w));
	map(0x01, 0x01).rw(FUNC(pc8801_31_device::data_r), FUNC(pc8801_31_device::data_w));
	map(0x04, 0x04).w(FUNC(pc8801_31_device::scsi_reset_w));
	map(0x08, 0x08).r(FUNC(pc8801_31_device::clock_r)).w(m_cddrive, FUNC(nscsi_cdrom_pc8801_30_device::fader_control_w));
	map(0x09, 0x09).rw(FUNC(pc8801_31_device::id_r), FUNC(pc8801_31_device::rom_bank_w));
	map(0x0b, 0x0b).r(FUNC(pc8801_31_device::volume_meter_r<1>));
	map(0x0d, 0x0d).r(FUNC(pc8801_31_device::volume_meter_r<0>));
	map(0x0f, 0x0f).lw8(
		NAME([this](u8 data) {
			// takabako and dioscd disables DMA for SUBQ commands to work right
			m_dma_enable = !!(BIT(data, 6));
			m_cddrive_enable = !!(BIT(data, 0));
		})
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
u8 pc8801_31_device::status_r()
{
	u8 res = (m_sasi->bsy_r() << 7 |
		m_sasi->req_r() << 6 |
		m_sasi->msg_r() << 5 |
		m_sasi->cd_r() << 4 |
		m_sasi->io_r() << 3 |
		m_cddrive_enable);

	// at boot up SEL=1 drives BSY, MSG, CD and IO low
	// according to Takeda nobubufu (+ redbook CD) also wants this behaviour
	if (m_sasi_sel)
		res &= ~0xb8;

	return res;
}

void pc8801_31_device::select_w(u8 data)
{
	if (BIT(data, 0))
	{
		if (m_cddrive_enable)
		{
			m_sasi->sel_w(1);

			// TODO: timing
			m_sel_off_timer->adjust(attotime::from_usec(5000));
		}
	}
	else
		m_sasi->sel_w(0);
}

u8 pc8801_31_device::data_r()
{
	u8 res = m_sasi->read();

	//if (m_sasi->bsy_r() && m_sasi->io_r() && !machine().side_effects_disabled())
	if (!machine().side_effects_disabled())
	{
		m_sasi->ack_w(1);
		//m_sasi->write(0);
		m_sasi->ack_w(0);
	}
	return res;
}

void pc8801_31_device::data_w(u8 data)
{
	m_sasi->write(data);

	// do not guard against anything, just ack the byte
	// (mirrors cares after the Views logo)
	//if (m_sasi->bsy_r()) //&& !m_sasi->io_r())
	{
		m_sasi->ack_w(1);
		//m_sasi->write(0);
		m_sasi->ack_w(0);
	}
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

	// Update: pc8801_flop:dslayed is the odd one: if this is active it tries to load a redbook CD
	// even if one isn't inserted, hanging in the process. Sense for CD motor?
	m_clock_hb ^= 1;
	return m_clock_hb << 7;
}

/*
 * I/O Port $94
 *
 * x--- ---- to SCSI RST
 *
 */
void pc8801_31_device::scsi_reset_w(u8 data)
{
	if (BIT(data, 7))
		m_sasibus->reset();
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
 * ? may just be sign, ignored by HW
 *
 */
// TODO: measure via real HW tests, is $9c / $9e actually low CDDA readback?
template <unsigned N> u8 pc8801_31_device::volume_meter_r()
{
	return m_cddrive->get_channel_sample(N) >> 8;
}

void pc8801_31_device::sasi_sel_w(int state)
{
	m_sasi_sel = state;
}

void pc8801_31_device::sasi_req_w(int state)
{
	if (!m_sasi_req && state)
	{
		// IO needed otherwise it will keep running the DRQ
		if (m_dma_enable && !m_sasi->cd_r() && !m_sasi->msg_r() && m_sasi->io_r())
		{
			m_drq_cb(1);
		}
		// else if (m_sasi->cd_r())
		// 	m_irq_cb(1);
	}
	else if(m_sasi_req && !state)
	{
		//m_sasi->ack_w(0);
		m_drq_cb(0);
		// m_irq_cb(0);
	}

	m_sasi_req = state;
}

