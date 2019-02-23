// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
// comments and BIOS versions:rfka01
/*****************************************************************************************************
*
* Schneider Rundfunkwerke AG Euro PC and Euro PC II driver
*
* Manuals and BIOS files: ftp://ftp.cpcszene.de/pub/Computer/Schneider_PC/EuroPC_XT/
*
* Euro PC: Computer and floppy drive integrated into the keyboard, 8088, 512K RAM, there was an upgrade card for the ISA slot that took it to 640K, single ISA slot
           FD360 external 360K 5.25" DS DD floppy, FD720 external 720K 3,5" DS DD floppy, HD-20 external harddisk, internal graphics card is CGA or Hercules, 64KB VRAM
* Euro PC II: like Euro PC, socket for 8087, 768K RAM on board, driver on Schneider DOS disk allowed the portion over 640K to be used as extended memory or ramdisk.
* Euro XT: conventional desktop, specs like Euro PC II, two ISA slots on a riser card, 102 key seperate keyboard, internal XT-IDE 20MB harddisk, connector for FD360 and FD720 was retained
*
* Only BIOS versions >2.06 are supported so far because of changes in the memory management, according to https://www.forum64.de/index.php?thread/43066-schneider-euro-pc-i-ii-xt-welche-bios-version-habt-ihr/
* Versions 2.04 and 2.05 only show a single dash on the top left of the screen.
*
* To get rid of the BIOS error messages when you first start the system, enter the BIOS with Ctrl-Alt-Esc, match the RAM size to your settings in MAME, set the CPU speed to 9.54MHz
* and the graphics adapter to Color/Graphics 80, internal graphics off
*
* To-Do: * An external 20MB harddisk (Schneider HD20) can be added to the PC and PC II. This is a XT IDE drive. The BIOSs contain their own copy of the WD XT IDE BIOS that can be activated from the BIOS setup menu.
*          (load debug, then g=f000:a000 to enter formatter routine)
*        * emulate internal graphics, but AGA is not quite the correct choice for the standard graphics adapter (it's a Commodore standard), as the Schneiders are only capable of switching between Hercules and CGA modes.
*        * The PC 2 and XT have 768K of memory that can be configured from the BIOS setup as 640K, 640K+128K EMS and 512K+256K EMS. The EMS options are not visible in our emulation and loading the EMS driver fails.
*          See http://forum.classic-computing.de/index.php?page=Thread&threadID=8380 for screenshots.
*        * use correct AT style keyboard for XT
*        * make BIOS versions v2.04 and v2.05 work
*
*****************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "bus/isa/aga.h"
#include "bus/isa/fdc.h"
#include "machine/genpc.h"
#include "machine/nvram.h"
#include "machine/pckeybrd.h"
#include "machine/ram.h"

#include "coreutil.h"


class europc_pc_state : public driver_device
{
public:
	europc_pc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_keyboard(*this, "pc_keyboard"),
		m_ram(*this, RAM_TAG),
		m_jim_state(0),
		m_port61(0)
	{ }

	void europc(machine_config &config);
	void europc2(machine_config &config);
	void euroxt(machine_config &config);

	void init_europc();

private:
	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<pc_keyboard_device> m_keyboard;
	required_device<ram_device> m_ram;

	DECLARE_WRITE8_MEMBER( europc_pio_w );
	DECLARE_READ8_MEMBER( europc_pio_r );

	DECLARE_WRITE8_MEMBER ( europc_jim_w );
	DECLARE_READ8_MEMBER ( europc_jim_r );
	DECLARE_READ8_MEMBER ( europc_jim2_r );

	DECLARE_READ8_MEMBER( europc_rtc_r );
	DECLARE_WRITE8_MEMBER( europc_rtc_w );

	void europc_rtc_set_time();

	uint8_t m_jim_data[16];
	uint8_t m_jim_state;
	AGA_MODE m_jim_mode;
	int m_port61; // bit 0,1 must be 0 for startup; reset?
	uint8_t m_rtc_data[0x10];
	int m_rtc_reg;
	int m_rtc_state;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	emu_timer* m_rtc_timer;

	enum
	{
		TIMER_RTC
	};
	void europc_io(address_map &map);
	void europc_map(address_map &map);
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

void europc_pc_state::init_europc()
{
	uint8_t *rom = &memregion("bios")->base()[0];

	/*
	  fix century rom bios bug !
	  if year <79 month (and not CENTURY) is loaded with 0x20
	*/
	if (rom[0xf93e]==0xb6){ // mov dh,
		rom[0xf93e]=0xb5; // mov ch,
		uint8_t a = 0;
		int i = 0x8000;
		for (; i < 0xffff; i++)
			a += rom[i];
		rom[0xffff] = 256 - a;
	}

	memset(&m_rtc_data,0,sizeof(m_rtc_data));
	m_rtc_reg = 0;
	m_rtc_state = 0;
	m_rtc_data[0xf]=1;

	m_rtc_timer = timer_alloc();
	m_rtc_timer->adjust(attotime::zero, 0, attotime(1,0));
	//  europc_rtc_set_time();

	subdevice<nvram_device>("nvram")->set_base(m_rtc_data, sizeof(m_rtc_data));
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
		if (m_mb->pit_out2())
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

