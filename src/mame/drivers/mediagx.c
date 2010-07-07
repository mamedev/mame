/*  Atari MediaGX

    Driver by Ville Linde

*/

/*
    Main Board number: P5GX-LG REV:1.1

    Components list on mainboard, clockwise from top left.
    ----------------------------
    SST29EE020 (2 Megabyte EEPROM, PLCC32, surface mounted, labelled 'PhoenixBIOS PENTIUM(tm) CPU (c)PHOENIX 1992-1993')
    Y1: XTAL KDS7M (To pin 122 of FDC37C932)
    SMC FDC37C932 (QFP160)
    Cyrix CX5510Rev2 (QFP208)
    40 PIN IDE CONNECTOR
    2.5 GIGABYTE IDE HARD DRIVE (Quantum EL2.5AT, CHS: 5300,15,63)
    74F245 (x3, SOIC20)
    78AT3TK HC14 (SOIC14)
    ICS9159M 9743-14 (SOIC28)
    Y2: XTAL 14.3N98F
    Y3: XTAL KDS7M
    MT4C1M16E5DJ-6 (x8, located on 2x 72 PIN SIMM Memory modules)
    CPU (?, located centrally on board, large QFP)
    ICS GENDAC ICS5342-3 (PLCC68, surface mounted)
    74HCT14 (SOIC14)
    ANALOG DEVICES AD1847JP 'SOUNDPORT' (PLCC44, surface mounted)
    AD 706 (SOIC8)
    15 PIN VGA CONNECTOR (DSUB15, plugged into custom Atari board)
    25 PIN PARALLEL CONNECTOR (DSUB25, plugged into custom Atari board)
    ICS9120 (SOIC8)
    AD706 (SOIC8)
    NE558 (SOIC16)
    74F245 (SOIC20)
    LGS GD75232 (x2, SOIC20)
    9 PIN SERIAL (x2, DSUB9, both not connected)
    PS-2 Keyboard & Mouse connectors (not connected)
    AT Power Supply Power Connectors (connected to custom Atari board)
    3 Volt Coin Battery
    74HCT32 (SOIC14)


    Custom Atari board number: ATARI GAMES INC. 5772-15642-02 JAMMIT

    Components list on custom Atari board, clockwise from top left.
    -------------------------------------
    DS1232 (SOIC8)
    74F244 (x4, SOIC20)
    3 PIN JUMPER for sound OUT (Set to MONO, alternative setting is STEREO)
    4 PIN POWER CONNECTOR for IDE Hard Drive
    15 PIN VGA CONNECTOR (DSUB15, plugged into MAIN board)
    25 PIN PARALLEL CONNECTOR (DSUB25, plugged into MAIN board)
    AD 813AR (x2, SOIC14)
    ST 084C P1S735 (x2, SOIC14)
    74F244 (SOIC20)
    7AAY2JK LS86A (SOIC14)
    74F07 (SOIC14)
    3 PIN JUMPER for SYNC (Set to -, alternative setting is +)
    OSC 14.31818MHz
    ACTEL A42MX09 (PLCC84, labelled 'JAMMINT U6 A-22505 (C)1996 ATARI')
    LS14 (SOIC14)
    74F244 (x2, SOIC20)
    ST ULN2064B (DIP16)
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "sound/dmadac.h"

#define SPEEDUP_HACKS	1

static UINT32 *cga_ram;
static UINT32 *bios_ram;
static UINT32 *vram;
static UINT8 pal[768];

static UINT32 *main_ram;

static UINT32 disp_ctrl_reg[256/4];
static int frame_width;
static int frame_height;

static UINT32 memory_ctrl_reg[256/4];
static int pal_index = 0;

static UINT32 biu_ctrl_reg[256/4];

static UINT8 mediagx_config_reg_sel;
static UINT8 mediagx_config_regs[256];

//static UINT8 controls_data = 0;
static UINT8 parallel_pointer;
static UINT8 parallel_latched;
static UINT32 parport;
//static int control_num = 0;
//static int control_num2 = 0;
//static int control_read = 0;

static UINT32 cx5510_regs[256/4];

static INT16 *dacl;
static INT16 *dacr;
static int dacl_ptr = 0;
static int dacr_ptr = 0;

static UINT8 ad1847_regs[16];
static UINT32 ad1847_sample_counter = 0;
static UINT32 ad1847_sample_rate;

static dmadac_sound_device *dmadac[2];

static struct {
	pit8254_device	*pit8254;
	pic8259_device	*pic8259_1;
	pic8259_device	*pic8259_2;
	i8237_device	*dma8237_1;
	i8237_device	*dma8237_2;
} mediagx_devices;


// Display controller registers
#define DC_UNLOCK				0x00/4
#define DC_GENERAL_CFG			0x04/4
#define DC_TIMING_CFG			0x08/4
#define DC_OUTPUT_CFG			0x0c/4
#define DC_FB_ST_OFFSET			0x10/4
#define DC_CB_ST_OFFSET			0x14/4
#define DC_CUR_ST_OFFSET		0x18/4
#define DC_VID_ST_OFFSET		0x20/4
#define DC_LINE_DELTA			0x24/4
#define DC_BUF_SIZE				0x28/4
#define DC_H_TIMING_1			0x30/4
#define DC_H_TIMING_2			0x34/4
#define DC_H_TIMING_3			0x38/4
#define DC_FP_H_TIMING			0x3c/4
#define DC_V_TIMING_1			0x40/4
#define DC_V_TIMING_2			0x44/4
#define DC_V_TIMING_3			0x48/4
#define DC_FP_V_TIMING			0x4c/4
#define DC_CURSOR_X				0x50/4
#define DC_V_LINE_CNT			0x54/4
#define DC_CURSOR_Y				0x58/4
#define DC_SS_LINE_CMP			0x5c/4
#define DC_PAL_ADDRESS			0x70/4
#define DC_PAL_DATA				0x74/4
#define DC_DFIFO_DIAG			0x78/4
#define DC_CFIFO_DIAG			0x7c/4


static void ide_interrupt(running_device *device, int state);




static const rgb_t cga_palette[16] =
{
	MAKE_RGB( 0x00, 0x00, 0x00 ), MAKE_RGB( 0x00, 0x00, 0xaa ), MAKE_RGB( 0x00, 0xaa, 0x00 ), MAKE_RGB( 0x00, 0xaa, 0xaa ),
	MAKE_RGB( 0xaa, 0x00, 0x00 ), MAKE_RGB( 0xaa, 0x00, 0xaa ), MAKE_RGB( 0xaa, 0x55, 0x00 ), MAKE_RGB( 0xaa, 0xaa, 0xaa ),
	MAKE_RGB( 0x55, 0x55, 0x55 ), MAKE_RGB( 0x55, 0x55, 0xff ), MAKE_RGB( 0x55, 0xff, 0x55 ), MAKE_RGB( 0x55, 0xff, 0xff ),
	MAKE_RGB( 0xff, 0x55, 0x55 ), MAKE_RGB( 0xff, 0x55, 0xff ), MAKE_RGB( 0xff, 0xff, 0x55 ), MAKE_RGB( 0xff, 0xff, 0xff ),
};

static VIDEO_START(mediagx)
{
	int i;
	for (i=0; i < 16; i++)
	{
		palette_set_color(machine, i, cga_palette[i]);
	}
}

static void draw_char(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx, int ch, int att, int x, int y)
{
	int i,j;
	const UINT8 *dp;
	int index = 0;
	const pen_t *pens = gfx->machine->pens;

	dp = gfx_element_get_data(gfx, ch);

	for (j=y; j < y+8; j++)
	{
		UINT32 *p = BITMAP_ADDR32(bitmap, j, 0);
		for (i=x; i < x+8; i++)
		{
			UINT8 pen = dp[index++];
			if (pen)
				p[i] = pens[gfx->color_base + (att & 0xf)];
			else
			{
				if (((att >> 4) & 7) > 0)
					p[i] = pens[gfx->color_base + ((att >> 4) & 0x7)];
			}
		}
	}
}

static void draw_framebuffer(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int i, j;
	int width, height;
	int line_delta = (disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) * 4;

	width = (disp_ctrl_reg[DC_H_TIMING_1] & 0x7ff) + 1;
	if (disp_ctrl_reg[DC_TIMING_CFG] & 0x8000)		// pixel double
	{
		width >>= 1;
	}
	width += 4;

	height = (disp_ctrl_reg[DC_V_TIMING_1] & 0x7ff) + 1;

	if ( (width != frame_width || height != frame_height) &&
		 (width > 1 && height > 1 && width <= 640 && height <= 480) )
	{
		rectangle visarea;

		frame_width = width;
		frame_height = height;

		visarea.min_x = visarea.min_y = 0;
		visarea.max_x = width - 1;
		visarea.max_y = height - 1;
		machine->primary_screen->configure(width, height * 262 / 240, visarea, machine->primary_screen->frame_period().attoseconds);
	}

	if (disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)		// 8-bit mode
	{
		UINT8 *framebuf = (UINT8*)&vram[disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		for (j=0; j < frame_height; j++)
		{
			UINT32 *p = BITMAP_ADDR32(bitmap, j, 0);
			UINT8 *si = &framebuf[j * line_delta];
			for (i=0; i < frame_width; i++)
			{
				int c = *si++;
				int r = pal[(c*3)+0] << 2;
				int g = pal[(c*3)+1] << 2;
				int b = pal[(c*3)+2] << 2;

				p[i] = r << 16 | g << 8 | b;
			}
		}
	}
	else			// 16-bit
	{
		UINT16 *framebuf = (UINT16*)&vram[disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		// RGB 5-6-5 mode
		if ((disp_ctrl_reg[DC_OUTPUT_CFG] & 0x2) == 0)
		{
			for (j=0; j < frame_height; j++)
			{
				UINT32 *p = BITMAP_ADDR32(bitmap, j, 0);
				UINT16 *si = &framebuf[j * (line_delta/2)];
				for (i=0; i < frame_width; i++)
				{
					UINT16 c = *si++;
					int r = ((c >> 11) & 0x1f) << 3;
					int g = ((c >> 5) & 0x3f) << 2;
					int b = (c & 0x1f) << 3;

					p[i] = r << 16 | g << 8 | b;
				}
			}
		}
		// RGB 5-5-5 mode
		else
		{
			for (j=0; j < frame_height; j++)
			{
				UINT32 *p = BITMAP_ADDR32(bitmap, j, 0);
				UINT16 *si = &framebuf[j * (line_delta/2)];
				for (i=0; i < frame_width; i++)
				{
					UINT16 c = *si++;
					int r = ((c >> 10) & 0x1f) << 3;
					int g = ((c >> 5) & 0x1f) << 3;
					int b = (c & 0x1f) << 3;

					p[i] = r << 16 | g << 8 | b;
				}
			}
		}
	}
}

static void draw_cga(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int i, j;
	const gfx_element *gfx = machine->gfx[0];
	UINT32 *cga = cga_ram;
	int index = 0;

	for (j=0; j < 25; j++)
	{
		for (i=0; i < 80; i+=2)
		{
			int att0 = (cga[index] >> 8) & 0xff;
			int ch0 = (cga[index] >> 0) & 0xff;
			int att1 = (cga[index] >> 24) & 0xff;
			int ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
		}
	}
}

static VIDEO_UPDATE(mediagx)
{
	bitmap_fill(bitmap, cliprect, 0);

	draw_framebuffer(screen->machine, bitmap, cliprect);

	if (disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)	// don't show MDA text screen on 16-bit mode. this is basically a hack
	{
		draw_cga(screen->machine, bitmap, cliprect);
	}
	return 0;
}

static READ32_HANDLER( disp_ctrl_r )
{
	UINT32 r = disp_ctrl_reg[offset];

	switch (offset)
	{
		case DC_TIMING_CFG:
			r |= 0x40000000;

			if (space->machine->primary_screen->vpos() >= frame_height)
				r &= ~0x40000000;

#if SPEEDUP_HACKS
			// wait for vblank speedup
			cpu_spinuntil_int(space->cpu);
#endif
			break;
	}

	return r;
}

static WRITE32_HANDLER( disp_ctrl_w )
{
	COMBINE_DATA(disp_ctrl_reg + offset);
}


static READ8_DEVICE_HANDLER(at_dma8237_2_r)
{
	return i8237_r(device, offset / 2);
}

static WRITE8_DEVICE_HANDLER(at_dma8237_2_w)
{
	i8237_w(device, offset / 2, data);
}


static READ32_DEVICE_HANDLER( ide_r )
{
	return ide_controller32_r(device, 0x1f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( ide_w )
{
	ide_controller32_w(device, 0x1f0/4 + offset, data, mem_mask);
}

static READ32_DEVICE_HANDLER( fdc_r )
{
	return ide_controller32_r(device, 0x3f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( fdc_w )
{
	ide_controller32_w(device, 0x3f0/4 + offset, data, mem_mask);
}



static READ32_HANDLER( memory_ctrl_r )
{
	return memory_ctrl_reg[offset];
}

static WRITE32_HANDLER( memory_ctrl_w )
{
//  mame_printf_debug("memory_ctrl_w %08X, %08X, %08X\n", data, offset, mem_mask);

	if (offset == 7)
	{
		pal_index = 0;
	}
	else if (offset == 8)
	{
		pal[pal_index] = data & 0xff;
		pal_index++;
		if (pal_index >= 768)
		{
			pal_index = 0;
		}
	}
	else
	{
		COMBINE_DATA(memory_ctrl_reg + offset);
	}
}



static READ32_HANDLER( biu_ctrl_r )
{
	if (offset == 0)
	{
		return 0xffffff;
	}
	return biu_ctrl_reg[offset];
}

static WRITE32_HANDLER( biu_ctrl_w )
{
//  mame_printf_debug("biu_ctrl_w %08X, %08X, %08X\n", data, offset, mem_mask);
	COMBINE_DATA(biu_ctrl_reg + offset);

	if (offset == 3)		// BC_XMAP_3 register
	{
//      mame_printf_debug("BC_XMAP_3: %08X, %08X, %08X\n", data, offset, mem_mask);
	}
}

#ifdef UNUSED_FUNCTION
static WRITE32_HANDLER(bios_ram_w)
{

}
#endif

static UINT8 mediagx_config_reg_r(void)
{
//  mame_printf_debug("mediagx_config_reg_r %02X\n", mediagx_config_reg_sel);
	return mediagx_config_regs[mediagx_config_reg_sel];
}

static void mediagx_config_reg_w(UINT8 data)
{
//  mame_printf_debug("mediagx_config_reg_w %02X, %02X\n", mediagx_config_reg_sel, data);
	mediagx_config_regs[mediagx_config_reg_sel] = data;
}

static READ8_DEVICE_HANDLER( io20_r )
{
	UINT8 r = 0;

	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x02)
	{
	}
	else if (offset == 0x03)
	{
		r = mediagx_config_reg_r();
	}
	else
	{
		r = pic8259_r(device, offset);
	}
	return r;
}

static WRITE8_DEVICE_HANDLER( io20_w )
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x02)
	{
		mediagx_config_reg_sel = data;
	}
	else if (offset == 0x03)
	{
		mediagx_config_reg_w(data);
	}
	else
	{
		pic8259_w(device, offset, data);
	}
}

static READ32_HANDLER( parallel_port_r )
{
	UINT32 r = 0;
	//static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7", "IN8" }; // but parallel_pointer takes values 0 -> 23

	if (ACCESSING_BITS_8_15)
	{
		UINT8 nibble = parallel_latched;//(input_port_read_safe(space->machine, portnames[parallel_pointer / 3], 0) >> (4 * (parallel_pointer % 3))) & 15;
		r |= ((~nibble & 0x08) << 12) | ((nibble & 0x07) << 11);
		logerror("%08X:parallel_port_r()\n", cpu_get_pc(space->cpu));
/*      if (controls_data == 0x18)
        {
            r |= input_port_read(space->machine, "IN0") << 8;
        }
        else if (controls_data == 0x60)
        {
            r |= input_port_read(space->machine, "IN1") << 8;
        }
        else if (controls_data == 0xff ||  controls_data == 0x50)
        {
            r |= input_port_read(space->machine, "IN2") << 8;
        }

        //r |= control_read << 8;*/
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= parport & 0xff0000;
	}

	return r;
}

