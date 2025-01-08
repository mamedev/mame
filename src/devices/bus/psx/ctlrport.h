// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_PSX_CTLRPORT_H
#define MAME_BUS_PSX_CTLRPORT_H

#pragma once


#include "memcard.h"

void psx_controllers(device_slot_interface &device);

DECLARE_DEVICE_TYPE(PSXCONTROLLERPORTS,      psxcontrollerports_device)
DECLARE_DEVICE_TYPE(PSX_CONTROLLER_PORT,     psx_controller_port_device)
DECLARE_DEVICE_TYPE(PSX_STANDARD_CONTROLLER, psx_standard_controller_device)

class psx_controller_port_device;

class device_psx_controller_interface : public device_interface
{
	friend class psx_multitap_device;
public:
	virtual ~device_psx_controller_interface();

	void clock_w(bool state) { if(!m_clock && !m_sel && state && !m_memcard) do_pad(); m_clock = state; }
	void sel_w(bool state);

	bool rx_r() { return m_rx; }
	bool ack_r() { return m_ack; }

protected:
	device_psx_controller_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
	virtual void interface_pre_reset() override;
	virtual void interface_post_stop() override;

	enum
	{
		QUERY_PAD_STATE = 0x42,
		CONFIG_MODE = 0x43
	};

private:
	virtual bool get_pad(int count, uint8_t *odata, uint8_t idata) = 0;
	virtual void do_pad();
	void ack_timer(int32_t param);

	uint8_t m_odata;
	uint8_t m_idata;
	int m_bit;
	int m_count;
	bool m_memcard;

	bool m_clock;
	bool m_sel;
	bool m_ack;
	bool m_rx;

	emu_timer *m_ack_timer;
	psx_controller_port_device *m_owner;
};

class psx_standard_controller_device :  public device_t,
										public device_psx_controller_interface
{
public:
	psx_standard_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override { }
private:
	virtual bool get_pad(int count, uint8_t *odata, uint8_t idata) override;

	required_ioport m_pad0;
	required_ioport m_pad1;
};


class psxcontrollerports_device : public device_t
{
public:
	psxcontrollerports_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ack();

	auto dsr() { return m_dsr_handler.bind(); }
	auto rxd() { return m_rxd_handler.bind(); }

	void write_sck(int state);
	void write_dtr(int state);
	void write_txd(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_device<psx_controller_port_device> m_port0;
	required_device<psx_controller_port_device> m_port1;

	devcb_write_line m_dsr_handler;
	devcb_write_line m_rxd_handler;
};

class psx_controller_port_device :  public device_t,
									public device_single_card_slot_interface<device_psx_controller_interface>
{
public:
	typedef device_delegate<void ()> void_cb;

	template <typename T>
	psx_controller_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: psx_controller_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	psx_controller_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T>
	void set_ack_cb(T &&... args) { m_ack_cb.set(std::forward<T>(args)...); }

	void ack() { m_ack_cb(); }

	void tx_w(int state) { m_tx = state; }
	void sel_w(int state) { if(m_dev) m_dev->sel_w(state); m_card->sel_w(state); }
	void clock_w(int state) { if(m_dev) m_dev->clock_w(state); m_card->clock_w(state); }

	int rx_r() { return (m_dev?m_dev->rx_r():true) && m_card->rx_r(); }
	int ack_r() { return (m_dev?m_dev->ack_r():true) && m_card->ack_r(); }
	int tx_r() { return m_tx; }

	void disable_card(bool status);

protected:
	virtual void device_start() override { m_ack_cb.resolve_safe(); }
	virtual void device_reset() override { m_tx = true; }
	virtual void device_config_complete() override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void_cb m_ack_cb;
	bool m_tx;

	device_psx_controller_interface *m_dev;
	required_device<psxcard_device> m_card;
};

#endif // MAME_BUS_PSX_CTLRPORT_H
