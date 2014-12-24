/******************************************************************************
 PeT mess@utanet.at 2007, 2014
 Peter Wilhelmsen peter.wilhelmsen@gmail.com
 Morten Shearman Kirkegaard morten+gamate@afdelingp.dk
 Juan Félix Mateos vectrex@hackermesh.org
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
//		, m_gfxdecode(*this, "gfxdecode")
		, m_io_joy(*this, "JOY")
		, m_palette(*this, "palette")
		, m_bios(*this, "bios")
	{ }

	DECLARE_PALETTE_INIT(gamate);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_READ8_MEMBER(gamate_cart_protection_r);
	DECLARE_WRITE8_MEMBER(gamate_cart_protection_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitchmulti_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitch_w);
	DECLARE_READ8_MEMBER(gamate_video_r);
	DECLARE_READ8_MEMBER(gamate_pad_r);
	DECLARE_WRITE8_MEMBER(gamate_video_w);
	DECLARE_READ8_MEMBER(gamate_audio_r);
	DECLARE_WRITE8_MEMBER(gamate_audio_w);
	DECLARE_WRITE8_MEMBER(gamate_bios_w);
	DECLARE_DRIVER_INIT(gamate);
	UINT32 screen_update_gamate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gamate_interrupt);
	TIMER_CALLBACK_MEMBER(gamate_timer);
	TIMER_CALLBACK_MEMBER(gamate_timer2);

private:
	virtual void machine_start();

	struct
	{
  	UINT8 reg[8];
  	struct {
    	bool write;
    	bool page2; // else page1
	UINT8 ypos, xpos/*tennis*/;
    	UINT8 data[2][0x100][0x20];
	  } bitmap;
  	UINT8 x, y;
		bool y_increment;
	} video;

	struct {
		int bit_shifter;
		UINT8 cartridge_byte;
		UINT16 address; // in reality something more like short local cartridge address offset
		bool unprotected;
		bool failed;
	} card_protection;

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
//	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_io_joy;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_bios;
	emu_timer *timer1;
	emu_timer *timer2;
};

WRITE8_MEMBER( gamate_state::gamate_cart_protection_w )
{
	switch (offset) {
	case 0:
		card_protection.failed= card_protection.failed || ((card_protection.cartridge_byte&0x80)!=0) != ((data&4)!=0);
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter>=8) {
			card_protection.cartridge_byte=m_cart->get_rom_base()[card_protection.address++];
			card_protection.bit_shifter=0;
		}
		break;
	}
}
READ8_MEMBER( gamate_state::gamate_cart_protection_r )
{
	UINT8 ret=1;
	switch (offset) {
	case 0:
		ret=(card_protection.cartridge_byte&0x80)?2:0;
		card_protection.cartridge_byte<<=1;
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter>=8) {
			card_protection.bit_shifter=0;
			card_protection.cartridge_byte=m_cart->get_rom_base()[card_protection.address++];
			card_protection.unprotected=true;
			if (!card_protection.failed) {
			} // now protection chip on cartridge activates cartridge chip select on cpu accesses
		}
		break;
	}
	return ret;
}

READ8_MEMBER( gamate_state::protection_r ) { return 1; }

WRITE8_MEMBER( gamate_state::gamate_video_w )
{
  video.reg[offset]=data;
  switch (offset) {
  case 1: video.bitmap.write=data&0xc0; // more addressing mode
		video.y_increment=data&0x40;
		break;
	case 2: video.bitmap.xpos=data;break; // at least 7 bits
	case 3: video.bitmap.ypos=data;break; // at least 7 bits
  case 4: video.bitmap.page2=data&0x80;video.x=data&0x7f;break;
  case 5: video.y=data;break;
  case 7:
    if (video.bitmap.write) {
      if (video.x<ARRAY_LENGTH(video.bitmap.data[0][0]) /*&& video.y<ARRAY_LENGTH(video.bitmap.data[0])*/)
        video.bitmap.data[video.bitmap.page2][video.y][video.x]=data;
      else
        logerror("%.6f %04x video bitmap x %x invalid\n",machine().time().as_double(), m_maincpu->pc(), video.x);
    } else {
        video.bitmap.data[0][video.y][video.x&(ARRAY_LENGTH(video.bitmap.data[0][0])-1)]=data;
    }
    if (video.y_increment) video.y++;
		else video.x++;
  }
}

WRITE8_MEMBER( gamate_state::cart_bankswitchmulti_w )
{
	membank("bankmulti")->set_base(m_cart->get_rom_base()+0x4000*data);
}

WRITE8_MEMBER( gamate_state::cart_bankswitch_w )
{
	membank("bank")->set_base(m_cart->get_rom_base()+0x4000*data);
}

READ8_MEMBER( gamate_state::gamate_video_r )
{
	if (offset!=6) return 0;
  UINT8 data=0;
  if (video.bitmap.write) {
      if (video.x<ARRAY_LENGTH(video.bitmap.data[0][0]) /*&& video.y<ARRAY_LENGTH(video.bitmap.data[0])*/)
        data=video.bitmap.data[video.bitmap.page2][video.y][video.x];
      else
        logerror("%.6f video bitmap x %x invalid\n",machine().time().as_double(),video.x);
  } else {
    data=video.bitmap.data[0][video.y][video.x&(ARRAY_LENGTH(video.bitmap.data[0][0])-1)];
  }
  if (m_maincpu->pc()<0xf000)
    logerror("%.6f video read %04x %02x\n",machine().time().as_double(),offset, data);
  return data;
}

WRITE8_MEMBER( gamate_state::gamate_audio_w )
{
  logerror("%.6f %04x audio write %04x %02x\n",machine().time().as_double(),m_maincpu->pc(),offset,data);
}

