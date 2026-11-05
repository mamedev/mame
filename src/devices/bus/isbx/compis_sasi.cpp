// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Floppy Disk Controller with SASI host emulation

    No documentation of this board has been found. It is reconstructed
    from BIOS v3.03, which tells it from the older floppy board by the
    8272 not being mirrored at MCS0+2..6, and only drives the SASI
    host (INT EEh units 4-5, Xebec S1410 LUN 0-1) on this board:

    MCS0+0  R    8272 status
    MCS0+1  R/W  8272 data
    MCS0+7  R    board ID in bits 3-0
    MCS0+7  W    control: BIOS writes 00h for 250 kbps, 90h for 500 kbps

    MCS1+0  R/W  data, access asserts ACK
    MCS1+1  R    status: 0=REQ 1=I/O 2=C/D 3=BSY
    MCS1+2  R    drive type jumpers: 3-2=LUN 0 1-0=LUN 1
    MCS1+2  W    assert SEL, released by BSY
    MCS1+3  W    selection ID, active low (BIOS writes FCh)

    The BIOS never reads the message byte that follows the status
    byte, so the board acknowledges message-in phase by itself.

    The "slimline" disk's controller is unidentified. It takes the
    S1410 command set, but both the BIOS and WMAINT put head, cylinder
    and sector into the command block instead of a logical address.

**********************************************************************/

#include "emu.h"
#include "compis_sasi.h"

#include "bus/nscsi/devices.h"
#include "bus/nscsi/s1410.h"
#include "machine/nscsi_bus.h"

#include <algorithm>


namespace {

class nscsi_compis_s1410_device : public nscsi_s1410_device
{
public:
	nscsi_compis_s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// nscsi_full_device implementation
	virtual void scsi_command() override;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NSCSI_COMPIS_S1410, nscsi_s1410_device, nscsi_compis_s1410_device, "compis_s1410", "Compis slimline SASI controller")

namespace {

nscsi_compis_s1410_device::nscsi_compis_s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_s1410_device(mconfig, NSCSI_COMPIS_S1410, tag, owner, clock)
{
}

const tiny_rom_entry *nscsi_compis_s1410_device::device_rom_region() const
{
	return nullptr;
}

void nscsi_compis_s1410_device::device_add_mconfig(machine_config &config)
{
	nscsi_harddisk_device::device_add_mconfig(config);
}

void nscsi_compis_s1410_device::scsi_command()
{
	switch (m_scsi_cmdbuf[0])
	{
	case SC_FORMAT_UNIT:
	case SC_CHECK_TRACK_FORMAT:
	case SC_FORMAT_TRACK:
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
	case SC_FORMAT_ALT_TRACK:
		if (!(m_scsi_cmdbuf[1] >> 5) && image->exists())
		{
			const auto &info = image->get_info();
			const uint32_t head = m_scsi_cmdbuf[1] & 0x1f;
			const uint32_t cylinder = ((m_scsi_cmdbuf[2] & 0xc0) << 2) | m_scsi_cmdbuf[3];
			const uint32_t sector = m_scsi_cmdbuf[2] & 0x3f;

			if ((head >= info.heads) || (cylinder >= info.cylinders) || (sector >= info.sectors))
			{
				std::fill(std::begin(m_scsi_sense_buffer), std::end(m_scsi_sense_buffer), 0);
				m_scsi_sense_buffer[0] = SS_SECTOR_NOT_FOUND;
				scsi_status_complete(SS_CHECK_CONDITION);
				return;
			}

			const uint32_t address = ((cylinder * info.heads) + head) * info.sectors + sector;
			m_scsi_cmdbuf[1] = (m_scsi_cmdbuf[1] & 0xe0) | BIT(address, 16, 5);
			m_scsi_cmdbuf[2] = BIT(address, 8, 8);
			m_scsi_cmdbuf[3] = BIT(address, 0, 8);
		}
		break;
	}

	nscsi_s1410_device::scsi_command();
}

void compis_sasi_devices(device_slot_interface &device)
{
	device.option_add_internal("compis_s1410", NSCSI_COMPIS_S1410);
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_FDC_SASI, compis_fdc_sasi_device, "compis_fdc_sasi", "Compis FDC with SASI")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compis_fdc_sasi_device::device_add_mconfig(machine_config &config)
{
	compis_fdc_device::device_add_mconfig(config);

	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", compis_sasi_devices, "compis_s1410", true);
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this] (device_t *device)
		{
			nscsi_callback_device &cb = downcast<nscsi_callback_device &>(*device);
			cb.req_callback().set(*this, FUNC(compis_fdc_sasi_device::req_w));
			cb.io_callback().set(*this, FUNC(compis_fdc_sasi_device::io_w));
			cb.bsy_callback().set(*this, FUNC(compis_fdc_sasi_device::bsy_w));
		});
}


