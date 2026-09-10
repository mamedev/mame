// license:BSD-3-Clause
// copyright-holders:superctr, giulioz
#ifndef MAME_SOUND_ROLAND_XP_H
#define MAME_SOUND_ROLAND_XP_H

#pragma once

#include "roland_xpd.h"

class roland_xp_device : public cpu_device, public device_sound_interface, protected roland_xp_disassembler::info
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// the wave ROM, the internal RAM and the external delay RAM, beside the program
	enum { AS_WAVE = AS_DATA, AS_IRAM = AS_IO, AS_ERAM = AS_OPCODES + 1 };

	static constexpr int MAX_VOICES = 64;
	static constexpr int BUS_COUNT = 64;
	static constexpr int DSP_SLOTS = 288;
	static constexpr int DSP_BUDGET = 256;
	static constexpr int IRAM_CELLS = 192;
	static constexpr int ERAM_SIZE = 0x10000;
	static constexpr int OUTPUT_PORTS = 3;
	static constexpr int OUTPUTS = OUTPUT_PORTS * 2;

	// the serial ports the `ext=2` strobes clock out on, in the order of the stream's channel pairs;
	// port A, the six-line bus the `ext=1` strobes clock, has its own interface below
	enum output_port { PORT_B = 0, PORT_C = 1, PORT_D = 2 };

	// the per-voice pages, by page number
	enum page_index
	{
		CONTROL = 0x00, ADDRESS = 0x01, LOOP = 0x02, END = 0x03, EXPONENTS = 0x04,
		PREDICTOR = 0x0c, INCREMENT = 0x0d, PHASE = 0x0e, SERVICE = 0x10,
		RESO_TARGET = 0x11, PITCH_TARGET = 0x12, TVF_TARGET = 0x13, TVA2_TARGET = 0x14, TVA1_TARGET = 0x15,
		RESO_CONTROL = 0x16, PITCH_CONTROL = 0x17, TVF_CONTROL = 0x18, TVA2_CONTROL = 0x19, TVA1_CONTROL = 0x1a,
		PITCH_SEED = 0x1b, TVF_SEED = 0x1c, TVA2_SEED = 0x1d, TVA1_SEED = 0x1e,
		FILTER = 0x20, RESO_SEED = 0x21, CUTOFF = 0x22, AMPLITUDE = 0x23,
		PITCH_STEP = 0x24, TVF_STEP = 0x25, TVA1_STEP = 0x26, SMOOTH = 0x27,
		FILTER_BAND = 0x28, FILTER_LOW = 0x29, OUTPUT = 0x2a
	};

	// the control block and the other blocks, by byte address
	enum register_address
	{
		CRAM_BASE = 0x2c00, IRAM_BASE = 0x3000, IRAM3_BASE = 0x3200, IRAM3_TARGET_BASE = 0x3300, PRAM_BASE = 0x3400,
		RUN_MASK = 0x3900, ROM_SELECT = 0x3908, READBACK_LOW = 0x3910, READBACK_HIGH = 0x3912, HIGHEST_VOICE = 0x3914,
		DSP_MODE = 0x3916, IRQ_STATUS = 0x3918, IRQ_ACK = 0x391a, STATUS = 0x391c, ROM_PAGE = 0x3920, ROM_BANK = 0x3922,
		SERIAL_CONFIG = 0x3924, DSP_CONFIG = 0x3926, IRAM3_RATE = 0x3928, DIAG_SELECT = 0x3930, SERIAL_FORMAT = 0x3932,
		VOICE_SELECT = 0x3934, VOICE_WINDOW = 0x3940, VOICE_WINDOW_END = 0x39f0, SEND_WINDOW = 0x39f8,
		SEND_BASE = 0x3a00, ROM_WINDOW = 0x3c00
	};

	// one per interrupt source, as the status word reports them
	enum irq_reason
	{
		IRQ_RESO_DONE = 0, IRQ_TVF_DONE = 1, IRQ_PITCH_DONE = 2, IRQ_TVA2_DONE = 3, IRQ_VOICE_DONE = 4,
		IRQ_LOOP_REACHED = 5, IRQ_LOOP_ALTERNATE = 6, IRQ_FETCH_OVERLOAD = 7, IRQ_MUTE_CHANGED = 8,
		IRQ_REASONS = 16
	};

	roland_xp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto int_callback() { return m_int_callback.bind(); }

	// a word clocked out on port B, C or D (SDOB, SDOC, SDOD): the offset is port * 2 + half, the word
	// the 24-bit word with the bits the wire does not carry cleared.  The stream carries the same six words.
	auto port_out_callback() { return m_port_out_cb.bind(); }

	// port A, the six-line bidirectional bus (SDIA0-5, SDOA0-5): the input is taken once per `ext=1`
	// strobe and the argument is the strobe's ordinal in the frame; what this chip presented at that
	// strobe in its last frame is port_a_out_r() -- the SC-8850's two chips read each other through it
	auto port_a_in_callback() { return m_port_a_in_cb.bind(); }

	// port B's input (SDIB), sampled once per three-strobe group; the argument is the group's ordinal
	auto port_b_in_callback() { return m_port_b_in_cb.bind(); }

	// a chip whose frame another device clocks, as the SC-8850's slave is clocked by its master:
	// execute_run() stands still and run_frame() is one frame
	void set_pumped(bool pumped) { m_pumped = pumped; }

	auto frame_callback() { return m_frame_cb.bind(); }

	void run_frame();

	u32 port_a_out_r(int strobe);

	u16 read(offs_t offset, u16 mem_mask = ~0);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	enum ramp_index { RAMP_PITCH, RAMP_TVF, RAMP_RESO, RAMP_TVA2, RAMP_TVA1 };
	enum ramp_law { LAW_LINEAR, LAW_EXPONENTIAL, LAW_S_CURVE };
	enum launch_phase { IDLE, PRELOAD, INITIALIZE, STARTING, RUNNING };

	// the pages a ramp lives in
	struct ramp_pages
	{
		u8 current;
		u8 control;
		u8 target;
		u8 step;
		u8 reason;
	};

	// what a voice keeps outside its pages
	struct voice
	{
		u8 phase = IDLE;
		u8 format = 0;
		u8 fade_entry = 0;
	};

	struct address_step
	{
		u32 address;
		bool backward;
	};

	struct wave_cell
	{
		s32 mantissa;
		int exponent;
	};

	// one decoded DSP instruction with its coefficient
	// the multiply input a slot's col[5:4] selects
	enum dsp_input { INPUT_PREVIOUS = 0, INPUT_ACC = 1, INPUT_R = 2, INPUT_LATCH = 3 };

	// what col[5:4] selects instead when the function is 0
	enum dsp_special { SPECIAL_NOP = 0, SPECIAL_BRANCH = 1, SPECIAL_INDEXED_READ = 2, SPECIAL_PARALLEL = 3 };

	struct dsp_slot
	{
		u8 st;
		u8 word;
		u8 input;
		u8 function;
		u8 ext;
		u8 eram_op;
		u8 eram_second;
		u16 eram_offset;
		u16 cram;
		s32 coefficient;
		s32 raw;

		bool special(dsp_special which) const { return function == 0 && input == which; }
	};

	// the DSP datapath registers that live across slots and samples
	struct dsp_state
	{
		s32 acc = 0;
		s32 product = 0;
		s32 product_prev = 0;
		s32 r = 0;
		s32 input = 0;
		s32 input_prev = 0;
		s32 now = 0;
		s32 latch = 0;
		s32 gain = 0;
		u8 now_valid = 0;
		u16 cursor = 0;
	};

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

	// roland_xp_disassembler::info implementation
	virtual u16 xpd_cram_r(offs_t address) const override { return m_regs[(CRAM_BASE >> 1) + (address % DSP_SLOTS)]; }
	virtual int xpd_ramp_base() const override { return ramp_base(); }

	s32 exp_decode(s32 value) const;

	int voice_count() const { return (m_regs[HIGHEST_VOICE >> 1] & 0x3f) + 1; }
	int slot_count() const { return voice_count() * 4; }
	int ramp_base() const { return 256 - ((m_regs[DSP_CONFIG >> 1] >> 8) & 0x1f); }
	u32 frame_rate() const { return clock() / (3 * slot_count()); }

	static int page_word(int voice, int index) { return ((index & 0xff) << 7) | ((voice & 63) << 1); }
	u32 page(int voice, int index) const;
	void set_page(int voice, int index, u32 value);
	u16 send(int voice, int bank) const { return m_regs[(SEND_BASE >> 1) + (bank & 3) * 64 + (voice & 63)]; }
	bool running(int voice) const { return BIT(m_run_mask, voice & 63); }

	void write_run_mask(int word, u16 data);
	void commit_run_mask();
	void load_latch(offs_t address);
	void update_int();
	bool offer_irq(int voice, int reason);
	void marker_reached(int voice);
	void update_mute(int voice);

	static bool linear_law(int index, u32 control);
	static s32 linear_step(s32 target, s32 current, int rate);
	static bool s_curve_law(int index, u32 control);
	void service_ramp(int voice, int index, bool launching);
	void update_amplitude(int voice);

	u8 rom_byte(int region, u32 offset) { return m_wave_cache.read_byte((u32(region & 0x7f) << 20) | (offset & 0xfffff)); }
	wave_cell cell_at(int voice, u32 control, u32 address);
	static s32 delta_of(wave_cell c);
	static s32 tap(s32 weight, wave_cell c);
	void launch(int voice);
	address_step advance(int voice, u32 control, address_step s) const;
	bool at_marker(int voice, u32 control, address_step s) const;
	void deposit(int voice, int bank);
	void run_voice(int voice);

	static s32 clamp24(s64 value) { return s32(std::clamp<s64>(value, -0x800000, 0x7fffff)); }
	static s32 clamp29(s64 value) { return s32(std::clamp<s64>(value, -0x10000000, 0x0fffffff)); }
	static constexpr s32 wrap29(s64 value) { return s32(util::sext(value, 29)); }
	static constexpr s32 wrap24(s32 value) { return util::sext(value, 24); }
	static constexpr s32 wrap20(s32 value) { return util::sext(value, 20); }
	static constexpr s32 wrap18(s32 value) { return util::sext(value, 18); }
	static s32 fold24(s32 value);
	static s32 multiply(s32 operand, s32 coefficient) { return clamp29((s64(operand) * coefficient) / 8192); }
	static s32 multiply_q15(s32 operand, s32 factor, int shift) { return clamp29(((s64(operand) * factor) << shift) / 32768); }
	static s32 gain_current(s32 cell) { return s16(cell >> 10); }
	static s32 gain_goal(s32 cell);
	int cell(int word, int parity) const;
	int cell(int word) const { return cell(word, m_parity); }
	static int host_cell(offs_t address) { return (address - IRAM_BASE) >> 2; }
	s32 wire_word(s32 word) const;
	void write_iram(int cell, u32 value);
	void write_iram_target(int word, u16 value);
	void decode_program();
	void update_iram_ramps();
	static const char *unimplemented(const dsp_slot &s);
	s32 operand(const dsp_slot &s) const;
	s32 factor(int select, bool complement) const;
	bool alu(int function, int mode, s32 immediate);
	void parallel_op(const dsp_slot &s);
	void execute(const dsp_slot &s);
	void strobe(const dsp_slot &s);
	void dsp_frame_start();
	void dsp_step();
	void frame_end();
	void cycle();

	void pram_map(address_map &map) ATTR_COLD;
	void iram_map(address_map &map) ATTR_COLD;
	void eram_map(address_map &map) ATTR_COLD;

	u32 pram_r(offs_t address);
	void pram_w(offs_t address, u32 data);
	u32 iram_r(offs_t address) { return m_iram[address % IRAM_CELLS] & 0x3ffffff; }
	void iram_w(offs_t address, u32 data) { write_iram(address % IRAM_CELLS, data); }
	u32 eram_r(offs_t address) { return m_eram[address & 0xffff] & 0xffffff; }
	void eram_w(offs_t address, u32 data) { m_eram[address & 0xffff] = wrap24(data); }

	static const ramp_pages RAMPS[5];

	address_space_config m_pram_config;
	address_space_config m_wave_config;
	address_space_config m_iram_config;
	address_space_config m_eram_config;
	memory_access<27, 0, 0, ENDIANNESS_LITTLE>::cache m_wave_cache;

	devcb_write_line m_int_callback;
	devcb_write32 m_port_out_cb;
	devcb_read32 m_port_a_in_cb;
	devcb_read32 m_port_b_in_cb;
	devcb_write_line m_frame_cb;

	sound_stream *m_stream;

	std::unique_ptr<u16[]> m_regs;
	struct voice m_voices[MAX_VOICES];
	u64 m_bus_written;

	u64 m_run_mask;
	u64 m_run_pending;
	u32 m_read_latch;
	u16 m_write_latch;
	u32 m_frame_counter;

	u16 m_irq_event;
	bool m_irq_active;
	bool m_irq_frame_used;
	bool m_int_state;

	std::unique_ptr<s32[]> m_eram;
	s32 m_exp_table[257];
	s32 m_iram[IRAM_CELLS];
	u8 m_iram_ramping[64];
	dsp_slot m_program[DSP_SLOTS];
	dsp_state m_dsp;
	s32 m_landing[2];
	u8 m_landing_valid;
	bool m_program_dirty;
	bool m_dsp_enabled;
	u8 m_parity;

	int m_icount;
	u16 m_pc;
	u16 m_cycle;

	u8 m_position;
	u8 m_strobe_a;
	u8 m_strobe_bcd;
	s32 m_port_a_out[DSP_SLOTS];
	s32 m_port_word[OUTPUT_PORTS][2];
	s32 m_port_a_in;
	s32 m_port_b_in;
	bool m_port_in_enabled;
	bool m_pumped;
};

DECLARE_DEVICE_TYPE(ROLAND_XP, roland_xp_device)

#endif // MAME_SOUND_ROLAND_XP_H
