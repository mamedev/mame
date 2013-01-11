/***************************************************************************

    Intel 82439TX System Controller (MTXC)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef __I82439TX_H__
#define __I82439TX_H__

#include "machine/pci.h"
#include "machine/northbridge.h"

// ======================> i82439tx_interface

struct i82439tx_interface
{
	const char *m_cputag;
	const char *m_rom_region;
};

// ======================> i82439tx_device

class i82439tx_device :  public northbridge_device,
							public pci_device_interface,
							public i82439tx_interface
{
public:
	// construction/destruction
	i82439tx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask);
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	void i82439tx_configure_memory(UINT8 val, offs_t begin, offs_t end);

private:
	address_space *m_space;
	UINT8 *m_rom;

	UINT32 m_regs[8];
	UINT32 m_bios_ram[0x40000 / 4];

};

// device type definition
extern const device_type I82439TX;

#endif /* __I82439TX_H__ */
