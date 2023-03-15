// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Bingo Circus Terminal

    © 1989 Sega

    Hardware:
    - 2x Z0840008PSC
    - 32 MHz XTAL
    - MB8464A-15LL-SK (next to EPR-12646)
    - 2x ASSP 5C68A
    - 4x HM65256BLSP (next to 5C68A)
    - 315-5338
    - D71051C
    - D71054C
    - MB8464A-15LL-SK (near 5C68A and 315-5330)
    - 315-5330
    - HM62256BLSP-10 (next to 315-5330)
    - 315-5246
    - 2x DIPSW8

    PCB 837-7151

    TODO:
    - Serial connection to host system
    - Artwork, lamps and LED digits

    Notes:
    - No schematics, everything is guessed

***************************************************************************/

#include "emu.h"
#include "bingoct.h"
#include "315_5338a.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "sound/rf5c68.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BINGOCT, bingoct_device, "bingoct", "Bingo Circus Terminal")

//-------------------------------------------------
//  maincpu memory maps
//-------------------------------------------------

void bingoct_device::main_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("gfxbank");
	map(0xe000, 0xffff).ram().share("nvram");
}

void bingoct_device::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).lw8(NAME([this] (uint8_t data) { m_gfxbank->set_entry(data & 0x07); }));
	map(0x20, 0x23).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x40, 0x43).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x6f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
	map(0x84, 0x84).rw(m_vdp, FUNC(sega315_5246_device::data_read), FUNC(sega315_5246_device::data_write));
	map(0x85, 0x85).rw(m_vdp, FUNC(sega315_5246_device::control_read), FUNC(sega315_5246_device::control_write));
}

//-------------------------------------------------
//  soundcpu memory maps
//-------------------------------------------------

void bingoct_device::sound_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("soundcpu", 0);
	map(0x8000, 0x9fff).bankr("soundbank");
	map(0xa000, 0xa7ff).ram();
	map(0xc000, 0xdfff).m("5c68_1", FUNC(rf5c68_device::map));
	map(0xe000, 0xffff).m("5c68_2", FUNC(rf5c68_device::map));
}

void bingoct_device::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).lw8(NAME([this] (uint8_t data) { m_soundbank->set_entry(data & 0x3f); }));
	map(0x80, 0x80).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
}

void bingoct_device::pcm_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( terminal )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("epr-12646.ic20", 0x00000, 0x20000, CRC(c52e31a2) SHA1(901e84f3c9b65f207f7614d64e685e762b23987e))

	ROM_REGION(0x80000, "soundcpu", 0)
	ROM_LOAD("epr-12647.ic24", 0x00000, 0x20000, CRC(33198811) SHA1(6fb9db294a7f40303f22f68c3822e67cbd3560fa))
	ROM_LOAD("epr-12648.ic25", 0x20000, 0x20000, CRC(a34737e5) SHA1(76feec0091afb92af8ced99af61495f28f981120))
	// two empty sockets
ROM_END

