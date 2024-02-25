// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"
#include "ati_mach8.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// TODO: remove this enum
enum
{
	IBM8514_IDLE = 0,
	IBM8514_DRAWING_RECT,
	IBM8514_DRAWING_LINE,
	IBM8514_DRAWING_BITBLT,
	IBM8514_DRAWING_PATTERN,
	IBM8514_DRAWING_SSV_1,
	IBM8514_DRAWING_SSV_2,
	MACH8_DRAWING_SCAN
};

DEFINE_DEVICE_TYPE(MACH8,      mach8_device,      "mach8",      "Mach8")

mach8_device::mach8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ibm8514a_device(mconfig, type, tag, owner, clock)
{
}

mach8_device::mach8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach8_device(mconfig, MACH8, tag, owner, clock)
{
}

void mach8_device::device_start()
{
	ibm8514a_device::device_start();
	memset(&mach8, 0, sizeof(mach8));
}

uint16_t mach8_device::mach8_ec0_r()
{
	return ibm8514.ec0;
}

void mach8_device::mach8_ec0_w(uint16_t data)
{
	ibm8514.ec0 = data;
	LOG( "8514/A: Extended configuration 0 write %04x\n",data);
}

uint16_t mach8_device::mach8_ec1_r()
{
	return ibm8514.ec1;
}

void mach8_device::mach8_ec1_w(uint16_t data)
{
	ibm8514.ec1 = data;
	LOG( "8514/A: Extended configuration 1 write %04x\n",data);
}

uint16_t mach8_device::mach8_ec2_r()
{
	return ibm8514.ec2;
}

void mach8_device::mach8_ec2_w(uint16_t data)
{
	ibm8514.ec2 = data;
	LOG( "8514/A: Extended configuration 2 write %04x\n",data);
}

uint16_t mach8_device::mach8_ec3_r()
{
	return ibm8514.ec3;
}

void mach8_device::mach8_ec3_w(uint16_t data)
{
	ibm8514.ec3 = data;
	LOG( "8514/A: Extended configuration 3 write %04x\n",data);
}

uint16_t mach8_device::mach8_ext_fifo_r()
{
	return 0x00;  // for now, report all FIFO slots as free
}

void mach8_device::mach8_linedraw_index_w(uint16_t data)
{
	mach8.linedraw = data & 0x07;
	LOG( "Mach8: Line Draw Index write %04x\n",data);
}

uint16_t mach8_device::mach8_bresenham_count_r()
{
	return ibm8514.rect_width & 0x1fff;
}

void mach8_device::mach8_bresenham_count_w(uint16_t data)
{
	ibm8514.rect_width = data & 0x1fff;
	LOG( "Mach8: Bresenham count write %04x\n",data);
}

uint16_t mach8_device::mach8_linedraw_r()
{
	return 0xff;
}

void mach8_device::mach8_linedraw_w(uint16_t data)
{
	// TODO: actually draw the lines
	switch(mach8.linedraw)
	{
	case 0:  // Set current X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 1:  // Set current Y
		ibm8514.curr_y = data;
		mach8.linedraw++;
		break;
	case 2:  // Line end X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 3:  // Line end Y
		ibm8514.curr_y = data;
		mach8.linedraw = 2;
		break;
	case 4:  // Set current X
		ibm8514.curr_x = data;
		mach8.linedraw++;
		break;
	case 5:  // Set current Y
		ibm8514.curr_y = data;
		mach8.linedraw = 4;
		break;
	}
	LOG( "ATI: Linedraw register write %04x, mode %i\n",data,mach8.linedraw);
}

uint16_t mach8_device::mach8_sourcex_r()
{
	return ibm8514.dest_x & 0x07ff;
}

uint16_t mach8_device::mach8_sourcey_r()
{
	return ibm8514.dest_y & 0x07ff;
}

void mach8_device::mach8_ext_leftscissor_w(uint16_t data)
{
	// TODO
}

void mach8_device::mach8_ext_topscissor_w(uint16_t data)
{
	// TODO
}

uint16_t mach8_device::mach8_scratch0_r()
{
	return mach8.scratch0;
}

void mach8_device::mach8_scratch0_w(uint16_t data)
{
	mach8.scratch0 = data;
	LOG( "Mach8: Scratch Pad 0 write %04x\n",data);
}

uint16_t mach8_device::mach8_scratch1_r()
{
	return mach8.scratch1;
}

void mach8_device::mach8_scratch1_w(uint16_t data)
{
	mach8.scratch1 = data;
	LOG( "Mach8: Scratch Pad 1 write %04x\n",data);
}

void mach8_device::mach8_crt_pitch_w(uint16_t data)
{
	mach8.crt_pitch = data & 0x00ff;
	m_vga->set_offset(mach8.crt_pitch);
	LOG( "Mach8: CRT pitch write %04x\n",mach8.crt_pitch);
}

void mach8_device::mach8_ge_offset_l_w(uint16_t data)
{
	mach8.ge_offset = (mach8.ge_offset & 0x0f0000) | data;
	LOG( "Mach8: Graphics Engine Offset (Low) write %05x\n",mach8.ge_offset);
}

void mach8_device::mach8_ge_offset_h_w(uint16_t data)
{
	mach8.ge_offset = (mach8.ge_offset & 0x00ffff) | ((data & 0x000f) << 16);
	LOG( "Mach8: Graphics Engine Offset (High) write %05x\n",mach8.ge_offset);
}

