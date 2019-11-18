// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at march 2002
******************************************************************************/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"
#include "sound/spkrdev.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


class gmaster_state : public driver_device
{
public:
	gmaster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cart(*this, "cartslot")
	{ }

	void gmaster(machine_config &config);

	void init_gmaster() { memset(&m_video, 0, sizeof(m_video)); memset(m_ram, 0, sizeof(m_ram)); }

private:
	void gmaster_palette(palette_device &palette) const;
	DECLARE_READ8_MEMBER(gmaster_io_r);
	DECLARE_WRITE8_MEMBER(gmaster_io_w);
	DECLARE_READ8_MEMBER(gmaster_portb_r);
	DECLARE_READ8_MEMBER(gmaster_portc_r);
	DECLARE_READ8_MEMBER(gmaster_portd_r);
	DECLARE_READ8_MEMBER(gmaster_portf_r);
	DECLARE_WRITE8_MEMBER(gmaster_porta_w);
	DECLARE_WRITE8_MEMBER(gmaster_portb_w);
	DECLARE_WRITE8_MEMBER(gmaster_portc_w);
	DECLARE_WRITE8_MEMBER(gmaster_portd_w);
	DECLARE_WRITE8_MEMBER(gmaster_portf_w);
	uint32_t screen_update_gmaster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void gmaster_mem(address_map &map);
	virtual void machine_start() override;

	struct
	{
		uint8_t data[8];
		int index;
		int x, y;
		bool mode; // true read does not increase address
		bool delayed;
		uint8_t pixels[8][64];
	} m_video;

	uint8_t m_ports[5];
	uint8_t m_ram[0x4000];
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
};


READ8_MEMBER(gmaster_state::gmaster_io_r)
{
	uint8_t data = 0;

	if (m_ports[2] & 1)
	{
		data = m_ram[offset];
		logerror("%.4x external memory %.4x read %.2x\n", m_maincpu->pc(), 0x4000 + offset, data);
	}
	else
	{
		switch (offset)
		{
		case 1:
			data = m_video.pixels[m_video.y][m_video.x];
			logerror("%.4x lcd x:%.2x y:%.2x %.4x read %.2x\n", m_maincpu->pc(), m_video.x, m_video.y, 0x4000 + offset, data);
			if (!(m_video.mode) && m_video.delayed)
			{
				m_video.x++;
			}
			m_video.delayed = true;
			break;
		default:
			logerror("%.4x memory %.4x read %.2x\n", m_maincpu->pc(), 0x4000 + offset, data);
			break;
		}
	}
	return data;
}

#define BLITTER_Y ((m_ports[2]&4)|(m_video.data[0]&3))


WRITE8_MEMBER(gmaster_state::gmaster_io_w)
{
	if (m_ports[2] & 1)
	{
		m_ram[offset] = data;
		logerror("%.4x external memory %.4x written %.2x\n", m_maincpu->pc(), 0x4000 + offset, data);
	}
	else
	{
		switch (offset)
		{
		case 0:
			m_video.delayed = false;
			logerror("%.4x lcd %.4x written %.2x\n", m_maincpu->pc(), 0x4000 + offset, data);
			// e2 af a4 a0 a9 falling block init for both halves
			if ((data & 0xfc) == 0xb8)
			{
				m_video.index = 0;
				m_video.data[m_video.index] = data;
				m_video.y = BLITTER_Y;
			}
			else if ((data & 0xc0) == 0)
			{
				m_video.x = data;
			}
			else if ((data & 0xf0) == 0xe0)
			{
				m_video.mode = (data & 0xe) ? false : true;
			}
			m_video.data[m_video.index] = data;
			m_video.index = (m_video.index + 1) & 7;
			break;
		case 1:
			m_video.delayed = false;
			if (m_video.x < ARRAY_LENGTH(m_video.pixels[0])) // continental galaxy flutlicht
			{
				m_video.pixels[m_video.y][m_video.x] = data;
			}
			logerror("%.4x lcd x:%.2x y:%.2x %.4x written %.2x\n", m_maincpu->pc(), m_video.x, m_video.y, 0x4000 + offset, data);
			m_video.x++;
/* 02 b8 1a
   02 bb 1a
   02 bb 22
   04 b8 12
   04 b8 1a
   04 b8 22
   04 b9 12
   04 b9 1a
   04 b9 22
   02 bb 12
    4000 e0
    rr w rr w rr w rr w rr w rr w rr w rr w
    4000 ee
*/
			break;
		default:
			logerror("%.4x memory %.4x written %.2x\n", m_maincpu->pc(), 0x4000 + offset, data);
			break;
		}
	}
}


READ8_MEMBER(gmaster_state::gmaster_portb_r)
{
//  uint8_t data = m_ports[1];
	uint8_t data = 0xff;

	logerror("%.4x port B read %.2x\n", m_maincpu->pc(), data);

	return data;
}

READ8_MEMBER(gmaster_state::gmaster_portc_r)
{
//  uint8_t data = m_ports[2];
	uint8_t data = 0xff;

	logerror("%.4x port C read %.2x\n", m_maincpu->pc(), data);

	return data;
}

READ8_MEMBER(gmaster_state::gmaster_portd_r)
{
//  uint8_t data = m_ports[3];
	uint8_t data = 0xff;

	logerror("%.4x port D read %.2x\n", m_maincpu->pc(), data);

	return data;
}

