// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SD2IEC emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_SD2IEC_H
#define MAME_BUS_CBMIEC_SD2IEC_H

#pragma once

#include "cbmiec.h"
#include "cpu/avr8/avr8.h"
#include "machine/pcf8583.h"
#include "machine/spi_sdcard.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sd2iec_device

class sd2iec_device : public device_t,
					  public device_cbm_iec_interface,
					  public device_nvram_interface
{
public:
	// construction/destruction
	sd2iec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_clk(int state) override;
	virtual void cbm_iec_data(int state) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	required_device<atmega1284_device> m_maincpu;
	required_device<pcf8583_device> m_rtc;
	required_device<spi_sdcard_device> m_sdcard;
	output_finder<3> m_leds;
	uint8_t *m_eeprom = nullptr;

	int m_sdcard_miso = 0;

	uint8_t pa_r();
	void pa_w(uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	uint8_t pc_r();
	void pc_w(uint8_t data);
	uint8_t pd_r();
	void sdcard_miso_w(int state);

	void main_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SD2IEC, sd2iec_device)

#endif // MAME_BUS_CBMIEC_SD2IEC_H
