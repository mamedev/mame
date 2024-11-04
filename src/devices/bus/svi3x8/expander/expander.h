// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expander Slot

    50-pin slot

     1  +5V         2  /CNTRL2
     3  +12V        4  -12V
     5  /CNTRL1     6  /WAIT
     7  /RST        8  CPUCLK
     9  A15        10  A14
    11  A13        12  A12
    13  A11        14  A10
    15  A9         16  A8
    17  A7         18  A6
    19  A5         20  A4
    21  A3         22  A2
    23  A1         24  A0
    25  /RFSH      26  /EXCSR
    27  /M1        28  /EXCSW
    29  /WR        30  /MREQ
    31  /IORQ      32  /RD
    33  D0         34  D1
    35  D2         36  D3
    37  D4         38  D5
    39  D6         40  D7
    41  CSOUND     42  /INT
    43  /RAMDIS    44  /ROMDIS
    45  /BK32      46  /BK31
    47  /BK22      48  /BK21
    49  GND        50  GND

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_EXPANDER_EXPANDER_H
#define MAME_BUS_SVI3X8_EXPANDER_EXPANDER_H

#pragma once

// forward declaration
class device_svi_expander_interface;


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

class svi_expander_device :
	public device_t,
	public device_single_card_slot_interface<device_svi_expander_interface>,
	public device_mixer_interface
{
public:
	// construction/destruction
	template <typename T>
	svi_expander_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts)
		: svi_expander_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	svi_expander_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~svi_expander_device();

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto romdis_handler() { return m_romdis_handler.bind(); }
	auto ramdis_handler() { return m_ramdis_handler.bind(); }
	auto ctrl1_handler() { return m_ctrl1_handler.bind(); }
	auto ctrl2_handler() { return m_ctrl2_handler.bind(); }
	auto excsr_handler() { return m_excsr_handler.bind(); }
	auto excsw_handler() { return m_excsw_handler.bind(); }

	// called from cart device
	void int_w(int state) { m_int_handler(state); }
	void romdis_w(int state) { m_romdis_handler(state); }
	void ramdis_w(int state) { m_ramdis_handler(state); }
	void ctrl1_w(int state) { m_ctrl1_handler(state); }
	void ctrl2_w(int state) { m_ctrl2_handler(state); }

	uint8_t excs_r(offs_t offset) { return m_excsr_handler(offset); }
	void excs_w(offs_t offset, uint8_t data) { m_excsw_handler(offset, data); }

	// called from host
	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	void bk21_w(int state);
	void bk22_w(int state);
	void bk31_w(int state);
	void bk32_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	device_svi_expander_interface *m_module;

	devcb_write_line m_int_handler;
	devcb_write_line m_romdis_handler;
	devcb_write_line m_ramdis_handler;
	devcb_write_line m_ctrl1_handler;
	devcb_write_line m_ctrl2_handler;

	devcb_read8 m_excsr_handler;
	devcb_write8 m_excsw_handler;

	uint8_t m_dummy; // needed for save-state support
};


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

class device_svi_expander_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_svi_expander_interface();

	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }

	virtual void bk21_w(int state) { }
	virtual void bk22_w(int state) { }
	virtual void bk31_w(int state) { }
	virtual void bk32_w(int state) { }

protected:
	device_svi_expander_interface(const machine_config &mconfig, device_t &device);

	svi_expander_device *m_expander;
};

// device type declaration
DECLARE_DEVICE_TYPE(SVI_EXPANDER, svi_expander_device)

// include here so drivers don't need to
#include "modules.h"

#endif // MAME_BUS_SVI3X8_EXPANDER_EXPANDER_H
