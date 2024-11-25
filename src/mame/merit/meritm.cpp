// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
  Merit CRT-250 and CRT-260 hardware

  Driver by Mariusz Wojcieszek

CRT-250
+-----------------------------------------------+
|                                 JP6      JP7  |
| 81464 81464 81464 81464     DS1225Y U9   U13  |
| 81464 81464 81464 81464 Z80    PAL  U10  U14  |
| 81464 81464 81464 81464  PAL        U11  U15  |
|      21.47727MHz                    U12  U16  |
|                                               |
|   V9938 V9938                AY8930           |
|                       |-J2-|                  |
|                              Z80APIO          |
|                                               |
|                P8255A        Z80APIO          |
|                                               |
|                                               |
| |----------------J3-----------------| DSW VOL |
+-----------------------------------------------+

  CPU: Z80B
Video: Yamaha V9938 Video Processor x 2 (MSX2 video chip)
Sound: AY8930 (or compatible)
  OSC: 21.47727MHz
Other: Z80APIO x 2  (I/O and interrupt controllers)
       P8255A (I/O)
       8-switch DSW (socketed)
       Volume pot

Memory: Fujitsu MB81464-12 (or compatible) all socketed. The 3rd row of memory
        may or may not be populated.

JP6 - 3 pin jumper: ROM size (1-2 = 27256, 2-3 = 27512)
JP7 - 3 pin jumper: RAM / ROM for U13 to U16? (1-2 = RAM, 2-3 = ROM)
J2 - 15 pin connector for the optional CRT-254
J3 - 65 Pin connector:

 #1 - Blue  (video out)    #23 - V-Meter Lamp              #45 - P2 Discard 3 Switch
 #2 - Green (video out)    #24 - P2 Play Lamp              #46 - P2 Discard 2 Switch
 #3 - Red   (video out)    #25 - Audit Meter #2 12v        #47 - P2 Discard 1 Switch
 #4 - Vertical Sync        #26 - Audit Meter #1 12v        #48 - Adjust 3 Terminal
 #5 - Horizontal Sync      #27 - V-Lamp return             #49 - Adjust 2 Terminal
 #6 - NC                   #28 - Lockout Coil Option 12v   #50 - Adjust 1 Terminal
 #7 - Video Ground         #29 - P2 Cancel Lamp            #51 - Switch Common Ground
 #8 - Ground               #30 - Undefined Lamp            #52 - Switch Common Ground
 #9 - Logic (PCB) Ground   #31 - P1 Cancel Lamp            #53 - P2 Cancel Switch
#10 - Logic (PCB) Ground   #32 - P1 Play Lamp              #54 - Diagnostics Switch
#11 - +5VDC Power          #33 - P1 Discard 5 Lamp         #55 - Books Switch
#12 - +5VDC Power          #34 - P1 Discard 4 Lamp         #56 - Coin 2 Switch
#13 - +12VDC Power         #35 - P1 Discard 3 Lamp         #57 - Coin 1 Switch
#14 - V-Lamp               #36 - P1 Discard 2 Lamp         #58 - Undefined Switch
#15 - V-Lamp Return        #37 - P1 Discard 1 Lamp         #59 - P1 Cancel Switch
#16 - NC                   #38 - NC                        #60 - P1 Play Switch
#17 - P2 Discard 1 Lamp    #39 - Speaker -                 #61 - P1 Discard 5 Switch
#18 - P2 Discard 2 Lamp    #40 - Speaker +                 #62 - P1 Discard 4 Switch
#19 - P2 Discard 3 Lamp    #41 - NC                        #63 - P1 Discard 3 Switch
#20 - P2 Discard 4 Lamp    #42 - P2 Play Switch            #64 - P1 Discard 2 Switch
#21 - P2 Discard 5 Lamp    #43 - P2 Discard 5 Switch       #65 - P1 Discard 1 Switch
#22 - V-Meter Lamp         #44 - P2 Discard 4 Switch

Power & Common Ground wires are 18 gauge, all other wires are 20 or 22 gauge.

  Optional addon boards for the CRT-250:
  - CRT-254: Connects to a Dallas DS1204 electronic key
  - CRT-256: Stores question roms (aka Memory Expansion board)
  - CRT-258: Microtouch touch screen controller:
              Connects through the Z80 socket & contains a PC16550DN UART,
              relocated Z80, 1.8432MHz OSC, PAL labeled SC3980 & RS232C port

  CRT-260 same basic components as CRT-250 with these additional components:
  - Microtouch touch screen controller (SMT-3)
  - PC16550 UART (for communication with touch screen controller)
  - DS1204 Electronic Key (for protection)
  - DS1232 Reset and Watchdog
  - MAX232 (for MegaLink)

    One of the following Dallas Non-volatile SRAM chips:
    - DS1225Y 64K Non-volatile SRAM (Mega Touch 4)
    - DS1230Y 256K Non-volatile SRAM (Mega Touch 6)
    - DS1644 32K NVRAM + RTC (Tournament sets)

  Known Games:

  CRT-250:
  Americana (c) 1987
  Dodge City (c) 1988
  Pit Boss II (c)1988
  Super Pit Boss (c)1988
  Pit Boss Superstar (c)1990
  *Pit Boss Superstar 30 (c)1993
  Pit Boss Superstar III 30 (c)1993
  Pit Boss Megastar (c)1994
  Pit Boss Supertouch 30 (c)1993/4
  Pit Boss Megatouch (c)1994

Custom Program Versions (Superstar 30 / Supertouch 30):

PROGRAM#    Program Version      Program Differences
---------------------------------------------------------------------------------------------
923x-00-01  Standard Version     Includes all Options, no Restrictions
923x-00-02  Minnesota Version    Excludes Casino Games
923x-00-03  Louisiana Version    Excludes all Poker Games
923x-00-04  Wisconsin Version    Game Cannot End if Player Busts; 1,000 Points are Added to End of Each Hand
923x-00-05  Montana Version      Excludes Blackjack, Dice and Photo Finish
923x-00-06  California Version   Excludes Poker Double-up feature
9234-00-07  New Jersey Version   Excludes Sex Trivia and includes 2-Coin Limit with Lockout Coil (Only for Supertouch 30 & Megastar)


  CRT-260:
  Megatouch II (c)1994
  Megatouch III (c)1995
  Megatouch III Tournament Edition (c)1996
  Megatouch IV (c)1996
  Megatouch IV Tournament Edition (c)1996
  Super Megatouch IV (c)1996  (rom labels 9255-41-0x, see below)
  Super Megatouch IV Tournament Edition (c)1996
  The Real Broadway (c)1995
  Megatouch 5 (c)1997
  Megatouch 5 Tournament Edition (c)1997
  Megatouch 6 (c)1998
  Megatouch 7 Encore Edition (c)2000


Custom Program Versions (from different Megatouch manuals):

PROGRAM#    Program Version      Program Differences
---------------------------------------------------------------------------------------------
9255-xx-01  Standard Version       Includes all Options, no Restrictions
9255-xx-02  Minnesota Version      Excludes Casino Games
9255-xx-03  Louisiana Version      Excludes all Poker Games
9255-xx-04  Wisconsin Version      Game Cannot End if Player Busts; 1,000 Points are Added to End of Each Hand
9255-xx-06  California Version     Excludes Poker Double-up feature & No Free Game in Solitaire
9255-xx-07  New Jersey Version     Includes 2-Coin Limit with Lockout Coil
9255-xx-50  Bi-Lingual ENG/GER     Same as Standard Version, Without Word/Casino Games
9255-xx-54  Bi-Lingual ENG/SPA     Same as Standard Version, Without Word Games
9255-xx-56  No Free Credits        Same as Standard Version, Without Word Games and No Free Credits
9255-xx-57  International Version  Same as Standard Version, Without Word Games
9255-xx-60  Bi-Lingual ENG/FRE     Same as Standard Version, Without Word/Casino Games
9255-xx-62  No Free Credit         Same as Standard Version, With No Free Credit (see regional notes below)
9255-xx-62  Croatia                Same as Standard Version, With No Free Credit (see regional notes below)
9255-xx-70  Australia Version      Same as Standard Version with Special Question Set
9255-xx-71  South Africa Ver.      Same as Standard Version with Special Question Set

xx = game/version code:

 20 - Megatouch 3
 30 - Megatouch 3 Tournament
 40 - Megatouch 4
 41 - Megatouch Super 4
 50 - Megatouch 4 Tournament
 51 - Megatouch Super 4 Tournament
 60 - Megatouch 5
 70 - Megatouch 5 Tournament
 80 - Megatouch 6
 90 - Megatouch 7 Encore

Not all regional versions are available for each Megatouch series
 For Megatouch 4,       set 9255-40-62 is Croatia
 For Megatouch Super 4, set 9255-41-62 is No Free Credit

  Notes/ToDo:
  - offset for top V9938 layer is hard coded, probably should be taken from V9938 setup
  - blinking on Megatouch title screen is probably incorrect
  - clean up V9938 interrupt implementation
  - finish inputs, dsw, outputs (lamps)
  - problem with registering touches on the bottom of the screen (currently hacked to work)
*/

#include "emu.h"

#include "microtouchlayout.h"

#include "cpu/z80/z80.h"
#include "machine/ds1204.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/microtch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "sound/ay8910.h"
#include "video/v9938.h"

#include "speaker.h"


namespace {

class meritm_state : public driver_device
{
public:
	meritm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_z80pio(*this, "z80pio%u", 0U)
		, m_ds1204(*this, "ds1204")
		, m_ppi(*this, "ppi8255")
		, m_v9938(*this, "v9938_%u", 0U)
		, m_microtouch(*this, "microtouch")
		, m_uart(*this, "ns16550")
		, m_maincpu(*this, "maincpu")
		, m_banks(*this, "bank%u", 0U)
		, m_region_maincpu(*this, "maincpu")
		, m_region_extra(*this, "extra")
		, m_p1_disc_lamp(*this, "P1 DISC %u LAMP", 1U)
		, m_p1_play_lamp(*this, "P1 PLAY LAMP")
		, m_p1_cancel_lamp(*this, "P1 CANCEL LAMP")
	{ }

	void init_megat3te();

