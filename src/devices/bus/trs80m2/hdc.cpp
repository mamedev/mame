// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 hard disk controller emulation

    Port    Read                    Write
    C0      write protect (D7/D6)
    C1      control                 control
    C4-C7   CTC                     CTC
    C8      sector buffer           sector buffer
    C9      error                   write precompensation
    CA-CE   WD1010 task file        WD1010 task file
    CF      status                  command

    Control register (C1):

    bit     description

    0       DMA enable
    1       interrupt enable
    3       DEVEN, WD1010 access enable
    4       SFTRST, controller reset

*********************************************************************/

#include "emu.h"
#include "hdc.h"

#include <algorithm>


namespace {

enum : uint8_t
{
	CONTROL_DMA    = 0x01,
	CONTROL_INT    = 0x02,
	CONTROL_DEVEN  = 0x08,
	CONTROL_SFTRST = 0x10
};

enum : uint8_t
{
	SDH_HEAD = 0x07
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(TRS80M2_HDC, trs80m2_hdc_device, "trs80m2_hdc", "Tandy Model II hard disk controller")


trs80m2_hdc_device::trs80m2_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_HDC, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_hdc(*this, "hdc"),
	m_ctc(*this, "ctc"),
	m_hdd(*this, "hdc:%u", 0U),
	m_ptr(0),
	m_control(0),
	m_sdh(0),
	m_intrq(false),
	m_bdrq(false)
{
	std::fill(std::begin(m_buf), std::end(m_buf), 0);
}

void trs80m2_hdc_device::device_add_mconfig(machine_config &config)
{
	WD1010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(FUNC(trs80m2_hdc_device::intrq_w));
	m_hdc->out_bdrq_callback().set(FUNC(trs80m2_hdc_device::bdrq_w));
	m_hdc->out_bcr_callback().set(FUNC(trs80m2_hdc_device::bcr_w));
	m_hdc->in_data_callback().set(FUNC(trs80m2_hdc_device::buf_in));
	m_hdc->out_data_callback().set(FUNC(trs80m2_hdc_device::buf_out));

	HARDDISK(config, m_hdd[0], "st506_hdd");
	HARDDISK(config, m_hdd[1], "st506_hdd");

	Z80CTC(config, m_ctc, DERIVED_CLOCK(1, 1));
	m_ctc->intr_callback().set([this] (int state) { m_slot->int_w(state); });
}

void trs80m2_hdc_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(trs80m2_hdc_device::wp_r));
	map(0x1, 0x1).rw(FUNC(trs80m2_hdc_device::control_r), FUNC(trs80m2_hdc_device::control_w));
	map(0x2, 0x3).r(FUNC(trs80m2_hdc_device::unused_r)).nopw();
	map(0x4, 0x7).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x8, 0x8).rw(FUNC(trs80m2_hdc_device::data_r), FUNC(trs80m2_hdc_device::data_w));
	map(0x9, 0xf).rw(FUNC(trs80m2_hdc_device::hdc_r), FUNC(trs80m2_hdc_device::hdc_w));
}

void trs80m2_hdc_device::device_start()
{
	m_slot->io().install_device(0xc0, 0xcf, *this, &trs80m2_hdc_device::map);

	save_item(NAME(m_buf));
	save_item(NAME(m_ptr));
	save_item(NAME(m_control));
	save_item(NAME(m_sdh));
	save_item(NAME(m_intrq));
	save_item(NAME(m_bdrq));
}

void trs80m2_hdc_device::device_reset()
{
	m_control = 0;
	m_ptr = 0;

	m_hdc->brdy_w(0);
	m_hdc->sc_w(1);
	update_drdy();

	m_ctc->trg0(0);
	m_slot->drq_w(0);
}

int trs80m2_hdc_device::z80daisy_irq_state()
{
	return static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_state();
}

int trs80m2_hdc_device::z80daisy_irq_ack()
{
	return static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_ack();
}

void trs80m2_hdc_device::z80daisy_irq_reti()
{
	static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_reti();
}

void trs80m2_hdc_device::update_drdy()
{
	int const drive = BIT(m_sdh, 3, 2);

	m_hdc->drdy_w((drive < 2 && m_hdd[drive]->exists()) ? 1 : 0);
}

uint8_t trs80m2_hdc_device::wp_r()
{
	uint8_t data = 0;

	if (m_hdd[0]->exists() && m_hdd[0]->is_readonly())
		data |= 0x80;

	if (m_hdd[1]->exists() && m_hdd[1]->is_readonly())
		data |= 0x40;

	return data;
}

uint8_t trs80m2_hdc_device::control_r()
{
	return m_control;
}

void trs80m2_hdc_device::control_w(uint8_t data)
{
	if (data & CONTROL_SFTRST)
	{
		m_hdc->mr_w(1);
		m_hdc->mr_w(0);
		m_ptr = 0;
		m_hdc->brdy_w(0);
		m_hdc->sc_w(1);
		update_drdy();
	}

	m_control = data & ~CONTROL_SFTRST;

	m_ctc->trg0((m_control & CONTROL_INT) && m_intrq);
	m_slot->drq_w((m_control & CONTROL_DMA) && m_bdrq);
}

uint8_t trs80m2_hdc_device::unused_r()
{
	return 0xff;
}

uint8_t trs80m2_hdc_device::data_r()
{
	uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];

	if (!machine().side_effects_disabled())
	{
		if ((m_ptr++ & (BUFFER_SIZE - 1)) == (BUFFER_SIZE - 1))
			m_hdc->brdy_w(1);
	}

	return data;
}

void trs80m2_hdc_device::data_w(uint8_t data)
{
	m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;

	if ((m_ptr++ & (BUFFER_SIZE - 1)) == (BUFFER_SIZE - 1))
		m_hdc->brdy_w(1);
}

uint8_t trs80m2_hdc_device::hdc_r(offs_t offset)
{
	if (!(m_control & CONTROL_DEVEN))
		return 0xff;

	return m_hdc->read(offset + 1);
}

void trs80m2_hdc_device::hdc_w(offs_t offset, uint8_t data)
{
	if (!(m_control & CONTROL_DEVEN))
		return;

	switch (offset + 1)
	{
	case 6:
		m_sdh = data;
		m_hdc->head_w(data & SDH_HEAD);
		update_drdy();
		break;

	case 7:
		m_hdc->brdy_w(0);
		m_ptr = 0;
		break;
	}

	m_hdc->write(offset + 1, data);
}

uint8_t trs80m2_hdc_device::buf_in()
{
	return m_buf[m_ptr++ & (BUFFER_SIZE - 1)];
}

void trs80m2_hdc_device::buf_out(uint8_t data)
{
	m_buf[m_ptr++ & (BUFFER_SIZE - 1)] = data;
}

void trs80m2_hdc_device::intrq_w(int state)
{
	m_intrq = state;
	m_ctc->trg0((m_control & CONTROL_INT) && m_intrq);
}

void trs80m2_hdc_device::bdrq_w(int state)
{
	m_bdrq = state;
	m_slot->drq_w((m_control & CONTROL_DMA) && m_bdrq);
}

void trs80m2_hdc_device::bcr_w(int state)
{
	if (state)
		m_ptr = 0;
}
