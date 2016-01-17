// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82439TX System Controller (MTXC)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef __I82439TX_H__
#define __I82439TX_H__

#include "pci.h"
#include "northbridge.h"

#define MCFG_I82439TX_CPU( _tag ) \
	i82439tx_device::static_set_cpu(*device, _tag);

#define MCFG_I82439TX_REGION( _tag ) \
	i82439tx_device::static_set_region(*device, _tag);

// ======================> i82439tx_device

class i82439tx_device :  public northbridge_device,
	public pci_device_interface
{
public:
	// construction/destruction
	i82439tx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_cpu(device_t &device, std::string tag) { dynamic_cast<i82439tx_device &>(device).m_cpu_tag = tag; }
	static void static_set_region(device_t &device, std::string tag) { dynamic_cast<i82439tx_device &>(device).m_region_tag = tag; }

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void i82439tx_configure_memory(UINT8 val, offs_t begin, offs_t end);

private:
	std::string m_cpu_tag;
	std::string m_region_tag;

	address_space *m_space;
	UINT8 *m_rom;

	UINT32 m_regs[8];
	UINT32 m_bios_ram[0x40000 / 4];
};

// device type definition
extern const device_type I82439TX;

#endif /* __I82439TX_H__ */
