// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

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

#define MCFG_IEEE488_BUS_ADD() \
	MCFG_DEVICE_ADD(IEEE488_TAG, IEEE488, 0)


#define MCFG_IEEE488_EOI_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_eoi_callback(DEVCB_##_write);

#define MCFG_IEEE488_DAV_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_dav_callback(DEVCB_##_write);

#define MCFG_IEEE488_NRFD_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_nrfd_callback(DEVCB_##_write);

#define MCFG_IEEE488_NDAC_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_ndac_callback(DEVCB_##_write);

#define MCFG_IEEE488_IFC_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_ifc_callback(DEVCB_##_write);

#define MCFG_IEEE488_SRQ_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_srq_callback(DEVCB_##_write);

#define MCFG_IEEE488_ATN_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_atn_callback(DEVCB_##_write);

#define MCFG_IEEE488_REN_CALLBACK(_write) \
	downcast<ieee488_device *>(device)->set_ren_callback(DEVCB_##_write);


#define MCFG_IEEE488_SLOT_ADD(_tag, _address, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, IEEE488_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<ieee488_slot_device *>(device)->set_address(_address);


#define MCFG_CBM_IEEE488_ADD(_default_drive) \
	MCFG_IEEE488_SLOT_ADD("ieee4", 4, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee8", 8, cbm_ieee488_devices, _default_drive) \
	MCFG_IEEE488_SLOT_ADD("ieee9", 9, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee10", 10, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee11", 11, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee12", 12, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee13", 13, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee14", 14, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee15", 15, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_BUS_ADD()



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ieee488_device

class ieee488_slot_device;
class device_ieee488_interface;

class ieee488_device : public device_t
{
public:
	// construction/destruction
	ieee488_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _write> void set_eoi_callback(_write wr) { m_write_eoi.set_callback(wr); }
	template<class _write> void set_dav_callback(_write wr) { m_write_dav.set_callback(wr); }
	template<class _write> void set_nrfd_callback(_write wr) { m_write_nrfd.set_callback(wr); }
	template<class _write> void set_ndac_callback(_write wr) { m_write_ndac.set_callback(wr); }
	template<class _write> void set_ifc_callback(_write wr) { m_write_ifc.set_callback(wr); }
	template<class _write> void set_srq_callback(_write wr) { m_write_srq.set_callback(wr); }
	template<class _write> void set_atn_callback(_write wr) { m_write_atn.set_callback(wr); }
	template<class _write> void set_ren_callback(_write wr) { m_write_ren.set_callback(wr); }

	void add_device(ieee488_slot_device *slot, device_t *target);

	// reads for both host and peripherals
	UINT8 dio_r() { return get_data(); }
	DECLARE_READ8_MEMBER( dio_r ) { return get_data(); }
	DECLARE_READ_LINE_MEMBER( eoi_r ) { return get_signal(EOI); }
	DECLARE_READ_LINE_MEMBER( dav_r ) { return get_signal(DAV); }
	DECLARE_READ_LINE_MEMBER( nrfd_r ) { return get_signal(NRFD); }
	DECLARE_READ_LINE_MEMBER( ndac_r ) { return get_signal(NDAC); }
	DECLARE_READ_LINE_MEMBER( ifc_r ) { return get_signal(IFC); }
	DECLARE_READ_LINE_MEMBER( srq_r ) { return get_signal(SRQ); }
	DECLARE_READ_LINE_MEMBER( atn_r ) { return get_signal(ATN); }
	DECLARE_READ_LINE_MEMBER( ren_r ) { return get_signal(REN); }

	// writes for host (driver_device)
	void dio_w(UINT8 data) { return set_data(this, data); }
	DECLARE_WRITE8_MEMBER( dio_w ) { set_data(this, data); }
	DECLARE_WRITE_LINE_MEMBER( eoi_w ) { set_signal(this, EOI, state); }
	DECLARE_WRITE_LINE_MEMBER( dav_w ) { set_signal(this, DAV, state); }
	DECLARE_WRITE_LINE_MEMBER( nrfd_w ) { set_signal(this, NRFD, state); }
	DECLARE_WRITE_LINE_MEMBER( ndac_w ) { set_signal(this, NDAC, state); }
	DECLARE_WRITE_LINE_MEMBER( ifc_w ) { set_signal(this, IFC, state); }
	DECLARE_WRITE_LINE_MEMBER( srq_w ) { set_signal(this, SRQ, state); }
	DECLARE_WRITE_LINE_MEMBER( atn_w ) { set_signal(this, ATN, state); }
	DECLARE_WRITE_LINE_MEMBER( ren_w ) { set_signal(this, REN, state); }

	// writes for peripherals (device_t)
	void dio_w(device_t *device, UINT8 data) { set_data(device, data); }
	void eoi_w(device_t *device, int state) { set_signal(device, EOI, state); }
	void dav_w(device_t *device, int state) { set_signal(device, DAV, state); }
	void nrfd_w(device_t *device, int state) { set_signal(device, NRFD, state); }
	void ndac_w(device_t *device, int state) { set_signal(device, NDAC, state); }
	void ifc_w(device_t *device, int state) { set_signal(device, IFC, state); }
	void srq_w(device_t *device, int state) { set_signal(device, SRQ, state); }
	void atn_w(device_t *device, int state) { set_signal(device, ATN, state); }
	void ren_w(device_t *device, int state) { set_signal(device, REN, state); }

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
	virtual void device_start() override;
	virtual void device_stop() override;

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
	devcb_write_line   m_write_eoi;
	devcb_write_line   m_write_dav;
	devcb_write_line   m_write_nrfd;
	devcb_write_line   m_write_ndac;
	devcb_write_line   m_write_ifc;
	devcb_write_line   m_write_srq;
	devcb_write_line   m_write_atn;
	devcb_write_line   m_write_ren;

	void set_signal(device_t *device, int signal, int state);
	int get_signal(int signal);
	void set_data(device_t *device, UINT8 data);
	UINT8 get_data();

	int m_line[SIGNAL_COUNT];
	UINT8 m_dio;
};


// ======================> ieee488_slot_device

class ieee488_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	ieee488_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void set_address(int address) { m_address = address; }
	int get_address() { return m_address; }

	// device-level overrides
	virtual void device_start() override;

protected:
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

	ieee488_device *m_bus;
	ieee488_slot_device *m_slot;
};


// device type definition
extern const device_type IEEE488;
extern const device_type IEEE488_SLOT;


SLOT_INTERFACE_EXTERN( cbm_ieee488_devices );



#endif
