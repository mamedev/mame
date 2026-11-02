// license:BSD-3-Clause
// copyright-holders:Carl Lom
/***************************************************************************

    Fujitsu MB654419U (Roland TVF gate array) + TVF-16 filter chip

***************************************************************************/

#ifndef MAME_ROLAND_MB654419U_H
#define MAME_ROLAND_MB654419U_H

#pragma once

class mb654419u_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr int NUM_CHANNELS = 16;

	mb654419u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// host interface, even bytes f000-f01f (offset = (address - f000) / 2)
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// mixed output at the current time (1.0 = one full-scale voice)
	double output_level();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	enum : int
	{
		PARAM_CUTOFF = 0,
		PARAM_LEVEL = 1,
		PARAM_RESONANCE = 2,
		NUM_PARAMS = 8
	};

	struct channel_t
	{
		// Chamberlin state variable filter state
		double lp, bp;
		// coefficients: frequency and damping (registers / 0x4000)
		double f, q;
		bool bypass;
		bool dirty;
	};

	void regs_map(address_map &map) ATTR_COLD;
	void register_w(u16 address, u16 data);
	void update_coefficients(int channel);

	// debugger view of the registers: word address = parameter * 0x10 + channel
	const address_space_config m_regs_config;

	sound_stream *m_stream;

	u16 m_address;      // f000/f002
	u16 m_data;         // f004/f006
	bool m_address_set; // an address was written since the last data commit
	u8 m_setup[6];      // f008-f012
	u16 m_regs[NUM_PARAMS][NUM_CHANNELS];

	channel_t m_channel[NUM_CHANNELS];
	double m_output_level; // mixed output of the latest sample
};

DECLARE_DEVICE_TYPE(MB654419U, mb654419u_device)

#endif // MAME_ROLAND_MB654419U_H
