// license:BSD-3-Clause
// copyright-holders:R. Belmont
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
	AM_RANGE(0x00, 0x0f) AM_READWRITE(lasrr_r,   lasrr_w  )
	AM_RANGE(0x10, 0x13) AM_READWRITE(eromrr_r,  eromrr_w )
	AM_RANGE(0x14, 0x23) AM_READWRITE(lasba_r,   lasba_w  )
	AM_RANGE(0x24, 0x27) AM_READWRITE(eromba_r,  eromba_w )
	AM_RANGE(0x28, 0x37) AM_READWRITE(lasbrd_r,  lasbrd_w )
	AM_RANGE(0x38, 0x3b) AM_READWRITE(erombrd_r, erombrd_w)
	AM_RANGE(0x3c, 0x4b) AM_READWRITE(csbase_r,  csbase_w )
	AM_RANGE(0x4c, 0x4f) AM_READWRITE(intcsr_r,  intcsr_w )
	AM_RANGE(0x50, 0x53) AM_READWRITE(cntrl_r,   cntrl_w  )
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(empty, 32, pci9050_device)
ADDRESS_MAP_END

pci9050_device::pci9050_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, PCI9050, "PLX PCI9050 PCI to Local Bus Bridge", tag, owner, clock, "pci9050", __FILE__)
{
	for(int i=0; i<4; i++) {
		m_devices[i] = nullptr;
		m_names[i] = nullptr;
	}
}

void pci9050_device::set_map(int id, address_map_constructor map, const char *name, device_t *device)
{
	m_maps[id] = map;
	m_names[id] = name;
	m_devices[id] = device;
}

void pci9050_device::device_start()
{
	typedef void (pci9050_device::*tramp_t)(address_map &, device_t &);
	static const tramp_t trampolines[4] = {
		&pci9050_device::map_trampoline<0>,
		&pci9050_device::map_trampoline<1>,
		&pci9050_device::map_trampoline<2>,
		&pci9050_device::map_trampoline<3>
	};

	pci_device::device_start();

	add_map(0x100, M_MEM, FUNC(pci9050_device::map));           // map 0 is our config registers, mem space
	add_map(0x100, M_IO,  FUNC(pci9050_device::map));           // map 1 is our config registers, i/o space

	for(int i=0; i<4; i++)
		if(m_names[i])
			//          add_map(0, M_MEM | M_DISABLED, m_maps[i], m_names[i], m_devices[i]);
			add_map(0, M_MEM | M_DISABLED, trampolines[i], m_names[i]);
		else
			add_map(0, M_MEM | M_DISABLED, FUNC(pci9050_device::empty));
}

void pci9050_device::device_config_complete()
{
}

void pci9050_device::device_reset()
{
	pci_device::device_reset();
	set_map_address(0, 0);
	set_map_address(1, 0);
	for(int i=0; i<4; i++) {
		m_lasrr[i] = i ? 0 : 0x0ff00000;
		m_lasba[i] = 0;
		m_lasbrd[i] = 0x00800000;
		m_csbase[i] = 0;
		set_map_flags(i+2, M_MEM | M_DISABLED);
	}
	m_eromrr = 0x07ff8000;
	m_eromba = 0x00080000;
	m_erombrd = 0x00800000;
	m_intcsr = 0;
	m_cntrl = 0;
}

void pci9050_device::remap_local(int id)
{
	UINT32 csbase = m_csbase[id];
	UINT32 lasrr = m_lasrr[id];
	logerror("%d csbase=%08x lasrr=%08x\n", id, csbase, lasrr);

	if(!(csbase & 1)) {
		set_map_flags(id+2, M_MEM | M_DISABLED);
		return;
	}
	int lsize;
	for(lsize=1; lsize<28 && !(csbase & (1<<lsize)); lsize++) {};
	if(lsize == 28) {
		set_map_flags(id+2, M_MEM | M_DISABLED);
		return;
	}
	int size = 2 << lsize;
	if(csbase & 0x0fffffff & ~(size-1)) {
		logerror("PCI9050 local bus %d disabled due to unimplemented post-decode remapping\n", id);
		//      set_map_flags(id+2, M_MEM | M_DISABLED);
		//      return;
	}

	UINT32 mask = ~(size - 1);
	if(lasrr & 1)
		mask &= 0x0ffffffc;
	else
		mask &= 0x0ffffff0;

	if((lasrr & mask) != mask) {
		logerror("PCI9050 local bus %d disabled due to unimplemented pci mirroring\n", id);
		//      set_map_flags(id+2, M_MEM | M_DISABLED);
		//      return;
	}

	set_map_size(id+2, size);
	set_map_flags(id+2, lasrr & 1 ? M_IO : lasrr & 8 ? M_MEM | M_PREF : M_MEM);
}

