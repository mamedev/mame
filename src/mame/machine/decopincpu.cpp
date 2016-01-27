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
 *   - make use of solenoid callbacks
 *   - printer option (type 3b)
 */

#include "decopincpu.h"

const device_type DECOCPU1 = &device_creator<decocpu_type1_device>;
const device_type DECOCPU2 = &device_creator<decocpu_type2_device>;
const device_type DECOCPU3 = &device_creator<decocpu_type3_device>;
const device_type DECOCPU3B = &device_creator<decocpu_type3b_device>;

static ADDRESS_MAP_START( decocpu1_map, AS_PROGRAM, 8, decocpu_type1_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_WRITE(solenoid2_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	//AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decocpu2_map, AS_PROGRAM, 8, decocpu_type2_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_WRITE(solenoid2_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	//AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( decocpu1 )
	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, decocpu_type1_device, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, decocpu_type1_device, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void decocpu_type1_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_cpu->set_input_line(M6800_IRQ_LINE,ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,E_CLOCK),0);
			m_irq_active = true;
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));
		}
		else
		{
			m_cpu->set_input_line(M6800_IRQ_LINE,CLEAR_LINE);
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
		m_cpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( decocpu_type1_device::audio_nmi )
{
	// Not on DECO board?
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

WRITE_LINE_MEMBER( decocpu_type1_device::pia21_ca2_w )
{
// sound ns
	m_ca2 = state;
}

WRITE8_MEMBER( decocpu_type1_device::lamp0_w )
{
	m_cpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
	m_write_lamp(0,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::lamp1_w )
{
	m_write_lamp(1,data,0xff);
}

READ8_MEMBER( decocpu_type1_device::display_strobe_r )
{
	UINT8 ret = 0x80;

	if(BIT(ioport("DIAGS")->read(), 4))  // W7 Jumper
		ret &= ~0x80;

	return ret | (m_read_display(0) & 0x7f);
}

WRITE8_MEMBER( decocpu_type1_device::display_strobe_w )
{
	m_write_display(0,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::display_out1_w )
{
	m_write_display(1,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::display_out2_w )
{
	m_write_display(2,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::display_out3_w )
{
	m_write_display(3,data,0xff);
}

READ8_MEMBER( decocpu_type1_device::display_in3_r )
{
	return m_read_display(3);
}

WRITE8_MEMBER( decocpu_type1_device::switch_w )
{
	m_write_switch(0,data,0xff);
}

READ8_MEMBER( decocpu_type1_device::switch_r )
{
	return m_read_switch(0);
}

READ8_MEMBER( decocpu_type1_device::dmdstatus_r )
{
	return m_read_dmdstatus(0);
}

WRITE8_MEMBER( decocpu_type1_device::display_out4_w )
{
	m_write_display(4,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::sound_w )
{
	m_write_soundlatch(0,data,0xff);
}

WRITE8_MEMBER( decocpu_type1_device::solenoid1_w )
{
	// todo
}

WRITE8_MEMBER( decocpu_type1_device::solenoid2_w )
{
	// todo
}

static MACHINE_CONFIG_FRAGMENT( decocpu1 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(decocpu1_map)

	/* Devices */
	MCFG_DEVICE_ADD("pia21", PIA6821, 0) // 5F - PIA at 0x2100
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(decocpu_type1_device, solenoid1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(decocpu_type1_device, pia21_ca2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_DEVICE_ADD("pia24", PIA6821, 0) // 11D - PIA at 0x2400
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(decocpu_type1_device, lamp0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(decocpu_type1_device, lamp1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_DEVICE_ADD("pia28", PIA6821, 0) // 11B - PIA at 0x2800
	MCFG_PIA_READPA_HANDLER(READ8(decocpu_type1_device, display_strobe_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(decocpu_type1_device, display_strobe_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(decocpu_type1_device, display_out1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_DEVICE_ADD("pia2c", PIA6821, 0) // 9B - PIA at 0x2c00
	MCFG_PIA_READPB_HANDLER(READ8(decocpu_type1_device, display_in3_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(decocpu_type1_device, display_out2_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(decocpu_type1_device, display_out3_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_DEVICE_ADD("pia30", PIA6821, 0) // 8H - PIA at 0x3000
	MCFG_PIA_READPA_HANDLER(READ8(decocpu_type1_device, switch_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(decocpu_type1_device, switch_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_DEVICE_ADD("pia34", PIA6821, 0) // 7B - PIA at 0x3400
	MCFG_PIA_READPA_HANDLER(READ8(decocpu_type1_device, dmdstatus_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(decocpu_type1_device, display_out4_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(decocpu_type1_device, sound_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(decocpu_type1_device, cpu_pia_irq))

	MCFG_NVRAM_ADD_1FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( decocpu2 )
	MCFG_FRAGMENT_ADD(decocpu1)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(decocpu2_map)
MACHINE_CONFIG_END

machine_config_constructor decocpu_type1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( decocpu1 );
}

ioport_constructor decocpu_type1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( decocpu1 );
}

decocpu_type1_device::decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECOCPU1, "Data East Pinball CPU Board Type 1", tag, owner, clock, "decocpu1", __FILE__),
		m_cpu(*this,"maincpu"),
		m_pia21(*this, "pia21"),
		m_pia24(*this, "pia24"),
		m_pia28(*this, "pia28"),
		m_pia2c(*this, "pia2c"),
		m_pia30(*this, "pia30"),
		m_pia34(*this, "pia34"),
		m_read_display(*this),
		m_write_display(*this),
		m_read_dmdstatus(*this),
		m_write_soundlatch(*this),
		m_read_switch(*this),
		m_write_switch(*this),
		m_write_lamp(*this),
		m_write_solenoid(*this)
{}

decocpu_type1_device::decocpu_type1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_cpu(*this,"maincpu"),
		m_pia21(*this, "pia21"),
		m_pia24(*this, "pia24"),
		m_pia28(*this, "pia28"),
		m_pia2c(*this, "pia2c"),
		m_pia30(*this, "pia30"),
		m_pia34(*this, "pia34"),
		m_read_display(*this),
		m_write_display(*this),
		m_read_dmdstatus(*this),
		m_write_soundlatch(*this),
		m_read_switch(*this),
		m_write_switch(*this),
		m_write_lamp(*this),
		m_write_solenoid(*this)
{}

void decocpu_type1_device::device_start()
{
	UINT8* ROM = memregion(m_cputag)->base();

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

	m_cpu->space(AS_PROGRAM).install_rom(0x4000,0xffff,ROM+0x4000);
}

void decocpu_type1_device::static_set_cpuregion(device_t &device, const char *tag)
{
	decocpu_type1_device &cpuboard = downcast<decocpu_type1_device &>(device);
	cpuboard.m_cputag = tag;
}

decocpu_type2_device::decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: decocpu_type1_device(mconfig, DECOCPU2, "Data East Pinball CPU Board Type 2", tag, owner, clock, "decocpu2", __FILE__)
{}

decocpu_type2_device::decocpu_type2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: decocpu_type1_device(mconfig, type, name, tag, owner, clock, shortname, source)
{}

machine_config_constructor decocpu_type2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( decocpu2 );
}

void decocpu_type2_device::device_start()
{
	decocpu_type1_device::device_start();
}

decocpu_type3_device::decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: decocpu_type2_device(mconfig, DECOCPU3, "Data East Pinball CPU Board Type 3", tag, owner, clock, "decocpu3", __FILE__)
{}

decocpu_type3_device::decocpu_type3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: decocpu_type2_device(mconfig, type, name, tag, owner, clock, shortname, source)
{}

void decocpu_type3_device::device_start()
{
	decocpu_type1_device::device_start();
}

decocpu_type3b_device::decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: decocpu_type3_device(mconfig, DECOCPU3, "Data East Pinball CPU Board Type 3B", tag, owner, clock, "decocpu3b", __FILE__)
{}

void decocpu_type3b_device::device_start()
{
	decocpu_type1_device::device_start();
}
