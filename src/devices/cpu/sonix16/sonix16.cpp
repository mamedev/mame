// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Sonix 16-bit DSP emulation (preliminary)

    Instruction timings are undocumented, and the timings currently used
    are probably very far from accurate.

    No variant-specific features are emulated yet.

***************************************************************************/

#include "emu.h"
#include "sonix16.h"
#include "sonix16d.h"

// device type definitions
DEFINE_DEVICE_TYPE(SNL320, snl320_device, "snl320", "Sonix SNL320")
DEFINE_DEVICE_TYPE(SNC7001A, snc7001a_device, "snc7001a", "Sonix SNC7001A")
DEFINE_DEVICE_TYPE(SNC7648S, snc7648s_device, "snc7648s", "Sonix SNC7648S")
DEFINE_DEVICE_TYPE(SNT110, snt110_device, "snt110", "Sonix SNT110")

sonix16_device::sonix16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_rom_config("rom", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_ram_config("ram", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 7, -1, address_map_constructor(FUNC(sonix16_device::io_map), this))
	, m_mr(0)
	, m_sp(0)
	, m_iosw(0)
	, m_icount(0)
{
	std::fill(std::begin(m_r), std::end(m_r), 0);
	std::fill(std::begin(m_ir), std::end(m_ir), 0);
	std::fill(std::begin(m_ixbk), std::end(m_ixbk), 0);
	std::fill(std::begin(m_ixbkram), std::end(m_ixbkram), 0);
}

snl320_device::snl320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sonix16_device(mconfig, SNL320, tag, owner, clock)
{
}

snc7001a_device::snc7001a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sonix16_device(mconfig, SNC7001A, tag, owner, clock)
{
}

snc7648s_device::snc7648s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sonix16_device(mconfig, SNC7648S, tag, owner, clock)
{
}

snt110_device::snt110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sonix16_device(mconfig, SNT110, tag, owner, clock)
{
}

std::unique_ptr<util::disasm_interface> sonix16_device::create_disassembler()
{
	return std::make_unique<sonix16_disassembler>();
}

device_memory_interface::space_config_vector sonix16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_rom_config),
		std::make_pair(AS_DATA, &m_ram_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

u16 sonix16_device::ssf_r()
{
	return m_ssf;
}

void sonix16_device::ssf_w(u16 data)
{
	// System status flag
	m_ssf = data & 0x003f;
}

u16 sonix16_device::ix_r(offs_t offset)
{
	return m_ir[offset];
}

void sonix16_device::ix_w(offs_t offset, u16 data)
{
	m_ir[offset] = data;
}

u16 sonix16_device::iy_r(offs_t offset)
{
	return m_ir[offset + 2];
}

void sonix16_device::iy_w(offs_t offset, u16 data)
{
	m_ir[offset + 2] = data;
}

u16 sonix16_device::rambk_r()
{
	return m_rambk;
}

void sonix16_device::rambk_w(u16 data)
{
	m_rambk = data & 0x00fe;
}

u16 sonix16_device::pch_r()
{
	return m_pc >> 16;
}

void sonix16_device::pch_w(u16 data)
{
	// Program counter high word
	m_pc = u32(data & 0x00ff) << 16 | (m_pc & 0x00ffff);
}

u16 sonix16_device::pcl_r()
{
	return m_pc & 0x00ffff;
}

void sonix16_device::pcl_w(u16 data)
{
	// Program counter low word
	m_pc = (m_pc & 0xff0000) | data;
}

u16 sonix16_device::sp_r()
{
	return m_sp;
}

void sonix16_device::sp_w(u16 data)
{
	m_sp = data;
}

u16 sonix16_device::mr2_r()
{
	return s16(s8(BIT(m_mr, 32, 8)));
}

void sonix16_device::mr2_w(u16 data)
{
	m_mr = u64(data & 0x00ff) << 32 | (m_mr & 0x00ffffffff);
}

u16 sonix16_device::ixbk_r(offs_t offset)
{
	return m_ixbk[offset];
}

void sonix16_device::ixbk_w(offs_t offset, u16 data)
{
	m_ixbk[offset] = data & 0x00ff;
}

u16 sonix16_device::ixbkram_r(offs_t offset)
{
	return m_ixbkram[offset];
}

void sonix16_device::ixbkram_w(offs_t offset, u16 data)
{
	m_ixbkram[offset] = data & 0x00ff;
}

u16 sonix16_device::iybk_r(offs_t offset)
{
	return m_ixbk[offset + 2];
}

void sonix16_device::iybk_w(offs_t offset, u16 data)
{
	m_ixbk[offset + 2] = data & 0x00ff;
}

u16 sonix16_device::inten_r()
{
	return m_inten;
}

void sonix16_device::inten_w(u16 data)
{
	// Interrupt enable
	m_inten = data;
}

u16 sonix16_device::iosw_r()
{
	return m_iosw;
}

void sonix16_device::iosw_w(u16 data)
{
	// I/O byte swap
	m_iosw = swapendian_int16(data);
}

void sonix16_device::io_map(address_map &map)
{
	// TODO: may need different maps for variants
	map(0x00, 0x00).rw(FUNC(sonix16_device::ssf_r), FUNC(sonix16_device::ssf_w));
	map(0x02, 0x03).rw(FUNC(sonix16_device::ix_r), FUNC(sonix16_device::ix_w));
	map(0x0d, 0x0d).rw(FUNC(sonix16_device::rambk_r), FUNC(sonix16_device::rambk_w));
	map(0x0e, 0x0f).rw(FUNC(sonix16_device::ixbk_r), FUNC(sonix16_device::ixbk_w));
	map(0x13, 0x14).rw(FUNC(sonix16_device::iy_r), FUNC(sonix16_device::iy_w));
	map(0x15, 0x15).rw(FUNC(sonix16_device::pch_r), FUNC(sonix16_device::pch_w));
	map(0x16, 0x16).rw(FUNC(sonix16_device::pcl_r), FUNC(sonix16_device::pcl_w));
	map(0x18, 0x18).rw(FUNC(sonix16_device::sp_r), FUNC(sonix16_device::sp_w));
	map(0x19, 0x19).rw(FUNC(sonix16_device::mr2_r), FUNC(sonix16_device::mr2_w));
	map(0x1a, 0x1b).rw(FUNC(sonix16_device::iybk_r), FUNC(sonix16_device::iybk_w));
	map(0x1c, 0x1d).rw(FUNC(sonix16_device::ixbkram_r), FUNC(sonix16_device::ixbkram_w));
	map(0x20, 0x20).rw(FUNC(sonix16_device::inten_r), FUNC(sonix16_device::inten_w));
	map(0x3a, 0x3a).rw(FUNC(sonix16_device::iosw_r), FUNC(sonix16_device::iosw_w));
}

void sonix16_device::device_start()
{
	space(AS_PROGRAM).specific(m_rom_space);
	space(AS_DATA).specific(m_ram_space);
	space(AS_IO).specific(m_io_space);
	space(AS_PROGRAM).cache(m_cache);

	set_icountptr(m_icount);

	using namespace std::placeholders;
	state_add(SONIX16_PC, "PC", m_pc).mask(0xffffff);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(0xffffff).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(0xffffff).noshow();
	state_add(SONIX16_X0, "X0", m_r[0]);
	state_add(SONIX16_X1, "X1", m_r[1]);
	state_add(SONIX16_R0, "R0", m_r[2]);
	state_add(SONIX16_R1, "R1", m_r[3]);
	state_add(SONIX16_Y0, "Y0", m_r[4]);
	state_add(SONIX16_Y1, "Y1", m_r[5]);
	state_add(SONIX16_IX0, "Ix0", m_ir[0]);
	state_add(SONIX16_IX1, "Ix1", m_ir[1]);
	state_add(SONIX16_IY0, "Iy0", m_ir[2]);
	state_add(SONIX16_IY1, "Iy1", m_ir[3]);
	state_add(SONIX16_MR, "MR", m_mr).mask(0xffffffffff);
	state_add<u16>(SONIX16_MR0, "MR0", std::bind(&sonix16_device::get_reg, this, 6), std::bind(&sonix16_device::set_reg, this, 6, _1)).noshow();
	state_add<u16>(SONIX16_MR1, "MR1", std::bind(&sonix16_device::get_reg, this, 7), std::bind(&sonix16_device::set_reg, this, 7, _1)).noshow();
	state_add<u16>(SONIX16_MR2, "MR2", std::bind(&sonix16_device::mr2_r, this), std::bind(&sonix16_device::mr2_w, this, _1)).mask(0x00ff).noshow();
	state_add(SONIX16_SSF, "SSF", m_ssf).mask(0x3f);
	state_add(STATE_GENFLAGS, "CURFLAGS", m_ssf).mask(0x3f).formatstr("%6s").noshow();
	state_add(SONIX16_RAMBK, "RAMBk", m_rambk).mask(0xfe);
	state_add(SONIX16_IX0BK, "Ix0BK", m_ixbk[0]);
	state_add(SONIX16_IX1BK, "Ix1BK", m_ixbk[1]);
	state_add(SONIX16_IY0BK, "Iy0BK", m_ixbk[2]);
	state_add(SONIX16_IY1BK, "Iy1BK", m_ixbk[3]);
	state_add(SONIX16_IX0BKRAM, "Ix0BKRAM", m_ixbkram[0]);
	state_add(SONIX16_IX1BKRAM, "Ix1BKRAM", m_ixbkram[1]);
	state_add(SONIX16_SP, "SP", m_sp);
	state_add(SONIX16_INTEN, "INTEN", m_inten);

	save_item(NAME(m_r));
	save_item(NAME(m_ir));
	save_item(NAME(m_mr));
	save_item(NAME(m_rambk));
	save_item(NAME(m_ixbk));
	save_item(NAME(m_ixbkram));
	save_item(NAME(m_sp));
	save_item(NAME(m_inten));
	save_item(NAME(m_iosw));
	save_item(NAME(m_pc));
}

void sonix16_device::device_reset()
{
	m_pc = 0;

	m_ssf = 0;
	m_rambk = 0;
	m_inten = 0;
}

u16 sonix16_device::get_reg(unsigned r) const noexcept
{
	if (r < 6)
		return m_r[r];
	else
		return BIT(m_mr, r == 6 ? 0 : 16, 16);
}

void sonix16_device::set_reg(unsigned r, u16 v) noexcept
{
	if (r < 6)
		m_r[r] = v;
	else if (r == 6)
		m_mr = (m_mr & 0xffffff0000) | v;
	else
		m_mr = (m_mr & 0xff0000ffff) | u64(v) << 16;
}

u16 sonix16_device::add(u16 xop, u16 yop, bool cin) noexcept
{
	u32 r = u32(xop) + yop + (cin ? 1 : 0);
	m_ssf = (m_ssf & 0x30)
			| (BIT((r ^ xop) & ~(xop ^ yop), 15) ? 0x08 : 0)
			| (BIT(r, 16) ? 0x04 : 0)
			| (BIT(r, 15) ? 0x02 : 0)
			| (u16(r) ? 0 : 0x01);
	return u16(r);
}

void sonix16_device::execute_run()
{
	do
	{
		debugger_instruction_hook(m_pc);
		u16 inst = m_cache.read_word(m_pc++);

		if ((inst & 0x8000) == 0)
		{
			logerror("Unhandled CALL encountered (0x%04X, PC = 0x%06X)\n", inst, m_pc - 1);
			m_icount--;
		}
		else if ((inst & 0xf000) == 0x8000)
		{
			m_pc = (m_pc + util::sext(inst, 12)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xfe00) == 0x9000)
		{
			// jeq/jz, jne/jnz
			if (BIT(m_ssf, 0) != BIT(inst, 8))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xff00) == 0x9200)
		{
			// jgt
			if (!BIT(m_ssf, 0) && (BIT(m_ssf, 1) == BIT(m_ssf, 3)))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xff00) == 0x9300)
		{
			// jge
			if (BIT(m_ssf, 1) == BIT(m_ssf, 3))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xff00) == 0x9400)
		{
			// jlt
			if (BIT(m_ssf, 1) != BIT(m_ssf, 3))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xff00) == 0x9500)
		{
			// jle
			if (BIT(m_ssf, 0) || (BIT(m_ssf, 1) != BIT(m_ssf, 3)))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xfe00) == 0x9600)
		{
			// jav, jnav
			if (BIT(m_ssf, 3) != BIT(inst, 8))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xfe00) == 0x9800)
		{
			// jac, jnac
			if (BIT(m_ssf, 2) != BIT(inst, 8))
				m_pc = (m_pc + util::sext(inst, 8)) & 0xffffff;
			m_icount--;
		}
		else if ((inst & 0xf000) == 0xa000)
		{
			m_ram_space.write_word(u16(m_rambk) << 8 | (BIT(inst, 11) ? 0x100 : 0) | BIT(inst, 0, 8), get_reg(BIT(inst, 8, 3)));
			m_icount--;
		}
		else if ((inst & 0xf000) == 0xb000)
		{
			set_reg(BIT(inst, 8, 3), m_ram_space.read_word(u16(m_rambk) << 8 | (BIT(inst, 11) ? 0x100 : 0) | BIT(inst, 0, 8)));
			m_icount--;
		}
		else if ((inst & 0xf800) == 0xc000)
		{
			unsigned r = BIT(inst, 8, 3);
			if (r < 6)
				m_r[r] = ((inst << 8) & 0xff00) | (m_r[r] & 0x00ff);
			else
				m_ir[r - 6] = ((inst << 8) & 0xff00) | (m_ir[r - 6] & 0x00ff);
			m_icount--;
		}
		else if ((inst & 0xf880) == 0xc800)
		{
			// AU(2)
			u32 addr = u32(m_ixbkram[BIT(inst, 8)]) << 16 | m_ir[BIT(inst, 8)];
			u16 rop = m_r[BIT(inst, 5, 2)];
			u16 yop = BIT(inst, 3, 2) == 0 ? 1 : m_r[BIT(inst, 0, 2) + (BIT(inst, 1) ? 0 : 4)];
			switch (BIT(inst, 2, 3))
			{
			case 0: case 2:
				rop = add(rop, yop, false);
				break;

			case 1: case 4:
				rop = add(rop, ~yop, true);
				break;

			case 3:
				rop = add(rop, yop, BIT(m_ssf, 2));
				break;

			case 5:
				rop = add(rop, ~yop, BIT(m_ssf, 2));
				break;

			case 6:
				rop = add(~rop, yop, true);
				break;

			case 7:
				rop = add(~rop, yop, BIT(m_ssf, 2));
				break;
			}
			m_ram_space.write_word(addr, rop);
			switch (BIT(inst, 9, 2))
			{
			case 0:
				break;

			case 1:
				logerror("Unimplemented Ix modification encountered (PC = 0x%06X)\n", m_pc - 1);
				break;

			case 2:
				m_ir[BIT(inst, 8)] = (addr + 1) & 0xffff;
				break;

			case 3:
				m_ir[BIT(inst, 8)] = (addr - 1) & 0xffff;
				break;
			}
			m_icount--;
		}
		else if ((inst & 0xf884) == 0xc880)
		{
			// LU1
			u16 op = m_r[BIT(inst, 0, 2) + (BIT(inst, 1) ? 0 : 4)];
			switch (BIT(inst, 3, 2))
			{
			case 0: // AND
				op &= m_r[BIT(inst, 5, 2)];
				break;

			case 1: // OR
				op |= m_r[BIT(inst, 5, 2)];
				break;

			case 2: // XOR
				op ^= m_r[BIT(inst, 5, 2)];
				break;

			case 3: // NOT
				op = ~op;
				break;
			}
			m_ssf = (m_ssf & 0x30) | (BIT(op, 15) ? 0x02 : 0) | (op == 0 ? 0x01 : 0);
			set_reg(BIT(inst, 8, 3), op);
			m_icount--;
		}
		else if ((inst & 0xf884) == 0xc884)
		{
			// LU2
			unsigned bit = BIT(inst, 9, 2) << 2 | BIT(inst, 5, 2);
			u16 op = m_r[BIT(inst, 0, 2) + (BIT(inst, 1) ? 0 : 4)];
			switch (BIT(inst, 3, 2))
			{
			case 0: // BSET
				op |= 1 << bit;
				break;

			case 1: // BCLR
				op &= ~(1 << bit);
				break;

			case 2: // BTOG
				op ^= 1 << bit;
				break;

			case 3: // BTST
				op = BIT(op, bit);
				break;
			}
			m_ssf = (m_ssf & 0x30) | (BIT(op, 15) ? 0x02 : 0) | (op == 0 ? 0x01 : 0);
			m_r[BIT(inst, 8) + 2] = op;
			m_icount--;
		}
		else if ((inst & 0xf000) == 0xd000)
		{
			unsigned r = BIT(inst, 8, 3);
			if (r < 6)
				m_r[r] = (BIT(inst, 11) ? 0 : m_r[r] & 0xff00) | (inst & 0x00ff);
			else
				m_ir[r - 6] = (BIT(inst, 11) ? 0 : m_ir[r - 6] & 0xff00) | (inst & 0x00ff);
			m_icount--;
		}
		else if ((inst & 0xf883) == 0xe000)
		{
			u32 addr = u32(m_ixbkram[BIT(inst, 2)]) << 16 | m_ir[BIT(inst, 2, 2)];
			if (BIT(inst, 6))
				set_reg(BIT(inst, 8, 3), m_ram_space.read_word(addr));
			else
				m_ram_space.write_word(addr, get_reg(BIT(inst, 8, 3)));
			switch (BIT(inst, 4, 2))
			{
			case 0:
				break;

			case 1:
				logerror("Unimplemented Ix modification encountered (PC = 0x%06X)\n", m_pc - 1);
				break;

			case 2:
				m_ir[BIT(inst, 2, 2)] = (addr + 1) & 0xffff;
				break;

			case 3:
				m_ir[BIT(inst, 2, 2)] = (addr - 1) & 0xffff;
				break;
			}
			m_icount--;
		}
		else if ((inst & 0xf8c3) == 0xe041)
		{
			u32 addr = u32(m_ixbk[BIT(inst, 2, 2)]) << 16 | m_ir[BIT(inst, 2, 2)];
			set_reg(BIT(inst, 8, 3), m_rom_space.read_word(addr));
			switch (BIT(inst, 4, 2))
			{
			case 0:
				break;

			case 1:
				logerror("Unimplemented Ix modification encountered (PC = 0x%06X)\n", m_pc - 1);
				break;

			case 2:
				m_ir[BIT(inst, 2, 2)] = (addr + 1) & 0xffff;
				break;

			case 3:
				m_ir[BIT(inst, 2, 2)] = (addr - 1) & 0xffff;
				break;
			}
			m_icount--;
		}
		else if ((inst & 0xfc80) == 0xe080)
		{
			m_io_space.write_word(BIT(inst, 0, 7), m_r[BIT(inst, 8, 2)]); // may change PC
			m_icount--;
		}
		else if ((inst & 0xfc80) == 0xe480)
		{
			m_r[BIT(inst, 8, 2)] = m_io_space.read_word(BIT(inst, 0, 7));
			m_icount--;
		}
		else if ((inst & 0xf800) == 0xe800)
		{
			// AU(1)
			u16 rop = get_reg(BIT(inst, 5, 3));
			u16 yop = BIT(inst, 3, 2) == 0 ? 1 << BIT(inst, 0, 2) : m_r[BIT(inst, 0, 2) + (BIT(inst, 1) ? 0 : 4)];
			switch (BIT(inst, 2, 3))
			{
			case 0: case 2:
				rop = add(rop, yop, false);
				break;

			case 1: case 4:
				rop = add(rop, ~yop, true);
				break;

			case 3:
				rop = add(rop, yop, BIT(m_ssf, 2));
				break;

			case 5:
				rop = add(rop, ~yop, BIT(m_ssf, 2));
				break;

			case 6:
				rop = add(~rop, yop, true);
				break;

			case 7:
				rop = add(~rop, yop, BIT(m_ssf, 2));
				break;
			}
			set_reg(BIT(inst, 8, 3), rop);
			m_icount--;
		}
		else if ((inst & 0xfffc) == 0xf000)
		{
			// TODO: MAC flags
			m_mr = s64(s32(s16(m_r[BIT(inst, 1)])) * s16(m_r[BIT(inst, 0) + 4])) & 0xffffffffff;
			m_icount--;
		}
		else if ((inst & 0xff03) == 0xf800)
		{
			set_reg(BIT(inst, 2, 3), get_reg(BIT(inst, 5, 3)));
			m_icount--;
		}
		else if ((inst & 0xff1f) == 0xf802)
		{
			// push R
			m_ram_space.write_word(m_sp, get_reg(BIT(inst, 5, 3)));
			m_sp++;
			m_icount--;
		}
		else if ((inst & 0xff1f) == 0xf803)
		{
			// pop R
			m_sp--;
			set_reg(BIT(inst, 5, 3), m_ram_space.read_word(m_sp));
			m_icount--;
		}
		else if ((inst & 0xfe18) == 0xfa08)
		{
			// SL
			u16 op = get_reg(BIT(inst, 5, 3));
			u16 r = op << (BIT(inst, 0, 3) + 1);
			// TODO: arithmetic overflow flag
			m_ssf = (m_ssf & 0x30) | (BIT(op, 15 - BIT(inst, 0, 3)) ? 0x04 : 0) | (BIT(r, 15) ? 0x02 : 0) | (r == 0 ? 0x01 : 0);
			m_r[BIT(inst, 8) + 2] = r;
			m_icount--;
		}
		else if ((inst & 0xfe18) == 0xfa10)
		{
			// SRA
			u16 op = get_reg(BIT(inst, 5, 3));
			u16 r = s16(op) >> (BIT(inst, 0, 3) + 1);
			// TODO: arithmetic overflow flag
			m_ssf = (m_ssf & 0x30) | (BIT(op, BIT(inst, 0, 3)) ? 0x04 : 0) | (BIT(r, 15) ? 0x02 : 0) | (r == 0 ? 0x01 : 0);
			m_r[BIT(inst, 8) + 2] = r;
			m_icount--;
		}
		else if ((inst & 0xfe18) == 0xfa18)
		{
			// SRL
			u16 op = get_reg(BIT(inst, 5, 3));
			u16 r = op >> (BIT(inst, 0, 3) + 1);
			// TODO: arithmetic overflow flag
			m_ssf = (m_ssf & 0x30) | (BIT(op, BIT(inst, 0, 3)) ? 0x04 : 0) | (BIT(r, 15) ? 0x02 : 0) | (r == 0 ? 0x01 : 0);
			m_r[BIT(inst, 8) + 2] = r;
			m_icount--;
		}
		else if ((inst & 0xffc0) == 0xfc80)
		{
			// push IO
			m_ram_space.write_word(m_sp, m_io_space.read_word(BIT(inst, 0, 6)));
			m_sp++;
			m_icount--;
		}
		else if ((inst & 0xffc0) == 0xfcc0)
		{
			// pop IO
			m_sp--;
			m_io_space.write_word(BIT(inst, 0, 6), m_ram_space.read_word(m_sp));
			m_icount--;
		}
		else if ((inst & 0xff00) == 0xfd00)
		{
			// callff
			u32 addr = u32(BIT(inst, 0, 8)) << 16 | m_cache.read_word(m_pc);
			m_pc++;
			m_ram_space.write_word(m_sp, m_pc & 0xffff);
			m_sp++;
			m_ram_space.write_word(m_sp, m_pc >> 16);
			m_sp++;
			m_pc = addr;
			m_icount -= 2;
		}
		else if ((inst & 0xff00) == 0xfe00)
		{
			// jmpff
			u32 addr = u32(BIT(inst, 0, 8)) << 16 | m_cache.read_word(m_pc);
			m_pc = addr;
			m_icount -= 2;
		}
		else if (inst == 0xff42)
		{
			// retff
			m_sp--;
			u32 addr = u32(m_ram_space.read_word(m_sp)) << 16;
			m_sp--;
			addr = (addr & 0xff0000) | m_ram_space.read_word(m_sp);
			m_pc = addr;
			m_icount -= 2;
		}
		else if (inst == 0xffff)
		{
			// nop
			m_icount--;
		}
		else
		{
			logerror("Unknown/invalid instruction word encountered (0x%04X, PC = 0x%06X)\n", inst, m_pc - 1);
			m_icount--;
		}
	} while (m_icount > 0);
}

void sonix16_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c%c%c",
				BIT(m_ssf, 5) ? 'I' : '.',
				BIT(m_ssf, 4) ? 'M' : '.',
				BIT(m_ssf, 3) ? 'A' : '.',
				BIT(m_ssf, 2) ? 'C' : '.',
				BIT(m_ssf, 1) ? 'N' : '.',
				BIT(m_ssf, 0) ? 'Z' : '.');
		break;
	}
}