	void crt260(machine_config &config);
	void crt250(machine_config &config);
	void crt250_questions(machine_config &config);
	void crt250_crt252_crt258(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device_array<z80pio_device, 2> m_z80pio;
	required_device<ds1204_device> m_ds1204;
	required_device<i8255_device> m_ppi;
	required_device_array<v9938_device, 2> m_v9938;
	optional_device<microtouch_device> m_microtouch;
	optional_device<ns16550_device> m_uart;
	required_device<z80_device> m_maincpu;
	optional_memory_bank_array<3> m_banks;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_extra;
	std::unique_ptr<uint8_t[]> m_ram;
	output_finder<5> m_p1_disc_lamp;
	output_finder<> m_p1_play_lamp;
	output_finder<> m_p1_cancel_lamp;

	int m_vint;
	int m_interrupt_vdp0_state;
	int m_interrupt_vdp1_state;
	int m_bank;
	int m_psd_a15;
	uint16_t m_questions_loword_address;

	void crt250_bank_w(uint8_t data);
	void psd_a15_w(uint8_t data);
	void bank_w(uint8_t data);
	void crt250_questions_lo_w(uint8_t data);
	void crt250_questions_hi_w(uint8_t data);
	void crt250_questions_bank_w(uint8_t data);
	void ds1644_w(offs_t offset, uint8_t data);
	uint8_t ds1644_r(offs_t offset);
	uint8_t _8255_port_c_r();
	void crt250_port_b_w(uint8_t data);
	void ay8930_port_b_w(uint8_t data);
	uint8_t audio_pio_port_a_r();
	uint8_t audio_pio_port_b_r();
	void audio_pio_port_a_w(uint8_t data);
	void audio_pio_port_b_w(uint8_t data);
	void io_pio_port_a_w(uint8_t data);
	void io_pio_port_b_w(uint8_t data);

	DECLARE_MACHINE_START(crt250_questions);
	DECLARE_MACHINE_START(crt250_crt252_crt258);
	DECLARE_MACHINE_START(crt260);
	DECLARE_MACHINE_START(common);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(vblank_start_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(vblank_end_tick);
	void crt250_switch_banks();
	void switch_banks();
	int touch_coord_transform(int *touch_x, int *touch_y);
	uint8_t binary_to_BCD(uint8_t data);
	[[maybe_unused]] void vdp0_interrupt(int state);
	[[maybe_unused]] void vdp1_interrupt(int state);
	void crt250_crt258_io_map(address_map &map) ATTR_COLD;
	void crt250_io_map(address_map &map) ATTR_COLD;
	void crt250_map(address_map &map) ATTR_COLD;
	void crt250_questions_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
};


#define SYSTEM_CLK  XTAL(21'477'272)
#define UART_CLK    XTAL(1'843'200) // standard 8250 clock


/*************************************
 *
 *  Microtouch touch coordinate transformation
 *
 *************************************/

int meritm_state::touch_coord_transform(int *touch_x, int *touch_y)
{
	int xscr = int(double(*touch_x)/0x4000*544);
	int yscr = int(double(*touch_y)/0x4000*480);

	if( (xscr < 16) ||
		(xscr > 544-16) ||
		(yscr < 16) ||
		(yscr > 480-16))
	{
		return 0;
	}
	if ( yscr > 480-63 )
	{
		*touch_y = 0x3fff;
	}
	else
	{
		*touch_y = int(double(yscr - 16)*0x4000/(480-16-63));
	}
	*touch_x = int(double(xscr - 16)*0x4000/(544-16-16));

	return 1;
}

/*************************************
 *
 *  Video
 *
 *************************************/

void meritm_state::vdp0_interrupt(int state)
{
	if (state != m_interrupt_vdp0_state)
	{
		m_vint = (m_vint & 8) | (state ? 0 : 0x10);
		m_interrupt_vdp0_state = state;
		m_z80pio[0]->port_a_write(m_vint);
	}
}

void meritm_state::vdp1_interrupt(int state)
{
	if (state != m_interrupt_vdp1_state)
	{
		m_vint = (m_vint & 0x10) | (state ? 0 : 8);
		m_interrupt_vdp1_state = state;
		m_z80pio[0]->port_a_write(m_vint);
	}
}


void meritm_state::video_start()
{
	m_vint = 0x18;
	m_interrupt_vdp0_state = 0;
	m_interrupt_vdp1_state = 0;

	save_item(NAME(m_vint));
	save_item(NAME(m_interrupt_vdp0_state));
	save_item(NAME(m_interrupt_vdp1_state));
}

uint32_t meritm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	copybitmap(bitmap, m_v9938[0]->get_bitmap(), 0, 0, 0, 0, cliprect);
	copybitmap_transalpha(bitmap, m_v9938[1]->get_bitmap(), 0, 0, -6, -12, cliprect);

	return 0;
}

/*************************************
 *
 *  Bank switching (ROM/RAM)
 *
 *************************************/

void meritm_state::crt250_switch_banks(  )
{
	int rombank = (m_bank & 0x07) ^ 0x07;

	//logerror( "CRT250: Switching banks: rom = %0x (bank = %x)\n", rombank, m_bank );
	m_banks[0]->set_entry(rombank );
}

void meritm_state::crt250_bank_w(uint8_t data)
{
	crt250_switch_banks();
}

void meritm_state::switch_banks(  )
{
	int rambank = (m_psd_a15 >> 2) & 0x3;
	int rombank = (((m_bank >> 3) & 0x3) << 5) |
				(((m_psd_a15 >> 1) & 0x1) << 4) |
				(((m_bank & 0x07) ^ 0x07) << 1) |
				(m_psd_a15 & 0x1);

	//logerror( "Switching banks: rom = %0x (bank = %x), ram = %0x\n", rombank, m_bank, rambank);
	m_banks[0]->set_entry(rombank );
	m_banks[1]->set_entry(rombank | 0x01);
	m_banks[2]->set_entry(rambank);
}

void meritm_state::psd_a15_w(uint8_t data)
{
	m_psd_a15 = data;
	//logerror( "Writing PSD_A15 with %02x at PC=%04X\n", data, m_maincpu->pc() );
	switch_banks();
}

void meritm_state::bank_w(uint8_t data)
{
	switch_banks();
}

/*************************************
 *
 *  CRT250 question roms reading
 *
 *************************************/

void meritm_state::crt250_questions_lo_w(uint8_t data)
{
	m_questions_loword_address &= 0xff00;
	m_questions_loword_address |= data;
}

void meritm_state::crt250_questions_hi_w(uint8_t data)
{
	m_questions_loword_address &= 0x00ff;
	m_questions_loword_address |= (data << 8);
}

void meritm_state::crt250_questions_bank_w(uint8_t data)
{
	uint32_t questions_address;

	if ((m_bank & 0x07) != 0)
	{
		logerror("crt250_questions_bank_w: bank is %d\n", m_bank);
		return;
	}

	uint8_t *dst = memregion("maincpu")->base() + 0x70000 + 2;

	if (data == 0)
	{
		*dst = 0xff;
	}
	else if (data == 0xff)
	{
		// ignore
	}
	else
	{
		switch(data)
		{
			case 0x6c: questions_address = 0x00000; break;
			case 0x6d: questions_address = 0x10000; break;
			case 0x6e: questions_address = 0x20000; break;
			case 0x6f: questions_address = 0x30000; break;
			case 0x5c: questions_address = 0x40000; break;
			case 0x5d: questions_address = 0x50000; break;
			case 0x5e: questions_address = 0x60000; break;
			case 0x5f: questions_address = 0x70000; break;
			case 0x3c: questions_address = 0x80000; break;
			case 0x3d: questions_address = 0x90000; break;
			case 0x3e: questions_address = 0xa0000; break;
			case 0x3f: questions_address = 0xb0000; break;
			default: logerror( "crt250_questions_bank_w: unknown data = %02x\n", data ); return;
		}
		logerror( "Reading question byte at %06X\n", questions_address | m_questions_loword_address);
		*dst = m_region_extra->base()[questions_address | m_questions_loword_address];
	}
}


/*************************************
 *
 *  DS1644 RTC
 *
 *************************************/

void meritm_state::ds1644_w(offs_t offset, uint8_t data)
{
	int rambank = (m_psd_a15 >> 2) & 0x3;
	if (rambank < 3)
	{
		m_ram[rambank*0x2000 + 0x1ff8 + offset] = data;
	}
	else
	{
		if (offset == 0)
		{
			m_ram[0x7ff8] = data;
		}
		//logerror( "Writing RTC, reg = %d, data = %x\n", offset, data);
	}
}

uint8_t meritm_state::binary_to_BCD(uint8_t data)
{
	data %= 100;

	return ((data / 10) << 4) | (data %10);
}

uint8_t meritm_state::ds1644_r(offs_t offset)
{
	system_time systime;
	int rambank = (m_psd_a15 >> 2) & 0x3;
	if (rambank == 3)
	{
		//logerror( "Reading RTC, reg = %x\n", offset);

		machine().current_datetime(systime);
		m_ram[0x7ff9] = binary_to_BCD(systime.local_time.second);
		m_ram[0x7ffa] = binary_to_BCD(systime.local_time.minute);
		m_ram[0x7ffb] = binary_to_BCD(systime.local_time.hour);
		m_ram[0x7ffc] = binary_to_BCD(systime.local_time.weekday+1);
		m_ram[0x7ffd] = binary_to_BCD(systime.local_time.mday);
		m_ram[0x7ffe] = binary_to_BCD(systime.local_time.month+1);
		m_ram[0x7fff] = binary_to_BCD(systime.local_time.year % 100);
	}
	return m_ram[rambank*0x2000 + 0x1ff8 + offset];
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

void meritm_state::crt250_map(address_map &map)
{
	map(0x0000, 0xdfff).bankr(m_banks[0]);
	map(0xe000, 0xffff).ram().share("nvram");
}

void meritm_state::crt250_questions_map(address_map &map)
{
	map(0x0000, 0xdfff).bankr(m_banks[0]);
	map(0x0000, 0x0000).w(FUNC(meritm_state::crt250_questions_lo_w));
	map(0x0001, 0x0001).w(FUNC(meritm_state::crt250_questions_hi_w));
	map(0x0002, 0x0002).w(FUNC(meritm_state::crt250_questions_bank_w));
	map(0xe000, 0xffff).ram().share("nvram");
}

void meritm_state::crt250_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x13).rw(m_v9938[0], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x20, 0x23).rw(m_v9938[1], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x30, 0x33).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw(m_z80pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw(m_z80pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x80, 0x80).r("aysnd", FUNC(ay8930_device::data_r));
	map(0x80, 0x81).w("aysnd", FUNC(ay8930_device::address_data_w));
	map(0xff, 0xff).w(FUNC(meritm_state::crt250_bank_w));
}

void meritm_state::crt250_crt258_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x13).rw(m_v9938[0], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x20, 0x23).rw(m_v9938[1], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x30, 0x33).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw(m_z80pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw(m_z80pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x60, 0x67).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80, 0x80).r("aysnd", FUNC(ay8930_device::data_r));
	map(0x80, 0x81).w("aysnd", FUNC(ay8930_device::address_data_w));
	map(0xff, 0xff).w(FUNC(meritm_state::crt250_bank_w));
}

void meritm_state::map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_banks[0]);
	map(0x8000, 0xdfff).bankr(m_banks[1]);
	map(0xe000, 0xffff).bankrw(m_banks[2]).share("nvram");
}

