// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/******************************************************************************

Tasc ChessSystem

Commonly known as Tasc R30, it's basically a dedicated ChessMachine.
The King chess engines are also compatible with Tasc's The ChessMachine software
on PC, however the prototype Gideon 2.1(internally: Rebel 2.01) is not.

The King 2.23 version was not released to the public. It has an opening book
meant for chesscomputer competitions.
For more information, see: http://chesseval.com/ChessEvalJournal/R30v223.htm

R30 hardware notes:
- ARM6 CPU(P60ARM/CG) @ 30MHz
- 256KB system ROM (2*27C010)
- 512KB program RAM (4*MT5C1008), 128KB permanent RAM (KM681000ALP-7L)
- Toshiba LCD drivers (3*T7778A, T7900, T6963C), TC5565AFL-15
- SB20 or SB30 "SmartBoard" chessboard with piece recognition

R40 hardware notes:
- ARM6 CPU(VY86C061PSTC) @ 40MHz
- +512KB extra RAM piggybacked
- rest same as R30

Documentation for the Toshiba chips is hard to find, but similar chips exist:
T7778 is equivalent to T6A39, T7900 is equivalent to T6A40.

references:
- https://www.schach-computer.info/wiki/index.php?title=Tasc_R30
- https://www.schach-computer.info/wiki/index.php?title=Tasc_R40
- https://www.schach-computer.info/wiki/index.php?title=Tasc_SmartBoard
- https://www.miclangschach.de/index.php?n=Main.TascR30

notes:
- holding LEFT+RIGHT on boot load the QC TestMode
- holding UP+DOWN on boot load the TestMode

TODO:
- bootrom disable timer shouldn't be needed, real ARM has already fetched the next opcode
- sound is too high pitched, same problem as in risc2500

******************************************************************************/

#include "emu.h"

#include "cpu/arm/arm.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/smartboard.h"
#include "machine/timer.h"
#include "video/t6963c.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "tascr30.lh"


namespace {

class tasc_state : public driver_device
{
public:
	tasc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_lcd(*this, "lcd"),
		m_smartboard(*this, "smartboard"),
		m_dac(*this, "dac"),
		m_disable_bootrom(*this, "disable_bootrom"),
		m_inputs(*this, "IN.%u", 0U),
		m_out_leds(*this, "pled%u", 0U)
	{ }

	void tasc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { install_bootrom(true); }
	virtual void device_post_load() override { install_bootrom(m_bootrom_enabled); }

private:
	// devices/pointers
	required_device<arm_cpu_device> m_maincpu;
	required_region_ptr<u32> m_rom;
	required_shared_ptr<u32> m_mainram;
	required_device<lm24014h_device> m_lcd;
	required_device<tasc_sb30_device> m_smartboard;
	required_device<dac_byte_interface> m_dac;
	required_device<timer_device> m_disable_bootrom;
	required_ioport_array<4> m_inputs;
	output_finder<2> m_out_leds;

	void main_map(address_map &map);
	void nvram_map(address_map &map);

	// I/O handlers
	u32 input_r();
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void install_bootrom(bool enable);
	void disable_bootrom_next();
	TIMER_DEVICE_CALLBACK_MEMBER(disable_bootrom) { install_bootrom(false); }
	bool m_bootrom_enabled = false;

	u32 m_control = 0;
};

void tasc_state::machine_start()
{
	m_out_leds.resolve();

	save_item(NAME(m_bootrom_enabled));
	save_item(NAME(m_control));
}



/******************************************************************************
    I/O
******************************************************************************/

// bootrom bankswitch

void tasc_state::install_bootrom(bool enable)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.unmap_readwrite(0, std::max(m_rom.bytes(), m_mainram.bytes()) - 1);

	if (enable)
		program.install_rom(0, m_rom.bytes() - 1, m_rom);
	else
		program.install_ram(0, m_mainram.bytes() - 1, m_mainram);

	m_bootrom_enabled = enable;
}

void tasc_state::disable_bootrom_next()
{
	// disconnect bootrom from the bus after next opcode
	if (m_bootrom_enabled && !m_disable_bootrom->enabled() && !machine().side_effects_disabled())
		m_disable_bootrom->adjust(m_maincpu->cycles_to_attotime(5));
}


// main I/O

u32 tasc_state::input_r()
{
	disable_bootrom_next();

	// read chessboard
	u32 data = m_smartboard->data_r();

	// read keypad
	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_control, i))
			data |= (m_inputs[i]->read() << 24);
	}

	return data;
}

