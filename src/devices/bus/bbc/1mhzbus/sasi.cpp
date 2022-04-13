// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Torch SCSI Host Adaptor

    Circuit diagram: http://mdfs.net/Info/Comp/Torch/scsi1.gif

    http://mdfs.net/Docs/Comp/BBC/Hardware/TorchSCSI

    Supported by Hard Disc Utilities:
                         Mb  ss  i  c  h s
      BASF 6188          11  256 9 360 4 32
      BASF 6185          20  256 9 440 6 32
      Mitsubishi MR522   19  256 9 612 4 32
      NEC D5126          19  256 9 615 4 32
      Rodime 200/10      10  256 9 320 4 32
      Rodime 200/20      20  256 9 320 8 32
      Rodime 202E        20  256 9 640 4 32
      Rodime 204E        40  256 9 640 8 32
      Vertex V130        23  256 9 987 5 32
      Vertex V150        38  256 9 987 5 32
      Vertex V170        53  256 9 987 7 32

**********************************************************************/

#include "emu.h"
#include "sasi.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_SASI, bbc_sasi_device, "bbc_sasi", "Torch SCSI Host Adaptor");
DEFINE_DEVICE_TYPE(BBC_TORCHHD, bbc_torchhd_device, "bbc_torchhd", "Torch Hard Disc Pack");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_sasi_device::device_add_mconfig(machine_config& config)
{
	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(bbc_sasi_device::req_w));
			downcast<nscsi_callback_device&>(*device).sel_callback().set(*this, FUNC(bbc_sasi_device::sel_w));
		});
}

void bbc_torchhd_device::device_add_mconfig(machine_config &config)
{
	bbc_sasi_device::device_add_mconfig(config);

	/* Xebec S1410 */
	subdevice<nscsi_connector>("sasi:0")->set_default_option("s1410");
	subdevice<nscsi_connector>("sasi:0")->set_fixed(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_sasi_device - constructor
//-------------------------------------------------

bbc_sasi_device::bbc_sasi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_sasi(*this, "sasi:7:scsicb")
	, m_sel_state(0)
{
}

bbc_sasi_device::bbc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sasi_device(mconfig, BBC_SASI, tag, owner, clock)
{
}

bbc_torchhd_device::bbc_torchhd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sasi_device(mconfig, BBC_TORCHHD, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_sasi_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_sel_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_sasi_device::device_reset()
{
	m_sasi->rst_w(1);
	m_sasi->rst_w(0);
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_sasi_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xf0:
		data = m_sasi->read();
		m_sasi->ack_w(1);
		break;
	case 0xf1:
		data = (m_sasi->req_r() << 0)
			| (m_sasi->ack_r() << 1)
			| (m_sasi->rst_r() << 2)
			| (!m_sel_state << 3)
			| (m_sasi->io_r() << 4)
			| (m_sasi->cd_r() << 5)
			| (m_sasi->msg_r() << 6)
			| (m_sasi->bsy_r() << 7);
		break;
	case 0xf2:
		m_sasi->ack_w(1);
		break;
	}
	return data;
}

void bbc_sasi_device::jim_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xf0:
		m_sasi->write(data);
		m_sasi->ack_w(1);
		break;
	case 0xf1:
		m_sasi->rst_w(1);
		m_sasi->rst_w(0);
		break;
	case 0xf2:
		m_sasi->sel_w(1);
		m_sel_state = 1;
		break;
	case 0xf3:
		m_sasi->sel_w(0);
		m_sel_state = 0;
		break;
	}
}

WRITE_LINE_MEMBER(bbc_sasi_device::req_w)
{
	m_sasi->ack_w(0);
}

WRITE_LINE_MEMBER(bbc_sasi_device::sel_w)
{
	m_sel_state = state;
}
