// license:BSD-3-Clause
// copyright-holders: Carl, Angelo Salese

#include "emu.h"
#include "pc_vga_oak.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(OTI111,     oak_oti111_vga_device,  "oti111_vga",  "Oak Technologies Spitfire 64111")

oak_oti111_vga_device::oak_oti111_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, OTI111, tag, owner, clock)
	, m_xga(*this, "xga")
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(oak_oti111_vga_device::io_3bx_3dx_map), this));
}

void oak_oti111_vga_device::device_add_mconfig(machine_config &config)
{
	XGA_COPRO(config, m_xga, 0);
	m_xga->mem_read_callback().set(FUNC(oak_oti111_vga_device::mem_linear_r));
	m_xga->mem_write_callback().set(FUNC(oak_oti111_vga_device::mem_linear_w));
	m_xga->set_type(xga_copro_device::TYPE::OTI111);
}

void oak_oti111_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	// TODO: convert to address_space
	map(0x0e, 0xe).lrw8(
		NAME([this] (offs_t offset) {
			return m_oak_idx;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_oak_idx = data;
		})
	);
	map(0x0f, 0xf).lrw8(
		NAME([this] (offs_t offset) {
			return m_oak_idx <= 0x3a ? m_oak_regs[m_oak_idx] : space().unmap();
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_oak_regs[m_oak_idx] = data;
			switch(m_oak_idx)
			{
				case 0x21:
					svga.rgb8_en = BIT(data, 2);
					break;
				case 0x33:
					vga.crtc.no_wrap = BIT(data, 0);
					break;
			}
		})
	);
}

// TODO: convert to map
u8 oak_oti111_vga_device::xga_read(offs_t offset)
{
	switch(offset)
	{
		case 0x13: //fifo status
			return 0xf;
		default:
			return m_xga->xga_read(offset);
	}
	return 0;
}

void oak_oti111_vga_device::xga_write(offs_t offset, u8 data)
{
	m_xga->xga_write(offset, data);
}

void oak_oti111_vga_device::device_start()
{
	svga_device::device_start();
	std::fill(std::begin(m_oak_regs), std::end(m_oak_regs), 0);
}

void oak_oti111_vga_device::ramdac_mmio_map(address_map &map)
{
	map.unmap_value_high();
//  TODO: 0x04, 0x05 alt accesses for CRTC?
	map(0x06, 0x06).rw(FUNC(oak_oti111_vga_device::ramdac_mask_r), FUNC(oak_oti111_vga_device::ramdac_mask_w));
	map(0x07, 0x07).rw(FUNC(oak_oti111_vga_device::ramdac_state_r), FUNC(oak_oti111_vga_device::ramdac_read_index_w));
	map(0x08, 0x08).rw(FUNC(oak_oti111_vga_device::ramdac_write_index_r), FUNC(oak_oti111_vga_device::ramdac_write_index_w));
	map(0x09, 0x09).rw(FUNC(oak_oti111_vga_device::ramdac_data_r), FUNC(oak_oti111_vga_device::ramdac_data_w));
}

uint16_t oak_oti111_vga_device::offset()
{
	uint16_t off = svga_device::offset();

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
		return vga.crtc.offset << 4;  // TODO: there must a register to control this
	else
		return off;
}

