// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Buffering interface for the disassembly windows

#include "emu.h"
#include "debugbuf.h"

debug_disasm_buffer::debug_data_buffer::debug_data_buffer(util::disasm_interface const &intf) : m_intf(intf)
{
	m_dev = nullptr;
	m_spacenum = -1;
	m_back = nullptr;
	m_opcode = true;
	m_lstart = m_lend = 0;
	m_wrapped = false;
}

bool debug_disasm_buffer::debug_data_buffer::active() const
{
	return m_dev || m_back;
}

void debug_disasm_buffer::debug_data_buffer::set_source(device_memory_interface *dev, int spacenum)
{
	m_dev = dev;
	m_spacenum = spacenum;
	setup_methods();
}

void debug_disasm_buffer::debug_data_buffer::set_source(debug_data_buffer &back, bool opcode)
{
	m_back = &back;
	m_dev = back.m_dev;
	m_spacenum = back.m_spacenum;
	m_opcode = opcode;
	setup_methods();
}

u8  debug_disasm_buffer::debug_data_buffer::r8 (offs_t pc) const
{
	return m_do_r8(pc & m_pc_mask);
}

u16 debug_disasm_buffer::debug_data_buffer::r16(offs_t pc) const
{
	return m_do_r16(pc & m_pc_mask);
}

u32 debug_disasm_buffer::debug_data_buffer::r32(offs_t pc) const
{
	return m_do_r32(pc & m_pc_mask);
}

u64 debug_disasm_buffer::debug_data_buffer::r64(offs_t pc) const
{
	return m_do_r64(pc & m_pc_mask);
}

void debug_disasm_buffer::debug_data_buffer::fill(offs_t lstart, offs_t size) const
{
	offs_t lend = (lstart + size) & m_pc_mask;
	if(m_page_mask) {
		if((lstart ^ lend) & ~m_page_mask) {
			lstart = lstart & ~m_page_mask;
			lend = (((lend - 1) | m_page_mask) + 1) & m_pc_mask;
		}
	}

	if(!m_buffer.empty()) {
		if(m_lstart == m_lend)
			return;
		if(m_wrapped) {
			if(lstart >= m_lstart && (lend > m_lstart || lend <= m_lend))
				return;
			if(lstart < m_lend && lend <= m_lend)
				return;
		} else {
			if(lstart < lend && lstart >= m_lstart && lend <= m_lend)
				return;
		}
	}

	// FIXME: This buffer tends to hog more memory than necessary for typical disassembly tasks.
	// If the PC values supplied are far enough apart, the buffer may suddenly increase in size to a gigabyte or more.
	if(m_buffer.empty()) {
		m_lstart = lstart;
		m_lend = lend;
		m_wrapped = lend < lstart;
		offs_t size = m_pc_delta_to_bytes((lend - lstart) & m_pc_mask);
		m_buffer.resize(size);
		m_do_fill(lstart, lend);

	} else {
		offs_t n_lstart, n_lend;
		if(lstart > lend) {
			if(m_wrapped) {
				// Old is wrapped, new is wrapped, just extend
				n_lstart = std::min(m_lstart, lstart);
				n_lend = std::max(m_lend, lend);
			} else {
				// Old is unwrapped, new is wrapped.  Reduce the amount of "useless" data.
				offs_t gap_post = m_lend >= lstart ? 0 : lstart - m_lend;
				offs_t gap_pre  = m_lstart <= lend ? 0 : m_lstart - lend;
				if(gap_post < gap_pre) {
					// extend the old one end until it reaches the new one
					n_lstart = std::min(m_lstart, lstart);
					n_lend = lend;
				} else {
					// extend the old one start until it reaches the new one
					n_lstart = lstart;
					n_lend = std::max(m_lend, lend);
				}
				m_wrapped = true;
			}
		} else if(m_wrapped) {
			// Old is wrapped, new is unwrapped.  Reduce the amount of "useless" data.
			offs_t gap_post = m_lend >= lstart ? 0 : lstart - m_lend;
			offs_t gap_pre  = m_lstart <= lend ? 0 : m_lstart - lend;
			if(gap_post < gap_pre) {
				// extend the old one end until it reaches the new one
				n_lstart = m_lstart;
				n_lend = lend;
			} else {
				// extend the old one start until it reaches the new one
				n_lstart = lstart;
				n_lend = m_lend;
			}
		} else {
			// Both are unwrapped, decide whether to wrap.
			// If there's overlap, don't wrap, just extend
			if(lend >= m_lstart && lstart < m_lend) {
				n_lstart = std::min(m_lstart, lstart);
				n_lend = std::max(m_lend, lend);
			} else {
				// If there's no overlap, compute the gap with wrapping or without
				offs_t gap_unwrapped = lstart > m_lstart ? lstart - m_lend : m_lstart - lend;
				offs_t gap_wrapped = lstart > m_lstart ? (m_lstart - lend) & m_pc_mask : (lstart - m_lend) & m_pc_mask;
				if(gap_unwrapped < gap_wrapped) {
					n_lstart = std::min(m_lstart, lstart);
					n_lend = std::max(m_lend, lend);
				} else {
					n_lstart = std::max(m_lstart, lstart);
					n_lend = std::min(m_lend, lend);
					m_wrapped = true;
				}
			}
		}

		if(n_lstart != m_lstart) {
			offs_t size = m_pc_delta_to_bytes((m_lstart - n_lstart) & m_pc_mask);
			m_buffer.insert(m_buffer.begin(), size, 0);
			offs_t old_lstart = m_lstart;
			m_lstart = n_lstart;
			m_do_fill(m_lstart, old_lstart);
		}
		if(n_lend != m_lend) {
			offs_t size = m_pc_delta_to_bytes((n_lend - m_lstart) & m_pc_mask);
			m_buffer.resize(size);
			offs_t old_lend = m_lend;
			m_lend = n_lend;
			m_do_fill(old_lend, m_lend);
		}
	}
}

