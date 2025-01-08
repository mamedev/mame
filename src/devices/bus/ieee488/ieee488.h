// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

**********************************************************************/

#ifndef MAME_BUS_IEEE488_IEEE488_H
#define MAME_BUS_IEEE488_IEEE488_H

#pragma once

void cbm_ieee488_devices(device_slot_interface &device);
void hp_ieee488_devices(device_slot_interface &device);
void remote488_devices(device_slot_interface &device);
void grid_ieee488_devices(device_slot_interface &device);

DECLARE_DEVICE_TYPE(IEEE488,      ieee488_device)
DECLARE_DEVICE_TYPE(IEEE488_SLOT, ieee488_slot_device)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define IEEE488_TAG         "ieee_bus"



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
	ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto eoi_callback() { return m_write_eoi.bind(); }
	auto dav_callback() { return m_write_dav.bind(); }
	auto nrfd_callback() { return m_write_nrfd.bind(); }
	auto ndac_callback() { return m_write_ndac.bind(); }
	auto ifc_callback() { return m_write_ifc.bind(); }
	auto srq_callback() { return m_write_srq.bind(); }
	auto atn_callback() { return m_write_atn.bind(); }
	auto ren_callback() { return m_write_ren.bind(); }

	// This CB reports changes to the DIO lines on the bus (whose value comes from
	// ANDing the DIO lines of each device on the bus)
	// This CB is needed by those controllers that start a parallel poll and wait
	// for some condition to be set by devices on the DIO lines (e.g. PHI controller).
	auto dio_callback() { return m_write_dio.bind(); }

	void add_device(ieee488_slot_device *slot, device_t *target);

	// reads for both host and peripherals
	uint8_t dio_r() { return get_data(); }
	int eoi_r() { return get_signal(EOI); }
	int dav_r() { return get_signal(DAV); }
	int nrfd_r() { return get_signal(NRFD); }
	int ndac_r() { return get_signal(NDAC); }
	int ifc_r() { return get_signal(IFC); }
	int srq_r() { return get_signal(SRQ); }
	int atn_r() { return get_signal(ATN); }
	int ren_r() { return get_signal(REN); }

	// writes for host (driver_device)
	void host_dio_w(uint8_t data) { set_data(this, data); }
	void host_eoi_w(int state) { set_signal(this, EOI, state); }
	void host_dav_w(int state) { set_signal(this, DAV, state); }
	void host_nrfd_w(int state) { set_signal(this, NRFD, state); }
	void host_ndac_w(int state) { set_signal(this, NDAC, state); }
	void host_ifc_w(int state) { set_signal(this, IFC, state); }
	void host_srq_w(int state) { set_signal(this, SRQ, state); }
	void host_atn_w(int state) { set_signal(this, ATN, state); }
	void host_ren_w(int state) { set_signal(this, REN, state); }

	// writes for peripherals (device_t)
	void dio_w(device_t *device, uint8_t data) { set_data(device, data); }
	void eoi_w(device_t *device, int state) { set_signal(device, EOI, state); }
	void dav_w(device_t *device, int state) { set_signal(device, DAV, state); }
	void nrfd_w(device_t *device, int state) { set_signal(device, NRFD, state); }
	void ndac_w(device_t *device, int state) { set_signal(device, NDAC, state); }
	void ifc_w(device_t *device, int state) { set_signal(device, IFC, state); }
	void srq_w(device_t *device, int state) { set_signal(device, SRQ, state); }
	void atn_w(device_t *device, int state) { set_signal(device, ATN, state); }
	void ren_w(device_t *device, int state) { set_signal(device, REN, state); }

	// helper functions
	static void add_cbm_devices(machine_config &config, const char *_default_drive)
	{
		IEEE488_SLOT(config, "ieee4", 4, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee8", 8, cbm_ieee488_devices, _default_drive);
		IEEE488_SLOT(config, "ieee9", 9, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee10", 10, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee11", 11, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee12", 12, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee13", 13, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee14", 14, cbm_ieee488_devices, nullptr);
		IEEE488_SLOT(config, "ieee15", 15, cbm_ieee488_devices, nullptr);
		IEEE488(config, IEEE488_TAG);
	}
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);
		daisy_entry *next() const { return m_next; }

		daisy_entry *               m_next;         // next device
		device_t *                  m_device;       // associated device
		device_ieee488_interface *  m_interface;    // associated device's daisy interface

		int m_line[SIGNAL_COUNT];
		uint8_t m_dio;
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
	devcb_write8       m_write_dio;

	void set_signal(device_t *device, int signal, int state);
	int get_signal(int signal);
	void set_data(device_t *device, uint8_t data);
	uint8_t get_data();

	int m_line[SIGNAL_COUNT];
	uint8_t m_dio;
};


// ======================> ieee488_slot_device

class ieee488_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	ieee488_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, int address, T &&opts, char const *dflt)
		: ieee488_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_address(address);
	}
	ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void add_cbm_slot(machine_config &config, const char *_tag, int _address, const char *_def_slot);
	static void add_cbm_defaults(machine_config &config, const char *_default_drive)
	{
		add_cbm_slot(config, "ieee4", 4, nullptr);
		add_cbm_slot(config, "ieee8", 8, _default_drive);
		add_cbm_slot(config, "ieee9", 9, nullptr);
		add_cbm_slot(config, "ieee10", 10, nullptr);
		add_cbm_slot(config, "ieee11", 11, nullptr);
		add_cbm_slot(config, "ieee12", 12, nullptr);
		add_cbm_slot(config, "ieee13", 13, nullptr);
		add_cbm_slot(config, "ieee14", 14, nullptr);
		add_cbm_slot(config, "ieee15", 15, nullptr);
	}

	void set_address(int address) { m_address = address; }
	int get_address() { return m_address; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

protected:
	int m_address;
};


// ======================> device_ieee488_interface

class device_ieee488_interface : public device_interface
{
	friend class ieee488_device;
	template <class ElementType> friend class simple_list;

public:
	// construction/destruction
	virtual ~device_ieee488_interface();

	device_ieee488_interface *next() const { return m_next; }

	// optional operation overrides
	virtual void ieee488_eoi(int state) { }
	virtual void ieee488_dav(int state) { }
	virtual void ieee488_nrfd(int state) { }
	virtual void ieee488_ndac(int state) { }
	virtual void ieee488_ifc(int state) { }
	virtual void ieee488_srq(int state) { }
	virtual void ieee488_atn(int state) { }
	virtual void ieee488_ren(int state) { }

protected:
	device_ieee488_interface(const machine_config &mconfig, device_t &device);

	ieee488_device *m_bus;
	ieee488_slot_device *m_slot;

private:
	device_ieee488_interface *m_next;
};

#endif // MAME_BUS_IEEE488_IEEE488_H