void mach8_device::mach8_ge_pitch_w(uint16_t data)
{
	mach8.ge_pitch = data & 0x00ff;
	LOG( "Mach8: Graphics Engine pitch write %04x\n",mach8.ge_pitch);
}

void mach8_device::mach8_scan_x_w(uint16_t data)
{
	mach8.scan_x = data & 0x07ff;

	if((mach8.dp_config & 0xe000) == 0x4000)  // foreground source is the Pixel Transfer register
	{
		ibm8514.state = MACH8_DRAWING_SCAN;
		ibm8514.bus_size = (mach8.dp_config & 0x0200) >> 9;
		ibm8514.data_avail = true;
	}
	// TODO: non-wait version of Scan To X
	LOG( "Mach8: Scan To X write %04x\n",mach8.scan_x);
}

void mach8_device::mach8_pixel_xfer_w(offs_t offset, uint16_t data)
{
	ibm8514_pixel_xfer_w(offset, data);

	if(ibm8514.state == MACH8_DRAWING_SCAN)
		mach8_wait_scan();
}

void mach8_device::mach8_wait_scan()
{
	uint32_t offset = mach8.ge_offset << 2;
	uint32_t addr = (ibm8514.prev_y * (mach8.ge_pitch * 8)) + ibm8514.prev_x;

	// TODO: support reverse direction
	if(mach8.dp_config & 0x0010) // drawing enabled
	{
		if(mach8.dp_config & 0x0200)  // 16-bit
		{
			ibm8514_write_fg(offset + addr);
			ibm8514.pixel_xfer >>= 8;
			ibm8514.prev_x++;
			ibm8514_write_fg(offset + addr + 1);
			ibm8514.prev_x++;
			mach8.scan_x -= 2;
			if(mach8.scan_x <= 0)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				ibm8514.curr_x = ibm8514.prev_x;
			}
		}
		else  // 8-bit
		{
			ibm8514_write_fg(offset + addr);
			ibm8514.prev_x++;
			mach8.scan_x--;
			if(mach8.scan_x <= 0)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				ibm8514.curr_x = ibm8514.prev_x;
			}
		}
	}
}

/*
 * CEEE (Write): Data Path Configuration
 * bit  0: Read/Write data
 *      1: Polygon-fill blit mode
 *      2: Read host data - 0=colour, 1=monochrome
 *      4: Enable Draw
 *    5-6: Monochrome Data Source (0=always 1, 1=Mono pattern register, 2=Pixel Transfer register, 3=VRAM blit source)
 *    7-8: Background Colour Source (0=Foreground Colour register, 1=Background Colour register, 2=Pixel Transfer register, 3=VRAM blit source)
 *      9: Data width - 0=8-bit, 1=16-bit
 *     12: LSB First (ignored in mach8 mode when data width is not set)
 *  13-15: Foreground Colour Source (as Background Source, plus 5=Colour pattern shift register)
 */
void mach8_device::mach8_dp_config_w(uint16_t data)
{
	mach8.dp_config = data;
	LOG( "Mach8: Data Path Configuration write %04x\n",mach8.dp_config);
}

/*
12EEh W(R):  Configuration Status 1 Register                           (Mach8)
bit    0  CLK_MODE. Set to use clock chip, clear to use crystals.
       1  BUS_16. Set for 16bit bus, clear for 8bit bus
       2  MC_BUS. Set for MicroChannel bus, clear for ISA/EISA bus
       3  EEPROM_ENA. EEPROM enabled if set
       4  DRAM_ENA. Set for DRAM, clear for VRAM.
     5-6  MEM_INSTALLED. Video memory. 0: 512K, 1: 1024K
       7  ROM_ENA. Set is ROM is enabled
       8  ROM_PAGE_ENA. Set if ROM paging enabled
    9-15  ROM_LOCATION. If bit 2 and 3 are 0 the ROM will be at this location:
           0: C000h, 1: C080h, 2: C100h, .. 127: FF80h (unlikely)
 */
uint16_t mach8_device::mach8_config1_r()
{
	return 0x0082;
}

/*
16EEh (R):  Configuration Status 2 Register                            (Mach8)
bit    0  SHARE_CLOCK. If set the Mach8 shares clock with the VGA
       1  HIRES_BOOT. Boot in hi-res mode if set
       2  EPROM_16_ENA. Adapter configured for 16bit ROM if set
       3  WRITE_PER_BIT. Write masked VRAM operations supported if set
       4  FLASH_ENA. Flash page writes supported if set
 */
uint16_t mach8_device::mach8_config2_r()
{
	return 0x0002;
}

/* 7AEE (W)   Mach 8 (16-bit)
 * bits    0-2  Monitor Alias - Monitor ID
 * bit       3  Enable reporting of Monitor Alias
 * bit      12  EEPROM Data Out
 * bit      13  EEPROM Clock
 * bit      14  EEPROM Chip Select
 * bit      15  EEPROM Select (Enables read/write of external EEPROM)
 */
void mach8_device::mach8_ge_ext_config_w(uint16_t data)
{
	mach8.ge_ext_config = data;
	if(data & 0x8000)
		popmessage("EEPROM enabled via 7AEE");
}