//-------------------------------------------------
//  INPUT_PORTS( compis_fdc_sasi )
//-------------------------------------------------

static INPUT_PORTS_START( compis_fdc_sasi )
	PORT_START("TYPE")
	PORT_DIPNAME( 0x0c, 0x0c, "LUN 0 Drive Type" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x03, 0x03, "LUN 1 Drive Type" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor compis_fdc_sasi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( compis_fdc_sasi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_fdc_sasi_device - constructor
//-------------------------------------------------

compis_fdc_sasi_device::compis_fdc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	compis_fdc_device(mconfig, COMPIS_FDC_SASI, tag, owner, clock),
	m_sasi(*this, "sasi:7:scsicb"),
	m_type(*this, "TYPE"),
	m_data(0),
	m_id(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_fdc_sasi_device::device_start()
{
	compis_fdc_device::device_start();

	save_item(NAME(m_data));
	save_item(NAME(m_id));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void compis_fdc_sasi_device::device_reset()
{
	compis_fdc_device::device_reset();

	m_fdc->set_rate(250000);

	m_sasi->sel_w(0);
	m_sasi->ack_w(0);
	drive_bus();
}


//-------------------------------------------------
//  drive_bus -
//-------------------------------------------------

void compis_fdc_sasi_device::drive_bus()
{
	if (m_sasi->sel_r())
		m_sasi->write(~m_id);
	else if (m_sasi->bsy_r() && !m_sasi->io_r())
		m_sasi->write(m_data);
	else
		m_sasi->write(0);
}


//-------------------------------------------------
//  req_w -
//-------------------------------------------------

void compis_fdc_sasi_device::req_w(int state)
{
	if (!state)
		m_sasi->ack_w(0);
	else if (m_sasi->msg_r() && m_sasi->io_r())
		m_sasi->ack_w(1);
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

void compis_fdc_sasi_device::io_w(int state)
{
	drive_bus();
}


//-------------------------------------------------
//  bsy_w -
//-------------------------------------------------

void compis_fdc_sasi_device::bsy_w(int state)
{
	if (state && m_sasi->sel_r())
		m_sasi->sel_w(0);

	drive_bus();
}


//-------------------------------------------------
//  mcs0_r - chip select 0 read
//-------------------------------------------------

uint8_t compis_fdc_sasi_device::mcs0_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 7)
	{
	case 0: data = m_fdc->msr_r(); break;
	case 1: data = m_fdc->fifo_r(); break;
	case 7: data = 0xf0; break;
	}

	return data;
}


//-------------------------------------------------
//  mcs0_w - chip select 0 write
//-------------------------------------------------

void compis_fdc_sasi_device::mcs0_w(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 1:
		m_fdc->fifo_w(data);
		break;

	case 7:
		m_fdc->set_rate(BIT(data, 7) ? 500000 : 250000);
		break;
	}
}


//-------------------------------------------------
//  mcs1_r - chip select 1 read
//-------------------------------------------------

uint8_t compis_fdc_sasi_device::mcs1_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 3)
	{
	case 0:
		data = m_sasi->read();
		if (!machine().side_effects_disabled() && m_sasi->req_r())
			m_sasi->ack_w(1);
		break;

	case 1:
		data = (m_sasi->req_r() << 0) | (m_sasi->io_r() << 1) | (m_sasi->cd_r() << 2) | (m_sasi->bsy_r() << 3);
		break;

	case 2:
		data = m_type->read();
		break;
	}

	return data;
}


//-------------------------------------------------
//  mcs1_w - chip select 1 write
//-------------------------------------------------

void compis_fdc_sasi_device::mcs1_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0:
		m_data = data;
		drive_bus();
		if (m_sasi->req_r())
			m_sasi->ack_w(1);
		break;

	case 2:
		if (!m_sasi->bsy_r())
		{
			m_sasi->write(~m_id);
			m_sasi->sel_w(1);
		}
		break;

	case 3:
		m_id = data;
		break;
	}
}
