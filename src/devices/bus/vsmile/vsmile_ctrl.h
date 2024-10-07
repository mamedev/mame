// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_VSMILE_VSMILE_CTRL_H
#define MAME_BUS_VSMILE_VSMILE_CTRL_H

#pragma once


/***************************************************************************
 FORWARD DECLARATIONS
 ***************************************************************************/

class vsmile_ctrl_port_device;


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> device_vsmile_ctrl_interface

class device_vsmile_ctrl_interface : public device_interface
{
public:
	virtual ~device_vsmile_ctrl_interface();

protected:
	device_vsmile_ctrl_interface(machine_config const &mconfig, device_t &device);

	// device_interface implementation
	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void interface_pre_start() override;

	// otuput signals
	void rts_out(int state);
	void data_out(uint8_t data);

private:
	// input signal handlers for implementataions to override
	virtual void select_w(int state) = 0;
	virtual void data_w(uint8_t data) = 0;

	vsmile_ctrl_port_device *const m_port;

	friend class vsmile_ctrl_port_device;
};


// ======================> vsmile_ctrl_port_device

class vsmile_ctrl_port_device : public device_t, public device_single_card_slot_interface<device_vsmile_ctrl_interface>
{
public:
	// configuration
	auto rts_cb() { return m_rts_cb.bind(); }
	auto data_cb() { return m_data_cb.bind(); }

	// construction/destruction
	template <typename T>
	vsmile_ctrl_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: vsmile_ctrl_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vsmile_ctrl_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~vsmile_ctrl_port_device();

	// input signals
	void select_w(int state) { if (m_device) m_device->select_w(state); }
	void data_w(uint8_t data) { if (m_device) m_device->data_w(data); }

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	device_vsmile_ctrl_interface *m_device;
	devcb_write_line m_rts_cb;
	devcb_write8 m_data_cb;

	friend class device_vsmile_ctrl_interface;
};


// ======================> vsmile_ctrl_device_base

class vsmile_ctrl_device_base : public device_t, public device_vsmile_ctrl_interface
{
public:
	// destruction
	virtual ~vsmile_ctrl_device_base();

protected:
	// construction
	vsmile_ctrl_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// UART simulation helpers
	bool is_tx_empty() const { return m_tx_fifo_empty; }
	bool queue_tx(uint8_t data);
	bool is_selected() { return m_select; }

private:
	// device_vsmile_ctrl_interfaceA implementation
	virtual void select_w(int state) override;
	virtual void data_w(uint8_t data) override;

	// UART simulation handlers
	virtual void tx_complete() = 0;
	virtual void tx_timeout() = 0;
	virtual void rx_complete(uint8_t data, bool select) = 0;

	// internal helpers
	TIMER_CALLBACK_MEMBER(tx_timer_expired);
	TIMER_CALLBACK_MEMBER(rts_timer_expired);

	emu_timer *m_tx_timer, *m_rts_timer;

	uint8_t m_tx_fifo[32];
	uint8_t m_tx_fifo_head, m_tx_fifo_tail;
	bool m_tx_fifo_empty, m_tx_active;
	bool m_select;
};


/***************************************************************************
 INLINE FUNCTIONS
 ***************************************************************************/

inline void device_vsmile_ctrl_interface::rts_out(int state) { m_port->m_rts_cb(state); }
inline void device_vsmile_ctrl_interface::data_out(uint8_t data) { m_port->m_data_cb(data); }


/***************************************************************************
 FUNCTIONS
 ***************************************************************************/

void vsmile_controllers(device_slot_interface &device);


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_CTRL_PORT, vsmile_ctrl_port_device)

#endif // MAME_BUS_VSMILE_VSMILE_CTRL_H
