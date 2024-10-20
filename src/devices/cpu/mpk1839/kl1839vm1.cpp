// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"

#include "kl1839vm1.h"
#include "kl1839vm1dasm.h"


#define VERBOSE ( LOG_GENERAL )
#include "logmacro.h"

#define UNIMPLEMENTED(msg) LOG("Unimplemented: %s\n", msg)

#define MCA    m_mca.w.l // Micro Command Address
#define VMA    m_vma.d   // Virtual Memory Address
#define RV     m_rv.d
#define SCH    m_sch.b.l // Program Counter
#define RSP    m_rsp.b.l

#define K(x)   m_consts[x & 0x0f]
#define PCM    K(5)
#define RC     K(6)

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
	const u8 copwt = BIT(op, 15, 1);

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
		case 0b10: left_bit = 31; break; // ??? dd
		case 0b11: left_bit = 7; break;
	}
	const u8 rd = l_r ? left_bit : 0;
	const u8 wr = l_r ? 0 : left_bit;

	const bool carry_out = BIT(val, rd);
	u32 res = l_r ? (val << 1) : (val >> 1);
	res &= ~(1 << wr);
	if (!a_c)
	{
		res |= (va ? (RSP & CF) : carry_out) << wr;
	}
	else if (l_r && !va)
	{
		res |= carry_out << wr;
	}

	RSP &= ~(VF | CF);
	RSP |= carry_out;
	if (a_c && l_r && (fo == 0b10) && (carry_out != BIT(res, rd)))
	{
		RSP |= VF;
	}
	return res;
}

void kl1839vm1_device::kop(u8 kop, u8 fd, u32 x, u32 y, u32 &z, u8 ps, bool va = false, u8 fo = 0)
{
	u32 res = 0;
	RSP &= ~(NF | ZF | VF);
	if ((kop & 0b1001) == 0b1001) // shr
	{
		RSP &= ~CF;
	}
	switch(kop)
	{
		case 0b0000: res = y; break;
		//   0b0001:
		case 0b0010: res = x + y; break;
		case 0b0011: res = x; break;
		case 0b0100: res = x - y; break;
		//   0b0101
		case 0b0110: res = y - x; break;
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

	switch(fd)
	{
		case 0b00:
			z = res;
			RSP |= BIT(res, 31) ? NF : 0;
			break;
		case 0b01:
			z = (z & 0xffff0000) | (res & 0x0000ffff);
			RSP |= BIT(res, 15) ? NF : 0;
			break;
		case 0b11:
			z = (z & 0xffffff00) | (res & 0x000000ff);
			RSP |= BIT(res, 7) ? NF : 0;
			break;
		default: RKA = res; break; // 0b10
	}
	RSP |= (res == 0) ? ZF : 0;

	switch (ps)
	{
		case 0b00: PSW = (PSW & ~0x0f) | (RSP & 0x0f); break;
		case 0b01: PSW = (PSW & ~0x0e) | (RSP & 0x0e); break;
		case 0b10: PSW = (PSW & ~0x0f) | (RSP & 0x0c); break;
		default: break;
	}
}

void kl1839vm1_device::mreg_r()
{
	const u8 kob_tmp = BIT(m_vma_tmp.d, 24, 3);
	if (kob_tmp == 0b001)
	{
		const u8 no_tmp = BIT(m_vma_tmp.d, 30, 2);
		switch (no_tmp)
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
	}
}

void kl1839vm1_device::mreg_w()
{
	const u8 kob_tmp = BIT(m_vma_tmp.d, 24, 3);
	if (kob_tmp == 0b010)
	{
		const u8 no_tmp = BIT(m_vma_tmp.d, 30, 2);
		switch (no_tmp)
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
	const u8 am = BIT(op, 20, 4);
	const u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool pd = BIT(op, 4);
	const bool po = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (am==5 || am==8 || am>=0xd)
		UNIMPLEMENTED("Read regs with data mirror");

	if (po | py | px)
		UNIMPLEMENTED("MA: P*");

	u32 kob_data = kop1 ? R(x) : K(am);
	kop(kop1, fd, R(x), K(am), R(x), ps, va, fo);
	if (pd || x == 0x1f) mreg_w();
	if (va)
	{
		kob_data = R(x);
	}
	kob_process(no, fo, kob, kob_data);
}

void kl1839vm1_device::mb(u32 op)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 kop2 = BIT(op, 25, 3);
	const u8 y = BIT(op, 20, 5);
	const u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);
	const bool va = BIT(op, 12);
	const u8 no = BIT(op, 10, 2);
	const u8 fo = BIT(op, 8, 2);
	const u8 kob = BIT(op, 5, 3);
	const bool pd = BIT(op, 4);
	const bool po = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (po | py | px)
		UNIMPLEMENTED("MB: P*");

	u32 kob_data = kop2 ? R(x) : R(y);
	if (pd || y == 0x1f) mreg_r();
	kop(kop2 << 1, fd, R(x), R(y), R(x), ps, va, fo);
	if (pd || x == 0x1f) mreg_w();
	if (va)
	{
		kob_data = R(x);
	}
	kob_process(no, fo, kob, kob_data);
}

