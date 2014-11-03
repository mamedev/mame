#include "i82875p.h"

const device_type I82875P_HOST = &device_creator<i82875p_host_device>;
const device_type I82875P_AGP  = &device_creator<i82875p_agp_device>;

DEVICE_ADDRESS_MAP_START(agp_translation_map, 32, i82875p_host_device)
ADDRESS_MAP_END

i82875p_host_device::i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_host_device(mconfig, I82875P_HOST, "i82875p northbridge", tag, owner, clock, "i82875p_host", __FILE__)
{
}

void i82875p_host_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void i82875p_host_device::set_ram_size(int _ram_size)
{
	ram_size = _ram_size;
}

void i82875p_host_device::device_start()
{
	pci_host_device::device_start();
	cpu = machine().device<cpu_device>(cpu_tag);
	memory_space = &cpu->space(AS_PROGRAM);
	io_space = &cpu->space(AS_IO);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	ram.resize(ram_size/4);

	// Resizeable with the apsize register
	add_map(256*1024*1024, M_MEM, FUNC(i82875p_host_device::agp_translation_map));
}

void i82875p_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void i82875p_host_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);
}




i82875p_agp_device::i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: agp_bridge_device(mconfig, I82875P_AGP, "i82875p AGP bridge", tag, owner, clock, "i82875p_agp", __FILE__)
{
}

void i82875p_agp_device::device_start()
{
	agp_bridge_device::device_start();
}

void i82875p_agp_device::device_reset()
{
	agp_bridge_device::device_reset();
}
