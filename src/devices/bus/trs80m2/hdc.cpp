// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16B/6000 internal hard disk controller emulation

    8000202 (8898798, AX-9432): WD1010 (u26) with a WD1100-11 (u27)
    managed 1K sector buffer (u28), a Z80 CTC for interrupts on the
    host daisy chain, and two ST506 drive ports.

    Port    Read                    Write
    C0      control (as C1)
    C1      control                 control
    C2-C3
    C4-C7   CTC                     CTC
    C8      sector buffer           sector buffer
    C9      error                   write precompensation
    CA-CE   WD1010 task file        WD1010 task file
    CF      status                  command

    Control register (C1):

    bit     description

    0       DMA enable
    1       interrupt enable
    3       DEVEN, CTC and WD1010 access enable
    4       SFTRST, controller reset on the rising edge

*********************************************************************/

#include "emu.h"
#include "hdc.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_HDC, trs80m2_hdc_device, "trs80m2_hdc", "Tandy Model 16B/6000 hard disk controller")


//**************************************************************************
//  WD1010 CONTROLLER BASE
//**************************************************************************

trs80m2_wd1010_card_device_base::trs80m2_wd1010_card_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_hdc(*this, "hdc"),
	m_ctc(*this, "ctc"),
	m_config(*this, "CONFIG"),
	m_control(0),
	m_bdrq(false),
	m_ptr(0),
	m_sdh(0),
	m_bcs(false),
	m_ctc_int(false),
	m_installed(false)
{
	std::fill(std::begin(m_buf), std::end(m_buf), 0);
}

void trs80m2_wd1010_card_device_base::device_add_mconfig(machine_config &config)
{
	WD1010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->set_rotational_latency(attotime::from_hz(120));
	m_hdc->out_intrq_callback().set(FUNC(trs80m2_wd1010_card_device_base::intrq_w));
	m_hdc->out_bdrq_callback().set(FUNC(trs80m2_wd1010_card_device_base::bdrq_w));
	m_hdc->out_bcs_callback().set(FUNC(trs80m2_wd1010_card_device_base::bcs_w));
	m_hdc->out_bcr_callback().set(FUNC(trs80m2_wd1010_card_device_base::bcr_w));
	m_hdc->in_data_callback().set(FUNC(trs80m2_wd1010_card_device_base::buf_in));
	m_hdc->out_data_callback().set(FUNC(trs80m2_wd1010_card_device_base::buf_out));

	Z80CTC(config, m_ctc, DERIVED_CLOCK(1, 1));
	m_ctc->intr_callback().set(FUNC(trs80m2_wd1010_card_device_base::ctc_int_w));
}

void trs80m2_wd1010_card_device_base::map(address_map &map)
{
	map(0x2, 0x3).noprw();
	map(0x4, 0x7).lrw8(
			NAME([this] (offs_t offset) -> uint8_t { return (m_control & CONTROL_DEVEN) ? m_ctc->read(offset) : 0; }),
			NAME([this] (offs_t offset, uint8_t data) { if (m_control & CONTROL_DEVEN) m_ctc->write(offset, data); }));
	map(0x8, 0x8).rw(FUNC(trs80m2_wd1010_card_device_base::data_r), FUNC(trs80m2_wd1010_card_device_base::data_w));
	map(0x9, 0xf).rw(FUNC(trs80m2_wd1010_card_device_base::hdc_r), FUNC(trs80m2_wd1010_card_device_base::hdc_w));
}

void trs80m2_wd1010_card_device_base::device_start()
{
	save_item(NAME(m_buf));
	save_item(NAME(m_ptr));
	save_item(NAME(m_control));
	save_item(NAME(m_sdh));
	save_item(NAME(m_bdrq));
	save_item(NAME(m_bcs));
	save_item(NAME(m_ctc_int));
}

void trs80m2_wd1010_card_device_base::device_reset()
{
	if (!m_installed)
	{
		m_installed = true;

		static constexpr offs_t BASE[4] = { 0xc0, 0x70, 0x60, 0x50 };
		offs_t const base = BASE[m_config->read() & 0x03];

		m_slot->bus().io().install_device(base, base + 0x0f, *this, &trs80m2_wd1010_card_device_base::map);
	}

	m_control = 0;
	m_ptr = 0;

	m_hdc->brdy_w(0);
	m_hdc->sc_w(1);
	update_drdy();

	update_int();
	update_xferrq();
}

int trs80m2_wd1010_card_device_base::z80daisy_irq_state()
{
	return static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_state();
}

int trs80m2_wd1010_card_device_base::z80daisy_irq_ack()
{
	return static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_ack();
}

void trs80m2_wd1010_card_device_base::z80daisy_irq_reti()
{
	static_cast<device_z80daisy_interface &>(*m_ctc).z80daisy_irq_reti();
}

void trs80m2_wd1010_card_device_base::update_int()
{
	m_slot->int_w(m_ctc_int && (m_control & CONTROL_INT));
}

