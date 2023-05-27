// license:BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Marcelina Koscielnicka
/**************************************************************************************************

nVidia NV3:G80 VGA core

TODO:
- Base CRTC;
- Verify that NV10 really uses this chip (seems to access more regs?)
- I2C;
- Interrupts;
- RMA access;
- PLL portion(s);
- Video overlay (PVIDEO, NV10+);
- VGA stack (NV40+);
- TV encoder (NV17:NV20, NV30:G80);

Notes:
- NV1 looks very different, not worth to subclass here;

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_nvidia.h"

#define LOG_WARN      (1U << 1)
#define LOG_RMA       (1U << 2)
#define LOG_CRTC      (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_RMA | LOG_CRTC)
#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGRMA(...)             LOGMASKED(LOG_RMA, __VA_ARGS__)
#define LOGCRTC(...)            LOGMASKED(LOG_CRTC, __VA_ARGS__)

DEFINE_DEVICE_TYPE(NVIDIA_NV3_VGA,  nvidia_nv3_vga_device,  "nvidia_nv3_vga",  "nVidia NV3 VGA i/f")

nvidia_nv3_vga_device::nvidia_nv3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, NVIDIA_NV3_VGA, tag, owner, clock)
{
}

void nvidia_nv3_vga_device::device_add_mconfig(machine_config &config)
{
	// TODO: I2C, likely for DDC (AIDA16 extensively test that in non-safe mode)
}

static INPUT_PORTS_START(nv3_vga_sense)
	PORT_START("VGA_SENSE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // causes a red screen at POST if low
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

ioport_constructor nvidia_nv3_vga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nv3_vga_sense);
}

void nvidia_nv3_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	vga.crtc.maximum_scan_line = 1;

	vga.svga_intf.seq_regcount = 0x05;
	vga.svga_intf.crtc_regcount = 0x40;
	// TODO: shared with main GPU, fake it for now
	// Start address ends at 20 bits so 0x1fffff mask / 2,097,151 bytes / 2MB window is given
	vga.svga_intf.vram_size = 2*1024*1024;
	
	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, pal1bit(i & 1), pal1bit((i & 2) >> 1), pal1bit((i & 4) >> 2));

	vga.miscellaneous_output = 0;
}

// TODO: should really calculate the access bit internally 
uint8_t nvidia_nv3_vga_device::port_03b0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				LOGRMA("RMA_ACCESS R %02x\n", offset & 3);
				break;

			case 5:
				res = crtc_reg_read(vga.crtc.index);
				break;

			default:
				res = vga_device::port_03b0_r(offset);
				break;
		}
	}

	return res;
}

void nvidia_nv3_vga_device::port_03b0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				LOGRMA("RMA_ACCESS W %02x -> %02x\n", offset & 3, data);
				break;

			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				crtc_reg_write(vga.crtc.index, data);
				break;

			default:
				vga_device::port_03b0_w(offset, data);
				break;
		}
	}
	//define_video_mode();
}

uint8_t nvidia_nv3_vga_device::port_03d0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				LOGRMA("RMA_ACCESS R %02x\n", offset & 3);
				break;

			case 5:
				res = crtc_reg_read(vga.crtc.index);
				break;

			default:
				res = vga_device::port_03d0_r(offset);
				break;
		}
	}

	return res;
}

void nvidia_nv3_vga_device::port_03d0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				LOGRMA("RMA_ACCESS W %02x -> %02x\n", offset & 3, data);
				break;

			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				crtc_reg_write(vga.crtc.index, data);
				break;

			default:
				vga_device::port_03d0_w(offset, data);
				break;
		}
	}
	//define_video_mode();
}

/*
 * xxx- ---- Row offset bits 3-5
 * ---x xxxx Start address bits 16-20
 */
// map(0x19, 0x19) REPAINT_0
/*
 * ---- -x-- Shifts row offset by 1 bit (verify)
 */
// map(0x1a, 0x1a) REPAINT_1
/*
 * --xx xxxx write bank, in 32k units
 */
// map(0x1d, 0x1d) WRITE_BANK
/*
 * --xx xxxx read bank
 */
// map(0x1e, 0x1e) READ_BANK
/*
 * ---x ---- Horizontal Total bit 8
 * ---- x--- Vertical Sync Start bit 10
 * ---- -x-- Vertical Blank Start bit 10
 * ---- --x- Vertical Display End bit 10
 * ---- ---x Vertical Total bit 10
 */
// map(0x25, 0x25) EXTENDED_VERT
/*
 * ---- --xx Pixel Format
 * ---- --00 VGA
 * ---- --01 8bpp
 * ---- --10 16bpp
 * ---- --11 32bpp
 */
// map(0x28, 0x28) PIXEL_FMT
/*
 * ---- ---x Horizontal Display End bit 8
 */
// map(0x2d, 0x2d) EXTENDED_HORZ
/*
 * ---- xxx- RMA index
 * ---- ---x enable RMA
 */
// map(0x38, 0x38) RMA_MODE
/*
 * ---- x--- SDA
 * ---- -x-- SCL
 */
// map(0x3e, 0x3e) I2C_READ
/*
 * ---x ---- SCL
 * ---- x--- SDA
 */
// map(0x3f, 0x3f) I2C_WRITE

uint8_t nvidia_nv3_vga_device::crtc_reg_read(uint8_t index)
{
	uint8_t res = 0;

	//index &= 0x3f;

	if(index <= 0x18)
		return svga_device::crtc_reg_read(index);

	switch(index)
	{
		case 0x38:
			res = vga.crtc.data[index];
			LOGRMA("RMA_MODE read %02x\n", res);
			break;
		default:
			LOGCRTC("EXT CRTC: %02x Read\n", index);
			//machine().debug_break();
			res = vga.crtc.data[index];

			break;
	}

	return res;
}

void nvidia_nv3_vga_device::crtc_reg_write(uint8_t index, uint8_t data)
{
	//index &= 0x3f;
	if(index <= 0x18)
	{
		svga_device::crtc_reg_write(index,data);
		return;
	}

	switch(index)
	{
		case 0x38:
			vga.crtc.data[index] = data;
			LOGRMA("RMA_MODE write %s, index %d\n", data & 1 ? "enable" : "disable", (data & 0xe) >> 1);
			break;
		default:
			vga.crtc.data[index] = data;

			LOGCRTC("EXT CRTC [%02x] <- %02x\n",index,data);
			break;
	}
}
