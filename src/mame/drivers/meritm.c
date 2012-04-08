/*
  Merit CRT250 and CRT260 hardware

  Driver by Mariusz Wojcieszek

  CRT 250 (basic components, also used by CRT260)
   Main CPU: Z80B
      Sound: Yamaha YM2149F (or compatible)
      Video: 2 Yamaha V9938 (MSX2 video chip!)
      Other: 2 Z80APIO (I/O and interrupt controllers)
             1 8255 (I/O)

  CRT 256: addon board for CRT 250, stores question roms (aka Memory Expansion board)

  CRT 258: addon board for CRT 250, contains UART and Microtouch touch screen controller

  CRT 260 additional components:
  - Microtouch touch screen controller (SMT-3)
  - PC16550 UART (for communication with touch screen controller)
  - DS1204 Electronic Key (for protection)
  - DS1232 Reset and Watchdog
  - MAX232 (for MegaLink)

    One of the following Dallas Nonvolatile SRAM chips:
    - DS1225Y 64K Non-volitile SRAM (Mega Touch 4)
    - DS1230Y 256K Non-volitile SRAM (Mega Touch 6)
    - DS1644 32K NVRAM + RTC (Tournament sets)

  Known Games:

  CRT 250:
  Pit Boss II (c)1988
  Super Pit Boss (c)1988
  Pit Boss Superstar (c)1990
  *Pit Boss Superstar 30 (c)1993
  *Pit Boss Superstar III 30 (c)1993?
  Pit Boss Megastar (c)1994
  Pit Boss Supertouch 30 (c)1993/4

  CRT 260:
  *Megatouch Video (c)1994?
  Megatouch II (c)1994
  Megatouch III (c)1995
  Megatouch III Tournament Edition (c)1996
  Megatouch IV (c)1996
  Megatouch IV Tournament Edition (c)1996
  Super Megatouch IV (c) 1996  (rom labels 9255-41-0x, see below)
  Super Megatouch IV Tournament Edition (c) 1996
  Megatouch 5 (c)1997
  Megatouch 5 Tournament Edition (c)1997
  Megatouch 6 (c)1998
  *Megatouch 7 Encore (c)2000

  * indicates that game needs to be dumped or redumped

Custom Program Versions (from different Megatouch manuals):

PROGRAM#    Program Version      Program Differences
---------------------------------------------------------------------------------------------
9255-xx-01  Standard Version     Includes all Options, no Restrictions
9255-xx-02  Minnesota Version    Excludes Casino Games
9255-xx-03  Louisiana Version    Excludes all Poker Games
9255-xx-04  Wisconsin Version    Game Connot End if Player Busts; 1,000 Points are Added to End of Each Hand
9255-xx-06  California Version   Excludes Poker Double-up feature & No Free Game in Solitaire
9255-xx-07  New Jersey Version   Includes 2-Coin Limit with Lockout Coil
9255-xx-50  Bi-Lingual ENG/GER   Same as Standard Version, Without Word/Casino Games
9255-xx-54  Bi-Lingual ENG/SPA   Same as Standard Version, Without Word Games
9255-xx-56  No Free Credits      Same as Standard Version, Without Word Games and No Free Credits
9255-xx-57  Internation Version  Same as Standard Version, Without Word Games
9255-xx-60  Bi-Lingual ENG/FRE   Same as Standard Version, Without Word/Casino Games
9255-xx-62  No Free Credit       Same as Standard Version, With No Free Credit (see regional notes below)
9255-xx-62  Croatia              Same as Standard Version, With No Free Credit (see regional notes below)
9255-xx-70  Australia Version    Same as Standard Version with Special Question Set
9255-xx-71  South Africa Ver.    Same as Standard Version with Special Question Set

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

Not all regional versions are available for each Megatouch series
 For Megatouch 4,       set 9255-40-62 is Croatia
 For Megatouch Super 4, set 9255-41-62 is No Free Credit

  Notes/ToDo:
  - offset for top V9938 layer is hardcoded, probably should be taken from V9938 setup
  - blinking on Megatouch title screen is probably incorrect
  - clean up V9938 interrupt implementation
  - finish inputs, dsw, outputs (lamps)
  - problem with registering touches on the bottom of the screen (currently hacked to work)
  - megat5a: has jmp $0000 in the initialization code causing infinite loop, rom U38 is dumped at half size / bad dump
  - for pbst30 only roms were found, it appears that two roms with graphics data were missing, using pitbossm roms for now
 */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "machine/8255ppi.h"
#include "machine/z80pio.h"
#include "machine/pc16552d.h"
#include "machine/microtch.h"
#include "machine/nvram.h"


typedef struct
{
	int state;
	int read_ptr;
	int last_clk;
	UINT8 key[8];
	UINT8 nvram[16];
	int out_bit;
	UINT8 command[3];

} ds1204_t;

class meritm_state : public driver_device
{
public:
	meritm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_v9938_0(*this, "v9938_0"),
		  m_v9938_1(*this, "v9938_1"),
		  m_microtouch(*this, "microtouch") { }

	DECLARE_WRITE8_MEMBER(microtouch_tx);
	UINT8* m_ram;
	device_t *m_z80pio[2];
	int m_vint;
	int m_interrupt_vdp0_state;
	int m_interrupt_vdp1_state;
	int m_layer0_enabled;
	int m_layer1_enabled;
	int m_bank;
	int m_psd_a15;
	UINT16 m_questions_loword_address;
	ds1204_t m_ds1204;
	required_device<v9938_device> m_v9938_0;
	required_device<v9938_device> m_v9938_1;
	required_device<microtouch_device> m_microtouch;
	DECLARE_WRITE8_MEMBER(meritm_crt250_bank_w);
	DECLARE_WRITE8_MEMBER(meritm_psd_a15_w);
	DECLARE_WRITE8_MEMBER(meritm_bank_w);
	DECLARE_WRITE8_MEMBER(meritm_crt250_questions_lo_w);
	DECLARE_WRITE8_MEMBER(meritm_crt250_questions_hi_w);
	DECLARE_WRITE8_MEMBER(meritm_crt250_questions_bank_w);
	DECLARE_WRITE8_MEMBER(meritm_ds1644_w);
	DECLARE_READ8_MEMBER(meritm_ds1644_r);
};


#define SYSTEM_CLK	21470000
#define UART_CLK	XTAL_18_432MHz



/*************************************
 *
 *  DS1204 Electronic Key
 *
 *************************************/

#define DS1204_STATE_IDLE			0
#define DS1204_STATE_COMMAND			1
#define DS1204_STATE_READ_KEY			2
#define DS1204_STATE_WRITE_SECURITY_MATCH	3
#define DS1204_STATE_READ_NVRAM			4

