// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    Intel 440BX 82443BX Host section

    TODO:
    - better subclassing leap, we currently use i82339hx base for convenience

**************************************************************************************************/

#include "emu.h"
#include "i82443bx_host.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82443BX_HOST, i82443bx_host_device, "i82443bx_host", "Intel 82443BX PAC Host to PCI northbridge")

i82443bx_host_device::i82443bx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82439hx_host_device(mconfig, I82443BX_HOST, tag, owner, clock)
{
	// TODO: Device ID (DID) is 0x7192 when AGP_DIS is '1'
	// rev 0x02 82443BX B-1
	set_ids_host(0x80867190, 0x02, 0x00000000);
}

void i82443bx_host_device::config_map(address_map &map)
{
	i82439hx_host_device::config_map(map);

	// override first BAR slot for gfx window base address
	map(0x10, 0x13).rw(FUNC(pci_device::address_base_r), FUNC(pci_device::address_base_w));

	map(0x34, 0x34).r(FUNC(i82443bx_host_device::capptr_r));

//  map(0x50, 0x53).lr32(NAME([this] () { return machine().rand(); }));
	// TODO: no DRT, moved as DRTC / DWTC in dword format to $e0/$e8
	map(0x68, 0x68).rw(FUNC(i82443bx_host_device::fdhc_r), FUNC(i82443bx_host_device::fdhc_w));

	// TODO: midqslvr.cpp r/ws the AGPCTRL register but don't seem to match what's in datasheet
	// I also haven't yet enabled CAPPTR lolwut?
	//map(0xb0, 0xb3)
	map(0xd0, 0xd7).rw(FUNC(i82443bx_host_device::bspad_r), FUNC(i82443bx_host_device::bspad_w));
}

void i82443bx_host_device::apbase_map(address_map &map)
{
	// ...
}

void i82443bx_host_device::device_start()
{
	i82439hx_host_device::device_start();

	// TODO: size
	add_map(8*1024*1024, M_MEM, FUNC(i82443bx_host_device::apbase_map));

	save_item(NAME(m_fdhc));
	save_pointer(NAME(m_bspad), 8);
}

void i82443bx_host_device::device_reset()
{
	i82439hx_host_device::device_reset();

	m_fdhc = 0;
	for (int i = 0; i < 8; i++)
		m_bspad[i] = 0;
}

u8 i82443bx_host_device::capptr_r()
{
	return 0xa0;
}

// Register is different wrt i82439hx
std::tuple<bool, bool> i82443bx_host_device::read_memory_holes()
{
	const bool lower_hole = (m_fdhc & 0xc0) == 0x40;
	// TODO: 11 is "reserved"
	const bool upper_hole = (m_fdhc & 0xc0) != 0x80;
	return std::make_tuple(lower_hole, upper_hole);
}

u8 i82443bx_host_device::fdhc_r()
{
	return m_fdhc;
}

void i82443bx_host_device::fdhc_w(u8 data)
{
	m_fdhc = data;
	LOGIO("FDHC = %02x\n", data);
	remap_cb();
}

/*
 * BSPAD - BIOS Scratch Pad Register
 * 8 bytes of pseudo-RAM
 */
u8 i82443bx_host_device::bspad_r(offs_t offset)
{
	LOGIO("BSPAD[%d] R\n", offset);
	return m_bspad[offset];
}

void i82443bx_host_device::bspad_w(offs_t offset, u8 data)
{
	LOGIO("BSPAD[%d] = %02x\n", offset, data);
	m_bspad[offset] = data;
}

/*****************************
 *
 * Virtual PCI-to-PCI bridge implementation
 *
 ****************************/

DEFINE_DEVICE_TYPE(I82443BX_BRIDGE, i82443bx_bridge_device, "i82443bx_bridge", "Intel 82443BX Virtual PCI-to-PCI bridge")

i82443bx_bridge_device::i82443bx_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, I82443BX_BRIDGE, tag, owner, clock)
//  , m_vga(*this, finder_base::DUMMY_TAG)
{
	set_ids_bridge(0x80867191, 0x00);
}

void i82443bx_bridge_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
)
{
	if (BIT(bridge_control, 3))
	{
		//memory_space->install_device(0, 0xfffff, *m_vga, &sis630_gui_device::legacy_memory_map);
		//io_space->install_device(0, 0x0fff, *m_vga, &sis630_gui_device::legacy_io_map);
	}
}

void i82443bx_bridge_device::bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	pci_bridge_device::bridge_control_w(offset, data, mem_mask);
	LOGMAP("- %s VGA control\n", bridge_control & 8 ? "Enable" : "Disable");
	remap_cb();
}

void i82443bx_bridge_device::device_start()
{
	pci_bridge_device::device_start();
}

void i82443bx_bridge_device::device_reset()
{
	pci_bridge_device::device_reset();
}