READ8_MEMBER( gamate_state::gamate_audio_r )
{
  logerror("%.6f %04x audio read %04x \n",machine().time().as_double(),m_maincpu->pc(),offset);
	return 0;
}


READ8_MEMBER( gamate_state::gamate_pad_r )
{
  UINT8 data=m_io_joy->read();
  return data;
}

static ADDRESS_MAP_START( gamate_mem, AS_PROGRAM, 8, gamate_state )
 	AM_RANGE(0x0000, 0x03ff) AM_RAM
  AM_RANGE(0x4000, 0x400d) AM_READWRITE(gamate_audio_r, gamate_audio_w)
  AM_RANGE(0x4400, 0x4400) AM_READ(gamate_pad_r)
  AM_RANGE(0x5000, 0x5007) AM_READWRITE(gamate_video_r, gamate_video_w)
  AM_RANGE(0x5a00, 0x5a00) AM_READ(protection_r)

  AM_RANGE(0x6000, 0x9fff) AM_READ_BANK("bankmulti")
  AM_RANGE(0xa000, 0xdfff) AM_READ_BANK("bank")

	AM_RANGE(0x6000, 0x6002) AM_READWRITE(gamate_cart_protection_r, gamate_cart_protection_w)
//	AM_RANGE(0x6000, 0xdfff) AM_READWRITE(gamate_cart_r, gamate_cart_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(cart_bankswitchmulti_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(cart_bankswitch_w)

  AM_RANGE(0xf000, 0xffff) AM_ROM AM_SHARE("bios")
ADDRESS_MAP_END


static INPUT_PORTS_START( gamate )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("start/pause")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("select")
INPUT_PORTS_END

#ifdef UNUSED_CODE
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

static GFXDECODE_START( gamate_charlayout )
        GFXDECODE_ENTRY( "gfx1", 0x0000, gamate_charlayout, 0, 0x100 )
GFXDECODE_END
#endif

/* palette in red, green, blue tribles */
static const unsigned char gamate_colors[4][3] =
{
  { 255,255,255 },
  { 0xa0, 0xa0, 0xa0 },
  { 0x60, 0x60, 0x60 },
  { 0, 0, 0 }
};

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
  for (y=0;y<152;y++) {
    for (x=-(video.bitmap.xpos&7), j=0;x<160;x+=8, j++) {
      UINT8 d1=video.bitmap.data[0][(y+video.bitmap.ypos)&0xff][(j+video.bitmap.xpos/8)&0x1f];
      UINT8 d2=video.bitmap.data[1][(y+video.bitmap.ypos)&0xff][(j+video.bitmap.xpos/8)&0x1f];
      BlitPlane(&bitmap.pix16(y, x+4), d1, d2);
      BlitPlane(&bitmap.pix16(y, x), d1>>4, d2>>4);
    }
  }
  return 0;
}

DRIVER_INIT_MEMBER(gamate_state,gamate)
{
	memset(&video, 0, sizeof(video));/* memset(m_ram, 0, sizeof(m_ram));*/
	UINT8 *gfx=memregion("gfx1")->base();	for (int i=0; i<256; i++) gfx[i]=i;
	timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer),this));
	timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer2),this));
}


void gamate_state::machine_start()
{
	if (m_cart->exists()) {
//		m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, READ8_DELEGATE(gamate_state, gamate_cart_protection_r));
		membank("bankmulti")->set_base(m_cart->get_rom_base());
		membank("bank")->set_base(m_cart->get_rom_base()+0x4000); // bankswitched games in reality no offset
	}
	m_bios[0xdf1]=0xea; m_bios[0xdf2]=0xea; // $47 protection readback
	card_protection.address=0x6005-0x6001;
	card_protection.bit_shifter=0;
	card_protection.cartridge_byte=m_cart->get_rom_base()[card_protection.address++];//m_cart_rom[card_protection.address++];
	card_protection.failed=false;
	card_protection.unprotected=false;
	timer2->enable(TRUE);
	timer2->reset(m_maincpu->cycles_to_attotime(1000));
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

TIMER_CALLBACK_MEMBER(gamate_state::gamate_timer)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	timer1->enable(FALSE);
}

TIMER_CALLBACK_MEMBER(gamate_state::gamate_timer2)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	timer1->enable(TRUE);
	timer1->reset(m_maincpu->cycles_to_attotime(10/* cycles short enought to clear irq line early enough*/));
	timer2->enable(TRUE);
	timer2->reset(m_maincpu->cycles_to_attotime(40000));
}


INTERRUPT_GEN_MEMBER(gamate_state::gamate_interrupt)
{
}

static MACHINE_CONFIG_START( gamate, gamate_state )
	MCFG_CPU_ADD("maincpu", M6502, 4433000)
	MCFG_CPU_PROGRAM_MAP(gamate_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gamate_state,  gamate_interrupt)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(160, 152)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 152-1)
	MCFG_SCREEN_UPDATE_DRIVER(gamate_state, screen_update_gamate)
	MCFG_SCREEN_PALETTE("palette")

//	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gamate )
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gamate_colors))
//	MCFG_PALETTE_INDIRECT_ENTRIES(4)
	MCFG_PALETTE_INIT_OWNER(gamate_state, gamate)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "gamate_cart")
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list","gamate")
MACHINE_CONFIG_END


ROM_START(gamate)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("gamate_bios_umc.bin", 0xf000, 0x1000, CRC(07090415) SHA1(ea449dc607601f9a68d855ad6ab53800d2e99297) )
 ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END


/*    YEAR  NAME      PARENT  COMPAT    MACHINE   INPUT    CLASS          INIT      COMPANY    FULLNAME */
CONS( 19??, gamate,  0,      0,        gamate,  gamate, gamate_state, gamate, "Bit Corp", "Gamate", GAME_NO_SOUND)


