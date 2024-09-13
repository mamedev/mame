// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Risc 1MB/II (stylized "risc")

The chess engine in Mephisto Risc is also compatible with Tasc's The ChessMachine,
it is more or less equivalent to Gideon 3.0 (Risc 1MB) and Gideon 3.1 (Risc II),
see ROM defs for details. "Main" CPU is slow, but all the chess calculations are
done with the ARM.

Hardware notes:
- G65SC02P-4 @ 2.5MHz
- 128KB ROM
- Tasc ChessMachine EC PCB
- Mephisto modular display module
- Mephisto modular chessboard

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m6502/m65sc02.h"
#include "machine/74259.h"
#include "machine/chessmachine.h"
#include "machine/nvram.h"

// internal artwork
#include "mephisto_risc.lh"


namespace {

class risc_state : public driver_device
{
public:
	risc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_chessm(*this, "chessm"),
		m_rombank(*this, "rombank"),
		m_keys(*this, "KEY")
	{ }

	void mrisc(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<chessmachine_device> m_chessm;
	required_memory_bank m_rombank;
	required_ioport m_keys;

	void mrisc_mem(address_map &map);

	u8 keys_r(offs_t offset);
	u8 chessm_r();
	void chessm_w(u8 data);
};

void risc_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 risc_state::keys_r(offs_t offset)
{
	return ~(BIT(m_keys->read(), offset) << 7);
}

u8 risc_state::chessm_r()
{
	// d0: chessmachine data
	return m_chessm->data_r();
}

void risc_state::chessm_w(u8 data)
{
	// d0,d7: chessmachine data
	m_chessm->data0_w(BIT(data, 0));
	m_chessm->data1_w(BIT(data, 7));

	// d1: chessmachine reset
	m_chessm->reset_w(BIT(data, 1));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void risc_state::mrisc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0x2004, 0x2004).w("display", FUNC(mephisto_display2_device::io_w));
	map(0x2c00, 0x2c07).r(FUNC(risc_state::keys_r));
	map(0x2400, 0x2400).w("board", FUNC(mephisto_board_device::led_w));
	map(0x2800, 0x2800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x3000, 0x3000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x3800, 0x3800).w(FUNC(risc_state::chessm_w));
	map(0x3c00, 0x3c00).r(FUNC(risc_state::chessm_r));
	map(0x4000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("rombank");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mrisc )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("TRN / Pawn")      PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO / Knight")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM / Bishop")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS / Rook")      PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV / Queen")     PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("FCT / King")      PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT / New Game")  PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL / New Game")   PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void risc_state::mrisc(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, 10_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &risc_state::mrisc_mem);

	const attotime irq_period = attotime::from_hz(10_MHz_XTAL / 0x4000);
	m_maincpu->set_periodic_int(FUNC(risc_state::irq0_line_hold), irq_period);

	CHESSMACHINE(config, m_chessm, 14'000'000); // Mephisto manual says 14MHz (no XTAL)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");
	outlatch.parallel_out_cb().set_membank("rombank").rshift(6).mask(0x03).exor(0x01);

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_risc);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mrisc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	// contains ChessMachine engine at 0x0-0x03fff + 0x10000-0x1c74f, concatenate those sections and make a .bin file,
	// then it will work on ChessMachine software. It identifies as R E B E L ver. HG-021 03-04-92
	ROM_LOAD("meph-risci-v1-2.bin", 0x00000, 0x20000, CRC(19c6ab83) SHA1(0baab84e5aa6999c24250938d207145144945fd5) )
ROM_END

ROM_START( mrisc2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	// contains ChessMachine engine at 0x0-0x03fff + 0x10000-0x1cb7f, concatenate those sections and make a .bin file,
	// then it will work on ChessMachine software. It identifies as R E B E L ver. 2.31 22-07-93, world champion Madrid 1992
	ROM_LOAD("risc_2.31", 0x00000, 0x20000, CRC(9ecf9cd3) SHA1(7bfc628183037a172242c9589f15aca218d8fb12) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE   INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, mrisc,   0,       0,      mrisc,    mrisc, risc_state, empty_init, "Hegener + Glaser / Tasc", "Mephisto Risc 1MB", MACHINE_SUPPORTS_SAVE )
SYST( 1994, mrisc2,  mrisc,   0,      mrisc,    mrisc, risc_state, empty_init, "Hegener + Glaser / Tasc", "Mephisto Risc II",  MACHINE_SUPPORTS_SAVE )