static void ds1204_w( ds1204_t *ds1204, int rst, int clk, int dq )
{
	//logerror("ds1204_w: rst = %d, clk = %d, dq = %d\n", rst, clk, dq );
	if ( rst == 0 )
	{
		ds1204->state = DS1204_STATE_COMMAND;
		ds1204->read_ptr = 0;
	}
	else
	{
		if ( (ds1204->last_clk == 1) && (clk == 0) )
		{
			switch(ds1204->state)
			{
				case DS1204_STATE_COMMAND:
					//logerror("Command bit %d = %d\n", ds1204->read_ptr, dq);
					if ( ds1204->read_ptr < 24 )
					{
						if ( dq == 1 )
						{
							ds1204->command[ds1204->read_ptr >> 3] |= (1 << (ds1204->read_ptr & 0x7));
						}
						else
						{
							ds1204->command[ds1204->read_ptr >> 3] &= ~(1 << (ds1204->read_ptr & 0x7));
						}
						ds1204->read_ptr++;
					}
					if ( ds1204->read_ptr == 24 )
					{
						ds1204->state = DS1204_STATE_READ_KEY;
						ds1204->read_ptr = 0;
					}
					break;
				case DS1204_STATE_READ_KEY:
					//logerror("Key bit %d\n", ds1204->read_ptr);
					if (ds1204->read_ptr < 64)
					{
						ds1204->out_bit = (ds1204->key[ds1204->read_ptr >> 3] >> (ds1204->read_ptr & 0x7)) & 0x01;
						ds1204->read_ptr++;
					}
					if (ds1204->read_ptr == 64)
					{
						ds1204->state = DS1204_STATE_WRITE_SECURITY_MATCH;
						ds1204->read_ptr = 0;
					}
					break;
				case DS1204_STATE_WRITE_SECURITY_MATCH:
					//logerror( "Security match bit %d = %d\n", ds1204->read_ptr, dq);
					if (ds1204->read_ptr < 64)
					{
						ds1204->read_ptr++;
					}
					if (ds1204->read_ptr == 64)
					{
						ds1204->state = DS1204_STATE_READ_NVRAM;
						ds1204->read_ptr = 0;
					}
					break;
				case DS1204_STATE_READ_NVRAM:
					//logerror( "Read nvram bit = %d\n", ds1204->read_ptr );
					if (ds1204->read_ptr < 128)
					{
						ds1204->out_bit = (ds1204->nvram[ds1204->read_ptr >> 3] >> (ds1204->read_ptr & 0x7)) & 0x01;
						ds1204->read_ptr++;
					}
					if (ds1204->read_ptr == 128)
					{
						ds1204->state = DS1204_STATE_IDLE;
						ds1204->read_ptr = 0;
					}
					break;

			}
		}
		ds1204->last_clk = clk;
	}
};

static int ds1204_r(ds1204_t *ds1204)
{
	//logerror("ds1204_r\n");
	return ds1204->out_bit;
};

static void ds1204_init(running_machine &machine, const UINT8* key, const UINT8* nvram)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	memset(&state->m_ds1204, 0, sizeof(state->m_ds1204));
	if (key)
		memcpy(state->m_ds1204.key, key, sizeof(state->m_ds1204.key));
	if (nvram)
		memcpy(state->m_ds1204.nvram, nvram, sizeof(state->m_ds1204.nvram));

	state_save_register_item(machine, "ds1204", NULL, 0, state->m_ds1204.state);
	state_save_register_item(machine, "ds1204", NULL, 0, state->m_ds1204.read_ptr);
	state_save_register_item(machine, "ds1204", NULL, 0, state->m_ds1204.last_clk);
	state_save_register_item(machine, "ds1204", NULL, 0, state->m_ds1204.out_bit);
	state_save_register_item_array(machine, "ds1204", NULL, 0, state->m_ds1204.command);
};

/*************************************
 *
 *  Microtouch <-> pc16650 interface
 *
 *************************************/

static void pc16650d_tx_callback(running_machine &machine, int channel, int count, UINT8* data)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	for(int i = 0; i < count; i++)
		state->m_microtouch->rx(*machine.memory().first_space(), 0, data[i]);
}

WRITE8_MEMBER(meritm_state::microtouch_tx)
{
	pc16552d_rx_data(space.machine(), 0, 0, data);
}

/*************************************
 *
 *  Microtouch touch coordinate transformation
 *
 *************************************/
MICROTOUCH_TOUCH(meritm_touch_coord_transform)
{
	int xscr = (int)((double)(*touch_x)/0x4000*544);
	int yscr = (int)((double)(*touch_y)/0x4000*480);

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
		*touch_y = (int)((double)(yscr - 16)*0x4000/(480-16-63));
	}
	*touch_x = (int)((double)(xscr - 16)*0x4000/(544-16-16));

	return 1;
}

static const microtouch_interface meritm_microtouch_config =
{
	DEVCB_DRIVER_MEMBER(meritm_state, microtouch_tx),
	meritm_touch_coord_transform
};

/*************************************
 *
 *  Video
 *
 *************************************/


static TIMER_DEVICE_CALLBACK( meritm_interrupt )
{
	meritm_state *state = timer.machine().driver_data<meritm_state>();
	int scanline = param;

	if((scanline % 2) == 0)
	{
		state->m_v9938_0->set_sprite_limit(0);
		state->m_v9938_0->set_resolution(RENDER_HIGH);
		state->m_v9938_0->interrupt();

		state->m_v9938_1->set_sprite_limit(0);
		state->m_v9938_1->set_resolution(RENDER_HIGH);
		state->m_v9938_1->interrupt();
	}
}

static void meritm_vdp0_interrupt(device_t *, v99x8_device &device, int i)
{
	/* this is not used as the v9938 interrupt callbacks are broken
       interrupts seem to be fired quite randomly */
}

static void meritm_vdp1_interrupt(device_t *, v99x8_device &device, int i)
{
	/* this is not used as the v9938 interrupt callbacks are broken
       interrupts seem to be fired quite randomly */
}


static VIDEO_START( meritm )
{
	meritm_state *state = machine.driver_data<meritm_state>();
	state->m_layer0_enabled = state->m_layer1_enabled = 1;

	state->m_vint = 0x18;
	state_save_register_global(machine, state->m_vint);
	state_save_register_global(machine, state->m_interrupt_vdp0_state);
	state_save_register_global(machine, state->m_interrupt_vdp1_state);
}

static SCREEN_UPDATE_IND16( meritm )
{
	meritm_state *state = screen.machine().driver_data<meritm_state>();
	if(screen.machine().input().code_pressed_once(KEYCODE_Q))
	{
		state->m_layer0_enabled^=1;
		popmessage("Layer 0 %sabled",state->m_layer0_enabled ? "en" : "dis");
	}
	if(screen.machine().input().code_pressed_once(KEYCODE_W))
	{
		state->m_layer1_enabled^=1;
		popmessage("Layer 1 %sabled",state->m_layer1_enabled ? "en" : "dis");
	}

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if ( state->m_layer0_enabled )
	{
		copybitmap(bitmap, state->m_v9938_0->get_bitmap(), 0, 0, 0, 0, cliprect);
	}

	if ( state->m_layer1_enabled )
	{
		copybitmap_trans(bitmap, state->m_v9938_1->get_bitmap(), 0, 0, -6, -12, cliprect, state->m_v9938_1->get_transpen());
	}
	return 0;
}

/*************************************
 *
 *  Bank switching (ROM/RAM)
 *
 *************************************/


static void meritm_crt250_switch_banks( running_machine &machine )
{
	meritm_state *state = machine.driver_data<meritm_state>();
	int rombank = (state->m_bank & 0x07) ^ 0x07;

	//logerror( "CRT250: Switching banks: rom = %0x (bank = %x)\n", rombank, state->m_bank );
	memory_set_bank(machine, "bank1", rombank );
};

WRITE8_MEMBER(meritm_state::meritm_crt250_bank_w)
{
	meritm_crt250_switch_banks(machine());
};

static void meritm_switch_banks( running_machine &machine )
{
	meritm_state *state = machine.driver_data<meritm_state>();
	int rambank = (state->m_psd_a15 >> 2) & 0x3;
	int rombank = (((state->m_bank >> 3) & 0x3) << 5) |
			  (((state->m_psd_a15 >> 1) & 0x1) << 4) |
			  (((state->m_bank & 0x07) ^ 0x07) << 1) |
			  (state->m_psd_a15 & 0x1);

	//logerror( "Switching banks: rom = %0x (bank = %x), ram = %0x\n", rombank, state->m_bank, rambank);
	memory_set_bank(machine, "bank1", rombank );
	memory_set_bank(machine, "bank2", rombank | 0x01);
	memory_set_bank(machine, "bank3", rambank);
};

