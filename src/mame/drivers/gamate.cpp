// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at 2007, 2014
 Peter Wilhelmsen peter.wilhelmsen@gmail.com
 Morten Shearman Kirkegaard morten+gamate@afdelingp.dk
 Juan F??lix Mateos vectrex@hackermesh.org

 nmi unknown
 bomb blast top status line missing
 ******************************************************************************/

#include "emu.h"
#include "includes/gamate.h"
#include "ui/ui.h"
#include "rendlay.h"
#include "softlist.h"

class gamate_state : public driver_device
{
public:
	gamate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sound(*this, "custom")
		, m_cart(*this, "cartslot")
		, m_io_joy(*this, "JOY")
		, m_palette(*this, "palette")
		, m_bios(*this, "bios")
		, m_bank(*this, "bank")
		, m_bankmulti(*this, "bankmulti")
	{ }

	DECLARE_PALETTE_INIT(gamate);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_READ8_MEMBER(newer_protection_set);
	DECLARE_WRITE8_MEMBER(protection_reset);
	DECLARE_READ8_MEMBER(gamate_cart_protection_r);
	DECLARE_WRITE8_MEMBER(gamate_cart_protection_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitchmulti_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitch_w);
	DECLARE_READ8_MEMBER(gamate_video_r);
	DECLARE_READ8_MEMBER(gamate_nmi_r);
	DECLARE_WRITE8_MEMBER(gamate_video_w);
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
		struct
		{
			bool page2; // else page1
			UINT8 ypos, xpos/*tennis*/;
			UINT8 data[2][0x100][0x20];
		} bitmap;
		UINT8 x, y;
		bool y_increment;
	} video;

	struct
	{
		bool set;
		int bit_shifter;
		UINT8 cartridge_byte;
		UINT16 address; // in reality something more like short local cartridge address offset
		bool unprotected;
		bool failed;
	} card_protection;

	required_device<cpu_device> m_maincpu;
	required_device<gamate_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_bios;
	required_memory_bank m_bank;
	required_memory_bank m_bankmulti;
	emu_timer *timer1;
	emu_timer *timer2;
	UINT8 bank_multi;
	UINT8 *m_cart_ptr;
};

WRITE8_MEMBER( gamate_state::gamate_cart_protection_w )
{
	logerror("%.6f protection write %x %x address:%x data:%x shift:%d\n",machine().time().as_double(), offset, data, card_protection.address, card_protection.cartridge_byte, card_protection.bit_shifter);

	switch (offset)
	{
	case 0:
		card_protection.failed= card_protection.failed || ((card_protection.cartridge_byte&0x80)!=0) != ((data&4)!=0);
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter>=8)
		{
			card_protection.cartridge_byte=m_cart_ptr[card_protection.address++];
			card_protection.bit_shifter=0;
		}
		break;
	}
}

READ8_MEMBER( gamate_state::gamate_cart_protection_r )
{
	UINT8 ret=1;
	if (card_protection.bit_shifter==7 && card_protection.unprotected)
	{
		ret=m_cart_ptr[bank_multi*0x4000];
	}
	else
	{
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter==8)
		{
			card_protection.bit_shifter=0;
			card_protection.cartridge_byte='G';
			card_protection.unprotected=true;
		}
		ret=(card_protection.cartridge_byte&0x80) ? 2 : 0;
		if (card_protection.bit_shifter==7 && !card_protection.failed)
		{ // now protection chip on cartridge activates cartridge chip select on cpu accesses
//          m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, READ8_DELEGATE(gamate_state, gamate_cart_protection_r)); // next time I will try to get this working
		}
		card_protection.cartridge_byte<<=1;
	}
	logerror("%.6f protection read %x %x address:%x data:%x shift:%d\n",machine().time().as_double(), offset, ret, card_protection.address, card_protection.cartridge_byte, card_protection.bit_shifter);
	return ret;
}