void tasc_state::control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		if (BIT(data, 27))
			m_lcd->write(BIT(data, 26), data & 0xff);

		m_smartboard->data0_w(BIT(data, 30));
		m_smartboard->data1_w(BIT(data, 31));
	}
	else
	{
		m_out_leds[0] = BIT(data, 0);
		m_out_leds[1] = BIT(data, 1);
		m_dac->write((data >> 2) & 3);
	}

	COMBINE_DATA(&m_control);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void tasc_state::main_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mainram");
	map(0x01000000, 0x01000003).rw(FUNC(tasc_state::input_r), FUNC(tasc_state::control_w));
	map(0x02000000, 0x0203ffff).rom().region("maincpu", 0);
	map(0x03000000, 0x0307ffff).m("nvram_map", FUNC(address_map_bank_device::amap8)).umask32(0x000000ff);
}

void tasc_state::nvram_map(address_map &map)
{
	// nvram is 8-bit (128KB)
	map(0x00000, 0x1ffff).ram().share("nvram");
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( tasc )
	PORT_START("IN.0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)     PORT_NAME("PLAY")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("LEFT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("BACK")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("RIGHT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M)     PORT_NAME("MENU")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)    PORT_NAME("UP")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)     PORT_NAME("Left Clock")

	PORT_START("IN.3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("ENTER")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("DOWN")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)     PORT_NAME("Right Clock")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void tasc_state::tasc(machine_config &config)
{
	/* basic machine hardware */
	ARM(config, m_maincpu, 30_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tasc_state::main_map);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);
	m_maincpu->set_periodic_int(FUNC(tasc_state::irq1_line_hold), attotime::from_hz(32.768_kHz_XTAL/128)); // 256Hz

	TIMER(config, "disable_bootrom").configure_generic(FUNC(tasc_state::disable_bootrom));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	ADDRESS_MAP_BANK(config, "nvram_map").set_map(&tasc_state::nvram_map).set_options(ENDIANNESS_LITTLE, 8, 17);

	TASC_SB30(config, m_smartboard);
	subdevice<sensorboard_device>("smartboard:board")->set_nvram_enable(true);

	/* video hardware */
	LM24014H(config, m_lcd, 0);
	m_lcd->set_fs(1); // font size 6x8

	config.set_default_layout(layout_tascr30);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( tascr30 ) // system version V1.01 (17-Mar-95), program version 2.50 (26-Feb-95)
	ROM_REGION32_LE( 0x40000, "maincpu", 0 )
	ROM_LOAD32_WORD("r30_lo_1.01_king_2.5", 0x00000, 0x20000, CRC(9711c158) SHA1(87c60d2097cb437482df11916543f6ef7f18b0d3) )
	ROM_LOAD32_WORD_SWAP("r30_hi_1.01_king_2.5", 0x00002, 0x20000, CRC(df913abf) SHA1(1bc2ea4b6514bf9fec18f52c264f1440ba7c8c01) )
ROM_END

ROM_START( tascr30a ) // system version V0.31 (3-May-93), program version 2.20 (23-Apr-93)
	ROM_REGION32_LE( 0x40000, "maincpu", 0 )
	ROM_LOAD32_WORD("0.31_l", 0x00000, 0x20000, CRC(d30f81fe) SHA1(81957c7266bedec66b2c14b97008c4261bd67828) )
	ROM_LOAD32_WORD_SWAP("0.31_h", 0x00002, 0x20000, CRC(aeac3b46) SHA1(a757e0086636dfd3bf78e61cee46c7d92b39d3b9) )
ROM_END

ROM_START( tascr30b ) // system version V0.31 (3-May-93), program version 2.23 (16-May-93)
	ROM_REGION32_LE( 0x40000, "maincpu", 0 )
	ROM_LOAD32_WORD("r30_v2.23_lo", 0x00000, 0x20000, CRC(37251b1a) SHA1(4be768e861002b20ba59a18329f488dba0a0c9bf) )
	ROM_LOAD32_WORD_SWAP("r30_v2.23_hi", 0x00002, 0x20000, CRC(e546be93) SHA1(943ae65cf97ec4389b9730c6006e805935333072) )
ROM_END

ROM_START( tascr30g ) // system version V0.31 (3-May-93), program version 2.1 (3-Feb-93)
	ROM_REGION32_LE( 0x40000, "maincpu", 0 )
	ROM_LOAD32_WORD("r30_gideon_l", 0x00000, 0x20000, CRC(7041d051) SHA1(266843f375a8621320fc2cd1300775fb7a505c6e) )
	ROM_LOAD32_WORD_SWAP("r30_gideon_h", 0x00002, 0x20000, CRC(7345ee08) SHA1(9cad608bd32d804468b23196151be0a5f8cee214) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  CMP MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1995, tascr30,  0,       0, tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.50)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1993, tascr30a, tascr30, 0, tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.20)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )
CONS( 1993, tascr30b, tascr30, 0, tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.23, TM version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND ) // competed in several chesscomputer tournaments
CONS( 1993, tascr30g, tascr30, 0, tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (Gideon 2.1, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND ) // made in 1993, later released in 2012
