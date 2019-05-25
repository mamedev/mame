// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Track & Field Challenge TV Game
https://www.youtube.com/watch?v=wjn1lLylqog

Uses epoxy blobs for CPU etc.
These have been identified as Winbond 2005 BA5962 (large glob) + Winbond 200506 BA5934 (smaller glob)
seems to be G65816 derived with custom vectors?

PCB               Game
TV0001 R1.1       My First DDR
TV0002 R1.0       Track & Field

DDR & TF PCBs look identical, all the parts are in the same place, the traces are the same, and the silkscreened part # for resistors and caps are the same.

Some of m_unkregs must retain value (or return certain things) or RAM containing vectors gets blanked and game crashes.
The G65816 code on these is VERY ugly and difficult to follow, many redundant statements, excessive mode switching, accessing things via pointers to pointers etc.

One of the vectors points to 0x6000, there is nothing mapped there, could it be a small internal ROM or some debug trap for development?

*/

#include "emu.h"

#include "cpu/g65816/g65816.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"


class trkfldch_state : public driver_device
{
public:
	trkfldch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_mainram(*this, "mainram")
	{ }

	void trkfldch(machine_config &config);
	void vectors_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_mainram;

	uint32_t screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trkfldch_map(address_map &map);

	DECLARE_READ8_MEMBER(read_vector);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint8_t m_which_vector;

	DECLARE_READ8_MEMBER(unkregs_r);
	DECLARE_WRITE8_MEMBER(unkregs_w);

	uint8_t m_unkregs[0x100];

	uint8_t m_unkdata[0x100000];
	int m_unkdata_addr;

};

void trkfldch_state::video_start()
{
}

uint32_t trkfldch_state::screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// at 0x1189 in my1stddr
	// at 0xe9c (actually 0x0d0c when fully populated) in trkfldch
	// 7861 / 7860 point here most of the time in both games (so maybe DMA source, or just uses a direct pointer)

	//for (int i = 0x0d0c+0x100*5; i >= 0x0d0c; i -= 5)
	for (int i = 0x1189+0x100*5; i >= 0x1189; i -= 5)
	{
	//	printf("entry %02x %02x %02x %02x %02x\n", m_mainram[i + 0], m_mainram[i + 1], m_mainram[i + 2], m_mainram[i + 3], m_mainram[i + 4]);
	//	int tilegfxbase = 0x1f80; // select mode 
	//	int tilegfxbase = 0x2780; // 2nd demo (+0x800 from above)
	//	int tilegfxbase = 0x3780; // 1st demo and 'letters' minigame (+0x1000 from above)
		int tilegfxbase = (m_unkregs[0x15] * 0x800) - 0x80;

		int y = m_mainram[i + 1];
		int x = m_mainram[i + 3];
		int tile = m_mainram[i + 2];

		int tilehigh = m_mainram[i + 4] & 0x04;
		int tilehigh2 = m_mainram[i + 0] & 0x04;
		int tilehigh3 = m_mainram[i + 0] & 0x08;


		if (tilehigh)
			tile += 0x100;

		if (tilehigh2)
			tile += 0x200;

		if (tilehigh3)
			tile += 0x400;


		int xhigh = m_mainram[i + 4] & 0x01;
		int yhigh = m_mainram[i + 0] & 0x01; // or enable bit?

		x = x | (xhigh << 8);
		y = y | (yhigh << 8);

		y -= 0x100;
		y -= 16;
		x -= 16;

		gfx_element *gfx = m_gfxdecode->gfx(1);
		gfx->transpen(bitmap,cliprect,tile+tilegfxbase,0,0,0,x,y,0);
	}

	return 0;
}




void trkfldch_state::trkfldch_map(address_map &map)
{
	map(0x000000, 0x003fff).ram().share("mainram");

	map(0x006800, 0x006cff).ram();

	map(0x007000, 0x0072ff).ram();

	// 7800 - 78xx look like registers?
	map(0x007800, 0x0078ff).rw(FUNC(trkfldch_state::unkregs_r), FUNC(trkfldch_state::unkregs_w));

	map(0x008000, 0x3fffff).rom().region("maincpu", 0x000000); // good for code mapped at 008000 and 050000 at least
}

