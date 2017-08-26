// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**************************************************************************************************

	Mephisto Polgar and RISC

**************************************************************************************************/


#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/arm/arm.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "video/hd44780.h"
#include "screen.h"
#include "speaker.h"

#include "mephisto_lcd.lh"
#include "mephisto_academy.lh"
#include "mephisto_milano.lh"


class mephisto_polgar_state : public driver_device
{
public:
	mephisto_polgar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_keys(*this, "KEY")
	{ }

	DECLARE_WRITE8_MEMBER(polgar_led_w);
	DECLARE_READ8_MEMBER(polgar_keys_r);

protected:
	optional_ioport m_keys;
};

class mephisto_risc_state : public mephisto_polgar_state
{
public:
	mephisto_risc_state(const machine_config &mconfig, device_type type, const char *tag)
		: mephisto_polgar_state(mconfig, type, tag)
		, m_subcpu(*this, "subcpu")
		, m_rombank(*this, "rombank")
	{ }

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(latch0_r);
	DECLARE_WRITE8_MEMBER(latch0_w);
	DECLARE_WRITE8_MEMBER(latch1_w);
	DECLARE_READ8_MEMBER(latch1_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<arm_cpu_device> m_subcpu;
	required_memory_bank m_rombank;
	uint8_t m_bank;
	uint8_t m_com_latch0;
	uint8_t m_com_latch1;

	// ARM bootstrap HLE
	void arm_bootstrap(uint8_t data);
	TIMER_CALLBACK_MEMBER(clean_com_flag)	{ m_com_latch0 &= ~0x01; }

	emu_timer* m_arm_bootstrap_timer;
	uint16_t m_com_offset;
	uint8_t m_com_bits;
	uint8_t m_com_data;
};

class mephisto_milano_state : public mephisto_polgar_state
{
public:
	mephisto_milano_state(const machine_config &mconfig, device_type type, const char *tag)
		: mephisto_polgar_state(mconfig, type, tag)
		, m_board(*this, "board")
		, m_display(*this, "display")
	{ }

	DECLARE_READ8_MEMBER(milano_input_r);
	DECLARE_WRITE8_MEMBER(milano_led_w);
	DECLARE_WRITE8_MEMBER(milano_io_w);

protected:
	virtual void machine_reset() override;

private:
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display_modul_device> m_display;
	uint8_t m_led_latch;
};

class mephisto_academy_state : public mephisto_polgar_state
{
public:
	mephisto_academy_state(const machine_config &mconfig, device_type type, const char *tag)
		: mephisto_polgar_state(mconfig, type, tag)
		, m_board(*this, "board")
		, m_beeper(*this, "display:beeper")
	{ }

	INTERRUPT_GEN_MEMBER(academy_irq);
	DECLARE_WRITE8_MEMBER(academy_nmi_w);
	DECLARE_WRITE8_MEMBER(academy_beeper_w);
	DECLARE_WRITE8_MEMBER(academy_led_w);
	DECLARE_READ8_MEMBER(academy_input_r);

protected:
	virtual void machine_reset() override;

private:
	required_device<mephisto_board_device> m_board;
	required_device<beep_device> m_beeper;
	bool m_enable_nmi;
};

READ8_MEMBER(mephisto_polgar_state::polgar_keys_r)
{
	return (BIT(m_keys->read(), offset) << 7) | 0x7f;
}

WRITE8_MEMBER(mephisto_polgar_state::polgar_led_w)
{
	output().set_led_value(100 + offset, BIT(data, 7));
}

static ADDRESS_MAP_START(polgar_mem, AS_PROGRAM, 8, mephisto_polgar_state)
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2000, 0x2000 ) AM_DEVWRITE("display", mephisto_display_modul_device, latch_w)
	AM_RANGE( 0x2004, 0x2004 ) AM_DEVWRITE("display", mephisto_display_modul_device, io_w)
	AM_RANGE( 0x2400, 0x2400 ) AM_DEVWRITE("board", mephisto_board_device, led_w)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x2c00, 0x2c07 ) AM_READ(polgar_keys_r)
	AM_RANGE( 0x3000, 0x3000 ) AM_DEVREAD("board", mephisto_board_device, input_r)
	AM_RANGE( 0x3400, 0x3405 ) AM_WRITE(polgar_led_w)
	AM_RANGE( 0x4000, 0xffff ) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(mephisto_risc_state::bank_w)
{
	if      (offset == 0 &&  (data & 0x01))	m_bank &= ~0x01;
	else if (offset == 0 && !(data & 0x01))	m_bank |= 0x01;
	else if (offset == 1 &&  (data & 0x01))	m_bank |= 0x02;
	else if (offset == 1 && !(data & 0x01))	m_bank &= ~0x02;

	m_rombank->set_entry(m_bank);
}

