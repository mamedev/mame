// license:GPL-2.0+
// copyright-holders:Peter Trauner, hap
/*******************************************************************************

Hartung Game Master
Hong Kong LCD handheld console (mainly sold in Europe)
PeT mess@utanet.at march 2002

Hardware notes:
- NEC D78C11AGF (4KB internal ROM), 12.00MHz XTAL
- 2KB external RAM(UM6116-2L), cartridge slot for external ROM
- 2*LCDC hiding under epoxy, appears to be SED1520
- 61*64 1bpp LCD screen (the odd width is correct)
- 1-bit sound

Known releases:
- Hartung Game Master (Germany, gray)
- Impel Game Master (Hong Kong, gray)
- Systema 2000 (UK, gray)
- <unknown> Game Master / Game Tronic / Mega Tronic / Super Game (purple)
- Videojet Game Master (France, gray or white)
- Prodis PDJ-10 (Spain, gray or white)
- Delplay Game Plus (France, vertical orientation)

I presume it's an anonymous Hong Kong production. Most of the games too,
they have no copyright/company info in them. Some of the later games have
a copyright by Bon Treasure (a Hong Kong company that's also involved with
Watara Supervision), so perhaps it's them.

TODO:
- does port B do anything?
- according to one video on Youtube, hspace should have some kind of volume
  filter on the bgm? not sure what controls it, or maybe it's a hardware quirk

BTANB:
- LCD flickers partially, especially bad in finitezn
- fast button retriggers, for example the gear shift in carracing

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/spkrdev.h"
#include "video/sed1520.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "gmaster.lh"


namespace {

class gmaster_state : public driver_device
{
public:
	gmaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_lcd(*this, "lcd%u", 0),
		m_screen(*this, "screen"),
		m_speaker(*this, "speaker")
	{ }

	void gmaster(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<upd78c11_device> m_maincpu;
	required_shared_ptr<u8> m_ram;
	required_device_array<sed1520_device, 2> m_lcd;
	required_device<screen_device> m_screen;
	required_device<speaker_sound_device> m_speaker;

	u8 m_chipsel = 0;

	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);
	void portb_w(u8 data);
	void portc_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template<int N> SED1520_UPDATE_CB(screen_update_cb);

	void main_map(address_map &map) ATTR_COLD;
};

void gmaster_state::machine_start()
{
	save_item(NAME(m_chipsel));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD outputs

u32 gmaster_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u32 s0 = m_lcd[0]->screen_update(screen, bitmap, cliprect);
	u32 s1 = m_lcd[1]->screen_update(screen, bitmap, cliprect);
	return s0 & s1;
}

template<int N>
SED1520_UPDATE_CB(gmaster_state::screen_update_cb)
{
	// LCD #0: top half, LCD #1: bottom half
	rectangle clip = m_screen->visible_area();
	if (N == 1)
		clip.min_y = 32;
	else
		clip.max_y = 32-1;
	clip &= cliprect;

	for (int c = 0; c < 320; c++)
	{
		for (int b = 0; b < 8; b++)
		{
			int pixel = lcd_on ? BIT(dram[c], b) : 0;
			int x = c % 80;
			int y = N << 5 | (c / 80) << 3 | b;

			if (clip.contains(x, y))
				bitmap.pix(y, x) = pixel;
		}
	}
	return 0;
}


// memory mapped I/O

u8 gmaster_state::io_r(offs_t offset)
{
	u8 data = 0;

	// read from external RAM
	if (m_chipsel & 1)
		data |= m_ram[offset];

	// read from LCD
	for (int i = 0; i < 2; i++)
		if (BIT(m_chipsel, i + 1))
			data |= m_lcd[i]->read(offset & 1);

	return data;
}

void gmaster_state::io_w(offs_t offset, u8 data)
{
	// write to external RAM
	if (m_chipsel & 1)
		m_ram[offset] = data;

	// write to LCD
	for (int i = 0; i < 2; i++)
		if (BIT(m_chipsel, i + 1))
			m_lcd[i]->write(offset & 1, data);
}

void gmaster_state::main_map(address_map &map)
{
	// 0x0000-0x0fff is internal ROM
	map(0x4000, 0x47ff).mirror(0x3800).rw(FUNC(gmaster_state::io_r), FUNC(gmaster_state::io_w)).share("ram");
	map(0x8000, 0xffff).r("cartslot", FUNC(generic_slot_device::read_rom));
	// 0xff00-0xffff is internal RAM
}


// MCU ports

void gmaster_state::portb_w(u8 data)
{
	// ?
}

void gmaster_state::portc_w(u8 data)
{
	// d0: RAM CS
	// d1,d2: LCD CS
	m_chipsel = data & 7;

	// d4: speaker out
	m_speaker->level_w(BIT(data, 4));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gmaster )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) // B
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // A
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gmaster_state::gmaster(machine_config &config)
{
	// basic machine hardware
	UPD78C11(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gmaster_state::main_map);
	m_maincpu->pa_in_cb().set_ioport("JOY");
	m_maincpu->pb_out_cb().set(FUNC(gmaster_state::portb_w));
	m_maincpu->pc_out_cb().set(FUNC(gmaster_state::portc_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(80, 64);
	m_screen->set_visarea(0, 64-1-3, 0, 64-1);
	m_screen->set_screen_update(FUNC(gmaster_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	SED1520(config, m_lcd[0]).set_screen_update_cb(FUNC(gmaster_state::screen_update_cb<0>));
	SED1520(config, m_lcd[1]).set_screen_update_cb(FUNC(gmaster_state::screen_update_cb<1>));

	config.set_default_layout(layout_gmaster);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 0.50);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "gmaster_cart").set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("gmaster");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START(gmaster)
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "d78c11agf_e19.u1", 0x0000, 0x1000, CRC(05cc45e5) SHA1(05d73638dea9657ccc2791c0202d9074a4782c1e) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME       FLAGS
SYST( 1990, gmaster, 0,      0,      gmaster, gmaster, gmaster_state, empty_init, "Hartung", "Game Master", MACHINE_SUPPORTS_SAVE )
