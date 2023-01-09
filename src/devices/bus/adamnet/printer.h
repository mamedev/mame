// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam printer controller emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_PRINTER_H
#define MAME_BUS_ADAMNET_PRINTER_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"
#include "machine/bitmap_printer.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_printer_device

class adam_printer_device :  public device_t,
								public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;
	required_device<daisywheel_bitmap_printer_device> m_bitmap_printer;

	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	uint8_t p3_r();
	uint8_t p4_r();
	void p4_w(uint8_t data);

	void adam_prn_mem(address_map &map);

	TIMER_CALLBACK_MEMBER(platen_motor_timer);
	emu_timer *m_platen_motor_timer = nullptr;

	uint16_t m_platen_counter = 0;
	uint8_t m_p4_data = 0;

	const std::string m_daisywheel = "w\x7f ,W.MZBFCA:RSETHONILDUGYPQKJV;X1234056789-$+#%={>]~['_`<)|(*@\\^?/}!&\"mjvgxdlbcorneaithsfpuqkyz";

	int mod_positive(uint16_t num, uint16_t mod_value)
	{
		int retvalue = num % mod_value;  if (retvalue < 0) retvalue += mod_value; return retvalue;
	}

	const int wheel_home_sensor_pos = 0;      // can set arbitrarily as long as you calculate wheel_offset() with function below
	const int wheel_home_sensor_origin = 10;  // determined empirically  (in half steps)
	int wheel_offset() { return (wheel_home_sensor_pos - wheel_home_sensor_origin) / (-2); }
	// sensor 0  wheel_offset = 5    (decreasing sensor by step of 2, increment offset by 1)
	// sensor 6  wheel_offset = 2
	// sensor 10 wheel_offset = 0
};

// device type definition
DECLARE_DEVICE_TYPE(ADAM_PRN, adam_printer_device)

#endif // MAME_BUS_ADAMNET_PRINTER_H
