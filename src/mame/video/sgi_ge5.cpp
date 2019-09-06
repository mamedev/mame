// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics GE5 and HQ1 devices.
 *
 * This board handles the interface to the host, and mainly consists of the HQ1
 * instruction sequencer and the WTL3132 floating-point accelerator. The board
 * also contains instruction and data RAM for the HQ1, and a FIFO for host
 * communication.
 *
 * The undocumented HQ1 microcode instruction format is relatively well decoded
 * now, but the exact timing and function of many operations remains incomplete.
 *
 * TODO:
 *   - skeleton only
 *
 */

#include "emu.h"
#include "debugger.h"
#include "sgi_ge5.h"

#define LOG_GENERAL   (1U << 0)

//#define VERBOSE       (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_GE5, sgi_ge5_device, "ge5", "SGI Geometry Engine 5")

sgi_ge5_device::sgi_ge5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, SGI_GE5, tag, owner, clock)
	, m_code_config("code", ENDIANNESS_BIG, 64, 15, -3, address_map_constructor(FUNC(sgi_ge5_device::code_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 32, 13, -2, address_map_constructor(FUNC(sgi_ge5_device::data_map), this))
	, m_fpu(*this, "fpu")
	, m_int_cb(*this)
	, m_fifo_empty(*this)
	, m_fifo_read(*this)
	, m_re_r(*this)
	, m_re_w(*this)
	, m_icount(0)
{
	(void)m_dma_count;
}

void sgi_ge5_device::device_add_mconfig(machine_config &config)
{
	WTL3132(config, m_fpu, clock());
	m_fpu->out_fpcn().set([this](int state) { m_fpu_c = state; });
	m_fpu->out_zero().set([this](int state) { m_fpu_z = state; });
	m_fpu->out_port_x().set([this](u32 data) { m_bus = data; LOG("m_bus = %x\n", data); });
}


void sgi_ge5_device::code_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("code");
}

void sgi_ge5_device::data_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("data");
}

void sgi_ge5_device::device_start()
{
	m_int_cb.resolve_safe();
	m_fifo_empty.resolve();
	m_fifo_read.resolve();
	m_re_r.resolve();
	m_re_w.resolve();

	// TODO: save state
	state_add(STATE_GENPC,     "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(0, "PC", m_pc).formatstr("%04X");

	state_add(1, "MEMPTR", m_memptr).formatstr("%04X");
	state_add(2, "REPTR",  m_reptr).formatstr("%04X");

	set_icountptr(m_icount);
}

void sgi_ge5_device::device_reset()
{
	m_reptr = 0;
	m_memptr = 0;

	m_sp = 0;

	if (m_int_state)
	{
		m_int_state = 0;
		m_int_cb(m_int_state);
	}

	suspend(SUSPEND_REASON_HALT, false);
	LOG("stalled\n");
}

device_memory_interface::space_config_vector sgi_ge5_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_code_config),
		std::make_pair(AS_DATA,    &m_data_config),
	};
}

std::unique_ptr<util::disasm_interface> sgi_ge5_device::create_disassembler()
{
	return std::make_unique<sgi_ge5_disassembler>();
}

enum cx_mask : u8
{
	INCMEM = 0x08, // increment memptr
	FIELD2 = 0x10, // second field active
	INCRE  = 0x20, // increment reptr
	SRC    = 0xc0, // data bus source
	DST    = 0x06, // data bus source
	STALL  = 0x01,
};