READ8_MEMBER( gamate_state::protection_r )
{
	return card_protection.set? 3: 1;
} // bits 0 and 1 checked

WRITE8_MEMBER( gamate_state::protection_reset )
{
// writes 0x20
	card_protection.address=0x6005-0x6001;
	card_protection.bit_shifter=0;
	card_protection.cartridge_byte=m_cart_ptr[card_protection.address++]; //m_cart_rom[card_protection.address++];
	card_protection.failed=false;
	card_protection.unprotected=false;
}

READ8_MEMBER( gamate_state::newer_protection_set )
{
	card_protection.set=true;
	return 0;
}

WRITE8_MEMBER( gamate_state::gamate_video_w )
{
	video.reg[offset]=data;
	switch (offset)
	{
		case 1:
			if (data&0xf)
				printf("lcd mode %x\n", data);
			video.y_increment=data&0x40;
			break;
		case 2:
			video.bitmap.xpos=data;
			break;
		case 3:
			if (data>=200)
				printf("lcd ypos: %x\n", data);
			video.bitmap.ypos=data;
			break;
		case 4:
			video.bitmap.page2=data&0x80;
			video.x=data&0x1f;
			break;
		case 5:
			video.y=data;
			break;
		case 7:
			video.bitmap.data[video.bitmap.page2][video.y][video.x&(ARRAY_LENGTH(video.bitmap.data[0][0])-1)]=data;
			if (video.y_increment)
				video.y++;
			else
				video.x++; // overruns
	}
}

WRITE8_MEMBER( gamate_state::cart_bankswitchmulti_w )
{
	bank_multi=data;
	m_bankmulti->set_base(m_cart_ptr+0x4000*data+1);
}

WRITE8_MEMBER( gamate_state::cart_bankswitch_w )
{
	m_bank->set_base(m_cart_ptr+0x4000*data);
}

READ8_MEMBER( gamate_state::gamate_video_r )
{
	if (offset!=6)
		return 0;
	UINT8 data = video.bitmap.data[video.bitmap.page2][video.y][video.x&(ARRAY_LENGTH(video.bitmap.data[0][0])-1)];
//  if (m_maincpu->pc()<0xf000)
//    machine().ui().popup_time(2, "lcd read x:%x y:%x mode:%x data:%x\n", video.x, video.y, video.reg[1], data);
	if (video.y_increment)
		video.y++;
	else
		video.x++; // overruns?

	return data;
}

READ8_MEMBER( gamate_state::gamate_nmi_r )
{
	UINT8 data=0;
	machine().ui().popup_time(2, "nmi/4800 read\n");
	return data;
}

