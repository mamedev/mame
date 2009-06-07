/************************************************************************

Eolith Ghost Hardware driver

1. CPU
- 16/32bit RISC Microprocessor(ARM920T CPU core)
- Separate 16KB Instruction and 16KB Data Cache
- Operating Frequency : Up to 203MHz

2. MEMORY
- Main Memory : 32Mbytes(Maximum 128Mbytes) SDRAM
- NAND FLASH : 16M,32M, 64M, 128M, 256Mbytes

3. VIDEO
- RGB (5:6:5) format
- Supports 1,2,4 or 8 bpp(bit-per-pixel) palette color displays
- Supports 16 bpp non-palette true-color displays
- Supports multiple screen size
Typical actual screen size : 320*240, 640*480
Maximum virtual screen size is 4Mbytes
Maximum virtual screen size in 64K color mode : 2048*1024
- Rendering Performance : 480 Objects per sec.(256*256 sprite)

4. SOUND
Internal Audio Engine or External System Available
TDA1519(Philips)Stereo Power AMP
- Internal Audio Engine : 1-ch IIS-bus for audio interface
Serial, 8-/16-bit per channel data transfers
128 Bytes (64-Byte + 64-Byte) FIFO for Tx/Rx
- External System(Option) : General MIDI Chipset QDSP1000
MIDI 16th Channel(32 Poly) using as Effect
Effect EPROM : 512Kbyte or 1Mbyte
MIDI Background Music EPROM:512Kbyte

5. Board Feature
- JAMMA Standard ( Application of PACK TYPE ) : 1P,2P ? Joystick (Up, Down, Left, Right), 4 Push Buttons, Start Button
- 1P,2P ? Coin, Coin Counter
- On Board Service Button, On Board Test Button
- Photo Gun : 1P, 2P Photo Gun (or Light Pen) (Option)
- ETC : Security System Available
Multi I/O Port Available
Coin Counter Available
Gun Shooting Effect Available (Option)
Hopper, Ticket Counter, Prize System (Option)

6. Develoment
- On-chip ICEbreaker debug support with JTAG-based debugging solution
- MultiICE, OPENICE etc.
- Compiler : ADS, SDT


*/

#include "driver.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

static UINT32 *system_memory;
static UINT32 *flash_regs;
static UINT32 *lcd_control;
static UINT32 *io_port;

//static UINT32 fifoh[16];
//static UINT32 fifol[12];

static struct lcd_config
{
	int screen_type;
	int bpp_mode;
	int lcd_bank;
	int lcd_base_u;
	int lcd_base_sel;
	int line_val;
	int hoz_val;
	int off_size;
	int page_width;
} lcd;

static READ32_HANDLER( r1 )
{
//  int pc = cpu_get_pc(space->cpu);
//  if(pc != 0x9a0 && pc != 0x7b4) printf("r1 @ %X\n",pc);

	return 1;
}

static READ32_HANDLER( r2 )
{
//  int pc = cpu_get_pc(space->cpu);
//  if(pc != 0xd64 && pc != 0xd3c )printf("r2 @ %X\n",pc);

	return 2;
}

static UINT32 flash_addr = 0;
static int flash_addr_step = 0;

#define ADDR_STEP_CONFIG 4

static WRITE32_HANDLER( flash_reg_w )
{
	COMBINE_DATA(&flash_regs[offset]);

	switch(offset)
	{
	case 0:
		//if((flash_regs[offset] & 0xff) != 0x60)
		//  printf("%08x\n",flash_regs[offset]);
		break;
	case 1:
		break;
	case 2:

		switch(flash_addr_step)
		{
			case 0: flash_addr  = data <<  0; break;
			case 1: flash_addr |= data <<  8; break;
			case 2: flash_addr |= data << 16; break;
			case 3: flash_addr |= data << 24; break;
		}

		if(flash_addr_step == (ADDR_STEP_CONFIG - 1))
		{
			flash_addr <<=1;
		}

		flash_addr_step = (flash_addr_step + 1) % ADDR_STEP_CONFIG;

		break;
	}
}

static READ32_HANDLER( flash_reg_r )
{
	// always read back what it's written?
	return flash_regs[offset];
}

static READ32_HANDLER( flash_r )
{
	UINT8 *flash = (UINT8 *)memory_region(space->machine, "user1");
	UINT8 value = flash[flash_addr];
	flash_addr = (flash_addr + 1) % memory_region_length(space->machine, "user1");
	return value;
}

