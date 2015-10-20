// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "coreutil.h"
#include "includes/genpc.h"
#include "machine/nvram.h"
#include "machine/pckeybrd.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"

class europc_pc_state : public driver_device
{
public:
	europc_pc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_keyboard(*this, "pc_keyboard"),
		m_jim_state(0),
		m_port61(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<pc_keyboard_device> m_keyboard;
	isa8_aga_device *m_aga;

	DECLARE_WRITE8_MEMBER( europc_pio_w );
	DECLARE_READ8_MEMBER( europc_pio_r );

	DECLARE_WRITE8_MEMBER ( europc_jim_w );
	DECLARE_READ8_MEMBER ( europc_jim_r );
	DECLARE_READ8_MEMBER ( europc_jim2_r );

	DECLARE_READ8_MEMBER( europc_rtc_r );
	DECLARE_WRITE8_MEMBER( europc_rtc_w );

	DECLARE_DRIVER_INIT(europc);

	void europc_rtc_set_time();

	UINT8 m_jim_data[16];
	UINT8 m_jim_state;
	AGA_MODE m_jim_mode;
	int m_port61; // bit 0,1 must be 0 for startup; reset?
	UINT8 m_rtc_data[0x10];
	int m_rtc_reg;
	int m_rtc_state;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	emu_timer* m_rtc_timer;

	enum
	{
		TIMER_RTC
	};
};

/*
  europc
  fe107 bios checksum test
   memory test
  fe145
   irq vector init
  fe156
  fe169 fd774 // test of special europc registers 254 354
  fe16c fe817
  fe16f
   fec08 // test of special europc registers 800a rtc time or date error, rtc corrected
    fef66 0xf
    fdb3e 0x8..0xc
    fd7f8
     fdb5f
  fe172
   fecc5 // 801a video setup error
    fd6c9
   copyright output
  fe1b7
  fe1be di bits set mean output text!!!,
   (801a)
   0x8000 output
        1 rtc error
        2 rtc time or date error
        4 checksum error in setup
        8 rtc status corrected
       10 video setup error
       20 video ram bad
       40 monitor type not recogniced
       80 mouse port enabled
      100 joystick port enabled

  fe1e2 fdc0c cpu speed is 4.77 MHz
  fe1e5 ff9c0 keyboard processor error
  fe1eb fc617 external lpt1 at 0x3bc
  fe1ee fe8ee external coms at

  routines:
  fc92d output text at bp
  fdb3e rtc read reg cl
  fe8ee piep
  fe95e rtc write reg cl
   polls until jim 0xa is zero,
   output cl at jim 0xa
   write ah hinibble as lownibble into jim 0xa
   write ah lownibble into jim 0xa
  fef66 rtc read reg cl
   polls until jim 0xa is zero,
   output cl at jim 0xa
   read low 4 nibble at jim 0xa
   read low 4 nibble at jim 0xa
   return first nibble<<4|second nibble in ah
  ff046 seldom compares ret
  ffe87 0 -> ds

  469:
   bit 0: b0000 memory available
   bit 1: b8000 memory available
  46a: 00 jim 250 01 jim 350
 */


/*
  250..253 write only 00 be 00 10

  252 write 0 b0000 memory activ
  252 write 0x10 b8000 memory activ

  jim 04: 0:4.77 0x40:7.16
  pio 63: 11,19 4.77 51,59 7.16

  63 bit 6,7 clock select
  254 bit 6,7 clock select
  250 bit 0: mouse on
      bit 1: joystick on
  254..257 r/w memory ? JIM asic? ram behaviour

*/

WRITE8_MEMBER( europc_pc_state::europc_jim_w )
{
	switch (offset)
	{
	case 2:
		if (!(data & 0x80))
		{
			switch (data)
			{
			case 0x1f:
			case 0x0b: m_jim_mode = AGA_MONO; break;
			case 0xe: //80 columns?
			case 0xd: //40 columns?
			case 0x18:
			case 0x1a: m_jim_mode = AGA_COLOR; break;
			default: m_jim_mode = AGA_OFF; break;
			}
		}
//      mode= data&0x10?AGA_COLOR:AGA_MONO;
//      mode= data&0x10?AGA_COLOR:AGA_OFF;
		if (data & 0x80) m_jim_state = 0;
		break;
	case 4:
		switch(data & 0xc0)
		{
		case 0x00: m_maincpu->set_clock_scale(1.0 / 2); break;
		case 0x40: m_maincpu->set_clock_scale(3.0 / 4); break;
		default: m_maincpu->set_clock_scale(1); break;
		}
		break;
	case 0xa:
		europc_rtc_w(space, 0, data);
		return;
	}
	logerror("jim write %.2x %.2x\n", offset, data);
	m_jim_data[offset] = data;
}

READ8_MEMBER( europc_pc_state::europc_jim_r )
{
	int data = 0;
	switch(offset)
	{
	case 4: case 5: case 6: case 7: data = m_jim_data[offset]; break;
	case 0: case 1: case 2: case 3: data = 0; break;
	case 0xa: return europc_rtc_r(space, 0);
	}
	return data;
}

READ8_MEMBER( europc_pc_state::europc_jim2_r )
{
	switch (m_jim_state)
	{
	case 0: m_jim_state++; return 0;
	case 1: m_jim_state++; return 0x80;
	case 2:
		m_jim_state = 0;
		switch (m_jim_mode)
		{
		case AGA_COLOR: return 0x87; // for color;
		case AGA_MONO: return 0x90; //for mono
		case AGA_OFF: return 0x80; // for vram
//      return 0x97; //for error
		}
	}
	return 0;
}

/* realtime clock and nvram  EM M3002

   reg 0: seconds
   reg 1: minutes
   reg 2: hours
   reg 3: day 1 based
   reg 4: month 1 based
   reg 5: year bcd (no century, values bigger 88? are handled as 1900, else 2000)
   reg 6:
   reg 7:
   reg 8:
   reg 9:
   reg a:
   reg b: 0x10 written
    bit 0,1: 0 video startup mode: 0=specialadapter, 1=color40, 2=color80, 3=monochrom
    bit 2: internal video on
    bit 4: color
    bit 6,7: clock
   reg c:
    bit 0,1: language/country
   reg d: xor checksum
   reg e:
   reg 0f: 01 status ok, when not 01 written
*/

void europc_pc_state::europc_rtc_set_time()
{
	system_time systime;

	/* get the current date/time from the core */
	machine().current_datetime(systime);

	m_rtc_data[0] = dec_2_bcd(systime.utc_time.second);
	m_rtc_data[1] = dec_2_bcd(systime.utc_time.minute);
	m_rtc_data[2] = dec_2_bcd(systime.utc_time.hour);

	m_rtc_data[3] = dec_2_bcd(systime.utc_time.mday);
	m_rtc_data[4] = dec_2_bcd(systime.utc_time.month + 1);
	m_rtc_data[5] = dec_2_bcd(systime.utc_time.year % 100);
}

void europc_pc_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int month, year;