void trkfldch_state::vectors_map(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(trkfldch_state::read_vector));
}

READ8_MEMBER(trkfldch_state::read_vector)
{
	uint8_t *rom = memregion("maincpu")->base();

	/* what appears to be a table of vectors apepars at the START of ROM, maybe this gets copied to RAM, maybe used directly?
	00 : (invalid)
	02 : (invalid)
	04 : 0xA2C6  (dummy)
	06 : 0xA334  (real function - vbl?)
	08 : 0xA300  (dummy)
	0a : 0xA2E0  (dummy)
	0c : 0xA2B9  (dummy)
	0e : 0xA2ED  (dummy)
	10 : 0xA2D3  (dummy)
	12 : 0xA327  (dummy)
	14 : 0xA30D  (real function)
	16 : 0x6000  (points at ram? or some internal ROM? we have nothing mapped here, not cleared as RAM either)
	18 : 0xA31A  (dummy)
	1a : 0xA2AC  (dummy)
	1c : 0xA341  (boot vector)
	1e : (invalid)
	*/

	logerror("reading vector offset %02x\n", offset);

	if (offset == 0x0b)
	{	// NMI
		return rom[m_which_vector+1];
	}
	else if (offset == 0x0a)
	{	// NMI
		return rom[m_which_vector];
	}

	// boot vector
	return rom[offset];
}


TIMER_DEVICE_CALLBACK_MEMBER(trkfldch_state::scanline)
{
	int scanline = param;

	if (scanline == 200)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 201)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

	if (scanline == 20)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 21)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

}


static INPUT_PORTS_START( trkfldch )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("O") // selects / forward
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // directions correct based on 'letters' minigame
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X") // goes back
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

// dummy, doesn't appear to be tile based
static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 32, 33, 64, 65, 96, 97 },
	{ 8,10,12,14, 0,2,4,6, 24,26,28,30, 16,18,20,22,},
	{ STEP16(0,128) },
	128*16,
};

static GFXDECODE_START( gfx_trkfldch )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0, tiles16x16_layout, 0, 1 )
GFXDECODE_END

/*

7800 / 7801 seem to be IRQ related

7800 : 0001 - ? (there is no irq 0x00)
       0002 - ? (there is no irq 0x02)
       0004 used in irq 0x04
       0008 used in irq 0x06
	   0010 used in irq 0x08
	   0020 used in irq 0x0a
	   0x40 used in irq 0x0c
	   0x80 used in irq 0x0e (and by code accessing other ports in the main execution?!)

7801 : 0001 used in irq 0x10
     : 0002 used in irq 0x12
	 : 0004 used in irq 0x14
	 : 0008 - ? (there is no irq 0x016, it points to unknown area? and we have no code touching this bit)
	 : 0010 used in irq 0x18
	 : 0020 used in irq 0x1a and 0x06?! (used with OR instead of EOR in 0x06, force IRQ?)
	 : 0x40 - ? (there is no irq 0x1c - it's the boot vector)
	 : 0x80 - ? (there is no irq 0x1e)

*/

READ8_MEMBER(trkfldch_state::unkregs_r)
{
	uint8_t ret = m_unkregs[offset];

	switch (offset)
	{
	case 0x00: // IRQ status?, see above
		ret = machine().rand();
		//logerror("%s: unkregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x01: // IRQ status?, see above
		ret = machine().rand();
		//logerror("%s: unkregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x02: // ends up being read as a side effect of reading a 16-bit word at 0x1, but also directly too?
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x03: // ends up being read as a side effect of reading a 16-bit word at 0x2, any other purpose?
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	case 0x04:
		ret = 0xff;
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x05: // only read as a side effect of reading port 0x4 in 16-bit mode?
		ret = 0xff;
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x06:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	case 0x42:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x43:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x44:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;



	case 0x54:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x55:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x56: // side effect of reading 55
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;




	case 0x70: // read in irq (inputs?)
		ret = ioport("IN0")->read();
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x71:
		ret = ioport("IN1")->read();
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x73:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x74:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x75:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x76:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x77:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x7f:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x80: // only read as a side-effect of reading 0x7f?
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;



	case 0xb6:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0xb7:
		//logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	default:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;
	}
	return ret;
}