std::string debug_disasm_buffer::debug_data_buffer::data_to_string(offs_t pc, offs_t size) const
{
	return m_data_to_string(pc, size);
}

void debug_disasm_buffer::debug_data_buffer::data_get(offs_t pc, offs_t size, std::vector<u8> &data) const
{
	return m_data_get(pc, size, data);
}

void debug_disasm_buffer::debug_data_buffer::setup_methods()
{
	const address_space_config *config = m_dev->logical_space_config(m_spacenum);
	int shift = config->addr_shift();
	int alignment = m_intf.opcode_alignment();
	endianness_t endian = config->endianness();
	bool is_octal = config->is_octal();

	m_pc_mask = config->logaddrmask();

	if(m_intf.interface_flags() & util::disasm_interface::PAGED)
		m_page_mask = (1 << m_intf.page_address_bits()) - 1;
	else
		m_page_mask = 0;

	// Define the byte counter
	switch(shift) {
	case -3: m_pc_delta_to_bytes = [](offs_t delta) { return delta << 3; }; break;
	case -2: m_pc_delta_to_bytes = [](offs_t delta) { return delta << 2; }; break;
	case -1: m_pc_delta_to_bytes = [](offs_t delta) { return delta << 1; }; break;
	case  0: m_pc_delta_to_bytes = [](offs_t delta) { return delta;      }; break;
	case  3: m_pc_delta_to_bytes = [](offs_t delta) { return delta >> 3; }; break;
	default: throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer::setup_methods: Abnormal address bus shift\n");
	}

	// Define the filler
	if(!m_back) {
		// get the data from given space
		if(m_intf.interface_flags() & util::disasm_interface::NONLINEAR_PC) {
			switch(shift) {
			case -1:
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u16 *dest = get_ptr<u16>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = m_intf.pc_linear_to_real(lpc);
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space)) {
							auto dis = space->device().machine().disable_side_effects();
							*dest++ = space->read_word(tpc);
						} else
							*dest++ = 0;
					}
				};
				break;
			case 0:
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u8 *dest = get_ptr<u8>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = m_intf.pc_linear_to_real(lpc);
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_byte(tpc);
						else
							*dest++ = 0;
					}
				};
				break;
			}

		} else {
			switch(shift) {
			case -3: // bus granularity 64
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u64 *dest = get_ptr<u64>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = lpc;
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_qword(tpc);
						else
							*dest++ = 0;
					}
				};
				break;

			case -2: // bus granularity 32
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u32 *dest = get_ptr<u32>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = lpc;
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_dword(tpc);
						else
							*dest++ = 0;
					}
				};
				break;

			case -1: // bus granularity 16
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u16 *dest = get_ptr<u16>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = lpc;
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_word(tpc);
						else
							*dest++ = 0;
					}
				};
				break;

			case  0: // bus granularity 8
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u8 *dest = get_ptr<u8>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask) {
						offs_t tpc = lpc;
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_byte(tpc);
						else
							*dest++ = 0;
					}
				};
				break;

			case  3: // bus granularity 1, stored as u16
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					auto dis = m_dev->device().machine().disable_side_effects();
					u16 *dest = reinterpret_cast<u16 *>(&m_buffer[0]) + ((lstart - m_lstart) >> 4);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 0x10) & m_pc_mask) {
						offs_t tpc = lpc;
						address_space *space;
						if (m_dev->translate(m_spacenum, device_memory_interface::TR_FETCH, tpc, space))
							*dest++ = space->read_word(tpc);
						else
							*dest++ = 0;
					}
				};
				break;
			}
		}
	} else {
		// get the data from a back buffer and decrypt it through the device
		// size chosen is alignment * granularity
		assert(!(m_intf.interface_flags() & util::disasm_interface::NONLINEAR_PC));

		switch(shift) {
		case -3: // bus granularity 64, endianness irrelevant
			m_do_fill = [this](offs_t lstart, offs_t lend) {
				u64 *dest = get_ptr<u64>(lstart);
				for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask)
					*dest++ = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
			};
			break;

		case -2: // bus granularity 32
			switch(alignment) {
			case 1: // bus granularity 32, alignment 32, endianness irrelevant
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					u32 *dest = get_ptr<u32>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask)
						*dest++ = m_intf.decrypt32(m_back->r32(lpc), lpc, m_opcode);
				};
				break;

			case 2: // bus granularity 32, alignment 64
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 32, alignment 64, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u32 *dest = get_ptr<u32>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >> 32;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 32, bus width 64, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u32 *dest = get_ptr<u32>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val >> 32;
							*dest++ = val;
						}
					};
					break;
				}
				break;
			}
			break;

		case -1: // bus granularity 16
			switch(alignment) {
			case 1: // bus granularity 16, alignment 16, endianness irrelevant
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					u16 *dest = get_ptr<u16>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask)
						*dest++ = m_intf.decrypt16(m_back->r16(lpc), lpc, m_opcode);
				};
				break;

			case 2: // bus granularity 16, alignment 32
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 16, alignment 32, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u16 *dest = get_ptr<u16>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u32 val = m_intf.decrypt32(m_back->r32(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >> 16;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 16, alignment 32, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u16 *dest = get_ptr<u16>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u32 val = m_intf.decrypt32(m_back->r32(lpc), lpc, m_opcode);
							*dest++ = val >> 16;
							*dest++ = val;
						}
					};
					break;
				}
				break;

			case 4: // bus granularity 16, alignment 64
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 16, alignment 64, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u16 *dest = get_ptr<u16>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 4) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >> 16;
							*dest++ = val >> 32;
							*dest++ = val >> 48;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 16, alignment 64, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u16 *dest = get_ptr<u16>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 4) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val >> 48;
							*dest++ = val >> 32;
							*dest++ = val >> 16;
							*dest++ = val;
						}
					};
					break;
				}
				break;
			}
			break;

		case  0: // bus granularity 8
			switch(alignment) {
			case 1: // bus granularity 8, alignment 8, endianness irrelevant
				m_do_fill = [this](offs_t lstart, offs_t lend) {
					u8 *dest = get_ptr<u8>(lstart);
					for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 1) & m_pc_mask)
						*dest++ = m_intf.decrypt8(m_back->r8(lpc), lpc, m_opcode);
				};
				break;

			case 2: // bus granularity 8, alignment 16
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 8, alignment 16, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u16 val = m_intf.decrypt16(m_back->r16(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >>  8;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 16, alignment 16, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u16 val = m_intf.decrypt16(m_back->r16(lpc), lpc, m_opcode);
							*dest++ = val >>  8;
							*dest++ = val;
						}
					};
					break;
				}
				break;

			case 4: // bus granularity 8, alignment 32
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 8, alignment 16, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 4) & m_pc_mask) {
							u32 val = m_intf.decrypt32(m_back->r32(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >>  8;
							*dest++ = val >> 16;
							*dest++ = val >> 24;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 16, alignment 32, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 4) & m_pc_mask) {
							u32 val = m_intf.decrypt32(m_back->r32(lpc), lpc, m_opcode);
							*dest++ = val >> 24;
							*dest++ = val >> 16;
							*dest++ = val >>  8;
							*dest++ = val;
						}
					};
					break;
				}
				break;


			case 8: // bus granularity 8, alignment 64
				switch(endian) {
				case ENDIANNESS_LITTLE:  // bus granularity 8, alignment 64, little endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 8) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val;
							*dest++ = val >>  8;
							*dest++ = val >> 16;
							*dest++ = val >> 24;
							*dest++ = val >> 32;
							*dest++ = val >> 40;
							*dest++ = val >> 48;
							*dest++ = val >> 56;
						}
					};
					break;

				case ENDIANNESS_BIG:  // bus granularity 8, alignment 64, big endian
					m_do_fill = [this](offs_t lstart, offs_t lend) {
						u8 *dest = get_ptr<u8>(lstart);
						for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 2) & m_pc_mask) {
							u64 val = m_intf.decrypt64(m_back->r64(lpc), lpc, m_opcode);
							*dest++ = val >> 56;
							*dest++ = val >> 48;
							*dest++ = val >> 40;
							*dest++ = val >> 32;
							*dest++ = val >> 24;
							*dest++ = val >> 16;
							*dest++ = val >>  8;
							*dest++ = val;
						}
					};
					break;
				}
				break;
			}
			break;

		case  3: // bus granularity 1, alignment 16, little endian (bit addressing, stored as u16, tms3401x)
			assert(alignment == 16);
			assert(endian == ENDIANNESS_LITTLE);
			m_do_fill = [this](offs_t lstart, offs_t lend) {
				u16 *dest = reinterpret_cast<u16 *>(&m_buffer[0]) + ((lstart - m_lstart) >> 4);
				for(offs_t lpc = lstart; lpc != lend; lpc = (lpc + 0x10) & m_pc_mask)
					*dest++ = m_intf.decrypt16(m_back->r16(lpc), lpc, m_opcode);
			};
			break;
		}
	}

	// Define the accessors
	if(m_intf.interface_flags() & util::disasm_interface::NONLINEAR_PC) {
		switch(shift) {
		case -1:
			m_do_r8  = [](offs_t pc) -> u8  { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r8 access on 16-bits granularity bus\n"); };
			m_do_r16 = [this](offs_t pc) -> u16 {
				offs_t lpc = m_intf.pc_real_to_linear(pc);
				fill(lpc, 1);
				const u16 *src = get_ptr<u16>(lpc);
				return src[0];
			};

			switch(endian) {
			case ENDIANNESS_LITTLE:
				m_do_r32 = [this](offs_t pc) -> u32 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 2);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u16>(lpc) << (j*16);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r64 = [this](offs_t pc) -> u64 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 4);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(get<u16>(lpc)) << (j*16);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				break;

			case ENDIANNESS_BIG:
				m_do_r32 = [this](offs_t pc) -> u32 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 2);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u16>(lpc) << ((1-j)*16);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r64 = [this](offs_t pc) -> u64 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 4);
					u64 r = 0;
					for(int j=0; j != 4; j++) {
						r |= u64(get<u16>(lpc)) << ((3-j)*16);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				break;
			}
			break;

		case 0:
			m_do_r8 = [this](offs_t pc) -> u8 {
				offs_t lpc = m_intf.pc_real_to_linear(pc);
				fill(lpc, 1);
				const u8 *src = get_ptr<u8>(lpc);
				return src[0];
			};

			switch(endian) {
			case ENDIANNESS_LITTLE:
				m_do_r16 = [this](offs_t pc) -> u16 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 2);
					u16 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(lpc) << (j*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r32 = [this](offs_t pc) -> u32 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 4);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(lpc) << (j*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r64 = [this](offs_t pc) -> u64 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 8);
					u64 r = 0;
					for(int j=0; j != 8; j++) {
						r |= u64(get<u8>(lpc)) << (j*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				break;

			case ENDIANNESS_BIG:
				m_do_r16 = [this](offs_t pc) -> u16 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 2);
					u16 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(lpc) << ((1-j)*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r32 = [this](offs_t pc) -> u32 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 4);
					u32 r = 0;
					for(int j=0; j != 2; j++) {
						r |= get<u8>(lpc) << ((3-j)*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				m_do_r64 = [this](offs_t pc) -> u64 {
					offs_t lpc = m_intf.pc_real_to_linear(pc);
					fill(lpc, 8);
					u64 r = 0;
					for(int j=0; j != 8; j++) {
						r |= u64(get<u8>(lpc)) << ((7-j)*8);
						lpc = (lpc & ~m_page_mask) | ((lpc + 1) & m_page_mask);
					}
					return r;
				};
				break;
			}
			break;
		}
	} else {
		switch(shift) {
		case -3: // bus granularity 64
			m_do_r8  = [](offs_t pc) -> u8  { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r8 access on 64-bits granularity bus\n"); };
			m_do_r16 = [](offs_t pc) -> u16 { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r16 access on 64-bits granularity bus\n"); };
			m_do_r32 = [](offs_t pc) -> u32 { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r32 access on 64-bits granularity bus\n"); };
			m_do_r64 = [this](offs_t pc) -> u64 {
				fill(pc, 1);
				const u64 *src = get_ptr<u64>(pc);
				return src[0];
			};
			break;

		case -2: // bus granularity 32
			m_do_r8  = [](offs_t pc) -> u8  { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r8 access on 32-bits granularity bus\n"); };
			m_do_r16 = [](offs_t pc) -> u16 { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r16 access on 32-bits granularity bus\n"); };
			m_do_r32 = [this](offs_t pc) -> u32 {
				fill(pc, 1);
				const u32 *src = get_ptr<u32>(pc);
				return src[0];
			};
			switch(endian) {
			case ENDIANNESS_LITTLE:
				if(m_page_mask) {
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 2);
						u64 r = 0;
						for(int j=0; j != 2; j++) {
							r |= u64(get<u32>(pc)) << (j*32);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 2);
						const u32 *src = get_ptr<u32>(pc);
						return u64(src[0]) | (u64(src[1]) << 32);
					};
				}
				break;
			case ENDIANNESS_BIG:
				if(m_page_mask) {
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 2);
						u64 r = 0;
						for(int j=0; j != 2; j++) {
							r |= u64(get<u32>(pc)) << ((1-j)*32);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 2);
						const u32 *src = get_ptr<u32>(pc);
						return (u64(src[0]) << 32) | u64(src[1]);
					};
				}
				break;
			}
			break;

		case -1: // bus granularity 16
			m_do_r8  = [](offs_t pc) -> u8  { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r8 access on 16-bits granularity bus\n"); };
			m_do_r16 = [this](offs_t pc) -> u16 {
				fill(pc, 1);
				const u16 *src = get_ptr<u16>(pc);
				return src[0];
			};
			switch(endian) {
			case ENDIANNESS_LITTLE:
				if(m_page_mask) {
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 2);
						u32 r = 0;
						for(int j=0; j != 2; j++) {
							r |= get<u16>(pc) << (j*16);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 4);
						u64 r = 0;
						for(int j=0; j != 4; j++) {
							r |= u64(get<u16>(pc)) << (j*16);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 2);
						const u16 *src = get_ptr<u16>(pc);
						return src[0] | (src[1] << 16);
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 4);
						const u16 *src = get_ptr<u16>(pc);
						return u64(src[0]) | (u64(src[1]) << 16) | (u64(src[2]) << 32) | (u64(src[3]) << 48);
					};
				}
				break;
			case ENDIANNESS_BIG:
				if(m_page_mask) {
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 2);
						u32 r = 0;
						for(int j=0; j != 2; j++) {
							r |= get<u16>(pc) << ((1-j)*16);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 4);
						u64 r = 0;
						for(int j=0; j != 4; j++) {
							r |= u64(get<u16>(pc)) << ((3-j)*16);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 2);
						const u16 *src = get_ptr<u16>(pc);
						return (src[0] << 16) | src[1];
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 4);
						const u16 *src = get_ptr<u16>(pc);
						return (u64(src[0]) << 48) | (u64(src[1]) << 32) | u64(src[2] << 16) | u64(src[3]);
					};
				}
				break;
			}
			break;

		case  0: // bus granularity 8
			m_do_r8 = [this](offs_t pc) -> u8 {
				fill(pc, 1);
				const u8 *src = get_ptr<u8>(pc);
				return src[0];
			};
			switch(endian) {
			case ENDIANNESS_LITTLE:
				if(m_page_mask) {
					m_do_r16 = [this](offs_t pc) -> u16 {
						fill(pc, 2);
						u16 r = 0;
						for(int j=0; j != 2; j++) {
							r |= get<u8>(pc) << (j*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 4);
						u32 r = 0;
						for(int j=0; j != 4; j++) {
							r |= get<u8>(pc) << (j*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 8);
						u64 r = 0;
						for(int j=0; j != 8; j++) {
							r |= u64(get<u8>(pc)) << (j*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r16 = [this](offs_t pc) -> u16 {
						fill(pc, 2);
						const u8 *src = get_ptr<u8>(pc);
						return src[0] | (src[1] << 8);
					};
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 4);
						const u8 *src = get_ptr<u8>(pc);
						return src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 8);
						const u8 *src = get_ptr<u8>(pc);
						return u64(src[0]) | (u64(src[1]) << 8) | (u64(src[2]) << 16) | (u64(src[3]) << 24) |
						(u64(src[4]) << 32) | (u64(src[5]) << 40) | (u64(src[6]) << 48) | (u64(src[7]) << 56);
					};
				}
				break;
			case ENDIANNESS_BIG:
				if(m_page_mask) {
					m_do_r16 = [this](offs_t pc) -> u16 {
						fill(pc, 2);
						u16 r = 0;
						for(int j=0; j != 2; j++) {
							r |= get<u8>(pc) << ((1-j)*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 4);
						u32 r = 0;
						for(int j=0; j != 4; j++) {
							r |= get<u8>(pc) << ((3-j)*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 8);
						u64 r = 0;
						for(int j=0; j != 8; j++) {
							r |= u64(get<u8>(pc)) << ((7-j)*8);
							pc = (pc & ~m_page_mask) | ((pc + 1) & m_page_mask);
						}
						return r;
					};
				} else {
					m_do_r16 = [this](offs_t pc) -> u16 {
						fill(pc, 2);
						const u8 *src = get_ptr<u8>(pc);
						return (src[0] << 8) | src[1];
					};
					m_do_r32 = [this](offs_t pc) -> u32 {
						fill(pc, 4);
						const u8 *src = get_ptr<u8>(pc);
						return (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
					};
					m_do_r64 = [this](offs_t pc) -> u64 {
						fill(pc, 8);
						const u8 *src = get_ptr<u8>(pc);
						return (u64(src[0]) << 56) | (u64(src[1]) << 48) | (u64(src[2]) << 40) | (u64(src[3]) << 32) |
						(u64(src[4]) << 24) | (u64(src[5]) << 16) | (u64(src[6]) << 8) | u64(src[7]);
					};
				}
				break;
			}
			break;

		case  3: // bus granularity 1, u16 storage, no paging
			assert(endian == ENDIANNESS_LITTLE);
			assert(!m_page_mask);
			m_do_r8  = [](offs_t pc) -> u8  { throw emu_fatalerror("debug_disasm_buffer::debug_data_buffer: r8 access on 1-bit/16 wide granularity bus\n"); };
			m_do_r16 = [this](offs_t pc) -> u16 {
				fill(pc, 16);
				const u16 *src = reinterpret_cast<u16 *>(&m_buffer[0]) + ((pc - m_lstart) >> 4);
				return src[0];
			};
			m_do_r32 = [this](offs_t pc) -> u32 {
				fill(pc, 32);
				const u16 *src = reinterpret_cast<u16 *>(&m_buffer[0]) + ((pc - m_lstart) >> 4);
				return src[0] | (src[1] << 16);
			};
			m_do_r64 = [this](offs_t pc) -> u64 {
				fill(pc, 64);
				const u16 *src = reinterpret_cast<u16 *>(&m_buffer[0]) + ((pc - m_lstart) >> 4);
				return u64(src[0]) | (u64(src[1]) << 16) | (u64(src[2]) << 32) | (u64(src[3]) << 48);
			};
			break;
		}
	}

	// Define the data -> string conversion
	switch(shift) {
	case -3:
		if(is_octal)
			m_data_to_string = [this](offs_t pc, offs_t size) {
				std::ostringstream out;
				for(offs_t i=0; i != size; i++) {
					if(i)
						out << ' ';
					util::stream_format(out, "%022o", r64(pc));
					pc = m_next_pc_wrap(pc, 1);
				}
				return out.str();
			};
		else
			m_data_to_string = [this](offs_t pc, offs_t size) {
				std::ostringstream out;
				for(offs_t i=0; i != size; i++) {
					if(i)
						out << ' ';
					util::stream_format(out, "%016X", r64(pc));
					pc = m_next_pc_wrap(pc, 1);
				}
				return out.str();
			};
		break;

	case -2:
		switch(alignment) {
		case 1:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%011o", r32(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%08X", r32(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			break;

		case 2:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
							out << ' ';
						util::stream_format(out, "%022o", r64(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
							out << ' ';
						util::stream_format(out, "%016X", r64(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			break;
		}
		break;

	case -1:
		switch(alignment) {
		case 1:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%06o", r16(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%04X", r16(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			break;

		case 2:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
							out << ' ';
						util::stream_format(out, "%011o", r32(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
						out << ' ';
							util::stream_format(out, "%08X", r32(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			break;

		case 4:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 4) {
						if(i)
							out << ' ';
						util::stream_format(out, "%022o", r64(pc));
						pc = m_next_pc_wrap(pc, 4);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 4) {
						if(i)
							out << ' ';
						util::stream_format(out, "%016X", r64(pc));
						pc = m_next_pc_wrap(pc, 4);
					}
					return out.str();
				};
			break;
		}
		break;

	case 0:
		switch(alignment) {
		case 1:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%03o", r8(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i++) {
						if(i)
							out << ' ';
						util::stream_format(out, "%02X", r8(pc));
						pc = m_next_pc_wrap(pc, 1);
					}
					return out.str();
				};
			break;

		case 2:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
							out << ' ';
						util::stream_format(out, "%06o", r16(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 2) {
						if(i)
							out << ' ';
						util::stream_format(out, "%04X", r16(pc));
						pc = m_next_pc_wrap(pc, 2);
					}
					return out.str();
				};
			break;

		case 4:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 4) {
						if(i)
							out << ' ';
						util::stream_format(out, "%011o", r32(pc));
						pc = m_next_pc_wrap(pc, 4);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 4) {
						if(i)
						out << ' ';
						util::stream_format(out, "%08X", r32(pc));
						pc = m_next_pc_wrap(pc, 4);
					}
					return out.str();
				};
			break;

		case 8:
			if(is_octal)
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 8) {
						if(i)
							out << ' ';
						util::stream_format(out, "%022o", r64(pc));
						pc = m_next_pc_wrap(pc, 8);
					}
					return out.str();
				};
			else
				m_data_to_string = [this](offs_t pc, offs_t size) {
					std::ostringstream out;
					for(offs_t i=0; i != size; i += 8) {
						if(i)
							out << ' ';
						util::stream_format(out, "%016X", r64(pc));
						pc = m_next_pc_wrap(pc, 8);
					}
					return out.str();
				};
			break;
		}
		break;

	case 3:
		if(is_octal)
			m_data_to_string = [this](offs_t pc, offs_t size) {
				std::ostringstream out;
				for(offs_t i=0; i != size; i += 16) {
					if(i)
						out << ' ';
					util::stream_format(out, "%06o", r16(pc));
					pc = m_next_pc_wrap(pc, 16);
				}
				return out.str();
			};
		else
			m_data_to_string = [this](offs_t pc, offs_t size) {
				std::ostringstream out;
				for(offs_t i=0; i != size; i += 16) {
					if(i)
						out << ' ';
					util::stream_format(out, "%04X", r16(pc));
					pc = m_next_pc_wrap(pc, 16);
				}
				return out.str();
			};
		break;
	}

	// Define the data extraction
	switch(shift) {
	case -3:
		m_data_get = [this](offs_t pc, offs_t size, std::vector<u8> &data) {
			for(offs_t i=0; i != size; i++) {
				u64 r = r64(pc);
				for(int j=0; j != 8; j++)
					data.push_back(r >> (8*j));
				pc = m_next_pc_wrap(pc, 1);
			}
		};
		break;

	case -2:
		m_data_get = [this](offs_t pc, offs_t size, std::vector<u8> &data) {
			for(offs_t i=0; i != size; i++) {
				u32 r = r32(pc);
				for(int j=0; j != 4; j++)
					data.push_back(r >> (8*j));
				pc = m_next_pc_wrap(pc, 1);
			}
		};
		break;

	case -1:
		m_data_get = [this](offs_t pc, offs_t size, std::vector<u8> &data) {
			for(offs_t i=0; i != size; i++) {
				u16 r = r16(pc);
				for(int j=0; j != 2; j++)
					data.push_back(r >> (8*j));
				pc = m_next_pc_wrap(pc, 1);
			}
		};
		break;

	case  0:
		m_data_get = [this](offs_t pc, offs_t size, std::vector<u8> &data) {
			for(offs_t i=0; i != size; i++) {
				data.push_back(r8(pc));
				pc = m_next_pc_wrap(pc, 1);
			}
		};
		break;

	case  3:
		m_data_get = [this](offs_t pc, offs_t size, std::vector<u8> &data) {
			for(offs_t i=0; i != size >> 4; i++) {
				u16 r = r16(pc);
				for(int j=0; j != 2; j++)
					data.push_back(r >> (8*j));
				pc = m_next_pc_wrap(pc, 16);
			}
		};
		break;
	}

	// Wrapped next pc computation
	if(m_intf.interface_flags() & util::disasm_interface::NONLINEAR_PC) {
		// lfsr pc is always paged
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			offs_t lpc = m_intf.pc_real_to_linear(pc);
			offs_t lpce = (lpc & ~m_page_mask) | ((lpc + size) & m_page_mask);
			return m_intf.pc_linear_to_real(lpce);
		};
	} else if(m_intf.interface_flags() & util::disasm_interface::PAGED) {
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			offs_t pce = (pc & ~m_page_mask) | ((pc + size) & m_page_mask);
			return pce;
		};
	} else {
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			return (pc + size) & m_pc_mask;
		};
	}
}

debug_disasm_buffer::debug_disasm_buffer(device_t &device) :
	m_dintf(dynamic_cast<device_disasm_interface *>(&device)->get_disassembler()),
	m_mintf(dynamic_cast<device_memory_interface *>(&device)),
	m_buf_raw(dynamic_cast<device_disasm_interface &>(device).get_disassembler()),
	m_buf_opcodes(dynamic_cast<device_disasm_interface &>(device).get_disassembler()),
	m_buf_params(dynamic_cast<device_disasm_interface &>(device).get_disassembler()),
	m_flags(m_dintf.interface_flags())
{
	const address_space_config *pconfig = m_mintf->logical_space_config(AS_PROGRAM);

	if(m_flags & util::disasm_interface::INTERNAL_DECRYPTION) {
		m_buf_raw.set_source(m_mintf, AS_PROGRAM);
		m_buf_opcodes.set_source(m_buf_raw, true);
		if((m_flags & util::disasm_interface::SPLIT_DECRYPTION) == util::disasm_interface::SPLIT_DECRYPTION)
			m_buf_params.set_source(m_buf_raw, false);
	} else {
		if(m_mintf->has_logical_space(AS_OPCODES)) {
			m_buf_opcodes.set_source(m_mintf, AS_OPCODES);
			m_buf_params.set_source(m_mintf, AS_PROGRAM);
		} else
			m_buf_opcodes.set_source(m_mintf, AS_PROGRAM);
	}

	m_pc_mask = pconfig->logaddrmask();

	if(m_flags & util::disasm_interface::PAGED)
		m_page_mask = (1 << m_dintf.page_address_bits()) - 1;
	else
		m_page_mask = 0;

	// Next pc computation
	if(m_flags & util::disasm_interface::NONLINEAR_PC) {
		// lfsr pc is always paged
		m_next_pc = [this](offs_t pc, offs_t size) {
			offs_t lpc = m_dintf.pc_real_to_linear(pc);
			offs_t lpce = lpc + size;
			if((lpc ^ lpce) & ~m_page_mask)
				lpce = (lpc | m_page_mask) + 1;
			lpce &= m_pc_mask;
			return m_dintf.pc_linear_to_real(lpce);
		};
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			offs_t lpc = m_dintf.pc_real_to_linear(pc);
			offs_t lpce = (lpc & ~m_page_mask) | ((lpc + size) & m_page_mask);
			return m_dintf.pc_linear_to_real(lpce);
		};

	} else if(m_flags & util::disasm_interface::PAGED) {
		m_next_pc = [this](offs_t pc, offs_t size) {
			offs_t pce = pc + size;
			if((pc ^ pce) & ~m_page_mask)
				pce = (pc | m_page_mask) + 1;
			pce &= m_pc_mask;
			return pce;
		};
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			offs_t pce = (pc & ~m_page_mask) | ((pc + size) & m_page_mask);
			return pce;
		};

	} else {
		m_next_pc = [this](offs_t pc, offs_t size) {
			return (pc + size) & m_pc_mask;
		};
		m_next_pc_wrap = [this](offs_t pc, offs_t size) {
			return (pc + size) & m_pc_mask;
		};
	}

	// pc to string conversion
	int aw = pconfig->logaddr_width();
	bool is_octal = pconfig->is_octal();
	if((m_flags & util::disasm_interface::PAGED2LEVEL) == util::disasm_interface::PAGED2LEVEL && aw > (m_dintf.page_address_bits() + m_dintf.page2_address_bits())) {
		int bits1 = m_dintf.page_address_bits();
		int bits2 = m_dintf.page2_address_bits();
		int bits3 = aw - bits1 - bits2;
		offs_t sm1 = (1 << bits1) - 1;
		int sh2 = bits1;
		offs_t sm2 = (1 << bits2) - 1;
		int sh3 = bits1+bits2;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			int nc2 = (bits2+2)/3;
			int nc3 = (bits3+2)/3;
			m_pc_to_string = [nc1, nc2, nc3, sm1, sm2, sh2, sh3](offs_t pc) -> std::string {
				return util::string_format("%0*o:%0*o:%0*o",
										   nc3, pc >> sh3,
										   nc2, (pc >> sh2) & sm2,
										   nc1, pc & sm1);
			};
		} else {
			int nc1 = (bits1+3)/4;
			int nc2 = (bits2+3)/4;
			int nc3 = (bits3+3)/4;
			m_pc_to_string = [nc1, nc2, nc3, sm1, sm2, sh2, sh3](offs_t pc) -> std::string {
				return util::string_format("%0*X:%0*X:%0*X",
										   nc3, pc >> sh3,
										   nc2, (pc >> sh2) & sm2,
										   nc1, pc & sm1);
			};
		}

	} else if((m_flags & util::disasm_interface::PAGED) && aw > m_dintf.page_address_bits()) {
		int bits1 = m_dintf.page_address_bits();
		int bits2 = aw - bits1;
		offs_t sm1 = (1 << bits1) - 1;
		int sh2 = bits1;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			int nc2 = (bits2+2)/3;
			m_pc_to_string = [nc1, nc2, sm1, sh2](offs_t pc) -> std::string {
				return util::string_format("%0*o:%0*o",
										   nc2, pc >> sh2,
										   nc1, pc & sm1);
			};
		} else {
			int nc1 = (bits1+3)/4;
			int nc2 = (bits2+3)/4;
			m_pc_to_string = [nc1, nc2, sm1, sh2](offs_t pc) -> std::string {
				return util::string_format("%0*X:%0*X",
										   nc2, pc >> sh2,
										   nc1, pc & sm1);
			};
		}

	} else {
		int bits1 = aw;

		if(is_octal) {
			int nc1 = (bits1+2)/3;
			m_pc_to_string = [nc1](offs_t pc) -> std::string {
				return util::string_format("%0*o",
										   nc1, pc);
			};
		} else {
			int nc1 = (bits1+3)/4;
			m_pc_to_string = [nc1](offs_t pc) -> std::string {
				return util::string_format("%0*X",
										   nc1, pc);
			};
		}
	}
}