static WRITE32_HANDLER( parallel_port_w )
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7", "IN8" };	// but parallel_pointer takes values 0 -> 23

	COMBINE_DATA( &parport );

	if (ACCESSING_BITS_0_7)
	{
		/*
            Controls:

                18 = reset internal pointer to 0
                19 = reset internal pointer to 1
                1a = reset internal pointer to 2
                1b = reset internal pointer to 3
                2x = set low 4 bits of general purpose output to 'x'
                3x = set high 4 bits of general purpose output to 'x'
                4x = control up to 4 coin counters; each bit of 'x' controls one
                5x = control up to 2 watchdogged outputs (kickers); bits D0-D1 control each one
                6x = watchdog reset
                7x..ff = advance pointer
        */

		logerror("%08X:", cpu_get_pc(space->cpu));

		parallel_latched = (input_port_read_safe(space->machine, portnames[parallel_pointer / 3], 0) >> (4 * (parallel_pointer % 3))) & 15;
//      parallel_pointer++;
//      logerror("[%02X] Advance pointer to %d\n", data, parallel_pointer);
		switch (data & 0xfc)
		{
			case 0x18:
				parallel_pointer = data & 3;
				logerror("[%02X] Reset pointer to %d\n", data, parallel_pointer);
				break;

			case 0x20:
			case 0x24:
			case 0x28:
			case 0x2c:
				logerror("[%02X] General purpose output = x%X\n", data, data & 0x0f);
				break;

			case 0x30:
			case 0x34:
			case 0x38:
			case 0x3c:
				logerror("[%02X] General purpose output = %Xx\n", data, data & 0x0f);
				break;

			case 0x40:
			case 0x44:
			case 0x48:
			case 0x4c:
				logerror("[%02X] Coin counters = %d%d%d%d\n", data, (data >> 3) & 1, (data >> 2) & 1, (data >> 1) & 1, data & 1);
				break;

			case 0x50:
			case 0x54:
			case 0x58:
			case 0x5c:
				logerror("[%02X] Kickers = %d%d\n", data, (data >> 1) & 1, data & 1);
				break;

			case 0x60:
			case 0x64:
			case 0x68:
			case 0x6c:
				logerror("[%02X] Watchdog reset\n", data);
				break;

			default:
				if (data >= 0x70)
				{
					parallel_pointer++;
					logerror("[%02X] Advance pointer to %d\n", data, parallel_pointer);
				}
				else
					logerror("[%02X] Unknown write\n", data);
				break;
		}
	}
}

