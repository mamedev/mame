// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Bryan McPhail
/****************************************************************************

    v30mz emulator based on NEC V20/V30/V33 emulator.

    The internal details of the prefetch queue are not exactly known. We
    keep a prefetch queue a 8 bytes even though the documentation mentions
    a prefetch queue of 8 words/16 bytes. Using 8 bytes keeps the amount of
    fetches limited on tight loops and is more than enough for the longest
    instruction.

    Todo!
      - Double check cycle timing is 100%.
            - Add penalties when BW, BP, SP, IX, IY etc are changed in the immediately
              preceding instruction.
            - wswan mjkiwame (at 0x40141) has rep in al,$b5 (f3 e4 b5). Should this
              repeat the in instruction or is this a bug made by the programmer?

****************************************************************************/

#include "emu.h"
#include "v30mz.h"
#include "cpu/nec/necdasm.h"


enum SREGS { DS1=0, PS, SS, DS0 };
enum WREGS { AW=0, CW, DW, BW, SP, BP, IX, IY };

enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
	SPL = NATIVE_ENDIAN_VALUE_LE_BE(0x8, 0x9),
	SPH = NATIVE_ENDIAN_VALUE_LE_BE(0x9, 0x8),
	BPL = NATIVE_ENDIAN_VALUE_LE_BE(0xa, 0xb),
	BPH = NATIVE_ENDIAN_VALUE_LE_BE(0xb, 0xa),
	IXL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
	IXH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
	IYL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
	IYH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
};

enum nec_irqs {
	DIVIDE_ERROR_INT = 0,
	BREAK_INT = 1,
	NMI_INT = 2,
	BRK_3_INT = 3,
	BRKV_INT = 4,
	CHKIND_INT = 5
};


#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      m_parity_table[(uint8_t)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)
#define MD      (m_MF!=0)


// The interrupt number of a pending external interrupt pending NMI is 2.
// For INTR interrupts, the level is caught on the bus during an INTA cycle

#define INT_IRQ 0x01
#define NMI_IRQ 0x02


DEFINE_DEVICE_TYPE(V30MZ, v30mz_cpu_device, "v30mz", "NEC V30MZ")


v30mz_cpu_device::v30mz_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, V30MZ, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
	, m_ip(0)
	, m_TF(0)
	, m_int_vector(0)
	, m_pc(0)
	, m_vector_func(*this)
{
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	// Set up parity lookup table.
	for (uint16_t i = 0; i < 256; i++)
	{
		uint16_t c = 0;
		for (uint16_t j = i; j > 0; j >>= 1)
		{
			if (j & 1) c++;
		}
		m_parity_table[i] = !(c & 1);
	}

	for (uint16_t i = 0; i < 256; i++)
	{
		m_Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		m_Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (uint16_t i = 0xc0; i < 0x100; i++)
	{
		m_Mod_RM.RM.w[i] = (WREGS)(i & 7);
		m_Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}

	std::fill(std::begin(m_regs.w), std::end(m_regs.w), 0);
	std::fill(std::begin(m_prefetch_queue), std::end(m_prefetch_queue), 0);
}

device_memory_interface::space_config_vector v30mz_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


void v30mz_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

	m_vector_func.resolve_safe(0);

	save_item(NAME(m_regs.w));
	save_item(NAME(m_sregs));
	save_item(NAME(m_ip));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
	save_item(NAME(m_MF));
	save_item(NAME(m_SignVal));
	save_item(NAME(m_int_vector));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_AuxVal));
	save_item(NAME(m_OverVal));
	save_item(NAME(m_ZeroVal));
	save_item(NAME(m_CarryVal));
	save_item(NAME(m_ParityVal));
	save_item(NAME(m_seg_prefix));
	save_item(NAME(m_seg_prefix_next));
	save_item(NAME(m_pfp));
	save_item(NAME(m_prefetch_queue));
	save_item(NAME(m_prefetch_queue_head));
	save_item(NAME(m_prefetch_queue_tail));

	// Register state for debugger
	state_add(NEC_IP, "IP", m_ip).callimport().callexport().formatstr("%04X");
	state_add(NEC_SP, "SP", m_regs.w[SP]).callimport().callexport().formatstr("%04X");
	state_add(NEC_AW, "AW", m_regs.w[AW]).callimport().callexport().formatstr("%04X");
	state_add(NEC_BW, "BW", m_regs.w[BW]).callimport().callexport().formatstr("%04X");
	state_add(NEC_CW, "CW", m_regs.w[CW]).callimport().callexport().formatstr("%04X");
	state_add(NEC_DW, "DW", m_regs.w[DW]).callimport().callexport().formatstr("%04X");
	state_add(NEC_BP, "BP", m_regs.w[BP]).callimport().callexport().formatstr("%04X");
	state_add(NEC_IX, "IX", m_regs.w[IX]).callimport().callexport().formatstr("%04X");
	state_add(NEC_IY, "IY", m_regs.w[IY]).callimport().callexport().formatstr("%04X");
	state_add(NEC_PS, "PS", m_sregs[PS]).callimport().callexport().formatstr("%04X");
	state_add(NEC_SS, "SS", m_sregs[SS]).callimport().callexport().formatstr("%04X");
	state_add(NEC_DS0, "DS0", m_sregs[DS0]).callimport().callexport().formatstr("%04X");
	state_add(NEC_DS1, "DS1", m_sregs[DS1]).callimport().callexport().formatstr("%04X");
	state_add(NEC_FLAGS, "PSW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(NEC_VECTOR, "V", m_int_vector).callimport().callexport().formatstr("%02X");
	state_add(NEC_PFP, "PFP", m_pfp).callimport().callexport().formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_pc).callexport().formatstr("%05X");
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callexport().formatstr("%05X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_TF).callimport().callexport().formatstr("%16s").noshow();

	set_icountptr(m_icount);
}


void v30mz_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case NEC_FLAGS:
		expand_flags(m_debugger_temp);
		break;
	}
}


void v30mz_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case NEC_FLAGS:
		m_debugger_temp = compress_flags();
		break;
	}
}


void v30mz_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			str = string_format("%08X", (m_sregs[PS] << 4) + m_ip);
			break;

		case STATE_GENFLAGS:
			{
				uint16_t flags = compress_flags();
				str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					flags & 0x8000 ? 'M':'.',
					flags & 0x4000 ? '?':'.',
					flags & 0x2000 ? '?':'.',
					flags & 0x1000 ? '?':'.',
					flags & 0x0800 ? 'O':'.',
					flags & 0x0400 ? 'D':'.',
					flags & 0x0200 ? 'I':'.',
					flags & 0x0100 ? 'T':'.',
					flags & 0x0080 ? 'S':'.',
					flags & 0x0040 ? 'Z':'.',
					flags & 0x0020 ? '?':'.',
					flags & 0x0010 ? 'A':'.',
					flags & 0x0008 ? '?':'.',
					flags & 0x0004 ? 'P':'.',
					flags & 0x0002 ? '?':'.',
					flags & 0x0001 ? 'C':'.');
			}
			break;
	}
}


void v30mz_cpu_device::device_reset()
{
	m_ZeroVal = 1;
	m_ParityVal = 1;
	m_regs.w[AW] = 0;
	m_regs.w[CW] = 0;
	m_regs.w[DW] = 0;
	m_regs.w[BW] = 0;
	m_regs.w[SP] = 0;
	m_regs.w[BP] = 0;
	m_regs.w[IX] = 0;
	m_regs.w[IY] = 0;
	m_sregs[DS1] = 0;
	m_sregs[PS] = 0xffff;
	m_sregs[SS] = 0;
	m_sregs[DS0] = 0;
	m_ip = 0;
	m_pfp = 0;
	m_SignVal = 0;
	m_AuxVal = 0;
	m_OverVal = 0;
	m_CarryVal = 0;
	m_TF = 0;
	m_IF = 0;
	m_DF = 0;
	m_MF = 1;
	m_int_vector = 0;
	m_pending_irq = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_no_interrupt = 0;
	m_fire_trap = 0;
	m_prefix_base = 0;
	m_seg_prefix = false;
	m_seg_prefix_next = false;
	m_ea_seg = 0;
	m_eo = 0;
	m_modrm = 0;
	m_dst = 0;
	m_src = 0;
	init_prefetch();
}


uint32_t v30mz_cpu_device::pc()
{
	m_pc = (m_sregs[PS] << 4) + m_ip;
	return m_pc;
}


void v30mz_cpu_device::read_prefetch()
{
	uint8_t data = m_cache.read_byte((m_sregs[PS] << 4) + m_pfp);
	m_prefetch_queue[m_prefetch_queue_head] = data;
	m_prefetch_queue_head = (m_prefetch_queue_head + 1) % PREFETCH_MAX_SIZE;
	m_pfp++;
}


void v30mz_cpu_device::init_prefetch()
{
	m_pfp = m_ip;
	m_prefetch_queue_tail = 0;
	m_prefetch_queue_head = 0;
	m_prefetch_fill_needed = true;
	if (m_ip & 1)
		clk(1);
}


inline uint8_t v30mz_cpu_device::read_byte(uint16_t segment, uint16_t addr)
{
	return m_program.read_byte((segment << 4) + addr);
}


inline uint16_t v30mz_cpu_device::read_word(uint16_t segment, uint16_t addr)
{
	if (addr & 1) {
		// penalty cycle when reading from an unaligned address
		clk(1);
		return m_program.read_byte((segment << 4)+ addr) |
			(m_program.read_byte((segment << 4) + ((addr + 1) & 0xffff)) << 8);
	} else {
		return m_program.read_word((segment << 4) + addr);
	}
}


inline void v30mz_cpu_device::write_byte(uint16_t segment, uint16_t addr, uint8_t data)
{
	m_program.write_byte((segment << 4) + addr, data);
}


inline void v30mz_cpu_device::write_word(uint16_t segment, uint16_t addr, uint16_t data)
{
	if (addr & 1) {
		m_program.write_byte((segment << 4) + addr, data);
		m_program.write_byte((segment << 4) + ((addr + 1) & 0xffff), data >> 8);
		// penalty cycle when writing to an unaligned address
		clk(1);
	} else {
		m_program.write_word((segment << 4) + addr, data);
	}
}


inline uint8_t v30mz_cpu_device::read_port(uint16_t port)
{
	return m_io.read_byte(port);
}


inline uint16_t v30mz_cpu_device::read_port_word(uint16_t port)
{
	if (port & 1)
		// penalty cycle when reading from an unaligned address
		clk(1);
	return m_io.read_word_unaligned(port);
}


inline void v30mz_cpu_device::write_port(uint16_t port, uint8_t data)
{
	m_io.write_byte(port, data);
}