void debug_disasm_buffer::disassemble(offs_t pc, std::string &instruction, offs_t &next_pc, offs_t &size, u32 &info) const
{
	std::ostringstream out;
	u32 result = m_dintf.disassemble(out, pc, m_buf_opcodes, m_buf_params.active() ? m_buf_params : m_buf_opcodes);
	instruction = out.str();
	size = result & util::disasm_interface::LENGTHMASK;
	next_pc = m_next_pc(pc, size);
	info = result;
}


u32 debug_disasm_buffer::disassemble_info(offs_t pc) const
{
	std::ostringstream out;
	return m_dintf.disassemble(out, pc, m_buf_opcodes, m_buf_params.active() ? m_buf_params : m_buf_opcodes);
}

std::string debug_disasm_buffer::pc_to_string(offs_t pc) const
{
	return m_pc_to_string(pc);
}

std::string debug_disasm_buffer::data_to_string(offs_t pc, offs_t size, bool opcode) const
{
	if(!opcode && !m_buf_params.active())
		return std::string();
	return (opcode ? m_buf_opcodes : m_buf_params).data_to_string(pc, size);
}

void debug_disasm_buffer::data_get(offs_t pc, offs_t size, bool opcode, std::vector<u8> &data) const
{
	data.clear();
	if(!opcode && !m_buf_params.active())
		return;
	(opcode ? m_buf_opcodes : m_buf_params).data_get(pc, size, data);
}

offs_t debug_disasm_buffer::next_pc(offs_t pc, offs_t step) const
{
	return m_next_pc(pc, step);
}

offs_t debug_disasm_buffer::next_pc_wrap(offs_t pc, offs_t step) const
{
	return m_next_pc_wrap(pc, step);
}