static UINT32 cx5510_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
//  mame_printf_debug("CX5510: PCI read %d, %02X, %08X\n", function, reg, mem_mask);

	switch (reg)
	{
		case 0:		return 0x00001078;
	}

	return cx5510_regs[reg/4];
}

static void cx5510_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
//  mame_printf_debug("CX5510: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(cx5510_regs + (reg/4));
}

/* Analog Devices AD1847 Stereo DAC */

static TIMER_DEVICE_CALLBACK( sound_timer_callback )
{
	ad1847_sample_counter = 0;
	timer.adjust(ATTOTIME_IN_MSEC(10));

	dmadac_transfer(&dmadac[0], 1, 0, 1, dacl_ptr, dacl);
	dmadac_transfer(&dmadac[1], 1, 0, 1, dacr_ptr, dacr);

	dacl_ptr = 0;
	dacr_ptr = 0;
}

static void ad1847_reg_write(running_machine *machine, int reg, UINT8 data)
{
	static const int divide_factor[] = { 3072, 1536, 896, 768, 448, 384, 512, 2560 };

	switch (reg)
	{
		case 8:		// Data format register
		{
			if (data & 0x1)
			{
				ad1847_sample_rate = 16934400 / divide_factor[(data >> 1) & 0x7];
			}
			else
			{
				ad1847_sample_rate = 24576000 / divide_factor[(data >> 1) & 0x7];
			}

			dmadac_set_frequency(&dmadac[0], 2, ad1847_sample_rate);

			if (data & 0x20)
			{
				fatalerror("AD1847: Companded data not supported");
			}
			if ((data & 0x40) == 0)
			{
				fatalerror("AD1847: 8-bit data not supported");
			}
			break;
		}

		default:
		{
			ad1847_regs[reg] = data;
			break;
		}
	}
}

