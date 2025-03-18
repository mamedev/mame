// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "kl1839vm1.h"
#include "kl1839vm1dasm.h"

#include "cpu/vax/vaxdasm.h"

#include <algorithm>
#include <regex>


#define LOG_VAX    (1U << 1)

//#define VERBOSE ( LOG_GENERAL | LOG_VAX )
#include "logmacro.h"

#define UNIMPLEMENTED(msg) LOG("Unimplemented: %s\n", msg)
#define LOGVAX(...)        LOGMASKED(LOG_VAX, __VA_ARGS__)

#define AMC    m_amc.d   // Microcode PC
#define RV     m_rv.d    // Return Address
#define SCH    m_sch.b.l // Counter
#define RSP    m_rsp.b.l // Flags

#define K(x)   m_consts[x & 0x0f]
#define PCM    K(5)
#define RC     K(6)      // Constant

/* registers of various sizes */
#define R(x)   m_reg[x].d

#define AP     R(0x0c)
#define FP     R(0x0d)
#define SP     R(0x0e)
#define PC     R(0x0f)

#define RNK    R(0x1c)
#define RKA    R(0x1d) // KRSP?
#define PSL    R(0x1e)
#define PSW    m_reg[0x1e].w.l
#define BO     R(0x1f)

#define NF     0x08
#define ZF     0x04
#define VF     0x02
#define CF     0x01

DEFINE_DEVICE_TYPE(KL1839VM1, kl1839vm1_device, "kl1839vm1", "KL1839VM1")


kl1839vm1_device::kl1839vm1_device(const machine_config &mconfig,  const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, KL1839VM1, tag, owner, clock)
	, m_microcode_config("microcode", ENDIANNESS_BIG, 32, 14, -2)
	, m_sysram_config("sysram", ENDIANNESS_BIG, 8, 24, 0)
	, m_ram_config("ram", ENDIANNESS_LITTLE, 8, 24, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 6, -2)
{
}

