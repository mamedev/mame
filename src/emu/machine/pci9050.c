/*********************************************************************

    pci9050.c - PLX PCI9050 PCI to 4x Local Bus Bridge
 
	by R. Belmont
 
    PCI spaces:
    0 - (config memory) not used
    1 - (config I/O) config regs
    2 - local bus 1 window
    3 - local bus 2 window
    4 - local bus 3 window
    5 - local bus 4 window
 
    PCI9050 is located, mapped, and initialized at BFC00700.
 
	The boot ROM then copies ROM to RAM, jumps to RAM, and starts trying to
    access Zeus 2 video through the mapped windows.
 
*********************************************************************/

#include "pci9050.h"

const device_type PCI9050 = &device_creator<pci9050_device>;

DEVICE_ADDRESS_MAP_START(map, 32, pci9050_device)
	AM_RANGE(0x00, 0x5f) AM_READWRITE(reg_r, reg_w)
ADDRESS_MAP_END

pci9050_device::pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, PCI9050, "PLX PCI9050 PCI to Local Bus Bridge", tag, owner, clock, "pci9050", __FILE__)
{
}

enum
{
	CFG_LS0_RANGE = 0,
	CFG_LS1_RANGE,
	CFG_LS2_RANGE,
	CFG_LS3_RANGE,
	CFG_ROM_RANGE,
	CFG_LS0_BASE,
	CFG_LS1_BASE,
	CFG_LS2_BASE,
	CFG_LS3_BASE,
	CFG_ROM_BASE,
	CFG_LS0_DESCRIPTORS,
	CFG_LS1_DESCRIPTORS,
	CFG_LS2_DESCRIPTORS,
	CFG_LS3_DESCRIPTORS,
	CFG_ROM_DESCRIPTORS,
	CFG_CS0_BASE,
	CFG_CS1_BASE,
	CFG_CS2_BASE,
	CFG_CS3_BASE,
	CFG_IRQ_CTRL_STATUS,
	CFG_MISC_CTRL
};

void pci9050_device::device_start()
{
	pci_device::device_start();

	skip_map_regs(1);	// we don't use map 0
	add_map(0x60, M_IO, FUNC(pci9050_device::map));			// map 1 is our config registers
}

void pci9050_device::device_config_complete()
{
}

void pci9050_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER (pci9050_device::reg_r)
{
	UINT32 result = m_regs[offset];

	logerror("%06X:PCI9050 read from reg %02x = %08x (mask %08x)\n", space.device().safe_pc(), offset*4, result, mem_mask);

	return result;
}

WRITE32_MEMBER(pci9050_device::reg_w)
{
	m_regs[offset] = data;

	switch (offset)
	{
		case CFG_LS0_RANGE:
			logerror("%06X:PCI9050 local bus 0 range %08x: %s flags %d pf %d addr bits 27-4 %08x\n", space.device().safe_pc(), data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
			break;

		case CFG_LS1_RANGE:
			logerror("%06X:PCI9050 local bus 1 range %08x: %s flags %d pf %d addr bits 27-4 %08x\n", space.device().safe_pc(), data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
			break;

		case CFG_LS2_RANGE:
			logerror("%06X:PCI9050 local bus 2 range %08x: %s flags %d pf %d addr bits 27-4 %08x\n", space.device().safe_pc(), data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
			break;

		case CFG_LS3_RANGE:
			logerror("%06X:PCI9050 local bus 3 range %08x: %s flags %d pf %d addr bits 27-4 %08x\n", space.device().safe_pc(), data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
			break;

		case CFG_ROM_RANGE:
			logerror("%06X:PCI9050 ROM range %08x: addr bits 27-11 %08x\n", space.device().safe_pc(), data, data & 0xfffff800);
			break;

		case CFG_LS0_BASE:
			logerror("%06X:PCI9050 local bus 0 base %08x: enable %d remap %08x\n", space.device().safe_pc(), data, data&1, data & 0x0ffffffe);
			break;

		case CFG_LS1_BASE:
			logerror("%06X:PCI9050 local bus 1 base %08x: enable %d remap %08x\n", space.device().safe_pc(), data, data&1, data & 0x0ffffffe);
			break;

		case CFG_LS2_BASE:
			logerror("%06X:PCI9050 local bus 2 base %08x: enable %d remap %08x\n", space.device().safe_pc(), data, data&1, data & 0x0ffffffe);
			break;

		case CFG_LS3_BASE:
			logerror("%06X:PCI9050 local bus 3 base %08x: enable %d remap %08x\n", space.device().safe_pc(), data, data&1, data & 0x0ffffffe);
			break;

		case CFG_ROM_BASE:
			logerror("%06X:PCI9050 ROM base %08x: remap %08x\n", space.device().safe_pc(), data, data & 0x0ffff800);
			break;

		case CFG_LS0_DESCRIPTORS:
			logerror("%06X:PCI9050 local bus 0 descriptors %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
			break;

		case CFG_LS1_DESCRIPTORS:
			logerror("%06X:PCI9050 local bus 1 descriptors %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
			break;

		case CFG_LS2_DESCRIPTORS:
			logerror("%06X:PCI9050 local bus 2 descriptors %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
			break;

		case CFG_LS3_DESCRIPTORS:
			logerror("%06X:PCI9050 local bus 3 descriptors %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
			break;

		case CFG_ROM_DESCRIPTORS:
			logerror("%06X:PCI9050 ROM descriptors %08x: burst %d prefetch %d bits %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
			break;

		case CFG_CS0_BASE:
			logerror("%06X:PCI9050 chip select 0 base %08x: enable %d size %08x\n", space.device().safe_pc(), data, data&1, data&0xfffffffe);
			break;

		case CFG_CS1_BASE:
			logerror("%06X:PCI9050 chip select 1 base %08x: enable %d size %08x\n", space.device().safe_pc(), data, data&1, data&0xfffffffe);
			break;

		case CFG_CS2_BASE:
			logerror("%06X:PCI9050 chip select 2 base %08x: enable %d size %08x\n", space.device().safe_pc(), data, data&1, data&0xfffffffe);
			break;

		case CFG_CS3_BASE:
			logerror("%06X:PCI9050 chip select 3 base %08x: enable %d size %08x\n", space.device().safe_pc(), data, data&1, data&0xfffffffe);
			break;

		case CFG_IRQ_CTRL_STATUS:
			logerror("%06X:PCI9050 IRQ control %08x\n", space.device().safe_pc(), data);
			break;

		case CFG_MISC_CTRL:
			logerror("%06X:PCI9050 misc control %08x\n", space.device().safe_pc(), data);
			break;

		default:
			logerror("%06X:PCI9050 write to offset %02x = %08x (mask %08x)\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;
	}

}
