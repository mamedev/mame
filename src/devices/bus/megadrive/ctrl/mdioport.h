// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Sega Mega Drive I/O Port

    Seven I/O lines with individually selectable direction, UART,
    and interrupt output.

    Mega Drive has three instances of this block connected to the
    CTRL1, CTRL2 and EXP connectors.

    Game Gear has one instance of this block connected to the EXT
    connector.

***********************************************************************/
#ifndef MAME_BUS_MEGADRIVE_CTRL_MDIOPORT_H
#define MAME_BUS_MEGADRIVE_CTRL_MDIOPORT_H

#pragma once

#include "diserial.h"

#include <utility>


class megadrive_io_port_device_base : public device_t, public device_serial_interface
{
public:
	using parallel_in_delegate = device_delegate<u8 ()>;
	using parallel_out_delegate = device_delegate<void (u8 data, u8 mem_mask)>;

	// external signal handler configuration
	template <typename... T>
	void set_in_handler(T &&... args)
	{
		m_in_callback.set(std::forward<T>(args)...);
	}
	template <typename... T>
	void set_out_handler(T &&... args)
	{
		m_out_callback.set(std::forward<T>(args)...);
	}

	// host output configuration
	auto hl_handler() { return m_hl_callback.bind(); }

	// host read registers
	u8 data_r();
	u8 s_ctrl_r() { return m_s_ctrl; }
	u8 txdata_r() { return m_txdata; }

	// host write registers
	void txdata_w(u8 data);

protected:
	enum : unsigned
	{
		DATA_UP_BIT = 0,
		DATA_DOWN_BIT,
		DATA_LEFT_BIT,
		DATA_RIGHT_BIT,
		DATA_TL_BIT,
		DATA_TR_BIT,
		DATA_TH_BIT,

		DATA_TXD_BIT = DATA_TL_BIT,
		DATA_RXD_BIT = DATA_TR_BIT
	};

	enum : u8
	{
		DATA_TH_MASK = 1U << DATA_TH_BIT,
		DATA_TR_MASK = 1U << DATA_TR_BIT,
		DATA_TL_MASK = 1U << DATA_TL_BIT,
		DATA_RIGHT_MASK = 1U << DATA_RIGHT_BIT,
		DATA_LEFT_MASK = 1U << DATA_LEFT_BIT,
		DATA_DOWN_MASK = 1U << DATA_DOWN_BIT,
		DATA_UP_MASK = 1U << DATA_UP_BIT,

		DATA_RXD_MASK = DATA_TR_MASK,
		DATA_TXD_MASK = DATA_TL_MASK
	};

	enum : u8
	{
		S_CTRL_SIN_MASK = 0x20,
		S_CTRL_SOUT_MASK = 0x10,
		S_CTRL_RINT_MASK = 0x08,
		S_CTRL_RERR_MASK = 0x04,
		S_CTRL_RRDY_MASK = 0x02,
		S_CTRL_TFUL_MASK = 0x01
	};

	megadrive_io_port_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock) ATTR_COLD;

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;

	bool ctrl_int() const { return BIT(m_ctrl, 7); }

	bool s_ctrl_sin() const { return m_s_ctrl & S_CTRL_SIN_MASK; }
	bool s_ctrl_sout() const { return m_s_ctrl & S_CTRL_SOUT_MASK; }
	bool s_ctrl_rint() const { return m_s_ctrl & S_CTRL_RINT_MASK; }
	bool s_ctrl_rerr() const { return m_s_ctrl & S_CTRL_RERR_MASK; }
	bool s_ctrl_rrdy() const { return m_s_ctrl & S_CTRL_RRDY_MASK; }
	bool s_ctrl_tful() const { return m_s_ctrl & S_CTRL_TFUL_MASK; }

	bool rrdy_int() const;
	u8 out_drive() const;
	void update_out();
	void set_data(u8 data);
	void set_ctrl(u8 data);
	bool set_s_ctrl(u8 data);
	bool data_received();

	parallel_in_delegate m_in_callback;
	parallel_out_delegate m_out_callback;
	devcb_write_line m_hl_callback;

	u8 m_th_in;
	u8 m_txd;
	u8 m_data;
	u8 m_ctrl;
	u8 m_s_ctrl;
	u8 m_txdata;
	u8 m_rxdata;
};


class megadrive_io_port_device : public megadrive_io_port_device_base
{
public:
	megadrive_io_port_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) ATTR_COLD;

	// external signal inputs
	void th_w(int state);

	// host read registers
	u8 ctrl_r() { return m_ctrl; }
	u8 rxdata_r();

	// host write registers
	void data_w(u8 data);
	void ctrl_w(u8 data);
	void s_ctrl_w(u8 data);

protected:
	// device_serial_interface implementation
	virtual void rcv_complete() override;

private:
	bool th_int() const;
};


class gamegear_io_port_device : public megadrive_io_port_device_base
{
public:
	gamegear_io_port_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) ATTR_COLD;

	// external signal inputs
	void th_w(int state);

	// host read registers
	u8 ctrl_r() { return ~m_ctrl; }
	u8 rxdata_r();

	// host write registers
	void data_w(u8 data);
	void ctrl_w(u8 data);
	void s_ctrl_w(u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void rcv_complete() override;

private:
	u8 m_pc6int;
};


DECLARE_DEVICE_TYPE(MEGADRIVE_IO_PORT, megadrive_io_port_device)
DECLARE_DEVICE_TYPE(GAMEGEAR_IO_PORT, gamegear_io_port_device)

#endif // MAME_BUS_MEGADRIVE_CTRL_MDIOPORT_H
