// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor X37 SASI hard disk controller emulation

*********************************************************************/

#include "emu.h"
#include "x37_sasi.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(LUXOR_X37_SASI, luxor_x37_sasi_device, "luxor_x37_sasi", "Luxor X37 SASI")

void luxor_x37_sasi_device::device_add_mconfig(machine_config &config)
{
	auto &sasi(NSCSI_BUS(config, "sasi"));
	NSCSI_CONNECTOR(config, "sasi:4", default_scsi_devices, "s1410");
	sasi.set_external_device(7, *this);
}

luxor_x37_sasi_device::luxor_x37_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUXOR_X37_SASI, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	m_write_int(*this),
	m_write_req0(*this),
	m_buffer(*this, "buffer", 16*2, ENDIANNESS_BIG)
{
}

TIMER_CALLBACK_MEMBER(luxor_x37_sasi_device::clear_sel)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);
}

TIMER_CALLBACK_MEMBER(luxor_x37_sasi_device::set_ack)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
}

TIMER_CALLBACK_MEMBER(luxor_x37_sasi_device::clear_ack)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
}

void luxor_x37_sasi_device::device_start()
{
	m_sel_clear_timer = timer_alloc(FUNC(luxor_x37_sasi_device::clear_sel), this);
	m_ack_set_timer = timer_alloc(FUNC(luxor_x37_sasi_device::set_ack), this);
	m_ack_clear_timer = timer_alloc(FUNC(luxor_x37_sasi_device::clear_ack), this);

	save_item(NAME(m_int));
	save_item(NAME(m_brq));
	save_item(NAME(m_brc));
	save_item(NAME(m_data_out));
	save_item(NAME(m_a));
	save_item(NAME(m_hlc));
	save_item(NAME(m_dir));
	save_item(NAME(m_dxd8));
	save_item(NAME(m_bsy));
}

void luxor_x37_sasi_device::device_reset()
{
	m_sel_clear_timer->adjust(attotime::never);
	m_ack_set_timer->adjust(attotime::never);
	m_ack_clear_timer->adjust(attotime::never);

	m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY|S_INP|S_REQ, S_ALL);
}

void luxor_x37_sasi_device::scsi_ctrl_changed()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();

	bool const bsy = ctrl & S_BSY;
	if (m_bsy != bsy) {
		if (bsy) {
			// BSY 0->1
			m_sel_clear_timer->adjust(attotime::zero);
			update_int(CLEAR_LINE);
		} else {
			// BSY 1->0
			if (m_bsy && m_dxd8) {
				update_int(ASSERT_LINE);
			}
		}
		m_bsy = bsy;
	}

	bool const dir = ctrl & S_INP;
	if (m_dir != dir) {
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_dir = dir;
	}

	if (ctrl & S_REQ) {
		update_req0((m_brc == m_dir) ? ASSERT_LINE : CLEAR_LINE);
		if (!m_brq) {
			transfer();
		}
	} else {
		m_ack_clear_timer->adjust(attotime::zero);
	}
}

void luxor_x37_sasi_device::transfer()
{
	if (m_dir) {
		if (!m_hlc) {
			m_buffer[m_a] = (m_scsi_bus->data_r() << 8) | (m_buffer[m_a] & 0xff);
			LOG("reading high byte %01x:%02x from bus\n", m_a, m_buffer[m_a] >> 8);
		} else {
			m_buffer[m_a] = m_scsi_bus->data_r() | (m_buffer[m_a] & 0xff00);
			LOG("reading low byte %01x:%02x from bus\n", m_a, m_buffer[m_a] & 0xff);
		}
	} else {
		if (!m_hlc) {
			LOG("writing high byte %01x:%02x to bus\n", m_a, m_buffer[m_a] >> 8);
			m_scsi_bus->data_w(m_scsi_refid, m_buffer[m_a] >> 8);
		} else {
			LOG("writing low byte %01x:%02x to bus\n", m_a, m_buffer[m_a] & 0xff);
			m_scsi_bus->data_w(m_scsi_refid, m_buffer[m_a] & 0xff);
		}
	}

	if (m_dxd8) {
		m_hlc = !m_hlc;
	}

	if (!m_hlc) {
		count();
	}

	m_ack_set_timer->adjust(attotime::zero);
}

void luxor_x37_sasi_device::count()
{
	m_a++;

	if (m_a == 16) {
		m_a = 0;

		if (m_dxd8) {
			m_brc = !m_brc;
			LOG("brc %u dir %u\n", m_brc, m_dir);
			if (m_brc != m_dir) {
				update_req0(CLEAR_LINE);

				if (m_scsi_bus->ctrl_r() & S_REQ) {
					transfer();
				}
			}
		}
	}
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
		8		BRQ
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
	data |= !m_brq << 8;
	data |= !m_dir << 9;
	data |= (m_scsi_bus->ctrl_r() & S_BSY ? 0 : 1) << 14;
	data |= m_int << 15;

	LOG("%s stat_r: data=%04x\n", machine().describe_context(), data);

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
		13      REQ
		14		I/O
		15

	*/

	LOG("%s ctrl_w: data=%04x\n", machine().describe_context(), data);

	if (ACCESSING_BITS_0_7) {
		m_data_out = data & 0xff;
	}

	if (ACCESSING_BITS_8_15) {
		bool const dxd8 = BIT(data, 8);

		if (dxd8) {
			bool const bsy = m_scsi_bus->ctrl_r() & S_BSY;
			// DXD8 0->1
			if (!bsy && !m_dxd8) {
				m_scsi_bus->data_w(m_scsi_refid, m_data_out);
				m_scsi_bus->ctrl_w(m_scsi_refid, S_SEL, S_SEL);
			} else {
				m_scsi_bus->data_w(m_scsi_refid, 0);
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);
			}
		} else {
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);
			update_int(CLEAR_LINE);

			m_a = 0;
			m_hlc = 0;
			m_brc = 1;
			update_req0(CLEAR_LINE);
		}

		m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 9) ? 0 : S_RST, S_RST);
		m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 13) ? S_REQ : 0, S_REQ);
		m_scsi_bus->ctrl_w(m_scsi_refid, BIT(data, 14) ? S_INP : 0, S_INP);

		if (m_dxd8 != dxd8) {
			m_dxd8 = dxd8;
		}
	}
}

uint16_t luxor_x37_sasi_device::tre_r(offs_t offset, uint16_t mem_mask)
{
	u16 data = 0;
	if (m_dxd8)	{
		data = m_buffer[m_a];
		LOG("%s tre_r: dxd8=1 tre=%04x a=%01x\n", machine().describe_context(), data, m_a);
		count();
	} else {
		m_a = offset & 0xf;
		m_hlc = 0;
		data = m_buffer[m_a];
		LOG("%s tre_r: dxd8=0 tre=%04x a=%01x\n", machine().describe_context(), data, m_a);
	}
	return data;
}

void luxor_x37_sasi_device::tre_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_dxd8) {
		m_buffer[m_a] = data;
		LOG("%s tre_w: dxd8=1 tre=%04x a=%01x\n", machine().describe_context(), data, m_a);
		count();
	} else {
		m_a = offset & 0xf;
		m_hlc = 0;
		m_buffer[m_a] = data;
		LOG("%s tre_w: dxd8=0 tre=%04x a=%01x\n", machine().describe_context(), data, m_a);
	}
}