void pci9050_device::remap_rom()
{
}

READ32_MEMBER (pci9050_device::lasrr_r)
{
	return m_lasrr[offset];
}

WRITE32_MEMBER(pci9050_device::lasrr_w)
{
	logerror("%06X:PCI9050 local bus %d range %08x: %s flags %d pf %d addr bits 27-4 %08x\n", space.device().safe_pc(), offset, data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
	m_lasrr[offset] = data;
	remap_local(offset);
}

READ32_MEMBER (pci9050_device::eromrr_r)
{
	return m_eromrr;
}

WRITE32_MEMBER(pci9050_device::eromrr_w)
{
	logerror("%06X:PCI9050 ROM range %08x: addr bits 27-11 %08x\n", space.device().safe_pc(), data, data & 0xfffff800);
	m_eromrr = data;
	remap_rom();
}

READ32_MEMBER (pci9050_device::lasba_r)
{
	return m_lasba[offset];
}

WRITE32_MEMBER(pci9050_device::lasba_w)
{
	logerror("%06X:PCI9050 local bus %d base %08x: enable %d remap %08x\n", space.device().safe_pc(), offset, data, data&1, data & 0x0ffffffe);
	m_lasba[offset] = data;
	remap_local(offset);
}

READ32_MEMBER (pci9050_device::eromba_r)
{
	return m_eromba;
}

WRITE32_MEMBER(pci9050_device::eromba_w)
{
	logerror("%06X:PCI9050 ROM base %08x: remap %08x\n", space.device().safe_pc(), data, data & 0x0ffff800);
	m_eromba = data;
	remap_rom();
}

READ32_MEMBER (pci9050_device::lasbrd_r)
{
	return m_lasbrd[offset];
}

WRITE32_MEMBER(pci9050_device::lasbrd_w)
{
	logerror("%06X:PCI9050 local bus %d descriptors %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", space.device().safe_pc(), offset, data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
	m_lasbrd[offset] = data;
	remap_local(offset);
}

READ32_MEMBER (pci9050_device::erombrd_r)
{
	return m_erombrd;
}

WRITE32_MEMBER(pci9050_device::erombrd_w)
{
	logerror("%06X:PCI9050 ROM descriptors %08x: burst %d prefetch %d bits %d, endian %s, endian mode %d\n", space.device().safe_pc(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
	m_erombrd = data;
	remap_rom();
}

READ32_MEMBER (pci9050_device::csbase_r)
{
	return m_csbase[offset];
}

WRITE32_MEMBER(pci9050_device::csbase_w)
{
	logerror("%06X:PCI9050 chip select %d base %08x: enable %d size %08x\n", space.device().safe_pc(), offset, data, data&1, data&0xfffffffe);
	m_csbase[offset] = data;
	remap_local(offset);
}

READ32_MEMBER (pci9050_device::intcsr_r)
{
	return m_intcsr;
}

WRITE32_MEMBER(pci9050_device::intcsr_w)
{
	logerror("%06X:PCI9050 IRQ control %08x\n", space.device().safe_pc(), data);
	m_intcsr = data;
	remap_rom();
}

READ32_MEMBER (pci9050_device::cntrl_r)
{
	return m_cntrl;
}

WRITE32_MEMBER(pci9050_device::cntrl_w)
{
	logerror("%06X:PCI9050 IRQ control %08x\n", space.device().safe_pc(), data);
	m_cntrl = data;
	remap_rom();
}
