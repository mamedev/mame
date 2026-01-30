// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_QIC02_QIC02_H
#define MAME_BUS_QIC02_QIC02_H

#pragma once

class device_qic02_interface;

class qic02_connector_device
	: public device_t
	, public device_single_card_slot_interface<device_qic02_interface>
{
public:
	template <typename T>
	qic02_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: qic02_connector_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	qic02_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	virtual ~qic02_connector_device();

	// connector to host
	auto ack() { return m_ack.bind(); }
	auto rdy() { return m_rdy.bind(); }
	auto exc() { return m_exc.bind(); }
	auto dir() { return m_dir.bind(); }

	// host to connector
	void onl_w(int state);
	void req_w(int state);
	void rst_w(int state);
	void xfr_w(int state);

	// controller to connector
	void ack_w(int state);
	void rdy_w(int state);
	void exc_w(int state);
	void dir_w(int state);

	// bidirectional data
	u8 data_r();
	void data_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_ack;
	devcb_write_line m_rdy;
	devcb_write_line m_exc;
	devcb_write_line m_dir;
};

class device_qic02_interface
	: public device_interface
{
public:
	virtual void onl_w(int state) = 0;
	virtual void req_w(int state) = 0;
	virtual void rst_w(int state) = 0;
	virtual void xfr_w(int state) = 0;

	virtual u8 data_r() = 0;
	virtual void data_w(u8 data) = 0;

protected:
	device_qic02_interface(machine_config const &mconfig, device_t &device);
	virtual ~device_qic02_interface() override;

	virtual void ack_w(int state) = 0;
	virtual void rdy_w(int state) = 0;
	virtual void exc_w(int state) = 0;
	virtual void dir_w(int state) = 0;

	qic02_connector_device &qic() const { return *m_qic; }

private:
	qic02_connector_device *m_qic;
};

DECLARE_DEVICE_TYPE(QIC02_CONNECTOR,  qic02_connector_device)

#endif // MAME_BUS_QIC02_QIC02_H