device_memory_interface::space_config_vector kl1839vm1_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_OPCODES, &m_microcode_config),
		std::make_pair(AS_DATA, &m_sysram_config),
		std::make_pair(AS_PROGRAM, &m_ram_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

void kl1839vm1_device::flag(u32 op)
{
	const bool tou = BIT(op, 16);
	if (tou)
	{
		// TODO Stop BO till the end of DP (Memory Manager?)
	}

	const bool copwt = BIT(op, 15);
	if (copwt)
	{
		RNK = PC;
		RKA = PSL;
	}
}

void kl1839vm1_device::rest(u32 op)
{
}

/*
    Shift/Rotate

    val:
    va: 0 - Carry read; 1 - Carry write
    fo: 0 - dw, 1 - w, 2 - b; 3 - dd
    a_c: 1 - arithmetical, 0 - cyclic
    l_r: 1 - left, 0 - right
*/
u32 kl1839vm1_device::shr(u32 val, bool va, u8 fo, bool a_c, bool l_r)
{
	u8 left_bit;
	switch (fo & 0b11)
	{
		case 0b00: left_bit = 31; break;
		case 0b01: left_bit = 15; break;
		case 0b10: left_bit = 31; break;
		case 0b11: left_bit = 7; break;
	}
	const u8 rd = l_r ? left_bit : 0;
	const u8 wr = l_r ? 0 : left_bit;

	const bool carry_out = BIT(val, rd);
	u32 res = l_r ? (val << 1) : (val >> 1);
	res &= ~(1 << wr);
	if (!a_c)
	{
		res |= (va ? (PSW & CF) : carry_out) << wr;
	}
	else if (l_r && !va)
	{
		res |= carry_out << wr;
	}

	RSP |= carry_out;
	if (a_c && l_r && (fo == 0b10) && (carry_out != BIT(res, rd)))
	{
		RSP |= VF;
	}
	return res;
}

void kl1839vm1_device::kop(u8 kop, u8 fd, u32 x, u32 y, u8 rz, u8 ps, bool va = false, u8 fo = 0)
{
	u64 res = 0;
	switch (fd)
	{
		case 0b11:
			x = s8(x);
			y = s8(y);
			break;
		case 0b01:
			x = s16(x);
			y = s16(y);
			break;
		case 0b00:
		default:  // 0b10
			break;
	}

	RSP &= ~(VF | CF);
	switch (kop)
	{
		case 0b0000: res = y; break;
		//   0b0001:
		case 0b0010: res = u64(x) + y; break;
		case 0b0011: res = x; break;
		case 0b0100: res = u64(x) - y; break;
		//   0b0101
		case 0b0110: res = u64(y) - x; break;
		case 0b0111: res = ~x; break;
		case 0b1000: res = x ^ y; break;
		case 0b1001: res = shr(x, va, fo, 1, 1); break; // AL
		case 0b1010: res = x | y; break;
		case 0b1011: res = shr(x, va, fo, 0, 1); break; // CL
		case 0b1100: res = x & ~y; break;
		case 0b1101: res = shr(x, va, fo, 1, 0); break; // AR
		case 0b1110: res = x & y; break;
		case 0b1111: res = shr(x, va, fo, 0, 0); break; // CR
		default: break; // 0b0001, 0b0101 - reserved
	}

	// flags
	RSP &= ~(NF | ZF);
	RSP |= BIT(res, 31) ? NF : 0;
	RSP |= (res == 0) ? ZF : 0;
	if (kop && ((kop & 0b1001) == 0b0000)) // +/-
	{
		const bool sig_y = BIT(y, 31);
		const bool sig_x = BIT(x, 31);
		RSP |= (kop == 0b0010) // +
			? (((sig_y == sig_x) && (BIT(res, 31) != sig_y)) ? VF : 0)
			: (((sig_y != sig_x) && (BIT(res, 31) != ((kop == 0b0110) ? sig_y : sig_x))) ? VF : 0);

		RSP |= (((kop & 0b1001) != 0b1001) && BIT(res, 32)) ? CF : 0;
	}
	switch (ps)
	{
		case 0b00: PSW = (PSW & ~0x0f) | (RSP & 0x0f); break;
		case 0b01: PSW = (PSW & ~0x0f) | (RSP & 0x0c); break;
		case 0b10: PSW = (PSW & ~0x0e) | (RSP & 0x0e); break;
		default: break;
	}

	switch (fd)
	{
		case 0b11:
			res &= 0x000000ff;
			R(rz) &= 0xffffff00;
			break;
		case 0b01:
			res &= 0x0000ffff;
			R(rz) &= 0xffff0000;
			break;
		case 0b00:
		default:  // 0b10
			R(rz) = 0;
			break;
	}
	R(rz) |= res;
	if (rz == 0x1f)
		mreg_w();
}

bool kl1839vm1_device::mreg_r()
{
	const u8 kob = BIT(m_vma_tmp.d, 24, 3);
	if ((kob & 0b101) == 0b001)
	{
		const u8 no = BIT(m_vma_tmp.d, 30, 2);
		switch (no)
		{
		case 0b00:
			BO = m_ram.read_dword(m_vma_tmp.d);
			break;
		case 0b10:
			BO = m_io.read_dword(m_vma_tmp.d);
			break;
		case 0b11:
			BO = m_sysram.read_dword(m_vma_tmp.d);
			break;
		default:
			UNIMPLEMENTED("Read w/ catch BRA");
			break;
		}

		return true;
	}

	return false;
}

void kl1839vm1_device::mreg_w()
{
	const u8 kob = BIT(m_vma_tmp.d, 24, 3);
	if ((kob & 0b110) == 0b010)
	{
		const u8 no = BIT(m_vma_tmp.d, 30, 2);
		switch (no)
		{
		case 0b00:
			m_ram.write_dword(m_vma_tmp.d, BO);
			break;
		case 0b10:
			m_io.write_dword(m_vma_tmp.d, BO);
			break;
		case 0b11:
			m_sysram.write_dword(m_vma_tmp.d, BO);
			break;
		default:
			UNIMPLEMENTED("Write w/ catch BRA");
			break;
		}
	}
}

void kl1839vm1_device::ma(u32 op)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 kop1 = BIT(op, 24, 4);
	u8 am = BIT(op, 20, 4);
	u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool pd = BIT(op, 4);
	const bool po = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (am>=0xd)
		UNIMPLEMENTED("Read const with data mirror K>0x0d");
	else if (am == 8)
		UNIMPLEMENTED("Read const with data mirror K=8"); // CRM priveleges check

	if (po)
		UNIMPLEMENTED("MA: PO");

	u32 kob_data = kop1 ? R(x) : K(am);

	if (!m_pcm_queue.empty())
	{
		if (py)
			am = vax_pcm_pull();
		else if (am == 5) // PCM
			K(5) = vax_pcm_pull(1);
		else if (pd && (am == 0x1f))
			BO = vax_pcm_pull(1);
		if (px)
			x = vax_pcm_pull();
		else if (pd && (x == 0x1f))
			BO = vax_pcm_pull(1);
	}
	else if (pd)
	{
		mreg_r();
	}

	kop(kop1, fd, R(x), K(am), x, ps, va, fo);
	if (va)
	{
		kob_data = R(x);
	}
	if (fd == 0b10)
	{
		RKA = m_sysram.read_dword(R(x));
	}
	else
	{
		kob_process(no, fo, kob, kob_data, R(x));
	}
}

