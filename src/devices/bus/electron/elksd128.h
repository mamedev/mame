// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD128 Electron SD Interface

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_ELKSD128_H
#define MAME_BUS_ELECTRON_ELKSD128_H

#include "exp.h"
#include "machine/spi_sdcard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_elksd128_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_elksd128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_flash;
	required_device<spi_sdcard_device> m_sdcard;
	required_ioport m_joy;

	uint8_t m_romsel;
	uint8_t m_adc_channel;
	uint8_t m_swr_lock;

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
DECLARE_DEVICE_TYPE(ELECTRON_ELKSD128, electron_elksd128_device)


#endif /* MAME_BUS_ELECTRON_ELKSD128_H */
