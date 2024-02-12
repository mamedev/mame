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
	m_oak_space_config = address_space_config("oak_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(oak_oti111_vga_device::oak_map), this));
}

void oak_oti111_vga_device::device_add_mconfig(machine_config &config)
{
	XGA_COPRO(config, m_xga, 0);
	m_xga->mem_read_callback().set(FUNC(oak_oti111_vga_device::mem_linear_r));
	m_xga->mem_write_callback().set(FUNC(oak_oti111_vga_device::mem_linear_w));
	m_xga->set_type(xga_copro_device::TYPE::OTI111);
}

device_memory_interface::space_config_vector oak_oti111_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_oak_space_config));
	return r;
}

void oak_oti111_vga_device::device_start()
{
	svga_device::device_start();
}

void oak_oti111_vga_device::device_reset()
{
	svga_device::device_reset();
	// Spitfire BIOS doesn't explicitly set IOAS at boot
	m_ioas = true;
	m_memory_size = 0x0a;
}

void oak_oti111_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x0e, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return m_oak_idx;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_oak_idx = data;
		})
	);
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return space(EXT_REG).read_byte(m_oak_idx);
		}),
		NAME([this] (offs_t offset, u8 data) {
			space(EXT_REG).write_byte(m_oak_idx, data);
		})
	);
}



void oak_oti111_vga_device::oak_map(address_map &map)
{
	// status, set by BIOS for memory size
	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) {
			//LOG("OAK02: Status read\n");
			return m_memory_size;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK02: Status %02x\n", data);
			m_memory_size = data & 0x0e;
		})
	);
	//map(0x03, 0x03) OTI Test 1
	//map(0x04, 0x04) OTI Test 2
	//map(0x06, 0x06) Video Clock Select
	// Hardware Configuration 1
	map(0x07, 0x07).lr8(
		NAME([] () {
			// TODO: MD[7:0] pins
			return 0x91;
		})
	);
	// Hardware Configuration 2
	map(0x08, 0x08).lr8(
		NAME([] () {
			// TODO: MD[15:8] pins
			return 0x81;
		})
	);
	// Hardware Configuration 3
	map(0x09, 0x09).lr8(
		NAME([] () {
			// TODO: MD[23:16] pins
			return 0x00;
		})
	);
	// i2c Control
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) {
			// HACK: loopback DDC for now
			return (m_i2c_data & 0x3) << 4;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2c_data = data & 3;
		})
	);
	// DSW port
	map(0x0d, 0x0d).lr8(
		NAME([] () {
			// TODO: read_cb
			return 0xff;
		})
	);
	//map(0x0e, 0x0e) EEPROM Control
	//map(0x0f, 0x0f) Power Management Control

	//map(0x10, 0x10) Local Bus Control
	//map(0x11, 0x11) (backward-)Compatible Segment
	//map(0x13, 0x13) ISA Bus Control
	//map(0x14, 0x14) Video Memory Mapping
	//(bit 0 -> enable linear segment)
	//map(0x15, 0x15) Memory & MMIO Enable
	//(bit 7 -> enable MMIO)

	//map(0x19, 0x19) Configuration/DAC/Auxiliary Range
	//map(0x20, 0x20) Display FIFO Depth
	map(0x21, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return svga.rgb8_en << 2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK21: Mode Select %02x\n", data);
			svga.rgb8_en = BIT(data, 2);
			recompute_params();
		})
	);
	//map(0x22, 0x22) Feature Select
	//map(0x23, 0x23) Extended Read Segment
	//map(0x24, 0x24) Extended Write Segment
	//map(0x25, 0x25) Extended Common R/W Segment
	//map(0x26, 0x26) RASn Control
	//map(0x27, 0x27) CASn Control
	//map(0x28, 0x28) Refresh Control
	//map(0x29, 0x29) Hardware Window Aribtration

	//map(0x30, 0x30) OTI CRT Overflow
	//map(0x31, 0x31) CRT Start Address
	//map(0x32, 0x32) HSync/2 Start
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.no_wrap;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK33: CRT Address Compatibility %02x\n", data);
			vga.crtc.no_wrap = BIT(data, 0);
		})
	);
	//map(0x38, 0x38) Pixel i/f
	//map(0x39, 0x3a) Extended Overscan Color

	//HC = Hardware Cursor
	//HW = Hardware Window
	//map(0x80, 0x81) HC & HW window H Position Start
	//map(0x82, 0x83) HC & HW window V Position Start
	//map(0x84, 0x84) HC Horizontal Preset/HW Width Low
	//map(0x85, 0x85) HW Width High
	//map(0x86, 0x86) HC Vertical Preset/HW Height Low
	//map(0x87, 0x87) HW Height High
	//map(0x88, 0x8a) HC Start Address
	//map(0x8c, 0x8f) HC Color 0
	//map(0x90, 0x93) HC Color 1
	//map(0x94, 0x94) HC Control
	//map(0x96, 0x97) HW Control
	//map(0x98, 0x9a) HW Mask Map Start Address
	//map(0x9b, 0x9b) Multimedia Mask Map Offset
	//map(0x9c, 0x9e) HW Start Address
	//map(0x9f, 0x9f) HW Address Offset
	//map(0xa0, 0xa1) Video Window Width
	//map(0xa2, 0xa3) Video Window Height

	// Scratch Pad
	map(0xf0, 0xf7).lrw8(
		NAME([this] (offs_t offset) {
			return m_scratchpad[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_scratchpad[offset] = data;
		})
	);
}

// TODO: move this logic to standalone OAK XGA core
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