static READ32_HANDLER( ad1847_r )
{
	switch (offset)
	{
		case 0x14/4:
			return ((ad1847_sample_rate) / 100) - ad1847_sample_counter;
	}
	return 0;
}

static WRITE32_HANDLER( ad1847_w )
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_16_31)
		{
			UINT16 ldata = (data >> 16) & 0xffff;
			dacl[dacl_ptr++] = ldata;
		}
		if (ACCESSING_BITS_0_15)
		{
			UINT16 rdata = data & 0xffff;
			dacr[dacr_ptr++] = rdata;
		}

		ad1847_sample_counter++;
	}
	else if (offset == 3)
	{
		int reg = (data >> 8) & 0xf;
		ad1847_reg_write(space->machine, reg, data & 0xff);
	}
}


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];


static READ8_HANDLER(at_page8_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(at_page8_w)
{
	at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	return memory_read_byte(space, page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	memory_write_byte(space, page_offset + offset, data);
}

static void set_dma_channel(running_device *device, int channel, int state)
{
	if (!state) dma_channel = channel;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { set_dma_channel(device, 0, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { set_dma_channel(device, 1, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { set_dma_channel(device, 2, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { set_dma_channel(device, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_LINE(pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_LINE(pc_dack0_w), DEVCB_LINE(pc_dack1_w), DEVCB_LINE(pc_dack2_w), DEVCB_LINE(pc_dack3_w) }
};

static I8237_INTERFACE( dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};


/*****************************************************************************/

static ADDRESS_MAP_START( mediagx_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM AM_BASE(&main_ram)
	AM_RANGE(0x000a0000, 0x000affff) AM_RAM
	AM_RANGE(0x000b0000, 0x000b7fff) AM_RAM AM_BASE(&cga_ram)
	AM_RANGE(0x000c0000, 0x000fffff) AM_RAM AM_BASE(&bios_ram)
	AM_RANGE(0x00100000, 0x00ffffff) AM_RAM
	AM_RANGE(0x40008000, 0x400080ff) AM_READWRITE(biu_ctrl_r, biu_ctrl_w)
	AM_RANGE(0x40008300, 0x400083ff) AM_READWRITE(disp_ctrl_r, disp_ctrl_w)
	AM_RANGE(0x40008400, 0x400084ff) AM_READWRITE(memory_ctrl_r, memory_ctrl_w)
	AM_RANGE(0x40800000, 0x40bfffff) AM_RAM AM_BASE(&vram)
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(mediagx_io, ADDRESS_SPACE_IO, 32)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_master", io20_r, io20_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port32le_r,		mc146818_port32le_w)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r,				at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_slave", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP		// I/O delay port
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_r, ide_w)
	AM_RANGE(0x0378, 0x037b) AM_READWRITE(parallel_port_r, parallel_port_w)
	AM_RANGE(0x03f0, 0x03ff) AM_DEVREADWRITE("ide", fdc_r, fdc_w)
	AM_RANGE(0x0400, 0x04ff) AM_READWRITE(ad1847_r, ad1847_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_32le_r,	pci_32le_w)
ADDRESS_MAP_END

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,					/* 8 x 16 characters */
    256,                    /* 256 characters */
    1,                      /* 1 bits per pixel */
    { 0 },                  /* no bitplanes; 1 bit per pixel */
    /* x offsets */
    { 0,1,2,3,4,5,6,7 },
    /* y offsets */
	{ 0*8,1*8,2*8,3*8,
	  4*8,5*8,6*8,7*8 },
    8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( CGA )
/* Support up to four CGA fonts */
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout,              0, 256 )   /* Font 0 */
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout,              0, 256 )   /* Font 1 */
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout,              0, 256 )   /* Font 2 */
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout,              0, 256 )   /* Font 3*/
GFXDECODE_END

static INPUT_PORTS_START(mediagx)
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x001, IP_ACTIVE_HIGH )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x800, IP_ACTIVE_HIGH, IPT_START4 )

	PORT_START("IN1")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("IN2")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON6 )

	PORT_START("IN3")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON9 )

	PORT_START("IN4")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN5")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)

	PORT_START("IN6")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN7")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN8")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
INPUT_PORTS_END

static IRQ_CALLBACK(irq_callback)
{
	int r;
	r = pic8259_acknowledge( mediagx_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge( mediagx_devices.pic8259_1);
	}
	return r;
}

static MACHINE_START(mediagx)
{
	mediagx_devices.pit8254 = machine->device<pit8254_device>( "pit8254" );
	mediagx_devices.pic8259_1 = machine->device<pic8259_device>( "pic8259_master" );
	mediagx_devices.pic8259_2 = machine->device<pic8259_device>( "pic8259_slave" );
	mediagx_devices.dma8237_1 = machine->device<i8237_device>( "dma8237_1" );
	mediagx_devices.dma8237_2 = machine->device<i8237_device>( "dma8237_2" );

	dacl = auto_alloc_array(machine, INT16, 65536);
	dacr = auto_alloc_array(machine, INT16, 65536);
}

static MACHINE_RESET(mediagx)
{
	UINT8 *rom = memory_region(machine, "bios");

	cpu_set_irq_callback(machine->device("maincpu"), irq_callback);

	memcpy(bios_ram, rom, 0x40000);
	machine->device("maincpu")->reset();

	timer_device *sound_timer = machine->device<timer_device>("sound_timer");
	sound_timer->adjust(ATTOTIME_IN_MSEC(10));

	dmadac[0] = machine->device<dmadac_sound_device>("dac1");
	dmadac[1] = machine->device<dmadac_sound_device>("dac2");
	dmadac_enable(&dmadac[0], 2, 1);
	devtag_reset(machine, "ide");
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

static WRITE_LINE_DEVICE_HANDLER( mediagx_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine, "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface mediagx_pic8259_1_config =
{
	DEVCB_LINE(mediagx_pic8259_1_set_int_line)
};

static const struct pic8259_interface mediagx_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir2_w)
};


/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

static const struct pit8253_config mediagx_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};