WRITE8_MEMBER(trkfldch_state::unkregs_w)
{
	switch (offset)
	{
	case 0x00: // IRQ ack/force?, see above
	//	logerror("%s: unkregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x01: // IRQ maybe status, see above
	//	logerror("%s: unkregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x02: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x03: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x04: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x05: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	// is it significant that 0x10 goes up to 0x1a, 0x20 to 0x2b, 0x30 to 0x3b could be 3 sets of similar things?

	case 0x10:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x11:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x12: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x13:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x14: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x15:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x16:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x17:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x18:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x19:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x1a:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;




	case 0x20: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x21: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x22: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;
	
	case 0x23: // after a long time
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x24: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x25: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x26:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x27:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x28:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x29:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x2a:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x2b:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;




	case 0x30:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x31:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x32: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x33: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x34: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x36: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x37: // rarely
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x3a:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x3b:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;




	case 0x42:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x43:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;




	case 0x54: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x55: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x56: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;



	case 0x60:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x61:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x62:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x63:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x64:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x65:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x66:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x67: // after a long time
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x68: // rarely (my1stddr)
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x69: // after a long time
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x6b: // after a long time
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x6c: // rarely (my1stddr)
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x6d: // after a long time
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;



	// 7x = I/O area?


	case 0x71: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x72: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x73: // some kind of serial device?
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x74: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x75: // some kind of serial device? (used with 73?)
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x76: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x77: // every second or so
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;
	
	case 0x78: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x79: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x7a: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x7f: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0x81: // startup (my1stddr)
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x82: // startup (my1stddr)
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x83:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x84:
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0xb5: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0xb6: // significant data transfer shortly after boot, seems to clock writes with 0073 writing  d0 / c0? (then writes 2 bytes here)
		       // values are coming from a structure in RAM
		       // how does it reset?

		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		m_unkdata[m_unkdata_addr] = data;

		m_unkdata_addr++;
		m_unkdata_addr &= 0xfffff;
		break;



	case 0xca: // startup
		//logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	default:
		//printf("%s: unkregs_w %04x %02x\n", machine().describe_context().c_str(), offset, data);
		break;
	}

	m_unkregs[offset] = data;
}

void trkfldch_state::machine_start()
{
	save_item(NAME(m_unkdata_addr));
	save_item(NAME(m_unkdata));
}

void trkfldch_state::machine_reset()
{
	m_which_vector = 0x06;

	for (int i = 0; i < 0x100; i++)
		m_unkregs[i] = 0x00;

	for (int i = 0; i < 0x100000; i++)
		m_unkdata[i] = 0;
 
	m_unkdata_addr = 0;

}

void trkfldch_state::trkfldch(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, 20000000);
	//m_maincpu->set_addrmap(AS_DATA, &trkfldch_state::mem_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &trkfldch_state::trkfldch_map);
	m_maincpu->set_addrmap(g65816_device::AS_VECTORS, &trkfldch_state::vectors_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(trkfldch_state::scanline), "screen", 0, 1);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(trkfldch_state::screen_update_trkfldch));
	m_screen->set_palette("palette");


	GFXDECODE(config, m_gfxdecode, "palette", gfx_trkfldch); // dummy
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG); // dummy
}

ROM_START( trkfldch )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "trackandfield.bin", 0x000000, 0x400000,  CRC(f4f1959d) SHA1(344dbfe8df1897adf77da6e5ca0435c4d47d6842) )
ROM_END

ROM_START( my1stddr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "myfirstddr.bin", 0x000000, 0x400000, CRC(2ef57bfc) SHA1(9feea5adb9de8fe17e915f3a037e8ddd70e58ae7) )
ROM_END


CONS( 2007, trkfldch,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "Track & Field Challenge", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 2006, my1stddr,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "My First Dance Dance Revolution (US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Japan version has different songs

