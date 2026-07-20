// license:BSD-3-Clause
// copyright-holders:

/*
Lucky Joker by Sammy

LUCKY JOKER OKI OM-2 PCB

2x Z0840006PSC main and audio CPU
LH5160-10L battery backed RAM
2x LH5116D-10 RAM
24.0000 MHz XTAL
YM2203C OPN
3x M5L8255AP-5 PPI
MB622412 BIGSPIN-1 custom
MB622411 BIGSPIN-2 custom
MB621195 BIGSPIN-3 custom
MB606449 BIGSPIN-4 custom
MB621196 BIGSPIN-5 custom
2x 051316 PSAC custom
4x bank of 8 switches

TODO:
* needs dump of the color PROM.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ymopn.h"
#include "video/k051316.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class luckyjkr_state : public driver_device
{
public:
	luckyjkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_hopper(*this, "hopper"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_k051316(*this, "k051316_%u", 0U)
	{ }

	void luckyjkr(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<hopper_device> m_hopper;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<k051316_device, 2> m_k051316;

	uint8_t m_audio_nmi_assert = 0;

	K051316_CB_MEMBER(zoom_callback_0);
	K051316_CB_MEMBER(zoom_callback_1);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audiocpu_nmi_w(uint8_t data);

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};


K051316_CB_MEMBER(luckyjkr_state::zoom_callback_0)
{
	code |= (color & 0x1) << 8;
	// color = ..
}


K051316_CB_MEMBER(luckyjkr_state::zoom_callback_1)
{
	code |= (color & 0x3) << 8;
	// color = ..
}

uint32_t luckyjkr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_k051316[0]->zoom_draw(screen, bitmap, cliprect, 0, 0);
	m_k051316[1]->zoom_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void luckyjkr_state::machine_start()
{
	save_item(NAME(m_audio_nmi_assert));
}

void luckyjkr_state::audiocpu_nmi_w(uint8_t data)
{
	if (!BIT(m_audio_nmi_assert, 0) && BIT(data, 0))
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_audio_nmi_assert = BIT(data, 0);
}

void luckyjkr_state::main_program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram();
	map(0xc400, 0xc403).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc410, 0xc413).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc420, 0xc423).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc440, 0xc440).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc470, 0xc470).w(FUNC(luckyjkr_state::audiocpu_nmi_w));
	map(0xc480, 0xc48f).w(m_k051316[0], FUNC(k051316_device::ctrl_w));
	map(0xc490, 0xc49f).w(m_k051316[1], FUNC(k051316_device::ctrl_w));
	map(0xc4c0, 0xc4c0).nopw(); // TODO: maybe IRQ ack or watchdog
	map(0xc800, 0xcfff).ram();
	map(0xd000, 0xd7ff).rw(m_k051316[0], FUNC(k051316_device::read), FUNC(k051316_device::write));
	map(0xd800, 0xdfff).rw(m_k051316[1], FUNC(k051316_device::read), FUNC(k051316_device::write));
	map(0xe000, 0xefff).ram(); // NVRAM?
	map(0xf000, 0xf7ff).r(m_k051316[0], FUNC(k051316_device::rom_r)).nopw();
	map(0xf800, 0xffff).r(m_k051316[1], FUNC(k051316_device::rom_r)).nopw();
}

void luckyjkr_state::audio_program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).ram();
}

void luckyjkr_state::audio_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x40, 0x40).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x80).lw8(NAME([this] (uint8_t data) { m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); }));
}


static INPUT_PORTS_START( luckyjkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(3)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(4)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles", 0, gfx_16x16x8_raw, 0, 16 ) // wrong
GFXDECODE_END


void luckyjkr_state::luckyjkr(machine_config &config)
{
	Z80(config, m_maincpu, 24_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &luckyjkr_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(luckyjkr_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 24_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &luckyjkr_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &luckyjkr_state::audio_io_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.in_pa_callback().set([this] () { logerror("%s PPI0 port A read\n", machine().describe_context()); return ioport("IN0")->read(); });
	ppi0.in_pb_callback().set([this] () { logerror("%s PPI0 port B read\n", machine().describe_context()); return ioport("IN1")->read(); });
	ppi0.in_pc_callback().set([this] () { logerror("%s PPI0 port C read\n", machine().describe_context()); return ioport("IN2")->read(); });

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.out_pa_callback().set([this] (uint8_t data) { logerror("%s PPI1 port A write: %02x\n", machine().describe_context(), data); });
	ppi1.out_pb_callback().set([this] (uint8_t data) { logerror("%s PPI1 port B write: %02x\n", machine().describe_context(), data); });
	ppi1.in_pc_callback().set([this] () { logerror("%s PPI1 port C read\n", machine().describe_context()); return ioport("DSW1")->read(); });
	ppi1.out_pc_callback().set([this] (uint8_t data) { logerror("%s PPI1 port C write: %02x\n", machine().describe_context(), data); });

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.in_pa_callback().set([this] () { logerror("%s PPI2 port A read\n", machine().describe_context()); return ioport("DSW2")->read(); });
	ppi2.in_pb_callback().set([this] () { logerror("%s PPI2 port B read\n", machine().describe_context()); return ioport("DSW3")->read(); });
	ppi2.in_pc_callback().set([this] () { logerror("%s PPI2 port C read\n", machine().describe_context()); return ioport("DSW4")->read(); });

	HOPPER(config, m_hopper, attotime::from_msec(100));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0, 32*8-1, 0, 32*8-1);
	m_screen->set_screen_update(FUNC(luckyjkr_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100);

	K051316(config, m_k051316[0], 24_MHz_XTAL / 2);
	m_k051316[0]->set_palette("palette");
	m_k051316[0]->set_bpp(8);
	m_k051316[0]->set_wrap(0);
	m_k051316[0]->set_zoom_callback(FUNC(luckyjkr_state::zoom_callback_0));

	K051316(config, m_k051316[1], 24_MHz_XTAL / 2);
	m_k051316[1]->set_palette("palette");
	m_k051316[1]->set_bpp(8);
	m_k051316[1]->set_wrap(0);
	m_k051316[1]->set_zoom_callback(FUNC(luckyjkr_state::zoom_callback_1));

	GENERIC_LATCH_8(config, "soundlatch");

	SPEAKER(config, "mono").front_center();

	ym2203_device &ym(YM2203(config, "ym", 24_MHz_XTAL / 8)); // divider not verified
	ym.irq_handler().set_inputline(m_audiocpu, 0);
	ym.add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START( luckyjkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sammy_prg-1.ic42", 0x00000, 0x10000, CRC(3e56e79e) SHA1(2172294925b791a035597fedd96c9dff318456c2) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "sammy_s-1.ic19", 0x0000, 0x8000, CRC(76538236) SHA1(1f8d774cf8cab0b448bc51b27054a8b3d76bb896) )

	ROM_REGION( 0x8000, "tiles", 0 )
	ROM_LOAD( "sammy_fix.ic74", 0x0000, 0x8000, CRC(0e6ab8ba) SHA1(fbf0aeaec58e388a0813dbf3f1e969ec91668a54) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x20000, "k051316_0", 0 )
	ROM_LOAD( "sammy_b-1.ic1", 0x00000, 0x20000, CRC(ca688678) SHA1(5ae25bba76d1195450d5c32af9d50f97c2f6c097) )

	ROM_REGION( 0x40000, "k051316_1", 0 )
	ROM_LOAD( "sammy_b2-1.ic23", 0x00000, 0x20000, CRC(85455604) SHA1(58cba2b2d162186a0ef4111f83bdb03a0dc8ac65) )
	ROM_LOAD( "sammy_b2-2.ic22", 0x20000, 0x20000, CRC(b74cf680) SHA1(0dd6a2d22b6176add7e316cb0ccf1ad468567df7) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "prom", 0 )
	ROM_LOAD( "n82s147an.ic3", 0x000, 0x200, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 199?, luckyjkr, 0, luckyjkr, luckyjkr, luckyjkr_state, empty_init, ROT0, "Sammy", "Lucky Joker", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