void mephisto_risc_state::arm_bootstrap(uint8_t data)
{
	if (data & 0x02)
	{
		m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_com_offset = 0;
	}

	if (m_com_offset < 0x100 && ((m_com_latch1 ^ data) & 0x80))
	{
		m_com_data |= (data & 1) << (7-m_com_bits);
		m_com_bits++;

		if (m_com_bits == 8)
		{
			m_subcpu->space(AS_PROGRAM).write_byte(m_com_offset, m_com_data);
			m_com_bits = 0;
			m_com_data = 0;
			m_com_offset++;
			
			if (m_com_offset == 0x100)
				m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}

		if (m_com_offset < 0x100)
		{
			m_com_latch0 |= 0x01;
			m_arm_bootstrap_timer->adjust(attotime::from_usec(15));
		}
	}
}

WRITE8_MEMBER(mephisto_risc_state::latch1_w)
{
	arm_bootstrap(data);
	m_com_latch1 = data;
	m_subcpu->set_input_line(ARM_FIRQ_LINE, ASSERT_LINE);
}


READ8_MEMBER(mephisto_risc_state::latch1_r)
{
	return m_com_latch1;
}

WRITE8_MEMBER(mephisto_risc_state::latch0_w)
{
	m_subcpu->set_input_line(ARM_FIRQ_LINE, CLEAR_LINE);
	m_com_latch0 = data;
}

READ8_MEMBER(mephisto_risc_state::latch0_r)
{
	return m_com_latch0;
}

static ADDRESS_MAP_START(mrisc_mem, AS_PROGRAM, 8, mephisto_risc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2000, 0x2000 ) AM_DEVWRITE("display", mephisto_display_modul_device, latch_w)
	AM_RANGE( 0x2004, 0x2004 ) AM_DEVWRITE("display", mephisto_display_modul_device, io_w)
	AM_RANGE( 0x2c00, 0x2c07 ) AM_READ(polgar_keys_r)
	AM_RANGE( 0x2400, 0x2400 ) AM_DEVWRITE("board", mephisto_board_device, led_w)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x3000, 0x3000 ) AM_DEVREAD("board", mephisto_board_device, input_r)
	AM_RANGE( 0x3400, 0x3405 ) AM_WRITE(polgar_led_w)
	AM_RANGE( 0x3406, 0x3407 ) AM_WRITE(bank_w)
	AM_RANGE( 0x3800, 0x3800 ) AM_WRITE(latch1_w)
	AM_RANGE( 0x3c00, 0x3c00 ) AM_READ(latch0_r)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("rombank")
ADDRESS_MAP_END


static ADDRESS_MAP_START(mrisc_arm_mem, AS_PROGRAM, 32, mephisto_risc_state)
	AM_RANGE( 0x00000000, 0x000fffff )  AM_RAM
	AM_RANGE( 0x00400000, 0x007fffff )  AM_READWRITE8(latch1_r, latch0_w, 0x000000ff)
ADDRESS_MAP_END


READ8_MEMBER(mephisto_milano_state::milano_input_r)
{
	return m_board->input_r(space, offset) ^ 0xff;
}

WRITE8_MEMBER(mephisto_milano_state::milano_led_w)
{
	m_led_latch = data;
	m_board->mux_w(space, offset, data);
}

WRITE8_MEMBER(mephisto_milano_state::milano_io_w)
{
	if ((data & 0xf0) == 0x90 || (data & 0xf0) == 0x60)
	{
		uint8_t base = (data & 0xf0) == 0x90 ? 0 : 8;
		for(int i=0; i<8; i++)
			output().set_led_value(base + i, BIT(m_led_latch, i) ? 0 : 1);
	}
	else
	{
		for(int i=0; i<16; i++)
			output().set_led_value(i, 0);
	}
	
	m_display->io_w(space, offset, data & 0x0f);
}

static ADDRESS_MAP_START(milano_mem, AS_PROGRAM, 8, mephisto_milano_state)
	AM_RANGE( 0x0000, 0x1fbf ) AM_RAM AM_SHARE("nvram")

	AM_RANGE( 0x1fc0, 0x1fc0 ) AM_DEVWRITE("display", mephisto_display_modul_device, latch_w)
	AM_RANGE( 0x1fd0, 0x1fd0 ) AM_WRITE(milano_led_w)
	AM_RANGE( 0x1fe0, 0x1fe0 ) AM_READ(milano_input_r)
	AM_RANGE( 0x1fe8, 0x1fed ) AM_WRITE(polgar_led_w)
	AM_RANGE( 0x1fd8, 0x1fdf ) AM_READ(polgar_keys_r)
	AM_RANGE( 0x1ff0, 0x1ff0 ) AM_WRITE(milano_io_w)

	AM_RANGE( 0x2000, 0xffff ) AM_ROM
ADDRESS_MAP_END


INTERRUPT_GEN_MEMBER(mephisto_academy_state::academy_irq)
{
	if (m_enable_nmi)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(mephisto_academy_state::academy_nmi_w)
{
	m_enable_nmi = data != 0;
}

WRITE8_MEMBER(mephisto_academy_state::academy_beeper_w)
{
	m_beeper->set_state(BIT(data, 7) ? 0 : 1);
}

WRITE8_MEMBER(mephisto_academy_state::academy_led_w)
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			if (BIT(data, i))
				output().set_led_value(100 + j * 4 + i, BIT(data, 4 + j) ? 0 : 1);
		}
}

READ8_MEMBER(mephisto_academy_state::academy_input_r)
{
	uint8_t data;
	if (m_board->mux_r(space, offset) == 0xff)
		data = m_keys->read();
	else
		data = m_board->input_r(space, offset);

	return data ^ 0xff;
}

static ADDRESS_MAP_START(academy_mem, AS_PROGRAM, 8, mephisto_academy_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2400, 0x2400 ) AM_READ(academy_input_r)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_w)
	AM_RANGE( 0x2c00, 0x2c00 ) AM_DEVWRITE("board", mephisto_board_device, led_w)
	AM_RANGE( 0x3002, 0x3002 ) AM_WRITE(academy_beeper_w)
	AM_RANGE( 0x3001, 0x3001 ) AM_WRITE(academy_nmi_w)
	AM_RANGE( 0x3400, 0x3400 ) AM_WRITE(academy_led_w)
	AM_RANGE( 0x3800, 0x3801 ) AM_DEVREADWRITE("display:hd44780", hd44780_device, read, write)
	AM_RANGE( 0x4000, 0xffff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(monteciv_mem, AS_PROGRAM, 8, mephisto_polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(megaiv_mem, AS_PROGRAM, 8, mephisto_polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( polgar )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("Trn")    PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("Info")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("Mem")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("Pos")    PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("LEV")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("FCT")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)	  PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END

static INPUT_PORTS_START( monteciv )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("1 Pawn")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("2 Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("3 Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("4 Rook")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("5 Queen")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("6 King")   PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("7 Black")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("8 White")  PORT_CODE(KEYCODE_8)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("9 Book")   PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("0 Pos")    PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Mem")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Info")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Clear")    PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Level")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("Reset")    PORT_CODE(KEYCODE_DEL)
INPUT_PORTS_END

void mephisto_risc_state::machine_start()
{
	m_arm_bootstrap_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mephisto_risc_state::clean_com_flag), this));
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);

	save_item(NAME(m_bank));
	save_item(NAME(m_com_latch0));
	save_item(NAME(m_com_latch1));
	save_item(NAME(m_com_offset));
	save_item(NAME(m_com_bits));
	save_item(NAME(m_com_data));
}

