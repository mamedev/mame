// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 quickpick5.cpp: Konami "Quick Pick 5" medal game

 Quick Pick 5
 (c) 1991 Konami

 Driver by R. Belmont

 PWB(A)352878A
 Rundown of PCB:
  Main CPU:  Z80

 Konami Custom chips:
  051649 (SCC1 sound)
  053252 (timing/interrupt controller)
  053244 (sprites)
  053245 (sprites)

***************************************************************************/

#include "emu.h"
#include "k053244_k053245.h"
#include "konami_helper.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k053252.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/k051649.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class quickpick5_state : public driver_device
{
public:
	quickpick5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_k053245(*this, "k053245"),
		m_k051649(*this, "k051649"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_outport(*this, "OUT"),
		m_sio_ports(*this, "SIO%u", 1U),
		m_ttlrom_offset(0)
	{ }

	void quickpick5(machine_config &config);
	void waijockey(machine_config &config);
	int serial_io_r();

private:
	u32 screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K05324X_CB_MEMBER(sprite_callback);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	void ccu_int_time_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void vbl_ack_w(int state) { m_maincpu->set_input_line(0, CLEAR_LINE); }
	void nmi_ack_w(int state) { m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); }

	// A0 is inverted to match the Z80's endianness.  Typical Konami.
	u8 k244_r(offs_t offset) { return m_k053245->k053244_r(offset^1);  }
	void k244_w(offs_t offset, u8 data) { m_k053245->k053244_w(offset^1, data); }
	u8 k245_r(offs_t offset) { return m_k053245->k053245_r(offset^1);  }
	void k245_w(offs_t offset, u8 data) { m_k053245->k053245_w(offset^1, data); }

	void control_w(u8 data)
	{
		membank("bank1")->set_entry(data&0x1);
		if (((m_control & 0x60) != 0x60) && ((data & 0x60) == 0x60))
		{
			m_ttlrom_offset = 0;
		}
		m_control = data;
	}

	u8 vram_r(address_space &space, offs_t offset);
	void vram_w(offs_t offset, u8 data);

	void serial_io_w(u8 data);
	void out_w(u8 data);

	void common_map(address_map &map) ATTR_COLD;
	void quickpick5_main(address_map &map) ATTR_COLD;
	void waijockey_main(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<k05324x_device> m_k053245;
	required_device<k051649_device> m_k051649;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;
	required_ioport m_outport;
	optional_ioport_array<3> m_sio_ports;

	int         m_ttl_gfx_index;
	tilemap_t   *m_ttl_tilemap;
	u8          m_control;
	int         m_ttlrom_offset;
	u8          m_vram[0x1000];
	bool        m_title_hack;
	int         m_ccu_int_time, m_ccu_int_time_count;

	u16 m_sio_out, m_sio_in0, m_sio_in1;
	u8 m_sio_prev;
};

void quickpick5_state::serial_io_w(u8 data)
{
	// bit 7 - coin counter OUT
	// bit 6 - coin counter IN
	// bits 0-4 - serial I/O device

	if (BIT(data, 1) && !BIT(m_sio_prev, 1))
	{
		switch (data & 0x1c)
		{
		case 0x00: // latch m_sio_in1 (lamps) value
			break;
		case 0x10:
			m_sio_out = 0;
			for (int i = 0; i < 3; i++)
				if (BIT(m_sio_in0, i))
					m_sio_out |= m_sio_ports[2 - i]->read();

			m_sio_in1 = (m_sio_in1 << 1) | BIT(data, 0);
			break;
		case 0x18:
			m_sio_out >>= 1;
			break;
		case 0x1c:
			m_sio_in0 = (m_sio_in0 << 1) | BIT(data, 0);
			break;
		default:
			logerror("serial_io: unhandled command %02X\n", data & 0x1f);
			break;
		}
	}
	m_sio_prev = data;
}

int quickpick5_state::serial_io_r()
{
	return BIT(m_sio_out, 0);
}

void quickpick5_state::out_w(u8 data)
{
	/*
	quickpick5:
	bit 7 - patlite?
	bit 1 - hopper
	bit 0 - ~coin blocker

	waijokey:
	bit 6 - ??? (toggle all the time)
	Bit 3 - coin counter In
	Bit 2 - lock medal
	Bit 1 - ???
	Bit 0 - hopper

	should be split into 2  handlers later
	*/
	m_outport->write(data);
}

void quickpick5_state::ccu_int_time_w(u8 data)
{
	if (m_ccu_int_time != data)
		logerror("ccu_int_time rewritten with value of %02x\n", data);
	m_ccu_int_time = data;
}

u8 quickpick5_state::vram_r(address_space &space, offs_t offset)
{
	if ((m_control & 0x10) == 0x10)
	{
		offset |= 0x800;
		if ((offset >= 0x800) && (offset <= 0x880))
		{
			return m_k051649->k051649_waveform_r(offset & 0x7f);
		}
		else if ((offset >= 0x8e0) && (offset <= 0x8ff))
		{
			return m_k051649->k051649_test_r(space);
		}
	}

	if ((m_control & 0x60) == 0x60)
	{
		u8 *ROM = memregion("ttl")->base();
		return ROM[m_ttlrom_offset++];
	}

	return m_vram[offset];
}

void quickpick5_state::vram_w(offs_t offset, u8 data)
{
	if ((m_control & 0x10) == 0x10)
	{
		offset |= 0x800;
		if ((offset >= 0x800) && (offset < 0x880))
		{
			m_k051649->k051649_waveform_w(offset-0x800, data);
			return;
		}
		else if (offset < 0x88a)
		{
			m_k051649->k051649_frequency_w(offset-0x880, data);
			return;
		}
		else if (offset < 0x88f)
		{
			m_k051649->k051649_volume_w(offset-0x88a, data);
			return;
		}
		else if (offset < 0x890)
		{
			m_k051649->k051649_keyonoff_w(data);
			return;
		}

		m_k051649->k051649_test_w(data);
		return;
	}

	m_vram[offset] = data;
	m_ttl_tilemap->mark_tile_dirty(offset>>1);
}

void quickpick5_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	int gfx_index;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("ttl")->base(), 0, m_palette->entries() / 16, 0));
	m_ttl_gfx_index = gfx_index;

	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quickpick5_state::ttl_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_ttl_tilemap->set_transparent_pen(0);
	m_ttl_tilemap->set_scrollx(80);
	m_ttl_tilemap->set_scrolly(28);

	m_ttlrom_offset = 0;
	m_ccu_int_time_count = 0;
	m_ccu_int_time = 20;
}

