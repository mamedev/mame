// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55-10828 "slow" floppy disk controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_LUXOR10828_H
#define MAME_BUS_ABCBUS_LUXOR10828_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ADDRESS_ABC830          45
#define ADDRESS_ABC832          44
#define ADDRESS_ABC834          44
#define ADDRESS_ABC850          44


#define DRIVE_TEAC_FD55F        0x01
#define DRIVE_BASF_6138         0x02
#define DRIVE_MICROPOLIS_1015F  0x03
#define DRIVE_BASF_6118         0x04
#define DRIVE_MICROPOLIS_1115F  0x05
#define DRIVE_BASF_6106_08      0x08
#define DRIVE_MPI_51            0x09
#define DRIVE_BASF_6105         0x0e
#define DRIVE_BASF_6106         0x0f



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_55_10828_device

class luxor_55_10828_device :  public device_t,
								public device_abcbus_card_interface
{
public:
	// construction/destruction
	luxor_55_10828_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;

private:
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);
	uint8_t pio_pb_r();
	void pio_pb_w(uint8_t data);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	void ctrl_w(uint8_t data);
	void status_w(uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	static void floppy_formats(format_registration &fr);

	void luxor_55_10828_io(address_map &map) ATTR_COLD;
	void luxor_55_10828_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_ioport m_sw1;
	required_ioport m_s1;

	bool m_cs;              // card selected
	uint8_t m_status;       // ABC BUS status
	uint8_t m_data;         // ABC BUS data
	bool m_fdc_irq;         // floppy interrupt
	bool m_fdc_drq;         // floppy data request
	int m_wait_enable;      // wait enable
	uint8_t m_sel;          // drive select
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_55_10828, luxor_55_10828_device)

#endif // MAME_BUS_ABCBUS_LUXOR10828_H
