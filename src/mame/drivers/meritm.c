/*
  Merit CRT250 and CRT260 hardware

  Driver by Mariusz Wojcieszek

  CRT 250 (basic components, also used by CRT260)
    Main CPU: 1xZ80B
  Sound: 1xYM2149F (or compatible)
    Video: 2xYamaha V9938 (MSX2 video chip!)
  Other: 2xZ80APIO (I/O and interrupt controllers)
         1x8255 (I/O)

  CRT 260 additional components:
  - Microtouch touch screen controller (SMT-3)
  - PC16550 UART (for communication with touch screen controller)
  - DS1204 Electronic Key (for protection)
  - DS1232 Reset and Watchdog
  - DS1644 (megat4te) 32K NVRAM + RTC
  - MAX232 (for MegaLink)

  Known Games:

  CRT 250:
  Pit Boss II (c)1988
  Super Pit Boss (c)1988
  *Pit Boss Superstar (c)1989
  *Pit Boss Superstar 30 (c)1993
  Pit Boss Megastar (c)1994

  CRT 260:
  *Pit Boss Supertouch 30 (c)1994?
  *Megatouch Video (c)1994?
  *Megatouch II (c)1995
  *Megatouch III (c)1995
  *Megatouch IV (c)1996
  Megatouch IV Tournament Edition (c)1996
  *Super Megatouch IV (c) 1996
  *Megatouch 5 (c)1997
  Megatouch 6 (c)1998
  *Megatouch 7 Encore (c)2000

  * indicates that game needs to be dumped or redumped

  Notes/ToDo:
  - offset for top V9938 layer is hardcoded, probably should be taken from V9938 setup
  - blinking on Meagtouch title screen is probably incorrect
  - clean up V9938 interrupt implementation
  - finish inputs, dsw, outputs (lamps)
  - calibration mode in Microtouch emulation (command CX)
  - problem with registering touches on the bottom of the screen (currently hacked to work)
  - megat3: u37 has bad size, should be 8MBit
  - megat4: rom u32 was bad, currently using u32 from megat4te
  - megat5: has jmp $0000 in the initialization code causing infinite loop, is rom bad?
 */

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "machine/8255ppi.h"
#include "machine/z80pio.h"
#include "machine/pc16552d.h"

/*************************************
 *
 *  Globals
 *
 *************************************/


#define SYSTEM_CLK	21470000
#define UART_CLK	XTAL_18_432MHz

static UINT8* meritm_ram;

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

static struct
{
	int state;
	int read_ptr;
	int last_clk;
	UINT8 key[8];
	UINT8 nvram[16];
	int out_bit;
	UINT8 command[3];

} ds1204;

static void ds1204_w( int rst, int clk, int dq )
{
	//logerror("ds1204_w: rst = %d, clk = %d, dq = %d\n", rst, clk, dq );
	if ( rst == 0 )
	{
		ds1204.state = DS1204_STATE_COMMAND;
		ds1204.read_ptr = 0;
	}
	else
	{
		if ( (ds1204.last_clk == 1) && (clk == 0) )
		{
			switch(ds1204.state)
			{
				case DS1204_STATE_COMMAND:
					//logerror("Command bit %d = %d\n", ds1204.read_ptr, dq);
					if ( ds1204.read_ptr < 24 )
					{
						if ( dq == 1 )
						{
							ds1204.command[ds1204.read_ptr >> 3] |= (1 << (ds1204.read_ptr & 0x7));
						}
						else
						{
							ds1204.command[ds1204.read_ptr >> 3] &= ~(1 << (ds1204.read_ptr & 0x7));
						}
						ds1204.read_ptr++;
					}
					if ( ds1204.read_ptr == 24 )
					{
						ds1204.state = DS1204_STATE_READ_KEY;
						ds1204.read_ptr = 0;
					}
					break;
				case DS1204_STATE_READ_KEY:
					//logerror("Key bit %d\n", ds1204.read_ptr);
					if (ds1204.read_ptr < 64)
					{
						ds1204.out_bit = (ds1204.key[ds1204.read_ptr >> 3] >> (ds1204.read_ptr & 0x7)) & 0x01;
						ds1204.read_ptr++;
					}
					if (ds1204.read_ptr == 64)
					{
						ds1204.state = DS1204_STATE_WRITE_SECURITY_MATCH;
						ds1204.read_ptr = 0;
					}
					break;
				case DS1204_STATE_WRITE_SECURITY_MATCH:
					//logerror( "Security match bit %d = %d\n", ds1204.read_ptr, dq);
					if (ds1204.read_ptr < 64)
					{
						ds1204.read_ptr++;
					}
					if (ds1204.read_ptr == 64)
					{
						ds1204.state = DS1204_STATE_READ_NVRAM;
						ds1204.read_ptr = 0;
					}
					break;
				case DS1204_STATE_READ_NVRAM:
					//logerror( "Read nvram bit = %d\n", ds1204.read_ptr );
					if (ds1204.read_ptr < 128)
					{
						ds1204.out_bit = (ds1204.nvram[ds1204.read_ptr >> 3] >> (ds1204.read_ptr & 0x7)) & 0x01;
						ds1204.read_ptr++;
					}
					if (ds1204.read_ptr == 128)
					{
						ds1204.state = DS1204_STATE_IDLE;
						ds1204.read_ptr = 0;
					}
					break;

			}
		}
		ds1204.last_clk = clk;
	}
};

static int ds1204_r(void)
{
	//logerror("ds1204_r\n");
	return ds1204.out_bit;
};

