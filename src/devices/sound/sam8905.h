// license:BSD-3-Clause
// copyright-holders:Draft
#ifndef MAME_SOUND_SAM8905_H
#define MAME_SOUND_SAM8905_H

#pragma once

class sam8905_device : public device_t, public device_sound_interface
{
public:
	sam8905_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// CPU interface
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	// External waveform memory callback (address 20-bit → data 12-bit signed)
	auto waveform_read_callback() { return m_waveform_read.bind(); }

	// External waveform memory write callback (address 15-bit, data 12-bit signed)
	// Used by FX chip to write to delay/reverb RAM
	auto waveform_write_callback() { return m_waveform_write.bind(); }

	// Sample output callback for inter-chip audio (L/R packed as upper/lower 16 bits)
	auto sample_output_callback() { return m_sample_output.bind(); }

	// Master-slave synchronization for chained SAM chips
	// In slave mode, the chip doesn't generate samples on its own - it's triggered by master
	void set_slave_mode(bool slave) { m_slave_mode = slave; }
	void process_frame(int32_t &out_l, int32_t &out_r);  // Process one frame, return L/R output

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// Chip-level working registers (shared across all slots)
	uint32_t m_a, m_b;          // Working registers
	uint32_t m_x, m_y;          // Waveform/multiplier registers
	uint32_t m_phi, m_wf;       // Phase and waveform select
	uint32_t m_mul_result;      // Multiplier pipeline latch
	bool m_carry;               // Adder carry flag
	bool m_clear_rqst;          // Clear request flag
	bool m_int_mod;             // Internal modulation flag
	uint32_t m_l_acc, m_r_acc;  // Output accumulators
	uint8_t m_mix_l, m_mix_r;   // Output attenuation

	// State
	sound_stream *m_stream;
	std::unique_ptr<uint16_t[]> m_aram; // 256 x 15-bit micro-instructions
	std::unique_ptr<uint32_t[]> m_dram; // 256 x 19-bit parameters
	uint8_t m_control_reg;
	uint8_t m_address_reg;
	uint32_t m_data_latch;
	uint8_t m_interrupt_latch;  // Slot/word that triggered WM WSP interrupt

	// Helpers
	void execute_cycle(int slot_idx, uint16_t inst, int pc_start = -1, int pc = -1);
	int32_t get_waveform(uint32_t wf, uint32_t phi, uint8_t mad, int slot_idx);
	uint32_t get_constant(uint8_t mad);

	// External waveform memory callbacks
	devcb_read16 m_waveform_read;
	devcb_write16 m_waveform_write;

	// Sample output callback for inter-chip audio
	devcb_write32 m_sample_output;

	// Master-slave mode for chained chips
	bool m_slave_mode;
	int32_t m_last_out_l, m_last_out_r;  // Last output for slave mode stream

	static constexpr uint32_t MASK19 = 0x7FFFF;
};

DECLARE_DEVICE_TYPE(SAM8905, sam8905_device)

#endif // MAME_SOUND_SAM8905_H
