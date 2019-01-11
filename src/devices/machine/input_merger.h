// license:BSD-3-Clause
// copyright-holders:Dirk Best, Vas Crabb
/***************************************************************************

    Input Merger

    Used to connect multiple lines to a single device input while
    keeping it pulled high or low

***************************************************************************/

#ifndef MAME_MACHINE_INPUT_MERGER_H
#define MAME_MACHINE_INPUT_MERGER_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INPUT_MERGER_ANY_HIGH(_tag) \
	MCFG_DEVICE_ADD(_tag, INPUT_MERGER_ANY_HIGH, 0)

#define MCFG_INPUT_MERGER_ALL_HIGH(_tag) \
	MCFG_DEVICE_ADD(_tag, INPUT_MERGER_ALL_HIGH, 0)

#define MCFG_INPUT_MERGER_ANY_LOW(_tag) \
	MCFG_DEVICE_ADD(_tag, INPUT_MERGER_ANY_LOW, 0)

#define MCFG_INPUT_MERGER_ALL_LOW(_tag) \
	MCFG_DEVICE_ADD(_tag, INPUT_MERGER_ALL_LOW, 0)

#define MCFG_INPUT_MERGER_OUTPUT_HANDLER(_devcb) \
	devcb = &downcast<input_merger_device &>(*device).set_output_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class input_merger_device : public device_t
{
public:
	// callback
	template <class Object> devcb_base &set_output_handler(Object &&cb) { return m_output_handler.set_callback(std::forward<Object>(cb)); }

	// input lines
	template <unsigned Bit> DECLARE_WRITE_LINE_MEMBER(in_w) { static_assert(Bit < 32, "invalid bit"); machine().scheduler().synchronize(timer_expired_delegate(FUNC(input_merger_device::update_state), this), (Bit << 1) | (state ? 1U : 0U)); }
	template <unsigned Bit> DECLARE_WRITE8_MEMBER(in_set) { in_w<Bit>(1); }
	template <unsigned Bit> DECLARE_WRITE8_MEMBER(in_clear) { in_w<Bit>(0); }

protected:
	// constructor/destructor
	input_merger_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			u32 initval,
			u32 xorval,
			int active);
	virtual ~input_merger_device() override;

	// device-level overrides
	virtual void device_start() override;

	TIMER_CALLBACK_MEMBER(update_state);

	devcb_write_line m_output_handler;

	u32 const m_initval, m_xorval;
	int const m_active;
	u32 m_state;
};


class input_merger_any_high_device : public input_merger_device
{
public:
	input_merger_any_high_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class input_merger_all_high_device : public input_merger_device
{
public:
	input_merger_all_high_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class input_merger_any_low_device : public input_merger_device
{
public:
	input_merger_any_low_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class input_merger_all_low_device : public input_merger_device
{
public:
	input_merger_all_low_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(INPUT_MERGER_ANY_HIGH, input_merger_any_high_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ALL_HIGH, input_merger_all_high_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ANY_LOW,  input_merger_any_low_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ALL_LOW,  input_merger_all_low_device)

#endif // MAME_MACHINE_INPUT_MERGER_H
