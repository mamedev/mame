// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_ROLAND_LSP_H
#define MAME_SOUND_ROLAND_LSP_H

#pragma once

class roland_lsp_device : public cpu_device
{
public:
	static constexpr int PROGRAM_SIZE = 384;
	static constexpr int PROGRAM_BASE = 0x080;
	static constexpr u16 ADDRESS_MASK = 0x1ff;
	static constexpr int IRAM_SIZE = 0x80;
	static constexpr int ERAM_SIZE = 0x10000;

	// the slots of the two special opcodes, by slot number
	enum special_slot
	{
		SLOT_JUMP_NEGATIVE = 0x0d, SLOT_JUMP_POSITIVE = 0x0e, SLOT_JUMP = 0x0f,
		SLOT_ERAM_WRITE = 0x10, SLOT_TAP = 0x13, SLOT_MULTIPLIER = 0x14,
		SLOT_AUDIO_OUT = 0x18, SLOT_ERAM_READ = 0x1a, SLOT_AUDIO_IN = 0x1e
	};

	// the sixteen host registers at CA0-3
	enum host_register
	{
		HOST_ADDRESS_LOW = 0x00, HOST_ADDRESS_HIGH = 0x01, HOST_DATA_LOW = 0x02,
		HOST_DATA_MID = 0x03, HOST_DATA_HIGH = 0x04, HOST_CONFIGURE = 0x06,
		HOST_READ_LOW = 0x08, HOST_READ_HIGH = 0x09
	};

	roland_lsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 host_r(offs_t offset);
	void host_w(offs_t offset, u8 data);

	// TRR in and TRS0 out, channel 0 left and 1 right, one pair per sample
	void ser_w(int channel, s32 sample) { m_serial_in[channel & 1] = sample; }
	s32 ser_r(int channel) const { return m_serial_out[channel & 1]; }
	void run_once();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return cycles * 2; }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// one instruction word, split into its fields
	struct instruction
	{
		u8 opcode;
		u8 store;
		u8 eram_cmd;
		u8 offset;
		u8 coefficient;
		u8 shift;

		int slot() const { return offset & 0x1f; }
		bool replace() const { return BIT(offset, 5); }
		bool immediate() const { return offset >= 1 && offset <= 4; }
	};

	enum opcode_family
	{
		OP_MAC_A = 0, OP_SET_A = 1, OP_MAC_B = 2, OP_SET_B = 3,
		OP_MUL = 4, OP_ABS = 5, OP_SPECIAL_A = 6, OP_SPECIAL_B = 7
	};

	static instruction decode(u32 word);

	static s32 saturate(s32 value) { return std::clamp<s32>(value, -0x800000, 0x7fffff); }
	static s32 narrow(s32 value) { return s32(u32(value) << 8) >> 8; }
	static s32 add(s32 accumulator, s32 term) { return s32(u32(accumulator) + u32(term)); }

	void program_map(address_map &map) ATTR_COLD;

	u32 internal_r(offs_t address);
	void internal_w(offs_t address, u32 data);
	void configure(u16 word);

	void eram_clock(u8 command);
	u16 eram_address(bool read) const;
	int iram_address(int offset) const { return (offset + m_buffer_pos) & 0x7f; }
	bool first_half() const { return m_slot < PROGRAM_SIZE / 2; }

	s32 source(const instruction &s) const;
	void multiply(const instruction &s, s32 operand);
	void special(const instruction &s);
	void step();
	void finish_sample();

	address_space_config m_program_config;

	std::unique_ptr<s32[]> m_eram;
	u32 m_program[PROGRAM_SIZE];
	s32 m_iram[IRAM_SIZE];

	int m_icount;
	u16 m_pc;
	u16 m_slot;
	u16 m_configuration;
	bool m_running;
	bool m_halted;

	s32 m_acc[2];
	s32 m_history[2][3];
	s32 m_eram_read;
	u8 m_prev_offset;
	s32 m_multiplier[2];
	u8 m_eram_cmd[16];
	u16 m_eram_base[2];
	bool m_eram_tap2[2];
	s32 m_eram_latch;
	u16 m_tap;
	u8 m_buffer_pos;
	u16 m_eram_pos;
	u16 m_jump_target;
	u8 m_jump_delay;

	s32 m_serial_in[2];
	s32 m_serial_out[2];
	s32 m_audio_out;

	u32 m_host_data;
	u32 m_host_read;
	u16 m_host_address;
};

DECLARE_DEVICE_TYPE(ROLAND_LSP, roland_lsp_device)

#endif // MAME_SOUND_ROLAND_LSP_H
