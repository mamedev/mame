// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Inder / Dinamic Video */

/* Inder / Dinamic Sound Board */


#include "emu.h"
#include "machine/inder_vid.h"



extern const device_type INDER_VIDEO = &device_creator<inder_vid_device>;


inder_vid_device::inder_vid_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, INDER_VIDEO, "Inder / Dinamic TMS Video", tag, owner, clock, "indervd", __FILE__),
/*  device_video_interface(mconfig, *this, false), */
		m_vram(*this, "vram"),
		m_palette(*this, "palette"),
		m_tms(*this, "tms")
{
}

static ADDRESS_MAP_START( megaphx_tms_map, AS_PROGRAM, 16, inder_vid_device )

	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("vram") // vram?

	AM_RANGE(0x04000000, 0x0400000f) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x04000010, 0x0400001f) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x04000030, 0x0400003f) AM_DEVWRITE8("ramdac",ramdac_device,index_r_w,0x00ff)
	AM_RANGE(0x04000090, 0x0400009f) AM_WRITENOP
	AM_RANGE(0x7fc00000, 0x7fffffff) AM_RAM AM_MIRROR(0x80000000)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
ADDRESS_MAP_END


TMS340X0_SCANLINE_RGB32_CB_MEMBER(inder_vid_device::scanline)
{
	UINT16 *vram = &m_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);

	const pen_t *paldata = m_palette->pens();

	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = paldata[pixels & 0xff];
		dest[x + 1] = paldata[pixels >> 8];
	}

}


TMS340X0_TO_SHIFTREG_CB_MEMBER(inder_vid_device::to_shiftreg)
{
	if (m_shiftfull == 0)
	{
		//printf("read to shift regs address %08x (%08x)\n", address, TOWORD(address) * 2);

		memcpy(shiftreg, &m_vram[TOWORD(address) & ~TOWORD(0x1fff)], TOBYTE(0x2000)); // & ~TOWORD(0x1fff) is needed for round 6
		m_shiftfull = 1;
	}
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(inder_vid_device::from_shiftreg)
{
//  printf("write from shift regs address %08x (%08x)\n", address, TOWORD(address) * 2);

	memcpy(&m_vram[TOWORD(address) & ~TOWORD(0x1fff)], shiftreg, TOBYTE(0x2000));

	m_shiftfull = 0;
}

WRITE_LINE_MEMBER(inder_vid_device::m68k_gen_int)
{
	cpu_device *maincpu = (cpu_device*)machine().device("maincpu");
	if (state) maincpu->set_input_line(4, ASSERT_LINE);
	else maincpu->set_input_line(4, CLEAR_LINE);
}


static ADDRESS_MAP_START( ramdac_map, AS_0, 8, inder_vid_device )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( inder_vid )
	MCFG_CPU_ADD("tms", TMS34010, XTAL_40MHz)
	MCFG_CPU_PROGRAM_MAP(megaphx_tms_map)
	MCFG_TMS340X0_HALT_ON_RESET(TRUE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(XTAL_40MHz/12) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(2) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(inder_vid_device, scanline)     /* scanline updater (RGB32) */
	MCFG_TMS340X0_OUTPUT_INT_CB(WRITELINE(inder_vid_device, m68k_gen_int))
	MCFG_TMS340X0_TO_SHIFTREG_CB(inder_vid_device, to_shiftreg)  /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(inder_vid_device, from_shiftreg) /* read from shiftreg function */

	MCFG_SCREEN_ADD("inder_screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_40MHz/12, 424, 0, 338-1, 262, 0, 246-1)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)


	MCFG_PALETTE_ADD("palette", 256)

	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")
	MCFG_RAMDAC_SPLIT_READ(1)

MACHINE_CONFIG_END

machine_config_constructor inder_vid_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( inder_vid );
}

void inder_vid_device::device_start()
{
}

void inder_vid_device::device_reset()
{
	m_shiftfull = 0;
}