inline void v30mz_cpu_device::write_port_word(uint16_t port, uint16_t data)
{
	if (port & 1)
		// penalty cycle when writing to an unaligned address
		clk(1);
	m_io.write_word_unaligned(port, data);
}


inline uint8_t v30mz_cpu_device::fetch_op()
{
	uint8_t data = m_prefetch_queue[m_prefetch_queue_tail];
	m_prefetch_queue_tail = (m_prefetch_queue_tail + 1) % PREFETCH_MAX_SIZE;
	read_prefetch();
	m_ip++;
	return data;
}


inline uint8_t v30mz_cpu_device::fetch()
{
	uint8_t data = m_prefetch_queue[m_prefetch_queue_tail];
	m_prefetch_queue_tail = (m_prefetch_queue_tail + 1) % PREFETCH_MAX_SIZE;
	read_prefetch();
	m_ip++;
	return data;
}


inline uint16_t v30mz_cpu_device::fetch_word()
{
	uint16_t data = fetch();
	data |= (fetch() << 8);
	return data;
}


inline uint8_t v30mz_cpu_device::repx_op()
{
	uint8_t next = fetch_op();
	bool seg_prefix = false;
	int seg = 0;

	switch (next)
	{
	case 0x26:
		seg_prefix = true;
		seg = DS1;
		break;
	case 0x2e:
		seg_prefix = true;
		seg = PS;
		break;
	case 0x36:
		seg_prefix = true;
		seg = SS;
		break;
	case 0x3e:
		seg_prefix = true;
		seg = DS0;
		break;
	}

	if (seg_prefix)
	{
		m_seg_prefix = true;
		m_seg_prefix_next = true;
		m_prefix_base = m_sregs[seg];
		next = fetch_op();
		clk(2);
	}

	return next;
}


inline void v30mz_cpu_device::clk(uint32_t cycles)
{
	m_icount -= cycles;
}


inline void v30mz_cpu_device::clkm(uint32_t cycles_reg, uint32_t cycles_mem)
{
	m_icount -= (m_modrm >= 0xc0) ? cycles_reg : cycles_mem;
}


inline uint16_t v30mz_cpu_device::default_base(int seg)
{
	if (m_seg_prefix && (seg==DS0 || seg==SS))
	{
		return m_prefix_base;
	}
	else
	{
		return m_sregs[seg];
	}
}


inline void v30mz_cpu_device::get_ea()
{
	switch (m_modrm & 0xc7)
	{
	case 0x00:
		m_eo = m_regs.w[BW] + m_regs.w[IX];
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x01:
		m_eo = m_regs.w[BW] + m_regs.w[IY];
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x02:
		m_eo = m_regs.w[BP] + m_regs.w[IX];
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x03:
		m_eo = m_regs.w[BP] + m_regs.w[IY];
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x04:
		m_eo = m_regs.w[IX];
		m_ea_seg = default_base(DS0);
		break;
	case 0x05:
		m_eo = m_regs.w[IY];
		m_ea_seg = default_base(DS0);
		break;
	case 0x06:
		m_eo = fetch_word();
		m_ea_seg = default_base(DS0);
		break;
	case 0x07:
		m_eo = m_regs.w[BW];
		m_ea_seg = default_base(DS0);
		break;

	case 0x40:
		m_eo = m_regs.w[BW] + m_regs.w[IX] + (int8_t)fetch();
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x41:
		m_eo = m_regs.w[BW] + m_regs.w[IY] + (int8_t)fetch();
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x42:
		m_eo = m_regs.w[BP] + m_regs.w[IX] + (int8_t)fetch();
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x43:
		m_eo = m_regs.w[BP] + m_regs.w[IY] + (int8_t)fetch();
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x44:
		m_eo = m_regs.w[IX] + (int8_t)fetch();
		m_ea_seg = default_base(DS0);
		break;
	case 0x45:
		m_eo = m_regs.w[IY] + (int8_t)fetch();
		m_ea_seg = default_base(DS0);
		break;
	case 0x46:
		m_eo = m_regs.w[BP] + (int8_t)fetch();
		m_ea_seg = default_base(SS);
		break;
	case 0x47:
		m_eo = m_regs.w[BW] + (int8_t)fetch();
		m_ea_seg = default_base(DS0);
		break;

	case 0x80:
		m_eo = m_regs.w[BW] + m_regs.w[IX] + (int16_t)fetch_word();
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x81:
		m_eo = m_regs.w[BW] + m_regs.w[IY] + (int16_t)fetch_word();
		m_ea_seg = default_base(DS0);
		clk(1);
		break;
	case 0x82:
		m_eo = m_regs.w[BP] + m_regs.w[IX] + (int16_t)fetch_word();
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x83:
		m_eo = m_regs.w[BP] + m_regs.w[IY] + (int16_t)fetch_word();
		m_ea_seg = default_base(SS);
		clk(1);
		break;
	case 0x84:
		m_eo = m_regs.w[IX] + (int16_t)fetch_word();
		m_ea_seg = default_base(DS0);
		break;
	case 0x85:
		m_eo = m_regs.w[IY] + (int16_t)fetch_word();
		m_ea_seg = default_base(DS0);
		break;
	case 0x86:
		m_eo = m_regs.w[BP] + (int16_t)fetch_word();
		m_ea_seg = default_base(SS);
		break;
	case 0x87:
		m_eo = m_regs.w[BW] + (int16_t)fetch_word();
		m_ea_seg = default_base(DS0);
		break;
	}
}


inline void v30mz_cpu_device::store_ea_rm_byte(uint8_t data)
{
	if (m_modrm >= 0xc0)
	{
		m_regs.b[m_Mod_RM.RM.b[m_modrm]] = data;
	}
	else
	{
		write_byte(m_ea_seg, m_eo, data);
	}
}


inline void v30mz_cpu_device::store_ea_rm_word(uint16_t data)
{
	if (m_modrm >= 0xc0)
	{
		m_regs.w[m_Mod_RM.RM.w[m_modrm]] = data;
	}
	else
	{
		write_word(m_ea_seg, m_eo, data);
	}
}


inline void v30mz_cpu_device::put_imm_rm_word()
{
	if (m_modrm >= 0xc0)
	{
		m_regs.w[m_Mod_RM.RM.w[m_modrm]] = fetch_word();
	}
	else
	{
		get_ea();
		write_word(m_ea_seg, m_eo, fetch_word());
	}
}


inline void v30mz_cpu_device::put_rm_word(uint16_t val)
{
	if (m_modrm >= 0xc0)
	{
		m_regs.w[m_Mod_RM.RM.w[m_modrm]] = val;
	}
	else
	{
		get_ea();
		write_word(m_ea_seg, m_eo, val);
	}
}


inline void v30mz_cpu_device::put_rm_byte(uint8_t val)
{
	if (m_modrm >= 0xc0)
	{
		m_regs.b[m_Mod_RM.RM.b[m_modrm]] = val;
	}
	else
	{
		get_ea();
		write_byte(m_ea_seg, m_eo, val);
	}
}


inline void v30mz_cpu_device::put_imm_rm_byte()
{
	if (m_modrm >= 0xc0)
	{
		m_regs.b[m_Mod_RM.RM.b[m_modrm]] = fetch();
	}
	else
	{
		get_ea();
		write_byte(m_ea_seg, m_eo, fetch());
	}
}


inline void v30mz_cpu_device::def_br8()
{
	m_modrm = fetch();
	m_src = reg_byte();
	m_dst = get_rm_byte();
}


inline void v30mz_cpu_device::def_wr16()
{
	m_modrm = fetch();
	m_src = reg_word();
	m_dst = get_rm_word();
}


inline void v30mz_cpu_device::def_r8b()
{
	m_modrm = fetch();
	m_src = get_rm_byte();
	m_dst = reg_byte();
}


inline void v30mz_cpu_device::def_r16w()
{
	m_modrm = fetch();
	m_src = get_rm_word();
	m_dst = reg_word();
}


inline void v30mz_cpu_device::def_ald8()
{
	m_src = fetch();
	m_dst = m_regs.b[AL];
}


inline void v30mz_cpu_device::def_awd16()
{
	m_src = fetch_word();
	m_dst = m_regs.w[AW];
}


inline void v30mz_cpu_device::reg_byte(uint8_t data)
{
	m_regs.b[m_Mod_RM.reg.b[m_modrm]] = data;
}


inline void v30mz_cpu_device::reg_word(uint16_t data)
{
	m_regs.w[m_Mod_RM.reg.w[m_modrm]] = data;
}


inline uint8_t v30mz_cpu_device::reg_byte()
{
	return m_regs.b[m_Mod_RM.reg.b[m_modrm]];
}


inline uint16_t v30mz_cpu_device::reg_word()
{
	return m_regs.w[m_Mod_RM.reg.w[m_modrm]];
}


inline uint16_t v30mz_cpu_device::get_rm_word()
{
	if (m_modrm >= 0xc0)
	{
		return m_regs.w[m_Mod_RM.RM.w[m_modrm]];
	}
	else
	{
		get_ea();
		return read_word(m_ea_seg, m_eo);
	}
}


inline uint16_t v30mz_cpu_device::get_next_rm_word()
{
	return read_word(m_ea_seg, m_eo + 2);
}


inline uint8_t v30mz_cpu_device::get_rm_byte()
{
	if (m_modrm >= 0xc0)
	{
		return m_regs.b[m_Mod_RM.RM.b[m_modrm]];
	}
	else
	{
		get_ea();
		return read_byte(m_ea_seg, m_eo);
	}
}


// Setting flags

inline void v30mz_cpu_device::set_CF_byte(uint32_t x)
{
	m_CarryVal = x & 0x100;
}


inline void v30mz_cpu_device::set_CF_word(uint32_t x)
{
	m_CarryVal = x & 0x10000;
}


inline void v30mz_cpu_device::set_AF(uint32_t x, uint32_t y, uint32_t z)
{
	m_AuxVal = (x ^ (y ^ z)) & 0x10;
}


inline void v30mz_cpu_device::set_SF(uint32_t x)
{
	m_SignVal = x;
}


inline void v30mz_cpu_device::set_ZF(uint32_t x)
{
	m_ZeroVal = x;
}


inline void v30mz_cpu_device::set_PF(uint32_t x)
{
	m_ParityVal = x;
}


inline void v30mz_cpu_device::set_SZPF_byte(uint32_t x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (int8_t)x;
}


inline void v30mz_cpu_device::set_SZPF_word(uint32_t x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (int16_t)x;
}


inline void v30mz_cpu_device::set_OF_word_add(uint32_t x, uint32_t y, uint32_t z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x8000;
}


inline void v30mz_cpu_device::set_OF_byte_add(uint32_t x, uint32_t y, uint32_t z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x80;
}