void sgi_ge5_device::execute_run()
{
	if (m_fetch)
	{
		// stall if fifo empty
		if (m_fifo_empty())
		{
			suspend(SUSPEND_REASON_HALT, false);
			return;
		}

		// fetch from fifo and set pc
		u64 data = m_fifo_read();
		m_bus = u32(data);
		m_pc = (data >> 31) & 0x1fe;
		m_fetch = false;
	}

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		u64 const insn = space(AS_PROGRAM).read_qword(m_pc);
		LOG("pc 0x%04x code 0x%011x\n", m_pc, insn);

		u8 const hq1_op = (insn >> 32) & 0xff;
		unsigned const src = (hq1_op & SRC) >> 6;
		unsigned const dst = (hq1_op & DST) >> 1;
		bool const stall = (hq1_op & STALL);
		if (hq1_op & FIELD2)
			LOG("hq1 op 0x%02x src %d dst %d stall %d branch %d\n", hq1_op, src, dst, stall, (insn >> 29) & 7);
		else
			LOG("hq1 op 0x%02x src %d dst %d stall %d\n", hq1_op, src, dst, stall);

		// stall when fifo empty and reading from fifo or stall flag
		if (((src == 1) || stall) && m_fifo_empty())
		{
			// enter fetch mode if destination is fetch
			m_fetch = (dst == 1);
			LOG(m_fetch ? "FETCH\n" : "STALL\n");
			suspend(SUSPEND_REASON_HALT, false);
			m_icount = 0;
			continue;
		}

		u16 branch = 0;
		if (hq1_op & FIELD2)
		{
			// handle field2
			u64 const field2 = space(AS_PROGRAM).read_qword(++m_pc);
			u16 const immediate = (field2 >> 19) & 0x7fff;

			LOG("pc 0x%04x pref 0x%011x immediate 0x%04x\n", m_pc, field2, immediate);

			switch ((field2 >> 32) & 0xfc)
			{
			case 0x8c:
				m_reptr = m_bus;
				LOG("REPTR=0x%04x\n", m_reptr);
				break;
			case 0x9c:
				m_reptr = immediate;
				LOG("REPTR=0x%04x\n", m_reptr);
				break;
			case 0xb0:
				m_memptr = m_bus;
				branch = immediate;
				LOG("MEMPTR=0x%04x BRANCH=0x%04x\n", m_memptr, branch);
				break;
			case 0xb4:
				m_memptr = immediate;
				LOG("MEMPTR=0x%04x\n", m_memptr);
				break;
			case 0xb8:
				m_memptr_temp = immediate;
				LOG("MEMPTR_TMP=0x%04x\n", m_memptr_temp);
				break;
			case 0xbc:
				branch = immediate;
				LOG("BRANCH=0x%04x\n", branch);
				break;
			case 0xfc: // also clear interrupt?
				if (!m_int_state)
				{
					LOG("ge interrupt asserted\n");
					m_int_state = 1;
					m_int_cb(m_int_state);
				}
				break;
			//case 0xfe: // set dma count?
			default:
				LOG("unknown pref 0x%02x\n", (field2 >> 32) & 0xfc);
				break;
			}
		}

		// increment memptr
		if (hq1_op & INCMEM)
		{
			m_memptr++;
			LOG("MEMPTR++\n");
		}

		// increment reptr
		if (hq1_op & INCRE)
		{
			m_reptr++;
			LOG("REPTR++\n");
		}

		// xx          - source (re, fifo, ram, fpu)
		//   x         - increment reptr
		//    x        - field2
		//     x       - increment memptr
		//      xx     - destination (re, fetch, ram, fpu)
		//        x    - stall? (if fifo empty)
		//         xxx - branch

		// 00xx xxxx -> ?
		// 01xx xxxx -> read fifo
		// 10xx xxxx -> read ram
		// 11xx xxxx -> read fpu

		u64 fpu_ctrl =
			(m_cwen ? wtl3132_device::M_CWEN : 0) |
			(2ULL << wtl3132_device::S_ENCN);

		// 43 - get fifo tag
		// 46 - get fifo data
		// 83 - stall?
		// generally reading the fifo causes stalls
		// 83 also causes a stall (because can't read code from data ram?

		switch (src)
		{
		case 0:
			m_bus = m_re_r(m_reptr);
			LOG("LOAD RE offset 0x%08x data 0x%08x\n", m_reptr, m_bus);
			break;
		case 1:
			m_bus = m_fifo_read();
			LOG("LOAD FIFO 0x%08x\n", m_bus);
			break;
		case 2:
			m_bus = space(AS_DATA).read_dword(m_memptr);
			LOG("LOAD MEM offset 0x%04x data 0x%08x\n", m_memptr, m_bus);
			break;
		case 3:
			// tells this instruction to write to the bus in two cycles from now
			LOG("FSTORE\n");
			fpu_ctrl |= (2ULL << wtl3132_device::S_IOCT);
			break;
		}

		// i/o dst
		switch (dst)
		{
		case 0:
			LOG("STORE RE offset 0x%02x data 0x%08x\n", m_reptr, m_bus);
			m_re_w(m_reptr, m_bus);
			break;
		case 1: // fetch
			if (stall)
			{
				m_pc = (m_bus >> 31) & 0x1fe;
				m_icount--;
				// skip pc update and fpu step?
				continue;
			}
			break;
		case 2:
			space(AS_DATA).write_dword(m_memptr, m_bus);
			LOG("STORE MEM offset 0x%02x data 0x%08x\n", m_memptr, m_bus);
			break;
		case 3:
			m_fpu->x_port_w(m_bus);
			fpu_ctrl |= (3ULL << wtl3132_device::S_IOCT);
			LOG("FLOAD 0x%08x\n", m_bus);
			break;
		}

		// hq1 branch
		if (hq1_op & FIELD2)
		{
			switch ((insn >> 29) & 7)
			{
			case 0: m_pc++; break;
			case 1:
				m_pc = branch;
				LOG("J 0x%04x\n", m_pc);
				break;
			case 2:
				if (m_fpu_c)
				{
					m_pc = branch;
					LOG("JC 0x%04x\n", m_pc);
				}
				break;
			case 3:
				if (!m_fpu_c)
				{
					m_pc = branch;
					LOG("JNC 0x%04x\n", m_pc);
				}
				break;
			case 4:
				m_stack[m_sp] = m_pc + 1;
				m_sp = (m_sp + 1) & 7;
				m_pc = branch;
				LOG("CALL 0x%04x\n", m_pc);
				break;
			case 5:
				if (m_fpu_z)
				{
					m_pc = branch;
					LOG("JZ 0x%04x\n", m_pc);
				}
				break;
			case 6:
				if (!m_fpu_z)
				{
					m_pc = branch;
					LOG("JNZ 0x%04x\n", m_pc);
				}
				break;
			case 7:
				m_sp = (m_sp + 7) & 7;
				m_pc = m_stack[m_sp];
				LOG("RET 0x%04x\n", m_pc);
				break;
			}
		}
		else
			m_pc++;

		// step fpu
		u64 const fpu = ((insn & 0x1fff'f800ULL) << 5) | ((insn & 0x0000'07ffULL) << 2);

		m_fpu->c_port_w(fpu | fpu_ctrl);
		m_fpu->clk_w(1);

		m_icount--;
	}
}

