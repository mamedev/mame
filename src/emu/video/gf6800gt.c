#include "gf6800gt.h"

const device_type GEFORCE_6800GT = &device_creator<geforce_6800gt_device>;

DEVICE_ADDRESS_MAP_START(map1, 32, geforce_6800gt_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map2, 32, geforce_6800gt_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map3, 32, geforce_6800gt_device)
ADDRESS_MAP_END

geforce_6800gt_device::geforce_6800gt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, GEFORCE_6800GT, "NVidia GeForce 6800GT", tag, owner, clock, "geforce_6800gt", __FILE__)
{
}

void geforce_6800gt_device::device_start()
{
	pci_device::device_start();
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_6800gt_device::map1));
	add_map(256*1024*1024, M_MEM, FUNC(geforce_6800gt_device::map2));
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_6800gt_device::map3));
}

void geforce_6800gt_device::device_reset()
{
	pci_device::device_reset();
}