const tiny_rom_entry *bingoct_device::device_rom_region() const
{
	return ROM_NAME(terminal);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( terminal )
	PORT_START("dipsw1")
	PORT_DIPNAME(0x0f, 0x0f, "Coin Mode") PORT_DIPLOCATION("DIPSW1:1,2,3,4")
	PORT_DIPSETTING(0x0f, "1 Credit")
	PORT_DIPSETTING(0x0e, "2 Credits")
	PORT_DIPSETTING(0x0d, "4 Credits")
	PORT_DIPSETTING(0x0c, "5 Credits")
	PORT_DIPSETTING(0x0b, "10 Credits")
	PORT_DIPSETTING(0x0a, "20 Credits")
	PORT_DIPSETTING(0x09, "25 Credits")
	PORT_DIPSETTING(0x08, "50 Credits")
	PORT_DIPSETTING(0x07, "100 Credits")
	PORT_DIPSETTING(0x06, "200 Credits")
	PORT_DIPSETTING(0x05, "250 Credits")
	PORT_DIPSETTING(0x04, "1 Credit (Invalid)")
	PORT_DIPSETTING(0x03, "1 Credit (Invalid)")
	PORT_DIPSETTING(0x02, "1 Credit (Invalid)")
	PORT_DIPSETTING(0x01, "1 Credit (Invalid)")
	PORT_DIPSETTING(0x00, "1 Credit (Invalid)")
	PORT_DIPNAME(0x70, 0x70, "Service Mode") PORT_DIPLOCATION("DIPSW1:5,6,7")
	PORT_DIPSETTING(0x70, "1 Credit")
	PORT_DIPSETTING(0x60, "2 Credits")
	PORT_DIPSETTING(0x50, "5 Credits")
	PORT_DIPSETTING(0x40, "10 Credits")
	PORT_DIPSETTING(0x30, "20 Credits")
	PORT_DIPSETTING(0x20, "25 Credits")
	PORT_DIPSETTING(0x10, "50 Credits")
	PORT_DIPSETTING(0x00, "100 Credits")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "DIPSW1:8")

	PORT_START("dipsw2")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "DIPSW2:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "DIPSW2:2")
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "DIPSW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "DIPSW2:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "DIPSW2:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "DIPSW2:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "DIPSW2:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "DIPSW2:8")

	PORT_START("in0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(KEYCODE_Z) PORT_NAME("Magic Screen A")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(KEYCODE_X) PORT_NAME("Magic Screen B")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(KEYCODE_C) PORT_NAME("Magic Screen C")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_CODE(KEYCODE_V) PORT_NAME("Magic Screen D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_CODE(KEYCODE_B) PORT_NAME("10 Bet")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_CODE(KEYCODE_N) PORT_NAME("1 Bet")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME("Credit")

	PORT_START("in1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SERVICE)        PORT_NAME("Test")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT)  PORT_NAME("Key")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE) PORT_NAME("Service")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)          PORT_NAME("Coin")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM)         PORT_NAME("Hopper Pool Sensor")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor bingoct_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(terminal);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bingoct_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 32_MHz_XTAL / 4); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &bingoct_device::main_mem_map);
	m_maincpu->set_addrmap(AS_IO, &bingoct_device::main_io_map);

	Z80(config, m_soundcpu, 32_MHz_XTAL / 4); // divider unknown
	m_soundcpu->set_addrmap(AS_PROGRAM, &bingoct_device::sound_mem_map);
	m_soundcpu->set_addrmap(AS_IO, &bingoct_device::sound_io_map);
	m_soundcpu->set_irq_acknowledge_callback(FUNC(bingoct_device::soundcpu_irq_ack));

	NVRAM(config, "nvram");

	GENERIC_LATCH_8(config, "soundlatch");

	pit8254_device &pit(PIT8254(config, "pit", 32_MHz_XTAL / 4)); // unknown clock
	pit.set_clk<0>(32_MHz_XTAL / 4); // unknown clock
	pit.set_clk<1>(32_MHz_XTAL / 4); // unknown clock
	pit.out_handler<0>().set("pit", FUNC(pit8254_device::write_clk2));
	pit.out_handler<2>().set_inputline(m_soundcpu, INPUT_LINE_IRQ0, ASSERT_LINE);

	I8251(config, "uart", 32_MHz_XTAL / 4); // unknown clock

	sega_315_5338a_device &io(SEGA_315_5338A(config, "io", 32_MHz_XTAL));
	io.in_pa_callback().set_ioport("dipsw1");
	io.in_pb_callback().set_ioport("dipsw2");
	io.in_pc_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	io.out_pc_callback().set("soundlatch", FUNC(generic_latch_8_device::write));
	io.in_pe_callback().set_ioport("in1");
	io.read_callback().set_ioport("in0");
	io.write_callback().set(FUNC(bingoct_device::output_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(10'738'635)/2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 192);
	screen.set_screen_update("vdp", FUNC(sega315_5246_device::screen_update));

	SEGA315_5246(config, m_vdp, XTAL(10'738'635));
	m_vdp->set_screen("screen");
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	rf5c68_device &rf5c68_1(RF5C68(config, "5c68_1", 32000000 / 4));
	rf5c68_1.add_route(ALL_OUTPUTS, "mono", 1.0);
	rf5c68_1.set_addrmap(0, &bingoct_device::pcm_map);

	rf5c68_device &rf5c68_2(RF5C68(config, "5c68_2", 32000000 / 4));
	rf5c68_2.add_route(ALL_OUTPUTS, "mono", 1.0);
	rf5c68_1.set_addrmap(0, &bingoct_device::pcm_map);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bingoct_device - constructor
//-------------------------------------------------

bingoct_device::bingoct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BINGOCT, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_soundcpu(*this, "soundcpu"),
	m_vdp(*this, "vdp"),
	m_gfxbank(*this, "gfxbank"),
	m_soundbank(*this, "soundbank")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bingoct_device::device_start()
{
	m_gfxbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);
	m_soundbank->configure_entries(0, 64, memregion("soundcpu")->base(), 0x2000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bingoct_device::device_reset()
{
}

//-------------------------------------------------
//  soundcpu_irq_ack - automatic irq ack
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bingoct_device::soundcpu_irq_ack)
{
	m_soundcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

//-------------------------------------------------
//  output_w - set lamps and led digits
//-------------------------------------------------

void bingoct_device::output_w(offs_t offset, uint8_t data)
{
	// logerror("output_w: %04x = %02x\n", offset, data);
	// offset 5 = led digits
}