static void ds1204_init(const UINT8* key, const UINT8* nvram)
{
	memset(&ds1204, 0, sizeof(ds1204));
	if (key)
		memcpy(ds1204.key, key, sizeof(ds1204.key));
	if (nvram)
		memcpy(ds1204.nvram, nvram, sizeof(ds1204.nvram));

	state_save_register_item("ds1204", 0, ds1204.state);
	state_save_register_item("ds1204", 0, ds1204.read_ptr);
	state_save_register_item("ds1204", 0, ds1204.last_clk);
	state_save_register_item("ds1204", 0, ds1204.out_bit);
	state_save_register_item_array("ds1204", 0, ds1204.command);
};

/*************************************
 *
 *  Microtouch touch screen controller
 *
 *************************************/

static struct
{
	UINT8		tx_buffer[16];
	int		tx_buffer_ptr;
	emu_timer*	timer;
	int		reset_done;
	int		format_tablet;
	int		mode_inactive;
	int		mode_stream;
	int		last_touch_state;
	int		last_x;
	int		last_y;
	void		(*tx_callback)(UINT8 data);
	int		(*touch_callback)(int *touch_x, int *touch_y);
} microtouch;


static int microtouch_check_command( const char* commandtocheck, int command_len, UINT8* command_data )
  {
	if ( (command_len == (strlen(commandtocheck) + 2)) &&
		 (command_data[0] == 0x01) &&
		 (strncmp(commandtocheck, (const char*)command_data + 1, strlen(commandtocheck)) == 0) &&
		 (command_data[command_len-1] == 0x0d) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
  }

static void microtouch_send_format_table_packet(UINT8 flag, int x, int y)
{
	microtouch.tx_callback(flag);
	// lower byte (7bits) of x coordinate
	microtouch.tx_callback(x & 0x7f);
	// higher byte (7bits) of x coordinate
	microtouch.tx_callback((x >> 7) & 0x7f);
	// lower byte (7bits) of y coordinate
	microtouch.tx_callback(y & 0x7f);
	// higher byte (7bits) of y coordinate
	microtouch.tx_callback((y >> 7) & 0x7f);
};

static TIMER_CALLBACK(microtouch_timer_callback)
{
	if ( (microtouch.reset_done == 0) ||
		 (microtouch.format_tablet == 0) ||
		 (microtouch.mode_inactive == 1) ||
		 (microtouch.mode_stream == 0) )
	{
		return;
	}

	// send format tablet packet
	if ( input_port_read(machine, "TOUCH") & 0x01 )
	{
		int tx = input_port_read(machine, "TOUCH_X");
		int ty = input_port_read(machine, "TOUCH_Y");

		if ( microtouch.touch_callback == NULL ||
			 microtouch.touch_callback( &tx, &ty ) != 0 )
		{
			ty = 0x4000 - ty;

			microtouch_send_format_table_packet(0xc0, tx, ty);
			microtouch.last_touch_state = 1;
			microtouch.last_x = tx;
			microtouch.last_y = ty;
		}
	}
	else
	{
		if ( microtouch.last_touch_state == 1 )
		{
			microtouch.last_touch_state = 0;
			microtouch_send_format_table_packet(0x80, microtouch.last_x, microtouch.last_y);
		}
	}
};

static void microtouch_init(void (*tx_cb)(UINT8 data),
							int (*touch_cb)(int *touch_x, int *touch_y))
{
	memset(&microtouch, 0, sizeof(microtouch));

	microtouch.last_touch_state = -1;
	microtouch.tx_callback = tx_cb;
	microtouch.touch_callback = touch_cb;

	microtouch.timer = timer_alloc(microtouch_timer_callback, NULL);
	timer_adjust_periodic(microtouch.timer, ATTOTIME_IN_HZ(167), 0, ATTOTIME_IN_HZ(167));

	state_save_register_item("microtouch", 0, microtouch.reset_done);
	state_save_register_item("microtouch", 0, microtouch.format_tablet);
	state_save_register_item("microtouch", 0, microtouch.mode_inactive);
	state_save_register_item("microtouch", 0, microtouch.mode_stream);
	state_save_register_item("microtouch", 0, microtouch.last_touch_state);
	state_save_register_item("microtouch", 0, microtouch.last_x);
	state_save_register_item("microtouch", 0, microtouch.last_y);
	state_save_register_item_array("microtouch", 0, microtouch.tx_buffer);
	state_save_register_item("microtouch", 0, microtouch.tx_buffer_ptr);

};


static void microtouch_rx(int count, UINT8* data)
{
	int i;

	for ( i = 0; (i < count) && ((microtouch.tx_buffer_ptr + i) < 16); i++ )
	{
		microtouch.tx_buffer[i+microtouch.tx_buffer_ptr] = data[i];
		microtouch.tx_buffer_ptr++;
	}

	if (microtouch.tx_buffer_ptr > 0 && (microtouch.tx_buffer[microtouch.tx_buffer_ptr-1] == 0x0d))
	{
		// check command
		if ( microtouch_check_command( "MS", microtouch.tx_buffer_ptr, microtouch.tx_buffer ) )
		{
			microtouch.mode_stream = 1;
			microtouch.mode_inactive = 0;
		}
		else if ( microtouch_check_command( "MI", microtouch.tx_buffer_ptr, microtouch.tx_buffer ) )
		{
			microtouch.mode_inactive = 1;
		}
		else if ( microtouch_check_command( "R", microtouch.tx_buffer_ptr, microtouch.tx_buffer ) )
		{
			microtouch.reset_done = 1;
		}
		else if ( microtouch_check_command( "FT", microtouch.tx_buffer_ptr, microtouch.tx_buffer ) )
		{
			microtouch.format_tablet = 1;
		}
		// send response
		microtouch.tx_callback(0x01);
		microtouch.tx_callback(0x30);
		microtouch.tx_callback(0x0d);
		microtouch.tx_buffer_ptr = 0;
	}
};

static INPUT_PORTS_START(microtouch)
	PORT_START_TAG("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(10) PORT_NAME( "Touch screen" )
	PORT_START_TAG("TOUCH_X")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
	PORT_START_TAG("TOUCH_Y")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

/*************************************
 *
 *  Microtouch <-> pc16650 interface
 *
 *************************************/

static void pc16650d_tx_callback(int channel, int count, UINT8* data)
{
	microtouch_rx(count, data);
};

static void meritm_microtouch_tx_callback(UINT8 data)
{
	pc16552d_rx_data(0, 0, data);
};

/*************************************
 *
 *  Microtouch touch coordinate transformation
 *
 *************************************/
static int meritm_touch_coord_transform(int *touch_x, int *touch_y)
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

/*************************************
 *
 *  Video
 *
 *************************************/

static int meritm_vint = 0x00;
static int meritm_interrupt_vdp0_state = 0;
static int meritm_interrupt_vdp1_state = 0;
static bitmap_t *vdp0_bitmap, *vdp1_bitmap;

static INTERRUPT_GEN( meritm_interrupt )
{
	v9938_set_sprite_limit(0, 0);
	v9938_set_resolution(0, RENDER_HIGH);
	v9938_interrupt(0);

	v9938_set_sprite_limit(1, 0);
	v9938_set_resolution(1, RENDER_HIGH);
	v9938_interrupt(1);
}

static void meritm_vdp0_interrupt(int i)
{
	if ( meritm_interrupt_vdp0_state != i )
	{
		meritm_interrupt_vdp0_state = i;
		if (i)
			meritm_vint &= ~0x08;
		else
			meritm_vint |= 0x08;

		if(i)
			z80pio_p_w(0, 0, meritm_vint);
	}
}

static void meritm_vdp1_interrupt(int i)
{
	if ( meritm_interrupt_vdp1_state != i )
	{
		meritm_interrupt_vdp1_state = i;
		if (i)
			meritm_vint &= ~0x10;
		else
			meritm_vint |= 0x10;

		if(i)
			z80pio_p_w(0, 0, meritm_vint);
	}
}

static VIDEO_START( meritm )
{
	vdp0_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	v9938_init (machine, 0, machine->primary_screen, vdp0_bitmap, MODEL_V9938, 0x20000, meritm_vdp0_interrupt);
	v9938_reset(0);

	vdp1_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	v9938_init (machine, 1, machine->primary_screen, vdp1_bitmap, MODEL_V9938, 0x20000, meritm_vdp1_interrupt);
	v9938_reset(1);

	state_save_register_global(meritm_vint);
	state_save_register_global(meritm_interrupt_vdp0_state);
	state_save_register_global(meritm_interrupt_vdp1_state);
	state_save_register_global_bitmap(vdp0_bitmap);
	state_save_register_global_bitmap(vdp1_bitmap);

}

static int layer0_enabled = 1, layer1_enabled = 1;

static VIDEO_UPDATE( meritm )
{
	if(input_code_pressed_once(KEYCODE_Q))
	{
		layer0_enabled^=1;
		popmessage("Layer 0 %sabled",layer0_enabled ? "en" : "dis");
	}
	if(input_code_pressed_once(KEYCODE_W))
	{
		layer1_enabled^=1;
		popmessage("Layer 1 %sabled",layer1_enabled ? "en" : "dis");
	}

	fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);

	if ( layer0_enabled )
	{
		copybitmap(bitmap, vdp0_bitmap, 0, 0, 0, 0, cliprect);
	}

	if ( layer1_enabled )
	{
		copybitmap_trans(bitmap, vdp1_bitmap, 0, 0, -6, -12, cliprect, v9938_get_transpen(1));
	}
	return 0;
}

/*************************************
 *
 *  Bank switching (ROM/RAM)
 *
 *************************************/

static int meritm_bank;
static int meritm_psd_a15;

static void meritm_crt250_switch_banks( void )
{
	int rombank = (meritm_bank & 0x07) ^ 0x07;

	//logerror( "CRT250: Switching banks: rom = %0x (bank = %x)\n", rombank, meritm_bank );
	memory_set_bank(1, rombank );
};

static WRITE8_HANDLER(meritm_crt250_bank_w)
{
	meritm_crt250_switch_banks();
};

static void meritm_switch_banks( void )
{
	int rambank = (meritm_psd_a15 >> 2) & 0x3;
	int rombank = (((meritm_bank >> 3) & 0x3) << 5) |
			  (((meritm_psd_a15 >> 1) & 0x1) << 4) |
			  (((meritm_bank & 0x07) ^ 0x07) << 1) |
			  (meritm_psd_a15 & 0x1);

	//logerror( "Switching banks: rom = %0x (bank = %x), ram = %0x\n", rombank, meritm_bank, rambank);
	memory_set_bank(1, rombank );
	memory_set_bank(2, rombank | 0x01);
	memory_set_bank(3, rambank);
};

static WRITE8_HANDLER(meritm_psd_a15_w)
{
	meritm_psd_a15 = data;
	//logerror( "Writing PSD_A15 with %02x at PC=%04X\n", data, activecpu_get_pc() );
	meritm_switch_banks();
};

static WRITE8_HANDLER(meritm_bank_w)
{
	meritm_switch_banks();
};

/*************************************
 *
 *  CRT250 question roms reading
 *
 *************************************/

static UINT16 questions_loword_address;

static WRITE8_HANDLER(meritm_crt250_questions_lo_w)
{
	questions_loword_address &= 0xff00;
	questions_loword_address |= data;
};

static WRITE8_HANDLER(meritm_crt250_questions_hi_w)
{
	questions_loword_address &= 0x00ff;
	questions_loword_address |= (data << 8);
};

static WRITE8_HANDLER(meritm_crt250_questions_bank_w)
{
	UINT32 questions_address;
	UINT8 *dst;

	if (meritm_bank != 0)
	{
		logerror("meritm_crt250_questions_bank_w: bank is %d\n", meritm_bank);
		return;
	}

	dst = memory_region(REGION_CPU1) + 0x70000 + 2;

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
		logerror( "Reading question byte at %06X\n", questions_address | questions_loword_address);
		*dst = memory_region(REGION_USER1)[questions_address | questions_loword_address];
	}
};