void kl1839vm1_device::mb(u32 op)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 kop2 = BIT(op, 25, 3);
	u8 y = BIT(op, 20, 5);
	u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool pd = BIT(op, 4);
	const bool po = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (po)
		UNIMPLEMENTED("MB: PO");

	u32 kob_data = kop2 ? R(x) : R(y);

	if (!m_pcm_queue.empty())
	{
		if (py)
			y = vax_pcm_pull();
		else if (pd && !mreg_r() && (y == 0x1f))
			BO = vax_pcm_pull(1);
		if (px)
			x = vax_pcm_pull();
		else if (pd && (x == 0x1f))
			BO = vax_pcm_pull(1);
	}
	else if (pd)
	{
		mreg_r();
	}

	kop(kop2 << 1, fd, R(x), R(y), x, ps, va, fo);
	if (va)
	{
		kob_data = R(x);
	}
	kob_process(no, fo, kob, kob_data, R(x));
}

void kl1839vm1_device::mc(u32 op)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 kop2 = BIT(op, 25, 3);
	u8 y = BIT(op, 20, 5);
	u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	u8 z = BIT(op, 7, 5);

	const bool pd = BIT(op, 4);
	const bool pz = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (!m_pcm_queue.empty())
	{
		if (py)
			y = vax_pcm_pull();
		else if (pd && !mreg_r() && (y == 0x1f))
			BO = vax_pcm_pull(1);
		if (px)
			x = vax_pcm_pull();
		else if (pd && !mreg_r() && (x == 0x1f))
			BO = vax_pcm_pull(1);
		if (pz)
			z = vax_pcm_pull();
		else if (pd && (z == 0x1f))
			BO = vax_pcm_pull(1);
	}
	else if (pd)
	{
		mreg_r();
	}

	kop(kop2 << 1, fd, R(x), kop2 ? R(y) : R(x), z, ps);
}

void kl1839vm1_device::mk(u32 op)
{
	const bool ret = BIT(op, 17);
	if (!ret)
	{
		RV = AMC;
	}

	if ((op & 0xfe000000) == 0xe0000000)
	{
		if (BIT(~op, 16)) // madr == 0
		{
			const u16 addr = BIT(op, 2, 14);
			AMC = addr;
		}
		else
		{
			u16 addr_hi = BIT(op, 8, 8) << 6;
			AMC = addr_hi | (R(0x18) & 0x3f);
		}
	}
	else if ((op & 0xfe000000) == 0xe2000000)
	{
		u16 addr_hi = BIT(op, 8, 8) << 6;
		//u8 rpp = BIT(op, 2, 4); // TODO rpp irq code
		AMC = addr_hi;
	}
}