static READ32_HANDLER( flash_ecc_r )
{
	//TODO

	if((flash_addr & 0x0fff) == 0)
	{
		//printf("%08x\n",flash_addr);
		return 1;
	}

	return 0;
}

static WRITE32_HANDLER( debug_w )
{
#if 1
	mame_printf_debug("%c",data & 0xff); //debug texts
#endif
}

/*
a) cpu #0 (PC=000007D0): unmapped program memory dword read from 4E000014 & 000000FF
b) cpu #0 (PC=000007D8): unmapped program memory dword read from 4E000014 & 0000FF00
c) cpu #0 (PC=000007E0): unmapped program memory dword read from 4E000014 & 00FF0000
1) read flash @ 0812
2) read flash @ 0813
3) read flash @ 0814
4) read flash @ 0815
5) read flash @ 0816
6) read flash @ 0817

a == 1
b == 2
c == 3
6 == 0xff

*/

// PC: 574 check -> jumps to execute code from ram

/*
Power management:
Normal, Idle, Slow & Power-off
4-ch 16 bit PWM ( Pulse Width Modulation), & 1-ch 16-bit timer for OS
RTC : 32.768 KHz, alarm interrupt
GPIO :117 (multiplexed I/O)
3-ch UARTs
4 ch DMA Controllers
8-ch 10-bit A/D (Max. 500KSPS), including TSP Controller
TFT LCD/STN LCD Controller (16bit, 640?480 maximum)
16-bit Watch-dog Timer
1-ch IIC-Bus Interface
IIS-Bus Interface
Screen Size: up to 640 x 480
2-ch SPI (Synchronous Serial I/O)
SD Host/MMC (Multi Media Card) Interface
USB Host/Device Interface
Debug & TEST
NAND Flash Controller (4KB internal buffer)
24-ch external interrupts Controller (Wake-up source 16-ch)
*/

static READ32_HANDLER( lcd_control_r )
{
	switch(offset)
	{
		case 0x00/4:
		{
			int line_val = video_screen_get_vpos(space->machine->primary_screen) & 0x3ff;
			return (lcd_control[offset] & ~(0x3ff << 18)) | ((lcd.line_val - line_val) << 18);
		}

		case 0x10/4:
		{
			//TODO: VSTATUS, HSTATUS

			static int VSTATUS = 0;
			VSTATUS ^= 0x18000;

			return (lcd_control[offset] & ~0x18000) | VSTATUS;
		}

		default:
		{
			return lcd_control[offset];
		}
	}
}

static WRITE32_HANDLER( lcd_control_w )
{
	COMBINE_DATA(&lcd_control[offset]);

	switch(offset)
	{
		case 0x00/4:
		{
			int line_val = video_screen_get_vpos(space->machine->primary_screen) & 0x3ff;
			int bpp_mode = (lcd_control[offset] & 0x1e) >> 1;
			int screen_type = (lcd_control[offset] & 0x60) >> 5;

			lcd.screen_type = screen_type;
			lcd.bpp_mode = bpp_mode;

			if(bpp_mode != 12)
				printf("bpp mode= %d\n",bpp_mode);

			lcd_control[offset] = (lcd_control[offset] & ~(0x3ff << 18)) | ((lcd.line_val - line_val) << 18);

			break;
		}

		case 0x04/4:
		{
			int line_val = (lcd_control[offset] >> 14) & 0x3ff;

			lcd.line_val = line_val;

			break;
		}

		case 0x08/4:
		{
			int hoz_val = (lcd_control[offset] >> 8) & 0x3ff;

			lcd.hoz_val = hoz_val;

			break;
		}

		case 0x10/4:
		{
			/*
            int frm565 = (lcd_control[offset] >> 11) & 1;
            int inv_vd = (lcd_control[offset] >> 7) & 1;
            int bs_swp = (lcd_control[offset] >> 1) & 1;
            int hw_swp = (lcd_control[offset] >> 0) & 1;
            */

			break;
		}

		case 0x14/4:
		{
			UINT32 lcd_bank = (lcd_control[offset] >> 21) & 0x1ff;
			UINT32 lcd_base_u = (lcd_control[offset] >> 0) & 0x1fffff;

			lcd.lcd_bank = lcd_bank;
			lcd.lcd_base_u = lcd_base_u;

			break;
		}

		case 0x18/4:
		{
			UINT32 lcd_base_sel = (lcd_control[offset] >> 0) & 0x1fffff;

			lcd.lcd_base_sel = lcd_base_sel;

			//popmessage("%08x",lcd.lcd_base_sel);

			break;
		}

		case 0x1c/4:
		{
			int off_size = (lcd_control[offset] >> 11) & 0x7ff;
			int page_width = (lcd_control[offset] >> 0) & 0x7ff;

			lcd.off_size = off_size;
			lcd.page_width = page_width;

			break;
		}
	}
}