void kl1839vm1_device::mc(u32 op)
{
	const u8 fd = BIT(op, 28, 2);
	const u8 kop2 = BIT(op, 25, 3);
	const u8 y = BIT(op, 20, 5);
	const u8 x = BIT(op, 15, 5);
	const u8 ps = BIT(op, 13, 2);

	const u8 z = BIT(op, 7, 5);

	const bool pd = BIT(op, 4);
	const bool pz = BIT(op, 3);
	const bool py = BIT(op, 2);
	const bool px = BIT(op, 1);

	if (pz | py | px)
		UNIMPLEMENTED("MC: P*");

	if (pd || x == 0x1f) mreg_r();
	kop(kop2 << 1, fd, R(x), kop2 ? R(y) : R(x), R(z), ps);
	if (pd || z == 0x1f) mreg_w();
}

void kl1839vm1_device::mk(u32 op)
{
	const bool ret = BIT(op, 17);
	if (!ret)
	{
		RV = MCA;
	}

	if ((op & 0xfe000000) == 0xe0000000)
	{
		if (BIT(~op, 16)) // madr == 0
		{
			const u16 addr = BIT(op, 2, 14);
			MCA = addr;
		}
		else
		{
			u16 addr_hi = BIT(op, 8, 8) << 6;
			MCA = addr_hi | (R(0x18) & 0x3f);
		}
	}
	else if ((op & 0xfe000000) == 0xe2000000)
	{
		u16 addr_hi = BIT(op, 8, 8) << 6;
		//u8 rpp = BIT(op, 2, 4); // TODO rpp irq code
		MCA = addr_hi;
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
	//const bool fpd = BIT(op, 22);
	//const bool prb = BIT(op, 21);
	//const bool rst = BIT(op, 20);
	//const bool rd = BIT(op, 19);
	const bool ret = BIT(op, 17);
	const bool zhs = BIT(op, 16);
	const u16 addr = BIT(op, 2, 14);

	bool jump = false;
	if (fp)
	{
		jump = uv == m_fp;
	}
	else
	{
		const u8 mask = (n << 3) | (z << 2) | (v << 1) | (c << 0);
		const u8 reg = zhs ? RSP : PSW;
		jump = uv ? ((reg & mask) != 0) : ((reg & mask) == 0);
	}

	if (jump)
	{
		if (!ret)
		{
			RV = MCA;
		}
		MCA = addr;
	}
}

void kl1839vm1_device::zsch(u32 op)
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

void kl1839vm1_device::psch(u32 op)
{
	const bool madr = BIT(op, 16);
	if (madr)
	{
		const u16 addr_hi = BIT(op, 8, 8) << 6;
		MCA = addr_hi | (R(0x18) & 0x3f);
	}
	else
	{
		if (SCH--)
		{
			const u16 addr = BIT(op, 2, 14);
			MCA = addr;
		}

	}
}

void kl1839vm1_device::rts(u32 op)
{
	MCA = RV;
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
		--MCA;
	}
	if (BIT(op, 2)) // JDZRA
	{
		UNIMPLEMENTED("SRF: JDZRA");
	}
	if (BIT(op, 1)) // INC
	{
		UNIMPLEMENTED("SRF: INC");
	}
}