void kl1839vm1_device::yp(u32 op)
{
	const bool uv = BIT(op, 28);
	const bool n = BIT(op, 27);
	const bool z = BIT(op, 26);
	const bool v = BIT(op, 25);
	const bool c = BIT(op, 24);
	const bool fp = BIT(op, 23);
	const bool fpd = BIT(op, 22);
	//const bool prb = BIT(op, 21);
	//const bool rst = BIT(op, 20);
	//const bool rd = BIT(op, 19);
	const bool ret = BIT(op, 17);
	const bool zhs = BIT(op, 16);
	const u16 addr = BIT(op, 2, 14);

	bool jump = false;
	if (fp)
	{
		// Jump based on SRF flags
		jump = uv == m_fp;
	}
	else if (fpd)
	{
		jump = true;
	}
	else
	{
		// zhs=1 - don't wait for the state completed
		const u8 mask = (n << 3) | (z << 2) | (v << 1) | (c << 0);
		const u8 reg = zhs ? RSP : PSW;
		jump = uv ? ((reg & mask) != 0) : ((reg & mask) == 0);
	}

	if (jump)
	{
		if (!ret)
		{
			RV = AMC;
		}
		AMC = addr;
	}
}

void kl1839vm1_device::zsch(u32 op)
{
	if (m_jzdra_waiting)
	{
		SCH = R(0x19);
		m_jzdra_waiting = false;
	}
	else
	{
		const bool madr = BIT(op, 16);
		if (madr)
		{
			const u8 cnst_hi = BIT(op, 8, 2) << 6;
			SCH = cnst_hi | (R(0x18) & 0x3f);
		}
		else
		{
			const u8 cnst = BIT(op, 2, 8);
			SCH = cnst;
		}
	}
}

void kl1839vm1_device::psch(u32 op)
{
	if (SCH--)
	{
		const bool madr = BIT(op, 16);
		if (madr)
		{
			const u16 addr_hi = BIT(op, 8, 8) << 6;
			AMC = addr_hi | (R(0x18) & 0x3f);
		}
		else
		{
			const u16 addr = BIT(op, 2, 14);
			AMC = addr;
		}
	}
}

void kl1839vm1_device::rts(u32 op)
{
	AMC = RV;
}

void kl1839vm1_device::acc(u32 op)
{
	UNIMPLEMENTED("ACC");
}

void kl1839vm1_device::chka(u32 op)
{
	const bool madr = BIT(op, 16);
	if (madr)
	{
		const u16 addr_hi = BIT(op, 8, 8) << 6;
		RC = m_microcode.read_dword(addr_hi | (R(0x18) & 0x3f));
	}
	else
	{
		const u16 addr = BIT(op, 2, 14);
		RC= m_microcode.read_dword(addr);
	}
}

void kl1839vm1_device::chlk(u32 op)
{
	const bool sb = BIT(op, 1);
	const u32 cnst = BIT(op, 2, 24);
	RC = (sb * 0xff000000) | cnst;
}

void kl1839vm1_device::srf(u32 op)
{
	if (BIT(op, 25)) // WIMM
	{
		UNIMPLEMENTED("SRF: WIMM");
	}
	if (BIT(op, 21)) // SFP1
	{
		m_fp = true;
	}
	if (BIT(op, 20)) // RFP1
	{
		m_fp = false;
	}
	if (BIT(op, 5)) // DEC
	{
		UNIMPLEMENTED("SRF: DEC");
	}
	if (BIT(op, 4)) // OCT
	{
	}
	if (BIT(op, 2)) // JDZRA
	{
		m_jzdra_waiting = true;
	}
	if (BIT(op, 1)) // INC
	{
		UNIMPLEMENTED("SRF: INC");
	}
}

void kl1839vm1_device::invalid(u32 op)
{
}

void kl1839vm1_device::kob_process(u8 no, u8 fo, u8 kob, u32 kob_data, u32 data)
{
	switch (kob) {
		case 0b000: // NOP
			break;
		case 0b001: // Data Read
		case 0b010: // Data Write
		case 0b011: // Read-Modify-Write
			m_vma_tmp.d = (no << 30) | (fo << 28) | (kob << 24) | kob_data;
			break;
		case 0b100: // Write Accum R(17)
			R(0x17) = data;
			break;
		case 0b101: // Read Command
			m_vma_tmp.d = 0;
			PC = data;
			break;
		case 0b110: // Offset Write
			UNIMPLEMENTED("KOB");
			break;

		case 0b111: // Reserve
		default:
			m_vma_tmp.d = 0;
			break;
	}
}