static READ32_HANDLER( io_port_r )
{
//  printf("%08x\n",offset*4);

	switch(offset)
	{

		case 0x64/4:
		{
			static int input = 0;
			input ^= 0x20;

			// bits from 15-0 can be configured as input/output/...
			//return input;

			return (io_port[offset] & ~0x20) | input;
		}

		default:
		{
			return io_port[offset];
		}
	}
}

static WRITE32_HANDLER( io_port_w )
{
	COMBINE_DATA(&io_port[offset]);
//  printf("[%08x] <- %08x\n",offset*4,data);
}


static ADDRESS_MAP_START( bballoon_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000fff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x30000000, 0x31ffffff) AM_RAM AM_BASE(&system_memory)
	AM_RANGE(0x40000000, 0x40000fff) AM_ROM AM_REGION("user1", 0) // mirror? seems so
	AM_RANGE(0x48000000, 0x48000033) AM_RAM // memory controller

	AM_RANGE(0x4d000000, 0x4d00001f) AM_READWRITE(lcd_control_r,lcd_control_w) AM_BASE(&lcd_control)

	AM_RANGE(0x4e000000, 0x4e00000b) AM_READWRITE(flash_reg_r, flash_reg_w) AM_BASE(&flash_regs)
	AM_RANGE(0x4e00000c, 0x4e00000f) AM_READ(flash_r)
	AM_RANGE(0x4e000010, 0x4e000013) AM_READ(r1)
	AM_RANGE(0x4e000014, 0x4e000017) AM_READ(flash_ecc_r)

	AM_RANGE(0x50000010, 0x50000013) AM_READ(r2)

	AM_RANGE(0x50000020, 0x50000023) AM_WRITE(debug_w)
	//AM_RANGE(0x51000040, 0x51000043) AM_READ() // a timer...

	AM_RANGE(0x56000000, 0x56000093) AM_READWRITE(io_port_r, io_port_w) AM_BASE(&io_port)

ADDRESS_MAP_END

static INPUT_PORTS_START( bballoon )
INPUT_PORTS_END

static int irq_en = 0;

static VIDEO_START( bballoon )
{

}
static int b=0;
static VIDEO_UPDATE( bballoon )
{
	if(input_code_pressed_once(KEYCODE_Q))
	{
		irq_en ^= 1;
		printf("en = %d\n",irq_en);
	}

	if(input_code_pressed(KEYCODE_W))
	{

		printf("b = %d\n",++b);
	}

	if(input_code_pressed(KEYCODE_E))
	{

		printf("b = %d\n",--b);
	}

	if(lcd_control[0] & 1)
	{
		// output enabled

		//bballoon setup
		//TFT 16bpp
		//5:6:5 format
		//normal
		//bs swap disabled
		//hw swap enabled

		int start_addr = (lcd.lcd_bank << 22) - 0x30000000;

		//popmessage("%08x %08x %08x",lcd.lcd_bank,lcd.lcd_base_sel,lcd.lcd_base_u);

		if(start_addr > 0x1ffffff)
			printf("max = %X\n",start_addr);
		else
		{
			UINT32* videoram = system_memory + start_addr/4 + lcd.lcd_base_u/4;
			int x,y,count;

			//popmessage("%08x %08x %d",lcd.lcd_base_u,lcd.lcd_base_sel,test);

			/*temp until I understand...*/
			switch(lcd.lcd_base_sel)
			{
				case 0x1aac00: count = -81920; break;
				case 0x192c00: count = -57344; break;
				default: count = 0;
			}
			//count = test;//lcd.lcd_base_sel/4;

			for (y=0;y <= 600;y++)
			{
				for (x=0;x < 400;x++)
				{
					UINT32 color;
					UINT32 b;
					UINT32 g;
					UINT32 r;

					color = (videoram[count] >> 16) & 0xffff;

					b = (color & 0x001f) << 3;
					g = (color & 0x07e0) >> 3;
					r = (color & 0xf800) >> 8;
					if(((x*2)+1)<cliprect->max_x && y<cliprect->max_y)
					*BITMAP_ADDR32(bitmap, y, x*2+1) = b | (g<<8) | (r<<16);

					color = videoram[count] & 0xffff;

					b = (color & 0x001f) << 3;
					g = (color & 0x07e0) >> 3;
					r = (color & 0xf800) >> 8;
					if(((x*2)+0)<cliprect->max_x && y<cliprect->max_y)
					*BITMAP_ADDR32(bitmap, y, x*2+0) = b | (g<<8) | (r<<16);

					count++;
				}
			}
		}

	}
	else
	{
		// output disabled
	}

	return 0;
}

