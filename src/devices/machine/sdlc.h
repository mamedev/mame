// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_MACHINE_SDLC_H
#define MAME_MACHINE_SDLC_H

#pragma once

#include <cstdint>
#include <memory>
#include <utility>


class device_sdlc_consumer_interface : public device_interface
{
public:
	static constexpr std::uint16_t POLY_SDLC = 0x1021U;

	template <typename T>
	static constexpr u16 update_frame_check(u16 poly, u16 current, T bit)
	{
		return (current << 1) ^ ((bool(bit) != bool(BIT(current, 15))) ? poly : 0U);
	}

protected:
	device_sdlc_consumer_interface(machine_config const &mconfig, device_t &device);
	virtual ~device_sdlc_consumer_interface();

	virtual void interface_post_start() override;

	void rx_bit(bool state);
	void rx_reset();

	bool is_line_active() const { return bool(m_line_active); }
	uint16_t get_frame_check() const { return m_frame_check; }
	bool is_frame_check_good() const { return 0x1d0fU == m_frame_check; }

private:
	template <typename... Params> void logerror(Params &&... args) { device().logerror(std::forward<Params>(args)...); }

	virtual void frame_start() { }
	virtual void frame_end() { }
	virtual void frame_abort() { }
	virtual void line_active() { }
	virtual void line_inactive() { }
	virtual void data_bit(bool value) { }

	std::uint8_t    m_line_active;
	std::uint8_t    m_discard_bits;
	std::uint8_t    m_in_frame;
	std::uint16_t   m_shift_register;
	std::uint16_t   m_frame_check;
};


class sdlc_logger_device : public device_t, public device_sdlc_consumer_interface
{
public:
	sdlc_logger_device(machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock);
	virtual ~sdlc_logger_device();

	// input signals
	void data_w(int state) { m_current_data = state ? 1U : 0U; }
	void clock_w(int state);

	// input format configuration
	void data_nrzi(int state) { m_data_nrzi = state ? 1U : 0U; }
	void clock_active(int state) { m_clock_active = state ? 1U : 0U; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	using device_t::logerror;

private:
	enum : std::size_t
	{
		BUFFER_BYTES = 1024U,
		BUFFER_BITS = BUFFER_BYTES << 3
	};

	virtual void frame_start() override;
	virtual void frame_end() override;
	virtual void frame_abort() override;
	virtual void data_bit(bool value) override;

	void shift_residual_bits();
	void log_frame(bool partial) const;

	std::uint8_t    m_data_nrzi;
	std::uint8_t    m_clock_active;

	std::uint8_t    m_current_data;
	std::uint8_t    m_last_data;
	std::uint8_t    m_current_clock;

	std::uint32_t                       m_frame_bits;
	std::uint16_t                       m_expected_fcs;
	std::unique_ptr<std::uint8_t []>    m_buffer;
};


DECLARE_DEVICE_TYPE(SDLC_LOGGER, sdlc_logger_device)

#endif // MAME_MACHINE_SDLC_H
