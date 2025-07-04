// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    disound.h

    Device sound interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DISOUND_H
#define MAME_EMU_DISOUND_H

#include <functional>
#include <utility>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

constexpr int ALL_OUTPUTS       = 65535;    // special value indicating all outputs for the current chip



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum sound_stream_flags : u32;


// ======================> device_sound_interface

class device_sound_interface : public device_interface
{
	friend class sound_manager;

public:
	class sound_route
	{
	public:
		u32                                 m_output;           // output index, or ALL_OUTPUTS
		u32                                 m_input;            // target input index
		float                               m_gain;             // gain
		std::reference_wrapper<device_t>    m_base;             // target search base
		std::string                         m_target;           // target tag
		device_sound_interface             *m_interface;        // target device interface
	};

	// construction/destruction
	device_sound_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sound_interface();

	// configuration access
	std::vector<sound_route> const &routes() const { return m_route_list; }

	// configuration helpers
	template <typename T, bool R>
	device_sound_interface &add_route(u32 output, const device_finder<T, R> &target, double gain, u32 channel = 0)
	{
		const std::pair<device_t &, const char *> ft(target.finder_target());
		return add_route(output, ft.first, ft.second, gain, channel);
	}
	device_sound_interface &add_route(u32 output, const char *target, double gain, u32 channel = 0);
	device_sound_interface &add_route(u32 output, device_sound_interface &target, double gain, u32 channel = 0);
	device_sound_interface &reset_routes() { m_route_list.clear(); return *this; }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) = 0;

	// stream creation
	sound_stream *stream_alloc(int inputs, int outputs, int sample_rate, sound_stream_flags flags = sound_stream_flags(0));

	// helpers
	int inputs() const;
	int outputs() const;
	std::pair<sound_stream *, int> input_to_stream_input(int inputnum) const;
	std::pair<sound_stream *, int> output_to_stream_output(int outputnum) const;
	float input_gain(int inputnum) const;
	float output_gain(int outputnum) const;
	float user_output_gain() const;
	float user_output_gain(int outputnum) const;
	void set_input_gain(int inputnum, float gain);
	void set_output_gain(int outputnum, float gain);
	void set_user_output_gain(float gain);
	void set_user_output_gain(int outputnum, float gain);
	void set_route_gain(int source_channel, device_sound_interface *target, int target_channel, float gain);

	void set_sound_hook(bool enable) { m_sound_hook = enable; }
	bool get_sound_hook() const { return m_sound_hook; }

protected:
	// configuration access
	std::vector<sound_route> &routes() { return m_route_list; }
	device_sound_interface &add_route(u32 output, device_t &base, const char *tag, double gain, u32 channel);

	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const override;

	u32 get_sound_requested_inputs() const { return m_sound_requested_inputs; }
	u32 get_sound_requested_outputs() const { return m_sound_requested_outputs; }
	u64 get_sound_requested_inputs_mask() const { return m_sound_requested_inputs_mask; }
	u64 get_sound_requested_outputs_mask() const { return m_sound_requested_outputs_mask; }

private:
	void sound_request_input(u32 input);

	// internal state
	std::vector<sound_route> m_route_list;      // list of sound routes
	std::vector<sound_stream *> m_sound_streams;
	u64 m_sound_requested_inputs_mask;
	u64 m_sound_requested_outputs_mask;
	u32 m_sound_requested_inputs;
	u32 m_sound_requested_outputs;
	bool m_sound_hook;

	void sound_before_devices_init();
	void sound_after_devices_init();
};

// iterator
typedef device_interface_enumerator<device_sound_interface> sound_interface_enumerator;



// ======================> device_mixer_interface

class device_mixer_interface : public device_sound_interface
{
public:
	// construction/destruction
	device_mixer_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_mixer_interface();

protected:
	// optional operation overrides
	virtual void interface_pre_start() override;

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
};

// iterator
typedef device_interface_enumerator<device_mixer_interface> mixer_interface_enumerator;


#endif // MAME_EMU_DISOUND_H