void mephisto_risc_state::machine_reset()
{
	m_bank = 1;
	m_com_latch0 = 0;
	m_com_latch1 = 0;
	m_rombank->set_entry(m_bank);

	// ARM bootstrap HLE
	m_com_offset = 0;
	m_com_bits = 0;
	m_com_data = 0;
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void mephisto_milano_state::machine_reset()
{
	m_led_latch = 0;
}

void mephisto_academy_state::machine_reset()
{
	m_enable_nmi = true;
}

static MACHINE_CONFIG_START( polgar )
	MCFG_CPU_ADD("maincpu", M65C02, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(polgar_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_polgar_state, nmi_line_pulse, XTAL_4_9152MHz / (1 << 13))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mephisto_lcd)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( polgar10, polgar )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL_10MHz )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mrisc )
	MCFG_CPU_ADD("maincpu", M65C02, XTAL_10MHz / 4)		// G65SC02
	MCFG_CPU_PROGRAM_MAP(mrisc_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_risc_state, irq0_line_hold, (double)XTAL_10MHz / (1 << 14))

	MCFG_CPU_ADD("subcpu", ARM, XTAL_14MHz)      		// VY86C010
	MCFG_CPU_PROGRAM_MAP(mrisc_arm_mem)
	MCFG_ARM_COPRO(VL86C020)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mephisto_lcd)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( milano, polgar )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(milano_mem)

	MCFG_DEVICE_REMOVE("board")
	MCFG_MEPHISTO_BUTTONS_BOARD_ADD("board")
	MCFG_MEPHISTO_BOARD_DISABLE_LEDS(true)
	MCFG_DEFAULT_LAYOUT(layout_mephisto_milano)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( academy, polgar )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(academy_mem)

	MCFG_DEFAULT_LAYOUT(layout_mephisto_academy)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( monteciv )
	MCFG_CPU_ADD("maincpu", M65C02, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP( monteciv_mem )
	MCFG_CPU_PERIODIC_INT_DRIVER(mephisto_polgar_state, nmi_line_pulse, XTAL_4_9152MHz / (1 << 13))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( megaiv, monteciv )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL_4_9152MHz )
	MCFG_CPU_PROGRAM_MAP(megaiv_mem)
MACHINE_CONFIG_END