void trs80m2_wd1010_card_device_base::update_xferrq()
{
	m_slot->xferrq_w((m_control & CONTROL_DMA) && m_bdrq);
}

void trs80m2_wd1010_card_device_base::ctc_int_w(int state)
{
	m_ctc_int = state;
	update_int();
}

uint8_t trs80m2_wd1010_card_device_base::control_r()
{
	return m_control;
}

void trs80m2_wd1010_card_device_base::control_w(uint8_t data)
{
	if (!(m_control & CONTROL_SFTRST) && (data & CONTROL_SFTRST))
	{
		m_hdc->mr_w(1);
		m_hdc->mr_w(0);
		m_ptr = 0;
		m_hdc->brdy_w(0);
		m_hdc->sc_w(1);
		update_drdy();
	}

	m_control = data;

	update_int();
	update_xferrq();
}

uint16_t trs80m2_wd1010_card_device_base::sector_size() const
{
	static constexpr uint16_t SIZE[4] = { 256, 512, 1024, 128 };

	return SIZE[BIT(m_sdh, 5, 2)];
}

uint8_t trs80m2_wd1010_card_device_base::data_r()
{
	if (!(m_control & CONTROL_DEVEN))
		return 0;

	if (m_bcs)
		return 0x80;

	uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];

	if (!machine().side_effects_disabled())
	{
		if ((++m_ptr % sector_size()) == 0)
			m_hdc->brdy_w(1);
	}

	return data;
}

void trs80m2_wd1010_card_device_base::data_w(uint8_t data)
{
	if (!(m_control & CONTROL_DEVEN) || m_bcs)
		return;

	m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;

	if ((++m_ptr % sector_size()) == 0)
		m_hdc->brdy_w(1);
}

uint8_t trs80m2_wd1010_card_device_base::hdc_r(offs_t offset)
{
	if (!(m_control & CONTROL_DEVEN))
		return 0;

	if (m_bcs)
		return 0x80;

	return m_hdc->read(offset + 1);
}

void trs80m2_wd1010_card_device_base::hdc_w(offs_t offset, uint8_t data)
{
	if (!(m_control & CONTROL_DEVEN) || m_bcs)
		return;

	switch (offset + 1)
	{
	case 6:
		m_sdh = data;
		m_hdc->head_w(data & 0x07);
		update_drdy();
		break;

	case 7:
		m_hdc->brdy_w(0);
		m_ptr = 0;
		break;
	}

	m_hdc->write(offset + 1, data);
}

uint8_t trs80m2_wd1010_card_device_base::buf_in()
{
	return m_buf[m_ptr++ & (BUFFER_SIZE - 1)];
}

void trs80m2_wd1010_card_device_base::buf_out(uint8_t data)
{
	m_buf[m_ptr++ & (BUFFER_SIZE - 1)] = data;
}

void trs80m2_wd1010_card_device_base::bcs_w(int state)
{
	m_bcs = state;
}

void trs80m2_wd1010_card_device_base::bdrq_w(int state)
{
	m_bdrq = state;
	update_xferrq();
}

void trs80m2_wd1010_card_device_base::bcr_w(int state)
{
	if (state)
		m_ptr = 0;
}


//**************************************************************************
//  MODEL 16B/6000 INTERNAL CONTROLLER
//**************************************************************************

static INPUT_PORTS_START( trs80m2_hdc )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Port base" )
	PORT_CONFSETTING( 0x00, "C0-CF (E1-E2)" )
	PORT_CONFSETTING( 0x01, "70-7F (E1-E3)" )
	PORT_CONFSETTING( 0x02, "60-6F (E1-E4)" )
	PORT_CONFSETTING( 0x03, "50-5F (E1-E5)" )
INPUT_PORTS_END

ioport_constructor trs80m2_hdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_hdc );
}


trs80m2_hdc_device::trs80m2_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_wd1010_card_device_base(mconfig, TRS80M2_HDC, tag, owner, clock),
	m_hdd(*this, "hdc:%u", 0U)
{
}

void trs80m2_hdc_device::device_add_mconfig(machine_config &config)
{
	trs80m2_wd1010_card_device_base::device_add_mconfig(config);

	HARDDISK(config, m_hdd[0], "st506_hdd");
	HARDDISK(config, m_hdd[1], "st506_hdd");
}

void trs80m2_hdc_device::map(address_map &map)
{
	trs80m2_wd1010_card_device_base::map(map);

	map(0x0, 0x1).r(FUNC(trs80m2_hdc_device::control_r));
	map(0x1, 0x1).w(FUNC(trs80m2_hdc_device::control_w));
}

void trs80m2_hdc_device::update_drdy()
{
	int const drive = selected_drive();

	m_hdc->drdy_w((drive < 2 && m_hdd[drive]->exists()) ? 1 : 0);
}

void trs80m2_hdc_device::intrq_w(int state)
{
	m_ctc->trg0(state);
}
