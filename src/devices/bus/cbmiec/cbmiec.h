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
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, nullptr) \
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, cbm_iec_devices, _default_drive) \
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, nullptr) \
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, nullptr) \
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, nullptr) \
	MCFG_CBM_IEC_BUS_ADD()


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

	template <class Object> devcb_base &set_srq_callback(Object &&wr) { return m_write_srq.set_callback(std::forward<Object>(wr)); }
	template <class Object> devcb_base &set_atn_callback(Object &&wr) { return m_write_atn.set_callback(std::forward<Object>(wr)); }
	template <class Object> devcb_base &set_clk_callback(Object &&wr) { return m_write_clk.set_callback(std::forward<Object>(wr)); }
	template <class Object> devcb_base &set_data_callback(Object &&wr) { return m_write_data.set_callback(std::forward<Object>(wr)); }
	template <class Object> devcb_base &set_reset_callback(Object &&wr) { return m_write_reset.set_callback(std::forward<Object>(wr)); }
	auto srq_callback() { return m_write_srq.bind(); }
	auto atn_callback() { return m_write_atn.bind(); }
	auto clk_callback() { return m_write_clk.bind(); }
	auto data_callback() { return m_write_data.bind(); }
	auto reset_callback() { return m_write_reset.bind(); }

	void add_device(cbm_iec_slot_device *slot, device_t *target);

	// reads for both host and peripherals
	DECLARE_READ_LINE_MEMBER( srq_r ) { return get_signal(SRQ); }
	DECLARE_READ_LINE_MEMBER( atn_r ) { return get_signal(ATN); }
	DECLARE_READ_LINE_MEMBER( clk_r ) { return get_signal(CLK); }
	DECLARE_READ_LINE_MEMBER( data_r ) { return get_signal(DATA); }
	DECLARE_READ_LINE_MEMBER( reset_r ) { return get_signal(RESET); }

	// writes for host (driver_device)
	DECLARE_WRITE_LINE_MEMBER( host_srq_w ) { set_signal(this, SRQ, state); }
	DECLARE_WRITE_LINE_MEMBER( host_atn_w ) { set_signal(this, ATN, state); }
	DECLARE_WRITE_LINE_MEMBER( host_clk_w ) { set_signal(this, CLK, state); }
	DECLARE_WRITE_LINE_MEMBER( host_data_w ) { set_signal(this, DATA, state); }
	DECLARE_WRITE_LINE_MEMBER( host_reset_w ) { set_signal(this, RESET, state); }

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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

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

DECLARE_DEVICE_TYPE(CBM_IEC,      cbm_iec_device)


// ======================> cbm_iec_slot_device

class cbm_iec_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	cbm_iec_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: cbm_iec_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	cbm_iec_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void add_slot(machine_config &config, const char *_tag, int _address, const char *_def_slot);
	template <typename T> static void add(machine_config &config, T &&_bus_tag, const char *_default_drive)
	{
		add_slot(config, "iec4", 4, nullptr);
		add_slot(config, "iec8", 8, _default_drive);
		add_slot(config, "iec9", 9, nullptr);
		add_slot(config, "iec10", 10, nullptr);
		add_slot(config, "iec11", 11, nullptr);

		CBM_IEC(config, std::forward<T>(_bus_tag), 0);
	}

	void set_address(int address) { m_address = address; }
	int get_address() { return m_address; }

	// device-level overrides
	virtual void device_start() override;

protected:
	int m_address;
};


// ======================> device_cbm_iec_interface

class device_cbm_iec_interface : public device_slot_card_interface
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

DECLARE_DEVICE_TYPE(CBM_IEC_SLOT, cbm_iec_slot_device)

#endif // MAME_BUS_CBMIEC_CBMIEC_H
