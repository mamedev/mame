// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball CPU boards
 *
 *  Type 1:  Based on Williams System 11 CPU boards, but without the generic pinball audio hardware
 *  Type 2:  RAM increased from 2kB to 8kB
 *  Type 3:  Adds CPU controlled solenoids
 *  Type 3b: Adds printer option
 *
 *  TODO:
 *   - printer option (type 3b)
 */

#include "emu.h"
#include "decopincpu.h"

DEFINE_DEVICE_TYPE(DECOCPU1,  decocpu_type1_device,  "decocpu1",  "Data East Pinball CPU Board Type 1")
DEFINE_DEVICE_TYPE(DECOCPU2,  decocpu_type2_device,  "decocpu2",  "Data East Pinball CPU Board Type 2")
DEFINE_DEVICE_TYPE(DECOCPU3,  decocpu_type3_device,  "decocpu3",  "Data East Pinball CPU Board Type 3")
DEFINE_DEVICE_TYPE(DECOCPU3B, decocpu_type3b_device, "decocpu3b", "Data East Pinball CPU Board Type 3B")

void decocpu_type1_device::decocpu1_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw("pia21", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(decocpu_type1_device::solenoid0_w)); // solenoids
	map(0x2400, 0x2403).rw("pia24", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw("pia28", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).rw("pia2c", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).rw("pia30", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).rw("pia34", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	//map(0x4000, 0xffff).rom();
}

void decocpu_type2_device::decocpu2_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2100, 0x2103).rw("pia21", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(decocpu_type2_device::solenoid0_w)); // solenoids
	map(0x2400, 0x2403).rw("pia24", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw("pia28", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).rw("pia2c", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).rw("pia30", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).rw("pia34", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	//map(0x4000, 0xffff).rom();
}

static INPUT_PORTS_START( decocpu1 )
	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, decocpu_type1_device, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void decocpu_type1_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_cpu->set_input_line(M6808_IRQ_LINE, ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,E_CLOCK),0);
			m_irq_active = true;
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));
		}
		else
		{
			m_cpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
			m_irq_active = false;
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

INPUT_CHANGED_MEMBER( decocpu_type1_device::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE_LINE_MEMBER(decocpu_type1_device::cpu_pia_irq)
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
		m_irq_active = false;
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
		m_irq_active = true;
	}
}

void decocpu_type1_device::lamp0_w(uint8_t data)
{
	m_cpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
	m_lamp_data = data ^ 0xff;
	m_write_lamp(0,data,0xff);
}

void decocpu_type1_device::lamp1_w(uint8_t data)
{
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);

	m_write_lamp(1,data,0xff);
}

uint8_t decocpu_type1_device::display_strobe_r()
{
	uint8_t ret = 0x80;

	if(BIT(ioport("DIAGS")->read(), 4))  // W7 Jumper
		ret &= ~0x80;

	return ret | (m_read_display(0) & 0x7f);
}

void decocpu_type1_device::display_strobe_w(uint8_t data)
{
	m_write_display(0,data,0xff);
}

void decocpu_type1_device::display_out1_w(uint8_t data)
{
	m_write_display(1,data,0xff);
}

void decocpu_type1_device::display_out2_w(uint8_t data)
{
	m_write_display(2,data,0xff);
}

void decocpu_type1_device::display_out3_w(uint8_t data)
{
	m_write_display(3,data,0xff);
}

uint8_t decocpu_type1_device::display_in3_r()
{
	return m_read_display(3);
}

void decocpu_type1_device::switch_w(uint8_t data)
{
	m_write_switch(0,data,0xff);
}

uint8_t decocpu_type1_device::switch_r()
{
	return m_read_switch(0);
}

uint8_t decocpu_type1_device::dmdstatus_r()
{
	return m_read_dmdstatus(0);
}

void decocpu_type1_device::display_out4_w(uint8_t data)
{
	m_write_display(4,data,0xff);
}

void decocpu_type1_device::sound_w(uint8_t data)
{
	m_write_soundlatch(0,data,0xff);
}

void decocpu_type1_device::solenoid1_w(uint8_t data)
{
	m_write_solenoid(1,data,0xff);
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i+8] = BIT(data, i);
}

