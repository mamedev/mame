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

#include "emu.h"
#include "pci9050.h"

DEFINE_DEVICE_TYPE(PCI9050, pci9050_device, "pci9050", "PLX PCI9050 PCI to Local Bus Bridge")

void pci9050_device::map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(pci9050_device::lasrr_r), FUNC(pci9050_device::lasrr_w));
	map(0x10, 0x13).rw(FUNC(pci9050_device::eromrr_r), FUNC(pci9050_device::eromrr_w));
	map(0x14, 0x23).rw(FUNC(pci9050_device::lasba_r), FUNC(pci9050_device::lasba_w));
	map(0x24, 0x27).rw(FUNC(pci9050_device::eromba_r), FUNC(pci9050_device::eromba_w));
	map(0x28, 0x37).rw(FUNC(pci9050_device::lasbrd_r), FUNC(pci9050_device::lasbrd_w));
	map(0x38, 0x3b).rw(FUNC(pci9050_device::erombrd_r), FUNC(pci9050_device::erombrd_w));
	map(0x3c, 0x4b).rw(FUNC(pci9050_device::csbase_r), FUNC(pci9050_device::csbase_w));
	map(0x4c, 0x4f).rw(FUNC(pci9050_device::intcsr_r), FUNC(pci9050_device::intcsr_w));
	map(0x50, 0x53).rw(FUNC(pci9050_device::cntrl_r), FUNC(pci9050_device::cntrl_w));
}

pci9050_device::pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, PCI9050, tag, owner, clock)
	, m_user_input_handler(*this, 0), m_user_output_handler(*this)
{
	// "other bridge device"
	set_ids(0x10b59050, 0x01, 0x068000, 0x10b59050);
	for(int i=0; i<4; i++) {
		m_devices[i] = nullptr;
		m_names[i] = nullptr;
	}
}

void pci9050_device::set_map(int id, const address_map_constructor &map, device_t *device)
{
	m_maps[id] = map;
	m_devices[id] = device;
}

void pci9050_device::device_start()
{
	pci_device::device_start();

	add_map(0x80, M_MEM, FUNC(pci9050_device::map));           // map 0 is our config registers, mem space
	add_map(0x80, M_IO,  FUNC(pci9050_device::map));           // map 1 is our config registers, i/o space

	for(int i=0; i<4; i++)
		if(!m_maps[i].isnull())
			add_map(0, M_MEM | M_DISABLED, m_maps[i], m_devices[i]);
		else
			add_map(0, M_MEM | M_DISABLED, address_map_constructor(), nullptr);

	// Save states
	save_item(NAME(m_lasrr));
	save_item(NAME(m_lasba));
	save_item(NAME(m_lasbrd));
	save_item(NAME(m_csbase));
	save_item(NAME(m_eromrr));
	save_item(NAME(m_eromba));
	save_item(NAME(m_erombrd));
	save_item(NAME(m_intcsr));
	save_item(NAME(m_cntrl));
}

void pci9050_device::device_post_load()
{
	remap_rom();
	for (int id = 0; id < 4; id++)
		remap_local(id);
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
	uint32_t csbase = m_csbase[id];
	uint32_t lasrr = m_lasrr[id];
	logerror("local bus %d csbase=%08x lasrr=%08x\n", id, csbase, lasrr);

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
	// Address map is directly connected to PCI address space so post-decode mapping is not needed. (Ted Green)
	if(0 & csbase & 0x0fffffff & ~(size-1)) {
		logerror("PCI9050 local bus %d size=%08x csbase=%08X disabled due to unimplemented post-decode remapping\n", id, size, csbase);
		set_map_flags(id+2, M_MEM | M_DISABLED);
		return;
	}

	uint32_t mask = ~(size - 1);
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
	switch ((m_cntrl >> 12) & 0x3) {
	case 0:
	case 3:
		set_map_flags(0, M_MEM);
		set_map_flags(1, M_IO);
		break;
	case 1:
		set_map_flags(0, M_MEM);
		set_map_flags(1, M_IO | M_DISABLED);
		break;
	case 2:
		set_map_flags(0, M_MEM | M_DISABLED);
		set_map_flags(1, M_IO);
		break;
	}
}

uint32_t pci9050_device::lasrr_r(offs_t offset)
{
	return m_lasrr[offset];
}

