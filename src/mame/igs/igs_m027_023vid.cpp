// license:BSD-3-Clause
// copyright-holders:

/*
IGS ARM7 (IGS027A) based mahjong / gambling platform(s),
with IGS customs normally used on PGM hardware.
Contrary to proper PGM hardware, these don't have a M68K.

Main components for the PCB-0457-03-GS are:
- IGS 027A (ARM7-based MCU)
- 33 MHz XTAL
- IGS 023 graphics chip
- ICS2115V Wavefront sound chip
- IGS 026B I/O chip
- 3 banks of 8 DIP switches

*/

#include "emu.h"

#include "igs023_video.h"
#include "igs027a.h"
#include "mahjong.h"
#include "pgmcrypt.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"

#include "sound/ics2115.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include <algorithm>


namespace {

class igs_m027_023vid_state : public driver_device
{
public:
	igs_m027_023vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_nvram(*this, "nvram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video(*this, "igs023"),
		m_ics(*this, "ics")
	{ }

	void m027_023vid(machine_config &config) ATTR_COLD;

	void init_mxsqy() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_region_ptr<u32> m_external_rom;
	required_shared_ptr<u32> m_nvram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<igs023_video_device> m_video;
	required_device<ics2115_device> m_ics;

	u32 m_xor_table[0x100];
	bool m_irq_source;

	u32 external_rom_r(offs_t offset);

	void xor_table_w(offs_t offset, u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u16 sprites_r(offs_t offset);
	void screen_vblank(int state);

	void m027_map(address_map &map) ATTR_COLD;

	template <unsigned N>
	void irq_w(int state);
	u32 gpio_r();

	u32 unk0_r() { return 0xffffffff; }
	u32 unk1_r() { return 0xffffffff; }
};

void igs_m027_023vid_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);
	m_irq_source = 0;

	save_item(NAME(m_xor_table));
	save_item(NAME(m_irq_source));
}


u32 igs_m027_023vid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_video->screen_update(screen, bitmap, cliprect);
}

void igs_m027_023vid_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		m_video->get_sprites();
	}
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_023vid_state::m027_map(address_map &map)
{
	map(0x0800'0000, 0x0807'ffff).r(FUNC(igs_m027_023vid_state::external_rom_r)); // Game ROM

	map(0x1800'0000, 0x1800'7fff).ram().share(m_nvram);

	map(0x2800'0000, 0x2800'0fff).ram();

	map(0x3890'0000, 0x3890'7fff).rw(m_video, FUNC(igs023_video_device::videoram_r), FUNC(igs023_video_device::videoram_w)).umask32(0xffffffff);

	map(0x38a0'0000, 0x38a0'11ff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x38b0'0000, 0x38b0'ffff).rw(m_video, FUNC(igs023_video_device::videoregs_r), FUNC(igs023_video_device::videoregs_w)).umask32(0xffffffff);

	map(0x4000'0008, 0x4000'000b).nopw();

	map(0x4800'0000, 0x4800'0003).r(FUNC(igs_m027_023vid_state::unk0_r));
	map(0x4800'0004, 0x4800'0007).r(FUNC(igs_m027_023vid_state::unk1_r));

	map(0x5000'0000, 0x5000'03ff).umask32(0x0000'00ff).w(FUNC(igs_m027_023vid_state::xor_table_w)); // uploads XOR table to external ROM here

	map(0x5800'0000, 0x5800'0007).rw("ics", FUNC(ics2115_device::read), FUNC(ics2115_device::write)).umask32(0x00ff00ff);

	map(0x7000'0108, 0x7000'010b).nopw();
}


/***************************************************************************

    Input Ports

***************************************************************************/

INPUT_PORTS_START( base )
	PORT_START("CLEARMEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("PORTB") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTB")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PORTC") // buttons?
	PORT_DIPNAME( 0x01, 0x01, "PORTC")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/


u32 igs_m027_023vid_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}


void igs_m027_023vid_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}


TIMER_DEVICE_CALLBACK_MEMBER(igs_m027_023vid_state::interrupt)
{
	int scanline = param;

	switch (scanline)
	{
	case 0:
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time()); // vbl?
		break;

	case 192:
		igs_m027_023vid_state::irq_w<0>(ASSERT_LINE);
		break;
	}
}


u16 igs_m027_023vid_state::sprites_r(offs_t offset)
{
	// there does seem to be a spritelist at the start of mainram like PGM
	// it is also copied to a secondary RAM area, which seems to be our datasource in this case

	address_space& mem = m_maincpu->space(AS_PROGRAM);
	u16 sprdata = mem.read_word(0x28000000 + offset * 2);
	return sprdata;
}

template <unsigned N>
void igs_m027_023vid_state::irq_w(int state)
{
	if (state)
	{
		m_irq_source = N;
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time());
	}
}

u32 igs_m027_023vid_state::gpio_r()
{
	u32 ret = -1;
	if (!m_irq_source)
	{
		ret ^= 2;
	}
	return ret;
}


void igs_m027_023vid_state::m027_023vid(machine_config &config)
{
	IGS027A(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_023vid_state::m027_map);
	m_maincpu->in_port().set(FUNC(igs_m027_023vid_state::gpio_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 448-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(igs_m027_023vid_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(igs_m027_023vid_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1200/2);

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027_023vid_state::interrupt), "screen", 0, 1);

	// PGM video
	IGS023_VIDEO(config, m_video, 0);
	m_video->set_palette(m_palette);
	m_video->read_spriteram_callback().set(FUNC(igs_m027_023vid_state::sprites_r));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ICS2115(config, m_ics, 33.8688_MHz_XTAL);
	m_ics->irq().set(FUNC(igs_m027_023vid_state::irq_w<1>));
	m_ics->add_route(ALL_OUTPUTS, "mono", 5.0);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/


ROM_START( mxsqy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "a8_027a.u41", 0x00000, 0x4000, CRC(f9ada8c4) SHA1(0715fdc3d15ae2d1af4e9c7d25f6410ae7c22d42) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "igs_m2401.u39", 0x000000, 0x80000, CRC(32e69540) SHA1(e5bc44700ba965fae433c3b39afa95bc753e3f2a) )

	ROM_REGION( 0x80000, "igs023",  0 )
	ROM_LOAD( "igs_l2405.u38", 0x00000, 0x80000, CRC(2f20eade) SHA1(aa11d26cb51483af5fdd4b181dff0f222baeaaff) )

	ROM_REGION16_LE( 0x400000, "igs023:sprcol", 0 )
	ROM_LOAD( "igs_l2404.u23", 0x000000, 0x400000, CRC(dc8ff7ae) SHA1(4609b5543d8bea7a8dea4e744f81c407688a96ee) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION16_LE( 0x400000, "igs023:sprmask", 0 )
	ROM_LOAD( "igs_m2403.u22", 0x000000, 0x400000, CRC(53940332) SHA1(3c703cbdc51dfb100f3ce10452a81091305dee01) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "igs_s2402.u21", 0x000000, 0x400000, CRC(a3e3b2e0) SHA1(906e5839ab62e570d9716e01b49e5b067e041269) )
ROM_END


void igs_m027_023vid_state::init_mxsqy()
{
	mxsqy_decrypt(machine());
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// internal ROM is 2003
GAME( 2003, mxsqy, 0, m027_023vid, base, igs_m027_023vid_state, init_mxsqy, ROT0, "IGS", "Ming Xing San Que Yi (China)", MACHINE_IS_SKELETON )
