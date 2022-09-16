// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************

Hartung Game Master
PeT mess@utanet.at march 2002

******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd7810/upd7811.h"
#include "sound/spkrdev.h"
#include "video/sed1520.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


class gmaster_state : public driver_device
{
public:
	gmaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd%u", 0),
		m_screen(*this, "screen"),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot")
	{ }

	void gmaster(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<upd78c11_device> m_maincpu;
	required_device_array<sed1520_device, 2> m_lcd;
	required_device<screen_device> m_screen;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;

	void gmaster_palette(palette_device &palette) const;
	uint8_t gmaster_io_r(offs_t offset);
	void gmaster_io_w(offs_t offset, uint8_t data);
	void gmaster_portb_w(uint8_t data);
	void gmaster_portc_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template<int N> SED1520_UPDATE_CB(screen_update_cb);

	void gmaster_mem(address_map &map);

	uint8_t m_ram[0x4000] = { };

	u8 m_chipsel = 0;
};


uint8_t gmaster_state::gmaster_io_r(offs_t offset)
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

void gmaster_state::gmaster_io_w(offs_t offset, uint8_t data)
{
	// write to external RAM
	if (m_chipsel & 1)
		m_ram[offset] = data;

	// write to LCD
	for (int i = 0; i < 2; i++)
		if (BIT(m_chipsel, i + 1))
			m_lcd[i]->write(offset & 1, data);
}


void gmaster_state::gmaster_portb_w(uint8_t data)
{
	// d0: ?
	// d1: ?
}

void gmaster_state::gmaster_portc_w(uint8_t data)
{
	// d0: RAM CS
	// d1: LCD1 CS
	// d2: LCD2 CS
	m_chipsel = data & 7;
}


void gmaster_state::gmaster_mem(address_map &map)
{
	map(0x4000, 0x7fff).rw(FUNC(gmaster_state::gmaster_io_r), FUNC(gmaster_state::gmaster_io_w));
	map(0x8000, 0xfeff).r("cartslot", FUNC(generic_slot_device::read_rom));
}


static INPUT_PORTS_START( gmaster )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START)
INPUT_PORTS_END


static constexpr rgb_t gmaster_pens[2] =
{
#if 1
	{ 130, 159, 166 },
	{ 45, 45, 43 }
#else
	{ 255,255,255 },
	{ 0, 0, 0 }
#endif
};


void gmaster_state::gmaster_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, gmaster_pens);
}


uint32_t gmaster_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u32 s0 = m_lcd[0]->screen_update(screen, bitmap, cliprect);
	u32 s1 = m_lcd[1]->screen_update(screen, bitmap, cliprect);
	return s0 & s1;
}

template<int N>
SED1520_UPDATE_CB(gmaster_state::screen_update_cb)
{
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

void gmaster_state::machine_start()
{
	save_item(NAME(m_ram));
}


void gmaster_state::gmaster(machine_config &config)
{
	UPD78C11(config, m_maincpu, 12_MHz_XTAL); // µPD78C11 in the unit
	m_maincpu->set_addrmap(AS_PROGRAM, &gmaster_state::gmaster_mem);
	m_maincpu->pa_in_cb().set_ioport("JOY");
	m_maincpu->pb_out_cb().set(FUNC(gmaster_state::gmaster_portb_w));
	m_maincpu->pc_out_cb().set(FUNC(gmaster_state::gmaster_portc_w));
	m_maincpu->to_func().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	SED1520(config, m_lcd[0]).set_screen_update_cb(FUNC(gmaster_state::screen_update_cb<0>));
	SED1520(config, m_lcd[1]).set_screen_update_cb(FUNC(gmaster_state::screen_update_cb<1>));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(80, 64);
	m_screen->set_visarea(0, 64-1-3, 0, 64-1);
	m_screen->set_screen_update(FUNC(gmaster_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(gmaster_state::gmaster_palette), std::size(gmaster_pens));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 0.50);

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "gmaster_cart").set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("gmaster");
}


ROM_START(gmaster)
	ROM_REGION(0x1000,"maincpu", 0)
	ROM_LOAD("d78c11agf_e19.u1", 0x0000, 0x1000, CRC(05cc45e5) SHA1(05d73638dea9657ccc2791c0202d9074a4782c1e) )
ROM_END


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME */
CONS( 1990, gmaster, 0,      0,      gmaster, gmaster, gmaster_state, empty_init, "Hartung", "Game Master", MACHINE_NOT_WORKING )