static ADDRESS_MAP_START( gamate_mem, AS_PROGRAM, 8, gamate_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x400d) AM_DEVREADWRITE("custom", gamate_sound_device, device_r, device_w)
	AM_RANGE(0x4400, 0x4400) AM_READ_PORT("JOY")
	AM_RANGE(0x4800, 0x4800) AM_READ(gamate_nmi_r)
	AM_RANGE(0x5000, 0x5007) AM_READWRITE(gamate_video_r, gamate_video_w)
	AM_RANGE(0x5800, 0x5800) AM_READ(newer_protection_set)
	AM_RANGE(0x5900, 0x5900) AM_WRITE(protection_reset)
	AM_RANGE(0x5a00, 0x5a00) AM_READ(protection_r)
	AM_RANGE(0x6001, 0x9fff) AM_READ_BANK("bankmulti")
	AM_RANGE(0xa000, 0xdfff) AM_READ_BANK("bank")
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(gamate_cart_protection_r, gamate_cart_protection_w)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start/Pause")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
INPUT_PORTS_END

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
	for (y=0;y<152;y++)
	{
		for (x=-(video.bitmap.xpos&7), j=0;x<160;x+=8, j++)
		{
			UINT8 d1, d2;
			if (video.bitmap.ypos<200)
			{
				d1=video.bitmap.data[0][(y+video.bitmap.ypos)%200][(j+video.bitmap.xpos/8)&0x1f];
				d2=video.bitmap.data[1][(y+video.bitmap.ypos)%200][(j+video.bitmap.xpos/8)&0x1f];
			}
			else
			if ((video.bitmap.ypos&0xf)<8)
			{ // lcdtest, of course still some registers not known, my gamate doesn't display bottom lines; most likely problematic 200 warp around hardware! no real usage
				int yi=(y+(video.bitmap.ypos&0xf)-8);
				if (yi<0)
					yi=video.bitmap.ypos+y; // in this case only 2nd plane used!?, source of first plane?
				d1=video.bitmap.data[0][yi][(j+video.bitmap.xpos/8)&0x1f]; // value of lines bevor 0 chaos
				d2=video.bitmap.data[1][yi][(j+video.bitmap.xpos/8)&0x1f];
			}
			else
			{
				d1=video.bitmap.data[0][y][(j+video.bitmap.xpos/8)&0x1f];
				d2=video.bitmap.data[1][y][(j+video.bitmap.xpos/8)&0x1f];
			}
			BlitPlane(&bitmap.pix16(y, x+4), d1, d2);
			BlitPlane(&bitmap.pix16(y, x), d1>>4, d2>>4);
		}
	}
	return 0;
}

DRIVER_INIT_MEMBER(gamate_state,gamate)
{
	memset(&video, 0, sizeof(video));/* memset(m_ram, 0, sizeof(m_ram));*/
	timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer),this));
	timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer2),this));
}


void gamate_state::machine_start()
{
	m_cart_ptr = memregion("maincpu")->base() + 0x6000;
	if (m_cart->exists())
	{
//      m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, READ8_DELEGATE(gamate_state, gamate_cart_protection_r));
		m_cart_ptr = m_cart->get_rom_base();
		m_bankmulti->set_base(m_cart->get_rom_base()+1);
		m_bank->set_base(m_cart->get_rom_base()+0x4000); // bankswitched games in reality no offset
	}
//  m_bios[0xdf1]=0xea; m_bios[0xdf2]=0xea; // default bios: $47 protection readback
	card_protection.set=false;
	bank_multi=0;
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
	timer2->reset(m_maincpu->cycles_to_attotime(32768/2));
}


INTERRUPT_GEN_MEMBER(gamate_state::gamate_interrupt)
{
}

static MACHINE_CONFIG_START( gamate, gamate_state )
	MCFG_CPU_ADD("maincpu", M6502, 4433000/2)
	MCFG_CPU_PROGRAM_MAP(gamate_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gamate_state,  gamate_interrupt)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(160, 152)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 152-1)
	MCFG_SCREEN_UPDATE_DRIVER(gamate_state, screen_update_gamate)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gamate_colors))
	MCFG_PALETTE_INIT_OWNER(gamate_state, gamate)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMATE_SND, 4433000/2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "gamate_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list","gamate")
MACHINE_CONFIG_END


ROM_START(gamate)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "default", "DEFAULT")
	ROMX_LOAD("gamate_bios_umc.bin", 0xf000, 0x1000, CRC(07090415) SHA1(ea449dc607601f9a68d855ad6ab53800d2e99297), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "newer", "NEWER")
	ROMX_LOAD("gamate_bios_9130__unknown__bit_icasc00001_9130-bs_r32261.bin", 0xf000, 0x1000, CRC(03a5f3a7) SHA1(4e9dfbfe916ca485530ef4221593ab68738e2217), ROM_BIOS(2) )
ROM_END


/*    YEAR  NAME     PARENT  COMPAT    MACHINE  INPUT   CLASS         INIT      COMPANY    FULLNAME */
CONS( 19??, gamate,  0,      0,        gamate,  gamate, gamate_state, gamate, "Bit Corp", "Gamate", 0)
