// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for AT&T 610 Business Communication Terminal (BCT).

    The video ASIC is a 68-pin PLCC marked:

        (M) AT&T
        1006B2*
        456555
        18289S 71

    Another custom IC (28-pin DIP) is next to the character generator:

           WE
         492A*
        18186 71

****************************************************************************/

#include "emu.h"

//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/mc68681.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "emupal.h"
#include "screen.h"

class att610_state : public driver_device
{
public:
	att610_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sio(*this, "sio")
		, m_screen(*this, "screen")
		, m_rom(*this, "rom")
	{
	}

	void att610(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cart_select_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<z80sio_device> m_sio;
	required_device<screen_device> m_screen;
	required_memory_bank m_rom;
};

void att610_state::machine_start()
{
	m_rom->configure_entry(0, memregion("firmware")->base());
	m_rom->configure_entry(1, memregion("cartridge")->base());

	m_sio->ctsb_w(0);
}

void att610_state::machine_reset()
{
	m_rom->set_entry(0);
}

u32 att610_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static const gfx_layout char_layout =
{
	8, 13,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END

void att610_state::cart_select_w(u8 data)
{
	m_rom->set_entry(BIT(data, 0));
}

void att610_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).bankr("rom");
	map(0xc000, 0xffff).ram();
}

void att610_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x10, 0x10).w(FUNC(att610_state::cart_select_w));
	map(0x60, 0x63).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x70, 0x7f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}


static INPUT_PORTS_START(att610)
	// TODO: supports 103-key (56K430/ACZ) and 98-key (56K420/ADA) keyboards
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void att610_state::att610(machine_config &config)
{
	Z80(config, m_maincpu, 27.72_MHz_XTAL / 7); // MK3880N-4; CPU clock guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &att610_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &att610_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 27.72_MHz_XTAL / 7)); // Z8430APS
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, 27.72_MHz_XTAL / 7); // Z8441APS (SIO/1)
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	scn2681_device &duart(SCN2681(config, "duart", 3'686'400)); // MC2681P (adjacent XTAL not legible)
	duart.irq_cb().set("sio", FUNC(z80sio_device::syncb_w)).invert();
	duart.outport_cb().set("sio", FUNC(z80sio_device::rxcb_w)).bit(3);
	duart.outport_cb().append("sio", FUNC(z80sio_device::txcb_w)).bit(3);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(21.6675_MHz_XTAL, 963, 0, 720, 375, 0, 351);
	//m_screen->set_raw(27.72_MHz_XTAL, 1232, 0, 924, 375, 0, 351);
	m_screen->set_screen_update(FUNC(att610_state::screen_update));
	m_screen->screen_vblank().set("ctc", FUNC(z80ctc_device::trg0));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}

ROM_START(att610)
	ROM_REGION(0x6000, "firmware", 0)
	ROM_LOAD("455798-1.d4", 0x0000, 0x4000, CRC(91bd636f) SHA1(53bc886ba580dd64446ebe9b8a042414ff8834d6)) // MBM27128-25
	ROM_LOAD("455799-1.b4", 0x4000, 0x2000, CRC(7fd75ee0) SHA1(597b23c43b3f283b49b51b9dee60109ff683b041)) // MBM2764-25

	ROM_REGION(0x6000, "cartridge", ROMREGION_ERASE00) // optional

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("att-tc85_456309-1.h7", 0x0000, 0x2000, CRC(d313e022) SHA1(a24df1d8d8c55413e4cdb0734783c0fa244bdf00)) // HN27C64G-15
ROM_END

ROM_START( att615 )
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("523481059.bin", 0x00000, 0x10000, CRC(9649bf59) SHA1(bed7900f8848a98b04ba69492c3bc3727d30d67d))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("457756-2.bin", 0x0000, 0x2000, CRC(85871007) SHA1(018ea318ea9668e65043bc7c52c5b6dd3ed68687))

	ROM_REGION(0x6000, "cartridge", 0)
	ROM_LOAD("523458727.bin", 0x0000, 0x4000, CRC(beea70fd) SHA1(7b7a50832e44e5ece85a0ef83d40599f9fff5692))
	ROM_LOAD("523458735.bin", 0x4000, 0x2000, CRC(7f1b9d63) SHA1(0e07c435e7f6bef153e4b1f0d1d9d391a7562602))

	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD("20417-21_8031.bin", 0x0000, 0x1000, CRC(c48b4a2a) SHA1(f01dadc0239f81a58d50ae077f5cb3a029bad110))
ROM_END

COMP(1986, att610, 0, 0, att610, att610, att610_state, empty_init, "AT&T", "610 Business Communication Terminal", MACHINE_IS_SKELETON)
COMP(1987, att615, 0, 0, att610, att610, att610_state, empty_init, "AT&T", "615 MT", MACHINE_IS_SKELETON)
