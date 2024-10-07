// license:BSD-3-Clause
// copyright-holders:Dirk Best, hap
/*******************************************************************************

    Saitek OSA Expansion Slot

    15-pin slot "PIO"

    STB-P    <
    D0       <>
    D1       <>
    D2       <>
    D3       <>
    D4       <>
    D5       <>
    D6       <>
    D7       <>
    ACK-P     >
    RTS-P    <
    PW        >
    GND       >
    NMI-P     >
    V+        >

*******************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_EXPANSION_H
#define MAME_BUS_SAITEKOSA_EXPANSION_H

#pragma once

#include "screen.h"

DECLARE_DEVICE_TYPE(SAITEKOSA_EXPANSION, saitekosa_expansion_device)


//******************************************************************************
//  TYPE DEFINITIONS
//******************************************************************************

class device_saitekosa_expansion_interface;

// ======================> saitekosa_expansion_device

class saitekosa_expansion_device : public device_t, public device_single_card_slot_interface<device_saitekosa_expansion_interface>
{
public:
	// construction/destruction
	template <typename T>
	saitekosa_expansion_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts) :
		saitekosa_expansion_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	saitekosa_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~saitekosa_expansion_device();

	// callbacks
	auto stb_handler() { return m_stb_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	// called from module device
	void stb_w(int state) { m_stb_handler(state); }
	void rts_w(int state) { m_rts_handler(state); }

	u8 data_state() { return m_data; }
	int nmi_state() { return m_nmi; }
	int ack_state() { return m_ack; }
	int pw_state() { return m_pw; }

	// called from host
	u8 data_r();
	void data_w(u8 data);
	void nmi_w(int state);
	void ack_w(int state);
	void pw_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	devcb_write_line m_stb_handler;
	devcb_write_line m_rts_handler;

	// input pins state
	u8 m_data = 0;
	int m_nmi = 0;
	int m_ack = 0;
	int m_pw = 0;

	device_saitekosa_expansion_interface *m_module;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// ======================> device_saitekosa_expansion_interface

class device_saitekosa_expansion_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_saitekosa_expansion_interface();

	virtual u8 data_r() { return 0xff; }
	virtual void data_w(u8 data) { }
	virtual void nmi_w(int state) { }
	virtual void ack_w(int state) { }
	virtual void pw_w(int state) { }

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }

protected:
	device_saitekosa_expansion_interface(const machine_config &mconfig, device_t &device);

	saitekosa_expansion_device *m_expansion;
};

void saitekosa_expansion_modules(device_slot_interface &device);


#endif // MAME_BUS_SAITEKOSA_EXPANSION_H
