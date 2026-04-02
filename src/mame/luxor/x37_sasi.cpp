// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor X37 SASI hard disk controller emulation

*********************************************************************/

#include "emu.h"
#include "x37_sasi.h"

DEFINE_DEVICE_TYPE(LUXOR_X37_SASI, luxor_x37_sasi_device, "luxor_x37_sasi", "Luxor X37 SASI")

void luxor_x37_sasi_device::device_add_mconfig(machine_config &config)
{
	auto &sasi(NSCSI_BUS(config, "sasi"));
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	sasi.set_external_device(7, *this);
}

luxor_x37_sasi_device::luxor_x37_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUXOR_X37_SASI, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	m_write_int(*this),
	m_write_req0(*this),
	m_buffer(*this, "buffer", 16*2, ENDIANNESS_LITTLE)
{
}

void luxor_x37_sasi_device::device_start()
{
	save_item(NAME(m_int));
	save_item(NAME(m_req0));
	save_item(NAME(m_data_out));
	save_item(NAME(m_a));
	save_item(NAME(m_hlc));
	save_item(NAME(m_dir));
	save_item(NAME(m_rc));
}

void luxor_x37_sasi_device::device_reset()
{
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY|S_REQ|S_CTL|S_MSG|S_INP, S_ALL);
}

void luxor_x37_sasi_device::scsi_ctrl_changed()
{
	if (m_scsi_bus->ctrl_r() & S_BSY)
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);

	m_dir = m_scsi_bus->ctrl_r() & S_INP ? 1 : 0;

	if (m_scsi_bus->ctrl_r() & S_REQ) {
		if (m_dir)
			if (m_hlc)
				m_buffer[m_a] = (m_scsi_bus->data_r() << 8) | (m_buffer[m_a] & 0xff);
			else
				m_buffer[m_a] = m_scsi_bus->data_r() | (m_buffer[m_a] & 0xff00);
		else
			if (m_hlc)
				m_scsi_bus->data_w(m_scsi_refid, m_buffer[m_a] >> 8);
			else
				m_scsi_bus->data_w(m_scsi_refid, m_buffer[m_a] & 0xff);

		m_hlc = !m_hlc;

		if (m_hlc) {
			m_rc = 0;
			m_a++;
			if (m_a == 16) {
				m_rc = 1;
				m_a = 0;
			}

			// TODO write_req0
		}

		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
	} else
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
}

uint16_t luxor_x37_sasi_device::stat_r(offs_t offset, uint16_t mem_mask)
{
	/*

		bit		description

		0		HLC
		1		SCSIA0
		2	  	SCSIA1
		3 		SCSIA2
		4 	    SCSIA3
		5		0
		6		0
		7		0
		8		*REQ0
		9		SCSIDIR
		10
		11
		12
		13
		14		BSY
		15		*SASIINT

	*/

	u16 data = 0;

	data |= m_hlc;
	data |= m_a << 1;
	data |= m_req0 << 8;
	data |= m_dir << 9;
	data |= m_scsi_bus->ctrl_r() & S_BSY ? 1 << 14 : 0;
	data |= m_int << 15;

	return data;
}

void luxor_x37_sasi_device::ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*

		bit		description

		0 		D0
		1		D1
		2		D2
		3		D3
		4		D4
		5		D5
		6		D6
		7		D7
		8		DXD8
		9		RST
		10
		11
		12
		13
		14		I/O
		15		REQ

	*/

	m_data_out = data & 0xff;

	if (BIT(data, 8)) {
		m_scsi_bus->data_w(m_scsi_refid, m_data_out);
		m_scsi_bus->ctrl_w(m_scsi_refid, S_SEL, S_SEL);
	} else {
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);

		m_a = offset & 0xf;
		m_hlc = 0;
	}

	m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 9) ? S_RST : 0, S_RST);
	m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 14) ? S_INP : 0, S_INP);
	m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 15) ? S_REQ : 0, S_REQ);
}

uint16_t luxor_x37_sasi_device::tre_r(offs_t offset, uint16_t mem_mask)
{
	u16 data = m_buffer[m_a];
	m_a++;
	m_a &= 0xf;
	return data;
}

void luxor_x37_sasi_device::tre_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_buffer[m_a] = data;
	m_a++;
	m_a &= 0xf;
}
