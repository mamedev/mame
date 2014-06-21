/***************************************************************************

    Toshiba TMPZ84C015, TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC(6/8MHz), PIO, WDT

***************************************************************************/

#include "emu.h"
#include "z80.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"

// If an external daisy chain is used, insert this before your own device tags:
#define TMPZ84C015_DAISY_INTERNAL { "ctc" }, { "sio" }, { "pio" }

// NOTE: for callbacks, see machine/z80dart.h, machine/z80ctc.h, machine/z80pio.h


class tmpz84c015_device : public z80_device
{
public:
	tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// devices/pointers
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_sio;
	required_device<z80pio_device> m_pio;

	DECLARE_WRITE8_MEMBER(irq_priority_w);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

	const address_space_config m_io_space_config;

	const address_space_config *memory_space_config(address_spacenum spacenum) const
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	}

private:
	UINT8 m_irq_priority;
};

extern const device_type TMPZ84C015;
