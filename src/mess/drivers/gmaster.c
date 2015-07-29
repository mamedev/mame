// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at march 2002
******************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "rendlay.h"


class gmaster_state : public driver_device
{
public:
	gmaster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cart(*this, "cartslot")
		, m_io_joy(*this, "JOY")
	{ }

	DECLARE_PALETTE_INIT(gmaster);
	DECLARE_READ8_MEMBER(gmaster_io_r);
	DECLARE_WRITE8_MEMBER(gmaster_io_w);
	DECLARE_READ8_MEMBER(gmaster_port_r);
	DECLARE_WRITE8_MEMBER(gmaster_port_w);
	DECLARE_DRIVER_INIT(gmaster) { memset(&m_video, 0, sizeof(m_video)); memset(m_ram, 0, sizeof(m_ram)); }
	UINT32 screen_update_gmaster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	virtual void machine_start();

	struct
	{
		UINT8 data[8];
		int index;
		int x, y;
		bool mode; // true read does not increase address
		bool delayed;
		UINT8 pixels[8][64];
	} m_video;

	UINT8 m_ports[5];
	UINT8 m_ram[0x4000];
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;
};


READ8_MEMBER(gmaster_state::gmaster_io_r)
{
	UINT8 data = 0;

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


READ8_MEMBER(gmaster_state::gmaster_port_r)
{
//  UINT8 data = m_ports[offset];
	UINT8 data = 0xff;
	switch (offset)
	{
	case UPD7810_PORTA:
		data = m_io_joy->read();
		break;
	default:
		logerror("%.4x port %d read %.2x\n", m_maincpu->pc(), offset, data);
		break;
	}
	return data;
}


WRITE8_MEMBER(gmaster_state::gmaster_port_w)
{
	m_ports[offset] = data;
	logerror("%.4x port %d written %.2x\n", m_maincpu->pc(), offset, data);
	switch (offset)
	{
		case UPD7810_PORTC:
			m_video.y = BLITTER_Y;
			m_speaker->level_w(BIT(data, 4));
			break;
	}
}


static ADDRESS_MAP_START( gmaster_mem, AS_PROGRAM, 8, gmaster_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE(gmaster_io_r, gmaster_io_w)
	//AM_RANGE(0x8000, 0xfeff)      // mapped by the cartslot
	AM_RANGE(0xff00, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(gmaster_io, AS_IO, 8, gmaster_state )
	AM_RANGE(UPD7810_PORTA, UPD7810_PORTF) AM_READWRITE(gmaster_port_r, gmaster_port_w )
ADDRESS_MAP_END


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


/* palette in red, green, blue tribles */
static const unsigned char gmaster_palette[2][3] =
{
#if 1
	{ 130, 159, 166 },
	{ 45,45,43 }
#else
	{ 255,255,255 },
	{ 0, 0, 0 }
#endif
};


PALETTE_INIT_MEMBER(gmaster_state, gmaster)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		palette.set_pen_color(i, gmaster_palette[i][0], gmaster_palette[i][1], gmaster_palette[i][2]);
	}
}


UINT32 gmaster_state::screen_update_gmaster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	for (y = 0; y < ARRAY_LENGTH(m_video.pixels); y++)
	{
		for (x = 0; x < ARRAY_LENGTH(m_video.pixels[0]); x++)
		{
			UINT8 d = m_video.pixels[y][x];
			UINT16 *line;

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
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xfeff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

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


static MACHINE_CONFIG_START( gmaster, gmaster_state )
	MCFG_CPU_ADD("maincpu", UPD7810, XTAL_12MHz/2/*?*/)  // upd78c11 in the unit
	MCFG_CPU_PROGRAM_MAP(gmaster_mem)
	MCFG_CPU_IO_MAP( gmaster_io)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 64-1-3, 0, 64-1)
	MCFG_SCREEN_UPDATE_DRIVER(gmaster_state, screen_update_gmaster)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gmaster_palette))
	MCFG_PALETTE_INIT_OWNER(gmaster_state, gmaster)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "gmaster_cart")
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list","gmaster")
MACHINE_CONFIG_END


ROM_START(gmaster)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("d78c11agf_e19.u1", 0x0000, 0x1000, CRC(05cc45e5) SHA1(05d73638dea9657ccc2791c0202d9074a4782c1e) )
ROM_END


/*    YEAR  NAME      PARENT  COMPAT    MACHINE   INPUT    CLASS          INIT      COMPANY    FULLNAME */
CONS( 1990, gmaster,  0,      0,        gmaster,  gmaster, gmaster_state, gmaster, "Hartung", "Game Master", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