void decocpu_type1_device::solenoid0_w(uint8_t data)
{
	m_write_solenoid(0,data,0xff);
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void decocpu_type1_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_cpu, XTAL(4'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &decocpu_type1_device::decocpu1_map);

	/* Devices */
	PIA6821(config, m_pia21, 0); // 5F - PIA at 0x2100
	m_pia21->writepb_handler().set(FUNC(decocpu_type1_device::solenoid1_w));
	m_pia21->cb2_handler().set(FUNC(decocpu_type1_device::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia21->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	PIA6821(config, m_pia24, 0); // 11D - PIA at 0x2400
	m_pia24->writepa_handler().set(FUNC(decocpu_type1_device::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(decocpu_type1_device::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(decocpu_type1_device::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(decocpu_type1_device::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia24->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	PIA6821(config, m_pia28, 0); // 11B - PIA at 0x2800
	m_pia28->readpa_handler().set(FUNC(decocpu_type1_device::display_strobe_r));
	m_pia28->writepa_handler().set(FUNC(decocpu_type1_device::display_strobe_w));
	m_pia28->writepb_handler().set(FUNC(decocpu_type1_device::display_out1_w));
	m_pia28->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia28->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	PIA6821(config, m_pia2c, 0); // 9B - PIA at 0x2c00
	m_pia2c->readpb_handler().set(FUNC(decocpu_type1_device::display_in3_r));
	m_pia2c->writepa_handler().set(FUNC(decocpu_type1_device::display_out2_w));
	m_pia2c->writepb_handler().set(FUNC(decocpu_type1_device::display_out3_w));
	m_pia2c->ca2_handler().set(FUNC(decocpu_type1_device::pia2c_ca2_w));
	m_pia2c->cb2_handler().set(FUNC(decocpu_type1_device::pia2c_cb2_w));
	m_pia2c->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia2c->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	PIA6821(config, m_pia30, 0); // 8H - PIA at 0x3000
	m_pia30->readpa_handler().set(FUNC(decocpu_type1_device::switch_r));
	m_pia30->writepb_handler().set(FUNC(decocpu_type1_device::switch_w));
	m_pia30->ca2_handler().set(FUNC(decocpu_type1_device::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(decocpu_type1_device::pia30_cb2_w));
	m_pia30->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia30->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	PIA6821(config, m_pia34, 0); // 7B - PIA at 0x3400
	m_pia34->readpa_handler().set(FUNC(decocpu_type1_device::dmdstatus_r));
	m_pia34->writepa_handler().set(FUNC(decocpu_type1_device::display_out4_w));
	m_pia34->writepb_handler().set(FUNC(decocpu_type1_device::sound_w));
	m_pia34->cb2_handler().set_nop();
	m_pia34->irqa_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));
	m_pia34->irqb_handler().set(FUNC(decocpu_type1_device::cpu_pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

ioport_constructor decocpu_type1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( decocpu1 );
}

decocpu_type1_device::decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type1_device(mconfig, DECOCPU1, tag, owner, clock)
{}

decocpu_type1_device::decocpu_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
		, m_cpu(*this,"maincpu")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia2c(*this, "pia2c")
		, m_pia30(*this, "pia30")
		, m_pia34(*this, "pia34")
		, m_rom(*this, finder_base::DUMMY_TAG)
		, m_read_display(*this)
		, m_write_display(*this)
		, m_read_dmdstatus(*this)
		, m_write_soundlatch(*this)
		, m_read_switch(*this)
		, m_write_switch(*this)
		, m_write_lamp(*this)
		, m_write_solenoid(*this)
		, m_io_outputs(*this, "out%d", 0U)
{}

void decocpu_type1_device::device_start()
{
	// resolve callbacks
	m_read_display.resolve_safe(0);
	m_write_display.resolve_safe();
	m_read_dmdstatus.resolve_safe(0);
	m_write_soundlatch.resolve_safe();
	m_read_switch.resolve_safe(0);
	m_write_switch.resolve_safe();
	m_write_lamp.resolve_safe();
	m_write_solenoid.resolve_safe();

	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
	m_irq_active = false;

	m_cpu->space(AS_PROGRAM).install_rom(0x4000,0xffff,&m_rom[0x4000]);

	m_io_outputs.resolve();
	save_item(NAME(m_lamp_data));
}

decocpu_type2_device::decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type2_device(mconfig, DECOCPU2, tag, owner, clock)
{}

decocpu_type2_device::decocpu_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type1_device(mconfig, type, tag, owner, clock)
{}

void decocpu_type2_device::device_add_mconfig(machine_config &config)
{
	decocpu_type1_device::device_add_mconfig(config);

	/* basic machine hardware */
	m_cpu->set_addrmap(AS_PROGRAM, &decocpu_type2_device::decocpu2_map);
}

void decocpu_type2_device::device_start()
{
	decocpu_type1_device::device_start();
}

decocpu_type3_device::decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type3_device(mconfig, DECOCPU3, tag, owner, clock)
{}

decocpu_type3_device::decocpu_type3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type2_device(mconfig, type, tag, owner, clock)
{}

void decocpu_type3_device::device_start()
{
	decocpu_type1_device::device_start();
}

decocpu_type3b_device::decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: decocpu_type3_device(mconfig, DECOCPU3B, tag, owner, clock)
{}

void decocpu_type3b_device::device_start()
{
	decocpu_type1_device::device_start();
}
