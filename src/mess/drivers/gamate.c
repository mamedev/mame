/******************************************************************************
 PeT mess@utanet.at 2007, 2014
******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "rendlay.h"

class gamate_state : public driver_device
{
public:
	gamate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
//      , m_gfxdecode(*this, "gfxdecode")
		, m_io_joy(*this, "JOY")
		,   m_palette(*this, "palette")
	{ }

	DECLARE_PALETTE_INIT(gamate);
	DECLARE_READ8_MEMBER(video_r);
	DECLARE_READ8_MEMBER(pad_r);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_WRITE8_MEMBER(audio_w);
	DECLARE_WRITE8_MEMBER(bios_w);
	DECLARE_DRIVER_INIT(gamate);
	UINT32 screen_update_gamate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gamate_interrupt);

private:
	virtual void machine_start();

	struct
	{
	UINT8 reg[8];
	struct {
		bool write; // else tilemap
		bool page2; // else page1
		UINT8 data[2][0x100][0x20];
		} bitmap;
	struct {
		UINT8 data[32][32];
	} tilemap;
	UINT8 x, y;
	} video;

//  UINT8 m_ports[5];
//  UINT8 m_ram[0x4000];
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
//  required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_io_joy;
	required_device<palette_device> m_palette;
};

WRITE8_MEMBER( gamate_state::video_w )
{
	if (m_maincpu->pc()<0xf000)
	logerror("%.6f %04x video write %04x %02x\n", machine().time().as_double(), m_maincpu->pc(), offset,data);
	video.reg[offset]=data;
	switch (offset) {
	case 1: video.bitmap.write=data&0x40;break; // probably y increment
	case 4: video.bitmap.page2=data&0x80;video.x=data&0x7f;break;
	case 5: video.y=data;break;
	case 7:
	if (video.bitmap.write) {
		if (video.x<ARRAY_LENGTH(video.bitmap.data[0][0]) /*&& video.y<ARRAY_LENGTH(video.bitmap.data[0])*/) 
		video.bitmap.data[video.bitmap.page2][video.y][video.x]=data;
		else
		logerror("%.6f %04x video bitmap x %x invalid\n",machine().time().as_double(), m_maincpu->pc(), video.x);
		video.y++;
	} else {
		if (video.x<ARRAY_LENGTH(video.tilemap.data[0]) && (video.y&0x1f)<ARRAY_LENGTH(video.tilemap.data))
		video.tilemap.data[video.y&0x1f][video.x]=data;
		else
		logerror("%.6f %04x video tilemap %x %x invalid\n",machine().time().as_double(), m_maincpu->pc(), video.x, video.y);
		video.x++;
	}
	}
}

READ8_MEMBER( gamate_state::video_r )
{
	if (offset!=6) return 0;
	UINT8 data=0;
	if (video.bitmap.write) {
		if (video.x<ARRAY_LENGTH(video.bitmap.data[0][0]) /*&& video.y<ARRAY_LENGTH(video.bitmap.data[0])*/)
		data=video.bitmap.data[video.bitmap.page2][video.y][video.x];
		else
		logerror("%.6f video bitmap x %x invalid\n",machine().time().as_double(),video.x);
	} else {
		if (video.x<ARRAY_LENGTH(video.tilemap.data[0]) && video.y<ARRAY_LENGTH(video.tilemap.data))
		data=video.tilemap.data[video.y][video.x];
		else
		logerror("%.6f video tilemap %x %x invalid\n",machine().time().as_double(),video.x, video.y);
	}
	if (m_maincpu->pc()<0xf000)
	logerror("%.6f video read %04x %02x\n",machine().time().as_double(),offset, data);
	return data;
}

WRITE8_MEMBER( gamate_state::audio_w )
{
	//  logerror("%.6f audio write %04x %02x\n",timer_get_time(),offset,data);
}

