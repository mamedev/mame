// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "ex1280.h"


DEFINE_DEVICE_TYPE(ISA16_EX1280, isa16_ex1280_device, "ex1280", "Vectrix EX1280")

//-------------------------------------------------
//  isa16_ex1280_device - constructor
//-------------------------------------------------

isa16_ex1280_device::isa16_ex1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_EX1280, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_cpu(*this, "maincpu")
	, m_ramdac(*this, "ramdac")
	, m_screen(*this, "screen")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_ex1280_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_flags));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_ex1280_device::device_reset()
{
	m_vram.clear();
	m_vram.resize(0xa00000);
	m_flags = 0x0002;
}


//-------------------------------------------------
//  main_map - memory map for the card's TMS34010
//-------------------------------------------------

void isa16_ex1280_device::main_map(address_map &map)
{
	map(0x00000000, 0x009fffff).rw(FUNC(isa16_ex1280_device::vram_r), FUNC(isa16_ex1280_device::vram_w));
	map(0x04000000, 0x04000fff).rw(FUNC(isa16_ex1280_device::regs_r), FUNC(isa16_ex1280_device::regs_w));
	map(0xfff00000, 0xfffbffff).ram();
	map(0xfffc0000, 0xffffffff).rom().region("ex1280", 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_ex1280_device::device_add_mconfig(machine_config &config)
{
	TMS34010(config, m_cpu, 64000000);
	m_cpu->set_addrmap(AS_PROGRAM, &isa16_ex1280_device::main_map);
	m_cpu->set_halt_on_reset(false);
	m_cpu->set_pixel_clock(64000000);
	m_cpu->set_pixels_per_clock(1);
	m_cpu->set_scanline_rgb32_callback(FUNC(isa16_ex1280_device::scanline_update));
	m_cpu->set_shiftreg_in_callback(FUNC(isa16_ex1280_device::to_shiftreg));         /* write to shiftreg function */
	m_cpu->set_shiftreg_out_callback(FUNC(isa16_ex1280_device::from_shiftreg));      /* read from shiftreg function */
	m_cpu->set_screen(m_screen);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(m_cpu, FUNC(tms34010_device::tms340x0_rgb32));
	m_screen->screen_vblank().set(FUNC(isa16_ex1280_device::vblank_w));

	BT451(config, m_ramdac, 0);
}


//-------------------------------------------------
//  regs_r - register read handler
//-------------------------------------------------

uint16_t isa16_ex1280_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;
	if (offset == 0)
	{
		data = m_flags;
	}
	else if (offset == 4)
	{
		data = 0x0000;
	}
	logerror("%s: regs_r: %08x: %04x & %04x\n", machine().describe_context(), 0x04000000 | (offset << 1), data, mem_mask);
	return data;
}


//-------------------------------------------------
//  regs_w - register write handler
//-------------------------------------------------

void isa16_ex1280_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: regs_w: %08x = %04x & %04x\n", machine().describe_context(), 0x04000000 | (offset << 1), data, mem_mask);
}


//-------------------------------------------------
//  vram_r - VRAM direct read handler
//-------------------------------------------------

uint16_t isa16_ex1280_device::vram_r(offs_t offset)
{
	logerror("vram_r: %08x = %04x\n", offset, m_vram[offset]);
	return m_vram[offset];
}


//-------------------------------------------------
//  vram_w - VRAM direct write handler
//-------------------------------------------------

void isa16_ex1280_device::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("vram_r: %08x = %04x & %04x\n", offset, data, mem_mask);
	COMBINE_DATA(&m_vram[offset]);
}


//-------------------------------------------------
//  scanline_update -
//-------------------------------------------------

TMS340X0_SCANLINE_RGB32_CB_MEMBER(isa16_ex1280_device::scanline_update)
{
	uint16_t *src = &m_vram[params->rowaddr << 8];
	uint32_t *dest = &bitmap.pix32(scanline);
	const pen_t *pens = m_ramdac->pens();
	int coladdr = params->coladdr << 1;

	/* copy the non-blanked portions of this scanline */
	for (int x = params->heblnk; x < params->hsblnk; x += 4)
	{
		uint16_t pixels = src[coladdr++];
		dest[x + 0] = pens[pixels & 0x0f];
		dest[x + 1] = pens[(pixels >> 4) & 0x0f];
		dest[x + 2] = pens[(pixels >> 8) & 0x0f];
		dest[x + 3] = pens[(pixels >> 12) & 0x0f];
	}
}


//-------------------------------------------------
//  to_shiftreg - handle VRAM->TMS shift register
//-------------------------------------------------

TMS340X0_TO_SHIFTREG_CB_MEMBER(isa16_ex1280_device::to_shiftreg)
{
	printf("address to shiftreg: %08x\n", address);
	if (address < 0xa00000)
	{
		memcpy(shiftreg, &m_vram[address >> 4], 0x400);
	}
}


//---------------------------------------------------
//  from_shiftreg - handle TMS shift register->VRAM
//---------------------------------------------------

TMS340X0_FROM_SHIFTREG_CB_MEMBER(isa16_ex1280_device::from_shiftreg)
{
	printf("address from shiftreg: %08x\n", address);
	if (address < 0xa00000)
	{
		memcpy(&m_vram[address >> 4], shiftreg, 0x400);
	}
}


//-------------------------------------------------
//  vblank_w - toggle vblank bit(?)
//-------------------------------------------------

WRITE_LINE_MEMBER(isa16_ex1280_device::vblank_w)
{
	//m_flags &= ~(1 << 1);
	//m_flags |= (state << 1);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( ex1280 )
	ROM_REGION16_LE(0x8000, "ex1280", 0)
	ROM_LOAD16_BYTE("hbv10lo.bin", 0x0000, 0x4000, CRC(4de578b8) SHA1(cd7bc7859dd44f978adca7506afb0a1a73cb1121) )
	ROM_LOAD16_BYTE("hbv10hi.bin", 0x0001, 0x4000, CRC(b87caa0c) SHA1(88c19cf36cb98e59810dad2a2f1f0e6c884cd7eb) )
ROM_END

const tiny_rom_entry *isa16_ex1280_device::device_rom_region() const
{
	return ROM_NAME( ex1280 );
}
