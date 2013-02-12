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

#define MCFG_CBM_IEC_BUS_ADD() \
	MCFG_DEVICE_ADD(CBM_IEC_TAG, CBM_IEC, 0)


#define MCFG_CBM_IEC_BUS_SRQ_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_srq_callback(DEVCB2_##_write);

#define MCFG_CBM_IEC_BUS_ATN_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_atn_callback(DEVCB2_##_write);

#define MCFG_CBM_IEC_BUS_CLK_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_clk_callback(DEVCB2_##_write);

#define MCFG_CBM_IEC_BUS_DATA_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_data_callback(DEVCB2_##_write);

#define MCFG_CBM_IEC_BUS_RESET_CALLBACK(_write) \
	downcast<cbm_iec_device *>(device)->set_reset_callback(DEVCB2_##_write);


#define MCFG_CBM_IEC_SLOT_ADD(_tag, _num, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, CBM_IEC_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	cbm_iec_slot_device::static_set_slot(*device, _num);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm_iec_device

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
	devcb2_write_line   m_write_srq;
	devcb2_write_line   m_write_atn;
	devcb2_write_line   m_write_clk;
	devcb2_write_line   m_write_data;
	devcb2_write_line   m_write_reset;

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
	virtual void cbm_iec_srq(int state) { };
	virtual void cbm_iec_atn(int state) { };
	virtual void cbm_iec_clk(int state) { };
	virtual void cbm_iec_data(int state) { };
	virtual void cbm_iec_reset(int state) { };

	cbm_iec_device  *m_bus;
	int m_address;
};


// device type definition
extern const device_type CBM_IEC;
extern const device_type CBM_IEC_SLOT;



#endif
