/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __IEEE488__
#define __IEEE488__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define IEEE488_TAG         "ieee_bus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_IEEE488_BUS_ADD(_config) \
	MCFG_DEVICE_ADD(IEEE488_TAG, IEEE488, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define IEEE488_INTERFACE(_name) \
	const ieee488_interface (_name) =


#define MCFG_IEEE488_SLOT_ADD(_tag, _num, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, IEEE488_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	ieee488_slot_device::static_set_slot(*device, _num);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ieee488_interface

struct ieee488_interface
{
	devcb_write_line    m_out_eoi_cb;
	devcb_write_line    m_out_dav_cb;
	devcb_write_line    m_out_nrfd_cb;
	devcb_write_line    m_out_ndac_cb;
	devcb_write_line    m_out_ifc_cb;
	devcb_write_line    m_out_srq_cb;
	devcb_write_line    m_out_atn_cb;
	devcb_write_line    m_out_ren_cb;
};


// ======================> ieee488_device

class device_ieee488_interface;

class ieee488_device : public device_t,
						public ieee488_interface
{
public:
	// construction/destruction
	ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void add_device(device_t *target, int address);

	// reads for both host and peripherals
	UINT8 dio_r();
	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_READ_LINE_MEMBER( eoi_r );
	DECLARE_READ_LINE_MEMBER( dav_r );
	DECLARE_READ_LINE_MEMBER( nrfd_r );
	DECLARE_READ_LINE_MEMBER( ndac_r );
	DECLARE_READ_LINE_MEMBER( ifc_r );
	DECLARE_READ_LINE_MEMBER( srq_r );
	DECLARE_READ_LINE_MEMBER( atn_r );
	DECLARE_READ_LINE_MEMBER( ren_r );

	// writes for host (driver_device)
	void dio_w(UINT8 data);
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_WRITE_LINE_MEMBER( eoi_w );
	DECLARE_WRITE_LINE_MEMBER( dav_w );
	DECLARE_WRITE_LINE_MEMBER( nrfd_w );
	DECLARE_WRITE_LINE_MEMBER( ndac_w );
	DECLARE_WRITE_LINE_MEMBER( ifc_w );
	DECLARE_WRITE_LINE_MEMBER( srq_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( ren_w );

	// writes for peripherals (device_t)
	void dio_w(device_t *device, UINT8 data);
	void eoi_w(device_t *device, int state);
	void dav_w(device_t *device, int state);
	void nrfd_w(device_t *device, int state);
	void ndac_w(device_t *device, int state);
	void ifc_w(device_t *device, int state);
	void srq_w(device_t *device, int state);
	void atn_w(device_t *device, int state);
	void ren_w(device_t *device, int state);

protected:
	enum
	{
		EOI = 0,
		DAV,
		NRFD,
		NDAC,
		IFC,
		SRQ,
		ATN,
		REN,
		SIGNAL_COUNT
	};

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_stop();

	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);
		daisy_entry *next() const { return m_next; }

		daisy_entry *               m_next;         // next device
		device_t *                  m_device;       // associated device
		device_ieee488_interface *  m_interface;    // associated device's daisy interface

		int m_line[SIGNAL_COUNT];
		UINT8 m_dio;
	};

	simple_list<daisy_entry> m_device_list;

private:
	devcb_resolved_write_line   m_out_eoi_func;
	devcb_resolved_write_line   m_out_dav_func;
	devcb_resolved_write_line   m_out_nrfd_func;
	devcb_resolved_write_line   m_out_ndac_func;
	devcb_resolved_write_line   m_out_ifc_func;
	devcb_resolved_write_line   m_out_srq_func;
	devcb_resolved_write_line   m_out_atn_func;
	devcb_resolved_write_line   m_out_ren_func;

	inline void set_signal(device_t *device, int signal, int state);
	inline int get_signal(int signal);
	inline void set_data(device_t *device, UINT8 data);
	inline UINT8 get_data();

	int m_line[SIGNAL_COUNT];
	UINT8 m_dio;
};


// ======================> ieee488_slot_device

class ieee488_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_slot(device_t &device, int address);

private:
	// configuration
	int m_address;
};


// ======================> device_ieee488_interface

class device_ieee488_interface : public device_slot_card_interface
{
	friend class ieee488_device;

public:
	// construction/destruction
	device_ieee488_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_ieee488_interface();

	device_ieee488_interface *next() const { return m_next; }
	device_ieee488_interface *m_next;

	// optional operation overrides
	virtual void ieee488_eoi(int state) { };
	virtual void ieee488_dav(int state) { };
	virtual void ieee488_nrfd(int state) { };
	virtual void ieee488_ndac(int state) { };
	virtual void ieee488_ifc(int state) { };
	virtual void ieee488_srq(int state) { };
	virtual void ieee488_atn(int state) { };
	virtual void ieee488_ren(int state) { };

	ieee488_device  *m_bus;
	int m_address;
};


// device type definition
extern const device_type IEEE488;
extern const device_type IEEE488_SLOT;



#endif