void kl1839vm1_device::decode_op(u32 op)
{
	if ((op & 0xc0000000) == 0x00000000) // MA
	{
		if ((op & 0xff000000) == 0x01000000)
			flag(op);
		else if ((op & 0xff000000) == 0x05000000)
			rest(op);
		else
			ma(op);
	}
	else if ((op & 0xc0000000) == 0x40000000) // MB
	{
		mb(op);
	}
	else if ((op & 0xc0000000) == 0x80000000) // MC
	{
		mc(op);
	}
	else
	{
		if ((op & 0xfc000000) == 0xe0000000)
			mk(op);
		else if ((op & 0xe0000000) == 0xc0000000)
			yp(op);
		else if ((op & 0xfc000000) == 0xec000000)
			zsch(op);
		else if ((op & 0xfc000000) == 0xe4000000)
			psch(op);
		else if ((op & 0xfc000000) == 0xf0000000)
			rts(op);
		else if ((op & 0xfc000000) == 0xe8000000)
			acc(op);
		else if ((op & 0xfc000000) == 0xf4000000)
			chka(op);
		else if ((op & 0xfc000000) == 0xf8000000)
			chlk(op);
		else if ((op & 0xfc000000) == 0xfc000000)
			srf(op);
		else
			UNIMPLEMENTED(op);
	}
}