TILE_GET_INFO_MEMBER(quickpick5_state::ttl_get_tile_info)
{
	u8 *lvram = &m_vram[0];
	int attr, code;

	attr = lvram[BYTE_XOR_LE((tile_index<<1)+1)];
	code = lvram[BYTE_XOR_LE((tile_index<<1))] | ((attr & 0xf) << 8);
	attr >>= 3;
	attr &= ~1;

	tileinfo.set(m_ttl_gfx_index, code, attr, 0);
}

u32 quickpick5_state::screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	m_title_hack = false;
	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());

	if (!m_title_hack)
	{
		m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 0xff);
	}

	return 0;
}

K05324X_CB_MEMBER(quickpick5_state::sprite_callback)
{
	*code = (*code & 0x7ff);
	*color = (*color & 0x003f);

	if (*code == 0x520)
	{
		m_title_hack = true;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(quickpick5_state::scanline)
{
	int scanline = param;

	// z80 /IRQ is connected to the IRQ1(vblank) pin of k053252 CCU
	if(scanline == 255)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}

	// z80 /NMI is connected to the IRQ2 pin of k053252 CCU
	// the following code is emulating INT_TIME of the k053252, this code will go away
	// when the new konami branch is merged.
	m_ccu_int_time_count--;
	if (m_ccu_int_time_count < 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_ccu_int_time_count = m_ccu_int_time;
	}
}

void quickpick5_state::common_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xdbff).ram().share("nvram");
	map(0xdc00, 0xdc0f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write));
	map(0xdc40, 0xdc4f).rw(FUNC(quickpick5_state::k244_r), FUNC(quickpick5_state::k244_w));
	map(0xdc80, 0xdc80).portr("DSW1");
	map(0xdc81, 0xdc81).portr("DSW2");
	map(0xdcc0, 0xdcc0).portr("IN1");
	map(0xdcc1, 0xdcc1).portr("IN2");
	map(0xdd00, 0xdd00).nopw();
	map(0xdd40, 0xdd40).portr("IN3");
	map(0xdd80, 0xdd80).w(FUNC(quickpick5_state::control_w));
	map(0xde40, 0xde40).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe000, 0xefff).rw(FUNC(quickpick5_state::vram_r), FUNC(quickpick5_state::vram_w));
	map(0xf000, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xffff).rw(FUNC(quickpick5_state::k245_r), FUNC(quickpick5_state::k245_w));
}

