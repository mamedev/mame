// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II User Port emulation

**********************************************************************

                    GND       1      14      2D0
                    PB2       2      15      1D7
                    GND       3      16      1D6
                    PB3       4      17      1D5
                     PC       5      18      1D4
                   FLAG       6      19      1D3
                    2D7       7      20      1D2
                    2D6       8      21      1D1
                    2D5       9      22      1D0
                    2D4      10      23      CNT
                    2D3      11      24      +5V
                    2D2      12      25      IRQ
                    2D1      13      26      SP

**********************************************************************/

#pragma once

#ifndef __CBM2_USER_PORT__
#define __CBM2_USER_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CBM2_USER_PORT_TAG       "user"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CBM2_USER_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, CBM2_USER_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_CBM2_USER_PORT_IRQ_CALLBACK(_write) \
	devcb = &cbm2_user_port_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_CBM2_USER_PORT_SP_CALLBACK(_write) \
	devcb = &cbm2_user_port_device::set_sp_wr_callback(*device, DEVCB_##_write);

#define MCFG_CBM2_USER_PORT_CNT_CALLBACK(_write) \
	devcb = &cbm2_user_port_device::set_cnt_wr_callback(*device, DEVCB_##_write);

#define MCFG_CBM2_USER_PORT_FLAG_CALLBACK(_write) \
	devcb = &cbm2_user_port_device::set_flag_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cbm2_user_port_device;

// ======================> device_cbm2_user_port_interface

// class representing interface-specific live cbm2_expansion card
class device_cbm2_user_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_cbm2_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cbm2_user_port_interface() { }

	virtual UINT8 cbm2_d1_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void cbm2_d1_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual UINT8 cbm2_d2_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void cbm2_d2_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual int cbm2_pb2_r() { return 1; }
	virtual void cbm2_pb2_w(int state) { };
	virtual int cbm2_pb3_r() { return 1; }
	virtual void cbm2_pb3_w(int state) { };

	virtual void cbm2_pc_w(int state) { };
	virtual void cbm2_cnt_w(int state) { };
	virtual void cbm2_sp_w(int state) { };

protected:
	cbm2_user_port_device *m_slot;
};


// ======================> cbm2_user_port_device

class cbm2_user_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	cbm2_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cbm2_user_port_device() { }

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<cbm2_user_port_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_sp_wr_callback(device_t &device, _Object object) { return downcast<cbm2_user_port_device &>(device).m_write_sp.set_callback(object); }
	template<class _Object> static devcb_base &set_cnt_wr_callback(device_t &device, _Object object) { return downcast<cbm2_user_port_device &>(device).m_write_cnt.set_callback(object); }
	template<class _Object> static devcb_base &set_flag_wr_callback(device_t &device, _Object object) { return downcast<cbm2_user_port_device &>(device).m_write_flag.set_callback(object); }

	// computer interface
	DECLARE_READ8_MEMBER( d1_r ) { UINT8 data = 0xff; if (m_card != nullptr) data = m_card->cbm2_d1_r(space, offset); return data; }
	DECLARE_WRITE8_MEMBER( d1_w ) { if (m_card != nullptr) m_card->cbm2_d1_w(space, offset, data); }
	DECLARE_READ8_MEMBER( d2_r ) { UINT8 data = 0xff; if (m_card != nullptr) data = m_card->cbm2_d2_r(space, offset); return data; }
	DECLARE_WRITE8_MEMBER( d2_w ) { if (m_card != nullptr) m_card->cbm2_d2_w(space, offset, data); }
	DECLARE_READ_LINE_MEMBER( pb2_r ) { return m_card ? m_card->cbm2_pb2_r() : 1; }
	DECLARE_WRITE_LINE_MEMBER( pb2_w ) { if (m_card != nullptr) m_card->cbm2_pb2_w(state); }
	DECLARE_READ_LINE_MEMBER( pb3_r ) { return m_card ? m_card->cbm2_pb3_r() : 1; }
	DECLARE_WRITE_LINE_MEMBER( pb3_w ) { if (m_card != nullptr) m_card->cbm2_pb3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pc_w ) { if (m_card != nullptr) m_card->cbm2_pc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( cnt_w ) { if (m_card != nullptr) m_card->cbm2_cnt_w(state); }
	DECLARE_WRITE_LINE_MEMBER( sp_w ) { if (m_card != nullptr) m_card->cbm2_sp_w(state); }

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_sp_w ) { m_write_sp(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_cnt_w ) { m_write_cnt(state); }
	DECLARE_WRITE_LINE_MEMBER( flag_w ) { m_write_flag(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_sp;
	devcb_write_line   m_write_cnt;
	devcb_write_line   m_write_flag;

	device_cbm2_user_port_interface *m_card;
};


// device type definition
extern const device_type CBM2_USER_PORT;


// slot devices
SLOT_INTERFACE_EXTERN( cbm2_user_port_cards );



#endif
