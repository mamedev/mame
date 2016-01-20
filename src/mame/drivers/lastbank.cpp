// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Last Bank (c) 1994 Excellent System

    driver by Angelo Salese

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - sound;

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "machine/tc009xlvc.h"

#define MASTER_CLOCK XTAL_14_31818MHz

class lastbank_state : public driver_device
{
public:
	lastbank_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "tc0091lvc")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<tc0091lvc_device> m_vdp;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

	UINT8 m_ram_bank[4];
	UINT8 m_rom_bank;
	UINT8 m_irq_vector[3];
	UINT8 m_irq_enable;
	UINT8 m_mux_data;

	DECLARE_READ8_MEMBER(lastbank_rom_r);

	DECLARE_READ8_MEMBER(lastbank_ram_0_r);
	DECLARE_READ8_MEMBER(lastbank_ram_1_r);
	DECLARE_READ8_MEMBER(lastbank_ram_2_r);
	DECLARE_READ8_MEMBER(lastbank_ram_3_r);
	DECLARE_WRITE8_MEMBER(lastbank_ram_0_w);
	DECLARE_WRITE8_MEMBER(lastbank_ram_1_w);
	DECLARE_WRITE8_MEMBER(lastbank_ram_2_w);
	DECLARE_WRITE8_MEMBER(lastbank_ram_3_w);

	DECLARE_READ8_MEMBER(mux_0_r);
	DECLARE_WRITE8_MEMBER(mux_w);

	DECLARE_READ8_MEMBER(lastbank_rom_bank_r);
	DECLARE_WRITE8_MEMBER(lastbank_rom_bank_w);
	DECLARE_READ8_MEMBER(lastbank_ram_bank_r);
	DECLARE_WRITE8_MEMBER(lastbank_ram_bank_w);
	DECLARE_READ8_MEMBER(lastbank_irq_vector_r);
	DECLARE_WRITE8_MEMBER(lastbank_irq_vector_w);
	DECLARE_READ8_MEMBER(lastbank_irq_enable_r);
	DECLARE_WRITE8_MEMBER(lastbank_irq_enable_w);

	UINT8 ram_bank_r(UINT16 offset, UINT8 bank_num);
	void ram_bank_w(UINT16 offset, UINT8 data, UINT8 bank_num);
	TIMER_DEVICE_CALLBACK_MEMBER(lastbank_irq_scanline);
};

void lastbank_state::video_start()
{
}

UINT32 lastbank_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_vdp->screen_update(screen, bitmap, cliprect);

	return 0;
}

void lastbank_state::screen_eof(screen_device &screen, bool state)
{
	if (state)
	{
		m_vdp->screen_eof();
	}
}

READ8_MEMBER(lastbank_state::lastbank_rom_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	return ROM[offset + m_rom_bank * 0x2000];
}

READ8_MEMBER(lastbank_state::lastbank_rom_bank_r)
{
	return m_rom_bank;
}

WRITE8_MEMBER(lastbank_state::lastbank_rom_bank_w)
{
	m_rom_bank = data;
}

READ8_MEMBER(lastbank_state::lastbank_irq_vector_r)
{
	return m_irq_vector[offset];
}

WRITE8_MEMBER(lastbank_state::lastbank_irq_vector_w)
{
	m_irq_vector[offset] = data;
}

READ8_MEMBER(lastbank_state::lastbank_irq_enable_r)
{
	return m_irq_enable;
}

WRITE8_MEMBER(lastbank_state::lastbank_irq_enable_w)
{
	m_irq_enable = data;
}

READ8_MEMBER(lastbank_state::lastbank_ram_bank_r)
{
	return m_ram_bank[offset];
}

WRITE8_MEMBER(lastbank_state::lastbank_ram_bank_w)
{
	m_ram_bank[offset] = data;
}

UINT8 lastbank_state::ram_bank_r(UINT16 offset, UINT8 bank_num)
{
	address_space &vdp_space = machine().device<tc0091lvc_device>("tc0091lvc")->space();
	return vdp_space.read_byte(offset + (m_ram_bank[bank_num]) * 0x1000);;
}

