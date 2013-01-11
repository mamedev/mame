/**********************************************************************

    Commodore IEC Serial Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_CBM_IEC_BUS_ADD(_config) \
	MCFG_DEVICE_ADD(CBM_IEC_TAG, CBM_IEC, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define CBM_IEC_INTERFACE(_name) \
	const cbm_iec_interface (_name) =


#define MCFG_CBM_IEC_SLOT_ADD(_tag, _num, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, CBM_IEC_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	cbm_iec_slot_device::static_set_slot(*device, _num);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm_iec_interface

struct cbm_iec_interface
{
	devcb_write_line    m_out_srq_cb;
	devcb_write_line    m_out_atn_cb;
	devcb_write_line    m_out_clk_cb;
	devcb_write_line    m_out_data_cb;
	devcb_write_line    m_out_reset_cb;
};


// ======================> cbm_iec_device

class device_cbm_iec_interface;

class cbm_iec_device : public device_t,
						public cbm_iec_interface
{
public:
	// construction/destruction
	cbm_iec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void add_device(device_t *target, int address);

	// reads for both host and peripherals
	DECLARE_READ_LINE_MEMBER( srq_r );
	DECLARE_READ_LINE_MEMBER( atn_r );
	DECLARE_READ_LINE_MEMBER( clk_r );
	DECLARE_READ_LINE_MEMBER( data_r );
	DECLARE_READ_LINE_MEMBER( reset_r );

	// writes for host (driver_device)
	DECLARE_WRITE_LINE_MEMBER( srq_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( data_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

	// writes for peripherals (device_t)
	void srq_w(device_t *device, int state);
	void atn_w(device_t *device, int state);
	void clk_w(device_t *device, int state);
	void data_w(device_t *device, int state);
	void reset_w(device_t *device, int state);

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
	virtual void device_config_complete();
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
	devcb_resolved_write_line   m_out_atn_func;
	devcb_resolved_write_line   m_out_clk_func;
	devcb_resolved_write_line   m_out_data_func;
	devcb_resolved_write_line   m_out_srq_func;
	devcb_resolved_write_line   m_out_reset_func;

	inline void set_signal(device_t *device, int signal, int state);
	inline int get_signal(int signal);

	int m_line[SIGNAL_COUNT];
};


// ======================> cbm_iec_slot_device

class cbm_iec_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	cbm_iec_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_slot(device_t &device, int address);

private:
	// configuration
	int m_address;
	cbm_iec_device  *m_bus;
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
	virtual void cbm_iec_atn(int state) { };
	virtual void cbm_iec_clk(int state) { };
	virtual void cbm_iec_data(int state) { };
	virtual void cbm_iec_srq(int state) { };
	virtual void cbm_iec_reset(int state) { };

	cbm_iec_device  *m_bus;
	int m_address;
};


// device type definition
extern const device_type CBM_IEC;
extern const device_type CBM_IEC_SLOT;



#endif