WRITE8_MEMBER(meritm_state::meritm_psd_a15_w)
{
	m_psd_a15 = data;
	//logerror( "Writing PSD_A15 with %02x at PC=%04X\n", data, cpu_get_pc(&space.device()) );
	meritm_switch_banks(machine());
};

WRITE8_MEMBER(meritm_state::meritm_bank_w)
{
	meritm_switch_banks(machine());
};

/*************************************
 *
 *  CRT250 question roms reading
 *
 *************************************/


WRITE8_MEMBER(meritm_state::meritm_crt250_questions_lo_w)
{
	m_questions_loword_address &= 0xff00;
	m_questions_loword_address |= data;
};

WRITE8_MEMBER(meritm_state::meritm_crt250_questions_hi_w)
{
	m_questions_loword_address &= 0x00ff;
	m_questions_loword_address |= (data << 8);
};

WRITE8_MEMBER(meritm_state::meritm_crt250_questions_bank_w)
{
	UINT32 questions_address;
	UINT8 *dst;

	if ((m_bank & 0x07) != 0)
	{
		logerror("meritm_crt250_questions_bank_w: bank is %d\n", m_bank);
		return;
	}

	dst = machine().region("maincpu")->base() + 0x70000 + 2;

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
			default: logerror( "meritm_crt250_questions_bank_w: unknown data = %02x\n", data ); return;
		}
		logerror( "Reading question byte at %06X\n", questions_address | m_questions_loword_address);
		*dst = machine().region("extra")->base()[questions_address | m_questions_loword_address];
	}
};


/*************************************
 *
 *  DS1644 RTC
 *
 *************************************/

WRITE8_MEMBER(meritm_state::meritm_ds1644_w)
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
};

static UINT8 binary_to_BCD(UINT8 data)
{
	data %= 100;

	return ((data / 10) << 4) | (data %10);
}

