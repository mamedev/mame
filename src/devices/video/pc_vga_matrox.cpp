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
//	map(0x00, 0x00) Address Generator Extensions
//	map(0x01, 0x01) Horizontal Counter Extensions
//	map(0x02, 0x02) Vertical Counter Extensions
//	map(0x03, 0x03) Miscellaneous
//	map(0x04, 0x04) Memory Page register
//	map(0x05, 0x05) Horizontal Video Half Count
//	map(0x06, 0x07) <Reserved>
//  \- $07 is actually checked by VESA test (PC=0xc62bb in rev3),
//     seems to disable SVGA drawing -> diagnostic check?
}
