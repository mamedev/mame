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
	AM_RANGE( 0x2400, 0x2400 ) AM_DEVWRITE("board", mephisto_board_device, led_upd_w)
	AM_RANGE( 0x2800, 0x2800 ) AM_DEVWRITE("board", mephisto_board_device, mux_upd_w)
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
	AM_RANGE( 0x2400, 0x2400 ) AM_DEVWRITE("board", mephisto_board_device, led_upd_w)
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


/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     CLASS                   INIT COMPANY             FULLNAME                     FLAGS */
CONS( 1989, polgar,   0,       0,      polgar,    polgar,   mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Polgar",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, polgar10, polgar,  0,      polgar10,  polgar,   mephisto_polgar_state,  0,   "Hegener & Glaser", "Mephisto Polgar 10MHz",     MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1992, mrisc,    0,       0,      mrisc,     polgar,   mephisto_risc_state,    0,   "Hegener & Glaser", "Mephisto RISC 1MB",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1994, mrisc2,   mrisc,   0,      mrisc,     polgar,   mephisto_risc_state,    0,   "Hegener & Glaser", "Mephisto RISC II",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