void kl1839vm1_device::vax_decode_pc()
{
	if (!m_pcm_queue.empty())
	{
		LOGVAX("Unused decoded data\n");
	}
	m_vma_tmp.d = 0;

	const u8 op = m_ram.read_byte(PC);
	m_op_size = 1;
	AMC = op << 4;

	m_pcm_queue.clear();
	const vax_disassembler::mode* args = vax_disassembler::get_operands(op);
	u8 arg_n = 0;
	do
	{
		m_mem_reg[arg_n] = ~0; // Default to unknown

		const vax_disassembler::mode mode = args[arg_n];
		switch (mode)
		{
			case vax_disassembler::mode::none:
				break;

			// byte
			case vax_disassembler::mode::bb:
				m_pcm_queue.push_back(s8(m_ram.read_byte(PC + m_op_size)));
				m_mem_reg[arg_n] = 0x8f;
				m_op_size += 1;
				break;

			case vax_disassembler::mode::cntb:
			case vax_disassembler::mode::mb:
			case vax_disassembler::mode::urb:
			case vax_disassembler::mode::srb:
			case vax_disassembler::mode::wb:
			{
				u8 p = m_ram.read_byte(PC + m_op_size);
				if (p == 0x8f) // M
				{
					m_pcm_queue.push_back(m_ram.read_byte(PC + m_op_size + 1));
					m_mem_reg[arg_n] = p;
					m_op_size += 2;
				}
				else if ((p & 0xf0) == 0x80) // M = R(n)+
				{
					m_pcm_queue.push_back(p & 0x0f);
					m_mem_reg[arg_n] = p;
					m_op_size += 1;
				}
				else if ((p & 0xf0) == 0x50) // R
				{
					m_pcm_queue.push_back(p & 0x0f);
					m_mem_reg[arg_n] = p;
					m_op_size += 1;
				}
				else // ?
				{
					LOGVAX("OP=%02x unknown prefix %02x in operand mode::%02d\n", op, p, u8(mode));
					m_op_size = 0;
				}
				break;
			}

			// word
			case vax_disassembler::mode::bw:
				m_pcm_queue.push_back(s16(m_ram.read_word(PC + m_op_size)));
				m_mem_reg[arg_n] = 0x8f;
				m_op_size += 2;
				break;

			// dword
			case vax_disassembler::mode::rl:
			case vax_disassembler::mode::url:
			case vax_disassembler::mode::ml:
			case vax_disassembler::mode::prl:
			case vax_disassembler::mode::srl:
			case vax_disassembler::mode::wl:
			{
				u8 p = m_ram.read_byte(PC + m_op_size);
				if (p == 0x8f) // M
				{
					m_pcm_queue.push_back(m_ram.read_dword(PC + m_op_size + 1));
					m_mem_reg[arg_n] = p;
					m_op_size += 5;
				}
				else if ((p & 0xf0) == 0x80) // M = R(n)+
				{
					m_pcm_queue.push_back(p & 0x0f);
					m_mem_reg[arg_n] = p;
					m_op_size += 1;
				}
				else if ((p & 0xf0) == 0x50) // R
				{
					m_pcm_queue.push_back(p & 0x0f);
					m_mem_reg[arg_n] = p;
					m_op_size += 1;
				}
				else // ?
				{
					LOGVAX("OP=%02x unknown prefix %02x in operand mode::%02d\n", op, p, u8(mode));
					m_op_size = 0;
				}
				break;
			}

			default:
				LOGVAX("(%x): unknown operand mode %02d in OP=%02x (n=%d)\n", PC, u8(mode), op, arg_n + 1);
				m_pcm_queue.clear();
				m_op_size = 0;
				break;
		}

		++arg_n;
	} while ((arg_n < 6) && (args[arg_n] != vax_disassembler::mode::none));

	if (m_op_size > 0) // above completed without failure
	{
		u8 args_type = 0;
		if (!m_pcm_queue.empty())
		{
			for (u8 i = 0; i < arg_n; ++i)
			{
				args_type <<= 1;
				if ((m_mem_reg[i] & 0xf0) == 0x80)
					args_type |= 1;
				else if ((m_mem_reg[i] & 0xf0) == 0x50)
					args_type |= 0;
				else
					LOGVAX("Unknown argument type: %02x\n", m_mem_reg[i]);
			}
		}

		switch (op)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				assert(m_op_size == 1); // no operands
				break;

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			case 0x30: case 0x31:
				assert(arg_n == 1);
				break;

			case 0x94: case 0x95: case 0x96: case 0x97:
			case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				assert(arg_n == 1);
				AMC += 0x0c * args_type;
				break;

			case 0xda:
			case 0xe8: case 0xe9:
				assert(arg_n == 2);
				break;

			case 0xdb:
				assert(arg_n == 2);
				AMC += BIT(args_type, 0) ? 0x0 : 0x2;
				break;

			case 0x80: case 0x82: case 0x84: case 0x86: case 0x88: case 0x8a: case 0x8c:
			case 0x90: case 0x92:
			case 0xa0: case 0xa2: case 0xa4: case 0xa6: case 0xa8: case 0xaa: case 0xac:
			case 0xb0: case 0xb2:
			case 0xc0: case 0xc2: case 0xc4: case 0xc6: case 0xc8: case 0xca: case 0xcc:
			case 0xd0: case 0xd2:
				assert(arg_n == 2);
				switch (args_type)
				{
					case 0b00: AMC += 0x0; break; // RR
					case 0b01: AMC += 0x4; break; // RM
					case 0b10: AMC += 0xe; break; // MR
					case 0b11: AMC += 0xc; break; // MM
				}
				break;

			case 0x91: case 0x93:
			case 0xb1: case 0xb3:
			case 0xd1: case 0xd3:
				assert(arg_n == 2);
				switch (args_type)
				{
					case 0b00: AMC += 0x0; break; // RR
					case 0b01: AMC += 0x4; break; // RM
					case 0b10:
					case 0b11: AMC += 0xc; break; // MM, MR
				}
				break;

			case 0x78:
				assert(arg_n == 3);
				AMC += BIT(args_type, 0) ? 0x0 : 0x2;
				break;

			case 0x81: case 0x83: case 0x85: case 0x87: case 0x89: case 0x8b: case 0x8d:
			case 0xa1: case 0xa3: case 0xa5: case 0xa7: case 0xa9: case 0xab: case 0xad:
			case 0xc1: case 0xc3: case 0xc5: case 0xc7: case 0xc9: case 0xcb: case 0xcd:
				assert(arg_n == 3);
				switch (args_type)
				{
					case 0b000: AMC += 0x0; break; // RRR
					case 0b011: AMC += 0x4; break; // RMM
					case 0b010: AMC += 0x6; break; // RMR
					case 0b001: AMC += 0x8; break; // RRM
					case 0b110: AMC += 0xe; break; // MMR
					case 0b111: AMC += 0xc; break; // MMM
				}
				break;

			default:
				LOGVAX("(%x): OP=%02x with %d operands is not implemented or supported\n", PC, op, arg_n);
		}
	}

	if (!m_op_size)
	{
		LOGVAX("(%x): undecoded OP=%02x .. EXIT\n", PC, op);
	}
	else
	{
		PC += m_op_size; // move to a next op
		/*LOGVAX("(%x): Decoded: OP=%02x args:%d \n", PC, op, m_pcm_queue.size())*/;
	}
}

