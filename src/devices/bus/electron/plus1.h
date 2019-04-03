// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ALA11 - Acorn Plus 1

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_PLUS1_H
#define MAME_BUS_ELECTRON_PLUS1_H

#include "exp.h"
#include "softlist.h"
#include "machine/adc0844.h"
#include "bus/centronics/ctronics.h"
#include "bus/electron/cart/slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_plus1_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_plus1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	u8 status_r();
	DECLARE_WRITE_LINE_MEMBER(busy_w);
	DECLARE_WRITE_LINE_MEMBER(ready_w);

	required_memory_region m_exp_rom;
	required_device<electron_cartslot_device> m_cart_sk1;
	required_device<electron_cartslot_device> m_cart_sk2;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<adc0844_device> m_adc;
	required_ioport_array<4> m_joy;
	required_ioport m_buttons;

	uint8_t m_romsel;
	int m_centronics_busy;
	int m_adc_ready;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_PLUS1, electron_plus1_device)


#endif /* MAME_BUS_ELECTRON_PLUS1_H */
