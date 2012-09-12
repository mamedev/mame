/***************************************************************************

    machine/mpc105.h

    Motorola MPC105 PCI bridge

***************************************************************************/

#ifndef MPC105_H
#define MPC105_H

#include "machine/pci.h"

#define MPC105_MEMORYBANK_COUNT		8

// ======================> mpc105_interface

struct mpc105_interface
{
	const char *m_cputag;
	int m_bank_base_default;
};

// ======================> mpc105_device

class mpc105_device : public device_t,
					  public pci_device_interface,
					  public mpc105_interface
{
public:
    // construction/destruction
    mpc105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask);
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_config_complete();

	void update_memory();

private:
	int m_bank_base;
	UINT8 m_bank_enable;
	UINT32 m_bank_registers[8];

	cpu_device*   m_maincpu;
};


// device type definition
extern const device_type MPC105;

#endif /* MPC105_H */