static MACHINE_DRIVER_START(mediagx)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", MEDIAGX, 166000000)
	MDRV_CPU_PROGRAM_MAP(mediagx_map)
	MDRV_CPU_IO_MAP(mediagx_io)

	MDRV_MACHINE_START(mediagx)
	MDRV_MACHINE_RESET(mediagx)

	MDRV_PCI_BUS_ADD("pcibus", 0)
	MDRV_PCI_BUS_DEVICE(18, NULL, cx5510_pci_r, cx5510_pci_w)

	MDRV_PIT8254_ADD( "pit8254", mediagx_pit8254_config )

	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MDRV_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )

	MDRV_PIC8259_ADD( "pic8259_master", mediagx_pic8259_1_config )

	MDRV_PIC8259_ADD( "pic8259_slave", mediagx_pic8259_2_config )

	MDRV_IDE_CONTROLLER_ADD("ide", ide_interrupt)

	MDRV_TIMER_ADD("sound_timer", sound_timer_callback)

	MDRV_NVRAM_HANDLER( mc146818 )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 239)

	MDRV_GFXDECODE(CGA)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(mediagx)
	MDRV_VIDEO_UPDATE(mediagx)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("dac1", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MDRV_SOUND_ADD("dac2", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

static void set_gate_a20(running_machine *machine, int a20)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine *machine, int state)
{
	pic8259_ir1_w(mediagx_devices.pic8259_1, state);
}

static void ide_interrupt(running_device *device, int state)
{
	pic8259_ir6_w(mediagx_devices.pic8259_2, state);
}

static int mediagx_get_out2(running_machine *machine)
{
	return pit8253_get_output( mediagx_devices.pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, mediagx_get_out2
};

static void mediagx_set_keyb_int(running_machine *machine, int state)
{
	pic8259_ir1_w(mediagx_devices.pic8259_1, state);
}

static void init_mediagx(running_machine *machine)
{
	frame_width = frame_height = 1;

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT,mediagx_set_keyb_int);
	mc146818_init(machine, MC146818_STANDARD);

	kbdc8042_init(machine, &at8042);
}

#if SPEEDUP_HACKS

typedef struct _speedup_entry speedup_entry;
struct _speedup_entry
{
	UINT32			offset;
	UINT32			pc;
};

static const speedup_entry *speedup_table;
static UINT32 speedup_hits[12];
static int speedup_count;

INLINE UINT32 generic_speedup(const address_space *space, int idx)
{
	if (cpu_get_pc(space->cpu) == speedup_table[idx].pc)
	{
		speedup_hits[idx]++;
		cpu_spinuntil_int(space->cpu);
	}
	return main_ram[speedup_table[idx].offset/4];
}

static READ32_HANDLER( speedup0_r ) { return generic_speedup(space, 0); }
static READ32_HANDLER( speedup1_r ) { return generic_speedup(space, 1); }
static READ32_HANDLER( speedup2_r ) { return generic_speedup(space, 2); }
static READ32_HANDLER( speedup3_r ) { return generic_speedup(space, 3); }
static READ32_HANDLER( speedup4_r ) { return generic_speedup(space, 4); }
static READ32_HANDLER( speedup5_r ) { return generic_speedup(space, 5); }
static READ32_HANDLER( speedup6_r ) { return generic_speedup(space, 6); }
static READ32_HANDLER( speedup7_r ) { return generic_speedup(space, 7); }
static READ32_HANDLER( speedup8_r ) { return generic_speedup(space, 8); }
static READ32_HANDLER( speedup9_r ) { return generic_speedup(space, 9); }
static READ32_HANDLER( speedup10_r ) { return generic_speedup(space, 10); }
static READ32_HANDLER( speedup11_r ) { return generic_speedup(space, 11); }

static const read32_space_func speedup_handlers[] =
{
	speedup0_r,		speedup1_r,		speedup2_r,		speedup3_r,
	speedup4_r,		speedup5_r,		speedup6_r,		speedup7_r,
	speedup8_r,		speedup9_r,		speedup10_r,	speedup11_r
};

#ifdef MAME_DEBUG
static void report_speedups(running_machine &machine)
{
	int i;

	for (i = 0; i < speedup_count; i++)
		printf("Speedup %2d: offs=%06X pc=%06X hits=%d\n", i, speedup_table[i].offset, speedup_table[i].pc, speedup_hits[i]);
}
#endif

static void install_speedups(running_machine *machine, const speedup_entry *entries, int count)
{
	int i;

	assert(count < ARRAY_LENGTH(speedup_handlers));

	speedup_table = entries;
	speedup_count = count;

	for (i = 0; i < count; i++)
		memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), entries[i].offset, entries[i].offset + 3, 0, 0, speedup_handlers[i]);

#ifdef MAME_DEBUG
	machine->add_notifier(MACHINE_NOTIFY_EXIT, report_speedups);
#endif
}

static const speedup_entry a51site4_speedups[] =
{
	{ 0x5504c, 0x0363e },
	{ 0x5f11c, 0x0363e },
	{ 0x5cac8, 0x0363e },
	{ 0x560fc, 0x0363e },
	{ 0x55298, 0x0363e },
	{ 0x63a88, 0x049d9 },
	{ 0x5e01c, 0x049d9 },
	{ 0x5e3ec, 0x049d9 },
	{ 0x60504, 0x1c91d },
	{ 0x60440, 0x1c8c4 },
};

#endif

static DRIVER_INIT( a51site4 )
{
	init_mediagx(machine);

#if SPEEDUP_HACKS
	install_speedups(machine, a51site4_speedups, ARRAY_LENGTH(a51site4_speedups));
#endif
}

/*****************************************************************************/

ROM_START( a51site4 )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("a51s4_bios_07-11-98.u1", 0x00000, 0x40000, CRC(5ee189cc) SHA1(0b0d9321a4c59b1deea6854923e655a4d8c4fcfe)) /* Build date 07/11/98 string stored at 0x3fff5 */
	ROM_LOAD("a51s4_bios_09-15-98.u1", 0x00000, 0x40000, CRC(f8cd6a6b) SHA1(75f851ae21517b729a5596ce5e042ebfaac51778)) /* Build date 09/15/98 string stored at 0x3fff5 */

	ROM_REGION(0x08100, "gfx1", 0)
    ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a51site4", 0, SHA1(48496666d1613700ae9274f9a5361ea5bbaebea0) )
ROM_END

/*****************************************************************************/

GAME( 1998, a51site4, 0,	mediagx, mediagx, a51site4,	ROT0,   "Atari Games",  "Area 51: Site 4", GAME_NOT_WORKING )
