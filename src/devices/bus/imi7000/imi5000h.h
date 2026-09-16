// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IMI 5000H 5.25" Winchester Hard Disk Controller emulation

    Used in Corvus Systems H-Series drives (Model 6/11/20)

**********************************************************************/

#ifndef MAME_BUS_IMI7000_IMI5000H_H
#define MAME_BUS_IMI7000_IMI5000H_H

#pragma once

#include "imi7000.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> imi5000h_device

class imi5000h_device :  public device_t,
							public device_imi7000_interface
{
public:
	// construction/destruction
	imi5000h_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);

	uint8_t pio0_pa_r();
	void pio0_pa_w(uint8_t data);
	uint8_t pio0_pb_r();
	void pio0_pb_w(uint8_t data);

	uint8_t pio2_pa_r();
	void pio2_pa_w(uint8_t data);
	uint8_t pio2_pb_r();
	void pio2_pb_w(uint8_t data);

	uint8_t pio3_pa_r();
	void pio3_pa_w(uint8_t data);
	uint8_t pio3_pb_r();
	void pio3_pb_w(uint8_t data);

	void imi5000h_io(address_map &map) ATTR_COLD;
	void imi5000h_mem(address_map &map) ATTR_COLD;

	enum
	{
		LED_FAULT,
		LED_BUSY,
		LED_READY
	};

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_ioport m_lsi11;
	required_ioport m_mux;
	required_ioport m_format;
	required_ioport m_ub4;
};


// device type definition
DECLARE_DEVICE_TYPE(IMI5000H, imi5000h_device)

#endif // MAME_BUS_IMI7000_IMI5000H_H