void lastbank_state::ram_bank_w(UINT16 offset, UINT8 data, UINT8 bank_num)
{
	address_space &vdp_space = machine().device<tc0091lvc_device>("tc0091lvc")->space();
	vdp_space.write_byte(offset + (m_ram_bank[bank_num]) * 0x1000,data);;
}

READ8_MEMBER(lastbank_state::lastbank_ram_0_r) { return ram_bank_r(offset, 0); }
READ8_MEMBER(lastbank_state::lastbank_ram_1_r) { return ram_bank_r(offset, 1); }
READ8_MEMBER(lastbank_state::lastbank_ram_2_r) { return ram_bank_r(offset, 2); }
READ8_MEMBER(lastbank_state::lastbank_ram_3_r) { return ram_bank_r(offset, 3); }
WRITE8_MEMBER(lastbank_state::lastbank_ram_0_w) { ram_bank_w(offset, data, 0); }
WRITE8_MEMBER(lastbank_state::lastbank_ram_1_w) { ram_bank_w(offset, data, 1); }
WRITE8_MEMBER(lastbank_state::lastbank_ram_2_w) { ram_bank_w(offset, data, 2); }
WRITE8_MEMBER(lastbank_state::lastbank_ram_3_w) { ram_bank_w(offset, data, 3); }



READ8_MEMBER(lastbank_state::mux_0_r)
{
	const char *const keynames[2][5] = {
		{"P1_KEY0", "P1_KEY1", "P1_KEY2", "P1_KEY3", "P1_KEY4"},
		{"P2_KEY0", "P2_KEY1", "P2_KEY2", "P2_KEY3", "P2_KEY4"} };
	UINT8 res;
	int i;

	res = 0xff;

	for(i=0;i<5;i++)
	{
		if(m_mux_data & 1 << i)
			res = ioport(keynames[0][i])->read();
	}

	return res;
}

WRITE8_MEMBER(lastbank_state::mux_w)
{
	m_mux_data = data;
}

static ADDRESS_MAP_START( tc0091lvc_map, AS_PROGRAM, 8, lastbank_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_READ(lastbank_rom_r)

	AM_RANGE(0x8000, 0x9fff) AM_RAM

	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(lastbank_ram_0_r,lastbank_ram_0_w)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(lastbank_ram_1_r,lastbank_ram_1_w)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(lastbank_ram_2_r,lastbank_ram_2_w)
	AM_RANGE(0xf000, 0xfdff) AM_READWRITE(lastbank_ram_3_r,lastbank_ram_3_w)

	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("tc0091lvc", tc0091lvc_device, vregs_r, vregs_w)
	AM_RANGE(0xff00, 0xff02) AM_READWRITE(lastbank_irq_vector_r, lastbank_irq_vector_w)
	AM_RANGE(0xff03, 0xff03) AM_READWRITE(lastbank_irq_enable_r, lastbank_irq_enable_w)
	AM_RANGE(0xff04, 0xff07) AM_READWRITE(lastbank_ram_bank_r, lastbank_ram_bank_w)
	AM_RANGE(0xff08, 0xff08) AM_READWRITE(lastbank_rom_bank_r, lastbank_rom_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lastbank_map, AS_PROGRAM, 8, lastbank_state )
	AM_IMPORT_FROM( tc0091lvc_map )
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("COINS") AM_WRITENOP
	AM_RANGE(0xa801, 0xa801) AM_WRITENOP
	AM_RANGE(0xa802, 0xa802) AM_WRITENOP
	AM_RANGE(0xa803, 0xa803) AM_WRITE(mux_w) // mux for $a808 / $a80c
	AM_RANGE(0xa804, 0xa804) AM_READ_PORT("VBLANK")
	AM_RANGE(0xa805, 0xa805) AM_WRITENOP
	AM_RANGE(0xa806, 0xa806) AM_WRITENOP
	AM_RANGE(0xa807, 0xa807) AM_WRITENOP
	AM_RANGE(0xa808, 0xa808) AM_READ(mux_0_r)
	AM_RANGE(0xa80c, 0xa80c) AM_READ(mux_0_r)
	AM_RANGE(0xa81c, 0xa81c) AM_READ_PORT("DSW0")
	AM_RANGE(0xa81d, 0xa81d) AM_READ_PORT("DSW1")
	AM_RANGE(0xa81e, 0xa81e) AM_READ_PORT("DSW2")
	AM_RANGE(0xa81f, 0xa81f) AM_READ_PORT("DSW3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( lastbank_audio_map, AS_PROGRAM, 8, lastbank_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( lastbank_audio_io, AS_IO, 8, lastbank_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( lastbank )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Bookkeeping")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("VBLANK")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service")
	PORT_DIPNAME( 0x04, 0x04, "Hopper Count" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Hopper Empty" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN ) // bit 6 is a status of some sort
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("1P 5-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("1P 4-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("1P 4-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("1P Small") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P FF") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("1P Big") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_NAME("1P Cancel") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("1P 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P 2-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Payout") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("1P Autobet") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("1P 3-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, O2+0*32, O2+1*32, O2+2*32, O2+3*32, O2+4*32, O2+5*32, O2+6*32, O2+7*32 },
	8*8*4*4
};
#undef O
#undef O2


static GFXDECODE_START( lastbank )
	GFXDECODE_ENTRY( "gfx1",        0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1",        0, sp2_layout, 0, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(lastbank_state::lastbank_irq_scanline)
{
	int scanline = param;

	if (scanline == 240 && (m_irq_enable & 4))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_vector[2]);
	}

	if (scanline == 0 && (m_irq_enable & 2))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_vector[1]);
	}
}

