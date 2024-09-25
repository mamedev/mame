// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD-Plus 1 Electron SD Cartridge

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_ELKSDP1_H
#define MAME_BUS_ELECTRON_CART_ELKSDP1_H

#include "slot.h"
#include "machine/spi_sdcard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_elksdp1_device : public device_t, public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_elksdp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_device<spi_sdcard_device> m_sdcard;

	TIMER_CALLBACK_MEMBER(spi_clock);

	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_clock_sysclk;
	int m_spi_clock_cycles;
	int m_in_bit;
	uint8_t m_in_latch;
	uint8_t m_out_latch;

	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ELKSDP1, electron_elksdp1_device)


#endif // MAME_BUS_ELECTRON_CART_ELKSDP1_H