void pci9050_device::lasrr_w(offs_t offset, uint32_t data)
{
	logerror("%s:PCI9050 local bus %d range = %08x: %s flags %d pf %d addr bits 27-4 %08x\n", machine().describe_context(), offset, data, (data & 1) ? "I/O" : "MEM", (data & 6)>>1, (data & 8)>>3, data & 0xfffffff);
	m_lasrr[offset] = data;
	remap_local(offset);
}

uint32_t pci9050_device::eromrr_r()
{
	return m_eromrr;
}

void pci9050_device::eromrr_w(uint32_t data)
{
	logerror("%s:PCI9050 ROM range = %08x: addr bits 27-11 %08x\n", machine().describe_context(), data, data & 0xfffff800);
	m_eromrr = data;
	remap_rom();
}

uint32_t pci9050_device::lasba_r(offs_t offset)
{
	return m_lasba[offset];
}

void pci9050_device::lasba_w(offs_t offset, uint32_t data)
{
	logerror("%s:PCI9050 local bus %d base = %08x: enable %d remap %08x\n", machine().describe_context(), offset, data, data&1, data & 0x0ffffffe);
	m_lasba[offset] = data;
	remap_local(offset);
}

uint32_t pci9050_device::eromba_r()
{
	return m_eromba;
}

void pci9050_device::eromba_w(uint32_t data)
{
	logerror("%s:PCI9050 ROM base = %08x: remap %08x\n", machine().describe_context(), data, data & 0x0ffff800);
	m_eromba = data;
	remap_rom();
}

uint32_t pci9050_device::lasbrd_r(offs_t offset)
{
	return m_lasbrd[offset];
}

void pci9050_device::lasbrd_w(offs_t offset, uint32_t data)
{
	logerror("%s:PCI9050 local bus %d descriptors = %08x: burst %d prefetch %d width %d, endian %s, endian mode %d\n", machine().describe_context(), offset, data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
	m_lasbrd[offset] = data;
	remap_local(offset);
}

uint32_t pci9050_device::erombrd_r()
{
	return m_erombrd;
}

void pci9050_device::erombrd_w(uint32_t data)
{
	logerror("%s:PCI9050 ROM descriptors = %08x: burst %d prefetch %d bits %d, endian %s, endian mode %d\n", machine().describe_context(), data, data&1, (data >> 5) & 1, (data >> 22) & 3, ((data >> 24) & 1) ? "BE" : "LE", (data >> 25) & 1);
	m_erombrd = data;
	remap_rom();
}

uint32_t pci9050_device::csbase_r(offs_t offset)
{
	return m_csbase[offset];
}

void pci9050_device::csbase_w(offs_t offset, uint32_t data)
{
	logerror("%s:PCI9050 chip select %d base = %08x: enable %d size %08x\n", machine().describe_context(), offset, data, data&1, data&0xfffffffe);
	m_csbase[offset] = data;
	remap_local(offset);
}

uint32_t pci9050_device::intcsr_r()
{
	logerror("%s:PCI9050 IRQ CSR read %08x\n", machine().describe_context(), m_intcsr);
	return m_intcsr;
}

void pci9050_device::intcsr_w(uint32_t data)
{
	logerror("%s:PCI9050 IRQ CSR write %08x\n", machine().describe_context(), data);
	m_intcsr = data;
	remap_rom();
}

uint32_t pci9050_device::cntrl_r()
{
	if (!m_user_input_handler.isunset())
	{
		int readData = m_user_input_handler();
		for (int userIndex = 0; userIndex < 4; userIndex++)
			if ((m_cntrl & (1 << (1 + userIndex * 3)))==0)
				m_cntrl = (m_cntrl & ~(1<<(2 + userIndex * 3))) | (((readData>>userIndex)&1) << (2 + userIndex * 3));
	}
	if (0)
		logerror("%s:PCI9050 CNTRL read = %08x\n", machine().describe_context(), m_cntrl);
	return m_cntrl;
}

void pci9050_device::cntrl_w(uint32_t data)
{
	if (0)
		logerror("%s:PCI9050 CNTRL write %08x\n", machine().describe_context(), data);
	uint32_t oldData = m_cntrl;
	m_cntrl = data;
	remap_rom();
	if ((oldData ^ m_cntrl) & 0x3000)
		remap_cb();
	if (!m_user_output_handler.isunset()) {
		int userData = 0;
		for (int userIndex = 0; userIndex < 4; userIndex++)
			userData |= ((m_cntrl >> (2 + userIndex * 3)) & 1) << userIndex;
		m_user_output_handler(userData);
	}
}
