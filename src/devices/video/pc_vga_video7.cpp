// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Video7 / Headland / SPEA (S)VGA chipsets

TODO:
- drops in MDA mode with IOAS = 1 (EGA switch or extended switch at $f7?)

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_video7.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(HT208_VIDEO7_VGA,    ht208_video7_vga_device,    "ht208_video7_vga",    "Headland HT208 VGA i/f")

ht208_video7_vga_device::ht208_video7_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(ht208_video7_vga_device::sequencer_map), this));
}

ht208_video7_vga_device::ht208_video7_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ht208_video7_vga_device(mconfig, HT208_VIDEO7_VGA, tag, owner, clock)
{
}

void ht208_video7_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	vga.crtc.maximum_scan_line = 1;

	save_item(NAME(m_seq_unlock_reg));
	save_item(NAME(m_ext_b0_scratch));
	save_item(NAME(m_ext_fbctrl));
	save_item(NAME(m_ext_16bit));
}

void ht208_video7_vga_device::device_reset()
{
	svga_device::device_reset();

	std::fill(std::begin(m_ext_b0_scratch), std::end(m_ext_b0_scratch), 0);
	m_seq_unlock_reg = false;
	m_ext_fbctrl = 0;
	m_ext_16bit = 0;
}

void ht208_video7_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	map(0x08, 0xff).unmaprw();

	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			// xxxx xxx- <unused>
			// ---- ---x unlocked if '1'
			return m_seq_unlock_reg;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: log & actual space unlock
			if (data == 0xea)
				m_seq_unlock_reg = true;
			if (data == 0xae)
				m_seq_unlock_reg = false;
		})
	);

	map(0xa4, 0xa4).lrw8(
		NAME([this] (offs_t offset) { return m_ext_clock_select; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("ERA4: Clock Select %02x\n", data);
			m_ext_clock_select = data;
		})
	);
	map(0xb0, 0xbf).lrw8(
		NAME([this] (offs_t offset) { return m_ext_b0_scratch[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_ext_b0_scratch[offset] = data;
		})
	);

	map(0xf7, 0xf7).lr8(
		NAME([] (offs_t offset) { return 0; })
	);

	map(0xfe, 0xfe).lrw8(
		NAME([this] (offs_t offset) { return m_ext_fbctrl; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("ERFE: Foreground/Background Control %02x\n", data);
			m_ext_fbctrl = data;
		})
	);
	map(0xff, 0xff).lrw8(
		NAME([this] (offs_t offset) { return m_ext_16bit; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("ERFF: 16-bit i/f Control %02x\n", data);
			m_ext_16bit = data;
		})
	);
}
