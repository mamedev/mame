// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc_vga_matrox.h"

DEFINE_DEVICE_TYPE(MATROX_VGA,  matrox_vga_device,  "matrox_vga",  "Matrox MGA2064W VGA")

matrox_vga_device::matrox_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, MATROX_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(matrox_vga_device::io_3bx_3dx_map), this));
	// 3 bits of address space?
	m_crtcext_space_config = address_space_config("crtcext_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(matrox_vga_device::crtcext_map), this));
	// TODO: docs mentions using 0x22 / 0x24 / 0x26 for regular CRTC (coming from plain VGA?)
}

device_memory_interface::space_config_vector matrox_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG, &m_crtcext_space_config));
	return r;
}

void matrox_vga_device::device_reset()
{
	svga_device::device_reset();

	m_crtcext_index = 0;
}

void matrox_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x0e, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtcext_index;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_crtcext_index = data;
		})
	);
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return space(EXT_REG).read_byte(m_crtcext_index);
		}),
		NAME([this] (offs_t offset, u8 data) {
			space(EXT_REG).write_byte(m_crtcext_index, data);
		})
	);
}

// "CRTCEXT*"
void matrox_vga_device::crtcext_map(address_map &map)
{
//  map(0x00, 0x00) Address Generator Extensions
//  map(0x01, 0x01) Horizontal Counter Extensions
//  map(0x02, 0x02) Vertical Counter Extensions
//  map(0x03, 0x03) Miscellaneous
//  CRTCEXT4 Memory Page register
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) { return svga.bank_w & 0x7f; }),
		NAME([this] (offs_t offset, u8 data) { svga.bank_w = data & 0x7f; })
	);
//  map(0x05, 0x05) Horizontal Video Half Count
//  map(0x06, 0x07) <Reserved>
//  \- $07 is actually checked by VESA test (PC=0xc62bb in rev3),
//     seems to disable SVGA drawing -> diagnostic check?
}

// RAMDAC
// - paired with a Texas Instruments TVP3026 here -> a superset of INMOS IMSG176/IMSG178
// - integrated with a superset in G400 at least
void matrox_vga_device::ramdac_ext_map(address_map &map)
{
//	map(0x00, 0x00).rw(FUNC(matrox_vga_device::ramdac_write_index_r), FUNC(matrox_vga_device::ramdac_write_index_w));
//	map(0x01, 0x01).rw(FUNC(matrox_vga_device::ramdac_data_r), FUNC(matrox_vga_device::ramdac_data_w));
//	map(0x02, 0x02).rw(FUNC(matrox_vga_device::ramdac_mask_r), FUNC(matrox_vga_device::ramdac_mask_w));
//	map(0x03, 0x03).rw(FUNC(matrox_vga_device::ramdac_read_index_r), FUNC(matrox_vga_device::ramdac_read_index_w));
//	map(0x04, 0x04) Cursor/Overscan Color Write Index
//	map(0x05, 0x05) Cursor/Overscan Color data
//	map(0x07, 0x07) Cursor/Overscan Color Read Index
//	map(0x09, 0x09) Direct Cursor control
	map(0x0a, 0x0a).rw(FUNC(matrox_vga_device::ramdac_ext_indexed_r), FUNC(matrox_vga_device::ramdac_ext_indexed_w));
//	map(0x0b, 0x0b) Cursor RAM data
//	map(0x0c, 0x0f) Cursor X/Y positions
//	map(0x10, 0x1f) <reserved>
}

u8 matrox_vga_device::ramdac_ext_indexed_r()
{
	// Unclear from the docs, according to usage seems to be the write index with no autoincrement
	logerror("RAMDAC ext read [%02x]++\n", vga.dac.write_index);
	return m_ramdac_mode;
}

void matrox_vga_device::ramdac_ext_indexed_w(offs_t offset, u8 data)
{
	// TODO: really uses the palette write index, cheating for now
	m_ramdac_mode = data;
	logerror("RAMDAC ext [%02x]++ %02x\n", vga.dac.write_index, data);
	if (!machine().side_effects_disabled())
		vga.dac.write_index ++;
	svga.rgb16_en = (data & 0xf8) == 0xf0;
}

uint8_t matrox_vga_device::mem_r(offs_t offset)
{
	// TODO: MGA mode CRTCEXT3 bit 7
	if (svga.rgb16_en)
		return svga_device::mem_linear_r(offset + (svga.bank_w * 0x10000));
	return svga_device::mem_r(offset);
}

void matrox_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb16_en)
	{
		svga_device::mem_linear_w(offset + (svga.bank_w * 0x10000), data);
		return;
	}
	svga_device::mem_w(offset, data);
}

uint16_t matrox_vga_device::offset()
{
	// TODO: shifts depending on RAMDAC mode + CRTCEXT0 bits 5-4
	if (svga.rgb16_en)
		return (vga.crtc.offset << 5);
	return svga_device::offset();
}
