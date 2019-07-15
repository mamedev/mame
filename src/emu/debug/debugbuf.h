// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Buffering interface for the disassembly windows

#ifndef MAME_EMU_DEBUG_DEBUGBUF_H
#define MAME_EMU_DEBUG_DEBUGBUF_H

#pragma once

class debug_disasm_buffer
{
public:
	debug_disasm_buffer(device_t &device);

	void disassemble(offs_t pc, std::string &instruction, offs_t &next_pc, offs_t &size, u32 &info) const;
	u32 disassemble_info(offs_t pc) const;
	std::string pc_to_string(offs_t pc) const;
	std::string data_to_string(offs_t pc, offs_t size, bool opcode) const;
	void data_get(offs_t pc, offs_t size, bool opcode, std::vector<u8> &data) const;

	offs_t next_pc(offs_t pc, offs_t step) const;
	offs_t next_pc_wrap(offs_t pc, offs_t step) const;

private:
	class debug_data_buffer : public util::disasm_interface::data_buffer
	{
	public:
		debug_data_buffer(util::disasm_interface const &intf);
		~debug_data_buffer() = default;

		virtual u8  r8 (offs_t pc) const override;
		virtual u16 r16(offs_t pc) const override;
		virtual u32 r32(offs_t pc) const override;
		virtual u64 r64(offs_t pc) const override;

		void set_source(address_space &space);
		void set_source(debug_data_buffer &back, bool opcode);

		bool active() const;

		address_space *get_underlying_space() const;
		std::string data_to_string(offs_t pc, offs_t size) const;
		void data_get(offs_t pc, offs_t size, std::vector<u8> &data) const;

	private:
		util::disasm_interface const &m_intf;

		std::function<offs_t (offs_t)> m_pc_delta_to_bytes;
		std::function<void (offs_t, offs_t)> m_do_fill;
		std::function<u8  (offs_t)> m_do_r8;
		std::function<u16 (offs_t)> m_do_r16;
		std::function<u32 (offs_t)> m_do_r32;
		std::function<u64 (offs_t)> m_do_r64;
		std::function<std::string (offs_t, offs_t)> m_data_to_string;
		std::function<void (offs_t, offs_t, std::vector<u8> &)> m_data_get;
		std::function<offs_t (offs_t, offs_t)> m_next_pc_wrap;

		address_space *m_space;
		debug_data_buffer *m_back;
		bool m_opcode;
		offs_t m_page_mask, m_pc_mask;
		mutable offs_t m_lstart, m_lend;
		mutable bool m_wrapped;
		mutable std::vector<u8> m_buffer;

		template<typename T> T *get_ptr(offs_t lpc) {
			return reinterpret_cast<T *>(&m_buffer[0]) + ((lpc - m_lstart) & m_pc_mask);
		}

		template<typename T> T get(offs_t lpc) const {
			return reinterpret_cast<const T *>(&m_buffer[0])[(lpc - m_lstart) & m_pc_mask];
		}

		void setup_methods();
		void fill(offs_t lstart, offs_t size) const;

	};

	util::disasm_interface &m_dintf;
	device_memory_interface *const m_mintf;

	std::function<offs_t (offs_t, offs_t)> m_next_pc;
	std::function<offs_t (offs_t, offs_t)> m_next_pc_wrap;
	std::function<std::string (offs_t)> m_pc_to_string;

	debug_data_buffer m_buf_raw, m_buf_opcodes, m_buf_params;
	u32 const m_flags;
	offs_t m_page_mask, m_pc_mask;
};

#endif

