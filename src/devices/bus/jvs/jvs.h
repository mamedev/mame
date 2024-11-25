// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_BUS_JVS_JVS_H
#define MAME_BUS_JVS_JVS_H

#pragma once

class device_jvs_interface;

class jvs_port_device :
	public device_t,
	public device_single_card_slot_interface<device_jvs_interface>
{
	friend class device_jvs_interface;

public:
	template <typename T>
	jvs_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: jvs_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	jvs_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~jvs_port_device();

	struct sense { enum : uint8_t
	{
		Initialized = 0, // 0V
		Uninitialized = 1, // 2.5V
		None = 2 // 5v
	}; };

	auto rxd() { return m_rxd_cb.bind(); }
	auto sense() { return m_sense_cb.bind(); }

	void txd(int state);
	void rts(int state);

protected:
	// device_t
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	device_jvs_interface *m_device;

private:
	void debug_rxd(int state);
	void debug_flush(wchar_t ch = 0);
	void debug_stop();
	TIMER_CALLBACK_MEMBER(debug_timer_callback);

	emu_timer *m_debug_timer;
	int8_t m_debug_bit;
	uint16_t m_debug_index;
	int8_t m_debug_rxd;
	uint8_t m_debug_buffer[0x102];

	void update_rts(int32_t state);
	void update_rxd();

	devcb_write_line m_rxd_cb;
	devcb_write8 m_sense_cb;
	int8_t m_rts;
	int8_t m_rxd;
	int8_t m_txd;
};

class device_jvs_interface :
	public device_interface
{
	friend class jvs_port_device;

public:
	virtual ~device_jvs_interface();

	auto system() { m_default_inputs = false; return m_system_cb.bind(); }
	template<unsigned N> auto player() { m_default_inputs = false; return m_player_cb[N].bind(); }
	template<unsigned N> auto coin() { m_default_inputs = false; return m_coin_cb[N].bind(); }
	template<unsigned N> auto analog_input() { m_default_inputs = false; return m_analog_input_cb[N].bind(); }
	template<unsigned N> auto rotary_input() { m_default_inputs = false; return m_rotary_input_cb[N].bind(); }
	template<unsigned N> auto screen_position_enable() { m_default_inputs = false; return m_screen_position_enable_cb[N].bind(); }
	template<unsigned N> auto screen_position_x() { m_default_inputs = false; return m_screen_position_x_cb[N].bind(); }
	template<unsigned N> auto screen_position_y() { m_default_inputs = false; return m_screen_position_y_cb[N].bind(); }
	auto output() { return m_output_cb.bind(); }
	template<unsigned N> auto analog_output() { return m_analog_output_cb[N].bind(); }

	auto jvs() const { if (m_jvs.target()) return m_jvs.target(); return device().subdevice<jvs_port_device>(m_jvs.finder_tag()); }

protected:
	device_jvs_interface(const machine_config &mconfig, device_t &device);

	// device_interface
	virtual void interface_post_start() override;

	virtual void rxd(int state) = 0;

	void rts(int state);
	void txd(int state);
	void sense(uint8_t state);
	virtual void jvs_sense(uint8_t state);

	void add_jvs_port(machine_config &config);

	uint8_t system_r(uint8_t mem_mask);
	uint32_t player_r(offs_t offset, uint32_t mem_mask);
	uint8_t coin_r(offs_t offset, uint8_t mem_mask);
	uint16_t analog_input_r(offs_t offset, uint16_t mem_mask);
	uint16_t rotary_input_r(offs_t offset);
	uint16_t screen_position_enable_r(offs_t offset);
	uint16_t screen_position_x_r(offs_t offset);
	uint16_t screen_position_y_r(offs_t offset);

	optional_device<jvs_port_device> m_jvs;
	uint8_t m_jvs_sense;

	bool m_default_inputs;
	devcb_write64 m_output_cb;
	devcb_write16::array<8> m_analog_output_cb;

private:
	optional_ioport m_system;
	devcb_read8 m_system_cb;
	optional_ioport_array<8> m_player;
	devcb_read32::array<8> m_player_cb;
	optional_ioport_array<8> m_coin;
	devcb_read8::array<8> m_coin_cb;
	optional_ioport_array<8> m_analog_input;
	devcb_read16::array<8> m_analog_input_cb;
	optional_ioport_array<8> m_rotary_input;
	devcb_read16::array<8> m_rotary_input_cb;
	optional_ioport_array<8> m_screen_position_enable;
	devcb_read16::array<8> m_screen_position_enable_cb;
	optional_ioport_array<8> m_screen_position_x;
	devcb_read16::array<8> m_screen_position_x_cb;
	optional_ioport_array<8> m_screen_position_y;
	devcb_read16::array<8> m_screen_position_y_cb;

	jvs_port_device *m_port;
	jvs_port_device *m_root;
	void update_rts(int32_t state);
	void update_sense(int32_t state);

	int8_t m_rts;
	uint8_t m_sense;
	int8_t m_txd;
};

DECLARE_DEVICE_TYPE(JVS_PORT, jvs_port_device)

void jvs_port_devices(device_slot_interface &device);

#endif // MAME_BUS_JVS_JVS_H