void sgi_ge5_device::command_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
	case 0x00: // clear stall
		//resume(SUSPEND_REASON_HALT);
		LOG("unstalled\n");
		break;

	case 0x10: // setss
	case 0x20: // clearss
	case 0x30: // executess
		break;

	case 0x50: // clearintr
		if (m_int_state)
		{
			LOG("ge interrupt cleared\n");
			m_int_state = 0;
			m_int_cb(m_int_state);
		}
		break;
	}
}

u32 sgi_ge5_device::code_r(offs_t offset)
{
	m_pc = offset | offs_t(m_mar & 0x7f) << 8;
	u64 const data = space(AS_PROGRAM).read_qword(m_pc);

	return m_mar_msb ? u32(data >> 32) : u32(data);
}

void sgi_ge5_device::code_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_pc = offset | offs_t(m_mar & 0x7f) << 8;

	LOG("code_w msb %d offset 0x%08x data 0x%08x mask 0x%08x (%s)\n", m_mar_msb, m_pc, data, mem_mask, machine().describe_context());

	if (m_mar_msb)
	{
		u64 const mask = u64(mem_mask & 0x000000ffU) << 32;

		space(AS_PROGRAM).write_qword(m_pc, u64(data) << 32, mask);
	}
	else
		space(AS_PROGRAM).write_qword(m_pc, data, mem_mask);
}

u32 sgi_ge5_device::data_r(offs_t offset)
{
	m_memptr = offset | offs_t(m_mar & 0x1f) << 8;

	return space(AS_DATA).read_dword(m_memptr);
}

void sgi_ge5_device::data_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_memptr = offset | offs_t(m_mar & 0x1f) << 8;

	LOG("data_w offset 0x%08x data 0x%08x mask 0x%08x (%s)\n", m_memptr, data, mem_mask, machine().describe_context());

	space(AS_DATA).write_dword(m_memptr, data, mem_mask);
}

