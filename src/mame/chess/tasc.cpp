// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/*******************************************************************************

Tasc ChessSystem

Commonly known as Tasc R30, it's basically a dedicated ChessMachine.
The King chess engines are also compatible with Tasc's The ChessMachine software
on PC, however the prototype Gideon 3.1(internally: 2.1, Rebel 2.01) is not.

The King 2.23 version was not sold to consumers. It has an opening book meant
for chesscomputer competitions.
For more information, see: http://chesseval.com/ChessEvalJournal/R30v223.htm

Gideon only uses 128KB program RAM, no matter the RAM configuration.

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

EPROMs are interchangeable between R30 and R40.

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
- more accurate dynamic cpu clock divider (same problem as in saitek/risc2500.cpp),
  sound pitch is correct now though
- does the R40 version have the same clock divider value?

BTANB:
- R40 calls itself "R30" on the system information screen (there is a photo of
  an R40 that does say "R40", but it appears to be a modified ROM)

*******************************************************************************/

#include "emu.h"

#include "cpu/arm/arm.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/smartboard.h"
#include "sound/dac.h"
#include "video/t6963c.h"

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
		m_boot_view(*this, "boot_view"),
		m_rom(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_nvram(*this, "nvram", 0x20000, ENDIANNESS_LITTLE),
		m_lcd(*this, "lcd"),
		m_smartboard(*this, "smartboard"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0U),
		m_out_leds(*this, "pled%u", 0U)
	{ }

	void tasc(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<arm_cpu_device> m_maincpu;
	memory_view m_boot_view;
	required_region_ptr<u32> m_rom;
	required_device<ram_device> m_ram;
	memory_share_creator<u8> m_nvram;
	required_device<lm24014h_device> m_lcd;
	required_device<tasc_sb30_device> m_smartboard;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<4> m_inputs;
	output_finder<2> m_out_leds;

	emu_timer *m_boot_timer;

	u32 m_control = 0;
	u32 m_prev_pc = 0;
	u64 m_prev_cycle = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	u32 input_r();
	u32 rom_r(offs_t offset);
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }

	TIMER_CALLBACK_MEMBER(disable_bootrom) { m_boot_view.select(1); }
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void tasc_state::machine_start()
{
	m_out_leds.resolve();

	m_boot_timer = timer_alloc(FUNC(tasc_state::disable_bootrom), this);
	m_boot_view[1].install_ram(0, m_ram->size() - 1, m_ram->pointer());

	// register for savestates
	save_item(NAME(m_control));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_prev_cycle));
}

void tasc_state::machine_reset()
{
	m_boot_view.select(0);
	m_boot_timer->adjust(attotime::never);

	m_prev_pc = m_maincpu->pc();
	m_prev_cycle = m_maincpu->total_cycles();
}

INPUT_CHANGED_MEMBER(tasc_state::change_cpu_freq)
{
	// R30 is 30MHz, R40 is 40MHz
	m_maincpu->set_unscaled_clock((newval & 1) ? 40_MHz_XTAL : 30_MHz_XTAL);
}



/*******************************************************************************
    I/O
*******************************************************************************/

u32 tasc_state::input_r()
{
	if (!machine().side_effects_disabled())
	{
		// disconnect bootrom from the bus after next opcode
		if (m_boot_timer->remaining().is_never())
			m_boot_timer->adjust(m_maincpu->cycles_to_attotime(10));

		m_maincpu->set_input_line(ARM_FIRQ_LINE, CLEAR_LINE);
	}

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

u32 tasc_state::rom_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		// handle dynamic cpu clock divider when accessing rom
		s64 diff = m_maincpu->total_cycles() - m_prev_cycle;
		u32 pc = m_maincpu->pc();

		if (diff > 0)
		{
			static constexpr int arm_branch_cycles = 3;
			static constexpr int arm_max_cycles = 17; // datablock transfer
			static constexpr int divider = -7 + 1;

			// this takes care of almost all cases, otherwise, total cycles taken can't be determined
			if (diff <= arm_branch_cycles || (diff <= arm_max_cycles && (pc - m_prev_pc) == 4 && (pc & ~0x02000000) == (offset * 4)))
				m_maincpu->adjust_icount(divider * (int)diff);
			else
				m_maincpu->adjust_icount(divider);
		}

		m_prev_cycle = m_maincpu->total_cycles();
		m_prev_pc = pc;
	}

	return m_rom[offset];
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void tasc_state::main_map(address_map &map)
{
	map(0x00000000, 0x007fffff).view(m_boot_view);
	m_boot_view[0](0x00000000, 0x0003ffff).r(FUNC(tasc_state::rom_r));

	map(0x01000000, 0x01000003).rw(FUNC(tasc_state::input_r), FUNC(tasc_state::control_w));
	map(0x02000000, 0x0203ffff).r(FUNC(tasc_state::rom_r));
	map(0x03000000, 0x0307ffff).rw(FUNC(tasc_state::nvram_r), FUNC(tasc_state::nvram_w)).umask32(0x000000ff);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tasc )
	PORT_START("IN.0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Play")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Menu")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left Clock")

	PORT_START("IN.3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Right Clock")

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, tasc_state, change_cpu_freq, 0)
	PORT_CONFSETTING(    0x00, "30MHz (R30)" )
	PORT_CONFSETTING(    0x01, "40MHz (R40)" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void tasc_state::tasc(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 30_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tasc_state::main_map);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL / 128); // 256Hz
	m_maincpu->set_periodic_int(FUNC(tasc_state::irq1_line_assert), irq_period);

	RAM(config, m_ram).set_extra_options("512K, 1M, 2M, 4M, 8M"); // see driver notes
	m_ram->set_default_size("512K");
	m_ram->set_default_value(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TASC_SB30(config, m_smartboard);
	subdevice<sensorboard_device>("smartboard:board")->set_nvram_enable(true);

	// video hardware
	LM24014H(config, m_lcd, 0);
	m_lcd->set_fs(1); // font size 6x8

	config.set_default_layout(layout_tascr30);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

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



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1995, tascr30,  0,       0,      tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.50)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
SYST( 1993, tascr30a, tascr30, 0,      tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.20)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
SYST( 1993, tascr30b, tascr30, 0,      tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (The King 2.23, TM version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING ) // competed in several chesscomputer tournaments
SYST( 1993, tascr30g, tascr30, 0,      tasc,    tasc,  tasc_state, empty_init, "Tasc", "ChessSystem R30 (Gideon 3.1, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING ) // made in 1993, later released in 2012