READ8_MEMBER(meritm_state::meritm_ds1644_r)
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
};

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( meritm_crt250_map, AS_PROGRAM, 8, meritm_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_crt250_questions_map, AS_PROGRAM, 8, meritm_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0x0000, 0x0000) AM_WRITE(meritm_crt250_questions_lo_w)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(meritm_crt250_questions_hi_w)
	AM_RANGE(0x0002, 0x0002) AM_WRITE(meritm_crt250_questions_bank_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_crt250_io_map, AS_IO, 8, meritm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("v9938_0", v9938_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("v9938_1", v9938_device, read, write)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE_LEGACY("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE_LEGACY("z80pio_0", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE_LEGACY("z80pio_1", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x80, 0x80) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(meritm_crt250_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_crt250_crt258_io_map, AS_IO, 8, meritm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("v9938_0", v9938_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("v9938_1", v9938_device, read, write)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE_LEGACY("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE_LEGACY("z80pio_0", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE_LEGACY("z80pio_1", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x60, 0x67) AM_READWRITE_LEGACY(pc16552d_0_r,pc16552d_0_w)
	AM_RANGE(0x80, 0x80) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(meritm_crt250_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_map, AS_PROGRAM, 8, meritm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xdfff) AM_ROMBANK("bank2")
	AM_RANGE(0xe000, 0xffff) AM_RAMBANK("bank3") AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_io_map, AS_IO, 8, meritm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(meritm_psd_a15_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("v9938_0", v9938_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("v9938_1", v9938_device, read, write)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE_LEGACY("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE_LEGACY("z80pio_0", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE_LEGACY("z80pio_1", z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0x60, 0x67) AM_READWRITE_LEGACY(pc16552d_0_r,pc16552d_0_w)
	AM_RANGE(0x80, 0x80) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(meritm_bank_w)
ADDRESS_MAP_END

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

	PORT_START("DSW")	/* need for AY-8910 accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(meritm_crt250)
	PORT_START("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIO1_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DSW")	/* need for AY-8910 accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(pitbossm)
	PORT_INCLUDE(meritm_crt250)

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW1:1" )				/* Unused */
	PORT_DIPNAME( 0x02, 0x02, "Solitaire Timer Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Sex Trivia" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Limit" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "No Coin Limit" )
	PORT_DIPSETTING(    0x08, "4 Coin Limit" )	/* With Lockout coil */
	PORT_DIPNAME( 0x10, 0x10, "Run 21 and Trivia Whiz 2000: Coins to start" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "2 Coins" )
	PORT_DIPSETTING(    0x10, "1 Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Great Solitaire: Coins to start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "2 Coins" )
	PORT_DIPSETTING(    0x20, "1 Coin" )
	PORT_DIPNAME( 0x40, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:7")	/* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Sync Adjustment (Set by factory)" ) PORT_DIPLOCATION("SW1:8")	/* Sync Adjustment (Set by factory) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(pitbossa)
	PORT_INCLUDE(pitbossm)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, "Great Solitaire: Coins to start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "4 Coins" )
	PORT_DIPSETTING(    0x20, "2 Coins" )
INPUT_PORTS_END

static INPUT_PORTS_START(pbst30)
	PORT_INCLUDE(meritm_crt260)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Invert Touch Screen Coordinates" ) PORT_DIPLOCATION("SW1:1") /* In case you installed the touch screen upside down??? */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Casino Games" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Sexy Trivia Category" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4") /* Likely Coin limit */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  8255
 *
 *************************************/

static READ8_DEVICE_HANDLER(meritm_8255_port_c_r)
{
	//logerror( "8255 port C read\n" );
	return 0xff;
};

static WRITE8_DEVICE_HANDLER(meritm_crt250_port_b_w)
{
	//popmessage("Lamps: %d %d %d %d %d %d %d", BIT(data,0), BIT(data,1), BIT(data,2), BIT(data,3), BIT(data,4), BIT(data,5), BIT(data,6) );
	output_set_value("P1 DISC 1 LAMP", !BIT(data,0));
	output_set_value("P1 DISC 2 LAMP", !BIT(data,1));
	output_set_value("P1 DISC 3 LAMP", !BIT(data,2));
	output_set_value("P1 DISC 4 LAMP", !BIT(data,3));
	output_set_value("P1 DISC 5 LAMP", !BIT(data,4));
	output_set_value("P1 PLAY LAMP", !BIT(data,5));
	output_set_value("P1 CANCEL LAMP", !BIT(data,6));
}

static const ppi8255_interface crt260_ppi8255_intf =
{
	DEVCB_NULL,								/* Port A read */
	DEVCB_NULL,								/* Port B read */
	DEVCB_HANDLER(meritm_8255_port_c_r),	/* Port C read */
	DEVCB_NULL,								/* Port A write (used) */
	DEVCB_NULL,								/* Port B write (used LMP x DRIVE) */
	DEVCB_NULL								/* Port C write */
};

static const ppi8255_interface crt250_ppi8255_intf =
{
	DEVCB_NULL,								/* Port A read */
	DEVCB_NULL,								/* Port B read */
	DEVCB_HANDLER(meritm_8255_port_c_r),	/* Port C read */
	DEVCB_NULL,								/* Port A write (used) */
	DEVCB_HANDLER(meritm_crt250_port_b_w),	/* Port B write (used LMP x DRIVE) */
	DEVCB_NULL								/* Port C write */
};

/*************************************
 *
 *  AY8930
 *
 *************************************/

/*
 Port A: DSW
 Port B: Bits 0,1 used
*/

static WRITE8_DEVICE_HANDLER(meritm_ay8930_port_b_w)
{
	// lamps
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW"), /* Port A read */
	DEVCB_NULL, /* Port B read */
	DEVCB_NULL, /* Port A write */
	DEVCB_HANDLER(meritm_ay8930_port_b_w)  /* Port B write */
};

/*************************************
 *
 *  PIOs
 *
 *************************************/

static READ8_DEVICE_HANDLER(meritm_audio_pio_port_a_r)
{
	meritm_state *state = device->machine().driver_data<meritm_state>();
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

	return state->m_vint;
};

static READ8_DEVICE_HANDLER(meritm_audio_pio_port_b_r)
{
	meritm_state *state = device->machine().driver_data<meritm_state>();
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

	return ds1204_r(&state->m_ds1204);
};

static WRITE8_DEVICE_HANDLER(meritm_audio_pio_port_a_w)
{
	meritm_state *state = device->machine().driver_data<meritm_state>();
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

	state->m_bank = (data & 7) | ((data >> 2) & 0x18);
	//logerror("Writing BANK with %x (raw = %x)\n", state->m_bank, data);
};

static WRITE8_DEVICE_HANDLER(meritm_audio_pio_port_b_w)
{
	meritm_state *drvstate = device->machine().driver_data<meritm_state>();
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

	ds1204_w(&drvstate->m_ds1204, (data & 0x4) >> 2, (data & 0x2) >> 1, data & 0x01);
};

static WRITE8_DEVICE_HANDLER(meritm_io_pio_port_a_w)
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
};

static WRITE8_DEVICE_HANDLER(meritm_io_pio_port_b_w)
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
};

static Z80PIO_INTERFACE( meritm_audio_pio_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_HANDLER(meritm_audio_pio_port_a_r),
	DEVCB_HANDLER(meritm_audio_pio_port_a_w),
	DEVCB_NULL,
	DEVCB_HANDLER(meritm_audio_pio_port_b_r),
	DEVCB_HANDLER(meritm_audio_pio_port_b_w),
	DEVCB_NULL
};

static Z80PIO_INTERFACE( meritm_io_pio_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_INPUT_PORT("PIO1_PORTA"),
	DEVCB_HANDLER(meritm_io_pio_port_a_w),
	DEVCB_NULL,
	DEVCB_INPUT_PORT("PIO1_PORTB"),
	DEVCB_HANDLER(meritm_io_pio_port_b_w),
	DEVCB_NULL
};

static const z80_daisy_config meritm_daisy_chain[] =
{
	{ "z80pio_0" },
	{ "z80pio_1" },
	{ NULL }
};

static MACHINE_START(merit_common)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	state->m_z80pio[0] = machine.device( "z80pio_0" );
	state->m_z80pio[1] = machine.device( "z80pio_1" );

	z80pio_astb_w(state->m_z80pio[0], 1);
	z80pio_bstb_w(state->m_z80pio[0], 1);
	z80pio_astb_w(state->m_z80pio[1], 1);
	z80pio_bstb_w(state->m_z80pio[1], 1);
};

static MACHINE_START(meritm_crt250)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	memory_configure_bank(machine, "bank1", 0, 8, machine.region("maincpu")->base(), 0x10000);
	state->m_bank = 0xff;
	meritm_crt250_switch_banks(machine);
	MACHINE_START_CALL(merit_common);
	state_save_register_global(machine, state->m_bank);

};

static MACHINE_START(meritm_crt250_questions)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	MACHINE_START_CALL(meritm_crt250);
	state_save_register_global(machine, state->m_questions_loword_address);
};

static MACHINE_START(meritm_crt250_crt252_crt258)
{
	MACHINE_START_CALL(meritm_crt250_questions);
	pc16552d_init(machine, 0, UART_CLK, NULL, pc16650d_tx_callback);
}

static MACHINE_START(meritm_crt260)
{
	meritm_state *state = machine.driver_data<meritm_state>();
	state->m_ram = auto_alloc_array(machine, UINT8,  0x8000 );
	machine.device<nvram_device>("nvram")->set_base(state->m_ram, 0x8000);
	memset(state->m_ram, 0x00, 0x8000);
	memory_configure_bank(machine, "bank1", 0, 128, machine.region("maincpu")->base(), 0x8000);
	memory_configure_bank(machine, "bank2", 0, 128, machine.region("maincpu")->base(), 0x8000);
	memory_configure_bank(machine, "bank3", 0, 4, state->m_ram, 0x2000);
	state->m_bank = 0xff;
	state->m_psd_a15 = 0;
	meritm_switch_banks(machine);
	MACHINE_START_CALL(merit_common);
	pc16552d_init(machine, 0, UART_CLK, NULL, pc16650d_tx_callback);
	state_save_register_global(machine, state->m_bank);
	state_save_register_global(machine, state->m_psd_a15);
	state_save_register_global_pointer(machine, state->m_ram, 0x8000);
};

// from MSX2 driver, may be not accurate for merit games
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2

static TIMER_DEVICE_CALLBACK( vblank_start_tick )
{
	meritm_state *state = timer.machine().driver_data<meritm_state>();
	/* this is a workaround to signal the v9938 vblank interrupt correctly */
	state->m_vint = 0x08;
	z80pio_pa_w(state->m_z80pio[0], 0, state->m_vint);
}

static TIMER_DEVICE_CALLBACK( vblank_end_tick )
{
	meritm_state *state = timer.machine().driver_data<meritm_state>();
	/* this is a workaround to signal the v9938 vblank interrupt correctly */
	state->m_vint = 0x18;
	z80pio_pa_w(state->m_z80pio[0], 0, state->m_vint);
}

static MACHINE_CONFIG_START( meritm_crt250, meritm_state )
	MCFG_CPU_ADD("maincpu", Z80, SYSTEM_CLK/6)
	MCFG_CPU_PROGRAM_MAP(meritm_crt250_map)
	MCFG_CPU_IO_MAP(meritm_crt250_io_map)
	MCFG_CPU_CONFIG(meritm_daisy_chain)
	MCFG_TIMER_ADD_SCANLINE("scantimer", meritm_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START(meritm_crt250)

	MCFG_PPI8255_ADD( "ppi8255", crt250_ppi8255_intf )

	MCFG_Z80PIO_ADD( "z80pio_0", SYSTEM_CLK/6, meritm_audio_pio_intf )
	MCFG_Z80PIO_ADD( "z80pio_1", SYSTEM_CLK/6, meritm_io_pio_intf )

	MCFG_TIMER_ADD_SCANLINE("vblank_start", vblank_start_tick, "screen", 259, 262)
	MCFG_TIMER_ADD_SCANLINE("vblank_end",   vblank_end_tick,   "screen", 262, 262)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_V9938_ADD("v9938_0", "screen", 0x20000)
	MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(meritm_vdp0_interrupt)

	MCFG_V9938_ADD("v9938_1", "screen", 0x20000)
	MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(meritm_vdp1_interrupt)

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MCFG_SCREEN_UPDATE_STATIC(meritm)
	MCFG_PALETTE_LENGTH(512)

	MCFG_PALETTE_INIT( v9938 )

	MCFG_VIDEO_START(meritm)

  /* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, SYSTEM_CLK/12)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_MICROTOUCH_ADD("microtouch", meritm_microtouch_config)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( meritm_crt250_questions, meritm_crt250 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(meritm_crt250_questions_map)
	MCFG_MACHINE_START(meritm_crt250_questions)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( meritm_crt250_crt252_crt258, meritm_crt250_questions )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(meritm_crt250_crt258_io_map)
	MCFG_MACHINE_START(meritm_crt250_crt252_crt258)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( meritm_crt260, meritm_crt250 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(meritm_map)
	MCFG_CPU_IO_MAP(meritm_io_map)

	MCFG_PPI8255_RECONFIG( "ppi8255", crt260_ppi8255_intf )

	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(1200))	// DS1232, TD connected to VCC
	MCFG_MACHINE_START(meritm_crt260)

MACHINE_CONFIG_END


/*
    Pit Boss II - Merit Industries Inc. 1988
    ----------------------------------------

    All eproms are 27C512

    One 8 bank dip switch.

    Two YAMAHA V9938 Video Processors.

    21.47727 MHz Crystal

    CPU Z80

    Audio AY8930

    Two Z80A-PIO

    One bq4010YMA-150 NVRAM
    Eight V53C464AP80 (41464) RAMS

    One PAL16L8AN
    One PAL20L10NC
*/

ROM_START( pitboss2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u9",  0x00000, 0x10000, CRC(a1b6ac15) SHA1(b7b395f3e7e14dbb84003e03bf7d054e795a7211) )
	ROM_LOAD( "u10", 0x10000, 0x10000, CRC(207aa83c) SHA1(1955d75b9e561312e98831571c9853579ded3734) )
	ROM_LOAD( "u11", 0x20000, 0x10000, CRC(2052e043) SHA1(36b6cbc5712fc736c748a68bd12675291eae669d) )
	ROM_LOAD( "u12", 0x30000, 0x10000, CRC(33653f16) SHA1(57b9822499324502d66dc5a40e662596e5336943) )
	ROM_LOAD( "u13", 0x40000, 0x10000, CRC(4f139e88) SHA1(425dd34804cc614aa93a468d2ba3e16de62f099c) )
	ROM_LOAD( "u14", 0x50000, 0x10000, CRC(a58078cd) SHA1(a028be67fa05670a689144dfb9c9da51c5732389) )
	ROM_LOAD( "u15", 0x60000, 0x10000, CRC(239b5d03) SHA1(fffb69cd7af215445da2b1281bcbc5f4fb6cfcc3) )
	ROM_LOAD( "u16", 0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
ROM_END

ROM_START( pitbosss )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-10-0b.u9",  0x00000, 0x10000, CRC(e1fbf7cb) SHA1(e04163219c357cd3da2a78ba2590d453df8e9477) )
	ROM_LOAD( "9221-10-0.u10",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-10-0b.u11", 0x20000, 0x10000, CRC(6d6dfaf3) SHA1(de76c577eef1bb6637aacffedcc40266af92506e) )
	ROM_LOAD( "9221-10-0.u12",  0x30000, 0x10000, CRC(3577a203) SHA1(80f9c827ad9dea2c6af788bd3b46ab65e8c594eb) )
	ROM_LOAD( "9221-10-0.u13",  0x40000, 0x10000, CRC(466f81f9) SHA1(88429d9ff53d27bf639200852a7bf61768c8fd1b) )
	ROM_LOAD( "9221-10-0.u14",  0x50000, 0x10000, CRC(0720faa6) SHA1(1d78d711e3aab1ecf604ae7b9c374d27639a97c3) )
	ROM_LOAD( "9221-10-0.u15",  0x60000, 0x10000, CRC(c302b4c2) SHA1(d62d4bb33a9ccb95d1e550f9e439be3316b94c99) )
	ROM_LOAD( "9221-10-0.u16",  0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
ROM_END

ROM_START( pitbosssa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9221-10-0a.u9",  0x00000, 0x10000, CRC(41be6b30) SHA1(c4df87a599e310ce29ee9277e5adc916ff68f060) )
	ROM_LOAD( "9221-10-0.u10",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9221-10-0a.u11", 0x20000, 0x10000, CRC(c9137469) SHA1(618680609bdffa92b919a2417bd3ec41a4c8bf2b) )
	ROM_LOAD( "9221-10-0.u12",  0x30000, 0x10000, CRC(3577a203) SHA1(80f9c827ad9dea2c6af788bd3b46ab65e8c594eb) )
	ROM_LOAD( "9221-10-0.u13",  0x40000, 0x10000, CRC(466f81f9) SHA1(88429d9ff53d27bf639200852a7bf61768c8fd1b) )
	ROM_LOAD( "9221-10-0.u14a", 0x50000, 0x10000, CRC(0a928852) SHA1(c6c623f63a73b3de6298f436a4ca339c1447888d) )
	ROM_LOAD( "9221-10-0.u15",  0x60000, 0x10000, CRC(c302b4c2) SHA1(d62d4bb33a9ccb95d1e550f9e439be3316b94c99) )
	ROM_LOAD( "9221-10-0.u16",  0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
ROM_END

ROM_START( spitboss )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u9-0a.rom",  0x00000, 0x10000, CRC(e0c45c9c) SHA1(534bff67c8fee08f1c348275de8977659efa9f69) )
	ROM_LOAD( "u10.rom",    0x10000, 0x10000, CRC(ed010c58) SHA1(02750944a28c1c27ce2a9904d11b7e46272a940e) )
	ROM_LOAD( "u11-0a.rom", 0x20000, 0x10000, CRC(0c65fa86) SHA1(7906a8d615116ca67bf370dfb2da8cb2389a313d) )
	ROM_LOAD( "u12.rom",    0x30000, 0x10000, CRC(0cf95b0e) SHA1(c6ffc13703892b9ae0da39a02db37c4ec890f79e) )
	ROM_LOAD( "u13",        0x40000, 0x10000, CRC(4f139e88) SHA1(425dd34804cc614aa93a468d2ba3e16de62f099c) ) // matches pitboss2
	ROM_LOAD( "u14",        0x50000, 0x10000, CRC(a58078cd) SHA1(a028be67fa05670a689144dfb9c9da51c5732389) ) // matches pitboss2
	ROM_LOAD( "u15",        0x60000, 0x10000, CRC(239b5d03) SHA1(fffb69cd7af215445da2b1281bcbc5f4fb6cfcc3) ) // matches pitboss2
	ROM_LOAD( "u16",        0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // matches pitboss2
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
3- An additional Solitaire Instruction decal has beed added to the kit.
   This new Instruction decal is to be mounted in a visivle loction for
   players use.

*/

ROM_START( pitbossm ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9244-00 U1-RO1 C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9244-00-01_u9-r0",  0x00000, 0x10000, CRC(8317fea1) SHA1(eb84fdca7cd51883153561785571790d12d0d612) )
	ROM_LOAD( "9244-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Could also be labeled 9234-00-01 U10-R0 */
	ROM_LOAD( "9244-00-01_u11-r0", 0x20000, 0x10000, CRC(45223e0d) SHA1(45070e85d87aa67ecd6a1355212f1d24142fcbd0) )
	ROM_LOAD( "9244-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Could also be labeled 9234-00-01 U12-R0 */
	ROM_LOAD( "9244-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Could also be labeled 9234-00-01 U13-R0 */
	ROM_LOAD( "9244-00-01_u14-r0", 0x50000, 0x10000, CRC(c0d18911) SHA1(def939c6bac1e3124197f3f783d06f3bef3d03e9) )
	ROM_LOAD( "9244-00-01_u15-r0", 0x60000, 0x10000, CRC(740e3734) SHA1(6440d258af114f3820683b4e6fba5db6aea02231) )
	ROM_RELOAD(     0x70000, 0x10000)


	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9243-00-01_u7-r0",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) ) /* These 3 roms are on CRT-256 sattalite PCB */
	ROM_LOAD( "qs9243-00-01_u6-r0",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "qs9243-00-01_u5-r0",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

ROM_START( pitbossma ) /* Unprotected or patched??  The manual shows a DS1204 key for this set */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9243-00-01_u9-r0",  0x00000, 0x10000, CRC(55e14fb1) SHA1(ec29764d1b63360f64b82452e0db8054b99fcca0) )
	ROM_LOAD( "9243-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) ) /* Could also be labeled 9234-00-01 U10-R0 */
	ROM_LOAD( "9243-00-01_u11-r0", 0x20000, 0x10000, CRC(47a9dfc7) SHA1(eca100003f5605bcf405f610a0458ccb67894d35) )
	ROM_LOAD( "9243-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) ) /* Could also be labeled 9234-00-01 U12-R0 */
	ROM_LOAD( "9243-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) /* Could also be labeled 9234-00-01 U13-R0 */
	ROM_RELOAD(     0x50000, 0x10000) /* U14 is unused for this set */
	ROM_LOAD( "9243-00-01_u15-r0", 0x60000, 0x10000, CRC(27034061) SHA1(cff6be592a4a3ab01c204b081470f224e6186c4d) )
	ROM_RELOAD(     0x70000, 0x10000)


	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9243-00-01_u7-r0",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) ) /* These 3 roms are on CRT-256 sattalite PCB */
	ROM_LOAD( "qs9243-00-01_u6-r0",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "qs9243-00-01_u5-r0",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

ROM_START( pbst30 ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9234-10 U1-RO1 C1994 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9234-10-01_u9-r0",  0x00000, 0x10000, CRC(96f39c9a) SHA1(df698e94a5204cf050ceadc5c257ca5f68171114) )
	ROM_LOAD( "9234-00-01_u10-r0", 0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9234-10-01_u11-r0", 0x20000, 0x10000, CRC(835fa041) SHA1(2ae754c5fcf50548eb214902409217d1643c6eaa) )
	ROM_LOAD( "9234-00-01_u12-r0", 0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9234-00-01_u13-r0", 0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
	ROM_LOAD( "9234-10-01_u14-r0", 0x50000, 0x10000, CRC(9b0873a4) SHA1(7362c6220aa4bf1a9ab7c11cb8a51587a2a0a992) )
	ROM_LOAD( "9234-10-01_u15-r0", 0x60000, 0x10000, CRC(9fbd8582) SHA1(c0f68c8a7cdca34c8736cefc71767c421bcaba8a) )


	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9234-01_u7-r0",  0x00000, 0x40000, CRC(c0534aaa) SHA1(4b3cbf03f29fd5b4b8fd423e73c0c8147692fa75) ) /* These 3 roms are on CRT-256 sattalite PCB */
	ROM_LOAD( "qs9234-01_u6-r0",  0x40000, 0x40000, CRC(fe2cd934) SHA1(623011dc53ed6eefefa0725dba6fd1efee2077c1) )
	ROM_LOAD( "qs9234-01_u5-r0",  0x80000, 0x40000, CRC(293fe305) SHA1(8a551ae8fb4fa4bf329128be1bfd6f1c3ff5a366) )
ROM_END

ROM_START( pbst30b ) /* Dallas DS1204V security key attached to CRT-254 connected to J2 connector labeled 9234-01 U1-RO1 C1993 MII */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9234-00-01_u9-r0a",  0x00000, 0x10000, CRC(5f058f95) SHA1(98382935340a076bdb1b20c7f16c25b6084599fe) )
	ROM_LOAD( "9234-00-01_u10-r0",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "9234-00-01_u11-r0a", 0x20000, 0x10000, CRC(79125fb5) SHA1(6ca4f33c363cfb6f5c0f23b8fcc8cfcc076f68b1) )
	ROM_LOAD( "9234-00-01_u12-r0",  0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "9234-00-01_u13-r0",  0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
	ROM_LOAD( "9234-00-01_u14-r0a", 0x50000, 0x10000, CRC(e83f91d5) SHA1(1d64c943787b239763f44be412ee7f5ad13eb37d) )
	ROM_LOAD( "9234-00-01_u15-r0a", 0x60000, 0x10000, CRC(f10f0d39) SHA1(2b5d5a93adb5251e09160b10c067b6e70289f608) )


	ROM_REGION( 0xc0000, "extra", 0 ) // question roms
	ROM_LOAD( "qs9234-01_u7-r0",  0x00000, 0x40000, CRC(c0534aaa) SHA1(4b3cbf03f29fd5b4b8fd423e73c0c8147692fa75) ) /* These 3 roms are on CRT-256 sattalite PCB */
	ROM_LOAD( "qs9234-01_u6-r0",  0x40000, 0x40000, CRC(fe2cd934) SHA1(623011dc53ed6eefefa0725dba6fd1efee2077c1) )
	ROM_LOAD( "qs9234-01_u5-r0",  0x80000, 0x40000, CRC(293fe305) SHA1(8a551ae8fb4fa4bf329128be1bfd6f1c3ff5a366) )
ROM_END


/*
    Mega Touch
    by Merit Industries

    CTR-260 PB10004

    System Info
    -----------
     This is a counter top Touch screen game.

    processor.. Z80 (Z0840006PSC)
    sound chip Yamaha YM2149F
    other chips- two Yamaha V9938
             one LM1203
             one PC16550DN
             one PB255a or L5220574
             One Dallas DS1204 Data Key
             One Dallas DS1225Y 64k Non-volitile SRAM (Mega Touch 4)
              or Dallas DS1230Y 256K Non-volitile SRAM (Mega Touch 6)
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
	ROM_LOAD( "9255-10-01_u38-r0e",    0x300000, 0x080000, CRC(797fbbaf) SHA1(8d093374f109831e469133aaebc3f7c2a5ed0623) ) /* Location U38, 11/29/1994 10:51:00 - Standard version */
	ROM_RELOAD(                        0x380000, 0x080000)
ROM_END

ROM_START( megat2a ) /* Dallas DS1204U-3 security key labeled 9255-10-01-U5-R0 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "qs9235-01_u5oru32-r0a", 0x000000, 0x080000, CRC(ec0c18f6) SHA1(ae4f60f516097607249dbd902f8aacfe95acb065) ) /* Location U32 */
	ROM_RELOAD(                        0x080000, 0x080000)
	ROM_LOAD( "qs9235-01_u6oru36-r0",  0x100000, 0x080000, CRC(0a358743) SHA1(cc7c1b75e391204a7bdae2e1cecd9b55b572f8d5) ) /* Location U36 */
	ROM_RELOAD(                        0x180000, 0x080000)
	ROM_LOAD( "qs9235-01_u7oru37-r0",  0x200000, 0x080000, CRC(16643f83) SHA1(347af99f535a8b473c8780067d5132add7fa0d8c) ) /* Location U37 */
	ROM_RELOAD(                        0x280000, 0x080000)
	ROM_LOAD( "9255-10-01_u38-r0d",    0x300000, 0x080000, CRC(f43de55f) SHA1(456b4098e22982d5f1c6f872684eefb473939747) ) /* Location U38, 941123 514 - Standard version */
	ROM_RELOAD(                        0x380000, 0x080000)
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
	ROM_LOAD( "9255-20-01_u38-r0f", 0x300000, 0x080000, CRC(85f48b91) SHA1(7a38644ac7ee55a254c037122af919fb268744a1) ) /* Location U38, 10/27/1995 14:23:00 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000)

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

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat3cb ) /* Dallas DS1204V security key at U5 labeled 9255-20-01 U5-RO1 C1995 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-20-01_u32-r0d", 0x000000, 0x080000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) ) /* Location U32 */
	ROM_RELOAD(                     0x080000, 0x080000)
	ROM_LOAD( "qs9255-01_u36-r0",   0x100000, 0x080000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000)
	ROM_LOAD( "qs9255-01_u37-r0",   0x200000, 0x100000, CRC(273560bd) SHA1(5de8b9f5a7c4b676f131dd7d47ec71d35fa1755c) ) /* Location U37 */
	ROM_LOAD( "9255-20-06_u38-r0d", 0x300000, 0x080000, CRC(c40b3a57) SHA1(7a13172b94188c5cba32622016a05eb904714a86) ) /* Location U38, 07/24/1995 12:05:34 - California version */
	ROM_RELOAD(                     0x380000, 0x080000)

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
	ROM_LOAD( "9255-40-01_u38-r0",  0x300000, 0x80000,  CRC(ec96813d) SHA1(f93bb08ae89ab5ec1c6b33d5b1040c50d3db9ef5) ) /* Location U38, 04/03/1996 14:01 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x80000)

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

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943(__megat4s).u20",     0x000, 0x117, CRC(f31864ff) SHA1(ff44820379a350e7bd788ffb6926612b3483e114) )
	ROM_LOAD( "sc3944-0a(__megat4s).u19",  0x000, 0x2dd, CRC(ad4fddaa) SHA1(10c1575dcaa5ca4af5dc630d84f43a9ed1cb3ace) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4sn ) /* Dallas DS1204V security key at U5 labeled 9255-40-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-40-01_u32-r0a", 0x000000, 0x100000, CRC(9ace8f52) SHA1(7c755c77cbfb234e1d6f531c90e4a8661275d464) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-41-07_u38-r0g", 0x300000, 0x80000,  CRC(71eac4d4) SHA1(73b9ed876f0af94bbd88503921a2b4f26bcfd397) ) /* Location U38, 02/11/1997 11:59:41 - New Jersey version */
	ROM_RELOAD(                     0x380000, 0x80000)

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1225Y nv ram
	ROM_LOAD( "mt4sn_ds1225y.u31",  0x0000, 0x8000, CRC(8d2a97e7) SHA1(7cb01d9499fed1674da6a04a11ed1cef0a39b3c0) ) /* No actual label, so use a unique name for this set */

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

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4tea_ds1644.u31",  0x00000,  0x8000,   CRC(11e2c7ed) SHA1(99ee83410f7dbf5a259b11193829bb5c706d9fca) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4st ) /* Dallas DS1204V security key at U5 labeled 9255-51-01 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-51-01_u38-r0b", 0x300000, 0x080000, CRC(181a83cb) SHA1(b8f92ae76ebba3849db76b084f0ab7d82256d81a) ) /* Location U38, 12/10/1996 16:59:23 - Standard Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1644 nv ram
	ROM_LOAD( "mt4st_ds1644.u31",  0x00000,  0x8000,   CRC(c6226d91) SHA1(20c9fa7ad135ac229c6bdf85b901629a0ecb8a81) ) /* No actual label, so use a unique name for this set */

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

ROM_START( megat4stg ) /* Dallas DS1204V security key at U5 labeled 9255-51-50 U5-B-RO1 C1996 MII */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "9255-41-01_u32-r0",  0x000000, 0x100000, CRC(f51ae565) SHA1(99c58063bfa24b4383c8b37a1eab670fa6e4c62c) ) /* Location U32 */
	ROM_LOAD( "qs9255-02_u36-r0",   0x100000, 0x80000,  CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x80000)
	ROM_LOAD( "qs9255-02_u37-r0",   0x200000, 0x80000,  CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) ) /* Location U37 */
	ROM_RELOAD(                     0x280000, 0x80000)
	ROM_LOAD( "9255-51-50_u38-r0a", 0x300000, 0x080000, CRC(f7c2914d) SHA1(5d05b8db5ca734f7b05c3e215c0ef5b917455537) ) /* Location U38, 11/18/1996 10:11:01 - Bi-Lingual GER/ENG Version */
	ROM_RELOAD(                     0x380000, 0x080000 )

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
	ROM_LOAD( "9255-60-01_u38-r0c", 0x300000, 0x100000, BAD_DUMP CRC(1091e7fd) SHA1(3c31c178eb7bea0d2c7e839dc3ec549463092296) ) /* Location U38, 07/10/1997 16:49:56 - Standard Version */

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
	ROM_LOAD( "9255-60-01_u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) ) /* Location U32 */
	ROM_LOAD( "qs9255-08_u36-r0",   0x100000, 0x080000, CRC(800f5a1f) SHA1(4d3ee6fb896d6452aab1f279a3ee878284bd1acc) ) /* Location U36 */
	ROM_RELOAD(                     0x180000, 0x080000 )
	ROM_LOAD( "qs9255-08_u37-r0",   0x200000, 0x100000, CRC(5ba01949) SHA1(1598949ea18d07bbc78af0ddd279a687173c1229) ) /* Location U37 */
	ROM_LOAD( "9255-80-01_u38-r0a", 0x300000, 0x100000, CRC(3df6b840) SHA1(31ba1ac04eed3e76cdf637507dedcc5f7e22c919) ) /* Location U38, 08/07/1998 15:54:23 - Standard Version */

	ROM_REGION( 0x8000, "nvram", 0 ) // DS1230 nv ram
	ROM_LOAD( "ds1230y.u31",  0x00000, 0x8000, CRC(51b6da5c) SHA1(1d53af89d7867bb48b9d46feff6fc3b7e8e80ac8) )

	ROM_REGION( 0x1000, "user2", 0 ) // PALs
	ROM_LOAD( "sc3943.u20",     0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.u19",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.u40",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc3981-0a.u51",  0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

static DRIVER_INIT(pitbossm)
{
	static const UINT8 pitbossm_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 pitbossm_ds1204_nvram[16] =
		{ 0x16, 0x90, 0xa0, 0x52, 0xd8, 0x6c, 0x12, 0xaf, 0x36, 0x22, 0x61, 0x35, 0x0d, 0x58, 0x0c, 0x00 };

	ds1204_init(machine, pitbossm_ds1204_key, pitbossm_ds1204_nvram);

};

static DRIVER_INIT(pbst30)
{
	static const UINT8 pbst30b_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 pbst30b_ds1204_nvram[16] =
		{ 0x3e, 0x9a, 0x3c, 0x3f, 0x1d, 0x51, 0x72, 0xc9, 0x28, 0x2c, 0x1d, 0x2d, 0x0e, 0x56, 0x41, 0x00 };

	ds1204_init(machine, pbst30b_ds1204_key, pbst30b_ds1204_nvram);

};

static DRIVER_INIT(pbst30b)
{
	static const UINT8 pbst30b_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 pbst30b_ds1204_nvram[16] =
		{ 0xa9, 0xdb, 0x41, 0xf8, 0xe4, 0x42, 0x20, 0x6e, 0xde, 0xaf, 0x4f, 0x046, 0x3d, 0x55, 0x44, 0x00 };

	ds1204_init(machine, pbst30b_ds1204_key, pbst30b_ds1204_nvram);

};

static DRIVER_INIT(megat2)
{
	static const UINT8 pitbosmt_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 pitbosmt_ds1204_nvram[16] =
		{ 0x00, 0xfe, 0x03, 0x03, 0x08, 0x00, 0xa2, 0x03, 0x4b, 0x07, 0x00, 0xe6, 0x02, 0xd3, 0x05, 0x00 };

	ds1204_init(machine, pitbosmt_ds1204_key, pitbosmt_ds1204_nvram);

};

static DRIVER_INIT(megat3)
{
	static const UINT8 megat3_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 megat3_ds1204_nvram[16] =
		{ 0x51, 0xa1, 0xc0, 0x7c, 0x27, 0x6e, 0x51, 0xb9, 0xa5, 0xb2, 0x27, 0x0c, 0xb9, 0x88, 0x82, 0x2c };

	ds1204_init(machine, megat3_ds1204_key, megat3_ds1204_nvram);

};

static DRIVER_INIT(megat3te)
{
	static const UINT8 megat3_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 megat3_ds1204_nvram[16] =
		{ 0x99, 0x53, 0xfc, 0x29, 0x3a, 0x95, 0x8b, 0x58, 0xca, 0xca, 0x00, 0xc2, 0x30, 0x62, 0x0b, 0x96 };

	ds1204_init(machine, megat3_ds1204_key, megat3_ds1204_nvram);

	meritm_state *state = machine.driver_data<meritm_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0xfff8, 0xffff, read8_delegate(FUNC(meritm_state::meritm_ds1644_r), state), write8_delegate(FUNC(meritm_state::meritm_ds1644_w), state));

};

static DRIVER_INIT(megat4)
{
	static const UINT8 megat4_ds1204_nvram[16] =
		{ 0xe3, 0x08, 0x39, 0xd8, 0x4c, 0xbb, 0xc4, 0xf8, 0xf0, 0xe2, 0xd8, 0x77, 0xa8, 0x3d, 0x95, 0x02 };

	ds1204_init(machine, 0, megat4_ds1204_nvram);
}

static DRIVER_INIT(megat4c) /* First version of MegaTouch IV requires "key" like previous MegaTouch versions */
{
	static const UINT8 megat4c_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 megat4_ds1204_nvram[16] =
		{ 0xe3, 0x08, 0x39, 0xd8, 0x4c, 0xbb, 0xc4, 0xf8, 0xf0, 0xe2, 0xd8, 0x77, 0xa8, 0x3d, 0x95, 0x02 };

	ds1204_init(machine, megat4c_ds1204_key, megat4_ds1204_nvram);
}

static DRIVER_INIT(megat4te)
{
	static const UINT8 megat4te_ds1204_nvram[16] =
		{ 0x05, 0x21, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(machine, 0, megat4te_ds1204_nvram);

	meritm_state *state = machine.driver_data<meritm_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0xfff8, 0xffff, read8_delegate(FUNC(meritm_state::meritm_ds1644_r), state), write8_delegate(FUNC(meritm_state::meritm_ds1644_w), state));

};

static DRIVER_INIT(megat4st)
{
	static const UINT8 megat4te_ds1204_nvram[16] =
		{ 0x11, 0x04, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(machine, 0, megat4te_ds1204_nvram);

	meritm_state *state = machine.driver_data<meritm_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0xfff8, 0xffff, read8_delegate(FUNC(meritm_state::meritm_ds1644_r), state), write8_delegate(FUNC(meritm_state::meritm_ds1644_w), state));

};

static DRIVER_INIT(megat5)
{
	static const UINT8 megat5_ds1204_nvram[16] =
		{ 0x06, 0x23, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(machine, 0, megat5_ds1204_nvram);

}

static DRIVER_INIT(megat5t)
{
	static const UINT8 megat5_ds1204_nvram[16] =
		{ 0x08, 0x22, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(machine, 0, megat5_ds1204_nvram);

	meritm_state *state = machine.driver_data<meritm_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler(0xfff8, 0xffff, read8_delegate(FUNC(meritm_state::meritm_ds1644_r), state), write8_delegate(FUNC(meritm_state::meritm_ds1644_w), state));

}

static DRIVER_INIT(megat6)
{
	static const UINT8 megat6_ds1204_nvram[16] =
		{ 0x07, 0x15, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(machine, 0, megat6_ds1204_nvram);

}

/* CRT 250 */
GAME( 1988, pitboss2,  0,        meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Pit Boss II", GAME_IMPERFECT_GRAPHICS )
GAME( 1988, spitboss,  0,        meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Super Pit Boss", GAME_IMPERFECT_GRAPHICS )
GAME( 1990, pitbosss,  0,        meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Pit Boss Superstar (9221-10-00B)", GAME_IMPERFECT_GRAPHICS )
GAME( 1990, pitbosssa, pitbosss, meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Pit Boss Superstar (9221-10-00A)", GAME_IMPERFECT_GRAPHICS )

/* CRT 250 + CRT 254 + CRT 256 */
GAME( 1994, pbst30,    0,      meritm_crt250_crt252_crt258, pbst30, pbst30,  ROT0, "Merit", "Pit Boss Supertouch 30 (9234-10-01)", GAME_IMPERFECT_GRAPHICS )
GAME( 1993, pbst30b,   pbst30, meritm_crt250_crt252_crt258, pbst30, pbst30b, ROT0, "Merit", "Pit Boss Supertouch 30 (9234-00-01)", GAME_IMPERFECT_GRAPHICS )

/* CRT 250 + CRT 254 + CRT 256 */
GAME( 1994, pitbossm,  0,         meritm_crt250_questions, pitbossm, pitbossm, ROT0, "Merit", "Pit Boss Megastar (9244-00-01)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, pitbossma, pitbossm,  meritm_crt250_questions, pitbossa, 0,        ROT0, "Merit", "Pit Boss Megastar (9243-00-01)", GAME_IMPERFECT_GRAPHICS )

/* CRT 260 */
GAME( 1994, megat2,    0,      meritm_crt260, meritm_crt260, megat2,   ROT0, "Merit", "Pit Boss Megatouch II (9255-10-01 ROE, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, megat2a ,  megat2, meritm_crt260, meritm_crt260, megat2,   ROT0, "Merit", "Pit Boss Megatouch II (9255-10-01 ROD, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, megat2ca,  megat2, meritm_crt260, meritm_crt260, megat2,   ROT0, "Merit", "Pit Boss Megatouch II (9255-10-06 ROG, California version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, megat2caa, megat2, meritm_crt260, meritm_crt260, megat2,   ROT0, "Merit", "Pit Boss Megatouch II (9255-10-06 ROE, California version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat3,    0,      meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III (9255-20-01 RON, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, megat3a,   megat3, meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III (9255-20-01 ROF, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat3ca,  megat3, meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III (9255-20-06 RON, California version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, megat3cb,  megat3, meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III (9255-20-06 ROD, California version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, megat3nj,  megat3, meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III (9255-20-07 ROG, New Jersey version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat3te,  megat3, meritm_crt260, meritm_crt260, megat3te, ROT0, "Merit", "Megatouch III Tournament Edition (9255-30-01 ROE, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4,    0,      meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Megatouch IV (9255-40-01 ROE, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4a,   megat4, meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Megatouch IV (9255-40-01 ROD, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4b,   megat4, meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Megatouch IV (9255-40-01 ROB, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4c,   megat4, meritm_crt260, meritm_crt260, megat4c,  ROT0, "Merit", "Megatouch IV (9255-40-01 RO, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4s,   megat4, meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Super Megatouch IV (9255-41-01 ROG, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4sn,  megat4, meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Super Megatouch IV (9255-41-07 ROG, New Jersey version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4te,  megat4, meritm_crt260, meritm_crt260, megat4te, ROT0, "Merit", "Megatouch IV Tournament Edition (9255-50-01 ROD, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4tea, megat4, meritm_crt260, meritm_crt260, megat4te, ROT0, "Merit", "Megatouch IV Tournament Edition (9255-50-01 ROA, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4st,  megat4, meritm_crt260, meritm_crt260, megat4st, ROT0, "Merit", "Super Megatouch IV Tournament Edition (9255-51-01 ROB, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4stg, megat4, meritm_crt260, meritm_crt260, megat4st, ROT0, "Merit", "Super Megatouch IV Turnier Version (9255-51-50 ROA, Bi-Lingual GER/ENG version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, megat5,    0,      meritm_crt260, meritm_crt260, megat5,   ROT0, "Merit", "Megatouch 5 (9255-60-01 ROI, Standard version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, megat5a,   megat5, meritm_crt260, meritm_crt260, megat5,   ROT0, "Merit", "Megatouch 5 (9255-60-01 ROC, Standard version)", GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1998, megat5nj,  megat5, meritm_crt260, meritm_crt260, megat5,   ROT0, "Merit", "Megatouch 5 (9255-60-07 RON, New Jersey version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, megat5tg,  megat5, meritm_crt260, meritm_crt260, megat5t,  ROT0, "Merit", "Megatouch 5 Turnier Version (9255-70-50 ROD, Bi-Lingual GER/ENG version)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, megat6,    0,      meritm_crt260, meritm_crt260, megat6,   ROT0, "Merit", "Megatouch 6 (9255-80-01 ROA, Standard version)", GAME_IMPERFECT_GRAPHICS )