static MACHINE_CONFIG_START( lastbank, lastbank_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/4) //!!! TC0091LVC !!!
	MCFG_CPU_PROGRAM_MAP(lastbank_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", lastbank_state, lastbank_irq_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu",Z80,MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(lastbank_audio_map)
	MCFG_CPU_IO_MAP(lastbank_audio_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(lastbank_state, nmi_line_pulse, 60)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	//MCFG_MACHINE_START_OVERRIDE(lastbank_state,lastbank)
	//MCFG_MACHINE_RESET_OVERRIDE(lastbank_state,lastbank)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lastbank_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(lastbank_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lastbank )
	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("tc0091lvc", TC0091LVC, 0)
	MCFG_TC0091LVC_GFXDECODE("gfxdecode")
	MCFG_TC0091LVC_PALETTE("palette")

//  MCFG_VIDEO_START_OVERRIDE(lastbank_state,lastbank)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// es8712
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( lastbank )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "3.u9", 0x00000, 0x40000, CRC(f430e1f0) SHA1(dd5b697f5c2250d98911f4c7d3e7d4cc16b0b40f) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "8.u48", 0x00000, 0x10000, CRC(3a7bfe10) SHA1(7dc543e11d3c0b9872fcc622339ade25383a1eb3) )

	ROM_REGION( 0x120000, "gfx1", 0 )
	ROM_LOAD( "u11",   0x000000, 0x100000, CRC(2588d82d) SHA1(426f6821862d54123e53410e2776586ddf6b21e7) )
	ROM_LOAD( "5.u10", 0x100000, 0x020000, CRC(51f3c5a7) SHA1(73d4c8817fe96d75be32c43e816e93c52b5d2b27) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "6.u55", 0x00000, 0x40000, CRC(9e78e234) SHA1(031f93e4bc338d0257fa673da7ce656bb1cda5fb) )
	ROM_LOAD( "7.u60", 0x40000, 0x80000, CRC(41be7146) SHA1(00f1c0d5809efccf888e27518a2a5876c4b633d8) )
ROM_END

GAME( 1994, lastbank,  0,   lastbank, lastbank, driver_device,  0, ROT0, "Excellent System", "Last Bank (v1.16)", MACHINE_NO_SOUND )