inline void v30mz_cpu_device::set_OF_word_sub(uint32_t x, uint32_t y, uint32_t z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x8000;
}


inline void v30mz_cpu_device::set_OF_byte_sub(uint32_t x, uint32_t y, uint32_t z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x80;
}


inline uint16_t v30mz_cpu_device::compress_flags() const
{
	return 0x7002
	  | (CF ? 1 : 0)
		| (PF ? 4 : 0)
		| (AF ? 0x10 : 0)
		| (ZF ? 0x40 : 0)
		| (SF ? 0x80 : 0)
		| (m_TF << 8)
		| (m_IF << 9)
		| (m_DF << 10)
		| (OF << 11)
		| (MD << 15);
}


inline void v30mz_cpu_device::expand_flags(uint16_t f)
{
	m_CarryVal = (f) & 1;
	m_ParityVal = !((f) & 4);
	m_AuxVal = (f) & 16;
	m_ZeroVal = !((f) & 64);
	m_SignVal = (f) & 128 ? -1 : 0;
	m_TF = ((f) & 256) == 256;
	m_IF = ((f) & 512) == 512;
	m_DF = ((f) & 1024) == 1024;
	m_OverVal = (f) & 2048;
	m_MF = ((f) & 0x8000) == 0x8000;
}


inline void v30mz_cpu_device::i_insb()
{
	write_byte(m_sregs[DS1], m_regs.w[IY], read_port(m_regs.w[DW]));
	m_regs.w[IY] += -2 * m_DF + 1;
	clk(6);
}


inline void v30mz_cpu_device::i_insw()
{
	write_word(m_sregs[DS1], m_regs.w[IY], read_port_word(m_regs.w[DW]));
	m_regs.w[IY] += -4 * m_DF + 2;
	clk(6);
}


inline void v30mz_cpu_device::i_outsb()
{
	write_port(m_regs.w[DW], read_byte(default_base(DS0), m_regs.w[IX]));
	m_regs.w[IX] += -2 * m_DF + 1;
	clk(7);
}


inline void v30mz_cpu_device::i_outsw()
{
	write_port_word(m_regs.w[DW], read_word(default_base(DS0), m_regs.w[IX]));
	m_regs.w[IX] += -4 * m_DF + 2;
	clk(7);
}


inline void v30mz_cpu_device::i_movsb()
{
	uint8_t tmp = read_byte(default_base(DS0), m_regs.w[IX]);
	write_byte(m_sregs[DS1], m_regs.w[IY], tmp);
	m_regs.w[IY] += -2 * m_DF + 1;
	m_regs.w[IX] += -2 * m_DF + 1;
	clk(5);
}


inline void v30mz_cpu_device::i_movsw()
{
	uint16_t tmp = read_word(default_base(DS0), m_regs.w[IX]);
	write_word(m_sregs[DS1], m_regs.w[IY], tmp);
	m_regs.w[IY] += -4 * m_DF + 2;
	m_regs.w[IX] += -4 * m_DF + 2;
	clk(5);
}


inline void v30mz_cpu_device::i_cmpsb()
{
	m_src = read_byte(m_sregs[DS1], m_regs.w[IY]);
	m_dst = read_byte(default_base(DS0), m_regs.w[IX]);
	sub_byte();
	m_regs.w[IY] += -2 * m_DF + 1;
	m_regs.w[IX] += -2 * m_DF + 1;
	clk(6);
}


inline void v30mz_cpu_device::i_cmpsw()
{
	m_src = read_word(m_sregs[DS1], m_regs.w[IY]);
	m_dst = read_word(default_base(DS0), m_regs.w[IX]);
	sub_word();
	m_regs.w[IY] += -4 * m_DF + 2;
	m_regs.w[IX] += -4 * m_DF + 2;
	clk(6);
}


inline void v30mz_cpu_device::i_stosb()
{
	write_byte(m_sregs[DS1], m_regs.w[IY], m_regs.b[AL]);
	m_regs.w[IY] += -2 * m_DF + 1;
	clk(3);
}


inline void v30mz_cpu_device::i_stosw()
{
	write_word(m_sregs[DS1], m_regs.w[IY], m_regs.w[AW]);
	m_regs.w[IY] += -4 * m_DF + 2;
	clk(3);
}


inline void v30mz_cpu_device::i_lodsb()
{
	m_regs.b[AL] = read_byte(default_base(DS0), m_regs.w[IX]);
	m_regs.w[IX] += -2 * m_DF + 1;
	clk(3);
}


inline void v30mz_cpu_device::i_lodsw()
{
	m_regs.w[AW] = read_word(default_base(DS0), m_regs.w[IX]);
	m_regs.w[IX] += -4 * m_DF + 2;
	clk(3);
}


inline void v30mz_cpu_device::i_scasb()
{
	m_src = read_byte(m_sregs[DS1], m_regs.w[IY]);
	m_dst = m_regs.b[AL];
	sub_byte();
	m_regs.w[IY] += -2 * m_DF + 1;
	clk(4);
}


inline void v30mz_cpu_device::i_scasw()
{
	m_src = read_word(m_sregs[DS1], m_regs.w[IY]);
	m_dst = m_regs.w[AW];
	sub_word();
	m_regs.w[IY] += -4 * m_DF + 2;
	clk(4);
}


inline void v30mz_cpu_device::i_popf()
{
	uint32_t tmp = pop();

	expand_flags(tmp);
	clk(3);
	if (m_TF)
	{
		m_fire_trap = 1;
	}
}


inline void v30mz_cpu_device::add_byte()
{
	uint32_t res = (m_dst & 0xff) + (m_src & 0xff);

	set_CF_byte(res);
	set_OF_byte_add(res, m_src, m_dst);
	set_AF(res, m_src, m_dst);
	set_SZPF_byte(res);
	m_dst = res & 0xff;
}


inline void v30mz_cpu_device::add_word()
{
	uint32_t res = (m_dst & 0xffff) + (m_src & 0xffff);

	set_CF_word(res);
	set_OF_word_add(res, m_src, m_dst);
	set_AF(res, m_src, m_dst);
	set_SZPF_word(res);
	m_dst = res & 0xffff;
}


inline void v30mz_cpu_device::sub_byte()
{
	uint32_t res = (m_dst & 0xff) - (m_src & 0xff);

	set_CF_byte(res);
	set_OF_byte_sub(res, m_src, m_dst);
	set_AF(res, m_src, m_dst);
	set_SZPF_byte(res);
	m_dst = res & 0xff;
}


inline void v30mz_cpu_device::sub_word()
{
	uint32_t res = (m_dst & 0xffff) - (m_src & 0xffff);

	set_CF_word(res);
	set_OF_word_sub(res, m_src, m_dst);
	set_AF(res, m_src, m_dst);
	set_SZPF_word(res);
	m_dst = res & 0xffff;
}


inline void v30mz_cpu_device::or_byte()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_byte(m_dst);
}


inline void v30mz_cpu_device::or_word()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_word(m_dst);
}


inline void v30mz_cpu_device::and_byte()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_byte(m_dst);
}


inline void v30mz_cpu_device::and_word()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_word(m_dst);
}


inline void v30mz_cpu_device::xor_byte()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_byte(m_dst);
}


inline void v30mz_cpu_device::xor_word()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_word(m_dst);
}


inline void v30mz_cpu_device::rol_byte()
{
	m_CarryVal = m_dst & 0x80;
	m_dst = (m_dst << 1) | (CF ? 1 : 0);
}


inline void v30mz_cpu_device::rol_word()
{
	m_CarryVal = m_dst & 0x8000;
	m_dst = (m_dst << 1) | (CF ? 1 : 0);
}


inline void v30mz_cpu_device::ror_byte()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) | (CF ? 0x80 : 0x00);
}


inline void v30mz_cpu_device::ror_word()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) + (CF ? 0x8000 : 0x0000);
}


inline void v30mz_cpu_device::rolc_byte()
{
	m_dst = (m_dst << 1) | (CF ? 1 : 0);
	set_CF_byte(m_dst);
}


inline void v30mz_cpu_device::rolc_word()
{
	m_dst = (m_dst << 1) | (CF ? 1 : 0);
	set_CF_word(m_dst);
}


inline void v30mz_cpu_device::rorc_byte()
{
	m_dst |= (CF ? 0x100 : 0x00);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}


inline void v30mz_cpu_device::rorc_word()
{
	m_dst |= (CF ? 0x10000 : 0);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}


inline void v30mz_cpu_device::shl_byte(uint8_t c)
{
	m_icount -= c;
	m_dst <<= c;
	set_CF_byte(m_dst);
	set_SZPF_byte(m_dst);
	store_ea_rm_byte(m_dst);
}


inline void v30mz_cpu_device::shl_word(uint8_t c)
{
	m_icount -= c;
	m_dst <<= c;
	set_CF_word(m_dst);
	set_SZPF_word(m_dst);
	store_ea_rm_word(m_dst);
}


inline void v30mz_cpu_device::shr_byte(uint8_t c)
{
	m_icount -= c;
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_byte(m_dst);
	store_ea_rm_byte(m_dst);
}


inline void v30mz_cpu_device::shr_word(uint8_t c)
{
	m_icount -= c;
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_word(m_dst);
	store_ea_rm_word(m_dst);
}