void quickpick5_state::quickpick5_main(address_map &map)
{
	common_map(map);
	map(0xddc0, 0xddc0).w(FUNC(quickpick5_state::serial_io_w));
	map(0xde00, 0xde00).w(FUNC(quickpick5_state::out_w));
}

void quickpick5_state::waijockey_main(address_map &map)
{
	common_map(map);
	map(0xdd41, 0xdd41).nopr(); // DSW3?
	map(0xddc0, 0xddc0).nopw(); // bit 6 - coin counter OUT
	map(0xde00, 0xde00).w(FUNC(quickpick5_state::out_w));
	//map(0xde80, 0xdfff).nopw();
}

static INPUT_PORTS_START( quickpick5 )
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_SERVICE_NO_TOGGLE(0x08, IP_ACTIVE_LOW)
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Reset Switch") PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Global Stats") PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Last Game Stats") PORT_TOGGLE
	PORT_BIT(0xd8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )   PORT_DIPLOCATION("DIPSW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )   PORT_DIPLOCATION("DIPSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x30, 0x30, "Jack Pot" )   PORT_DIPLOCATION("DIPSW1:5,6")
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Pay" )   PORT_DIPLOCATION("DIPSW1:7,8")
	PORT_DIPSETTING(    0xc0, "400" )
	PORT_DIPSETTING(    0x80, "500" )
	PORT_DIPSETTING(    0x40, "700" )
	PORT_DIPSETTING(    0x00, "1000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, "Max Payout" )   PORT_DIPLOCATION("DIPSW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "94%" )
	PORT_DIPSETTING(    0x01, "92%" )
	PORT_DIPSETTING(    0x02, "91%" )
	PORT_DIPSETTING(    0x03, "90%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x05, "88%" )
	PORT_DIPSETTING(    0x06, "87%" )
	PORT_DIPSETTING(    0x07, "86%" )
	PORT_DIPSETTING(    0x08, "85%" )
	PORT_DIPSETTING(    0x09, "84%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x0b, "82%" )
	PORT_DIPSETTING(    0x0c, "80%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0e, "70%" )
	PORT_DIPSETTING(    0x0f, "65%" )
	PORT_DIPNAME( 0x30, 0x30, "Button Time" )   PORT_DIPLOCATION("DIPSW2:5,6")
	PORT_DIPSETTING(    0x00, "40 Seconds" )
	PORT_DIPSETTING(    0x10, "30 Seconds" )
	PORT_DIPSETTING(    0x20, "20 Seconds" )
	PORT_DIPSETTING(    0x30, "15 Seconds" )
	PORT_DIPNAME( 0x40, 0x00, "Backup Memory" )   PORT_DIPLOCATION("DIPSW2:7")
	PORT_DIPSETTING(    0x00, "Clear" )
	PORT_DIPSETTING(    0x40, "Keep" )
	PORT_DIPNAME( 0x80, 0x00, "Attract Sound" )   PORT_DIPLOCATION("DIPSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(quickpick5_state, serial_io_r)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen") // guess
	PORT_BIT(0x7e, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SIO1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_GAMBLE_BET)
	PORT_BIT(0x540, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("SIO2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Clear")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("A.S")
	PORT_BIT(0x2e0, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("SIO3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("11") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("12") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("13") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("14") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("15") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x500, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("OUT")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("hopper", hopper_device, motor_w)
INPUT_PORTS_END

// Control panel buttons layout:
// Payout 1 2 1-2 1-3 1-4
//        3 4 2-3 2-4 3-4 Start
static INPUT_PORTS_START( waijockey )
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // * B16
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1) PORT_NAME("Medal")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2) PORT_NAME("100 Yen")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // * B19
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 1-2") PORT_CODE(KEYCODE_D)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 3-4") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 2-4") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 2-3") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 4") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 3") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 2") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 1-4") PORT_CODE(KEYCODE_G)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )   PORT_DIPLOCATION("DIPSW1:1,2,3,4")
	PORT_DIPNAME( 0xf0, 0xf0, "Standard of Payout" ) PORT_DIPLOCATION("DIPSW1:5,6,7,8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )   PORT_DIPLOCATION("DIPSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )   PORT_DIPLOCATION("DIPSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )   PORT_DIPLOCATION("DIPSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )   PORT_DIPLOCATION("DIPSW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Play Mode" )   PORT_DIPLOCATION("DIPSW2:5") // more like gameplay test mode
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, "Payout Mode" )   PORT_DIPLOCATION("DIPSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, "Attract Sound" )   PORT_DIPLOCATION("DIPSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, "Backup Memory" )   PORT_DIPLOCATION("DIPSW2:8")
	PORT_DIPSETTING(    0x00, "Clear" )
	PORT_DIPSETTING(    0x80, "Keep" )

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // * B14
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bet 1-3") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x60, IP_ACTIVE_LOW, IPT_UNUSED) // battery sensor
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen") // guess

	PORT_START("OUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("hopper", hopper_device, motor_w)
INPUT_PORTS_END

void quickpick5_state::machine_start()
{
	membank("bank1")->configure_entries(0, 0x2, memregion("maincpu")->base()+0x8000, 0x4000);

	save_item(NAME(m_control));
	save_item(NAME(m_ccu_int_time));
	save_item(NAME(m_ccu_int_time_count));
	save_item(NAME(m_sio_out));
	save_item(NAME(m_sio_in0));
	save_item(NAME(m_sio_in1));
	save_item(NAME(m_sio_prev));
}

void quickpick5_state::machine_reset()
{
	membank("bank1")->set_entry(0);

	m_control = 0;
	m_ccu_int_time = 0;
	m_ccu_int_time_count = 0;
	m_sio_out = m_sio_in0 = m_sio_in1 = 0;
	m_sio_prev = 0;
}

void quickpick5_state::quickpick5(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(32'000'000)/4); // z84c0008pec 8mhz part, 32Mhz xtal verified on PCB, divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &quickpick5_state::quickpick5_main);
	TIMER(config, "scantimer").configure_scanline(FUNC(quickpick5_state::scanline), "screen", 0, 1);
	HOPPER(config, "hopper", attotime::from_msec(100));

	K053252(config, m_k053252, XTAL(32'000'000)/4); /* K053252, xtal verified, divider not verified */
	m_k053252->int1_ack().set(FUNC(quickpick5_state::vbl_ack_w));
	m_k053252->int2_ack().set(FUNC(quickpick5_state::nmi_ack_w));
	m_k053252->int_time().set(FUNC(quickpick5_state::ccu_int_time_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(20));
	screen.set_size(64*8, 33*8);
	screen.set_visarea(88, 456-1, 28, 256-1);
	screen.set_screen_update(FUNC(quickpick5_state::screen_update_quickpick5));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_offsets(-(44+80), 20);
	m_k053245->set_sprite_callback(FUNC(quickpick5_state::sprite_callback));

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	K051649(config, m_k051649, XTAL(32'000'000)/9);  // xtal is verified, divider is not
	m_k051649->add_route(ALL_OUTPUTS, "mono", 0.45);

	OKIM6295(config, m_oki, XTAL(32'000'000)/18, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void quickpick5_state::waijockey(machine_config &config)
{
	quickpick5(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &quickpick5_state::waijockey_main);
}


ROM_START( quickp5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "117.10e.bin",  0x000000, 0x010000, CRC(3645e1a5) SHA1(7d0d98772f3732510e7a58f50a622fcec74087c3) )

	ROM_REGION( 0x40000, "k053245", 0 )   /* sprites */
	ROM_LOAD32_BYTE( "117-a02-7k.bin", 0x000003, 0x010000, CRC(745a1dc9) SHA1(33d876fb70cb802d62f87ad3721740e0961c7bec) )
	ROM_LOAD32_BYTE( "117-a03-7l.bin", 0x000002, 0x010000, CRC(07ec6db7) SHA1(7a94efc5f313fee6b9b63b7d2b6ba1cbf4158900) )
	ROM_LOAD32_BYTE( "117-a04-3l.bin", 0x000001, 0x010000, CRC(08dba5df) SHA1(2174be21c5a7db31ccc20ca0b88e4a94145776a5) )
	ROM_LOAD32_BYTE( "117-a05-3k.bin", 0x000000, 0x010000, CRC(9b2d0501) SHA1(3f1c69ef101153da5ac3335585541006c42e954d) )

	ROM_REGION( 0x80000, "ttl", 0 ) /* TTL text tilemap characters? */
	ROM_LOAD( "117-18e.bin",  0x000000, 0x020000, CRC(10e0d1e2) SHA1(f4ba190814d5e3f3e910c9da24845b6ddb259bff) )

	ROM_REGION( 0x20000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "117-a01-2e.bin", 0x000000, 0x020000, CRC(3d8fbd01) SHA1(f350da2a4e7bfff9975188a39acf73415bd85b3d) )

	ROM_REGION( 0x80000, "pals", 0 )
	ROM_LOAD( "054590.11g",   0x000000, 0x040000, CRC(0442621c) SHA1(2e79bea4e37028a3c1223fb4e3b3e12ccad2b39b) )
	ROM_LOAD( "054591.12g",   0x040000, 0x040000, CRC(eaa92d8f) SHA1(7a430f11127148f0c035973ce21cfec4cb60ce9d) )

ROM_END

// Konami PWB353330
ROM_START( waijockey )
	ROM_REGION( 0x10000, "maincpu", 0 ) // main program
	ROM_LOAD( "gs-257-a02.7n",  0x000000, 0x010000, CRC(e9a5f416) SHA1(b762b393bbe394339904636ff1d31d8eeb8b8d05) )

	ROM_REGION( 0x80000, "k053245", 0 )   // sprites
	ROM_LOAD32_BYTE( "gs-257-a03.3t",  0x000000, 0x020000, CRC(4aa2376b) SHA1(30e472457d10504fb805882a5eea7e548e812ff6) )
	ROM_LOAD32_BYTE( "gs-257-a05.3u",  0x000001, 0x020000, CRC(a5b18792) SHA1(d5ee5e6a8040a2297073ad4b42b8978c9865cceb) )
	ROM_LOAD32_BYTE( "gs-257-a04.10t", 0x000002, 0x020000, CRC(58c3ce20) SHA1(8d6df373a37770602d104325e27015611fdaaaff) )
	ROM_LOAD32_BYTE( "gs-257-a06.10u", 0x000003, 0x020000, CRC(260f7a2f) SHA1(3922ba8bffe7c37c6895a826e0a067627d0f3ff8) )

	ROM_REGION( 0x80000, "ttl", 0 ) // TTL text tilemap characters?
	ROM_LOAD( "gs-257-a07.28t",  0x000000, 0x020000, CRC(26e3afa6) SHA1(dba5f321b523717b9dd3f1e22ef15c1301af403b) )

	ROM_REGION( 0x20000, "oki", 0 )    // OKIM6295 samples
	ROM_LOAD( "gs-257-a01.1g", 0x000000, 0x020000, CRC(8ce0e693) SHA1(fad19ba37c7987a4d2797200b96ac9c050eb5d94) )
ROM_END

} // anonymous namespace


GAME( 1991, quickp5,   0, quickpick5, quickpick5,  quickpick5_state, empty_init, ROT0, "Konami", "Quick Pick 5", MACHINE_IMPERFECT_GRAPHICS)
GAME( 1993, waijockey, 0, waijockey,  waijockey,   quickpick5_state, empty_init, ROT0, "Konami", "Wai Wai Jockey", MACHINE_NOT_WORKING) // works but not playable due to bad gfx
