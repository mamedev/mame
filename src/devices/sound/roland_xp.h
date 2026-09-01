// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_ROLAND_XP_H
#define MAME_SOUND_ROLAND_XP_H

#pragma once

#include "roland_xpd.h"

class roland_xp_device : public cpu_device, public device_sound_interface, public roland_xp_disassembler::info
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// the wave ROM, the internal RAM and the external delay RAM, beside the program
	enum { AS_WAVE = AS_DATA, AS_IRAM = AS_IO, AS_ERAM = AS_OPCODES + 1 };

	static constexpr int MAX_VOICES = 64;
	static constexpr int BUS_COUNT = 64;
	static constexpr int DSP_SLOTS = 256;
	static constexpr int IRAM_SIZE = 256;
	static constexpr int ERAM_SIZE = 0x10000;
	static constexpr int DSP_INPUT_SHIFT = 4;
	static constexpr int OUTPUT_WORDS = 8;

	// the per-voice pages, by page number
	enum page_index
	{
		CONTROL = 0x00, ADDRESS = 0x01, LOOP = 0x02, END = 0x03,
		RESO_TARGET = 0x11, PITCH_TARGET = 0x12, TVF_TARGET = 0x13, TVA2_TARGET = 0x14, TVA1_TARGET = 0x15,
		RESO_CONTROL = 0x16, PITCH_CONTROL = 0x17, TVF_CONTROL = 0x18, TVA2_CONTROL = 0x19, TVA1_CONTROL = 0x1a,
		PITCH_SEED = 0x1b, TVF_SEED = 0x1c, TVA2_SEED = 0x1d, TVA1_SEED = 0x1e,
		FILTER = 0x20, RESO_SEED = 0x21
	};

	// the control block and the other blocks, by byte address
	enum register_address
	{
		CRAM_BASE = 0x2c00, IRAM_BASE = 0x3000, IRAM3_BASE = 0x3200, IRAM3_TARGET_BASE = 0x3300, PRAM_BASE = 0x3400,
		RUN_MASK = 0x3900, READBACK_LOW = 0x3910, READBACK_HIGH = 0x3912, DSP_MODE = 0x3916,
		IRQ_STATUS = 0x3918, IRQ_ACK = 0x391a, ROM_PAGE = 0x3920, ROM_BANK = 0x3922, IRAM3_RATE = 0x3928,
		SEND_BASE = 0x3a00, ROM_WINDOW = 0x3c00
	};

	enum irq_reason { IRQ_VOICE_DONE = 4, IRQ_LOOP_REACHED = 5 };

	roland_xp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto int_callback() { return m_int_callback.bind(); }

	// the DSP RAM words the board's serial link clocks out to its effect processor
	void set_serial_output_words(int left, int right) { m_serial_out_word[0] = left & 0xff; m_serial_out_word[1] = right & 0xff; }

	// a stereo pair out and one sample back a frame, the effect processor's send and return
	auto serial_out_callback() { return m_serial_out_cb.bind(); }
	auto serial_in_callback() { return m_serial_in_cb.bind(); }

	u16 read(offs_t offset, u16 mem_mask = ~0);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	// roland_xp_disassembler::info implementation
	virtual u16 xpd_cram_r(offs_t address) const override { return m_regs[(CRAM_BASE >> 1) + (address & 0xff)]; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_post_load() override;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 1) / 3; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return cycles * 3; }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	enum ramp_law { LAW_LINEAR, LAW_EXPONENTIAL, LAW_S_CURVE };

	// a parameter ramp: a current value moving towards a target under one of three laws
	struct ramp
	{
		s32 current = 0;
		s32 previous = 0;
		s32 target = 0;
		s32 step = 0;
		s32 accumulator = 0;
		s32 midpoint = 0;
		u16 control = 0;
		u32 counter = 0;
		bool active = false;

		int rate() const { return control & 0xfff; }
		u32 hold_mask() const;
		ramp_law control_law() const;

		void seed(s32 value, ramp_law law);
		void retarget(s32 value, ramp_law law);
		void configure(u16 value, ramp_law law);
		void configure(u16 value);
		bool update(ramp_law law);
		s32 value_at(int phase, int period) const;
		s16 coefficient() const;
		s16 coefficient(int phase, int period) const;

	private:
		void arm(ramp_law law);
	};

	struct voice
	{
		ramp pitch, tvf, reso, tva1, tva2;

		u32 region = 0;
		u32 start = 0;
		u32 address = 0;
		u32 loop = 0;
		u32 end = 0;
		u8 start_pending = 0;
		u8 alternate = 0;
		u8 reverse = 0;
		u8 backward = 0;
		u8 reading = 0;
		u8 loop_reported = 0;
		u8 done_reported = 0;
		u16 sub_phase = 0;
		s32 predictor = 0;

		s32 filter_low = 0;
		s32 filter_band = 0;
	};

	struct address_step
	{
		u32 address;
		bool backward;
		bool stopped;
	};

	// one decoded DSP instruction with its coefficient
	struct dsp_slot
	{
		u8 st;
		u8 word;
		u8 col;
		u8 ext;
		u8 eram_op;
		bool read_bypass;
		u16 eram_offset;
		u16 cram;
		s32 coefficient;
		s32 raw;
	};

	// the DSP datapath registers that live across slots and samples
	struct dsp_state
	{
		s32 acc = 0;
		s32 acc_before = 0;
		s32 acc_before_prev = 0;
		s32 product = 0;
		s32 product_prev = 0;
		s32 r = 0;
		s32 r_prev = 0;
		s32 mem = 0;
		s32 mem_prev = 0;
		s32 now = 0;
		s32 latch = 0;
		s32 gain = 0;
		u8 r_age = 0;
		u8 latch_age = 0;
		u8 now_valid = 0;
		u16 fraction = 0;
		u16 cursor = 0;
	};

	s32 exp_decode(s32 value) const;

	u32 page(int voice, int index) const;
	u16 send(int voice, int bank) const { return m_regs[(SEND_BASE >> 1) + (bank & 3) * 64 + (voice & 63)]; }
	bool running(int voice) const { return BIT(m_run_mask, voice & 63); }

	void write_page(int voice, int index, u32 value);
	void write_run_mask(int word, u16 data);
	bool ramp_current(const struct voice &v, int index, u32 &value) const;
	void load_latch(offs_t address);
	void update_int();
	void raise_irq(int voice, int reason);

	u8 rom_byte(const struct voice &v, u32 offset) { return m_wave_cache.read_byte((v.region << 20) | (offset & 0xfffff)); }
	s32 delta_at(const struct voice &v, u32 address);
	void start_reader(int n);
	address_step advance(const struct voice &v, address_step s) const;

	void run_voice(int n);

	static s32 clamp24(s64 value) { return s32(std::clamp<s64>(value, -0x800000, 0x7fffff)); }
	static s32 wrap24(s32 value) { return s32(u32(value) << 8) >> 8; }
	s32 output_word(int word) const { return clamp24(s64(m_iram[word]) * 2); }
	s32 multiply(s32 operand, s32 coefficient) const { return s32((s64(operand) * coefficient + 0x1000) >> 13); }
	static int iram_word(offs_t address) { return (address < IRAM3_BASE ? 0 : 0x40) + ((address - IRAM_BASE) >> 2); }
	void write_iram(int word, u32 value);
	void write_iram_target(int word, u16 value);
	void decode_program();
	void update_iram_ramps();
	s32 operand(const dsp_slot &s) const;
	void execute(const dsp_slot &s);
	void exchange_serial();
	void dsp_frame_start();
	void dsp_step();
	void run_voices();

	void pram_map(address_map &map) ATTR_COLD;
	void iram_map(address_map &map) ATTR_COLD;
	void eram_map(address_map &map) ATTR_COLD;

	u32 pram_r(offs_t address);
	void pram_w(offs_t address, u32 data);
	u32 iram_r(offs_t address) { return m_iram[address & 0xff] & 0xffffff; }
	void iram_w(offs_t address, u32 data) { write_iram(address & 0xff, data); }
	u32 eram_r(offs_t address) { return m_eram[address & 0xffff] & 0xffffff; }
	void eram_w(offs_t address, u32 data) { m_eram[address & 0xffff] = wrap24(data); }

	address_space_config m_pram_config;
	address_space_config m_wave_config;
	address_space_config m_iram_config;
	address_space_config m_eram_config;
	memory_access<27, 0, 0, ENDIANNESS_LITTLE>::cache m_wave_cache;

	devcb_write_line m_int_callback;
	devcb_write32 m_serial_out_cb;
	devcb_read32 m_serial_in_cb;

	sound_stream *m_stream;

	std::unique_ptr<u16[]> m_regs;
	struct voice m_voices[MAX_VOICES];
	s32 m_bus[BUS_COUNT];

	u64 m_run_mask;
	u32 m_read_latch;
	u32 m_frame_counter;
	u32 m_noise;

	u16 m_irq_queue[MAX_VOICES];
	u8 m_irq_head;
	u8 m_irq_count;
	bool m_int_state;

	std::unique_ptr<s32[]> m_eram;
	s32 m_exp_table[257];
	s32 m_iram[IRAM_SIZE];
	u8 m_iram_ramping[IRAM_SIZE];
	dsp_slot m_program[DSP_SLOTS];
	dsp_state m_dsp;
	s32 m_landing[2];
	u8 m_landing_valid;
	bool m_program_dirty;
	bool m_dsp_enabled;

	int m_icount;
	u16 m_pc;

	u8 m_serial_out_word[2];
	s32 m_serial_frame[2];
	s32 m_serial_in;
	u8 m_serial_phase;
};

DECLARE_DEVICE_TYPE(ROLAND_XP, roland_xp_device)

#endif // MAME_SOUND_ROLAND_XP_H