void kl1839vm1_device::invalid(u32 op)
{
}

void kl1839vm1_device::kob_process(u8 no, u8 fo, u8 kob, u32 data)
{
	switch (kob) {
		case 0b001: // Data Read
		case 0b010: // Data Write
		case 0b011: // Read-Modify-Write
			m_vma_tmp.d = (no << 30) | (fo << 28) | (kob << 24) | data;
			break;
		case 0b100: // Write Accum R(17)
		case 0b101: // Read Command
		case 0b110: // Offset Write
			UNIMPLEMENTED("KOB");
			break;

		case 0b000: // NOP
		case 0b111: // Reserve
		default: break;
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
			invalid(op);
	}
}

void kl1839vm1_device::device_start()
{
	space(AS_OPCODES).cache(m_microcode);
	space(AS_DATA).specific(m_sysram);
	space(AS_PROGRAM).specific(m_ram);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_mca));
	save_item(NAME(m_vma));
	save_item(NAME(m_vma_tmp));
	save_item(NAME(m_rv));
	save_item(NAME(m_sch));
	save_item(NAME(m_rsp));
	save_item(NAME(m_ppc));
	save_item(NAME(m_fp));
	save_pointer(NAME(m_reg), 0x20);
	save_pointer(NAME(m_consts), 0x10);

	// Register debugger state
	state_add(KL1839_MCA, "MCA", MCA).formatstr("%04X");
	state_add(KL1839_VMA, "VMA", VMA).formatstr("%08X");
	state_add_divider(-1);
	state_add(KL1839_IF,  "cond",  m_fp).formatstr("%08s");
	state_add(KL1839_RSP, "RSP",   RSP).formatstr("%08s");
	state_add(KL1839_PCM, "PCM",   PCM).formatstr("%08X");
	state_add(KL1839_RC,  "RC",    RC).formatstr("%08X");
	state_add(KL1839_RV,  "RV",    RV).formatstr("%08X");
	state_add(KL1839_SCH, "SCH",   SCH).formatstr("%02X");
	state_add_divider(-1);
	state_add(VAX_AP, "AP", AP).formatstr("%08X");
	state_add(VAX_FP, "FP", FP).formatstr("%08X");
	state_add(VAX_SP, "SP", SP).formatstr("%08X");
	state_add(VAX_PC, "PC", PC).formatstr("%08X");
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
	state_add(VAX_RA,  "RA",  R(0x0a)).formatstr("%08X");
	state_add(VAX_RB,  "RB",  R(0x0b)).formatstr("%08X");
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

	state_add(STATE_GENPC, "GENPC", MCA).noshow(); // ???
	state_add(STATE_GENPCBASE, "CURPC", m_ppc.d).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", PSW).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void kl1839vm1_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
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
		case KL1839_RSP:
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
		case KL1839_IF:
			str = string_format("FP%d", m_fp);
			break;
	}
}

void kl1839vm1_device::device_reset()
{
	for (auto &reg : m_reg)
		reg.d = 0;

	m_ppc.d = MCA = 0x2000;
	VMA = 0;
	m_fp = false;
	RSP = 0;
	m_vma_tmp.d = 0;
	RV = 0;
	SCH = 0;
}

void kl1839vm1_device::execute_set_input(int irqline, int state)
{
}

void kl1839vm1_device::execute_run()
{
	do
	{
		m_ppc.d = MCA;
		debugger_instruction_hook(m_ppc.d);

		u32 op = m_microcode.read_dword(MCA);
		m_icount -= 2;
		++MCA &= 0x3fff;

		decode_op(op);
	} while (m_icount > 0);
}

std::unique_ptr<util::disasm_interface> kl1839vm1_device::create_disassembler()
{
	return std::make_unique<kl1839vm1_disassembler>();
}