ROM_START(polgar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("polgar.bin", 0x0000, 0x10000, CRC(88d55c0f) SHA1(e86d088ec3ac68deaf90f6b3b97e3e31b1515913))
ROM_END	

ROM_START(polgar10)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v101", "V10.1" )
	ROMX_LOAD("polg_101.bin", 0x00000, 0x10000, CRC(8fb6afa4) SHA1(d1cf868302a665ff351686b26a149ced0045fc81), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v100", "V10.0" )
	ROMX_LOAD("polgar10.bin", 0x00000, 0x10000, CRC(7c1960d4) SHA1(4d15b51f9e6f7943815945cd56078ca512a964d4), ROM_BIOS(2))
ROM_END	

ROM_START(mrisc)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("Meph-RiscI-V1-2.bin", 0x00000, 0x20000, CRC(19c6ab83) SHA1(0baab84e5aa6999c24250938d207145144945fd5))

	ROM_REGION(0x80, "arm_bootstrap", 0)
	ROM_LOAD32_BYTE( "74s288.1", 0x00, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.2", 0x01, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.3", 0x02, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.4", 0x03, 0x20, NO_DUMP )
ROM_END	

ROM_START(mrisc2)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("Meph-RiscII-V2.bin", 0x00000, 0x20000, CRC(9ecf9cd3) SHA1(7bfc628183037a172242c9589f15aca218d8fb12))

	ROM_REGION(0x80, "arm_bootstrap", 0)
	ROM_LOAD32_BYTE( "74s288.1", 0x00, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.2", 0x01, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.3", 0x02, 0x20, NO_DUMP )
	ROM_LOAD32_BYTE( "74s288.4", 0x03, 0x20, NO_DUMP )
ROM_END	

ROM_START(academy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "en", "English" )
	ROMX_LOAD("acad4000.bin", 0x4000, 0x4000, CRC(ee1222b5) SHA1(98541d87755a7186b69b9723cc4adbd07f20f0e2), ROM_BIOS(1))
	ROMX_LOAD("acad8000.bin", 0x8000, 0x8000, CRC(a967922b) SHA1(1327903ff89bf96d72c930c400f367ae19e3ec68), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "de", "German" )
	ROMX_LOAD("acad4000_de.bin", 0x4000, 0x4000, CRC(fb4d83c4) SHA1(f5132042c3b5a17c173f81eaa57e313ff0bb848e), ROM_BIOS(2))
	ROMX_LOAD("acad8000_de.bin", 0x8000, 0x8000, CRC(478155db) SHA1(d363ab6d5bc0f47a6cdfa5132b77535ef8da8256), ROM_BIOS(2))
ROM_END

ROM_START(milano)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v102", "V1.02" )
	ROMX_LOAD("milano102.bin", 0x0000, 0x10000, CRC(0e9c8fe1) SHA1(e9176f42d86fe57e382185c703c7eff7e63ca711), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v101", "V1.01" )
	ROMX_LOAD("milano101.bin", 0x0000, 0x10000, CRC(22efc0be) SHA1(921607d6dacf72c0686b8970261c43e2e244dc9f), ROM_BIOS(2))
ROM_END

ROM_START(nshort)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nshort.bin", 0x00000, 0x10000, CRC(4bd51e23) SHA1(3f55cc1c55dae8818b7e9384b6b8d43dc4f0a1af))
ROM_END


ROM_START(megaiv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("megaiv.bin", 0x8000, 0x8000, CRC(dee355d2) SHA1(6bc79c0fb169020f017412f5f9696b9ecafbf99f))
ROM_END

ROM_START(monteciv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mciv.bin", 0x8000, 0x8000, CRC(c4887694) SHA1(7f482d2a40fcb3125266e7a5407da315b4f9b49c))
ROM_END

/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     CLASS                   INIT COMPANY             FULLNAME                     FLAGS */
CONS( 1989, polgar,   0,       0,      polgar,    polgar,   mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Polgar",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, polgar10, polgar,  0,      polgar10,  polgar,   mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Polgar 10MHz",     MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1992, mrisc,    0,       0,      mrisc,     polgar,   mephisto_risc_state,    0,   "Hegener & Glaser", "Mephisto RISC 1MB",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1994, mrisc2,   mrisc,   0,      mrisc,     polgar,   mephisto_risc_state,    0,   "Hegener & Glaser", "Mephisto RISC II",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

// not modular boards
CONS( 1989, academy,  0,       0,      academy,   polgar,   mephisto_academy_state, 0,   "Hegener & Glaser", "Mephisto Academy",          MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, milano,   0,       0,      milano,    polgar,   mephisto_milano_state,  0,   "Hegener & Glaser", "Mephisto Milano",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1993, nshort,   milano,  0,      milano,    polgar,   mephisto_milano_state,  0,   "Hegener & Glaser", "Mephisto Nigel Short",      MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, megaiv,   0,       0,      megaiv,    monteciv, mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Mega IV",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 1990, monteciv, 0,       0,      monteciv,  monteciv, mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Monte Carlo IV LE",MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