WRITE8_MEMBER( gamate_state::bios_w )
{
	UINT8 *memory = *memregion("main_cpu"); //memory_region (REGION_CPU1);

	unsigned short stack=m_maincpu->sp();//cpu_get_reg(M6502_S)|0x100;
	unsigned short address= memory[stack+1]|(memory[stack+2]<<8);
	switch (offset) {
	case 0x12:
	logerror("%.6f bios api %04x %04x string:%04x x:%02x y:%02x\n",
				machine().time().as_double(), offset|0xf000, address,
				memory[0]|(memory[1]<<8), 0, 0);//cpu_get_reg(M6502_X), cpu_get_reg(M6502_Y) );
	break;
	case 0x15:
	logerror("%.6f bios api %04x %04x string:%04x x:%02x y:%02x\n",
				machine().time().as_double(), offset|0xf000, address,
				memory[0]|(memory[1]<<8), 0, 0); //cpu_get_reg(M6502_X), cpu_get_reg(M6502_Y) );
	break;
	case 0x18:
	logerror("%.6f bios api %04x %04x string:%04x\n",machine().time().as_double(), offset|0xf000, address,
				memory[0]|(memory[1]<<8) );
	break;
	case 0x1b:
	logerror("%.6f bios api %04x %04x string:%04x\n",machine().time().as_double(), offset|0xf000, address,
				memory[0]|(memory[1]<<8) );
	break;
	case 0x1e:
	logerror("%.6f bios api %04x %04x string:%04x\n",machine().time().as_double(), offset|0xf000, address,
				memory[0]|(memory[1]<<8) );
	break;
	case 0x2a: // cube up menu lighting
	logerror("%.6f bios api %04x %04x 1c1d:%04x a:%02x x:%02x y:%02x\n",
				machine().time().as_double(), offset|0xf000, address,
				memory[0x1c]|(memory[0x1d]<<8),
				0,0,0);//cpu_get_reg(M6502_A), cpu_get_reg(M6502_X), cpu_get_reg(M6502_Y) );
	break;
	default:
	logerror("%.6f bios api %04x %04x\n",machine().time().as_double(), offset|0xf000, address);
	}
}

READ8_MEMBER( gamate_state::pad_r )
{
	UINT8 data=m_io_joy->read();//readinputport(0);
	//  logerror("%.6f pad read %04x %02x\n",timer_get_time(),offset,data);
	return data;
}

static ADDRESS_MAP_START( gamate_mem, AS_PROGRAM, 8, gamate_state )
//  AM_RANGE(0x4000, 0x7fff) AM_READWRITE(gmaster_io_r, gmaster_io_w)

	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x400d) AM_WRITE(audio_w)
	AM_RANGE(0x4400, 0x4400) AM_READ(pad_r)
//  AM_RANGE(0x5006, 0x5006) AM_READ(video_r)
//  AM_RANGE(0x5000, 0x5007) AM_WRITE(video_w)
	AM_RANGE(0x5000, 0x5007) AM_READWRITE(video_r, video_w)

	AM_RANGE(0x6000, 0xdfff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( gamate )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // left?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // rechts?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("start/pause")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("select")
INPUT_PORTS_END

#if 0
static const struct gfx_layout gamate_charlayout =
{
		4,      /* width of object */
		1,      /* height of object */
		256,/* 256 characters */
		2,      /* bits per pixel */
		{ 0,1 }, /* no bitplanes */
		/* x offsets */
		{ 0,2,4,6 },
		/* y offsets */
		{ 0 },
		8*1 /* size of 1 object in bits */
};

static const unsigned short gamate_palette[4] =
{
	0,1,2,3
};
#endif

/* palette in red, green, blue tribles */
static const unsigned char gamate_colors[4][3] =
{
	{ 255,255,255 },
	{ 0xa0, 0xa0, 0xa0 },
	{ 0x60, 0x60, 0x60 },
	{ 0, 0, 0 }
};

#if 0
static GFXDECODE_START( gamate_charlayout )
		GFXDECODE_ENTRY( "gfx1", 0x0000, gamate_charlayout, 0, 0x100 )
GFXDECODE_END
#endif

PALETTE_INIT_MEMBER(gamate_state, gamate)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		palette.set_pen_color(i, gamate_colors[i][0], gamate_colors[i][1], gamate_colors[i][2]);
	}
#if 0
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, arcadia_colors[i]);

	for (int i = 0; i < 128+8; i++)
		palette.set_pen_indirect(i, arcadia_palette[i]);
#endif
}


static void BlitPlane(UINT16* line, UINT8 plane1, UINT8 plane2)
{
	line[3]=(plane1&1)|((plane2<<1)&2);
	line[2]=((plane1>>1)&1)|((plane2<<0)&2);
	line[1]=((plane1>>2)&1)|((plane2>>1)&2);
	line[0]=((plane1>>3)&1)|((plane2>>2)&2);
}