	switch(id)
	{
		case TIMER_RTC:
			m_rtc_data[0]=bcd_adjust(m_rtc_data[0]+1);
			if (m_rtc_data[0]>=0x60)
			{
				m_rtc_data[0]=0;
				m_rtc_data[1]=bcd_adjust(m_rtc_data[1]+1);
				if (m_rtc_data[1]>=0x60)
				{
					m_rtc_data[1]=0;
					m_rtc_data[2]=bcd_adjust(m_rtc_data[2]+1);
					if (m_rtc_data[2]>=0x24)
					{
						m_rtc_data[2]=0;
						m_rtc_data[3]=bcd_adjust(m_rtc_data[3]+1);
						month=bcd_2_dec(m_rtc_data[4]);
						year=bcd_2_dec(m_rtc_data[5])+2000; // save for julian_days_in_month_calculation
						if (m_rtc_data[3]> gregorian_days_in_month(month, year))
						{
							m_rtc_data[3]=1;
							m_rtc_data[4]=bcd_adjust(m_rtc_data[4]+1);
							if (m_rtc_data[4]>0x12)
							{
								m_rtc_data[4]=1;
								m_rtc_data[5]=bcd_adjust(m_rtc_data[5]+1)&0xff;
							}
						}
					}
				}
			}
			break;
	}
}

READ8_MEMBER( europc_pc_state::europc_rtc_r )
{
	int data=0;
	switch (m_rtc_state)
	{
	case 1:
		data=(m_rtc_data[m_rtc_reg]&0xf0)>>4;
		m_rtc_state++;
		break;
	case 2:
		data=m_rtc_data[m_rtc_reg]&0xf;
		m_rtc_state=0;
//      logerror("rtc read %x %.2x\n",m_rtc_reg, m_rtc_data[m_rtc_reg]);
		break;
	}
	return data;
}

WRITE8_MEMBER( europc_pc_state::europc_rtc_w )
{
	switch (m_rtc_state)
	{
	case 0:
		m_rtc_reg=data;
		m_rtc_state=1;
		break;
	case 1:
		m_rtc_data[m_rtc_reg]=(m_rtc_data[m_rtc_reg]&~0xf0)|((data&0xf)<<4);
		m_rtc_state++;
		break;
	case 2:
		m_rtc_data[m_rtc_reg]=(m_rtc_data[m_rtc_reg]&~0xf)|(data&0xf);
		m_rtc_state=0;
//      logerror("rtc written %x %.2x\n",m_rtc_reg, m_rtc_data[m_rtc_reg]);
		break;
	}
}