u32 kl1839vm1_device::vax_pcm_pull(bool is_bo)
{
	if (m_pcm_queue.empty())
	{
		LOGVAX("Pooling empty decoder queue\n");
	}
	else
	{
		PCM = m_pcm_queue.front();
		m_pcm_queue.pop_front();

		bool is_mem = (m_mem_reg[0] & 0xf0) == 0x80;
		if (is_bo && !is_mem)
		{
			PCM = R(PCM);
		}
		else if (is_mem && ((m_mem_reg[0] & 0x0f) != 0x0f))
		{
			u8 r = PCM;
			PCM = R(PCM);
			R(r) = PCM + 1;
		}
		std::copy(std::begin(m_mem_reg) + 1, std::end(m_mem_reg), std::begin(m_mem_reg));
	}

	return PCM;
}


void kl1839vm1_device::device_start()
{
	m_vax_dasm = std::make_unique<vax_disassembler>();

	space(AS_OPCODES).cache(m_microcode);
	space(AS_DATA).specific(m_sysram);
	space(AS_PROGRAM).specific(m_ram);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_vma_tmp));
	save_item(NAME(m_rv));
	save_item(NAME(m_sch));
	save_item(NAME(m_rsp));
	save_item(NAME(m_amc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_fp));
	save_pointer(NAME(m_reg), 0x20);
	save_pointer(NAME(m_consts), 0x10);

	// Register debugger state
	state_add(KL1839_AMC, "AMC",  AMC).formatstr("%08X");
	state_add(KL1839_PSW, "PSW",  PSW).formatstr("%08s");
	state_add(KL1839_IF,  "cond", m_fp).formatstr("%08s");
	state_add(KL1839_RC,  "RC",   RC).formatstr("%08X");
	state_add(KL1839_RV,  "RV",   RV).formatstr("%08X");
	state_add(KL1839_SCH, "SCH",  SCH).formatstr("%02X");
	state_add_divider(-1);
	state_add(VAX_R0,  "R0",  R(0)).formatstr("%08X");
	state_add(VAX_R1,  "R1",  R(1)).formatstr("%08X");
	state_add(VAX_R2,  "R2",  R(2)).formatstr("%08X");
	state_add(VAX_R3,  "R3",  R(3)).formatstr("%08X");
	state_add(VAX_R4,  "R4",  R(4)).formatstr("%08X");
	state_add(VAX_R5,  "R5",  R(5)).formatstr("%08X");
	state_add(VAX_R6,  "R6",  R(6)).formatstr("%08X");
	state_add(VAX_R7,  "R7",  R(7)).formatstr("%08X");
	state_add(VAX_R8,  "R8",  R(8)).formatstr("%08X");
	state_add(VAX_R9,  "R9",  R(9)).formatstr("%08X");
	state_add(VAX_R10, "R10", R(10)).formatstr("%08X");
	state_add(VAX_R11, "R11", R(11)).formatstr("%08X");
	state_add_divider(-1);
	state_add(VAX_AP, "AP", AP).formatstr("%08X");
	state_add(VAX_FP, "FP", FP).formatstr("%08X");
	state_add(VAX_SP, "SP", SP).formatstr("%08X");
	state_add(VAX_PC, "PC", PC).formatstr("%08X");
	state_add_divider(-1);
	state_add(VAX_INST, "INST", PC).formatstr("%20s");
	state_add_divider(-1);
	state_add(VAX_AK0, "AK0", R(0x14)).formatstr("%08X");
	state_add(VAX_AK1, "AK1", R(0x15)).formatstr("%08X");
	state_add(VAX_AK2, "AK2", R(0x16)).formatstr("%08X");
	state_add(VAX_AK3, "AK3", R(0x12)).formatstr("%08X");
	state_add(VAX_AK4, "AK4", R(0x11)).formatstr("%08X");
	state_add(VAX_AK5, "AK5", R(0x13)).formatstr("%08X");
	state_add(VAX_AK6, "AK6", R(0x17)).formatstr("%08X");
	state_add(VAX_AK7, "AK7", R(0x18)).formatstr("%08X");
	state_add(VAX_AK8, "AK8", R(0x10)).formatstr("%08X");
	state_add_divider(-1);
	state_add(VAX_RNK, "RNK", RNK).formatstr("%08X");
	state_add(VAX_RKA, "RKA", RKA).formatstr("%08X");
	state_add(VAX_PSL, "PSL", PSL).formatstr("%08X");
	state_add(VAX_BO,  "BO",  BO).formatstr("%08X");

	state_add(STATE_GENPC,     "GENPC",    AMC).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_ppc.d).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", RSP).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void kl1839vm1_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				RSP & 0x80 ? '?' : '.',
				RSP & 0x40 ? '?' : '.',
				RSP & 0x20 ? '?' : '.',
				RSP & 0x10 ? '?' : '.',
				RSP & NF   ? 'N' : '.',
				RSP & ZF   ? 'Z' : '.',
				RSP & VF   ? 'V' : '.',
				RSP & CF   ? 'C' : '.'
			);
			break;
		case KL1839_PSW:
			str = string_format("%c%c%c%c%c%c%c%c",
				PSW & 0x80 ? '?' : '.',
				PSW & 0x40 ? '?' : '.',
				PSW & 0x20 ? '?' : '.',
				PSW & 0x10 ? '?' : '.',
				PSW & NF   ? 'N' : '.',
				PSW & ZF   ? 'Z' : '.',
				PSW & VF   ? 'V' : '.',
				PSW & CF   ? 'C' : '.'
			);
			break;
		case KL1839_IF:
			str = string_format("FP%d", m_fp);
			break;
		case VAX_INST:
			if (m_op_size)
			{
				address_space &space_prg = space(AS_PROGRAM);
				std::ostringstream buffer;
				disasm_data_buffer databuf(space_prg);
				m_vax_dasm->disassemble(buffer, PC - m_op_size, databuf, databuf);

				const std::regex reg("\\s+");
				str = regex_replace(buffer.str(), reg, " ");
			}
			else
			{
				str = "--------";
			}
			break;
	}
}

void kl1839vm1_device::device_reset()
{
	for (auto &reg : m_reg)
		reg.d = 0;

	m_ppc.d = AMC = 0x2000;
	m_fp = false;
	m_jzdra_waiting = false;
	RSP = 0;
	m_vma_tmp.d = 0;
	RV = 0;
	SCH = 0;
	m_pcm_queue.clear();
	m_op_size = 0;
}

void kl1839vm1_device::execute_set_input(int irqline, int state)
{
}

void kl1839vm1_device::execute_run()
{
	do
	{
		m_ppc.d = AMC;
		debugger_instruction_hook(m_ppc.d);

		u32 op = m_microcode.read_dword(AMC);
		m_icount -= 2;
		++AMC &= 0x3fff;

		decode_op(op);

		if (op & 1) // S-bit
		{
			vax_decode_pc();
		}
	} while (m_icount > 0);
}

std::unique_ptr<util::disasm_interface> kl1839vm1_device::create_disassembler()
{
	return std::make_unique<kl1839vm1_disassembler>();
}
