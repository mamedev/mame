// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/******************************************************************************

Tasc ChessSystem

Commonly known as Tasc R30, it's basically a dedicated ChessMachine.
The King chess engines are also compatible with Tasc's The ChessMachine software
on PC, however the prototype Gideon 2.1(internally: Rebel 2.01) is not.

R30 hardware notes:
- ARM6 CPU(P60ARM/CG) @ 30MHz
- 256KB ROM, 512KB program RAM, 128KB permanent RAM
- Toshiba LCD drivers (3*T7778A, T7900, T6963C), TC5565AFL-15
- SB20 or SB30 "Smartboard" chessboard with piece recognition

R40 hardware notes:
- ARM6 CPU(VY86C061PSTC) @ 40MHz
- +512KB extra RAM
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
#include "machine/nvram.h"
#include "machine/smartboard.h"
#include "machine/timer.h"
#include "video/t6963c.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "tascr30.lh"


namespace {

class tasc_state : public driver_device
{
public:
	tasc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd"),
		m_smartboard(*this, "smartboard"),
		m_rom(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_mainram(*this, "mainram"),
		m_disable_bootrom(*this, "disable_bootrom"),
		m_inputs(*this, "IN.%u", 0U),
		m_out_leds(*this, "pled%u", 0U)
	{ }

	void tasc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<arm_cpu_device> m_maincpu;
	required_device<lm24014h_device> m_lcd;
	required_device<tasc_sb30_device> m_smartboard;
	required_region_ptr<u32> m_rom;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<u32> m_mainram;
	required_device<timer_device> m_disable_bootrom;
	required_ioport_array<4> m_inputs;
	output_finder<2> m_out_leds;

	void main_map(address_map &map);

	bool m_bootrom_enabled;
	uint32_t m_mux;
	TIMER_DEVICE_CALLBACK_MEMBER(disable_bootrom) { m_bootrom_enabled = false; }

	// I/O handlers
	DECLARE_READ32_MEMBER(bootrom_r);
	DECLARE_READ32_MEMBER(p1000_r);
	DECLARE_WRITE32_MEMBER(p1000_w);
};

void tasc_state::machine_start()
{
	m_out_leds.resolve();
	save_item(NAME(m_bootrom_enabled));
	save_item(NAME(m_mux));
}

void tasc_state::machine_reset()
{
	m_bootrom_enabled = true;
	m_mux = 0;
}



/******************************************************************************
    I/O
******************************************************************************/

READ32_MEMBER(tasc_state::bootrom_r)
{
	return (m_bootrom_enabled) ? m_rom[offset] : m_mainram[offset];
}

READ32_MEMBER(tasc_state::p1000_r)
{
	// disconnect bootrom from the bus after next opcode
	if (m_bootrom_enabled && !m_disable_bootrom->enabled() && !machine().side_effects_disabled())
		m_disable_bootrom->adjust(m_maincpu->cycles_to_attotime(5));

	uint32_t data = m_smartboard->read();

	for(int i=0; i<4; i++)
	{
		if (BIT(m_mux, i))
			data |= (m_inputs[i]->read() << 24);
	}

	return data;
}

WRITE32_MEMBER(tasc_state::p1000_w)
{
	if (ACCESSING_BITS_24_31)
	{
		if (BIT(data, 27))
			m_lcd->write(BIT(data, 26), data & 0xff);

		m_smartboard->write((data >> 24) & 0xff);
	}
	else
	{
		m_out_leds[0] = BIT(data, 0);
		m_out_leds[1] = BIT(data, 1);
		m_speaker->level_w((data >> 2) & 3);
	}

	COMBINE_DATA(&m_mux);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void tasc_state::main_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mainram");
	map(0x00000000, 0x0000000b).r(FUNC(tasc_state::bootrom_r));
	map(0x01000000, 0x01000003).rw(FUNC(tasc_state::p1000_r), FUNC(tasc_state::p1000_w));
	map(0x02000000, 0x0203ffff).rom().region("maincpu", 0);
	map(0x03000000, 0x0307ffff).ram().share("nvram").umask32(0x000000ff);
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( tasc )
	PORT_START("IN.0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)		PORT_NAME("PLAY")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)	PORT_NAME("LEFT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE)	PORT_NAME("BACK")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT)	PORT_NAME("RIGHT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M)		PORT_NAME("MENU")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)	PORT_NAME("UP")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)		PORT_NAME("Left Clock")

	PORT_START("IN.3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER)	PORT_NAME("ENTER")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN)	PORT_NAME("DOWN")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)		PORT_NAME("Right Clock")
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

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	LM24014H(config, m_lcd, 0);
	m_lcd->set_fs(1); // font size 6x8

	TASC_SB30(config, m_smartboard);

	config.set_default_layout(layout_tascr30);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	static const int16_t speaker_levels[4] = { 0, 32767, -32768, 0 };
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.75);
	m_speaker->set_levels(4, speaker_levels);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

#define ROM_LOAD32_WORD_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(bios))

#define ROM_LOAD32_WORD_SWAP_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(bios))

ROM_START( tascr30 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v25")
	ROM_SYSTEM_BIOS( 0, "v21", "System V0.31, Gideon 2.1" ) // 3-May-93, 3-Feb-93 (prototype, later released in 2012)
	ROM_SYSTEM_BIOS( 1, "v22", "System V0.31, The King 2.20" ) // 3-May-93, 23-Apr-93
	ROM_SYSTEM_BIOS( 2, "v223", "System V0.31, The King 2.23" ) // 3-May-93, 16-May-93 (unreleased)
	ROM_SYSTEM_BIOS( 3, "v25", "System V1.01, The King 2.50" ) // 17-Mar-95, 26-Feb-95

	ROM_LOAD32_WORD_BIOS(      0, "lo_21.bin",  0x00000, 0x20000, CRC(7041d051) SHA1(266843f375a8621320fc2cd1300775fb7a505c6e) )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "hi_21.bin",  0x00002, 0x20000, CRC(7345ee08) SHA1(9cad608bd32d804468b23196151be0a5f8cee214) )
	ROM_LOAD32_WORD_BIOS(      1, "lo_22.bin",  0x00000, 0x20000, CRC(d30f81fe) SHA1(81957c7266bedec66b2c14b97008c4261bd67828) )
	ROM_LOAD32_WORD_SWAP_BIOS( 1, "hi_22.bin",  0x00002, 0x20000, CRC(aeac3b46) SHA1(a757e0086636dfd3bf78e61cee46c7d92b39d3b9) )
	ROM_LOAD32_WORD_BIOS(      2, "lo_223.bin", 0x00000, 0x20000, CRC(37251b1a) SHA1(4be768e861002b20ba59a18329f488dba0a0c9bf) )
	ROM_LOAD32_WORD_SWAP_BIOS( 2, "hi_223.bin", 0x00002, 0x20000, CRC(e546be93) SHA1(943ae65cf97ec4389b9730c6006e805935333072) )
	ROM_LOAD32_WORD_BIOS(      3, "lo_25.bin",  0x00000, 0x20000, CRC(9711c158) SHA1(87c60d2097cb437482df11916543f6ef7f18b0d3) )
	ROM_LOAD32_WORD_SWAP_BIOS( 3, "hi_25.bin",  0x00002, 0x20000, CRC(df913abf) SHA1(1bc2ea4b6514bf9fec18f52c264f1440ba7c8c01) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1993, tascr30, 0,      0, tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND )
