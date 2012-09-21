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
#include "video/ramdac.h"

#define SPEEDUP_HACKS	1

struct speedup_entry
{
	UINT32			offset;
	UINT32			pc;
};

class mediagx_state : public driver_device
{
public:
	mediagx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_main_ram(*this, "main_ram"),
		m_cga_ram(*this, "cga_ram"),
		m_bios_ram(*this, "bios_ram"),
		m_vram(*this, "vram"){ }

	required_shared_ptr<UINT32> m_main_ram;
	required_shared_ptr<UINT32> m_cga_ram;
	required_shared_ptr<UINT32> m_bios_ram;
	required_shared_ptr<UINT32> m_vram;
	UINT8 m_pal[768];


	UINT32 m_disp_ctrl_reg[256/4];
	int m_frame_width;
	int m_frame_height;

	UINT32 m_memory_ctrl_reg[256/4];
	int m_pal_index;

	UINT32 m_biu_ctrl_reg[256/4];

	UINT8 m_mediagx_config_reg_sel;
	UINT8 m_mediagx_config_regs[256];

	//UINT8 m_controls_data;
	UINT8 m_parallel_pointer;
	UINT8 m_parallel_latched;
	UINT32 m_parport;
	//int m_control_num;
	//int m_control_num2;
	//int m_control_read;

	UINT32 m_cx5510_regs[256/4];

	INT16 *m_dacl;
	INT16 *m_dacr;
	int m_dacl_ptr;
	int m_dacr_ptr;

	UINT8 m_ad1847_regs[16];
	UINT32 m_ad1847_sample_counter;
	UINT32 m_ad1847_sample_rate;

	dmadac_sound_device *m_dmadac[2];

	pit8254_device	*m_pit8254;
	pic8259_device	*m_pic8259_1;
	pic8259_device	*m_pic8259_2;
	i8237_device	*m_dma8237_1;
	i8237_device	*m_dma8237_2;

	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];

#if SPEEDUP_HACKS
	const speedup_entry *m_speedup_table;
	UINT32 m_speedup_hits[12];
	int m_speedup_count;
#endif
	DECLARE_READ32_MEMBER(disp_ctrl_r);
	DECLARE_WRITE32_MEMBER(disp_ctrl_w);
	DECLARE_READ32_MEMBER(memory_ctrl_r);
	DECLARE_WRITE32_MEMBER(memory_ctrl_w);
	DECLARE_READ32_MEMBER(biu_ctrl_r);
	DECLARE_WRITE32_MEMBER(biu_ctrl_w);
	DECLARE_WRITE32_MEMBER(bios_ram_w);
	DECLARE_READ32_MEMBER(parallel_port_r);
	DECLARE_WRITE32_MEMBER(parallel_port_w);
	DECLARE_READ32_MEMBER(ad1847_r);
	DECLARE_WRITE32_MEMBER(ad1847_w);
	DECLARE_READ8_MEMBER(at_page8_r);
	DECLARE_WRITE8_MEMBER(at_page8_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_READ8_MEMBER(at_dma8237_2_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
	DECLARE_READ32_MEMBER(ide_r);
	DECLARE_WRITE32_MEMBER(ide_w);
	DECLARE_READ32_MEMBER(fdc_r);
	DECLARE_WRITE32_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(io20_r);
	DECLARE_WRITE8_MEMBER(io20_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(mediagx_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_DRIVER_INIT(a51site4);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ32_MEMBER(speedup0_r); 	
	DECLARE_READ32_MEMBER(speedup1_r); 	
	DECLARE_READ32_MEMBER(speedup2_r); 	
	DECLARE_READ32_MEMBER(speedup3_r); 	
	DECLARE_READ32_MEMBER(speedup4_r); 	
	DECLARE_READ32_MEMBER(speedup5_r); 	
	DECLARE_READ32_MEMBER(speedup6_r); 	
	DECLARE_READ32_MEMBER(speedup7_r); 	
	DECLARE_READ32_MEMBER(speedup8_r); 	
	DECLARE_READ32_MEMBER(speedup9_r); 	
	DECLARE_READ32_MEMBER(speedup10_r);	
	DECLARE_READ32_MEMBER(speedup11_r);	
};

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


static void ide_interrupt(device_t *device, int state);




static const rgb_t cga_palette[16] =
{
	MAKE_RGB( 0x00, 0x00, 0x00 ), MAKE_RGB( 0x00, 0x00, 0xaa ), MAKE_RGB( 0x00, 0xaa, 0x00 ), MAKE_RGB( 0x00, 0xaa, 0xaa ),
	MAKE_RGB( 0xaa, 0x00, 0x00 ), MAKE_RGB( 0xaa, 0x00, 0xaa ), MAKE_RGB( 0xaa, 0x55, 0x00 ), MAKE_RGB( 0xaa, 0xaa, 0xaa ),
	MAKE_RGB( 0x55, 0x55, 0x55 ), MAKE_RGB( 0x55, 0x55, 0xff ), MAKE_RGB( 0x55, 0xff, 0x55 ), MAKE_RGB( 0x55, 0xff, 0xff ),
	MAKE_RGB( 0xff, 0x55, 0x55 ), MAKE_RGB( 0xff, 0x55, 0xff ), MAKE_RGB( 0xff, 0xff, 0x55 ), MAKE_RGB( 0xff, 0xff, 0xff ),
};

void mediagx_state::video_start()
{
	int i;
	for (i=0; i < 16; i++)
	{
		palette_set_color(machine(), i, cga_palette[i]);
	}
}

static void draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y)
{
	int i,j;
	const UINT8 *dp;
	int index = 0;
	const pen_t *pens = gfx->machine().pens;

	dp = gfx->get_data(ch);

	for (j=y; j < y+8; j++)
	{
		UINT32 *p = &bitmap.pix32(j);
		for (i=x; i < x+8; i++)
		{
			UINT8 pen = dp[index++];
			if (pen)
				p[i] = pens[gfx->colorbase() + (att & 0xf)];
			else
			{
				if (((att >> 4) & 7) > 0)
					p[i] = pens[gfx->colorbase() + ((att >> 4) & 0x7)];
			}
		}
	}
}

static void draw_framebuffer(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();
	int i, j;
	int width, height;
	int line_delta = (state->m_disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) * 4;

	width = (state->m_disp_ctrl_reg[DC_H_TIMING_1] & 0x7ff) + 1;
	if (state->m_disp_ctrl_reg[DC_TIMING_CFG] & 0x8000)		// pixel double
	{
		width >>= 1;
	}
	width += 4;

	height = (state->m_disp_ctrl_reg[DC_V_TIMING_1] & 0x7ff) + 1;

	if ( (width != state->m_frame_width || height != state->m_frame_height) &&
		 (width > 1 && height > 1 && width <= 640 && height <= 480) )
	{
		rectangle visarea;

		state->m_frame_width = width;
		state->m_frame_height = height;

		visarea.set(0, width - 1, 0, height - 1);
		machine.primary_screen->configure(width, height * 262 / 240, visarea, machine.primary_screen->frame_period().attoseconds);
	}

	if (state->m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)		// 8-bit mode
	{
		UINT8 *framebuf = (UINT8*)&state->m_vram[state->m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];
		UINT8 *pal = state->m_pal;

		for (j=0; j < state->m_frame_height; j++)
		{
			UINT32 *p = &bitmap.pix32(j);
			UINT8 *si = &framebuf[j * line_delta];
			for (i=0; i < state->m_frame_width; i++)
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
		UINT16 *framebuf = (UINT16*)&state->m_vram[state->m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		// RGB 5-6-5 mode
		if ((state->m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x2) == 0)
		{
			for (j=0; j < state->m_frame_height; j++)
			{
				UINT32 *p = &bitmap.pix32(j);
				UINT16 *si = &framebuf[j * (line_delta/2)];
				for (i=0; i < state->m_frame_width; i++)
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
			for (j=0; j < state->m_frame_height; j++)
			{
				UINT32 *p = &bitmap.pix32(j);
				UINT16 *si = &framebuf[j * (line_delta/2)];
				for (i=0; i < state->m_frame_width; i++)
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

static void draw_cga(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();
	int i, j;
	gfx_element *gfx = machine.gfx[0];
	UINT32 *cga = state->m_cga_ram;
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

UINT32 mediagx_state::screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_framebuffer(machine(), bitmap, cliprect);

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)	// don't show MDA text screen on 16-bit mode. this is basically a hack
	{
		draw_cga(machine(), bitmap, cliprect);
	}
	return 0;
}

READ32_MEMBER(mediagx_state::disp_ctrl_r)
{
	UINT32 r = m_disp_ctrl_reg[offset];

	switch (offset)
	{
		case DC_TIMING_CFG:
			r |= 0x40000000;

			if (machine().primary_screen->vpos() >= m_frame_height)
				r &= ~0x40000000;

#if SPEEDUP_HACKS
			// wait for vblank speedup
			space.device().execute().spin_until_interrupt();
#endif
			break;
	}

	return r;
}

WRITE32_MEMBER(mediagx_state::disp_ctrl_w)
{

//  printf("disp_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	COMBINE_DATA(m_disp_ctrl_reg + offset);
}


READ8_MEMBER(mediagx_state::at_dma8237_2_r)
{
	device_t *device = machine().device("dma8237_2");
	return i8237_r(device, space, offset / 2);
}

WRITE8_MEMBER(mediagx_state::at_dma8237_2_w)
{
	device_t *device = machine().device("dma8237_2");
	i8237_w(device, space, offset / 2, data);
}


READ32_MEMBER(mediagx_state::ide_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, space, 0x1f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(mediagx_state::ide_w)
{
	device_t *device = machine().device("ide");
	ide_controller32_w(device, space, 0x1f0/4 + offset, data, mem_mask);
}

READ32_MEMBER(mediagx_state::fdc_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, space, 0x3f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(mediagx_state::fdc_w)
{
	device_t *device = machine().device("ide");
	ide_controller32_w(device, space, 0x3f0/4 + offset, data, mem_mask);
}



READ32_MEMBER(mediagx_state::memory_ctrl_r)
{

	return m_memory_ctrl_reg[offset];
}

WRITE32_MEMBER(mediagx_state::memory_ctrl_w)
{

//  printf("memory_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	if (offset == 0x20/4)
	{
		ramdac_device *ramdac = machine().device<ramdac_device>("ramdac");

		if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00e00000) == 0x00400000)
		{
			// guess: crtc params?
			// ...
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00000000)
		{
			m_pal_index = data;
			ramdac->index_w( space, 0, data );
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00100000)
		{
			m_pal[m_pal_index] = data & 0xff;
			m_pal_index++;
			if (m_pal_index >= 768)
			{
				m_pal_index = 0;
			}
			ramdac->pal_w( space, 0, data );
		}
	}
	else
	{
		COMBINE_DATA(m_memory_ctrl_reg + offset);
	}
}



READ32_MEMBER(mediagx_state::biu_ctrl_r)
{

	if (offset == 0)
	{
		return 0xffffff;
	}
	return m_biu_ctrl_reg[offset];
}

WRITE32_MEMBER(mediagx_state::biu_ctrl_w)
{

	//mame_printf_debug("biu_ctrl_w %08X, %08X, %08X\n", data, offset, mem_mask);
	COMBINE_DATA(m_biu_ctrl_reg + offset);

	if (offset == 3)		// BC_XMAP_3 register
	{
		//mame_printf_debug("BC_XMAP_3: %08X, %08X, %08X\n", data, offset, mem_mask);
	}
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(mediagx_state::bios_ram_w)
{

}
#endif

static UINT8 mediagx_config_reg_r(device_t *device)
{
	mediagx_state *state = device->machine().driver_data<mediagx_state>();

	//mame_printf_debug("mediagx_config_reg_r %02X\n", mediagx_config_reg_sel);
	return state->m_mediagx_config_regs[state->m_mediagx_config_reg_sel];
}

static void mediagx_config_reg_w(device_t *device, UINT8 data)
{
	mediagx_state *state = device->machine().driver_data<mediagx_state>();

	//mame_printf_debug("mediagx_config_reg_w %02X, %02X\n", mediagx_config_reg_sel, data);
	state->m_mediagx_config_regs[state->m_mediagx_config_reg_sel] = data;
}

READ8_MEMBER(mediagx_state::io20_r)
{
	device_t *device = machine().device("pic8259_master");
	UINT8 r = 0;

	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x02)
	{
	}
	else if (offset == 0x03)
	{
		r = mediagx_config_reg_r(device);
	}
	else
	{
		r = pic8259_r(device, space, offset);
	}
	return r;
}

WRITE8_MEMBER(mediagx_state::io20_w)
{
	device_t *device = machine().device("pic8259_master");

	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x02)
	{
		m_mediagx_config_reg_sel = data;
	}
	else if (offset == 0x03)
	{
		mediagx_config_reg_w(device, data);
	}
	else
	{
		pic8259_w(device, space, offset, data);
	}
}

READ32_MEMBER(mediagx_state::parallel_port_r)
{
	UINT32 r = 0;
	//static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7", "IN8" }; // but parallel_pointer takes values 0 -> 23

	if (ACCESSING_BITS_8_15)
	{
		UINT8 nibble = m_parallel_latched;//(ioport(m_portnames[m_parallel_pointer / 3])->read_safe(0) >> (4 * (m_parallel_pointer % 3))) & 15;
		r |= ((~nibble & 0x08) << 12) | ((nibble & 0x07) << 11);
		logerror("%08X:parallel_port_r()\n", space.device().safe_pc());
#if 0
		if (m_controls_data == 0x18)
		{
			r |= ioport("IN0")->read() << 8;
		}
		else if (m_controls_data == 0x60)
		{
			r |= ioport("IN1")->read() << 8;
		}
		else if (m_controls_data == 0xff || m_controls_data == 0x50)
		{
			r |= ioport("IN2")->read() << 8;
		}

		//r |= m_control_read << 8;
#endif
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= m_parport & 0xff0000;
	}

	return r;
}

WRITE32_MEMBER(mediagx_state::parallel_port_w)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7", "IN8" };	// but parallel_pointer takes values 0 -> 23

	COMBINE_DATA( &m_parport );

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

		logerror("%08X:", space.device().safe_pc());

		m_parallel_latched = (ioport(portnames[m_parallel_pointer / 3])->read_safe(0) >> (4 * (m_parallel_pointer % 3))) & 15;
		//parallel_pointer++;
		//logerror("[%02X] Advance pointer to %d\n", data, parallel_pointer);
		switch (data & 0xfc)
		{
			case 0x18:
				m_parallel_pointer = data & 3;
				logerror("[%02X] Reset pointer to %d\n", data, m_parallel_pointer);
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
					m_parallel_pointer++;
					logerror("[%02X] Advance pointer to %d\n", data, m_parallel_pointer);
				}
				else
					logerror("[%02X] Unknown write\n", data);
				break;
		}
	}
}

static UINT32 cx5510_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	mediagx_state *state = busdevice->machine().driver_data<mediagx_state>();

	//mame_printf_debug("CX5510: PCI read %d, %02X, %08X\n", function, reg, mem_mask);
	switch (reg)
	{
		case 0:		return 0x00001078;
	}

	return state->m_cx5510_regs[reg/4];
}

static void cx5510_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	mediagx_state *state = busdevice->machine().driver_data<mediagx_state>();

	//mame_printf_debug("CX5510: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(state->m_cx5510_regs + (reg/4));
}

/* Analog Devices AD1847 Stereo DAC */

static TIMER_DEVICE_CALLBACK( sound_timer_callback )
{
	mediagx_state *state = timer.machine().driver_data<mediagx_state>();

	state->m_ad1847_sample_counter = 0;
	timer.adjust(attotime::from_msec(10));

	dmadac_transfer(&state->m_dmadac[0], 1, 0, 1, state->m_dacl_ptr, state->m_dacl);
	dmadac_transfer(&state->m_dmadac[1], 1, 0, 1, state->m_dacr_ptr, state->m_dacr);

	state->m_dacl_ptr = 0;
	state->m_dacr_ptr = 0;
}

static void ad1847_reg_write(running_machine &machine, int reg, UINT8 data)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();
	static const int divide_factor[] = { 3072, 1536, 896, 768, 448, 384, 512, 2560 };

	switch (reg)
	{
		case 8:		// Data format register
		{
			if (data & 0x1)
			{
				state->m_ad1847_sample_rate = 16934400 / divide_factor[(data >> 1) & 0x7];
			}
			else
			{
				state->m_ad1847_sample_rate = 24576000 / divide_factor[(data >> 1) & 0x7];
			}

			dmadac_set_frequency(&state->m_dmadac[0], 2, state->m_ad1847_sample_rate);

			if (data & 0x20)
			{
				fatalerror("AD1847: Companded data not supported\n");
			}
			if ((data & 0x40) == 0)
			{
				fatalerror("AD1847: 8-bit data not supported\n");
			}
			break;
		}

		default:
		{
			state->m_ad1847_regs[reg] = data;
			break;
		}
	}
}

READ32_MEMBER(mediagx_state::ad1847_r)
{

	switch (offset)
	{
		case 0x14/4:
			return ((m_ad1847_sample_rate) / 100) - m_ad1847_sample_counter;
	}
	return 0;
}

WRITE32_MEMBER(mediagx_state::ad1847_w)
{

	if (offset == 0)
	{
		if (ACCESSING_BITS_16_31)
		{
			UINT16 ldata = (data >> 16) & 0xffff;
			m_dacl[m_dacl_ptr++] = ldata;
		}
		if (ACCESSING_BITS_0_15)
		{
			UINT16 rdata = data & 0xffff;
			m_dacr[m_dacr_ptr++] = rdata;
		}

		m_ad1847_sample_counter++;
	}
	else if (offset == 3)
	{
		int reg = (data >> 8) & 0xf;
		ad1847_reg_write(machine(), reg, data & 0xff);
	}
}


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/


READ8_MEMBER(mediagx_state::at_page8_r)
{
	UINT8 data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


WRITE8_MEMBER(mediagx_state::at_page8_w)
{

	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


WRITE_LINE_MEMBER(mediagx_state::pc_dma_hrq_changed)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( m_dma8237_1, state );
}


READ8_MEMBER(mediagx_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(mediagx_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static void set_dma_channel(device_t *device, int channel, int _state)
{
	mediagx_state *state = device->machine().driver_data<mediagx_state>();

	if (!_state) state->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(mediagx_state::pc_dack0_w){ set_dma_channel(m_dma8237_1, 0, state); }
WRITE_LINE_MEMBER(mediagx_state::pc_dack1_w){ set_dma_channel(m_dma8237_1, 1, state); }
WRITE_LINE_MEMBER(mediagx_state::pc_dack2_w){ set_dma_channel(m_dma8237_1, 2, state); }
WRITE_LINE_MEMBER(mediagx_state::pc_dack3_w){ set_dma_channel(m_dma8237_1, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(mediagx_state,pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mediagx_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(mediagx_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(mediagx_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(mediagx_state,pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(mediagx_state,pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(mediagx_state,pc_dack3_w) }
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

static ADDRESS_MAP_START( mediagx_map, AS_PROGRAM, 32, mediagx_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x000a0000, 0x000affff) AM_RAM
	AM_RANGE(0x000b0000, 0x000b7fff) AM_RAM AM_SHARE("cga_ram")
	AM_RANGE(0x000c0000, 0x000fffff) AM_RAM AM_SHARE("bios_ram")
	AM_RANGE(0x00100000, 0x00ffffff) AM_RAM
	AM_RANGE(0x40008000, 0x400080ff) AM_READWRITE(biu_ctrl_r, biu_ctrl_w)
	AM_RANGE(0x40008300, 0x400083ff) AM_READWRITE(disp_ctrl_r, disp_ctrl_w)
	AM_RANGE(0x40008400, 0x400084ff) AM_READWRITE(memory_ctrl_r, memory_ctrl_w)
	AM_RANGE(0x40800000, 0x40bfffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(mediagx_io, AS_IO, 32, mediagx_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_READWRITE8(io20_r, io20_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r,				at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP		// I/O delay port
	AM_RANGE(0x01f0, 0x01f7) AM_READWRITE(ide_r, ide_w)
	AM_RANGE(0x0378, 0x037b) AM_READWRITE(parallel_port_r, parallel_port_w)
	AM_RANGE(0x03f0, 0x03ff) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x0400, 0x04ff) AM_READWRITE(ad1847_r, ad1847_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
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
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout, 0, 256 )   /* Font 0 */
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout, 0, 256 )   /* Font 1 */
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout, 0, 256 )   /* Font 2 */
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout, 0, 256 )   /* Font 3*/
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
	mediagx_state *state = device->machine().driver_data<mediagx_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

void mediagx_state::machine_start()
{

	m_pit8254 = machine().device<pit8254_device>( "pit8254" );
	m_pic8259_1 = machine().device<pic8259_device>( "pic8259_master" );
	m_pic8259_2 = machine().device<pic8259_device>( "pic8259_slave" );
	m_dma8237_1 = machine().device<i8237_device>( "dma8237_1" );
	m_dma8237_2 = machine().device<i8237_device>( "dma8237_2" );

	m_dacl = auto_alloc_array(machine(), INT16, 65536);
	m_dacr = auto_alloc_array(machine(), INT16, 65536);
}

void mediagx_state::machine_reset()
{
	UINT8 *rom = memregion("bios")->base();

	machine().device("maincpu")->execute().set_irq_acknowledge_callback(irq_callback);

	memcpy(m_bios_ram, rom, 0x40000);
	machine().device("maincpu")->reset();

	timer_device *sound_timer = machine().device<timer_device>("sound_timer");
	sound_timer->adjust(attotime::from_msec(10));

	m_dmadac[0] = machine().device<dmadac_sound_device>("dac1");
	m_dmadac[1] = machine().device<dmadac_sound_device>("dac2");
	dmadac_enable(&m_dmadac[0], 2, 1);
	machine().device("ide")->reset();
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(mediagx_state::mediagx_pic8259_1_set_int_line)
{
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(mediagx_state::get_slave_ack)
{
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface mediagx_pic8259_1_config =
{
	DEVCB_DRIVER_LINE_MEMBER(mediagx_state,mediagx_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(mediagx_state,get_slave_ack)
};

static const struct pic8259_interface mediagx_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
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

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, mediagx_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	0
};

static const ide_config ide_intf =
{
	ide_interrupt,
	NULL,
	0
};

static MACHINE_CONFIG_START( mediagx, mediagx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MEDIAGX, 166000000)
	MCFG_CPU_PROGRAM_MAP(mediagx_map)
	MCFG_CPU_IO_MAP(mediagx_io)


	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(18, NULL, cx5510_pci_r, cx5510_pci_w)

	MCFG_PIT8254_ADD( "pit8254", mediagx_pit8254_config )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_master", mediagx_pic8259_1_config )

	MCFG_PIC8259_ADD( "pic8259_slave", mediagx_pic8259_2_config )

	MCFG_IDE_CONTROLLER_ADD("ide", ide_intf, ide_devices, "hdd", NULL, true)

	MCFG_TIMER_ADD("sound_timer", sound_timer_callback)

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(mediagx_state, screen_update_mediagx)

	MCFG_GFXDECODE(CGA)
	MCFG_PALETTE_LENGTH(256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static void set_gate_a20(running_machine &machine, int a20)
{
	machine.device("maincpu")->execute().set_input_line(INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int _state)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();

	pic8259_ir1_w(state->m_pic8259_1, _state);
}

static void ide_interrupt(device_t *device, int _state)
{
	mediagx_state *state = device->machine().driver_data<mediagx_state>();

	pic8259_ir6_w(state->m_pic8259_2, _state);
}

static int mediagx_get_out2(running_machine &machine)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();

	return pit8253_get_output( state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, mediagx_get_out2
};

static void mediagx_set_keyb_int(running_machine &machine, int _state)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();

	pic8259_ir1_w(state->m_pic8259_1, _state);
}

static void init_mediagx(running_machine &machine)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();

	state->m_frame_width = state->m_frame_height = 1;

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT,mediagx_set_keyb_int);

	kbdc8042_init(machine, &at8042);
}

#if SPEEDUP_HACKS

INLINE UINT32 generic_speedup(address_space &space, int idx)
{
	mediagx_state *state = space.machine().driver_data<mediagx_state>();

	if (space.device().safe_pc() == state->m_speedup_table[idx].pc)
	{
		state->m_speedup_hits[idx]++;
		space.device().execute().spin_until_interrupt();
	}
	return state->m_main_ram[state->m_speedup_table[idx].offset/4];
}

READ32_MEMBER(mediagx_state::speedup0_r) { return generic_speedup(space, 0); }
READ32_MEMBER(mediagx_state::speedup1_r) { return generic_speedup(space, 1); }
READ32_MEMBER(mediagx_state::speedup2_r) { return generic_speedup(space, 2); }
READ32_MEMBER(mediagx_state::speedup3_r) { return generic_speedup(space, 3); }
READ32_MEMBER(mediagx_state::speedup4_r) { return generic_speedup(space, 4); }
READ32_MEMBER(mediagx_state::speedup5_r) { return generic_speedup(space, 5); }
READ32_MEMBER(mediagx_state::speedup6_r) { return generic_speedup(space, 6); }
READ32_MEMBER(mediagx_state::speedup7_r) { return generic_speedup(space, 7); }
READ32_MEMBER(mediagx_state::speedup8_r) { return generic_speedup(space, 8); }
READ32_MEMBER(mediagx_state::speedup9_r) { return generic_speedup(space, 9); }
READ32_MEMBER(mediagx_state::speedup10_r) { return generic_speedup(space, 10); }
READ32_MEMBER(mediagx_state::speedup11_r) { return generic_speedup(space, 11); }

static const struct { read32_delegate func; } speedup_handlers[] =
{
	{ read32_delegate(FUNC(mediagx_state::speedup0_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup1_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup2_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup3_r),(mediagx_state*)0) },
	{ read32_delegate(FUNC(mediagx_state::speedup4_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup5_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup6_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup7_r),(mediagx_state*)0) },
	{ read32_delegate(FUNC(mediagx_state::speedup8_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup9_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup10_r),(mediagx_state*)0) },	{ read32_delegate(FUNC(mediagx_state::speedup11_r),(mediagx_state*)0) }
};

#ifdef MAME_DEBUG
static void report_speedups(running_machine &machine)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();
	int i;

	for (i = 0; i < state->m_speedup_count; i++)
		printf("Speedup %2d: offs=%06X pc=%06X hits=%d\n", i, state->m_speedup_table[i].offset, state->m_speedup_table[i].pc, state->m_speedup_hits[i]);
}
#endif

static void install_speedups(running_machine &machine, const speedup_entry *entries, int count)
{
	mediagx_state *state = machine.driver_data<mediagx_state>();
	int i;

	assert(count < ARRAY_LENGTH(speedup_handlers));

	state->m_speedup_table = entries;
	state->m_speedup_count = count;

	for (i = 0; i < count; i++) {
		read32_delegate func = speedup_handlers[i].func;
		func.late_bind(*state);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(entries[i].offset, entries[i].offset + 3, func);
	}

#ifdef MAME_DEBUG
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(report_speedups), &machine));
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

DRIVER_INIT_MEMBER(mediagx_state,a51site4)
{
	init_mediagx(machine());

#if SPEEDUP_HACKS
	install_speedups(machine(), a51site4_speedups, ARRAY_LENGTH(a51site4_speedups));
#endif
}

/*****************************************************************************/

ROM_START( a51site4 )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "new", "v1.0h" )
	ROMX_LOAD("a51s4_bios_09-15-98.u1", 0x00000, 0x40000, CRC(f8cd6a6b) SHA1(75f851ae21517b729a5596ce5e042ebfaac51778), ROM_BIOS(1)) /* Build date 09/15/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(1, "old", "v1.0f" )
	ROMX_LOAD("a51s4_bios_07-11-98.u1", 0x00000, 0x40000, CRC(5ee189cc) SHA1(0b0d9321a4c59b1deea6854923e655a4d8c4fcfe), ROM_BIOS(2)) /* Build date 07/11/98 string stored at 0x3fff5 */

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "a51site4-2_01", 0, SHA1(48496666d1613700ae9274f9a5361ea5bbaebea0) ) /* Happ replacement drive "A-22509", sticker on drive shows REV 2.01 and Test Mode screen shows the date September 7, 1998 */
ROM_END

ROM_START( a51site4a ) /* When dumped connected straight to IDE the cylinders were 4968 and heads were 16 */
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "new", "v1.0h" )
	ROMX_LOAD("a51s4_bios_09-15-98.u1", 0x00000, 0x40000, CRC(f8cd6a6b) SHA1(75f851ae21517b729a5596ce5e042ebfaac51778), ROM_BIOS(1)) /* Build date 09/15/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(1, "old", "v1.0f" )
	ROMX_LOAD("a51s4_bios_07-11-98.u1", 0x00000, 0x40000, CRC(5ee189cc) SHA1(0b0d9321a4c59b1deea6854923e655a4d8c4fcfe), ROM_BIOS(2)) /* Build date 07/11/98 string stored at 0x3fff5 */

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "a51site4-2_0", 0, SHA1(4de421e4d1708ecbdfb50730000814a1ea36a044) ) /* Stock drive, sticker on drive shows REV 2.0 and Test Mode screen shows the date September 11, 1998 */
ROM_END


/*****************************************************************************/

GAME( 1998, a51site4, 0       , mediagx, mediagx, mediagx_state, a51site4,	ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.01, September 7, 1998)", GAME_NOT_WORKING )
GAME( 1998, a51site4a,a51site4,	mediagx, mediagx, mediagx_state, a51site4,	ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.0, September 11, 1998)", GAME_NOT_WORKING )