static INTERRUPT_GEN( bballoon_interrupt )
{
	if(irq_en)
	cpu_set_input_line(device, ARM7_IRQ_LINE, HOLD_LINE);
	//cpu_set_input_line(device, ARM7_FIRQ_LINE, HOLD_LINE);

//  irq_en = 0;
}

static MACHINE_DRIVER_START( bballoon )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM7, 24000000)
	MDRV_CPU_PROGRAM_MAP(bballoon_map)
	MDRV_CPU_VBLANK_INT("screen", bballoon_interrupt)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
//  MDRV_SCREEN_SIZE(1024, 1024)
//  MDRV_SCREEN_VISIBLE_AREA(0, 1023, 0, 1023)

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(bballoon)
	MDRV_VIDEO_UPDATE(bballoon)

	/* sound hardware */
MACHINE_DRIVER_END

/*
Balloon & Balloon
Eolith, 2003

PCB Layout
----------

GHOST Ver1.1 2003.03.28
|---------------------------------------------|
|TDA1519A              B2.U20  LED      RESET |
|         QS1000                              |
|DA1311A            24MHz                  LED|
|                                             |
|                                             |
|                  |---------|                |
|   QS1001A.U17    |ARM 312  |                |
|J                 |SAMSUNG  |                |
|A       B1.U16    |S3C2410  |   IC42S16800   |
|M                 |         |                |
|M                 |---------|                |
|A                                            |
|                                IC42S16800   |
|                            12MHz            |
|                    M24CL16-S                |
|                                             |
|                                             |
|                             *       FLASH.U1|
|      SERVICE                                |
|DSW1  TEST                                   |
|---------------------------------------------|
Notes:
      *          - Unknown SOIC20 chip, surface scratched
      FLASH.U1   - Samsung K9F5608U0B 256MBit NAND FlashROM (TSOP48)
      U16        - 27C801 8MBit EPROM
      U20        - 29F040 EEPROM
      IC42S16800 - ISSI IC42S16800 4(2)M x 8(16) Bits x 4 Banks (128MBit) SDRAM

      QS1000     - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
                   see http://www.hwass.co.kr/product.htm for more info on QS100x chips.
      QS1001A    - QDSP QS1001A 512k x8 MaskROM (SOP32)

      qs1001a.u17 was not dumped from this PCB, but is a standard sample rom found on many Eolith games
                  see eolith.c and vegaeo.c drivers
*/

ROM_START( bballoon )
	ROM_REGION( 0x2000000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "flash.u1",     0x000000, 0x2000000, CRC(73285634) SHA1(4d0210c1bebdf3113a99978ffbcd77d6ee854168) )

	// banked every 0x10000 bytes ?
	ROM_REGION( 0x080000, "user2", 0 )
	ROM_LOAD( "b2.u20",       0x000000, 0x080000, CRC(0a12334c) SHA1(535b5b34f28435517218100d70147d87809f485a) )

	ROM_REGION( 0x100000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "b1.u16",       0x000000, 0x100000, CRC(c42c1c85) SHA1(e1f49d556ffd6bc27142a7784c3bb8e37999857d) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u17",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

static DRIVER_INIT( bballoon )
{
	UINT8 *flash = (UINT8 *)memory_region(machine, "user1");

	// nop flash ECC checks

	flash[0x844+0] = 0x00;
	flash[0x844+1] = 0x00;
	flash[0x844+2] = 0xA0;
	flash[0x844+3] = 0xE1;

	flash[0x850+0] = 0x00;
	flash[0x850+1] = 0x00;
	flash[0x850+2] = 0xA0;
	flash[0x850+3] = 0xE1;

	flash[0x860+0] = 0x00;
	flash[0x860+1] = 0x00;
	flash[0x860+2] = 0xA0;
	flash[0x860+3] = 0xE1;

	flash[0x86c+0] = 0x00;
	flash[0x86c+1] = 0x00;
	flash[0x86c+2] = 0xA0;
	flash[0x86c+3] = 0xE1;
}

GAME( 2003, bballoon, 0, bballoon, bballoon, bballoon, ROT0, "Eolith", "Balloon & Balloon", GAME_NO_SOUND | GAME_NOT_WORKING )
