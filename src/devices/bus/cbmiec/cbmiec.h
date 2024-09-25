// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEC Serial Bus emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_CBMIEC_H
#define MAME_BUS_CBMIEC_CBMIEC_H

#pragma once




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CBM_IEC_TAG         "iec_bus"


DECLARE_DEVICE_TYPE(CBM_IEC,      cbm_iec_device)
DECLARE_DEVICE_TYPE(CBM_IEC_SLOT, cbm_iec_slot_device)

void cbm_iec_devices(device_slot_interface &device);

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
	cbm_iec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto srq_callback() { return m_write_srq.bind(); }
	auto atn_callback() { return m_write_atn.bind(); }
	auto clk_callback() { return m_write_clk.bind(); }
	auto data_callback() { return m_write_data.bind(); }
	auto reset_callback() { return m_write_reset.bind(); }

	void add_device(cbm_iec_slot_device *slot, device_t *target);

	// reads for both host and peripherals
	int srq_r() { return get_signal(SRQ); }
	int atn_r() { return get_signal(ATN); }
	int clk_r() { return get_signal(CLK); }
	int data_r() { return get_signal(DATA); }
	int reset_r() { return get_signal(RESET); }

	// writes for host (driver_device)
	void host_srq_w(int state) { set_signal(this, SRQ, state); }
	void host_atn_w(int state) { set_signal(this, ATN, state); }
	void host_clk_w(int state) { set_signal(this, CLK, state); }
	void host_data_w(int state) { set_signal(this, DATA, state); }
	void host_reset_w(int state) { set_signal(this, RESET, state); }

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

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
	template <typename T>
	cbm_iec_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, int address, T &&opts, char const *dflt)
		: cbm_iec_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_address(address);
	}
	cbm_iec_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> static void add(machine_config &config, T &&_bus_tag, const char *_default_drive)
	{
		CBM_IEC_SLOT(config, "iec4", 4, cbm_iec_devices, nullptr);
		CBM_IEC_SLOT(config, "iec8", 8, cbm_iec_devices, _default_drive);
		CBM_IEC_SLOT(config, "iec9", 9, cbm_iec_devices, nullptr);
		CBM_IEC_SLOT(config, "iec10", 10, cbm_iec_devices, nullptr);
		CBM_IEC_SLOT(config, "iec11", 11, cbm_iec_devices, nullptr);

		CBM_IEC(config, std::forward<T>(_bus_tag), 0);
	}

	void set_address(int address) { m_address = address; }
	int get_address() { return m_address; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

protected:
	int m_address;
};


// ======================> device_cbm_iec_interface

class device_cbm_iec_interface : public device_interface
{
	friend class cbm_iec_device;

public:
	// construction/destruction
	virtual ~device_cbm_iec_interface();

	device_cbm_iec_interface *next() const { return m_next; }
	device_cbm_iec_interface *m_next;

	// optional operation overrides
	virtual void cbm_iec_srq(int state) { }
	virtual void cbm_iec_atn(int state) { }
	virtual void cbm_iec_clk(int state) { }
	virtual void cbm_iec_data(int state) { }
	virtual void cbm_iec_reset(int state) { }

protected:
	device_cbm_iec_interface(const machine_config &mconfig, device_t &device);

	cbm_iec_device *m_bus;
	cbm_iec_slot_device *m_slot;
};

#endif // MAME_BUS_CBMIEC_CBMIEC_H