void europc_pc_state::europc_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void europc_pc_state::europc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0060, 0x0063).rw(FUNC(europc_pc_state::europc_pio_r), FUNC(europc_pc_state::europc_pio_w));
	map(0x0250, 0x025f).rw(FUNC(europc_pc_state::europc_jim_r), FUNC(europc_pc_state::europc_jim_w));
	map(0x02e0, 0x02e0).r(FUNC(europc_pc_state::europc_jim2_r));
}

class europc_fdc_device : public isa8_fdc_device
{
public:
	europc_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

DEFINE_DEVICE_TYPE(EUROPC_FDC, europc_fdc_device, "europc_fdc", "EURO PC FDC hookup")

europc_fdc_device::europc_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_fdc_device(mconfig, EUROPC_FDC, tag, owner, clock)
{
}

static void pc_dd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void europc_fdc_device::device_add_mconfig(machine_config &config)
{
	wd37c65c_device &fdc(WD37C65C(config, m_fdc, 16_MHz_XTAL));
	fdc.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	fdc.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	// single built-in 3.5" 720K drive, connector for optional external 3.5" or 5.25" drive
	FLOPPY_CONNECTOR(config, "fdc:0", pc_dd_floppies, "35dd", isa8_fdc_device::floppy_formats).set_fixed(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_dd_floppies, "", isa8_fdc_device::floppy_formats);
}

static void europc_fdc(device_slot_interface &device)
{
	device.option_add("fdc", EUROPC_FDC);
}

//Euro PC
void europc_pc_state::europc(machine_config &config)
{
	I8088(config, m_maincpu, 4772720*2);
	m_maincpu->set_addrmap(AS_PROGRAM, &europc_pc_state::europc_map);
	m_maincpu->set_addrmap(AS_IO, &europc_pc_state::europc_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	PCNOPPI_MOTHERBOARD(config, "mb", 0).set_cputag(m_maincpu);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "aga", false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "lpt", true);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "com", true);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", europc_fdc, "fdc", true);
	PC_KEYB(config, m_keyboard);
	m_keyboard->keypress().set("mb:pic8259", FUNC(pic8259_device::ir1_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);;

	/* internal ram */
	// Machine came with 512K standard, 640K via expansion card, but BIOS offers 256K as well
	RAM(config, m_ram).set_default_size("512K").set_extra_options("256K, 640K");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
}

//Euro PC II
void europc_pc_state::europc2(machine_config &config)
{
	europc(config);
	// could be configured by the BIOS as 640K, 640K+128K EMS or 512K+256K EMS
	m_ram->set_default_size("768K");
}

//Euro XT
MACHINE_CONFIG_START(europc_pc_state::euroxt)
	europc(config);

	m_ram->set_default_size("768K");

