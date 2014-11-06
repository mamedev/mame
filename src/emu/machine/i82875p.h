// Intel i82875p northbridge

#ifndef I82875P_H
#define I82875P_H

#include "pci.h"

#define MCFG_I82875P_HOST_ADD(_tag, _subdevice_id, _cpu_tag, _ram_size)			\
	MCFG_PCI_HOST_ADD(_tag, I82875P_HOST, 0x80862578, 0x02, _subdevice_id) \
	downcast<i82875p_host_device *>(device)->set_cpu_tag(_cpu_tag); \
	downcast<i82875p_host_device *>(device)->set_ram_size(_ram_size);

#define MCFG_I82875P_AGP_ADD(_tag) \
	MCFG_AGP_BRIDGE_ADD(_tag, I82875P_AGP, 0x80862579, 0x02)

class i82875p_host_device : public pci_host_device {
public:
	i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_cpu_tag(const char *tag);
	void set_ram_size(int ram_size);

	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
						   UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(agp_translation_map, 32);

	const char *cpu_tag;
	int ram_size;
	cpu_device *cpu;
	dynamic_array<UINT32> ram;
};

class i82875p_agp_device : public agp_bridge_device {
public:
	i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};

extern const device_type I82875P_HOST;
extern const device_type I82875P_AGP;


#endif