inline void v30mz_cpu_device::shra_byte(uint8_t c)
{
	m_icount -= c;
	m_dst = ((int8_t)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_byte(m_dst);
	store_ea_rm_byte(m_dst);
}


inline void v30mz_cpu_device::shra_word(uint8_t c)
{
	m_icount -= c;
	m_dst = ((int16_t)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_word(m_dst);
	store_ea_rm_word(m_dst);
}


inline void v30mz_cpu_device::xchg_AW_reg(uint8_t reg)
{
	uint16_t tmp = m_regs.w[reg];

	m_regs.w[reg] = m_regs.w[AW];
	m_regs.w[AW] = tmp;
}


inline void v30mz_cpu_device::inc_word_reg(uint8_t reg)
{
	uint32_t tmp = m_regs.w[reg];
	uint32_t tmp1 = tmp+1;

	m_OverVal = (tmp == 0x7fff);
	set_AF(tmp1, tmp, 1);
	set_SZPF_word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void v30mz_cpu_device::dec_word_reg(uint8_t reg)
{
	uint32_t tmp = m_regs.w[reg];
	uint32_t tmp1 = tmp-1;

	m_OverVal = (tmp == 0x8000);
	set_AF(tmp1, tmp, 1);
	set_SZPF_word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void v30mz_cpu_device::push(uint16_t data)
{
	m_regs.w[SP] -= 2;
	write_word(m_sregs[SS], m_regs.w[SP], data);
}


inline uint16_t v30mz_cpu_device::pop()
{
	uint16_t data = read_word(m_sregs[SS], m_regs.w[SP]);

	m_regs.w[SP] += 2;
	return data;
}


inline void v30mz_cpu_device::jmp(bool cond)
{
	int rel  = (int)((int8_t)fetch());

	if (cond)
	{
		m_ip += rel;
		init_prefetch();
		clk(9);
	}
	clk(1);
}


inline void v30mz_cpu_device::adj4(int8_t param1, int8_t param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		uint16_t tmp;
		tmp = m_regs.b[AL] + param1;
		m_regs.b[AL] = tmp;
		m_AuxVal = 1;
		m_CarryVal |= tmp & 0x100;
	}
	if (CF || (m_regs.b[AL]>0x9f))
	{
		m_regs.b[AL] += param2;
		m_CarryVal = 1;
	}
	set_SZPF_byte(m_regs.b[AL]);
}


inline void v30mz_cpu_device::adjb(int8_t param1, int8_t param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		m_regs.b[AL] += param1;
		m_regs.b[AH] += param2;
		m_CarryVal = m_AuxVal;
		m_AuxVal = 1;
	}
	else
	{
		m_AuxVal = 0;
		m_CarryVal = 0;
	}
	m_regs.b[AL] &= 0x0F;
}


void v30mz_cpu_device::interrupt(int int_num)
{
	push(compress_flags());
	clk(2);
	m_TF = m_IF = 0;

	if (int_num == -1)
	{
		standard_irq_callback(0);
		int_num = m_vector_func();

		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
	}

	uint16_t dest_off = read_word(0, int_num * 4 + 0);
	uint16_t dest_seg = read_word(0, int_num * 4 + 2);

	push(m_sregs[PS]);
	push(m_ip);
	m_ip = dest_off;
	m_sregs[PS] = dest_seg;
	init_prefetch();
}


void v30mz_cpu_device::execute_set_input(int inptnum, int state)
{
	if (inptnum == INPUT_LINE_NMI)
	{
		if (m_nmi_state == state)
		{
			return;
		}
		m_nmi_state = state;
		if (state != CLEAR_LINE)
		{
			m_pending_irq |= NMI_IRQ;
		}
	}
	else
	{
		m_irq_state = state;
		if (state == CLEAR_LINE)
		{
			m_pending_irq &= ~INT_IRQ;
		}
		else
		{
			m_pending_irq |= INT_IRQ;
		}
	}
}


std::unique_ptr<util::disasm_interface> v30mz_cpu_device::create_disassembler()
{
	return std::make_unique<nec_disassembler>(this);
}


void v30mz_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_seg_prefix_next)
		{
			m_seg_prefix = true;
			m_seg_prefix_next = false;
		}
		else
		{
			m_seg_prefix = false;

			// Dispatch IRQ
			if (m_pending_irq && m_no_interrupt == 0)
			{
				if (m_pending_irq & NMI_IRQ)
				{
					interrupt(NMI_INT);
					m_pending_irq &= ~NMI_IRQ;
				}
				else if (m_IF)
				{
					// the actual vector is retrieved after pushing flags
					// and clearing the IF
					interrupt(-1);
				}
			}

			// No interrupt allowed between last instruction and this one
			if (m_no_interrupt)
			{
				m_no_interrupt--;
			}

			// trap should allow one instruction to be executed
			if (m_fire_trap)
			{
				if (m_fire_trap >= 2)
				{
					interrupt(BREAK_INT);
					m_fire_trap = 0;
				}
				else
				{
					m_fire_trap++;
				}
			}
		}

		if (m_prefetch_fill_needed) {
			for (int i = 0; i < PREFETCH_QUEUE_SIZE; i++) {
				read_prefetch();
			}
			m_prefetch_fill_needed = false;
		}

		debugger_instruction_hook(pc());

		uint8_t op = fetch_op();

		switch (op)
		{
			case 0x00: // i_add_br8
				def_br8();
				add_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x01: // i_add_wr16
				def_wr16();
				add_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x02: // i_add_r8b
				def_r8b();
				add_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x03: // i_add_r16w
				def_r16w();
				add_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x04: // i_add_ald8
				def_ald8();
				add_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x05: // i_add_axd16
				def_awd16();
				add_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x06: // i_push_es
				push(m_sregs[DS1]);
				clk(2);
				break;

			case 0x07: // i_pop_es
				m_sregs[DS1] = pop();
				clk(3);
				break;

			case 0x08: // i_or_br8
				def_br8();
				or_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x09: // i_or_wr16
				def_wr16();
				or_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x0a: // i_or_r8b
				def_r8b();
				or_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x0b: // i_or_r16w
				def_r16w();
				or_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x0c: // i_or_ald8
				def_ald8();
				or_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x0d: // i_or_axd16
				def_awd16();
				or_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x0e: // i_push_cs
				push(m_sregs[PS]);
				clk(2);
				break;

			case 0x0f: // i_pre_nec
				clk(1);
				break;


			case 0x10: // i_adc_br8
				def_br8();
				m_src += CF ? 1 : 0;
				add_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x11: // i_adc_wr16
				def_wr16();
				m_src += CF ? 1 : 0;
				add_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x12: // i_adc_r8b
				def_r8b();
				m_src += CF ? 1 : 0;
				add_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x13: // i_adc_r16w
				def_r16w();
				m_src += CF ? 1 : 0;
				add_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x14: // i_adc_ald8
				def_ald8();
				m_src += CF ? 1 : 0;
				add_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x15: // i_adc_axd16
				def_awd16();
				m_src += CF ? 1 : 0;
				add_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x16: // i_push_ss
				push(m_sregs[SS]);
				clk(2);
				break;

			case 0x17: // i_pop_ss
				m_sregs[SS] = pop();
				clk(3);
				m_no_interrupt = 1;
				break;


			case 0x18: // i_sbb_br8
				def_br8();
				m_src += CF ? 1 : 0;
				sub_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x19: // i_sbb_wr16
				def_wr16();
				m_src += CF ? 1 : 0;
				sub_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x1a: // i_sbb_r8b
				def_r8b();
				m_src += CF ? 1 : 0;
				sub_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x1b: // i_sbb_r16w
				def_r16w();
				m_src += CF ? 1 : 0;
				sub_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x1c: // i_sbb_ald8
				def_ald8();
				m_src += CF ? 1 : 0;
				sub_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x1d: // i_sbb_axd16
				def_awd16();
				m_src += CF ? 1 : 0;
				sub_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x1e: // i_push_ds
				push(m_sregs[DS0]);
				clk(2);
				break;

			case 0x1f: // i_pop_ds
				m_sregs[DS0] = pop();
				clk(3);
				break;


			case 0x20: // i_and_br8
				def_br8();
				and_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x21: // i_and_wr16
				def_wr16();
				and_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x22: // i_and_r8b
				def_r8b();
				and_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x23: // i_and_r16w
				def_r16w();
				and_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x24: // i_and_ald8
				def_ald8();
				and_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x25: // i_and_axd16
				def_awd16();
				and_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x26: // i_es
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[DS1];
				clk(1);
				break;

			case 0x27: // i_daa
				adj4(6,0x60);
				clk(10);
				break;


			case 0x28: // i_sub_br8
				def_br8();
				sub_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x29: // i_sub_wr16
				def_wr16();
				sub_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x2a: // i_sub_r8b
				def_r8b();
				sub_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x2b: // i_sub_r16w
				def_r16w();
				sub_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x2c: // i_sub_ald8
				def_ald8();
				sub_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x2d: // i_sub_axd16
				def_awd16();
				sub_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x2e: // i_cs
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[PS];
				clk(1);
				break;

			case 0x2f: // i_das
				adj4(-6, -0x60);
				clk(10);
				break;


			case 0x30: // i_xor_br8
				def_br8();
				xor_byte();
				store_ea_rm_byte(m_dst);
				clkm(1,3);
				break;

			case 0x31: // i_xor_wr16
				def_wr16();
				xor_word();
				store_ea_rm_word(m_dst);
				clkm(1,3);
				break;

			case 0x32: // i_xor_r8b
				def_r8b();
				xor_byte();
				reg_byte(m_dst);
				clkm(1,2);
				break;

			case 0x33: // i_xor_r16w
				def_r16w();
				xor_word();
				reg_word(m_dst);
				clkm(1,2);
				break;

			case 0x34: // i_xor_ald8
				def_ald8();
				xor_byte();
				m_regs.b[AL] = m_dst;
				clk(1);
				break;

			case 0x35: // i_xor_axd16
				def_awd16();
				xor_word();
				m_regs.w[AW] = m_dst;
				clk(1);
				break;

			case 0x36: // i_ss
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[SS];
				clk(1);
				break;

			case 0x37: // i_aaa
				adjb(6, (m_regs.b[AL] > 0xf9) ? 2 : 1);
				clk(9);
				break;


			case 0x38: // i_cmp_br8
				def_br8();
				sub_byte();
				clkm(1,2);
				break;

			case 0x39: // i_cmp_wr16
				def_wr16();
				sub_word();
				clkm(1,2);
				break;

			case 0x3a: // i_cmp_r8b
				def_r8b();
				sub_byte();
				clkm(1,2);
				break;

			case 0x3b: // i_cmp_r16w
				def_r16w();
				sub_word();
				clkm(1,2);
				break;

			case 0x3c: // i_cmp_ald8
				def_ald8();
				sub_byte();
				clk(1);
				break;

			case 0x3d: // i_cmp_axd16
				def_awd16();
				sub_word();
				clk(1);
				break;

			case 0x3e: // i_ds
				m_seg_prefix_next = true;
				m_prefix_base = m_sregs[DS0];
				clk(1);
				break;

			case 0x3f: // i_aas
				adjb(-6, (m_regs.b[AL] < 6) ? -2 : -1);
				clk(9);
				break;


			case 0x40: // i_inc_ax
				inc_word_reg(AW);
				clk(1);
				break;

			case 0x41: // i_inc_cx
				inc_word_reg(CW);
				clk(1);
				break;

			case 0x42: // i_inc_dx
				inc_word_reg(DW);
				clk(1);
				break;

			case 0x43: // i_inc_bx
				inc_word_reg(BW);
				clk(1);
				break;

			case 0x44: // i_inc_sp
				inc_word_reg(SP);
				clk(1);
				break;

			case 0x45: // i_inc_bp
				inc_word_reg(BP);
				clk(1);
				break;

			case 0x46: // i_inc_si
				inc_word_reg(IX);
				clk(1);
				break;

			case 0x47: // i_inc_di
				inc_word_reg(IY);
				clk(1);
				break;


			case 0x48: // i_dec_ax
				dec_word_reg(AW);
				clk(1);
				break;

			case 0x49: // i_dec_cx
				dec_word_reg(CW);
				clk(1);
				break;

			case 0x4a: // i_dec_dx
				dec_word_reg(DW);
				clk(1);
				break;

			case 0x4b: // i_dec_bx
				dec_word_reg(BW);
				clk(1);
				break;

			case 0x4c: // i_dec_sp
				dec_word_reg(SP);
				clk(1);
				break;

			case 0x4d: // i_dec_bp
				dec_word_reg(BP);
				clk(1);
				break;

			case 0x4e: // i_dec_si
				dec_word_reg(IX);
				clk(1);
				break;

			case 0x4f: // i_dec_di
				dec_word_reg(IY);
				clk(1);
				break;


			case 0x50: // i_push_ax
				push(m_regs.w[AW]);
				clk(1);
				break;

			case 0x51: // i_push_cx
				push(m_regs.w[CW]);
				clk(1);
				break;

			case 0x52: // i_push_dx
				push(m_regs.w[DW]);
				clk(1);
				break;

			case 0x53: // i_push_bx
				push(m_regs.w[BW]);
				clk(1);
				break;

			case 0x54: // i_push_sp
				push(m_regs.w[SP]);
				clk(1);
				break;

			case 0x55: // i_push_bp
				push(m_regs.w[BP]);
				clk(1);
				break;

			case 0x56: // i_push_si
				push(m_regs.w[IX]);
				clk(1);
				break;

			case 0x57: // i_push_di
				push(m_regs.w[IY]);
				clk(1);
				break;


			case 0x58: // i_pop_ax
				m_regs.w[AW] = pop();
				clk(1);
				break;

			case 0x59: // i_pop_cx
				m_regs.w[CW] = pop();
				clk(1);
				break;

			case 0x5a: // i_pop_dx
				m_regs.w[DW] = pop();
				clk(1);
				break;

			case 0x5b: // i_pop_bx
				m_regs.w[BW] = pop();
				clk(1);
				break;

			case 0x5c: // i_pop_sp
				m_regs.w[SP] = pop();
				clk(1);
				break;

			case 0x5d: // i_pop_bp
				m_regs.w[BP] = pop();
				clk(1);
				break;

			case 0x5e: // i_pop_si
				m_regs.w[IX] = pop();
				clk(1);
				break;

			case 0x5f: // i_pop_di
				m_regs.w[IY] = pop();
				clk(1);
				break;


			case 0x60: // i_pusha
				{
					uint32_t tmp = m_regs.w[SP];

					push(m_regs.w[AW]);
					push(m_regs.w[CW]);
					push(m_regs.w[DW]);
					push(m_regs.w[BW]);
					push(tmp);
					push(m_regs.w[BP]);
					push(m_regs.w[IX]);
					push(m_regs.w[IY]);
					clk(9);
				}
				break;

			case 0x61: // i_popa
				m_regs.w[IY] = pop();
				m_regs.w[IX] = pop();
				m_regs.w[BP] = pop();
								pop();
				m_regs.w[BW] = pop();
				m_regs.w[DW] = pop();
				m_regs.w[CW] = pop();
				m_regs.w[AW] = pop();
				clk(8);
				break;

			case 0x62: // i_chkind
				{
					uint32_t low,high,tmp;
					m_modrm = fetch();
					low = get_rm_word();
					high = get_next_rm_word();
					tmp = reg_word();
					if (tmp < low || tmp > high)
					{
						interrupt(CHKIND_INT);
						clk(20);
					}
					else
					{
						clk(13);
					}
					logerror("%s: %06x: bound %04x high %04x low %04x tmp\n", tag(), pc(), high, low, tmp);
				}
				break;

			case 0x64:  // REPNC not supported by v30mz
				fatalerror("%s: %06x: REPNC is not supported by v30mz\n", tag(), pc());
				{
					uint8_t next = repx_op();
					uint16_t c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  clk(2); if (c) do { i_insb();  c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  clk(2); if (c) do { i_insw();  c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  clk(2); if (c) do { i_outsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  clk(2); if (c) do { i_outsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  clk(2); if (c) do { i_movsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  clk(2); if (c) do { i_movsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  clk(2); if (c) do { i_cmpsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  clk(2); if (c) do { i_cmpsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  clk(2); if (c) do { i_stosb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  clk(2); if (c) do { i_stosw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  clk(2); if (c) do { i_lodsb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  clk(2); if (c) do { i_lodsw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  clk(2); if (c) do { i_scasb(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  clk(2); if (c) do { i_scasw(); c--; } while (c>0 && !CF); m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPNC invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						m_pfp--;
						if (m_prefetch_queue_tail == 0)
							m_prefetch_queue_tail = PREFETCH_QUEUE_SIZE - 1;
						else
							m_prefetch_queue_tail--;
						if (m_prefetch_queue_head == 0)
							m_prefetch_queue_head = PREFETCH_QUEUE_SIZE - 1;
						else
							m_prefetch_queue_head--;
						break;
					}
				}
				break;


			case 0x65: // REPC not supported by v30mz
				fatalerror("%s: %06x: REPC is not supported by v30mz\n", tag(), pc());
				{
					uint8_t next = repx_op();
					uint16_t c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  clk(2); if (c) do { i_insb();  c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  clk(2); if (c) do { i_insw();  c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  clk(2); if (c) do { i_outsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  clk(2); if (c) do { i_outsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  clk(2); if (c) do { i_movsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  clk(2); if (c) do { i_movsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  clk(2); if (c) do { i_cmpsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  clk(2); if (c) do { i_cmpsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  clk(2); if (c) do { i_stosb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  clk(2); if (c) do { i_stosw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  clk(2); if (c) do { i_lodsb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  clk(2); if (c) do { i_lodsw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  clk(2); if (c) do { i_scasb(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  clk(2); if (c) do { i_scasw(); c--; } while (c>0 && CF);    m_regs.w[CW]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPC invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						m_pfp--;
						if (m_prefetch_queue_tail == 0)
							m_prefetch_queue_tail = PREFETCH_QUEUE_SIZE - 1;
						else
							m_prefetch_queue_tail--;
						if (m_prefetch_queue_head == 0)
							m_prefetch_queue_head = PREFETCH_QUEUE_SIZE - 1;
						else
							m_prefetch_queue_head--;
						break;
					}
				}
				break;


			case 0x68: // i_push_d16
				push(fetch_word());
				clk(1);
				break;

			case 0x69: // i_imul_d16
				{
					uint32_t tmp;
					def_r16w();
					tmp = fetch_word();
					m_dst = (int32_t)((int16_t)m_src) * (int32_t)((int16_t)tmp);
					m_CarryVal = m_OverVal = (((int32_t)m_dst) >> 15 != 0) && (((int32_t)m_dst) >> 15 != -1);
					reg_word(m_dst);
					clkm(3,4);
				}
				break;

			case 0x6a: // i_push_d8
				push( (uint16_t)((int16_t)((int8_t)fetch())));
				clk(1);
				break;

			case 0x6b: // i_imul_d8
				{
					uint32_t src2;
					def_r16w();
					src2 = (uint16_t)((int16_t)((int8_t)fetch()));
					m_dst = (int32_t)((int16_t)m_src) * (int32_t)((int16_t)src2);
					m_CarryVal = m_OverVal = (((int32_t)m_dst) >> 15 != 0) && (((int32_t)m_dst) >> 15 != -1);
					reg_word(m_dst);
					clkm(3,4);
				}
				break;

			case 0x6c: // i_insb
				i_insb();
				break;

			case 0x6d: // i_insw
				i_insw();
				break;

			case 0x6e: // i_outsb
				i_outsb();
				break;

			case 0x6f: // i_outsw
				i_outsw();
				break;


			case 0x70: // i_jo
				jmp( OF);
				break;

			case 0x71: // i_jno
				jmp(!OF);
				break;

			case 0x72: // i_jc
				jmp( CF);
				break;

			case 0x73: // i_jnc
				jmp(!CF);
				break;

			case 0x74: // i_jz
				jmp( ZF);
				break;

			case 0x75: // i_jnz
				jmp(!ZF);
				break;

			case 0x76: // i_jce
				jmp(CF || ZF);
				break;

			case 0x77: // i_jnce
				jmp(!(CF || ZF));
				break;

			case 0x78: // i_js
				jmp( SF);
				break;

			case 0x79: // i_jns
				jmp(!SF);
				break;

			case 0x7a: // i_jp
				jmp( PF);
				break;

			case 0x7b: // i_jnp
				jmp(!PF);
				break;

			case 0x7c: // i_jl
				jmp((SF != OF) && (!ZF));
				break;

			case 0x7d: // i_jnl
				jmp((ZF) || (SF == OF));
				break;

			case 0x7e: // i_jle
				jmp((ZF) || (SF != OF));
				break;

			case 0x7f: // i_jnle
				jmp((SF == OF) && (!ZF));
				break;


			case 0x80: // i_80pre
				m_modrm = fetch();
				m_dst = get_rm_byte();
				m_src = fetch();
				if (m_modrm >=0xc0)              { clk(1); }
				else if ((m_modrm & 0x38)==0x38) { clk(2); }
				else                             { clk(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      add_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x08:                      or_byte();  store_ea_rm_byte(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; add_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; sub_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x20:                      and_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x28:                      sub_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x30:                      xor_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x38:                      sub_byte();                         break;  // CMP
				}
				break;


			case 0x81: // i_81pre
				m_modrm = fetch();
				m_dst = get_rm_word();
				m_src = fetch_word();
				if (m_modrm >=0xc0)              { clk(1); }
				else if ((m_modrm & 0x38)==0x38) { clk(2); }
				else                             { clk(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      add_word(); store_ea_rm_word(m_dst);   break;
				case 0x08:                      or_word();  store_ea_rm_word(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; add_word(); store_ea_rm_word(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; sub_word(); store_ea_rm_word(m_dst);   break;
				case 0x20:                      and_word(); store_ea_rm_word(m_dst);   break;
				case 0x28:                      sub_word(); store_ea_rm_word(m_dst);   break;
				case 0x30:                      xor_word(); store_ea_rm_word(m_dst);   break;
				case 0x38:                      sub_word();                         break;  // CMP
				}
				break;


			case 0x82: // i_82pre
				m_modrm = fetch();
				m_dst = get_rm_byte();
				m_src = (int8_t)fetch();
				if (m_modrm >=0xc0)                { clk(1); }
				else if ((m_modrm & 0x38) == 0x38) { clk(2); }
				else                               { clk(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      add_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x08:                      or_byte();  store_ea_rm_byte(m_dst);   break;
				case 0x10: m_src += CF ? 1 : 0; add_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x18: m_src += CF ? 1 : 0; sub_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x20:                      and_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x28:                      sub_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x30:                      xor_byte(); store_ea_rm_byte(m_dst);   break;
				case 0x38:                      sub_byte();                         break; // CMP
				}
				break;


			case 0x83: // i_83pre
				m_modrm = fetch();
				m_dst = get_rm_word();
				m_src = (int8_t)fetch();
				if ( m_modrm >= 0xc0)              { clk(1); }
				else if ((m_modrm & 0x38) == 0x38) { clk(2); }
				else                               { clk(3); }
				switch (m_modrm & 0x38)
				{
				case 0x00:                      add_word(); store_ea_rm_word(m_dst); break;
				case 0x08:                      or_word();  store_ea_rm_word(m_dst); break;
				case 0x10: m_src += CF ? 1 : 0; add_word(); store_ea_rm_word(m_dst); break;
				case 0x18: m_src += CF ? 1 : 0; sub_word(); store_ea_rm_word(m_dst); break;
				case 0x20:                      and_word(); store_ea_rm_word(m_dst); break;
				case 0x28:                      sub_word(); store_ea_rm_word(m_dst); break;
				case 0x30:                      xor_word(); store_ea_rm_word(m_dst); break;
				case 0x38:                      sub_word();                       break; // CMP
				}
				break;


			case 0x84: // i_test_br8
				def_br8();
				and_byte();
				clkm(1,2);
				break;

			case 0x85: // i_test_wr16
				def_wr16();
				and_word();
				clkm(1,2);
				break;

			case 0x86: // i_xchg_br8
				def_br8();
				reg_byte(m_dst);
				store_ea_rm_byte(m_src);
				clkm(3,5);
				break;

			case 0x87: // i_xchg_wr16
				def_wr16();
				reg_word(m_dst);
				store_ea_rm_word(m_src);
				clkm(3,5);
				break;


			case 0x88: // i_mov_br8
				m_modrm = fetch();
				m_src = reg_byte();
				put_rm_byte(m_src);
				clk(1);
				break;

			case 0x89: // i_mov_wr16
				m_modrm = fetch();
				m_src = reg_word();
				put_rm_word(m_src);
				clk(1);
				break;

			case 0x8a: // i_mov_r8b
				m_modrm = fetch();
				m_src = get_rm_byte();
				reg_byte(m_src);
				clk(1);
				break;

			case 0x8b: // i_mov_r16w
				m_modrm = fetch();
				m_src = get_rm_word();
				reg_word(m_src);
				clk(1);
				break;

			case 0x8c: // i_mov_wsreg
				m_modrm = fetch();
				put_rm_word(m_sregs[(m_modrm & 0x38) >> 3]);
				clkm(1,3);
				break;

			case 0x8d: // i_lea
				m_modrm = fetch();
				get_ea();
				reg_word(m_eo);
				clk(1);
				break;

			case 0x8e: // i_mov_sregw
				m_modrm = fetch();
				m_src = get_rm_word();
				clkm(2,3);
				switch (m_modrm & 0x38)
				{
				case 0x00:  // mov ds1,ew
					m_sregs[DS1] = m_src;
					break;
				case 0x08:  // mov cs,ew
					m_sregs[PS] = m_src;
					break;
				case 0x10:  // mov ss,ew
					m_sregs[SS] = m_src;
					break;
				case 0x18:  // mov ds0,ew
					m_sregs[DS0] = m_src;
					break;
				default:
					logerror("%s: %06x: Mov Sreg - Invalid register\n", tag(), pc());
				}
				m_no_interrupt = 1;
				break;

			case 0x8f: // i_popw
				m_modrm = fetch();
				put_rm_word(pop());
				clkm(1,3);
				break;

			case 0x90: // i_nop
				clk(1);
				break;

			case 0x91: // i_xchg_axcx
				xchg_AW_reg(CW);
				clk(3);
				break;

			case 0x92: // i_xchg_axdx
				xchg_AW_reg(DW);
				clk(3);
				break;

			case 0x93: // i_xchg_axbx
				xchg_AW_reg(BW);
				clk(3);
				break;

			case 0x94: // i_xchg_axsp
				xchg_AW_reg(SP);
				clk(3);
				break;

			case 0x95: // i_xchg_axbp
				xchg_AW_reg(BP);
				clk(3);
				break;

			case 0x96: // i_xchg_axsi
				xchg_AW_reg(IX);
				clk(3);
				break;

			case 0x97: // i_xchg_axdi
				xchg_AW_reg(IY);
				clk(3);
				break;


			case 0x98: // i_cbw
				m_regs.b[AH] = (m_regs.b[AL] & 0x80) ? 0xff : 0;
				clk(1);
				break;

			case 0x99: // i_cwd
				m_regs.w[DW] = (m_regs.b[AH] & 0x80) ? 0xffff : 0;
				clk(1);
				break;

			case 0x9a: // i_call_far
				{
					uint16_t tmp = fetch_word();
					uint16_t tmp2 = fetch_word();
					push(m_sregs[PS]);
					push(m_ip);
					m_ip = tmp;
					m_sregs[PS] = tmp2;
					init_prefetch();
					clk(10);
				}
				break;

			case 0x9b: // i_wait
				logerror("%s: %06x: Hardware POLL\n", tag(), pc());
				break;

			case 0x9c: // i_pushf
				push(compress_flags());
				clk(2);
				break;

			case 0x9d: // i_popf
				i_popf();
				break;

			case 0x9e: // i_sahf
				{
					uint32_t tmp = (compress_flags() & 0xff00) | (m_regs.b[AH] & 0xd5);
					expand_flags(tmp);
					clk(4);
				}
				break;

			case 0x9f: // i_lahf
				m_regs.b[AH] = compress_flags();
				clk(2);
				break;


			case 0xa0: // i_mov_aldisp
				m_regs.b[AL] = read_byte(default_base(DS0), fetch_word());
				clk(1);
				break;

			case 0xa1: // i_mov_axdisp
				m_regs.w[AW] = read_word(default_base(DS0), fetch_word());
				clk(1);
				break;

			case 0xa2: // i_mov_dispal
				write_byte(default_base(DS0), fetch_word(), m_regs.b[AL]);
				clk(1);
				break;

			case 0xa3: // i_mov_dispax
				write_word(default_base(DS0), fetch_word(), m_regs.w[AW]);
				clk(1);
				break;

			case 0xa4: // i_movsb
				i_movsb();
				break;

			case 0xa5: // i_movsw
				i_movsw();
				break;

			case 0xa6: // i_cmpsb
				i_cmpsb();
				break;

			case 0xa7: // i_cmpsw
				i_cmpsw();
				break;


			case 0xa8: // i_test_ald8
				def_ald8();
				and_byte();
				clk(1);
				break;

			case 0xa9: // i_test_axd16
				def_awd16();
				and_word();
				clk(1);
				break;

			case 0xaa: // i_stosb
				i_stosb();
				break;

			case 0xab: // i_stosw
				i_stosw();
				break;

			case 0xac: // i_lodsb
				i_lodsb();
				break;

			case 0xad: // i_lodsw
				i_lodsw();
				break;

			case 0xae: // i_scasb
				i_scasb();
				break;

			case 0xaf: // i_scasw
				i_scasw();
				break;


			case 0xb0: // i_mov_ald8
				m_regs.b[AL] = fetch();
				clk(1);
				break;

			case 0xb1: // i_mov_cld8
				m_regs.b[CL] = fetch();
				clk(1);
				break;

			case 0xb2: // i_mov_dld8
				m_regs.b[DL] = fetch();
				clk(1);
				break;

			case 0xb3: // i_mov_bld8
				m_regs.b[BL] = fetch();
				clk(1);
				break;

			case 0xb4: // i_mov_ahd8
				m_regs.b[AH] = fetch();
				clk(1);
				break;

			case 0xb5: // i_mov_chd8
				m_regs.b[CH] = fetch();
				clk(1);
				break;

			case 0xb6: // i_mov_dhd8
				m_regs.b[DH] = fetch();
				clk(1);
				break;

			case 0xb7: // i_mov_bhd8
				m_regs.b[BH] = fetch();
				clk(1);
				break;


			case 0xb8: // i_mov_axd16
				m_regs.w[AW] = fetch_word();
				clk(1);
				break;

			case 0xb9: // i_mov_cxd16
				m_regs.w[CW] = fetch_word();
				clk(1);
				break;

			case 0xba: // i_mov_dxd16
				m_regs.w[DW] = fetch_word();
				clk(1);
				break;

			case 0xbb: // i_mov_bxd16
				m_regs.w[BW] = fetch_word();
				clk(1);
				break;

			case 0xbc: // i_mov_spd16
				m_regs.w[SP] = fetch_word();
				clk(1);
				break;

			case 0xbd: // i_mov_bpd16
				m_regs.w[BP] = fetch_word();
				clk(1);
				break;

			case 0xbe: // i_mov_sid16
				m_regs.w[IX] = fetch_word();
				clk(1);
				break;

			case 0xbf: // i_mov_did16
				m_regs.w[IY] = fetch_word();
				clk(1);
				break;


			case 0xc0: // i_rotshft_bd8
				{
					uint8_t c;
					m_modrm = fetch();
					m_src = get_rm_byte();
					m_dst = m_src;
					c = fetch();
					clkm(3,5);
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { rol_byte();  c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x08: do { ror_byte();  c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x10: do { rolc_byte(); c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x18: do { rorc_byte(); c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x20: shl_byte(c); break;
						case 0x28: shr_byte(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xc0 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: shra_byte(c); break;
						}
					}
				}
				break;

			case 0xc1: // i_rotshft_wd8
				{
					uint8_t c;
					m_modrm = fetch();
					m_src = get_rm_word();
					m_dst = m_src;
					c = fetch();
					clkm(3,5);
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { rol_word();  c--; } while (c > 0); store_ea_rm_word(m_dst); break;
						case 0x08: do { ror_word();  c--; } while (c > 0); store_ea_rm_word(m_dst); break;
						case 0x10: do { rolc_word(); c--; } while (c > 0); store_ea_rm_word(m_dst); break;
						case 0x18: do { rorc_word(); c--; } while (c > 0); store_ea_rm_word(m_dst); break;
						case 0x20: shl_word(c); break;
						case 0x28: shr_word(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xc1 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: shra_word(c); break;
						}
					}
				}
				break;


			case 0xc2: // i_ret_d16
				{
					uint32_t count = fetch_word();
					m_ip = pop();
					m_regs.w[SP] += count;
					init_prefetch();
					clk(6);
				}
				break;

			case 0xc3: // i_ret
				m_ip = pop();
				init_prefetch();
				clk(6);
				break;

			case 0xc4: // i_les_dw
				m_modrm = fetch();
				reg_word(get_rm_word());
				m_sregs[DS1] = get_next_rm_word();
				clk(6);
				break;

			case 0xc5: // i_lds_dw
				m_modrm = fetch();
				reg_word(get_rm_word());
				m_sregs[DS0] = get_next_rm_word();
				clk(6);
				break;

			case 0xc6: // i_mov_bd8
				m_modrm = fetch();
				put_imm_rm_byte();
				clk(1);
				break;

			case 0xc7: // i_mov_wd16
				m_modrm = fetch();
				put_imm_rm_word();
				clk(1);
				break;


			case 0xc8: // i_enter
				{
					uint16_t nb = fetch();
					uint32_t level;

					clk(8);
					nb |= fetch() << 8;
					level = fetch();
					push(m_regs.w[BP]);
					m_regs.w[BP] = m_regs.w[SP];
					m_regs.w[SP] -= nb;
					for (int i = 1; i < level; i++)
					{
						push(read_word(default_base(SS), m_regs.w[BP] - i*2));
						clk(4);
					}
					if (level)
					{
						push(m_regs.w[BP]);
						clk((level == 1) ? 2 : 3);
					}
				}
				break;

			case 0xc9: // i_leave
				m_regs.w[SP] = m_regs.w[BP];
				m_regs.w[BP] = pop();
				clk(2);
				break;

			case 0xca: // i_retf_d16
				{
					uint32_t count = fetch_word();
					m_ip = pop();
					m_sregs[PS] = pop();
					m_regs.w[SP] += count;
					init_prefetch();
					clk(9);
				}
				break;

			case 0xcb: // i_retf
				m_ip = pop();
				m_sregs[PS] = pop();
				init_prefetch();
				clk(8);
				break;

			case 0xcc: // i_int3
				interrupt(BRK_3_INT);
				clk(9);
				break;

			case 0xcd: // i_int
				interrupt(fetch());
				clk(10);
				break;

			case 0xce: // i_into
				if (OF)
				{
					interrupt(BRKV_INT);
					clk(7);
				}
				clk(6);
				break;

			case 0xcf: // i_iret
				m_ip = pop();
				m_sregs[PS] = pop();
				i_popf();
				init_prefetch();
				clk(10);
				break;


			case 0xd0: // i_rotshft_b
				m_modrm = fetch();
				m_src = get_rm_byte();
				m_dst = m_src;
				clkm(1,3);
				switch (m_modrm & 0x38)
				{
				case 0x00: rol_byte();  store_ea_rm_byte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x08: ror_byte();  store_ea_rm_byte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x10: rolc_byte(); store_ea_rm_byte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x18: rorc_byte(); store_ea_rm_byte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x20: shl_byte(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x28: shr_byte(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
				case 0x30: logerror("%s: %06x: Undefined opcode 0xd0 0x30 (SHLA)\n", tag(), pc()); break;
				case 0x38: shra_byte(1); m_OverVal = 0; break;
				}
				break;

			case 0xd1: // i_rotshft_w
				m_modrm = fetch();
				m_src = get_rm_word();
				m_dst = m_src;
				clkm(1,3);
				switch (m_modrm & 0x38)
				{
				case 0x00: rol_word();  store_ea_rm_word(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x08: ror_word();  store_ea_rm_word(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x10: rolc_word(); store_ea_rm_word(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x18: rorc_word(); store_ea_rm_word(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
				case 0x20: shl_word(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
				case 0x28: shr_word(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
				case 0x30: logerror("%s: %06x: Undefined opcode 0xd1 0x30 (SHLA)\n", tag(), pc()); break;
				case 0x38: shra_word(1); m_OverVal = 0; break;
				}
				break;

			case 0xd2: // i_rotshft_bcl
				{
					uint8_t c;

					m_modrm = fetch();
					m_src = get_rm_byte();
					m_dst = m_src;
					c = m_regs.b[CL];
					clkm(3,5);
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { rol_byte();  c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x08: do { ror_byte();  c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x10: do { rolc_byte(); c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x18: do { rorc_byte(); c--; } while (c > 0); store_ea_rm_byte(m_dst); break;
						case 0x20: shl_byte(c); break;
						case 0x28: shr_byte(c); break;
						case 0x30: logerror("%s: %06x: Undefined opcode 0xd2 0x30 (SHLA)\n", tag(), pc()); break;
						case 0x38: shra_byte(c); break;
						}
					}
				}
				break;

			case 0xd3: // i_rotshft_wcl
				{
					uint8_t c;

					m_modrm = fetch();
					m_src = get_rm_word();
					m_dst = m_src;
					c = m_regs.b[CL];
					clkm(3,5);
					if (c)
					{
						switch (m_modrm & 0x38)
						{
							case 0x00: do { rol_word();  c--; } while (c > 0); store_ea_rm_word(m_dst); break;
							case 0x08: do { ror_word();  c--; } while (c > 0); store_ea_rm_word(m_dst); break;
							case 0x10: do { rolc_word(); c--; } while (c > 0); store_ea_rm_word(m_dst); break;
							case 0x18: do { rorc_word(); c--; } while (c > 0); store_ea_rm_word(m_dst); break;
							case 0x20: shl_word(c); break;
							case 0x28: shr_word(c); break;
							case 0x30: logerror("%s: %06x: Undefined opcode 0xd3 0x30 (SHLA)\n", tag(), pc()); break;
							case 0x38: shra_word(c); break;
						}
					}
				}
				break;

			case 0xd4: // cvtbd
				fetch();
				m_regs.b[AH] = m_regs.b[AL] / 10;
				m_regs.b[AL] %= 10;
				set_SZPF_word(m_regs.w[AW]);
				clk(17);
				break;

			case 0xd5: // cvtdb
				fetch();
				m_regs.b[AL] = m_regs.b[AH] * 10 + m_regs.b[AL];
				m_regs.b[AH] = 0;
				set_SZPF_byte(m_regs.b[AL]);
				clk(5);
				break;

			case 0xd6: // i_setalc
				m_regs.b[AL] = (CF) ? 0xff : 0x00;
				clk(3);
				logerror("%s: %06x: Undefined opcode (SETALC)\n", tag(), pc());
				break;

			case 0xd7: // i_trans
				m_regs.b[AL] = read_byte(default_base(DS0), m_regs.w[BW] + m_regs.b[AL]);
				clk(5);
				break;

			case 0xd8: // FPO1 not supported by v30mz
				m_modrm = fetch();
				clk(1);
				logerror("%s: %06x: Unimplemented floating point control %04x\n", tag(), pc(), m_modrm);
				break;


			case 0xe0: // i_loopne
				{
					int8_t disp = (int8_t)fetch();

					m_regs.w[CW]--;
					if (!ZF && m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						init_prefetch();
						clk(3);
					}
					clk(3);
				}
				break;

			case 0xe1: // i_loope
				{
					int8_t disp = (int8_t)fetch();

					m_regs.w[CW]--;
					if (ZF && m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						init_prefetch();
						clk(3);
					}
					clk(3);
				}
				break;

			case 0xe2: // i_loop
				{
					int8_t disp = (int8_t)fetch();

					m_regs.w[CW]--;
					if (m_regs.w[CW])
					{
						m_ip = m_ip + disp;
						init_prefetch();
						clk(3);
					}
					clk(2);
				}
				break;

			case 0xe3: // i_jcxz
				{
					int8_t disp = (int8_t)fetch();

					if (m_regs.w[CW] == 0)
					{
						m_ip = m_ip + disp;
						init_prefetch();
						clk(3);
					}
					clk(1);
				}
				break;

			case 0xe4: // i_inal
				m_regs.b[AL] = read_port(fetch());
				clk(6);
				break;

			case 0xe5: // i_inax
				m_regs.w[AW] = read_port_word(fetch());
				clk(6);
				break;

			case 0xe6: // i_outal
				write_port(fetch(), m_regs.b[AL]);
				clk(6);
				break;

			case 0xe7: // i_outax
				write_port_word(fetch(), m_regs.w[AW]);
				clk(6);
				break;


			case 0xe8: // i_call_d16
				{
					int16_t tmp = (int16_t)fetch_word();

					push(m_ip);
					m_ip = m_ip + tmp;
					init_prefetch();
					clk(5);
				}
				break;

			case 0xe9: // i_jmp_d16
				{
					int16_t offset = (int16_t)fetch_word();
					m_ip += offset;
					init_prefetch();
					clk(4);
				}
				break;

			case 0xea: // i_jmp_far
				{
					uint16_t tmp = fetch_word();
					uint16_t tmp1 = fetch_word();

					m_sregs[PS] = tmp1;
					m_ip = tmp;
					init_prefetch();
					clk(7);
				}
				break;

			case 0xeb: // i_jmp_d8
				{
					int tmp = (int)((int8_t)fetch());

					clk(4);
					if (tmp == -2 && m_no_interrupt == 0 && (m_pending_irq == 0) && m_icount > 0)
					{
						m_icount %= 12; // cycle skip
					}
					m_ip = (uint16_t)(m_ip + tmp);
					init_prefetch();
				}
				break;

			case 0xec: // i_inaldx
				m_regs.b[AL] = read_port(m_regs.w[DW]);
				clk(6);
				break;

			case 0xed: // i_inaxdx
				m_regs.w[AW] = read_port_word(m_regs.w[DW]);
				clk(6);
				break;

			case 0xee: // i_outdxal
				write_port(m_regs.w[DW], m_regs.b[AL]);
				clk(6);
				break;

			case 0xef: // i_outdxax
				write_port_word(m_regs.w[DW], m_regs.w[AW]);
				clk(6);
				break;


			case 0xf0: // i_lock
				logerror("%s: %06x: Warning - BUSLOCK\n", tag(), pc());
				m_no_interrupt = 1;
				clk(1);
				break;

			case 0xf2: // i_repne
				{
					uint8_t next = repx_op();
					uint16_t c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  clk(3); if (c) do { i_insb();  c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  clk(3); if (c) do { i_insw();  c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  clk(3); if (c) do { i_outsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  clk(3); if (c) do { i_outsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  clk(3); if (c) do { i_movsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  clk(3); if (c) do { i_movsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  clk(3); if (c) do { i_cmpsb(); c--; } while (c > 0 && !ZF);   m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  clk(3); if (c) do { i_cmpsw(); c--; } while (c > 0 && !ZF);   m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  clk(3); if (c) do { i_stosb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  clk(3); if (c) do { i_stosw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  clk(3); if (c) do { i_lodsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  clk(3); if (c) do { i_lodsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  clk(3); if (c) do { i_scasb(); c--; } while (c > 0 && !ZF);   m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  clk(3); if (c) do { i_scasw(); c--; } while (c > 0 && !ZF);   m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPNE invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						m_pfp--;
						if (m_prefetch_queue_tail == 0)
							m_prefetch_queue_tail = PREFETCH_MAX_SIZE - 1;
						else
							m_prefetch_queue_tail--;
						if (m_prefetch_queue_head == 0)
							m_prefetch_queue_head = PREFETCH_MAX_SIZE - 1;
						else
							m_prefetch_queue_head--;
						break;
					}
				}
				break;

			case 0xf3: // i_repe
				{
					uint8_t next = repx_op();
					uint16_t c = m_regs.w[CW];

					switch (next)
					{
					case 0x6c:  clk(3); if (c) do { i_insb();  c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  clk(3); if (c) do { i_insw();  c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  clk(3); if (c) do { i_outsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  clk(3); if (c) do { i_outsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa4:  clk(3); if (c) do { i_movsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa5:  clk(3); if (c) do { i_movsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa6:  clk(3); if (c) do { i_cmpsb(); c--; } while (c > 0 && ZF);    m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xa7:  clk(3); if (c) do { i_cmpsw(); c--; } while (c > 0 && ZF);    m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaa:  clk(3); if (c) do { i_stosb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xab:  clk(3); if (c) do { i_stosw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xac:  clk(3); if (c) do { i_lodsb(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xad:  clk(3); if (c) do { i_lodsw(); c--; } while (c > 0);          m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xae:  clk(3); if (c) do { i_scasb(); c--; } while (c > 0 && ZF);    m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0xaf:  clk(3); if (c) do { i_scasw(); c--; } while (c > 0 && ZF);    m_regs.w[CW] = c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						logerror("%s: %06x: REPE invalid\n", tag(), pc());
						// Decrement IP so the normal instruction will be executed next
						m_ip--;
						m_pfp--;
						if (m_prefetch_queue_tail == 0)
							m_prefetch_queue_tail = PREFETCH_MAX_SIZE - 1;
						else
							m_prefetch_queue_tail--;
						if (m_prefetch_queue_head == 0)
							m_prefetch_queue_head = PREFETCH_MAX_SIZE - 1;
						else
							m_prefetch_queue_head--;
						break;
					}
				}
				break;

			case 0xf4: // i_hlt
				m_icount = 0;
				break;

			case 0xf5: // i_cmc
				m_CarryVal = (CF ? 0 : 1);
				clk(4);
				break;

			case 0xf6: // i_f6pre
				{
					m_modrm = fetch();
					uint32_t tmp = get_rm_byte();
					switch (m_modrm & 0x38)
					{
					case 0x00:  // TEST
						tmp &= fetch();
						m_CarryVal = m_OverVal = 0;
						set_SZPF_byte(tmp);
						clkm(1,2);
						break;
					case 0x08:
						logerror("%s: %06x: Undefined opcode 0xf6 0x08\n", tag(), pc());
						break;
					case 0x10:  // NOT
						store_ea_rm_byte(~tmp);
						clkm(1,3);
						break;
					case 0x18:  // NEG, AF?
						m_CarryVal = (tmp != 0) ? 1 : 0;
						tmp = (~tmp) + 1;
						set_SZPF_byte(tmp);
						store_ea_rm_byte(tmp & 0xff);
						clkm(1,3);
						break;
					case 0x20:  // MULU
						{
							uint32_t uresult = m_regs.b[AL] * tmp;
							m_regs.w[AW] = (uint16_t)uresult;
							m_CarryVal = m_OverVal = (m_regs.b[AH] != 0) ? 1 : 0;
							clkm(3,4);
						}
						break;
					case 0x28:  // MUL
						{
							int32_t result = (int16_t)((int8_t)m_regs.b[AL]) * (int16_t)((int8_t)tmp);
							m_regs.w[AW] = (uint16_t)result;
							m_CarryVal = m_OverVal = (m_regs.b[AH] != 0) ? 1 : 0;
							clkm(3,4);
						}
						break;
					case 0x30:  // DIVU
						if (tmp)
						{
							uint32_t uresult = m_regs.w[AW];
							uint32_t uresult2 = uresult % tmp;
							if ((uresult /= tmp) > 0xff)
							{
								interrupt(DIVIDE_ERROR_INT);
							}
							else
							{
								m_regs.b[AL] = uresult;
								m_regs.b[AH] = uresult2;
							}
						}
						else
						{
							interrupt(DIVIDE_ERROR_INT);
						}
						clkm(15,16);
						break;
					case 0x38:  // DIV
						if (tmp)
						{
							int32_t result = (int16_t)m_regs.w[AW];
							int32_t result2 = result % (int16_t)((int8_t)tmp);
							if ((result /= (int16_t)((int8_t)tmp)) > 0xff)
							{
								interrupt(DIVIDE_ERROR_INT);
							}
							else
							{
								m_regs.b[AL] = result;
								m_regs.b[AH] = result2;
							}
						}
						else
						{
							interrupt(DIVIDE_ERROR_INT);
						}
						clkm(17,18);
						break;
					}
				}
				break;


			case 0xf7: // i_f7pre
				{
					uint32_t tmp,tmp2;
					uint32_t uresult,uresult2;
					int32_t result,result2;

					m_modrm = fetch();
					tmp = get_rm_word();
					switch (m_modrm & 0x38)
					{
					case 0x00:  // TEST
						tmp2 = fetch_word();
						tmp &= tmp2;
						m_CarryVal = m_OverVal = 0;
						set_SZPF_word(tmp);
						clkm(1,2);
						break;
					case 0x08:
						logerror("%s: %06x: Undefined opcode 0xf7 0x08\n", tag(), pc());
						break;
					case 0x10:  // NOT
						store_ea_rm_word(~tmp);
						clkm(1,3);
						break;
					case 0x18:  // NEG
						m_CarryVal = (tmp!=0) ? 1 : 0;
						tmp = (~tmp) + 1;
						set_SZPF_word(tmp);
						store_ea_rm_word(tmp);
						clkm(1,3);
						break;
					case 0x20:  // MULU
						uresult = m_regs.w[AW]*tmp;
						m_regs.w[AW] = uresult & 0xffff;
						m_regs.w[DW] = ((uint32_t)uresult) >> 16;
						m_CarryVal = m_OverVal = (m_regs.w[DW] != 0) ? 1 : 0;
						clkm(3,4);
						break;
					case 0x28:  // MUL
						result = (int32_t)((int16_t)m_regs.w[AW]) * (int32_t)((int16_t)tmp);
						m_regs.w[AW] = result & 0xffff;
						m_regs.w[DW] = result >> 16;
						m_CarryVal = m_OverVal = (m_regs.w[DW] != 0) ? 1 : 0;
						clkm(3,4);
						break;
					case 0x30:  // DIVU
						if (tmp)
						{
							uresult = (((uint32_t)m_regs.w[DW]) << 16) | m_regs.w[AW];
							uresult2 = uresult % tmp;
							if ((uresult /= tmp) > 0xffff)
							{
								interrupt(DIVIDE_ERROR_INT);
							}
							else
							{
								m_regs.w[AW] = uresult;
								m_regs.w[DW] = uresult2;
							}
						}
						else
						{
							interrupt(DIVIDE_ERROR_INT);
						}
						clkm(23,24);
						break;
					case 0x38:  // DIV
						if (tmp)
						{
							result = ((uint32_t)m_regs.w[DW] << 16) + m_regs.w[AW];
							result2 = result % (int32_t)((int16_t)tmp);
							if ((result /= (int32_t)((int16_t)tmp)) > 0xffff)
							{
								interrupt(DIVIDE_ERROR_INT);
							}
							else
							{
								m_regs.w[AW] = result;
								m_regs.w[DW] = result2;
							}
						}
						else
						{
							interrupt(DIVIDE_ERROR_INT);
						}
						clkm(24,25);
						break;
					}
				}
				break;


			case 0xf8: // i_clc
				m_CarryVal = 0;
				clk(4);
				break;

			case 0xf9: // i_stc
				m_CarryVal = 1;
				clk(4);
				break;

			case 0xfa: // i_di
				m_IF = 0;
				clk(4);
				break;

			case 0xfb: // i_ei
				m_IF = 1;
				clk(4);
				break;

			case 0xfc: // i_cld
				m_DF = 0;
				clk(4);
				break;

			case 0xfd: // i_std
				m_DF = 1;
				clk(4);
				break;

			case 0xfe: // i_fepre
				{
					uint32_t tmp, tmp1;
					m_modrm = fetch();
					tmp = get_rm_byte();
					switch (m_modrm & 0x38)
					{
					case 0x00:  // INC
						tmp1 = tmp + 1;
						m_OverVal = (tmp == 0x7f);
						set_AF(tmp1,tmp,1);
						set_SZPF_byte(tmp1);
						store_ea_rm_byte(tmp1);
						clkm(1,3);
						break;
					case 0x08:  // DEC
						tmp1 = tmp - 1;
						m_OverVal = (tmp == 0x80);
						set_AF(tmp1,tmp,1);
						set_SZPF_byte(tmp1);
						store_ea_rm_byte(tmp1);
						clkm(1,3);
						break;
					default:
						logerror("%s: %06x: FE Pre with unimplemented mod\n", tag(), pc());
						break;
					}
				}
				break;

			case 0xff: // i_ffpre
				{
					uint32_t tmp, tmp1;
					m_modrm = fetch();
					tmp = get_rm_word();
					switch (m_modrm & 0x38)
					{
					case 0x00:  // INC
						tmp1 = tmp + 1;
						m_OverVal = (tmp == 0x7fff);
						set_AF(tmp1, tmp, 1);
						set_SZPF_word(tmp1);
						store_ea_rm_word(tmp1);
						clkm(1,3);
						break;
					case 0x08:  // DEC
						tmp1 = tmp - 1;
						m_OverVal = (tmp == 0x8000);
						set_AF(tmp1, tmp, 1);
						set_SZPF_word(tmp1);
						store_ea_rm_word(tmp1);
						clkm(1,3);
						break;
					case 0x10:  // CALL
						push(m_ip);
						m_ip = tmp;
						init_prefetch();
						clkm(5,6);
						break;
					case 0x18:  // CALL FAR
						tmp1 = m_sregs[PS];
						m_sregs[PS] = get_next_rm_word();
						push(tmp1);
						push(m_ip);
						m_ip = tmp;
						init_prefetch();
						clkm(5,12);
						break;
					case 0x20:  // jmp
						m_ip = tmp;
						init_prefetch();
						clkm(4,5);
						break;
					case 0x28:  // jmp FAR
						m_ip = tmp;
						m_sregs[PS] = get_next_rm_word();
						init_prefetch();
						clk(10);
						break;
					case 0x30:
						push(tmp);
						clk(1);
						break;
					default:
						logerror("%s: %06x: FF Pre with unimplemented mod\n", tag(), pc());
						break;
					}
				}
				break;

		default:
			m_icount -= 10;
			logerror("%s: %06x: Invalid Opcode %02x\n", tag(), pc(), op);
			break;

		}
	}
}