READ8_MEMBER(gmaster_state::gmaster_portf_r)
{
//  uint8_t data = m_ports[4];
	uint8_t data = 0xff;

	logerror("%.4x port F read %.2x\n", m_maincpu->pc(), data);

	return data;
}


WRITE8_MEMBER(gmaster_state::gmaster_porta_w)
{
	m_ports[0] = data;
	logerror("%.4x port A written %.2x\n", m_maincpu->pc(), data);
}

WRITE8_MEMBER(gmaster_state::gmaster_portb_w)
{
	m_ports[1] = data;
	logerror("%.4x port B written %.2x\n", m_maincpu->pc(), data);
}

WRITE8_MEMBER(gmaster_state::gmaster_portc_w)
{
	m_ports[2] = data;
	logerror("%.4x port C written %.2x\n", m_maincpu->pc(), data);

	m_video.y = BLITTER_Y;
	m_speaker->level_w(BIT(data, 4));
}

WRITE8_MEMBER(gmaster_state::gmaster_portd_w)
{
	m_ports[3] = data;
	logerror("%.4x port D written %.2x\n", m_maincpu->pc(), data);
}

WRITE8_MEMBER(gmaster_state::gmaster_portf_w)
{
	m_ports[4] = data;
	logerror("%.4x port F written %.2x\n", m_maincpu->pc(), data);
}


void gmaster_state::gmaster_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).rw(FUNC(gmaster_state::gmaster_io_r), FUNC(gmaster_state::gmaster_io_w));
	//map(0x8000, 0xfeff)      // mapped by the cartslot
}


static INPUT_PORTS_START( gmaster )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start")
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


uint32_t gmaster_state::screen_update_gmaster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	for (y = 0; y < ARRAY_LENGTH(m_video.pixels); y++)
	{
		for (x = 0; x < ARRAY_LENGTH(m_video.pixels[0]); x++)
		{
			uint8_t d = m_video.pixels[y][x];
			uint16_t *line;

			line = &bitmap.pix16((y * 8), x);
			line[0] = BIT(d, 0);
			line = &bitmap.pix16((y * 8 + 1), x);
			line[0] = BIT(d, 1);
			line = &bitmap.pix16((y * 8 + 2), x);
			line[0] = BIT(d, 2);
			line = &bitmap.pix16((y * 8 + 3), x);
			line[0] = BIT(d, 3);
			line = &bitmap.pix16((y * 8 + 4), x);
			line[0] = BIT(d, 4);
			line = &bitmap.pix16((y * 8 + 5), x);
			line[0] = BIT(d, 5);
			line = &bitmap.pix16((y * 8 + 6), x);
			line[0] = BIT(d, 6);
			line = &bitmap.pix16((y * 8 + 7), x);
			line[0] = BIT(d, 7);
		}
	}
	return 0;
}


void gmaster_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xfeff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	save_item(NAME(m_video.data));
	save_item(NAME(m_video.index));
	save_item(NAME(m_video.x));
	save_item(NAME(m_video.y));
	save_item(NAME(m_video.mode));
	save_item(NAME(m_video.delayed));
	save_item(NAME(m_video.pixels));
	save_item(NAME(m_ports));
	save_item(NAME(m_ram));
}


void gmaster_state::gmaster(machine_config &config)
{
	upd7810_device &upd(UPD7810(config, m_maincpu, 12_MHz_XTAL/2/*?*/)); // µPD78C11 in the unit
	upd.set_addrmap(AS_PROGRAM, &gmaster_state::gmaster_mem);
	upd.pa_in_cb().set_ioport("JOY");
	upd.pb_in_cb().set(FUNC(gmaster_state::gmaster_portb_r));
	upd.pc_in_cb().set(FUNC(gmaster_state::gmaster_portc_r));
	upd.pd_in_cb().set(FUNC(gmaster_state::gmaster_portd_r));
	upd.pf_in_cb().set(FUNC(gmaster_state::gmaster_portf_r));
	upd.pa_out_cb().set(FUNC(gmaster_state::gmaster_porta_w));
	upd.pb_out_cb().set(FUNC(gmaster_state::gmaster_portb_w));
	upd.pc_out_cb().set(FUNC(gmaster_state::gmaster_portc_w));
	upd.pd_out_cb().set(FUNC(gmaster_state::gmaster_portd_w));
	upd.pf_out_cb().set(FUNC(gmaster_state::gmaster_portf_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(64, 64);
	screen.set_visarea(0, 64-1-3, 0, 64-1);
	screen.set_screen_update(FUNC(gmaster_state::screen_update_gmaster));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(gmaster_state::gmaster_palette), ARRAY_LENGTH(gmaster_pens));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 0.50);

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "gmaster_cart").set_must_be_loaded(true);


	SOFTWARE_LIST(config, "cart_list").set_original("gmaster");
}


ROM_START(gmaster)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("d78c11agf_e19.u1", 0x0000, 0x1000, CRC(05cc45e5) SHA1(05d73638dea9657ccc2791c0202d9074a4782c1e) )
ROM_END


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME */
CONS( 1990, gmaster, 0,      0,      gmaster, gmaster, gmaster_state, init_gmaster, "Hartung", "Game Master", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