void meritm_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(meritm_state::psd_a15_w));
	map(0x01, 0x01).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x10, 0x13).rw(m_v9938[0], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x20, 0x23).rw(m_v9938[1], FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x30, 0x33).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw(m_z80pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw(m_z80pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x60, 0x67).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80, 0x80).r("aysnd", FUNC(ay8930_device::data_r));
	map(0x80, 0x81).w("aysnd", FUNC(ay8930_device::address_data_w));
	map(0xff, 0xff).w(FUNC(meritm_state::bank_w));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START(meritm_crt260)
	PORT_START("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIO1_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Calibration" ) PORT_CODE(KEYCODE_C)

	PORT_START("DSW")   /* need for AY-8910 accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(meritm_crt250)
	PORT_START("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Raise")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIO1_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Used as "IPT_OTHER" in some games for Calibration / Clear High Score ect */

	PORT_START("DSW")   /* need for AY-8910 accesses */
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START(pitboss2)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x02, 0x02, "Enable Joker Poker / Super Stud" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia Category" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(spitboss)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia & Sex Phrases" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(pitbosss)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "In Tac Tac Trivia" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Categories are Lettered" )
	PORT_DIPSETTING(    0x01, "Categories are Numbered" )
	PORT_DIPNAME( 0x02, 0x02, "Enable Casino Games" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia Category" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(dodgecty)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Take / Lo" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME( "Hold 5 / Double Up / Hi" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START(mtjpoker)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* Touchscreen based, no keys used for this one. */

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Calibrate Touchsceen")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Enable Touchscreen" ) PORT_DIPLOCATION("SW1:1") /* MUST be ON or "touches" don't register */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Points Per Coin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Point / Coin" )
	PORT_DIPSETTING(    0x00, "5 Points / Coin" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Play" )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x00, "50 Raise Yes / 99 Raise No" )
INPUT_PORTS_END

static INPUT_PORTS_START(americna)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Take / Lo" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME( "Hold 5 / Double Up / Hi" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Raise")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME( "Hi-Score" )

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Setup / Test")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, "Points Per Coin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Point / Coin" )
	PORT_DIPSETTING(    0x00, "5 Points / Coin" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Play" )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x00, "50 Raise Yes / 99 Raise No" )
INPUT_PORTS_END

static INPUT_PORTS_START(realbrod)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME( "Stand / Clear Hi-Score" )

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Setup")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c, 0x0c, "Max Play" )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "50 Raise Yes / 99 Raise No" )
INPUT_PORTS_END

static INPUT_PORTS_START(pitbossm)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Pass/Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Collect/Quit/Raise")

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW1:1" )                /* Unused */
	PORT_DIPNAME( 0x02, 0x02, "Solitaire Timer Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia Category" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Limit" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "No Coin Limit" )
	PORT_DIPSETTING(    0x08, "4 Coin Limit" )  /* With Lockout coil */
	PORT_DIPNAME( 0x10, 0x10, "Run 21 and Trivia Whiz 2000: Coins to start" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "2 Coins" )
	PORT_DIPSETTING(    0x10, "1 Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Great Solitaire: Coins to start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "2 Coins" )
	PORT_DIPSETTING(    0x20, "1 Coin" )
	PORT_DIPNAME( 0x40, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:7")    /* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:8")    /* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(pitbossa)
	PORT_INCLUDE(pitbossm)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Collect/Quit")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, "Great Solitaire: Coins to start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "4 Coins" )
	PORT_DIPSETTING(    0x20, "2 Coins" )
INPUT_PORTS_END

static INPUT_PORTS_START(pbss330)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Clear High Score") /* In IDLE (Demo) mode */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "In Tac Tac Trivia" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Categories are Lettered" )
	PORT_DIPSETTING(    0x01, "Categories are Numbered" )
	PORT_DIPNAME( 0x02, 0x02, "Enable Casino Games" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia & Sex Phrases" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Limit" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "No Coin Limit" )
	PORT_DIPSETTING(    0x08, "2 Coin Limit" )  /* With Lockout coil */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, "Balls (Pit Pong) / Paddles (Breakin' Bricks)" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "15 Balls / 5 Paddles" )
	PORT_DIPSETTING(    0x20, "11 Balls / 3 Paddles" )
	PORT_DIPNAME( 0x40, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:7")    /* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:8")    /* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(pbst30)
	PORT_INCLUDE(pbss330)

	PORT_MODIFY("PIO1_PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED) /* Touchscreen based, no keys used for this one. */

	PORT_MODIFY("PIO1_PORTB")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Calibrate Touchsceen")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Touchscreen Type" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "13 Inch" )
	PORT_DIPSETTING(    0x01, "19 Inch (Inverts Screen Coordinates)" )
	PORT_DIPNAME( 0x20, 0x20, "Balls (Pit Pong) / Paddles (Breakin' Bricks)" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "11 Balls / 5 Paddles" )
	PORT_DIPSETTING(    0x20, "7 Balls / 3 Paddles" )
INPUT_PORTS_END

/*************************************
 *
 *  8255
 *
 *************************************/

uint8_t meritm_state::_8255_port_c_r()
{
	//logerror( "8255 port C read\n" );
	return 0xff;
}

void meritm_state::crt250_port_b_w(uint8_t data)
{
	//popmessage("Lamps: %d %d %d %d %d %d %d", BIT(data,0), BIT(data,1), BIT(data,2), BIT(data,3), BIT(data,4), BIT(data,5), BIT(data,6) );
	for (int i = 0; i < 5; i++)
		m_p1_disc_lamp[i] = !BIT(data, i);
	m_p1_play_lamp = !BIT(data, 5);
	m_p1_cancel_lamp = !BIT(data, 6);
}

/*************************************
 *
 *  AY8930
 *
 *************************************/

/*
 Port A: DSW
 Port B: Bits 0,1 used
*/

void meritm_state::ay8930_port_b_w(uint8_t data)
{
	// lamps
}

/*************************************
 *
 *  PIOs
 *
 *************************************/

uint8_t meritm_state::audio_pio_port_a_r()
{
	/*

	    bit     signal      description

	    0       BANK0
	    1       BANK1
	    2       BANK2
	    3       /VINT1      V9938 #1 INT
	    4       /VINT2      V9938 #2 INT
	    5       BANK3
	    6
	    7

	*/

	return m_vint;
}

uint8_t meritm_state::audio_pio_port_b_r()
{
	/*

	    bit     description

	    0       J4 D0
	    1       J4 D1
	    2       J4 D2
	    3       J4 D3
	    4       J4 D4
	    5       J4 D5
	    6       J4 D6
	    7       J4 D7

	*/

	return m_ds1204->read_dq();
}

void meritm_state::audio_pio_port_a_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       BANK0
	    1       BANK1
	    2       BANK2
	    3       /VINT1      V9938 #1 INT
	    4       /VINT2      V9938 #2 INT
	    5       BANK3
	    6
	    7

	*/

	m_bank = (data & 7) | ((data >> 2) & 0x18);
	//logerror("Writing BANK with %x (raw = %x)\n", m_bank, data);
}

void meritm_state::audio_pio_port_b_w(uint8_t data)
{
	/*

	    bit     description

	    0       J4 D0
	    1       J4 D1
	    2       J4 D2
	    3       J4 D3
	    4       J4 D4
	    5       J4 D5
	    6       J4 D6
	    7       J4 D7

	*/

	m_ds1204->write_rst((data >> 2) & 1);
	m_ds1204->write_clk((data >> 1) & 1);
	m_ds1204->write_dq(data & 0x01);
}

void meritm_state::io_pio_port_a_w(uint8_t data)
{
	/*

	    bit     description

	    0       J3 PE0
	    1       J3 PE1
	    2       J3 PE2
	    3       J3 PE3
	    4       J3 PE4
	    5       J3 PE5
	    6       J3 PE6
	    7       J3 PE7

	*/
}

void meritm_state::io_pio_port_b_w(uint8_t data)
{
	/*

	    bit     description

	    0       J3 PF0
	    1       J3 PF1
	    2       J3 PF2
	    3       J3 PF3
	    4       J3 PF4
	    5       J3 PF5
	    6       J3 PF6
	    7       J3 PF7

	*/
}

static const z80_daisy_config meritm_daisy_chain[] =
{
	{ "z80pio0" },
	{ "z80pio1" },
	{ nullptr }
};

MACHINE_START_MEMBER(meritm_state, common)
{
	m_z80pio[0]->strobe_a(1);
	m_z80pio[0]->strobe_b(1);
	m_z80pio[1]->strobe_a(1);
	m_z80pio[1]->strobe_b(1);
}

void meritm_state::machine_start()
{
	m_p1_disc_lamp.resolve();
	m_p1_play_lamp.resolve();
	m_p1_cancel_lamp.resolve();
	m_banks[0]->configure_entries(0, 8, m_region_maincpu->base(), 0x10000);
	m_bank = 0xff;
	crt250_switch_banks();
	MACHINE_START_CALL_MEMBER(common);
	save_item(NAME(m_bank));

}

MACHINE_START_MEMBER(meritm_state, crt250_questions)
{
	meritm_state::machine_start();
	save_item(NAME(m_questions_loword_address));
}

MACHINE_START_MEMBER(meritm_state, crt250_crt252_crt258)
{
	MACHINE_START_CALL_MEMBER(crt250_questions);
}

MACHINE_START_MEMBER(meritm_state, crt260)
{
	m_ram = std::make_unique<uint8_t[]>( 0x8000 );
	subdevice<nvram_device>("nvram")->set_base(m_ram.get(), 0x8000);
	memset(m_ram.get(), 0x00, 0x8000);
	m_banks[0]->configure_entries(0, 128, m_region_maincpu->base(), 0x8000);
	m_banks[1]->configure_entries(0, 128, m_region_maincpu->base(), 0x8000);
	m_banks[2]->configure_entries(0, 4, m_ram.get(), 0x2000);
	m_bank = 0xff;
	m_psd_a15 = 0;
	switch_banks();
	MACHINE_START_CALL_MEMBER(common);
	save_item(NAME(m_bank));
	save_item(NAME(m_psd_a15));
	save_pointer(NAME(m_ram), 0x8000);
}

TIMER_DEVICE_CALLBACK_MEMBER(meritm_state::vblank_start_tick)
{
	/* this is a workaround to signal the v9938 vblank interrupt correctly */
	m_vint = 0x08;
	m_z80pio[0]->port_a_write(m_vint);
}

TIMER_DEVICE_CALLBACK_MEMBER(meritm_state::vblank_end_tick)
{
	/* this is a workaround to signal the v9938 vblank interrupt correctly */
	m_vint = 0x18;
	m_z80pio[0]->port_a_write(m_vint);
}

void meritm_state::crt250(machine_config &config)
{
	Z80(config, m_maincpu, SYSTEM_CLK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &meritm_state::crt250_map);
	m_maincpu->set_addrmap(AS_IO, &meritm_state::crt250_io_map);
	m_maincpu->set_daisy_config(meritm_daisy_chain);

	I8255(config, m_ppi);
	m_ppi->out_pb_callback().set(FUNC(meritm_state::crt250_port_b_w));   // used LMP x DRIVE
	m_ppi->in_pc_callback().set(FUNC(meritm_state::_8255_port_c_r));

	Z80PIO(config, m_z80pio[0], SYSTEM_CLK/6);
	m_z80pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80pio[0]->in_pa_callback().set(FUNC(meritm_state::audio_pio_port_a_r));
	m_z80pio[0]->out_pa_callback().set(FUNC(meritm_state::audio_pio_port_a_w));
	m_z80pio[0]->in_pb_callback().set(FUNC(meritm_state::audio_pio_port_b_r));
	m_z80pio[0]->out_pb_callback().set(FUNC(meritm_state::audio_pio_port_b_w));

	Z80PIO(config, m_z80pio[1], SYSTEM_CLK/6);
	m_z80pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80pio[1]->in_pa_callback().set_ioport("PIO1_PORTA");
	m_z80pio[1]->out_pa_callback().set(FUNC(meritm_state::io_pio_port_a_w));
	m_z80pio[1]->in_pb_callback().set_ioport("PIO1_PORTB");
	m_z80pio[1]->out_pb_callback().set(FUNC(meritm_state::io_pio_port_b_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	DS1204(config, m_ds1204);

	V9938(config, m_v9938[0], SYSTEM_CLK);
	m_v9938[0]->set_screen_ntsc("screen");
	m_v9938[0]->set_vram_size(0x20000);
	//m_v9938[0]->int_cb().set(FUNC(meritm_state::vdp0_interrupt));

	V9938(config, m_v9938[1], SYSTEM_CLK);
	m_v9938[1]->set_screen_ntsc("screen");
	m_v9938[1]->set_vram_size(0x20000);
	//m_v9938[1]->int_cb().set(FUNC(meritm_state::vdp1_interrupt));

	TIMER(config, "vblank_start", 0).configure_scanline(FUNC(meritm_state::vblank_start_tick), "screen", 259, 262);
	TIMER(config, "vblank_end", 0).configure_scanline(FUNC(meritm_state::vblank_end_tick), "screen", 262, 262);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER).set_screen_update(FUNC(meritm_state::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8930_device &aysnd(AY8930(config, "aysnd", SYSTEM_CLK/12));
	aysnd.port_a_read_callback().set_ioport("DSW");
	aysnd.port_b_write_callback().set(FUNC(meritm_state::ay8930_port_b_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.85);
}

void meritm_state::crt250_questions(machine_config &config)
{
	crt250(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &meritm_state::crt250_questions_map);

	MCFG_MACHINE_START_OVERRIDE(meritm_state, crt250_questions)
}

void meritm_state::crt250_crt252_crt258(machine_config &config)
{
	crt250_questions(config);

	m_maincpu->set_addrmap(AS_IO, &meritm_state::crt250_crt258_io_map);

	MCFG_MACHINE_START_OVERRIDE(meritm_state, crt250_crt252_crt258)

	NS16550(config, m_uart, UART_CLK);
	m_uart->out_tx_callback().set(m_microtouch, FUNC(microtouch_device::rx));

	MICROTOUCH(config, m_microtouch, 9600).stx().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	m_microtouch->set_touch_callback(FUNC(meritm_state::touch_coord_transform));

	config.set_default_layout(layout_microtouch);
}

void meritm_state::crt260(machine_config &config)
{
	crt250(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &meritm_state::map);
	m_maincpu->set_addrmap(AS_IO, &meritm_state::io_map);

	m_ppi->out_pb_callback().set_nop();

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_msec(1200));  // DS1232, TD connected to VCC
	MCFG_MACHINE_START_OVERRIDE(meritm_state, crt260)

	NS16550(config, m_uart, UART_CLK);
	m_uart->out_tx_callback().set(m_microtouch, FUNC(microtouch_device::rx));

	MICROTOUCH(config, m_microtouch, 9600).stx().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	m_microtouch->set_touch_callback(FUNC(meritm_state::touch_coord_transform));

	config.set_default_layout(layout_microtouch);
}


/*

This set seems odd, it doesn't really have a title sequence or demo mode.
 NOT 100% sure this is "Americana" or just a no name touch Joker poker

The Touchscreen Calibration routine doesn't seem to work?

*/
ROM_START( mtjpoker ) /* Uses the CRT-258 touch controller board & Dallas DS1225Y NV SRAM */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9132-00-02_u9-r0.u9", 0x00000, 0x10000, CRC(4ec683b6) SHA1(7cff76ba1517deede3dfa2a419e11fd603dcf695) ) /* 9132-00-02 R0 46  940416 */
	ROM_LOAD( "9132-00_u10-r0.u10",  0x10000, 0x10000, CRC(d6f72934) SHA1(4f3d6a5227a3b0fc298533a03cc0a32f8e2f3840) ) // == 9131-00_u10-0.u10 from americna
	ROM_LOAD( "9132-00_u11-r0.u11",  0x20000, 0x10000, CRC(f2db6f5d) SHA1(3f734a7e8c72c14bf4a3e6f595819311739394d3) ) // == 9131-00_u11-0.u11 from americna

	ROM_REGION( 0x000022, "ds1204", 0 ) /* Connected via small CRT-254 daughter card through the J2 connector */
	ROM_LOAD( "9132-00-r0-key", 0x000000, 0x000022, BAD_DUMP CRC(c8ab29f4) SHA1(8cd352fb9335c4acba8db1a9db8ac1e08752b072) )
ROM_END

/*

Americana - Standard 5 card draw Joker Poker.

Pressing Service Mode "F2" brings up a Coins in for Coin1 & Coin2 plus total coins.

Entering the Setup menu "S":
 Hold3 switches selection choice.
 Hold5 advances through the list.
 Hi-Score will clear the High Scores

Is the "Stand" & "Hi-Score" keys the same? Without a separate Stand key, you cannot set up the "TWIN" bonus feature

REPLAYS - You start with 1 star per coin up to a maximum 15 stars. One star is deducted for each losing hand including
          busting on Double-ups while stars are added for winning hands. Running out of stars forces the player to add
          at least 1 more coin to continue to play EVEN if the player still has points available to use.

*/
ROM_START( americna ) /* Uses a small daughter card CRT-251 & Dallas DS1225Y NV SRAM */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9131-01_u9-2.u9",   0x00000, 0x10000, CRC(0410a1bf) SHA1(1218cf4b3d1067777841673620c84406cce418fb) ) /* 9131-01 U9-2  888020 - Has REPLAYS feature */
	ROM_LOAD( "9131-00_u10-0.u10", 0x10000, 0x10000, CRC(d6f72934) SHA1(4f3d6a5227a3b0fc298533a03cc0a32f8e2f3840) )
	ROM_LOAD( "9131-00_u11-0.u11", 0x20000, 0x10000, CRC(f2db6f5d) SHA1(3f734a7e8c72c14bf4a3e6f595819311739394d3) )
ROM_END

ROM_START( americnaa ) /* Uses a small daughter card CRT-251 & Dallas DS1225Y NV SRAM */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9131-00_u9-2.u9",   0x00000, 0x10000, CRC(8a741fb6) SHA1(2d77c67e5a0bdaf6199c31c4055df214672db3e1) ) /* 9131-00 U9-2  888020 - Doesn't have REPLAYS feature */
	ROM_LOAD( "9131-00_u10-0.u10", 0x10000, 0x10000, CRC(d6f72934) SHA1(4f3d6a5227a3b0fc298533a03cc0a32f8e2f3840) )
	ROM_LOAD( "9131-00_u11-0.u11", 0x20000, 0x10000, CRC(f2db6f5d) SHA1(3f734a7e8c72c14bf4a3e6f595819311739394d3) )
ROM_END

/*

This set seems odd, it doesn't really have a title sequence.

Same keys as Americana & same Stand / Hi-Score issue

This set also has additional bonuses for certain winning hands, but requires a minimum 10 point bet to qualify for any bonus.

*/
ROM_START( meritjp ) /* Uses a small daughter card CRT-255 & Dallas DS1225Y NV SRAM */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9131-09_u9-00.u9",   0x00000, 0x10000, CRC(dadaac6a) SHA1(798068f3cef6f5d41dd8ee4315e220932f31a3f7) ) /* 9131-09 U9-00 - Has REPLAYS feature */
	ROM_LOAD( "9131-01_u10-r0.u10", 0x10000, 0x10000, CRC(ebee75e2) SHA1(1ba1c92684d0f5251bb5b57a29b39467b57a1170) )
	ROM_LOAD( "9131-01_u11-r0.u11", 0x20000, 0x10000, CRC(f2db6f5d) SHA1(3f734a7e8c72c14bf4a3e6f595819311739394d3) )
ROM_END

ROM_START( dodgecty ) /* Uses a small daughter card CRT-255 & Dallas DS1225Y NV SRAM */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9131-02_u9-2t.u9",  0x00000, 0x10000, CRC(22e73039) SHA1(368f03b31f7c3cb81a95b20d1cb954e8557d2017) ) /* 9131-02 U9-2T  880111 */
	ROM_LOAD( "9131-02_u10-0.u10", 0x10000, 0x10000, CRC(bc3391f3) SHA1(4df46f31489bc5e3de3f6fc917e23b9bb5231e5a) )
	ROM_LOAD( "9131-02_u11-0.u11", 0x20000, 0x10000, CRC(f137d70c) SHA1(8ec04ec17300aa3a6ef14bcca1ca1c2aec0eea18) )
ROM_END

ROM_START( dodgectya ) /* Uses a Benchmarg BQ4010YMA-150 NV SRAM @ U8 */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9131-02_u9-2b.u9",  0x00000, 0x10000, CRC(4e8f9f67) SHA1(7d0c1963c83d00d62c80ff25345d7414031d724c) ) /* 9131-02 U9-2B  881280 */
	ROM_LOAD( "9131-02_u10-0.u10", 0x10000, 0x10000, CRC(bc3391f3) SHA1(4df46f31489bc5e3de3f6fc917e23b9bb5231e5a) )
	ROM_LOAD( "9131-02_u11-0.u11", 0x20000, 0x10000, CRC(f137d70c) SHA1(8ec04ec17300aa3a6ef14bcca1ca1c2aec0eea18) )
ROM_END

ROM_START( pitboss2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-01_u9-0c.u9",  0x00000, 0x10000, CRC(a1b6ac15) SHA1(b7b395f3e7e14dbb84003e03bf7d054e795a7211) ) /* 9221-01C  880221 */
	ROM_LOAD( "9221-01_u10-0.u10", 0x10000, 0x10000, CRC(207aa83c) SHA1(1955d75b9e561312e98831571c9853579ded3734) )
	ROM_LOAD( "9221-01_u11-0.u11", 0x20000, 0x10000, CRC(2052e043) SHA1(36b6cbc5712fc736c748a68bd12675291eae669d) )
	ROM_LOAD( "9221-01_u12-0.u12", 0x30000, 0x10000, CRC(33653f16) SHA1(57b9822499324502d66dc5a40e662596e5336943) )
	ROM_LOAD( "9221-01_u13-0.u13", 0x40000, 0x10000, CRC(4f139e88) SHA1(425dd34804cc614aa93a468d2ba3e16de62f099c) )
	ROM_LOAD( "9221-01_u14-0.u14", 0x50000, 0x10000, CRC(a58078cd) SHA1(a028be67fa05670a689144dfb9c9da51c5732389) )
	ROM_LOAD( "9221-01_u15-0.u15", 0x60000, 0x10000, CRC(239b5d03) SHA1(fffb69cd7af215445da2b1281bcbc5f4fb6cfcc3) )
	ROM_LOAD( "9221-01_u16-0.u16", 0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
ROM_END

ROM_START( spitboss )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-02_u9-0a.u9",   0x00000, 0x10000, CRC(e0c45c9c) SHA1(534bff67c8fee08f1c348275de8977659efa9f69) ) /* 9221-02A   886021 */
	ROM_LOAD( "9221-02_u10-0.u10",  0x10000, 0x10000, CRC(ed010c58) SHA1(02750944a28c1c27ce2a9904d11b7e46272a940e) )
	ROM_LOAD( "9221-02_u11-0a.u11", 0x20000, 0x10000, CRC(0c65fa86) SHA1(7906a8d615116ca67bf370dfb2da8cb2389a313d) )
	ROM_LOAD( "9221-02_u12-0.u12",  0x30000, 0x10000, CRC(0cf95b0e) SHA1(c6ffc13703892b9ae0da39a02db37c4ec890f79e) )
	ROM_LOAD( "9221-02_u13-0.u13",  0x40000, 0x10000, CRC(4f139e88) SHA1(425dd34804cc614aa93a468d2ba3e16de62f099c) ) // matches pitboss2
	ROM_LOAD( "9221-02_u14-0.u14",  0x50000, 0x10000, CRC(a58078cd) SHA1(a028be67fa05670a689144dfb9c9da51c5732389) ) // matches pitboss2
	ROM_LOAD( "9221-02_u15-0.u15",  0x60000, 0x10000, CRC(239b5d03) SHA1(fffb69cd7af215445da2b1281bcbc5f4fb6cfcc3) ) // matches pitboss2
	ROM_LOAD( "9221-02_u16-0.u16",  0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
ROM_END

ROM_START( pitbosss )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-10_u9-0b.u9",   0x00000, 0x10000, CRC(e1fbf7cb) SHA1(e04163219c357cd3da2a78ba2590d453df8e9477) ) /* 9221-10-00B 092190 */
	ROM_LOAD( "9221-10_u10-0.u10",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-10_u11-0b.u11", 0x20000, 0x10000, CRC(6d6dfaf3) SHA1(de76c577eef1bb6637aacffedcc40266af92506e) )
	ROM_LOAD( "9221-10_u12-0.u12",  0x30000, 0x10000, CRC(3577a203) SHA1(80f9c827ad9dea2c6af788bd3b46ab65e8c594eb) )
	ROM_LOAD( "9221-10_u13-0.u13",  0x40000, 0x10000, CRC(466f81f9) SHA1(88429d9ff53d27bf639200852a7bf61768c8fd1b) )
	ROM_LOAD( "9221-10_u14-0.u14",  0x50000, 0x10000, CRC(0720faa6) SHA1(1d78d711e3aab1ecf604ae7b9c374d27639a97c3) )
	ROM_LOAD( "9221-10_u15-0.u15",  0x60000, 0x10000, CRC(c302b4c2) SHA1(d62d4bb33a9ccb95d1e550f9e439be3316b94c99) )
	ROM_LOAD( "9221-10_u16-0.u16",  0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
ROM_END

ROM_START( pitbosssa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-10_u9-0a.u9",   0x00000, 0x10000, CRC(41be6b30) SHA1(c4df87a599e310ce29ee9277e5adc916ff68f060) ) /* 9221-10-00A  090370 */
	ROM_LOAD( "9221-10_u10-0.u10",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-10_u11-0a.u11", 0x20000, 0x10000, CRC(c9137469) SHA1(618680609bdffa92b919a2417bd3ec41a4c8bf2b) )
	ROM_LOAD( "9221-10_u12-0.u12",  0x30000, 0x10000, CRC(3577a203) SHA1(80f9c827ad9dea2c6af788bd3b46ab65e8c594eb) )
	ROM_LOAD( "9221-10_u13-0.u13",  0x40000, 0x10000, CRC(466f81f9) SHA1(88429d9ff53d27bf639200852a7bf61768c8fd1b) )
	ROM_LOAD( "9221-10_u14-0.u14a", 0x50000, 0x10000, CRC(0a928852) SHA1(c6c623f63a73b3de6298f436a4ca339c1447888d) )
	ROM_LOAD( "9221-10_u15-0.u15",  0x60000, 0x10000, CRC(c302b4c2) SHA1(d62d4bb33a9ccb95d1e550f9e439be3316b94c99) )
	ROM_LOAD( "9221-10_u16-0.u16",  0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
ROM_END

ROM_START( pitbosssc ) /* ROMs at U9 and U11 are localized for California, denoted by the "-1" in the label */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-12_u9-1.u9",   0x00000, 0x10000, CRC(6f55ba86) SHA1(6a9b09009f7640fe7862433a2d5ccee8ecd4f846) ) /* 9221-12-01  010892 */
	ROM_LOAD( "9221-12_u10-0.u10", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-12_u11-1.u11", 0x20000, 0x10000, CRC(869398d0) SHA1(50283ea8cf708ef177976245ec75a1a4c4408dfb) )
	ROM_LOAD( "9221-12_u12-0.u12", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9221-12_u13-0.u13", 0x40000, 0x10000, CRC(93880b55) SHA1(1bf48bb25ef85a5474d63d1a4912f46772b6be62) )
	ROM_LOAD( "9221-12_u14-0.u14", 0x50000, 0x10000, CRC(128b4dff) SHA1(945825d654b1dce2e71b4f8613029651c7641fac) )
	ROM_LOAD( "9221-12_u15-0.u15", 0x60000, 0x10000, CRC(b5beeaa9) SHA1(99db48f83d09616617b585b60614f5819f5dc607) )
	ROM_LOAD( "9221-12_u16-0.u16", 0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "pitbosssc-key", 0x000000, 0x000022, BAD_DUMP CRC(77249fe0) SHA1(719f66742147cb8e5720250ce744e5eb4983ab82) )
ROM_END

ROM_START( pitbosssm ) /* ROMs at U9 and U11 are localized for Minnesota, denoted by the "-2" in the label */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-12_u9-2.u9",   0x00000, 0x10000, CRC(d06163ae) SHA1(d6bb002dfee61153a8fd7190e34a538ad3d7c47f) ) /* 9221-12-02  010892 */
	ROM_LOAD( "9221-12_u10-0.u10", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-12_u11-2.u11", 0x20000, 0x10000, CRC(bb5233bd) SHA1(49a277fd2115661b498040fb7e20be0db3691c8c) )
	ROM_LOAD( "9221-12_u12-0.u12", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9221-12_u13-0.u13", 0x40000, 0x10000, CRC(93880b55) SHA1(1bf48bb25ef85a5474d63d1a4912f46772b6be62) )
	ROM_LOAD( "9221-12_u14-0.u14", 0x50000, 0x10000, CRC(128b4dff) SHA1(945825d654b1dce2e71b4f8613029651c7641fac) )
	ROM_LOAD( "9221-12_u15-0.u15", 0x60000, 0x10000, CRC(b5beeaa9) SHA1(99db48f83d09616617b585b60614f5819f5dc607) )
	ROM_LOAD( "9221-12_u16-0.u16", 0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "pitbosssc-key", 0x000000, 0x000022, BAD_DUMP CRC(77249fe0) SHA1(719f66742147cb8e5720250ce744e5eb4983ab82) )
ROM_END

ROM_START( pbss330 ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9233-01 U1-RO1 C1993 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9233-00-01_u9-r0b",  0x00000, 0x10000, CRC(a4747693) SHA1(f211bd095f9151a7fd7dbdb238409b56f06c5e2f) ) /* 9233-00-01  082693 */
	ROM_LOAD( "9233-00-01_u10-r0b", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) // == 9233-00-01_u10-r0
	ROM_LOAD( "9233-00-01_u11-r0b", 0x20000, 0x10000, CRC(07480c60) SHA1(7b698a58b139f28f079ccdfd5d256ac20c7d4336) )
	ROM_LOAD( "9233-00-01_u12-r0b", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) // == 9233-00-01_u12-r0
	ROM_LOAD( "9233-00-01_u13-r0b", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // == 9233-00-01_u13-r0
	ROM_LOAD( "9233-00-01_u14-r0b", 0x50000, 0x10000, CRC(19002aed) SHA1(925bcacbaff5a9f63cd2e161e65e942d59d8ba31) )
	ROM_LOAD( "9233-00-01_u15-r0b", 0x60000, 0x10000, CRC(81816257) SHA1(f627cb1a8c0e57c47537936c2b235e2e15164591) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9233-01_u1-r01_c1993_mii", 0x000000, 0x000022, BAD_DUMP CRC(93459659) SHA1(73ad4c3a7c52d3db3acb43662c535f8c2ed2376a) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9233-01_u7-r0",  0x00000, 0x40000, CRC(176dd688) SHA1(306cf78101219ef1122023a01d16dff5e9f2aecf) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9233-01_u6-r0",  0x40000, 0x40000, CRC(59c85a0a) SHA1(ef7f45c4e032d9dd14c4f5237f5b3c487be0cb2f) )
	ROM_LOAD( "qs9233-01_u5-r0",  0x80000, 0x40000, CRC(740b1274) SHA1(14eab68fc137b905a5a2739c7081900a48cba562) )
ROM_END

ROM_START( pbss330a ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9233-01 U1-RO1 C1993 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9233-00-01_u9-r0",  0x00000, 0x10000, CRC(887da433) SHA1(2950803cef75e0d337fbcedaeea994ec82c9db71) ) /* 9233-00-01  072893 */
	ROM_LOAD( "9233-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9233-00-01_u11-r0", 0x20000, 0x10000, CRC(0c02e464) SHA1(9283f324a8582ad98495e084750637e2a02a7474) )
	ROM_LOAD( "9233-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9233-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
	ROM_LOAD( "9233-00-01_u14-r0", 0x50000, 0x10000, CRC(c6701f15) SHA1(d475c4490df8dfa6f2374bb70ef12c7afaecd501) )
	ROM_LOAD( "9233-00-01_u15-r0", 0x60000, 0x10000, CRC(5810840e) SHA1(bad6457752ac212c3c11360a13a8d3473662a287) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9233-01_u1-r01_c1993_mii", 0x000000, 0x000022, BAD_DUMP CRC(93459659) SHA1(73ad4c3a7c52d3db3acb43662c535f8c2ed2376a) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9233-01_u7-r0",  0x00000, 0x40000, CRC(176dd688) SHA1(306cf78101219ef1122023a01d16dff5e9f2aecf) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9233-01_u6-r0",  0x40000, 0x40000, CRC(59c85a0a) SHA1(ef7f45c4e032d9dd14c4f5237f5b3c487be0cb2f) )
	ROM_LOAD( "qs9233-01_u5-r0",  0x80000, 0x40000, CRC(740b1274) SHA1(14eab68fc137b905a5a2739c7081900a48cba562) )
ROM_END

ROM_START( pbss330ca ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9233-06 U1-RO C1993 MII - California version */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9233-00-06_u9-r0a",  0x00000, 0x10000, CRC(0aaa94e3) SHA1(915a0d4643a781b39730c64dfcaa7599e5a0c447) ) /* 9233-00-06  081293 */
	ROM_LOAD( "9233-00-06_u10-r0a", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) // == 9233-00-01_u10-r0
	ROM_LOAD( "9233-00-06_u11-r0a", 0x20000, 0x10000, CRC(94cfb8b1) SHA1(bf2baf1fe9bd87abec353ec8402370e12809030a) )
	ROM_LOAD( "9233-00-06_u12-r0a", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) // == 9233-00-01_u12-r0
	ROM_LOAD( "9233-00-06_u13-r0a", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // == 9233-00-01_u13-r0
	ROM_LOAD( "9233-00-06_u14-r0a", 0x50000, 0x10000, CRC(2aa38f55) SHA1(c1d80b619b7b6506d457ceb6aa267e5ef7c3bdf2) )
	ROM_LOAD( "9233-00-06_u15-r0a", 0x60000, 0x10000, CRC(e3ce9cde) SHA1(54b25e0f2715e2b112916b80b918a0191bf87a48) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9233-06_u1-r0_c1993_mii", 0x000000, 0x000022, BAD_DUMP CRC(93459659) SHA1(73ad4c3a7c52d3db3acb43662c535f8c2ed2376a) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9233-01_u7-r0",  0x00000, 0x40000, CRC(176dd688) SHA1(306cf78101219ef1122023a01d16dff5e9f2aecf) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9233-01_u6-r0",  0x40000, 0x40000, CRC(59c85a0a) SHA1(ef7f45c4e032d9dd14c4f5237f5b3c487be0cb2f) )
	ROM_LOAD( "qs9233-01_u5-r0",  0x80000, 0x40000, CRC(740b1274) SHA1(14eab68fc137b905a5a2739c7081900a48cba562) )
ROM_END

/*
Basically this Pit Boss Megatouch set is Pit Boss Supertouch 30 v2.0 but marks the first time Merit
 started using the Megatouch name.

NOTE: Once again U10, U12 & U13 doesn't change between this set and the Pit Boss Supertouch 30 sets
      and the question roms are the same data with a new label and game number ID
*/
ROM_START( megat ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9234-20 U1-RO C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9234-20-01_u9-r0a",  0x00000, 0x10000, CRC(5a9fd092) SHA1(756b6a925dafb17451e7dc37c95a26d09ecfe2d7) ) /* 9234-20-01 R0A 940519 */
	ROM_LOAD( "9234-20-01_u10-r0a", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Also found as PBC U10 */
	ROM_LOAD( "9234-20-01_u11-r0a", 0x20000, 0x10000, CRC(8bd5f6bb) SHA1(95b23d7d14207fcafc01ee975400ebdd1e7b5ad5) )
	ROM_LOAD( "9234-20-01_u12-r0a", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Also found as PBC U12 */
	ROM_LOAD( "9234-20-01_u13-r0a", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Also found as PBC U13 */
	ROM_LOAD( "9234-20-01_u14-r0a", 0x50000, 0x10000, CRC(40d78506) SHA1(5e1d8e4ef8aa02faa2a323f5e988bf56d4747b60) )
	ROM_LOAD( "9234-20-01_u15-r0a", 0x60000, 0x10000, CRC(9adc67b8) SHA1(271e6b6473eeea01f2923ef82c192a583bb5e338) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9234-20_u1-r0_c1994_mii", 0x000000, 0x000022, BAD_DUMP CRC(6cbdbde1) SHA1(b076ee21fc792a5e85cdaed427bc41554568811e) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9234-20_u7-r0",  0x00000, 0x40000, CRC(c0534aaa) SHA1(4b3cbf03f29fd5b4b8fd423e73c0c8147692fa75) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9234-20_u6-r0",  0x40000, 0x40000, CRC(fe2cd934) SHA1(623011dc53ed6eefefa0725dba6fd1efee2077c1) ) /* Same data as Pit Boss Supertouch 30 sets, different label - verified */
	ROM_LOAD( "qs9234-20_u5-r0",  0x80000, 0x40000, CRC(293fe305) SHA1(8a551ae8fb4fa4bf329128be1bfd6f1c3ff5a366) )
ROM_END

ROM_START( pbst30 ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9234-10 U1-RO1 C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9234-10-01_u9-r0",  0x00000, 0x10000, CRC(96f39c9a) SHA1(df698e94a5204cf050ceadc5c257ca5f68171114) ) /* 9234-10-01 032294 */
	ROM_LOAD( "9234-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9234-10-01_u11-r0", 0x20000, 0x10000, CRC(835fa041) SHA1(2ae754c5fcf50548eb214902409217d1643c6eaa) )
	ROM_LOAD( "9234-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9234-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
	ROM_LOAD( "9234-10-01_u14-r0", 0x50000, 0x10000, CRC(9b0873a4) SHA1(7362c6220aa4bf1a9ab7c11cb8a51587a2a0a992) )
	ROM_LOAD( "9234-10-01_u15-r0", 0x60000, 0x10000, CRC(9fbd8582) SHA1(c0f68c8a7cdca34c8736cefc71767c421bcaba8a) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9234-10_u1-r01_c1994_mii", 0x000000, 0x000022, BAD_DUMP CRC(1c782f78) SHA1(8255afcffbe21a43f53cfb41867552681403ea47) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9234-01_u7-r0",  0x00000, 0x40000, CRC(c0534aaa) SHA1(4b3cbf03f29fd5b4b8fd423e73c0c8147692fa75) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9234-01_u6-r0",  0x40000, 0x40000, CRC(fe2cd934) SHA1(623011dc53ed6eefefa0725dba6fd1efee2077c1) )
	ROM_LOAD( "qs9234-01_u5-r0",  0x80000, 0x40000, CRC(293fe305) SHA1(8a551ae8fb4fa4bf329128be1bfd6f1c3ff5a366) )
ROM_END

ROM_START( pbst30a ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9234-01 U1-RO1 C1993 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9234-00-01_u9-r0a",  0x00000, 0x10000, CRC(5f058f95) SHA1(98382935340a076bdb1b20c7f16c25b6084599fe) ) /* 9234-00-01  122293 */
	ROM_LOAD( "9234-00-01_u10-r0",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9234-00-01_u11-r0a", 0x20000, 0x10000, CRC(79125fb5) SHA1(6ca4f33c363cfb6f5c0f23b8fcc8cfcc076f68b1) )
	ROM_LOAD( "9234-00-01_u12-r0",  0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9234-00-01_u13-r0",  0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
	ROM_LOAD( "9234-00-01_u14-r0a", 0x50000, 0x10000, CRC(e83f91d5) SHA1(1d64c943787b239763f44be412ee7f5ad13eb37d) )
	ROM_LOAD( "9234-00-01_u15-r0a", 0x60000, 0x10000, CRC(f10f0d39) SHA1(2b5d5a93adb5251e09160b10c067b6e70289f608) )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9234-01_u1-r01_c1993_mii", 0x000000, 0x000022, BAD_DUMP CRC(74bf0546) SHA1(eb44a057cf797279ee3456a74e166fa711547ea4) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9234-01_u7-r0",  0x00000, 0x40000, CRC(c0534aaa) SHA1(4b3cbf03f29fd5b4b8fd423e73c0c8147692fa75) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9234-01_u6-r0",  0x40000, 0x40000, CRC(fe2cd934) SHA1(623011dc53ed6eefefa0725dba6fd1efee2077c1) )
	ROM_LOAD( "qs9234-01_u5-r0",  0x80000, 0x40000, CRC(293fe305) SHA1(8a551ae8fb4fa4bf329128be1bfd6f1c3ff5a366) )
ROM_END

ROM_START( pitbossmb ) /* Unprotected or patched??  The manual shows a DS1204 key for this set */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9243-00-01_u9-r0",  0x00000, 0x10000, CRC(55e14fb1) SHA1(ec29764d1b63360f64b82452e0db8054b99fcca0) ) /* 9243-00-01  R0 940616 */
	ROM_LOAD( "9243-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Could also be labeled 9234-00-01 U10-R0 or PBC U10 */
	ROM_LOAD( "9243-00-01_u11-r0", 0x20000, 0x10000, CRC(47a9dfc7) SHA1(eca100003f5605bcf405f610a0458ccb67894d35) )
	ROM_LOAD( "9243-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Could also be labeled 9234-00-01 U12-R0 or PBC U12 */
	ROM_LOAD( "9243-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Could also be labeled 9234-00-01 U13-R0 or PBC U13 */
	ROM_RELOAD(                    0x50000, 0x10000) /* U14 is unused for this set */
	ROM_LOAD( "9243-00-01_u15-r0", 0x60000, 0x10000, CRC(27034061) SHA1(cff6be592a4a3ab01c204b081470f224e6186c4d) )
	ROM_RELOAD(                    0x70000, 0x10000)

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9243-00-01_u7-r0",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9243-00-01_u6-r0",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "qs9243-00-01_u5-r0",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

/*
    Pit Boss Megastar - Merit Industries Inc. 1994

 Runs on standard Merit CRT-250 PCB with the following additions:
  CRT-256 Memory Expansion board
  CRT-254 Module (to connect the DS1204 to main board via connector J2)
  CRT-243 Video Billboard board (Optional) with optional Video Billboard Keyboard

From the "PIT BOSS MEGASTAR CHIP SET UPGRADE KIT" Program No. 9243-XX PM8938 Owner's Manual:

  This kit updates the Superstar 30 program (9233-xx) to the Megastar program. The new EPROM set consists of 3
  EPROMS: 9243-xx-xx U9, U11, U15. EPROMs U10, U12 and U13 are unchanged and do not need to be replaced in a
  field upgrade. U14 will be removed as it is no longer used.

Custom Program Versions:

Program #      Description                  Differences
9243-00-01     Standard Megastar            Includes all Options / No Restrictions
9243-00-06     California Megastar          Excludes Free Play Feature
9243-00-07     New Jersey Megastar          Excludes Set Trivia and includes 4-coin limit with lockout coil

From ADDENDUM:

MANUAL: Megastar Owners Game Manual PM8939
DATE: 8/23/94
9244-00-01, 06, 07

Description of Changes:

1- Great Draw Poker and 7 Stud Poker have been added to the program set
2- On page 3-1 legend artwork has changed. PASS has been replaced with
   PASS/PLAY and COLLECT/QUIT has been replaced with COLLECT/QUIT/RAISE
3- An additional Solitaire Instruction decal has been added to the kit.
   This new Instruction decal is to be mounted in a visible location for
   players use.

*/
ROM_START( pitbossm ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9244-00 U1-RO1 C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9244-00-01_u9-r0b",  0x00000, 0x10000, CRC(6d59f06f) SHA1(2ece522ead84d2d116972a9bc714dafa90b2a27b) ) /* 9244-00-01 R0B  941123 */
	ROM_LOAD( "9244-00-01_u10-r0",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Could also be labeled 9234-00-01 U10-R0 or PBC U10 */
	ROM_LOAD( "9244-00-01_u11-r0b", 0x20000, 0x10000, CRC(3c1c8eb9) SHA1(a9685df6cc879ad7b665b82327f3d8410b7dded2) )
	ROM_LOAD( "9244-00-01_u12-r0",  0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Could also be labeled 9234-00-01 U12-R0 or PBC U12 */
	ROM_LOAD( "9244-00-01_u13-r0",  0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Could also be labeled 9234-00-01 U13-R0 or PBC U13 */
	ROM_LOAD( "9244-00-01_u14-r0b", 0x50000, 0x10000, CRC(d5532ea0) SHA1(26f5289d6cf3d7ebcfe300a6599e3ff49bc8eee7) )
	ROM_LOAD( "9244-00-01_u15-r0b", 0x60000, 0x10000, CRC(2109386c) SHA1(590dcff7543d71e0911f82f27626887fcf25f2b3) )
	ROM_RELOAD(                     0x70000, 0x10000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9244-00_u1-r01_c1994_mii", 0x000000, 0x000022, BAD_DUMP CRC(0455e18b) SHA1(919b48c25888af0af34b2d0cf34370476a97b79e) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9243-00-01_u7-r0",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9243-00-01_u6-r0",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "qs9243-00-01_u5-r0",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

ROM_START( pitbossma ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9244-00 U1-RO1 C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9244-00-01_u9-r0",  0x00000, 0x10000, CRC(8317fea1) SHA1(eb84fdca7cd51883153561785571790d12d0d612) ) /* 9244-00-01 R0  940822 */
	ROM_LOAD( "9244-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Could also be labeled 9234-00-01 U10-R0 or PBC U10 */
	ROM_LOAD( "9244-00-01_u11-r0", 0x20000, 0x10000, CRC(45223e0d) SHA1(45070e85d87aa67ecd6a1355212f1d24142fcbd0) )
	ROM_LOAD( "9244-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Could also be labeled 9234-00-01 U12-R0 or PBC U12 */
	ROM_LOAD( "9244-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Could also be labeled 9234-00-01 U13-R0 or PBC U13 */
	ROM_LOAD( "9244-00-01_u14-r0", 0x50000, 0x10000, CRC(c0d18911) SHA1(def939c6bac1e3124197f3f783d06f3bef3d03e9) )
	ROM_LOAD( "9244-00-01_u15-r0", 0x60000, 0x10000, CRC(740e3734) SHA1(6440d258af114f3820683b4e6fba5db6aea02231) )
	ROM_RELOAD(                    0x70000, 0x10000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9244-00_u1-r01_c1994_mii", 0x000000, 0x000022, BAD_DUMP CRC(0455e18b) SHA1(919b48c25888af0af34b2d0cf34370476a97b79e) )

	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9243-00-01_u7-r0",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) ) /* These 3 roms are on CRT-256 satellite PCB */
	ROM_LOAD( "qs9243-00-01_u6-r0",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "qs9243-00-01_u5-r0",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

/*

The Real Broadway - Standard 5 card draw Joker Poker.

 Single rom at U38 on a Merit CTR-260 PCB with a Dallas DS1204U-3 security key.
 Uses standard draw poker keys - NOT a touchscreen based game.
 Dipswitches are used for in game functions.

Pressing Service Mode "F2" brings up a Coins in for Coin1 & Coin2 plus total coins.
  At this screen pressing in order Hold1 through Hold5 results in a simple system test.
  The program responds with "STATUS OK."

Entering the Setup menu "S":
 Hold3 switches selection choice.
 Hold5 advances through the list.
 Stand will clear the High Scores
  Pressing Stand a second time clears All Time High Scores

In the rom there is text for:
  Hi Lo Double Up mini game
  DIP 5 use (unknown at this time)
  PUSH STAND FOR BONUS SET UP
  ENTER NEW PASSWORD
  TEN STRAIGHT NO-WINS RETURNS FIRST NINE BETS

 Additional Setup menu items:
  Double Up            Yes / No
  Unlimited Double Up  Yes / No
  Take 1/2, DBL 1/2    Yes / No
  Bonus Feature        Yes / No
  Bonus Type           10Loss / Twin
  Re-Bet               Yes / No
  Hi-Score             Yes / No

It's unknown if the above is used for regional versions of the game or left over from previous
 versions of games such as Americana or Dodge City which also contains many of the same text strings.
It's currently unknown how to access / enable those features or if it's possible to do so.

*/
ROM_START( realbrod ) /* Dallas DS1204U-3 security key labeled 9131-20-00-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	/* U32 Empty */
	/* U36 Empty */
	/* U37 Empty */
	ROM_LOAD( "9131-20-00_u38-r0c", 0x300000, 0x080000, CRC(1e6150d1) SHA1(c7963f829d9cfa5b478ed53a802c03128c961db9) ) /* Location U38, 9131-20-00 R0C 01/02/1996   16:50:06 */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9131-20-00-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(89e45123) SHA1(6eddd33e1b465112e9442be46aee69d95130d780) )
ROM_END

ROM_START( realbroda ) /* Dallas DS1204U-3 security key labeled 9131-20-00-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	/* U32 Empty */
	/* U36 Empty */
	/* U37 Empty */
	ROM_LOAD( "9131-20-00_u38-r0a", 0x300000, 0x080000, CRC(28e2b4f2) SHA1(d4441034aa85fee61b93b99867ab21203165a6f5) ) /* Location U38, 9131-20-00 R0A 04/07/1995   14:45:21 */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9131-20-00-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(89e45123) SHA1(6eddd33e1b465112e9442be46aee69d95130d780) )
ROM_END


/*
    Mega Touch
    by Merit Industries

    CTR-260 PB10004

    System Info
    -----------
     This is a counter top Touchscreen game.

    processor.. Z80 (Z0840006PSC)
    sound chip Yamaha YM2149F
    other chips- two Yamaha V9938
             one LM1203
             one PC16550DN
             one PB255a or L5220574
             One Dallas DS1204 Data Key
             One Dallas DS1225Y 64k Non-volatile SRAM (Mega Touch 4)
              or Dallas DS1230Y 256K Non-volatile SRAM (Mega Touch 6)
              or Dallas DS1644 32K NVRAM + RTC (Tournament sets)
             Two Z80APIO (Z0842004PSC)

    OSC 21.477270 MHz & 1.8432MHz (near the PC16550DN)


Actual Megatouch 4 rom labels
--------------------------------

9255-40-01
 U32-R0      = 27C801
C1996 MII

QS9255-02
 U36-R0      = 27C040
C1996 MII

QS9255-02
 U37-R0      = 27C040
C1996 MII

9255-40-01
 U38-R0E     = 27C4001 (AKA 27C040)
C1996 MII

9255-40-01
U5-B-RO1     =  Dallas DS1204V
C1996 MII


Actual Megatouch 6 rom labels
--------------------------------

9255-60-01
 U32-R0      = 27C801
C1997 MII

QS9255-08
 U36-R0      = 27C040
C1998 MII

QS9255-08
 U37-R0      = 27C801
C1998 MII

9255-80-01
 U38-R0A     = 27C801
C1998 MII

9255-80
U5-B-RO1     =  Dallas DS1204V
C1998 MII


PAL:
SC3944-0A.u19 = PALCE22V10H-25PC/4
SC3980.u40    = PALCE16V8H-25
SC3981-0A.u51 = PALCE16V8H-25
SC3943.u20    = ATF16V8B25PC

*/

ROM_START( megat2 ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0b", 0x000000, 0x080000, CRC(f7ecd49b) SHA1(34c1293da7304e8a46a96f1dbd7add3291afe3fc) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-01_u38-r0g",    0x300000, 0x080000, CRC(6bc7f1ce) SHA1(ca26afe19966f37e95f8ca25e69bbdcc1e8624d7) ) /* Location U38, 02/10/1995 09:14:15 - Standard version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat2a ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0b", 0x000000, 0x080000, CRC(f7ecd49b) SHA1(34c1293da7304e8a46a96f1dbd7add3291afe3fc) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-01_u38-r0e",    0x300000, 0x080000, CRC(797fbbaf) SHA1(8d093374f109831e469133aaebc3f7c2a5ed0623) ) /* Location U38, 11/29/1994 10:51:00 - Standard version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat2b ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0a", 0x000000, 0x080000, CRC(ec0c18f6) SHA1(ae4f60f516097607249dbd902f8aacfe95acb065) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-01_u38-r0d",    0x300000, 0x080000, CRC(f43de55f) SHA1(456b4098e22982d5f1c6f872684eefb473939747) ) /* Location U38, 941123 514 - Standard version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat2mn ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0b", 0x000000, 0x080000, CRC(f7ecd49b) SHA1(34c1293da7304e8a46a96f1dbd7add3291afe3fc) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-02_u38-r0g",    0x300000, 0x080000, CRC(22f508be) SHA1(a34c9c1ae588ec8186f328119aa62600d05f192e) ) /* Location U38, 02/21/1995 16:46:14 - Minnesota version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat2ca ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0b", 0x000000, 0x080000, CRC(f7ecd49b) SHA1(34c1293da7304e8a46a96f1dbd7add3291afe3fc) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-06_u38-r0g",    0x300000, 0x080000, CRC(51b8160a) SHA1(f2dd44ff3bd62c86c385b5e1438c560947f6c253) ) /* Location U38, 02/10/1995 10:03:52 - California version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat2caa ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0b", 0x000000, 0x080000, CRC(f7ecd49b) SHA1(34c1293da7304e8a46a96f1dbd7add3291afe3fc) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-06_u38-r0e",    0x300000, 0x080000, CRC(b3c0e60a) SHA1(a633fec476f44ec7964329bd80257b9070043209) ) /* Location U38, 11/29/1994 11:23:00 - California version */
	ROM_RELOAD(                        0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-10-01-u5-r0", 0x000000, 0x000022, BAD_DUMP CRC(b13c68d2) SHA1(99f9584ba005d32ad8abefd64159a8c296dcd580) )
ROM_END

ROM_START( megat3 ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-01_u38-r0n", 0x300000, 0x080000, CRC(c3b1739d) SHA1(a12d4d4205e71cf306c7e4a7b03af017096e2492) ) /* Location U38, 02/20/1996 09:32:34 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3a ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-01_u38-r0k", 0x300000, 0x080000, CRC(3c7dfff5) SHA1(b1265d6541199a1327a87881457616c56cbb8779) ) /* Location U38, 02/09/1996 14:11:24 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3b ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-01_u38-r0f", 0x300000, 0x080000, CRC(85f48b91) SHA1(7a38644ac7ee55a254c037122af919fb268744a1) ) /* Location U38, 10/27/1995 14:23:00 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3c ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0",  0x000000, 0x080000, CRC(69110f8f) SHA1(253487f0b4a82072efb7c70bebf953ea1c41d0d8) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-01_u38-r0b", 0x300000, 0x080000, CRC(e2d7e2c5) SHA1(bf0be5f2142e5563eb3286f5b1a643943d685621) ) /* Location U38, 06/22/1995 15:30:06 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3d ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0",  0x000000, 0x080000, CRC(69110f8f) SHA1(253487f0b4a82072efb7c70bebf953ea1c41d0d8) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-01_u38-r0a", 0x300000, 0x080000, CRC(1292c90e) SHA1(d6ca81396ae4f6c62a55ec688b3a36272b9c29fd) ) /* Location U38, 06/21/1995 09:31:31 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3ca ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0a", 0x000000, 0x080000, CRC(69110f8f) SHA1(253487f0b4a82072efb7c70bebf953ea1c41d0d8) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-06_u38-r0n", 0x300000, 0x080000, CRC(f9ff003a) SHA1(6c32098593c444785de2deca0f8748042980d84d) ) /* Location U38, 02/20/1996 09:24:17 - California version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3caa ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-06_u38-r0d", 0x300000, 0x080000, CRC(c40b3a57) SHA1(7a13172b94188c5cba32622016a05eb904714a86) ) /* Location U38, 07/24/1995 12:05:34 - California version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3nj ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-07_u38-r0g", 0x300000, 0x080000, CRC(0ac673e7) SHA1(6b014366fcc5cdaa3d6a7e40da580d14def80174) ) /* Location U38, 11/17/1995 09:43:15 - New Jersey version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-20-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(105fd1de) SHA1(da5e678b633df4d7eb5eac2647d7f1fbe04add7b) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3te ) /* Dallas DS1204V security key at U5 labeled 9255-30-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-30-01_u32-r0",  0x000000, 0x100000, CRC(31ac0004) SHA1(4bec97a852a7dadb0ab4f193bc376ed149102082) ) /* Location U32 */
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-30-01_u38-r0e", 0x300000, 0x080000, CRC(52ca7dd8) SHA1(9f44f158d67d7443405b87a18fc89d9c88be1dea) ) /* Location U38, 02/15/1996 16:04:36 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-30-01_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(562e83c8) SHA1(865c8f18711df5dac9c7301f67be5bfcc925cd3d) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3tg ) /* Dallas DS1204V security key at U5 labeled 9255-30-50 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-30-01_u32-r0",  0x000000, 0x100000, CRC(31ac0004) SHA1(4bec97a852a7dadb0ab4f193bc376ed149102082) ) /* Location U32 */
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-30-50_u38-r0f", 0x300000, 0x080000, CRC(dbab32d9) SHA1(c05f31c4aad0ba9ff74aa68e80e0376b014d52a1) ) /* Location U38, 03/13/1996 11:34:57 - Bi-Lingual GER/ENG Version */
	ROM_RELOAD(                     0x380000, 0x080000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-30-50_u5-r01_c1995_mii", 0x000000, 0x000022, BAD_DUMP CRC(6b5d2ac2) SHA1(463ab84972a065598e35e3d31176770afdebfbeb) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4 ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-40-01_u38-r0e", 0x300000, 0x80000,  CRC(407c5e57) SHA1(c7c907b3fd6a8e64dcc6c71288505980862effce) ) /* Location U38, 07/22/1996 14:52:24 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4a ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-40-01_u38-r0d", 0x300000, 0x80000,  CRC(0d098424) SHA1(ef2810ccd636e69378fd353c8a95605274bb227f) ) /* Location U38, 07/08/1996 14:16:56 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4b ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-40-01_u38-r0b", 0x300000, 0x80000,  CRC(0a16c846) SHA1(f0dcddb155f5e23a8dcf6bd8018cf6dc20c6bd34) ) /* Location U38, 05/03/1996 15:12 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4c ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-40-01_u38-r0a", 0x300000, 0x80000,  CRC(74188592) SHA1(aaa4cb5eb413e963c4ff3705904449e244b984ca) ) /* Location U38, 04/22/1996 14:31 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4d ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-40-01_u38-r0",  0x300000, 0x80000,  CRC(ec96813d) SHA1(f93bb08ae89ab5ec1c6b33d5b1040c50d3db9ef5) ) /* Location U38, 04/03/1996 14:01 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4s ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-01_u38-r0g", 0x300000, 0x80000,  CRC(9c0a515a) SHA1(01b9761a8ddf95e32498ac204844144d9dc32012) ) /* Location U38, 12/10/1996  17:08:08 - Standard version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(f31864ff) SHA1(ff44820379a350e7bd788ffb6926612b3483e114) ) // sldh
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(ad4fddaa) SHA1(10c1575dcaa5ca4af5dc630d84f43a9ed1cb3ace) ) // sldh
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4sa ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-01_u38-r0e", 0x300000, 0x80000,  CRC(69cbf865) SHA1(ce555b6ab70fa57f3f87a0028db563ceee4a416b) ) /* Location U38, 10/22/1996  11:05:04 - Standard version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(f31864ff) SHA1(ff44820379a350e7bd788ffb6926612b3483e114) ) // sldh
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(ad4fddaa) SHA1(10c1575dcaa5ca4af5dc630d84f43a9ed1cb3ace) ) // sldh
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4sb ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-01_u38-r0c", 0x300000, 0x80000,  CRC(14b9fe96) SHA1(a324e1ef616b33ee4235f6bed04f6d4b0b537521) ) /* Location U38, 10/04/1996  09:39:04 - Standard version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(f31864ff) SHA1(ff44820379a350e7bd788ffb6926612b3483e114) ) // sldh
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(ad4fddaa) SHA1(10c1575dcaa5ca4af5dc630d84f43a9ed1cb3ace) ) // sldh
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4smn ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-C-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-02_u38-r0c", 0x300000, 0x80000,  CRC(0493168d) SHA1(99da5454902aa5dbc5939d4bef22af3e467e61d2) ) /* Location U38, 10/08/1996 09:56:42 - Minnesota version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-c-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1225Y nv ram
	ROM_LOAD( "mt4smn_ds1225y.u31",  0x0000, 0x8000, CRC(3f47e8e9) SHA1(ecf2937ddf05206c68262bccb8cb4a6c2a4048e8) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4snj ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0a", 0x000000, 0x100000, CRC(9ace8f52) SHA1(7c755c77cbfb234e1d6f531c90e4a8661275d464) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-07_u38-r0g", 0x300000, 0x80000,  CRC(71eac4d4) SHA1(73b9ed876f0af94bbd88503921a2b4f26bcfd397) ) /* Location U38, 02/11/1997 11:59:41 - New Jersey version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-40-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(f1de113a) SHA1(0ddd963e24a5c36f11967c2653ec5991a6eaa1a4) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1225Y nv ram
	ROM_LOAD( "mt4snj_ds1225y.u31",  0x0000, 0x8000, CRC(8d2a97e7) SHA1(7cb01d9499fed1674da6a04a11ed1cef0a39b3c0) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4te ) /* Dallas DS1204V security key at U5 labeled 9255-50-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-50-01_u38-r0d", 0x300000, 0x080000, CRC(124d5b84) SHA1(3c2117f56d0dc406bfb508989729e36781e215a4) ) /* Location U38, 07/02/1996 14:41:59 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-50-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(9db02da4) SHA1(d4eec99e814dd6daa091f1ff2fb06bda314c5029) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4te_ds1644.u31",  0x00000,  0x8000,   CRC(d9485491) SHA1(c602bf954fe8b06f81b0f5002246e8fa89237705) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4tea ) /* Dallas DS1204V security key at U5 labeled 9255-50-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-50-01_u38-r0a", 0x300000, 0x080000, CRC(abf187a5) SHA1(d4d2327b4564f3cafa2640499f8c6ae818ed04b8) ) /* Location U38, 06/06/1996 13:43:39 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-50-01_u5-b-r01_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(9db02da4) SHA1(d4eec99e814dd6daa091f1ff2fb06bda314c5029) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4tea_ds1644.u31",  0x00000,  0x8000,   CRC(11e2c7ed) SHA1(99ee83410f7dbf5a259b11193829bb5c706d9fca) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4st ) /* Dallas DS1204V security key at U5 labeled 9255-51-01 U5-RO C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-51-01_u38-r0b", 0x300000, 0x080000, CRC(181a83cb) SHA1(b8f92ae76ebba3849db76b084f0ab7d82256d81a) ) /* Location U38, 12/10/1996 16:59:23 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-51-01_u5-r0_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(14e4dfa8) SHA1(6a6a2a49c6862bbba3bde766e8f000828b1b3998) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4st_ds1644.u31",  0x00000,  0x8000,   CRC(c6226d91) SHA1(20c9fa7ad135ac229c6bdf85b901629a0ecb8a81) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4stg ) /* Dallas DS1204V security key at U5 labeled 9255-51-01 U5-RO C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-51-50_u38-r0a", 0x300000, 0x080000, CRC(f7c2914d) SHA1(5d05b8db5ca734f7b05c3e215c0ef5b917455537) ) /* Location U38, 11/18/1996 10:11:01 - Bi-Lingual GER/ENG Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-51-01_u5-r0_c1996_mii", 0x000000, 0x000022, BAD_DUMP CRC(14e4dfa8) SHA1(6a6a2a49c6862bbba3bde766e8f000828b1b3998) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4stg_ds1644.u31",  0x00000,  0x8000,   CRC(7f6f8e57) SHA1(d65f20ae19afc05b33d7605143b8362d6e955e89) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5 ) /* Dallas DS1204V security key at U5 labeled 9255-60-01 U5-C-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-60-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) )
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-60-01_u38-r0i", 0x300000, 0x100000, CRC(82a4471d) SHA1(e66ab64bb7047e248f9edbf99eb83c480895dc68) ) /* Location U38, 09/26/1997 12:09:52 - Standard Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-60-01_u5-c-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(81f1c9b1) SHA1(e03ab8fae8225332edd353725039ad0cedcd9493) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5a ) /* Dallas DS1204V security key at U5 labeled 9255-60-01 U5-C-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-60-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) )
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-60-01_u38-r0c", 0x300000, 0x100000, CRC(1091e7fd) SHA1(3c31c178eb7bea0d2c7e839dc3ec549463092296) ) /* Location U38, 07/10/1997 16:49:56 - Standard Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-60-01_u5-c-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(81f1c9b1) SHA1(e03ab8fae8225332edd353725039ad0cedcd9493) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 (or equivalent)
	 // this was recreated, not dumped. To be 100% sure it would be better to get a real dump - will NOT start without a valid NVRAM
	ROM_LOAD( "mt5a_ds1644.u31",  0x00000, 0x8000, BAD_DUMP CRC(5e13a99e) SHA1(6010c76090ddae4ebdd36f5501cf5588655ab843) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5nj ) /* Dallas DS1204V security key at U5 labeled 9255-60-01 U5-B-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-60-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) )
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-60-07_u38-r0n", 0x300000, 0x100000, CRC(c8163fe8) SHA1(94199b892ce9e5f543e10f3f59a9aeee4782923f) ) /* Location U38, 07/13/1998 15:19:55 - New Jersey version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-60-01_u5-b-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(81f1c9b1) SHA1(e03ab8fae8225332edd353725039ad0cedcd9493) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5g ) /* Dallas DS1204V security key at U5 labeled 9255-60-01 U5-B-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-70-50_u32-r0",  0x000000, 0x100000, CRC(f57e4d36) SHA1(c16587c95fa1abe2e7df37027deb2cfbadb27038) ) /* This set requires 9255-70-50 not the standard 9255-60-1 ROM */
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-60-50_u38-r0g", 0x300000, 0x100000, CRC(45e356f2) SHA1(adf8ed861bc74a223f29d1a4dac0708c5560b382) ) /* Location U38, 08/12/1997 11:26:58 - Bi-Lingual GER/ENG Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-60-01_u5-b-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(81f1c9b1) SHA1(e03ab8fae8225332edd353725039ad0cedcd9493) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt5g_ds1644.u31",  0x00000, 0x8000, CRC(68e3fdcc) SHA1(a540480a5e93ba94247e59f02a075f860c93e9d3) ) /* No actual label, so use a unique name for this set - will NOT start without a valid NVRAM */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5t ) /* Dallas DS1204V security key at U5 labeled 9255-70-01 U5-RO C1997 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-60-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) )
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-70-01_u38-r0c", 0x300000, 0x100000, CRC(e4d71764) SHA1(7c4e8b484dc744a93ce42e24f3b6d5bb2a7c09e4) ) /* Location U38, 09/30/1997 12:13:24 - Standard version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-70-01_u5-r0_c1997_mii", 0x000000, 0x000022, BAD_DUMP CRC(1888e4f2) SHA1(045b94d6600345c10098cae321811717071c4902) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt5t_ds1644.u31",  0x00000,  0x8000,   CRC(d1b91acf) SHA1(5ae3449d83b35ba5b20f7ff60eba4359f29cb744) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat5tg ) /* Dallas DS1204V security key at U5 labeled 9255-70-50 U5-C-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-70-50_u32-r0",  0x000000, 0x100000, CRC(f57e4d36) SHA1(c16587c95fa1abe2e7df37027deb2cfbadb27038) )
	ROM_LOAD( "qs9255-05_u36-r0",   0x100000, 0x80000,  CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-05_u37-r0",   0x200000, 0x80000,  CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-70-50_u38-r0d", 0x300000, 0x100000, CRC(044d123f) SHA1(d73df1f97f6da03fdee2ca3fda3845ec262a0f9a) ) /* Location U38, 10/29/1997 10:19:08 - Bi-Lingual GER/ENG Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-70-50_u5-c-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(1888e4f2) SHA1(045b94d6600345c10098cae321811717071c4902) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt5tg_ds1644.u31",  0x00000,  0x8000,   CRC(a054bb32) SHA1(4efc19cb0a671dfe9249ce85d31f6bd633f2a237) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat6 ) /* Dallas DS1204V security key at U5 labeled 9255-80 U5-B-RO1 C1998 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-80-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) ) /* Location U32 */
	ROM_LOAD( "qs9255-08_u36-r0",   0x100000, 0x080000, CRC(800f5a1f) SHA1(4d3ee6fb896d6452aab1f279a3ee878284bd1acc) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-08_u37-r0",   0x200000, 0x100000, CRC(5ba01949) SHA1(1598949ea18d07bbc78af0ddd279a687173c1229) ) /* Location U37 */
	ROM_LOAD( "9255-80-01_u38-r0a", 0x300000, 0x100000, CRC(3df6b840) SHA1(31ba1ac04eed3e76cdf637507dedcc5f7e22c919) ) /* Location U38, 08/07/1998 15:54:23 - Standard Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-80_u5-b-r01_c1998_mii", 0x000000, 0x000022, BAD_DUMP CRC(975099f5) SHA1(bdb870b5d3aa1139320b426f8669418cf85a513e) )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1230 nv ram
	ROM_LOAD( "ds1230y.u31",  0x00000, 0x8000, CRC(51b6da5c) SHA1(1d53af89d7867bb48b9d46feff6fc3b7e8e80ac8) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

/* Sold as a 3 program ROM + Security chip update - Header touts new Trivia Whiz and Phraze Craze */
ROM_START( megat7e ) /* Dallas DS1204V security key at U5 labeled 9255-90 U5-B-RO1 C2000 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-90-01_u32-r00", 0x000000, 0x100000, CRC(3b9cecc7) SHA1(677ca2df35aeb708a0ad816e91c3e62f2cd273da) ) /* Location U32 */
	ROM_LOAD( "qs9255-08_u36-r0",   0x100000, 0x080000, CRC(800f5a1f) SHA1(4d3ee6fb896d6452aab1f279a3ee878284bd1acc) ) /* Location U36 - Not used? */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-09_u37-r00",  0x200000, 0x080000, CRC(f6954331) SHA1(3fcfa5fef54d559e32cdecea726afce4e20b1744) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x080000)
	ROM_LOAD( "9255-90-01_u38-r00", 0x300000, 0x100000, CRC(b4f94ff9) SHA1(d74334bcd913f269a8e5744f590e8943ec0b96b6) ) /* Location U38, 10/16/2000 15:09:35 - Standard Version */

	ROM_REGION( 0x000022, "ds1204", 0 )
	ROM_LOAD( "9255-90_u5-b-r01_c2000_mii", 0x000000, 0x000022, BAD_DUMP CRC(c1179dbc) SHA1(739bd89bd1ec5971bf1cfb4d77bc5411f0f51a70) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

void meritm_state::init_megat3te()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfff8, 0xffff, read8sm_delegate(*this, FUNC(meritm_state::ds1644_r)), write8sm_delegate(*this, FUNC(meritm_state::ds1644_w)));
}

} // anonymous namespace


/* CRT-250 */
GAME( 1987, americna,  0,        crt250, americna,  meritm_state, empty_init, ROT0, "Merit", "Americana (9131-01)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1987, americnaa, americna, crt250, americna,  meritm_state, empty_init, ROT0, "Merit", "Americana (9131-00)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, meritjp,   0,        crt250, americna,  meritm_state, empty_init, ROT0, "Merit", "Merit Joker Poker (9131-09)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, dodgecty,  0,        crt250, dodgecty,  meritm_state, empty_init, ROT0, "Merit", "Dodge City (9131-02, U9-2T)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, dodgectya, dodgecty, crt250, dodgecty,  meritm_state, empty_init, ROT0, "Merit", "Dodge City (9131-02, U9-2B)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, pitboss2,  0,        crt250, pitboss2,  meritm_state, empty_init, ROT0, "Merit", "Pit Boss II (9221-01C)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, spitboss,  0,        crt250, spitboss,  meritm_state, empty_init, ROT0, "Merit", "Super Pit Boss (9221-02A)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, pitbosss,  0,        crt250, pitbosss,  meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar (9221-10-00B)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, pitbosssa, pitbosss, crt250, pitbosss,  meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar (9221-10-00A)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, pitbosssc, pitbosss, crt250, pitbosss,  meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar (9221-12-01, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, pitbosssm, pitbosss, crt250, pitbosss,  meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar (9221-12-02, Minnesota version)", MACHINE_IMPERFECT_GRAPHICS )

/* CRT-250 + CRT-252 + CRT-256 + CRT-258 */
GAME( 1994, mtjpoker,  0,        crt250_crt252_crt258, mtjpoker,   meritm_state, empty_init, ROT0, "Merit", "Merit Touch Joker Poker (9132-00)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat,     0,        crt250_crt252_crt258, pbst30,     meritm_state, empty_init, ROT0, "Merit", "Pit Boss Megatouch (9234-20-01 R0A)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, pbst30,    0,        crt250_crt252_crt258, pbst30,     meritm_state, empty_init, ROT0, "Merit", "Pit Boss Supertouch 30 (9234-10-01 R0)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, pbst30a,   pbst30,   crt250_crt252_crt258, pbst30,     meritm_state, empty_init, ROT0, "Merit", "Pit Boss Supertouch 30 (9234-00-01 R0A)", MACHINE_IMPERFECT_GRAPHICS )

/* CRT-250 + CRT-254 + CRT-256 */
GAME( 1993, pbss330,   0,        crt250_questions,     pbss330,    meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar III 30 (9233-00-01 R0B, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, pbss330a,  pbss330,  crt250_questions,     pbss330,    meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar III 30 (9233-00-01 R0, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, pbss330ca, pbss330,  crt250_questions,     pbss330,    meritm_state, empty_init, ROT0, "Merit", "Pit Boss Superstar III 30 (9233-00-06 R0A, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, pitbossm,  0,        crt250_questions,     pitbossm,   meritm_state, empty_init, ROT0, "Merit", "Pit Boss Megastar (9244-00-01 R0B)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, pitbossma, pitbossm, crt250_questions,     pitbossm,   meritm_state, empty_init, ROT0, "Merit", "Pit Boss Megastar (9244-00-01 R0)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, pitbossmb, pitbossm, crt250_questions,     pitbossa,   meritm_state, empty_init, ROT0, "Merit", "Pit Boss Megastar (9243-00-01 R0)", MACHINE_IMPERFECT_GRAPHICS )

/* CRT-260 NON-touchscreen based */
GAME( 1996, realbrod,  0,        crt260, realbrod,    meritm_state, empty_init,    ROT0, "Merit", "The Real Broadway (9131-20-00 R0C)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, realbroda, realbrod, crt260, realbrod,    meritm_state, empty_init,    ROT0, "Merit", "The Real Broadway (9131-20-00 R0A)", MACHINE_IMPERFECT_GRAPHICS )

/* CRT-260 */
GAME( 1994, megat2,    0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-01 R0G, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat2a ,  megat2, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-01 R0E, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat2b ,  megat2, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-01 R0D, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat2mn,  megat2, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-02 R0G, Minnesota version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat2ca,  megat2, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-06 R0G, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, megat2caa, megat2, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Pit Boss Megatouch II (9255-10-06 R0E, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat3,    0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-01 R0N, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3a,   megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-01 R0K, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3b,   megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-01 R0F, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3c,   megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-01 R0B, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3d,   megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-01 R0A, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat3ca,  megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-06 R0N, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3caa, megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-06 R0D, California version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, megat3nj,  megat3, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch III (9255-20-07 R0G, New Jersey version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat3te,  megat3, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch III Tournament Edition (9255-30-01 R0E, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat3tg,  megat3, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch III Turnier Edition (9255-30-50 R0F, Bi-Lingual GER/ENG version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4,    0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch IV (9255-40-01 R0E, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4a,   megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch IV (9255-40-01 R0D, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4b,   megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch IV (9255-40-01 R0B, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4c,   megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch IV (9255-40-01 R0A, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4d,   megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch IV (9255-40-01 R0, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4s,   megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Super Megatouch IV (9255-41-01 R0G, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4sa,  megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Super Megatouch IV (9255-41-01 R0E, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4sb,  megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Super Megatouch IV (9255-41-01 R0C, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4smn, megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Super Megatouch IV (9255-41-02 R0C, Minnesota version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4snj, megat4, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Super Megatouch IV (9255-41-07 R0G, New Jersey version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4te,  megat4, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch IV Tournament Edition (9255-50-01 R0D, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4tea, megat4, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch IV Tournament Edition (9255-50-01 R0A, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4st,  megat4, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Super Megatouch IV Tournament Edition (9255-51-01 R0B, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, megat4stg, megat4, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Super Megatouch IV Turnier Version (9255-51-50 R0A, Bi-Lingual GER/ENG version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, megat5,    0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 5 (9255-60-01 R0I, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, megat5a,   megat5, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 5 (9255-60-01 R0C, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, megat5nj,  megat5, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 5 (9255-60-07 R0N, New Jersey version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, megat5g,   megat5, crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 5 (9255-60-50 R0G, Bi-Lingual GER/ENG version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, megat5t,   megat5, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch 5 Tournament Edition (9255-70-01 R0C, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, megat5tg,  megat5, crt260, meritm_crt260, meritm_state, init_megat3te, ROT0, "Merit", "Megatouch 5 Turnier Version (9255-70-50 R0D, Bi-Lingual GER/ENG version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, megat6,    0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 6 (9255-80-01 R0A, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, megat7e,   0,      crt260, meritm_crt260, meritm_state, empty_init,    ROT0, "Merit", "Megatouch 7 Encore Edition (9255-90-01 R00, Standard version)", MACHINE_IMPERFECT_GRAPHICS )