DRIVER_INIT_MEMBER(europc_pc_state,europc)
{
	UINT8 *rom = &memregion("maincpu")->base()[0];

	int i;
	/*
	  fix century rom bios bug !
	  if year <79 month (and not CENTURY) is loaded with 0x20
	*/
	if (rom[0xff93e]==0xb6){ // mov dh,
		UINT8 a;
		rom[0xff93e]=0xb5; // mov ch,
		for (i=0xf8000, a=0; i<0xfffff; i++ ) a+=rom[i];
		rom[0xfffff]=256-a;
	}

	memset(&m_rtc_data,0,sizeof(m_rtc_data));
	m_rtc_reg = 0;
	m_rtc_state = 0;
	m_rtc_data[0xf]=1;

	m_rtc_timer = timer_alloc();
	m_rtc_timer->adjust(attotime::zero, 0, attotime(1,0));
	//  europc_rtc_set_time();

	machine().device<nvram_device>("nvram")->set_base(m_rtc_data, sizeof(m_rtc_data));
	m_aga = machine().device<isa8_aga_device>("aga:aga");

}

WRITE8_MEMBER( europc_pc_state::europc_pio_w )
{
	switch (offset)
	{
	case 1:
		m_port61=data;
		m_mb->m_pit8253->write_gate2(BIT(data, 0));
		m_mb->pc_speaker_set_spkrdata(BIT(data, 1));
		m_keyboard->enable(BIT(data, 6));
		if(data & 0x80)
			m_mb->m_pic8259->ir1_w(0);
		break;
	}

	logerror("europc pio write %.2x %.2x\n", offset, data);
}


READ8_MEMBER( europc_pc_state::europc_pio_r )
{
	int data = 0;
	switch (offset)
	{
	case 0:
		data = m_keyboard->read(space, 0);
		break;
	case 1:
		data = m_port61;
		break;
	case 2:
		if (m_mb->m_pit_out2)
			data |= 0x20;
		break;
	}
	return data;
}

/*
layout of an uk europc

ESC, [SPACE], F1,F2,F3,F4,[SPACE],F5,F6,F7,F8,[SPACE],F9,F0,F11,F12
[SPACE]
\|, 1,2,3,4,5,6,7,8,9,0 -,+, BACKSPACE,[SPACE], NUM LOCK, SCROLL LOCK, PRINT SCREEN, KEYPAD -
TAB,Q,W,E,R,T,Y,U,I,O,P,[,], RETURN, [SPACE], KEYPAD 7, KEYPAD 8, KEYPAD 9, KEYPAD +
CTRL, A,S,D,F,G,H,J,K,L,;,@,~, RETURN, [SPACE],KEYPAD 4,KEYPAD 5,KEYPAD 6, KEYPAD +
LEFT SHIFT, Z,X,C,V,B,N,M,<,>,?,RIGHT SHIFT,[SPACE],KEYPAD 1, KEYPAD 2, KEYPAD 3, KEYPAD ENTER
ALT,[SPACE], SPACE BAR,[SPACE],CAPS LOCK,[SPACE], KEYPAD 0, KEYPAD ., KEYPAD ENTER

\ and ~ had to be swapped
i am not sure if keypad enter delivers the mf2 keycode
 */

static INPUT_PORTS_START( europc )
	PORT_START("DSW0") /* IN1 */

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE(pc_keyboard)
	PORT_MODIFY("pc_keyboard_2") /* IN6 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_BACKSLASH) /* `                           29  A9 */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_TILDE) /* \                           2B  AB */

	PORT_MODIFY("pc_keyboard_5") /* IN9 */
	PORT_BIT ( 0x0070, 0x0000, IPT_UNUSED )
	/* 0x40 non us backslash 2 not available */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11)      /* F11                         57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12)      /* F12                         58  D8 */

	PORT_START("pc_keyboard_6") /* IN10 */\
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP Enter") PORT_CODE(KEYCODE_ENTER_PAD)       /* PAD Enter                   60  e0 */
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static ADDRESS_MAP_START( europc_map, AS_PROGRAM, 8, europc_pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(europc_io, AS_IO, 8, europc_pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(europc_pio_r, europc_pio_w)
	AM_RANGE(0x0250, 0x025f) AM_READWRITE(europc_jim_r, europc_jim_w)
	AM_RANGE(0x02e0, 0x02e0) AM_READ(europc_jim2_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( europc, europc_pc_state )
	MCFG_CPU_ADD("maincpu", I8088, 4772720*2)
	MCFG_CPU_PROGRAM_MAP(europc_map)
	MCFG_CPU_IO_MAP(europc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb", "maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "aga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "fdc_xt", false)

	MCFG_PC_KEYB_ADD("pc_keyboard", DEVWRITELINE("mb:pic8259", pic8259_device, ir1_w))

	MCFG_NVRAM_ADD_0FILL("nvram");

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

ROM_START( europc )
	ROM_REGION(0x100000,"maincpu", 0)
	// hdd bios integrated!
	ROM_LOAD("50145", 0xf8000, 0x8000, CRC(1775a11d) SHA1(54430d4d0462860860397487c9c109e6f70db8e3)) // V2.07
ROM_END

COMP( 1988, europc,     ibm5150,    0,          europc,     europc, europc_pc_state,     europc,     "Schneider Rdf. AG", "EURO PC", MACHINE_NOT_WORKING)
