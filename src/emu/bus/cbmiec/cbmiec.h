// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEC Serial Bus emulation

**********************************************************************/

#pragma once

#ifndef __CBM_IEC__
#define __CBM_IEC__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CBM_IEC_TAG         "iec_bus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CBM_IEC_BUS_ADD() \
	MCFG_DEVICE_ADD(CBM_IEC_TAG, CBM_IEC, 0)


#define MCFG_CBM_IEC_BUS_SRQ_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_srq_callback(DEVCB_##_write);

#define MCFG_CBM_IEC_BUS_ATN_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_atn_callback(DEVCB_##_write);

#define MCFG_CBM_IEC_BUS_CLK_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_clk_callback(DEVCB_##_write);

#define MCFG_CBM_IEC_BUS_DATA_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_data_callback(DEVCB_##_write);

#define MCFG_CBM_IEC_BUS_RESET_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_reset_callback(DEVCB_##_write);


#define MCFG_CBM_IEC_SLOT_ADD(_tag, _address, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, CBM_IEC_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<cbm_iec_slot_device *>(device)->set_address(_address);


#define MCFG_CBM_IEC_ADD(_default_drive) \
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, cbm_iec_devices, _default_drive) \
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_BUS_ADD()



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm_iec_device

class cbm_iec_slot_device;
class device_cbm_iec_interface;

class cbm_iec_device : public device_t
{
public:
	// construction/destruction
	cbm_iec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _write> void set_srq_callback(_write wr) { m_write_srq.set_callback(wr); }
	template<class _write> void set_atn_callback(_write wr) { m_write_atn.set_callback(wr); }
	template<class _write> void set_clk_callback(_write wr) { m_write_clk.set_callback(wr); }
	template<class _write> void set_data_callback(_write wr) { m_write_data.set_callback(wr); }
	template<class _write> void set_reset_callback(_write wr) { m_write_reset.set_callback(wr); }

	void add_device(cbm_iec_slot_device *slot, device_t *target);

	// reads for both host and peripherals
	DECLARE_READ_LINE_MEMBER( srq_r ) { return get_signal(SRQ); }
	DECLARE_READ_LINE_MEMBER( atn_r ) { return get_signal(ATN); }
	DECLARE_READ_LINE_MEMBER( clk_r ) { return get_signal(CLK); }
	DECLARE_READ_LINE_MEMBER( data_r ) { return get_signal(DATA); }
	DECLARE_READ_LINE_MEMBER( reset_r ) { return get_signal(RESET); }

	// writes for host (driver_device)
	DECLARE_WRITE_LINE_MEMBER( srq_w ) { set_signal(this, SRQ, state); }
	DECLARE_WRITE_LINE_MEMBER( atn_w ) { set_signal(this, ATN, state); }
	DECLARE_WRITE_LINE_MEMBER( clk_w ) { set_signal(this, CLK, state); }
	DECLARE_WRITE_LINE_MEMBER( data_w ) { set_signal(this, DATA, state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { set_signal(this, RESET, state); }

	// writes for peripherals (device_t)
	void srq_w(device_t *device, int state) { set_signal(device, SRQ, state); }
	void atn_w(device_t *device, int state) { set_signal(device, ATN, state); }
	void clk_w(device_t *device, int state) { set_signal(device, CLK, state); }
	void data_w(device_t *device, int state) { set_signal(device, DATA, state); }
	void reset_w(device_t *device, int state) { set_signal(device, RESET, state); }

protected:
	enum
	{
		SRQ = 0,
		ATN,
		CLK,
		DATA,
		RESET,
		SIGNAL_COUNT
	};

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);
		daisy_entry *next() const { return m_next; }

		daisy_entry *               m_next;         // next device
		device_t *                  m_device;       // associated device
		device_cbm_iec_interface *  m_interface;    // associated device's daisy interface

		int m_line[SIGNAL_COUNT];
	};

	simple_list<daisy_entry> m_device_list;

private:
	devcb_write_line   m_write_srq;
	devcb_write_line   m_write_atn;
	devcb_write_line   m_write_clk;
	devcb_write_line   m_write_data;
	devcb_write_line   m_write_reset;

	void set_signal(device_t *device, int signal, int state);
	int get_signal(int signal);

	int m_line[SIGNAL_COUNT];
};


// ======================> cbm_iec_slot_device

class cbm_iec_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	cbm_iec_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_address(int address) { m_address = address; }
	int get_address() { return m_address; }

	// device-level overrides
	virtual void device_start();

protected:
	int m_address;
};


// ======================> device_cbm_iec_interface

class device_cbm_iec_interface : public device_slot_card_interface
{
	friend class cbm_iec_device;

public:
	// construction/destruction
	device_cbm_iec_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cbm_iec_interface();

	device_cbm_iec_interface *next() const { return m_next; }
	device_cbm_iec_interface *m_next;

	// optional operation overrides
	virtual void cbm_iec_srq(int state) { };
	virtual void cbm_iec_atn(int state) { };
	virtual void cbm_iec_clk(int state) { };
	virtual void cbm_iec_data(int state) { };
	virtual void cbm_iec_reset(int state) { };

	cbm_iec_device *m_bus;
	cbm_iec_slot_device *m_slot;
};


// device type definition
extern const device_type CBM_IEC;
extern const device_type CBM_IEC_SLOT;


SLOT_INTERFACE_EXTERN( cbm_iec_devices );



#endif