offs_t sgi_ge5_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	std::string prefix = "";
	std::string src, dst;
	u16 branch;

	u64 const insn = opcodes.r64(pc);

	if (BIT(insn, 36))
	{
		u64 const insn_prefix = opcodes.r64(pc + 1);
		u16 const immediate = (insn_prefix >> 19) & 0x7fff;

		switch ((insn_prefix >> 32) & 0xfc)
		{
		case 0x8c: prefix = std::string("LOAD REPTR"); break;
		case 0x9c: prefix = util::string_format("REPTR=%04x", immediate); break;
		case 0xb0: prefix = std::string("LOAD MEMPTR"); branch = immediate; break;
		case 0xb4: prefix = util::string_format("MEMPTR=%04x", immediate); break;
		case 0xb8: prefix = util::string_format("MEMPTR_TEMP=%04x", immediate); break;
		case 0xbc: branch = immediate; break;
		case 0xfc: prefix = std::string("SET_INT"); break;
		//case 0xfe: // set dma count?
		}
	}

	u8 const hq1_op = (insn >> 32) & 0xff;

	// pre-increment memptr
	//if (BIT(hq1_op, 3))
	//  ;

	// pre-increment reptr
	//if (BIT(hq1_op, 5))
	//  ;

	// xx       - source (?, fifo, ram, fpu)
	//   x      - increment reptr
	//    x     - ?
	//     x    - increment memptr
	//      xxx - destination (re, ?, bus, stall?, ram, ?, fpu, ?)

	// 00xx xxxx -> ?
	// 01xx xxxx -> read fifo
	// 10xx xxxx -> read ram
	// 11xx xxxx -> read fpu

	u64 fpu_ctrl = 0x2'8000'0000; // ENCN=2, IOCT=2

	switch (hq1_op >> 6)
	{
	case 0: src = std::string("0?"); break;
	case 1: src = std::string("fifo"); break;
	case 2: src = std::string("bus"); break;
	case 3: src = std::string("fpu"); break; // IOCT=2
	}

	// i/o dst
	switch (hq1_op & 7)
	{
	case 0: dst = std::string("REPTR"); break;
	case 1: dst = std::string("1?"); break;
	case 2: dst = std::string("bus?"); break;
	case 3: dst = std::string("stall?"); break;
	case 4: dst = std::string("MEMPTR"); break;
	case 5: dst = std::string("5?"); break;
	case 6: dst = std::string("FPU"); fpu_ctrl |= 0x4000'0000; break; // fpu IOCT=3
	case 7: dst = std::string("7?"); break;
	}

	std::string fpu = wtl3132_device::disassemble(
		bitswap<34>((insn & 0x0fff'ffff) | fpu_ctrl,
			28, 27, 26,          // f
			25, 24, 23, 22, 21,  // aadd
			20, 19, 18, 17, 16,  // badd
			15, 14, 13, 12, 11,  // cadd
			29,                  // cwen*
			31, 30,              // ioct*
			10, 9, 8, 7, 6,      // dadd
			5, 4, 3,             // abin
			2, 1,                // adst
			0,                   // mbin
			33, 32));            // encn*

	// hq1 branch
	switch ((insn >> 29) & 7)
	{
	case 0: util::stream_format(stream, "%s; %s; R:%s, W:%s", prefix, fpu, src, dst); break;
	case 1: util::stream_format(stream, "%s; %s; R:%s, W:%s; J:%04x", prefix, fpu, src, dst, branch); break;
	case 2: util::stream_format(stream, "%s; %s; R:%s, W:%s; JC:%04x", prefix, fpu, src, dst, branch); break;
	case 3: util::stream_format(stream, "%s; %s; R:%s, W:%s; JNC:%04x", prefix, fpu, src, dst, branch); break;
	case 4: util::stream_format(stream, "%s; %s; R:%s, W:%s; CALL:%04x", prefix, fpu, src, dst, branch); break;
	case 5: util::stream_format(stream, "%s; %s; R:%s, W:%s; JZ:%04x", prefix, fpu, src, dst, branch); break;
	case 6: util::stream_format(stream, "%s; %s; R:%s, W:%s; JNZ:%04x", prefix, fpu, src, dst, branch); break;
	case 7: util::stream_format(stream, "%s; %s; R:%s, W:%s; RET", prefix, fpu, src, dst); break;
	}

	return BIT(insn, 36) ? 2 : 1;
}
