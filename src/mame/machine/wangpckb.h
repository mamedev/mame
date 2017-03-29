// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC keyboard emulation

*********************************************************************/

#pragma once

#ifndef __WANGPC_KEYBOARD__
#define __WANGPC_KEYBOARD__


#include "cpu/mcs51/mcs51.h"
#include "sound/sn76496.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define WANGPC_KEYBOARD_TAG "wangpckb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WANGPC_KEYBOARD_ADD() \
	MCFG_DEVICE_ADD(WANGPC_KEYBOARD_TAG, WANGPC_KEYBOARD, 0)

#define MCFG_WANGPCKB_TXD_HANDLER(_devcb) \
	devcb = &wangpc_keyboard_t::set_txd_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_keyboard_t

class wangpc_keyboard_t :  public device_t,
						   public device_serial_interface
{
public:
	// construction/destruction
	wangpc_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<wangpc_keyboard_t &>(device).m_txd_handler.set_callback(object); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( write_rxd );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_WRITE8_MEMBER( kb_p3_w );

	DECLARE_READ8_MEMBER( mcs51_rx_callback );
	DECLARE_WRITE8_MEMBER( mcs51_tx_callback );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

private:
	required_device<i8051_device> m_maincpu;
	required_ioport_array<16> m_y;
	devcb_write_line m_txd_handler;

	uint8_t m_keylatch;
	int m_rxd;
};


// device type definition
extern const device_type WANGPC_KEYBOARD;



#endif