UINT32 gamate_state::screen_update_gamate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, j;
	for (y=0;y<160;y++) {
	for (x=0, j=0;x<160;x+=8, j++) {
//  for (y=0;y<256;y++) {
//    for (x=0, j=0;x<256;x+=8, j++) {
		UINT8 d1=video.bitmap.data[0][y][j];
		UINT8 d2=video.bitmap.data[1][y][j];
#if 0
		UINT16 data=PLANES2_2_PACKED(d1, d2);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), (data>>8)&0xff,0,0,0, x, y);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), data&0xff,0,0,0, x+4, y);
#else
			BlitPlane(&bitmap.pix16(y, x+4), d1, d2);
			BlitPlane(&bitmap.pix16(y, x), d1>>4, d2>>4);
#endif
	}
	}
	for (y=0; y<32; y++) {
	for (x=0; x<32; x++) {
#if 0
		UINT8 d=video.tilemap.data[y][x];
		if (d) {
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+1);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+2);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+3);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+4);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+5);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+6);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 256+x*8, y*8+7);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+1);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+2);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+3);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+4);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+5);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+6);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0xff,0,0,0, 260+x*8, y*8+7);
		} else {
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+1);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+2);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+3);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+4);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+5);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+6);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 256+x*8, y*8+7);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+1);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+2);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+3);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+4);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+5);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+6);
		m_gfxdecode->gfx(0)->opaque(bitmap, bitmap.cliprect(), 0,0,0,0, 260+x*8, y*8+7);
		}
#endif
	}
	}
	return 0;
}

DRIVER_INIT_MEMBER(gamate_state,gamate)
{
	memset(&video, 0, sizeof(video));/* memset(m_ram, 0, sizeof(m_ram));*/
	UINT8 *gfx=memregion("gfx1")->base();   for (int i=0; i<256; i++) gfx[i]=i;
}


void gamate_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0xdfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
#if 0
	save_item(NAME(m_video.data));
	save_item(NAME(m_video.index));
	save_item(NAME(m_video.x));
	save_item(NAME(m_video.y));
	save_item(NAME(m_video.mode));
	save_item(NAME(m_video.delayed));
	save_item(NAME(m_video.pixels));
	save_item(NAME(m_ports));
	save_item(NAME(m_ram));
#endif
}


INTERRUPT_GEN_MEMBER(gamate_state::gamate_interrupt)
{
//  m_maincpu->set_input_line(UPD7810_INTFE1, ASSERT_LINE);
	static bool state=false;
//  m_maincpu->set_input_line(M6502_IRQ_LINE, state?ASSERT_LINE: CLEAR_LINE);
	state=!state;
//  cpu_set_irq_line(0, M6502_INT_IRQ, PULSE_LINE);
}

static MACHINE_CONFIG_START( gamate, gamate_state )
	MCFG_CPU_ADD("maincpu", M6502, 4433000)
	MCFG_CPU_PROGRAM_MAP(gamate_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gamate_state,  gamate_interrupt)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
#if 0
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
#else
	MCFG_SCREEN_SIZE(160, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 160-1)
#endif
	MCFG_SCREEN_UPDATE_DRIVER(gamate_state, screen_update_gamate)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", gamate )
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gamate_colors))
//  MCFG_PALETTE_INDIRECT_ENTRIES(4)
	MCFG_PALETTE_INIT_OWNER(gamate_state, gamate)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "gamate_cart")
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list", "gamate")
MACHINE_CONFIG_END


ROM_START(gamate)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("gamate.bin", 0xf000, 0x1000, BAD_DUMP CRC(b8bf539b) SHA1(d00cb43b8a4cb0cc7fea06bee5f08490a71f5690) )
//  ROM_LOAD("gamate.bin", 0xf000, 0x1000, CRC(b8bf539b) SHA1(d00cb43b8a4cb0cc7fea06bee5f08490a71f5690) )
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END


/*    YEAR  NAME      PARENT  COMPAT    MACHINE   INPUT    CLASS          INIT      COMPANY    FULLNAME */
CONS( 19??, gamate,  0,      0,        gamate,  gamate, gamate_state, gamate, "Bit Corp", "Gamate", GAME_NOT_WORKING | GAME_NO_SOUND)