	MCFG_DEVICE_MODIFY("isa2")
	MCFG_SLOT_DEFAULT_OPTION(nullptr)
	MCFG_DEVICE_ADD("isa5", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "xtide", false) // FIXME: determine ISA bus clock
	MCFG_SLOT_FIXED(true)
	MCFG_DEVICE_ADD("isa6", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "lpt", false)
	MCFG_SLOT_FIXED(true)
MACHINE_CONFIG_END

ROM_START( europc )
	ROM_REGION(0x10000,"bios", 0)
	// hdd bios integrated!
	ROM_SYSTEM_BIOS( 0, "v2.06", "EuroPC v2.06" )
	ROMX_LOAD("bios_v2.06.bin", 0x8000, 0x8000, CRC(0a25a2eb) SHA1(d35f2f483d56b1eff558586e1d33d82f7efed639), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2.06b", "EuroPC v2.06b" )
	ROMX_LOAD("bios_v2.06b.bin", 0x8000, 0x8000, CRC(05d8a4c2) SHA1(52c6fd22fb739e29a1f0aa3c96ede79cdc659f72), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v2.07", "EuroPC v2.07" )
	ROMX_LOAD("50145", 0x8000, 0x8000, CRC(1775a11d) SHA1(54430d4d0462860860397487c9c109e6f70db8e3), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v2.08", "EuroPC v2.08" )
	ROMX_LOAD("bios_v2.08.bin", 0x8000, 0x8000, CRC(a7048349) SHA1(c2a0af7276c2ff6925abe5a5edef09c5a84106f2), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v2.08a", "EuroPC v2.08a" )
	ROMX_LOAD("bios_v2.08a.bin", 0x8000, 0x8000, CRC(872520b7) SHA1(9c94d33c0d454fab7bcd0c4516b50f1c3c6a30b8), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "v2.08b", "EuroPC v2.08b" )
	ROMX_LOAD("bios_v2.08b.bin", 0x8000, 0x8000, CRC(668c0d19) SHA1(69412e58e0ed1d141e633f094af91ec5f7ae064b), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "v2.04", "EuroPC v2.04" )
	ROMX_LOAD("bios_v2.04.bin", 0x8000, 0x8000, CRC(e623967c) SHA1(5196b14018da1f3198e2950af0e6eab41425f556), ROM_BIOS(6))
	ROM_SYSTEM_BIOS( 7, "v2.05", "EuroPC v2.05" )
	ROMX_LOAD("bios_2.05.bin", 0x8000, 0x8000, CRC(372ceed6) SHA1(bb3d3957a22422f98be2225bdc47705bcab96f56), ROM_BIOS(7)) // v2.04 and v2.05 don't work yet, , see comment section

	ROM_REGION(0x1000, "kbdctrl", 0)
	ROM_LOAD("zc86115p-mc6805u2.bin", 0x0000, 0x1000, CRC(d90c1fab) SHA1(ddb7060abddee7294723833c303090de35c1e79c))
ROM_END

ROM_START( europc2 )
	ROM_REGION(0x10000,"bios", 0)
	// hdd bios integrated!
	ROM_LOAD("europcii_bios_v3.01_500145.bin", 0x8000, 0x8000, CRC(ecca89c8) SHA1(802b89babdf0ab0a0a9c21d1234e529c8386d6fb))

	ROM_REGION(0x1000, "kbdctrl", 0)
	ROM_LOAD("zc86115p-mc6805u2.bin", 0x0000, 0x1000, CRC(d90c1fab) SHA1(ddb7060abddee7294723833c303090de35c1e79c))
ROM_END

ROM_START( euroxt )
	ROM_REGION(0x10000,"bios", 0)
	// hdd bios integrated!
	ROM_LOAD("euroxt_bios_v1.01.bin", 0x8000, 0x8000, CRC(1e1fe931) SHA1(bb7cae224d66ae48045f323ecb9ad59bf49ed0a2))
ROM_END

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS            INIT         COMPANY              FULLNAME      FLAGS
COMP( 1988, europc,  ibm5150, 0,      europc,  europc, europc_pc_state, init_europc, "Schneider Rdf. AG", "EURO PC",    MACHINE_NOT_WORKING)
COMP( 198?, europc2, ibm5150, 0,      europc2, europc, europc_pc_state, init_europc, "Schneider Rdf. AG", "EURO PC II", MACHINE_NOT_WORKING)
COMP( 198?, euroxt,  ibm5150, 0,      euroxt,  europc, europc_pc_state, init_europc, "Schneider Rdf. AG", "EURO XT",    MACHINE_NOT_WORKING)
