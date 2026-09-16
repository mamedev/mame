// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "pc_vga_alliance.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PROMOTION_VGA,  promotion_vga_device,  "promotion_vga",  "Alliance ProMotion VGA i/f")

promotion_vga_device::promotion_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, PROMOTION_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(promotion_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(promotion_vga_device::sequencer_map), this));
	// TODO: ATT20C408-13 RAMDAC (with bit 9 of address space), BIOS does extensive signature checks at POST
}

static INPUT_PORTS_START(promotion_vga_sense)
	PORT_START("VGA_SENSE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // read at POST
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

ioport_constructor promotion_vga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(promotion_vga_sense);
}

void promotion_vga_device::device_start()
{
	svga_device::device_start();
}

void promotion_vga_device::device_reset()
{
	svga_device::device_reset();

	m_remap_blt = 0;
	m_remap_mem = 0;
}

// TODO: lock mechanism
void promotion_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// TODO: $19-$1e for extended regs
}

// TODO: lock mechanism
void promotion_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	map(0x10, 0xff).unmaprw();
	map(0x1b, 0x1b).lrw8(
		NAME([this]() { return (m_remap_blt & 0x7) << 3 | (m_remap_mem & 0x7); }),
		NAME([this] (offs_t offset, u8 data) {
			m_remap_blt = (data >> 3) & 0x7;
			m_remap_mem = (data >> 0) & 0x7;
			LOG("aT1B: Remap control %02x (host blt %01x promotion %01x)\n", data, m_remap_blt, m_remap_mem);
		})
	);
}

uint8_t promotion_vga_device::mem_r(offs_t offset)
{
	if (m_remap_mem == 1 && (offset & 0x1f800) == 0x00000)
	{
		LOG("aT Ext Reg: [%03x] R\n", offset );
		return 0xff;
	}
	return svga_device::mem_r(offset);
}

void promotion_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (m_remap_mem == 1 && (offset & 0x1f800) == 0x00000)
	{
		LOG("aT Ext Reg: [%03x] W %02x\n", offset, data );
		return;
	}
	svga_device::mem_w(offset, data);
}