/*************************************
 *
 *  DS1644 RTC
 *
 *************************************/

static WRITE8_HANDLER(meritm_ds1644_w)
{
	int rambank = (meritm_psd_a15 >> 2) & 0x3;
	if (rambank < 3)
	{
		meritm_ram[rambank*0x2000 + 0x1ff8 + offset] = data;
	}
	else
	{
		if (offset == 0)
		{
			meritm_ram[0x7ff8] = data;
		}
		//logerror( "Writing RTC, reg = %d, data = %x\n", offset, data);
	}
};

static UINT8 binary_to_BCD(UINT8 data)
{
	data %= 100;

	return ((data / 10) << 4) | (data %10);
}

static READ8_HANDLER(meritm_ds1644_r)
{
	mame_system_time systime;
	int rambank = (meritm_psd_a15 >> 2) & 0x3;
	if (rambank == 3)
	{
		//logerror( "Reading RTC, reg = %x\n", offset);

		mame_get_current_datetime(machine, &systime);
		meritm_ram[0x7ff9] = binary_to_BCD(systime.local_time.second);
		meritm_ram[0x7ffa] = binary_to_BCD(systime.local_time.minute);
		meritm_ram[0x7ffb] = binary_to_BCD(systime.local_time.hour);
		meritm_ram[0x7ffc] = binary_to_BCD(systime.local_time.weekday+1);
		meritm_ram[0x7ffd] = binary_to_BCD(systime.local_time.mday);
		meritm_ram[0x7ffe] = binary_to_BCD(systime.local_time.month+1);
		meritm_ram[0x7fff] = binary_to_BCD(systime.local_time.year % 100);
	}
	return meritm_ram[rambank*0x2000 + 0x1ff8 + offset];
};

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( meritm_crt250_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROMBANK(1)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_crt250_questions_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROMBANK(1)
	AM_RANGE(0x0000, 0x0000) AM_WRITE(meritm_crt250_questions_lo_w)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(meritm_crt250_questions_hi_w)
	AM_RANGE(0x0002, 0x0002) AM_WRITE(meritm_crt250_questions_bank_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_crt250_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READWRITE(v9938_0_vram_r, v9938_0_vram_w)
	AM_RANGE(0x11, 0x11) AM_READWRITE(v9938_0_status_r, v9938_0_command_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(v9938_0_palette_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(v9938_0_register_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(v9938_1_vram_r, v9938_1_vram_w)
	AM_RANGE(0x21, 0x21) AM_READWRITE(v9938_1_status_r, v9938_1_command_w)
	AM_RANGE(0x22, 0x22) AM_WRITE(v9938_1_palette_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(v9938_1_register_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)
	AM_RANGE(0x40, 0x43) AM_READWRITE(z80pio_0_r, z80pio_0_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(z80pio_1_r, z80pio_1_w)
	AM_RANGE(0x80, 0x80) AM_READWRITE(AY8910_read_port_0_r, AY8910_control_port_0_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(meritm_crt250_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK(1)
	AM_RANGE(0x8000, 0xdfff) AM_ROMBANK(2)
	AM_RANGE(0xe000, 0xffff) AM_RAMBANK(3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( meritm_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(meritm_psd_a15_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10, 0x10) AM_READWRITE(v9938_0_vram_r, v9938_0_vram_w)
	AM_RANGE(0x11, 0x11) AM_READWRITE(v9938_0_status_r, v9938_0_command_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(v9938_0_palette_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(v9938_0_register_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(v9938_1_vram_r, v9938_1_vram_w)
	AM_RANGE(0x21, 0x21) AM_READWRITE(v9938_1_status_r, v9938_1_command_w)
	AM_RANGE(0x22, 0x22) AM_WRITE(v9938_1_palette_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(v9938_1_register_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)
	AM_RANGE(0x40, 0x43) AM_READWRITE(z80pio_0_r, z80pio_0_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(z80pio_1_r, z80pio_1_w)
	AM_RANGE(0x60, 0x67) AM_READWRITE(pc16552d_0_r,pc16552d_0_w)
	AM_RANGE(0x80, 0x80) AM_READWRITE(AY8910_read_port_0_r, AY8910_control_port_0_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(meritm_bank_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START(meritm_crt260)
	PORT_INCLUDE(microtouch)

	PORT_START_TAG("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("PIO1_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Calibration" ) PORT_CODE(KEYCODE_C)
INPUT_PORTS_END

static INPUT_PORTS_START(meritm_crt250)
	PORT_START_TAG("PIO1_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("PIO1_PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START(pitbossm)
	PORT_INCLUDE(meritm_crt250)

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Run 21 and Trivia Whiz 2000: Coins to start" )
	PORT_DIPSETTING(    0x00, "2 Coins" )
	PORT_DIPSETTING(    0x10, "1 Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Great Solitaire: Coins to start" )
	PORT_DIPSETTING(    0x00, "4 Coins" )
	PORT_DIPSETTING(    0x20, "2 Coins" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  8255
 *
 *************************************/

static READ8_HANDLER(meritm_8255_port_c_r)
{
	//logerror( "8255 port C read\n" );
	return 0xff;
};

static WRITE8_HANDLER(meritm_crt250_port_b_w)
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
	1,
	{ 0 }, /* Port A read */
	{ 0 }, /* Port B read */
	{ meritm_8255_port_c_r }, /* Port C read */
	{ 0 }, /* Port A write (used) */
	{ 0 }, /* Port B write (used LMP x DRIVE) */
	{ 0 }  /* Port C write */
};

static const ppi8255_interface crt250_ppi8255_intf =
{
	1,
	{ 0 }, /* Port A read */
	{ 0 }, /* Port B read */
	{ meritm_8255_port_c_r }, /* Port C read */
	{ 0 }, /* Port A write (used) */
	{ meritm_crt250_port_b_w }, /* Port B write (used LMP x DRIVE) */
	{ 0 }  /* Port C write */
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

static READ8_HANDLER(meritm_ay8930_port_a_r)
{
	return input_port_read_safe(machine, "DSW", 0);
};

static WRITE8_HANDLER(meritm_ay8930_port_b_w)
{
	// lamps
};

static const struct AY8910interface ay8910_interface =
{
	meritm_ay8930_port_a_r, /* Port A read */
	0, /* Port B read */
	0, /* Port A write */
	meritm_ay8930_port_b_w  /* Port B write */
};

/*************************************
 *
 *  PIOs
 *
 *************************************/

static void meritm_audio_pio_interrupt(int state)
{
	//logerror( "PIO(0) interrupt line: %d, V = %d, H = %d\n", state, video_screen_get_vpos(0), video_screen_get_hpos(0) );
	cpunum_set_input_line(Machine, 0, 0, state);
}

static void meritm_io_pio_interrupt(int state)
{
	//logerror( "PIO(1) interrupt line: %d, V = %d, H = %d\n", state, video_screen_get_vpos(0), video_screen_get_hpos(0) );
	cpunum_set_input_line(Machine, 0, 0, state);
}


static READ8_HANDLER(meritm_audio_pio_port_a_r)
{
	return meritm_vint;
};

static READ8_HANDLER(meritm_audio_pio_port_b_r)
{
	return ds1204_r();
};

static WRITE8_HANDLER(meritm_audio_pio_port_a_w)
{
	meritm_bank = (data & 7) | ((data >> 2) & 0x18);
	//logerror("Writing BANK with %x (raw = %x)\n", meritm_bank, data);
};

static WRITE8_HANDLER(meritm_audio_pio_port_b_w)
{
	ds1204_w((data & 0x4) >> 2, (data & 0x2) >> 1, data & 0x01);
};

static READ8_HANDLER(meritm_io_pio_port_a_r)
{
	return input_port_read(machine, "PIO1_PORTA");
};

static READ8_HANDLER(meritm_io_pio_port_b_r)
{
	return input_port_read(machine, "PIO1_PORTB");
};

static WRITE8_HANDLER(meritm_io_pio_port_a_w)
{
};

static WRITE8_HANDLER(meritm_io_pio_port_b_w)
{
};

static const z80pio_interface meritm_audio_pio_intf =
{
	meritm_audio_pio_interrupt,
	meritm_audio_pio_port_a_r,
	meritm_audio_pio_port_b_r,
	meritm_audio_pio_port_a_w,
	meritm_audio_pio_port_b_w,
	0,
	0
};

static const z80pio_interface meritm_io_pio_intf =
{
	meritm_io_pio_interrupt,
	meritm_io_pio_port_a_r,
	meritm_io_pio_port_b_r,
	meritm_io_pio_port_a_w,
	meritm_io_pio_port_b_w,
	0,
	0
};

/*
static void meritm_pio1_portb_input_changed_callback(void *param, UINT32 oldval, UINT32 newval)
{
    z80pio_p_w(1, 1, (UINT8)newval);
}
*/

static const struct z80_irq_daisy_chain meritm_daisy_chain[] =
{
	{ z80pio_reset, z80pio_irq_state, z80pio_irq_ack, z80pio_irq_reti, 1 }, /* PIO number 1 */
	{ z80pio_reset, z80pio_irq_state, z80pio_irq_ack, z80pio_irq_reti, 0 }, /* PIO number 0 */
	{ 0, 0, 0, 0, -1 }		/* end mark */
};

static MACHINE_START(merit_common)
{
	z80pio_init(0, &meritm_audio_pio_intf);
	z80pio_init(1, &meritm_io_pio_intf);
	//input_port_set_changed_callback(port_tag_to_index("PIO1_PORTB"), 0xff, meritm_pio1_portb_input_changed_callback, NULL);

};

static MACHINE_START(meritm_crt250)
{
	ppi8255_init(&crt250_ppi8255_intf);
	memory_configure_bank(1, 0, 8, memory_region(REGION_CPU1), 0x10000);
	meritm_bank = 0xff;
	meritm_crt250_switch_banks();
	machine_start_merit_common(machine);
	state_save_register_global(meritm_bank);

};

static MACHINE_START(meritm_crt250_questions)
{
	machine_start_meritm_crt250(machine);
	state_save_register_global(questions_loword_address);
};

static MACHINE_START(meritm_crt260)
{
	ppi8255_init(&crt260_ppi8255_intf);
	meritm_ram = auto_malloc( 0x8000 );
	memset( meritm_ram, 0x8000, 0x00 );
	memory_configure_bank(1, 0, 128, memory_region(REGION_CPU1), 0x8000);
	memory_configure_bank(2, 0, 128, memory_region(REGION_CPU1), 0x8000);
	memory_configure_bank(3, 0, 4, meritm_ram, 0x2000);
	meritm_bank = 0xff;
	meritm_psd_a15 = 0;
	meritm_switch_banks();
	machine_start_merit_common(machine);
	pc16552d_init(0, UART_CLK, NULL, pc16650d_tx_callback);
	microtouch_init(meritm_microtouch_tx_callback, meritm_touch_coord_transform);
	state_save_register_global(meritm_bank);
	state_save_register_global(meritm_psd_a15);
	state_save_register_global_pointer(meritm_ram, 0x8000);
};

static NVRAM_HANDLER(meritm_crt260)
{
	if (read_or_write)
		mame_fwrite(file, meritm_ram, 0x8000);
	else
		if (file)
			mame_fread(file, meritm_ram, 0x8000);
		else
			if ( memory_region(REGION_USER1) )
				memcpy(meritm_ram, memory_region(REGION_USER1), 0x8000);
};

// from MSX2 driver, may be not accurate for merit games
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2

static MACHINE_DRIVER_START(meritm_crt250)
	MDRV_CPU_ADD_TAG("main", Z80, SYSTEM_CLK/6)
	MDRV_CPU_PROGRAM_MAP(meritm_crt250_map,0)
	MDRV_CPU_IO_MAP(meritm_crt250_io_map,0)
	MDRV_CPU_CONFIG(meritm_daisy_chain)
  	MDRV_CPU_VBLANK_INT_HACK(meritm_interrupt,262)

	MDRV_MACHINE_START(meritm_crt250)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

  	MDRV_SCREEN_ADD("main",RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MDRV_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT( v9938 )

	MDRV_VIDEO_START(meritm)
	MDRV_VIDEO_UPDATE(meritm)

  /* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8910, SYSTEM_CLK/6)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START(meritm_crt250_questions)
	MDRV_IMPORT_FROM(meritm_crt250)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(meritm_crt250_questions_map,0)
	MDRV_MACHINE_START(meritm_crt250_questions)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START(meritm_crt260)
	MDRV_IMPORT_FROM(meritm_crt250)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(meritm_map,0)
	MDRV_CPU_IO_MAP(meritm_io_map,0)

	MDRV_WATCHDOG_TIME_INIT(UINT64_ATTOTIME_IN_MSEC(1200))	// DS1232, TD connected to VCC
	MDRV_MACHINE_START(meritm_crt260)

	MDRV_NVRAM_HANDLER(meritm_crt260)

MACHINE_DRIVER_END


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
	ROM_REGION( 0x80000, REGION_CPU1, 0 )
	ROM_LOAD( "u9",  0x00000, 0x10000, CRC(a1b6ac15) SHA1(b7b395f3e7e14dbb84003e03bf7d054e795a7211) )
	ROM_LOAD( "u10", 0x10000, 0x10000, CRC(207aa83c) SHA1(1955d75b9e561312e98831571c9853579ded3734) )
	ROM_LOAD( "u11", 0x20000, 0x10000, CRC(2052e043) SHA1(36b6cbc5712fc736c748a68bd12675291eae669d) )
	ROM_LOAD( "u12", 0x30000, 0x10000, CRC(33653f16) SHA1(57b9822499324502d66dc5a40e662596e5336943) )
	ROM_LOAD( "u13", 0x40000, 0x10000, CRC(4f139e88) SHA1(425dd34804cc614aa93a468d2ba3e16de62f099c) )
	ROM_LOAD( "u14", 0x50000, 0x10000, CRC(a58078cd) SHA1(a028be67fa05670a689144dfb9c9da51c5732389) )
	ROM_LOAD( "u15", 0x60000, 0x10000, CRC(239b5d03) SHA1(fffb69cd7af215445da2b1281bcbc5f4fb6cfcc3) )
	ROM_LOAD( "u16", 0x70000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) )
ROM_END

ROM_START( spitboss )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )
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

    Games are: Great Solitaire, Run 21 and Trivia Whiz 2000.
    -------------------------------------------------------
    Some of the pinouts probably flash lighted buttons as
    most Merit games have this feature.
    -------------------------------------------------------

    EPROMS 1,2, and 3 are 27C2001's
    EPROMS 4 through 9 are 27C512's

    One 8 bank dip switch.

    Two YAMAHA V9938 Video Processors.

    21.47727 MHz Crystal tied into pin 63 on both the V9938's

    CPU Z80B

    Audio YM2149F
    Two Z80A-PIO

    One Goldstar GM76C88L-15 (6264) SRAM
    Eight V53C464AP80 (41464) RAMS

    One PALCE16V8H-25PC/4
    One GAL22V10B

    chaneman Sept.23 2004
*/

ROM_START( pitbossm )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )
	ROM_LOAD( "4",  0x00000, 0x10000, CRC(55e14fb1) SHA1(ec29764d1b63360f64b82452e0db8054b99fcca0) )
	ROM_LOAD( "5",  0x10000, 0x10000, CRC(853a1a99) SHA1(45e33442aa7e51c05c9ac8b8458937ee3ff4c21d) )
	ROM_LOAD( "6",  0x20000, 0x10000, CRC(47a9dfc7) SHA1(eca100003f5605bcf405f610a0458ccb67894d35) )
	ROM_LOAD( "7",  0x30000, 0x10000, CRC(b9fb4203) SHA1(84b514d9739d9c2ab1081cfc7cdedb41155ee038) )
	ROM_LOAD( "8",  0x40000, 0x10000, CRC(574fb3c7) SHA1(213741df3055b97ddd9889c2aa3d3e863e2c86d3) ) // == u16 on pitboss2/spitboss
	ROM_RELOAD(		0x50000, 0x10000)
	ROM_LOAD( "9",  0x60000, 0x10000, CRC(27034061) SHA1(cff6be592a4a3ab01c204b081470f224e6186c4d) )
	ROM_RELOAD(		0x70000, 0x10000)


	ROM_REGION( 0xc0000, REGION_USER1, 0 ) // extra data / extra banks?
	ROM_LOAD( "3",  0x00000, 0x40000, CRC(35f4ca46) SHA1(87917b3017f505fae65d6bfa2c7d6fb503c2da6a) )
	ROM_LOAD( "2",  0x40000, 0x40000, CRC(606f1656) SHA1(7f1e3a698a34d3c3b8f9f2cd8d5224b6c096e941) )
	ROM_LOAD( "1",  0x80000, 0x40000, CRC(590a1565) SHA1(b80ea967b6153847b2594e9c59bfe87559022b6c) )
ROM_END

/*
    Mega Touch 3
    by Merit Industries

    Dumped by NAZ!
    on 9/20/1998


    System Info
    -----------
     This is a counter top Touch screen game.

    processor.. Z80
    sound processor- YM2149
    other chips- two Yamaha V9938
             one LM1203
             one PC165500N
             one PB255a or L5220574
             One Dallas DS1204 Data Key
             One dallas DS1225Y 16k Non-volitile RAM
             Two Z80APIO
*/

ROM_START( megat3 )
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD( "megat3.u32",  0x000000, 0x80000, CRC(ac969296) SHA1(7e09e9141637339b83c21f2488560cdf8a460069) )
	ROM_RELOAD(              0x080000, 0x80000)
	ROM_LOAD( "megat3.u36",  0x100000, 0x80000, CRC(96bb501e) SHA1(f48ef238e8543676c42e3b85464a25ac179dcdd1) )
	ROM_RELOAD(              0x180000, 0x80000)
	ROM_LOAD( "megat3.u37",  0x200000, 0x80000, BAD_DUMP CRC(96680164) SHA1(dfb8e07ba0e87316a947238e7a00fbf4d6ed5fe4) ) // should be 8MBit
	ROM_RELOAD(              0x280000, 0x80000)
	ROM_LOAD( "megat3.u38",  0x300000, 0x80000, CRC(85f48b91) SHA1(7a38644ac7ee55a254c037122af919fb268744a1) )
	ROM_RELOAD(              0x380000, 0x80000)
ROM_END

ROM_START( megat4 )
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD( "megat4.u32",  0x000000, 0x100000, BAD_DUMP CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) ) // from megat4te dump
	ROM_LOAD( "qs9255-02u36.bin",  0x100000, 0x80000, CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) )
	ROM_RELOAD(              0x180000, 0x80000)
	ROM_LOAD( "qs9255-02u37.bin",  0x200000, 0x80000, CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) )
	ROM_RELOAD(              0x280000, 0x80000)
	ROM_LOAD( "9255-40-01u38.bin",  0x300000, 0x80000, CRC(0d098424) SHA1(ef2810ccd636e69378fd353c8a95605274bb227f) )
	ROM_RELOAD(              0x380000, 0x80000)
ROM_END

/*
    Mega Touch 4
    by Merit Industries

    Dumped by Hugh McLenaghan!
    on 08/29/2007


    System Info
    -----------
     This is a counter top Touch screen game.

    processor.. Z80
    sound processor- YM2149
    other chips- two Yamaha V9938
             one LM1203
             one PC165500N
             one PB255a or L5220574
             One Dallas DS1204 Data Key
             One dallas DS1235Y 32k Non-volitile RAM
             Two Z80APIO
*/

ROM_START( megat4te )
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD( "megat4.u32",  0x000000, 0x100000, CRC(08b1b8fe) SHA1(c562f2e065d6d7f753f6fd1d0b8355b01cb089ec) )
	ROM_LOAD( "megat4.u36",  0x100000, 0x080000, CRC(57322328) SHA1(12bc604c9d34cde431ef7cd2aa33c7b12ac01833) )
    ROM_RELOAD(              0x180000, 0x080000 )
	ROM_LOAD( "megat4.u37",  0x200000, 0x080000, CRC(f2e8bb4e) SHA1(5c5475b3c176a6aca9b2c6aa4aee422675d20bd1) )
	ROM_RELOAD(              0x280000, 0x080000 )
	ROM_LOAD( "megat4.u38",  0x300000, 0x080000, CRC(124d5b84) SHA1(3c2117f56d0dc406bfb508989729e36781e215a4) )
	ROM_RELOAD(              0x380000, 0x080000 )

    ROM_REGION( 0x8000, REGION_USER1, 0 ) // DS1644 nv ram
	ROM_LOAD( "ds1644.u31",  0x00000,  0x8000,   CRC(0908bc39) SHA1(e6ec6238d6bf5c802e046407c0f25a83b09f6135) )

ROM_END

/*
    Mega Touch 5
    by Merit Industries

    Dumped by NAZ!
    on 9/20/1998


    System Info
    -----------
     This is a counter top Touch screen game.

    processor.. Z80
    sound processor- YM2149
    other chips- two Yamaha V9938
             one LM1203
             one PC165500N
             one PB255a or L5220574
             One Dallas DS1204 Data Key
             One dallas DS1230Y 32k Non-volitile RAM
             Two Z80APIO
*/

ROM_START( megat5 )
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD( "megat5.u32",  0x000000, 0x80000, BAD_DUMP CRC(89932443) SHA1(68d2fbf2a5050fc5371595a105fe06f4276b0b67) )
	ROM_RELOAD(              0x080000, 0x80000)
	ROM_LOAD( "megat5.u36",  0x100000, 0x80000, BAD_DUMP CRC(0bed9e27) SHA1(1414385ce562b127e1ddeccc20ea4ff2a7098b7e) )
	ROM_RELOAD(              0x180000, 0x80000)
	ROM_LOAD( "megat5.u37",  0x200000, 0x80000, BAD_DUMP CRC(b713a1c5) SHA1(d6ccba2ea90fd0e2ecf15249514231eed54000c1) )
	ROM_RELOAD(              0x280000, 0x80000)
	ROM_LOAD( "megat5.u38",  0x300000, 0x80000, BAD_DUMP CRC(018e36c7) SHA1(8e9b457238a40b10d59887d13bac9c0a05c73614) )
	ROM_RELOAD(              0x380000, 0x80000)
ROM_END

/*

Megatouch 6 - Merit 1998
--------------------------------

CPU:
1 Z80
1 8255
1 16550
2 x Z80 PIO
2 x V9938

Memory:
2 x V53C8256HP45 256K X 8 Fast Page Mode CMOS DRAM
1 DS1230 nv ram

Sound:
YM2149

Actual rom labels:
------------------
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
 U36-R0      = 27C801
C1998 MII

PAL:
SC39440A.u19 = PALCE22V10H-25PC/4
SC3980.u40   = PALCE16V8H-25
SC39810A.u15 = PALCE16V8H-25
SC3943.u20   = ATF16V8B25PC

*/

ROM_START( megat6 )
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD( "u32-r0",  0x000000, 0x100000, CRC(f8f7f48e) SHA1(1bebe1f8898c60b795a0f794ca9b79e03d2744e4) )
	ROM_LOAD( "u36-r0",  0x100000, 0x080000, CRC(800f5a1f) SHA1(4d3ee6fb896d6452aab1f279a3ee878284bd1acc) )
	ROM_RELOAD(          0x180000, 0x080000 )
	ROM_LOAD( "u37-r0",  0x200000, 0x100000, CRC(5ba01949) SHA1(1598949ea18d07bbc78af0ddd279a687173c1229) )
	ROM_LOAD( "u38-r0",  0x300000, 0x100000, CRC(3df6b840) SHA1(31ba1ac04eed3e76cdf637507dedcc5f7e22c919) )

	ROM_REGION( 0x8000, REGION_USER1, 0 ) // DS1230 nv ram
	ROM_LOAD( "ds1230y.u31",  0x00000, 0x8000, CRC(51b6da5c) SHA1(1d53af89d7867bb48b9d46feff6fc3b7e8e80ac8) )

	ROM_REGION( 0x1000, REGION_USER2, 0 ) // PALs
	ROM_LOAD( "sc3943.u20.bin", 0x000, 0x117, CRC(5a72fe78) SHA1(4b1a36904eb7048518507fe14bdade5c2589dbd7) )
	ROM_LOAD( "sc3944-0a.bin",  0x000, 0x2dd, CRC(4cc46c5e) SHA1(0bab970df1539ce905f43603ad13171b05449a01) )
	ROM_LOAD( "sc3980.bin",     0x000, 0x117, CRC(ee0cdab5) SHA1(216fef50a8a0f6a33b704d3501a4c5c3cbac2bad) )
	ROM_LOAD( "sc39810a.bin",   0x000, 0x117, CRC(4fc750d0) SHA1(d09ff7a8c66aeb5c49e9fec84bd1521e3f5d8d0a) )
ROM_END

static DRIVER_INIT(megat3)
{
	static const UINT8 megat3_ds1204_key[8] =
		{ 0xf0, 0xaa, 0x0f, 0x0f, 0x55, 0x55, 0xff, 0xab };

	static const UINT8 megat3_ds1204_nvram[16] =
		{ 0x51, 0xa1, 0xc0, 0x7c, 0x27, 0x6e, 0x51, 0xb9, 0xa5, 0xb2, 0x27, 0x0c, 0xb9, 0x88, 0x82, 0x2c };

	ds1204_init(megat3_ds1204_key, megat3_ds1204_nvram);

	// patch for rom check (?) (Question Set Error)
	((UINT8*)memory_region( REGION_CPU1 ))[0x300217] = 0xc3;
};

static DRIVER_INIT(megat4)
{
	static const UINT8 megat4_ds1204_nvram[16] =
		{ 0xe3, 0x08, 0x39, 0xd8, 0x4c, 0xbb, 0xc4, 0xf8, 0xf0, 0xe2, 0xd8, 0x77, 0xa8, 0x3d, 0x95, 0x02 };

	ds1204_init(0, megat4_ds1204_nvram);
}

static DRIVER_INIT(megat4te)
{
	static const UINT8 megat4te_ds1204_nvram[16] =
		{ 0x05, 0x21, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(0, megat4te_ds1204_nvram);

	memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfff8, 0xffff, 0, 0, meritm_ds1644_r, meritm_ds1644_w );

};

static DRIVER_INIT(megat6)
{
	static const UINT8 megat6_ds1204_nvram[16] =
		{ 0x07, 0x15, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };

	ds1204_init(0, megat6_ds1204_nvram);

}
/* CRT 250 */
GAME( 1988, pitboss2,  0,    meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Pit Boss II", GAME_IMPERFECT_GRAPHICS )
GAME( 1988, spitboss,  0,    meritm_crt250, meritm_crt250, 0, ROT0, "Merit", "Super Pit Boss", GAME_IMPERFECT_GRAPHICS )
/* CRT 250 + question roms */
GAME( 1994, pitbossm,  0,      meritm_crt250_questions, pitbossm, 0, ROT0, "Merit", "Pit Boss Megastar", GAME_IMPERFECT_GRAPHICS )
/* CRT 260 */
GAME( 1995, megat3,    0,      meritm_crt260, meritm_crt260, megat3,   ROT0, "Merit", "Megatouch III", GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1996, megat4,    0,      meritm_crt260, meritm_crt260, megat4,   ROT0, "Merit", "Megatouch IV", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, megat4te,  0,      meritm_crt260, meritm_crt260, megat4te, ROT0, "Merit", "Megatouch IV Tournament Edition", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, megat5,    0,      meritm_crt260, meritm_crt260, 0,	       ROT0, "Merit", "Megatouch 5", GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1998, megat6,    0,      meritm_crt260, meritm_crt260, megat6,   ROT0, "Merit", "Megatouch 6", GAME_IMPERFECT_GRAPHICS )
