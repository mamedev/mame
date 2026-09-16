// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1551 Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_PLUS4_C1551_H
#define MAME_BUS_PLUS4_C1551_H

#pragma once

#include "exp.h"
#include "cpu/m6502/m6510t.h"
#include "imagedev/floppy.h"
#include "machine/64h156.h"
#include "machine/6525tpi.h"
#include "machine/pla.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1551_device

class c1551_device : public device_t, public device_plus4_expansion_card_interface
{
public:
	// construction/destruction
	c1551_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_plus4_expansion_card_interface overrides
	virtual uint8_t plus4_cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;
	virtual void plus4_cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	uint8_t port_r();
	void port_w(uint8_t data);

	uint8_t tcbm_data_r();
	void tcbm_data_w(uint8_t data);
	uint8_t tpi0_pc_r();
	void tpi0_pc_w(uint8_t data);

	uint8_t tpi1_pb_r();
	uint8_t tpi1_pc_r();
	void tpi1_pc_w(uint8_t data);

	uint8_t tpi0_r(offs_t offset);
	void tpi0_w(offs_t offset, uint8_t data);

	void c1551_mem(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	enum
	{
		LED_POWER = 0,
		LED_ACT
	};

	bool tpi1_selected(offs_t offset);

	required_device<m6510t_device> m_maincpu;
	required_device<tpi6525_device> m_tpi0;
	required_device<tpi6525_device> m_tpi1;
	required_device<c64h156_device> m_ga;
	required_device<pls100_device> m_pla;
	required_device<floppy_image_device> m_floppy;
	required_device<plus4_expansion_slot_device> m_exp;
	required_ioport m_jp1;
	output_finder<2> m_leds;

	// TCBM bus
	uint8_t m_tcbm_data;                      // data
	int m_status;                           // status
	int m_dav;                              // data valid
	int m_ack;                              // acknowledge
	int m_dev;                              // device number

	// timers
	emu_timer *m_irq_timer;
};



// device type definition
DECLARE_DEVICE_TYPE(C1551, c1551_device)

#endif // MAME_BUS_PLUS4_C1551_H
