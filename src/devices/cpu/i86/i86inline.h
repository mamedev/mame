// license:BSD-3-Clause
// copyright-holders:Carl
#include "i86.h"

#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      m_parity_table[(uint8_t)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

inline uint16_t i8086_common_cpu_device::fetch_word()
{
	uint16_t data = fetch();
	data |= ( fetch() << 8 );
	return data;
}

inline uint8_t i8086_common_cpu_device::repx_op()
{
	uint8_t next = fetch_op();
	bool seg_prefix = false;
	int seg = 0;

	switch (next)
	{
	case 0x26:
		seg_prefix = true;
		seg = ES;
		break;
	case 0x2e:
		seg_prefix = true;
		seg = CS;
		break;
	case 0x36:
		seg_prefix = true;
		seg = SS;
		break;
	case 0x3e:
		seg_prefix = true;
		seg = DS;
		break;
	}

	if ( seg_prefix )
	{
		m_seg_prefix = true;
		m_seg_prefix_next = true;
		m_prefix_seg = seg;
		next = fetch_op();
		CLK(OVERRIDE);
	}

	return next;
}


inline void i8086_common_cpu_device::CLK(uint8_t op)
{
	m_icount -= m_timing[op];
}


inline void i8086_common_cpu_device::CLKM(uint8_t op_reg, uint8_t op_mem)
{
	m_icount -= ( m_modrm >= 0xc0 ) ? m_timing[op_reg] : m_timing[op_mem];
}


inline uint32_t i8086_common_cpu_device::get_ea(int size, int op)
{
	uint16_t e16;

	switch( m_modrm & 0xc7 )
	{
	case 0x00:
		m_icount -= 7;
		m_eo = m_regs.w[BX] + m_regs.w[SI];
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x01:
		m_icount -= 8;
		m_eo = m_regs.w[BX] + m_regs.w[DI];
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x02:
		m_icount -= 8;
		m_eo = m_regs.w[BP] + m_regs.w[SI];
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x03:
		m_icount -= 7;
		m_eo = m_regs.w[BP] + m_regs.w[DI];
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x04:
		m_icount -= 5;
		m_eo = m_regs.w[SI];
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x05:
		m_icount -= 5;
		m_eo = m_regs.w[DI];
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x06:
		m_icount -= 6;
		m_eo = fetch_word();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x07:
		m_icount -= 5;
		m_eo = m_regs.w[BX];
		m_ea = calc_addr(DS, m_eo, size, op);
		break;

	case 0x40:
		m_icount -= 11;
		m_eo = m_regs.w[BX] + m_regs.w[SI] + (int8_t)fetch();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x41:
		m_icount -= 12;
		m_eo = m_regs.w[BX] + m_regs.w[DI] + (int8_t)fetch();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x42:
		m_icount -= 12;
		m_eo = m_regs.w[BP] + m_regs.w[SI] + (int8_t)fetch();
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x43:
		m_icount -= 11;
		m_eo = m_regs.w[BP] + m_regs.w[DI] + (int8_t)fetch();
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x44:
		m_icount -= 9;
		m_eo = m_regs.w[SI] + (int8_t)fetch();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x45:
		m_icount -= 9;
		m_eo = m_regs.w[DI] + (int8_t)fetch();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x46:
		m_icount -= 9;
		m_eo = m_regs.w[BP] + (int8_t)fetch();
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x47:
		m_icount -= 9;
		m_eo = m_regs.w[BX] + (int8_t)fetch();
		m_ea = calc_addr(DS, m_eo, size, op);
		break;

	case 0x80:
		m_icount -= 11;
		e16 = fetch_word();
		m_eo = m_regs.w[BX] + m_regs.w[SI] + (int16_t)e16;
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x81:
		m_icount -= 12;
		e16 = fetch_word();
		m_eo = m_regs.w[BX] + m_regs.w[DI] + (int16_t)e16;
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x82:
		m_icount -= 11;
		e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[SI] + (int16_t)e16;
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x83:
		m_icount -= 11;
		e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[DI] + (int16_t)e16;
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x84:
		m_icount -= 9;
		e16 = fetch_word();
		m_eo = m_regs.w[SI] + (int16_t)e16;
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x85:
		m_icount -= 9;
		e16 = fetch_word();
		m_eo = m_regs.w[DI] + (int16_t)e16;
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	case 0x86:
		m_icount -= 9;
		e16 = fetch_word();
		m_eo = m_regs.w[BP] + (int16_t)e16;
		m_ea = calc_addr(SS, m_eo, size, op);
		break;
	case 0x87:
		m_icount -= 9;
		e16 = fetch_word();
		m_eo = m_regs.w[BX] + (int16_t)e16;
		m_ea = calc_addr(DS, m_eo, size, op);
		break;
	}
	return m_ea;
}


inline void i8086_common_cpu_device::PutbackRMByte(uint8_t data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = data;
	}
	else
	{
		write_byte(m_ea, data);
	}
}


inline void i8086_common_cpu_device::PutbackRMWord(uint16_t data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = data;
	}
	else
	{
		write_word(m_ea, data);
	}
}

inline void i8086_common_cpu_device::PutImmRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = fetch_word();
	}
	else
	{
		uint32_t addr = get_ea(2, I8086_WRITE);
		write_word(addr, fetch_word());
	}
}

inline void i8086_common_cpu_device::PutRMWord(uint16_t val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = val;
	}
	else
	{
		write_word(get_ea(2, I8086_WRITE), val);
	}
}


inline void i8086_common_cpu_device::PutRMByte(uint8_t val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = val;
	}
	else
	{
		write_byte(get_ea(1, I8086_WRITE), val);
	}
}


inline void i8086_common_cpu_device::PutImmRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = fetch();
	}
	else
	{
		uint32_t addr = get_ea(1, I8086_WRITE);
		write_byte(addr, fetch());
	}
}


inline void i8086_common_cpu_device::DEF_br8()
{
	m_modrm = fetch();
	m_src = RegByte();
	m_dst = GetRMByte();
}


inline void i8086_common_cpu_device::DEF_wr16()
{
	m_modrm = fetch();
	m_src = RegWord();
	m_dst = GetRMWord();
}


inline void i8086_common_cpu_device::DEF_r8b()
{
	m_modrm = fetch();
	m_dst = RegByte();
	m_src = GetRMByte();
}


inline void i8086_common_cpu_device::DEF_r16w()
{
	m_modrm = fetch();
	m_dst = RegWord();
	m_src = GetRMWord();
}


inline void i8086_common_cpu_device::DEF_ald8()
{
	m_src = fetch();
	m_dst = m_regs.b[AL];
}


inline void i8086_common_cpu_device::DEF_axd16()
{
	m_src = fetch_word();
	m_dst = m_regs.w[AX];
}



inline void i8086_common_cpu_device::RegByte(uint8_t data)
{
	m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ] = data;
}


inline void i8086_common_cpu_device::RegWord(uint16_t data)
{
	m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ] = data;
}


inline uint8_t i8086_common_cpu_device::RegByte()
{
	return m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ];
}


inline uint16_t i8086_common_cpu_device::RegWord()
{
	return m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ];
}


inline uint16_t i8086_common_cpu_device::GetRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ];
	}
	else
	{
		return read_word(get_ea(2, I8086_READ));
	}
}


inline uint16_t i8086_common_cpu_device::GetnextRMWord()
{
	return read_word((m_ea - m_eo) + ((m_eo + 2) & 0xffff));
}


inline uint8_t i8086_common_cpu_device::GetRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ];
	}
	else
	{
		return read_byte(get_ea(1, I8086_READ));
	}
}


inline void i8086_common_cpu_device::PutMemB(int seg, uint16_t offset, uint8_t data)
{
	write_byte(calc_addr(seg, offset, 1, I8086_WRITE), data);
}


inline void i8086_common_cpu_device::PutMemW(int seg, uint16_t offset, uint16_t data)
{
	write_word(calc_addr(seg, offset, 2, I8086_WRITE), data);
}


inline uint8_t i8086_common_cpu_device::GetMemB(int seg, uint16_t offset)
{
	return read_byte(calc_addr(seg, offset, 1, I8086_READ));
}


inline uint16_t i8086_common_cpu_device::GetMemW(int seg, uint16_t offset)
{
	return read_word(calc_addr(seg, offset, 2, I8086_READ));
}


// Setting flags

inline void i8086_common_cpu_device::set_CFB(uint32_t x)
{
	m_CarryVal = x & 0x100;
}

inline void i8086_common_cpu_device::set_CFW(uint32_t x)
{
	m_CarryVal = x & 0x10000;
}

inline void i8086_common_cpu_device::set_AF(uint32_t x,uint32_t y,uint32_t z)
{
	m_AuxVal = (x ^ (y ^ z)) & 0x10;
}

inline void i8086_common_cpu_device::set_SF(uint32_t x)
{
	m_SignVal = x;
}

inline void i8086_common_cpu_device::set_ZF(uint32_t x)
{
	m_ZeroVal = x;
}

inline void i8086_common_cpu_device::set_PF(uint32_t x)
{
	m_ParityVal = x;
}

inline void i8086_common_cpu_device::set_SZPF_Byte(uint32_t x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (int8_t)x;
}

inline void i8086_common_cpu_device::set_SZPF_Word(uint32_t x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (int16_t)x;
}

inline void i8086_common_cpu_device::set_OFW_Add(uint32_t x,uint32_t y,uint32_t z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x8000;
}

inline void i8086_common_cpu_device::set_OFB_Add(uint32_t x,uint32_t y,uint32_t z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x80;
}

inline void i8086_common_cpu_device::set_OFW_Sub(uint32_t x,uint32_t y,uint32_t z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x8000;
}

inline void i8086_common_cpu_device::set_OFB_Sub(uint32_t x,uint32_t y,uint32_t z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x80;
}


inline uint16_t i8086_common_cpu_device::CompressFlags() const
{
	return (CF ? 1 : 0)
		| (1 << 1)
		| (PF ? 4 : 0)
		| (AF ? 0x10 : 0)
		| (ZF ? 0x40 : 0)
		| (SF ? 0x80 : 0)
		| (m_TF << 8)
		| (m_IF << 9)
		| (m_DF << 10)
		| (OF << 11)
		| (m_IOPL << 12)
		| (m_NT << 14)
		| (m_MF << 15);
}

inline void i8086_common_cpu_device::ExpandFlags(uint16_t f)
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
	m_IOPL = (f >> 12) & 3;
	m_NT = ((f) & 0x4000) == 0x4000;
	m_MF = ((f) & 0x8000) == 0x8000;
}

inline void i8086_common_cpu_device::i_insb()
{
	uint32_t ea = calc_addr(ES, m_regs.w[DI], 1, I8086_WRITE);
	write_byte(ea, read_port_byte(m_regs.w[DX]));
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(IN_IMM8);
}

inline void i8086_common_cpu_device::i_insw()
{
	uint32_t ea = calc_addr(ES, m_regs.w[DI], 2, I8086_WRITE);
	write_word(ea, read_port_word(m_regs.w[DX]));
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(IN_IMM16);
}

inline void i8086_common_cpu_device::i_outsb()
{
	write_port_byte(m_regs.w[DX], GetMemB(DS, m_regs.w[SI]));
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(OUT_IMM8);
}

inline void i8086_common_cpu_device::i_outsw()
{
	write_port_word(m_regs.w[DX], GetMemW(DS, m_regs.w[SI]));
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(OUT_IMM16);
}

inline void i8086_common_cpu_device::i_movsb()
{
	uint8_t tmp = GetMemB( DS, m_regs.w[SI] );
	PutMemB( ES, m_regs.w[DI], tmp);
	m_regs.w[DI] += -2 * m_DF + 1;
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(MOVS8);
}

inline void i8086_common_cpu_device::i_movsw()
{
	uint16_t tmp = GetMemW( DS, m_regs.w[SI] );
	PutMemW( ES, m_regs.w[DI], tmp );
	m_regs.w[DI] += -4 * m_DF + 2;
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(MOVS16);
}

inline void i8086_common_cpu_device::i_cmpsb()
{
	m_src = GetMemB( ES, m_regs.w[DI] );
	m_dst = GetMemB( DS, m_regs.w[SI] );
	set_CFB(SUBB());
	m_regs.w[DI] += -2 * m_DF + 1;
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(CMPS8);
}

inline void i8086_common_cpu_device::i_cmpsw()
{
	m_src = GetMemW( ES, m_regs.w[DI] );
	m_dst = GetMemW( DS, m_regs.w[SI] );
	set_CFW(SUBX());
	m_regs.w[DI] += -4 * m_DF + 2;
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(CMPS16);
}

inline void i8086_common_cpu_device::i_stosb()
{
	PutMemB( ES, m_regs.w[DI], m_regs.b[AL] );
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(STOS8);
}

inline void i8086_common_cpu_device::i_stosw()
{
	PutMemW( ES, m_regs.w[DI], m_regs.w[AX] );
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(STOS16);
}

inline void i8086_common_cpu_device::i_lodsb()
{
	m_regs.b[AL] = GetMemB( DS, m_regs.w[SI] );
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(LODS8);
}

inline void i8086_common_cpu_device::i_lodsw()
{
	m_regs.w[AX] = GetMemW( DS, m_regs.w[SI] );
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(LODS16);
}

inline void i8086_common_cpu_device::i_scasb()
{
	m_src = GetMemB( ES, m_regs.w[DI] );
	m_dst = m_regs.b[AL];
	set_CFB(SUBB());
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(SCAS8);
}

inline void i8086_common_cpu_device::i_scasw()
{
	m_src = GetMemW( ES, m_regs.w[DI] );
	m_dst = m_regs.w[AX];
	set_CFW(SUBX());
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(SCAS16);
}


inline void i8086_common_cpu_device::i_popf()
{
	uint32_t tmp = POP();

	ExpandFlags(tmp | 0xf000);
	CLK(POPF);
	if (m_TF)
	{
		m_fire_trap = 1;
	}
}


inline uint32_t i8086_common_cpu_device::ADDB()
{
	uint32_t res = m_dst + m_src;

	set_OFB_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
	return res;
}


inline uint32_t i8086_common_cpu_device::ADDX()
{
	uint32_t res = m_dst + m_src;

	set_OFW_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
	return res;
}


inline uint32_t i8086_common_cpu_device::SUBB()
{
	uint32_t res = m_dst - m_src;

	set_OFB_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
	return res;
}


inline uint32_t i8086_common_cpu_device::SUBX()
{
	uint32_t res = m_dst - m_src;

	set_OFW_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
	return res;
}


inline void i8086_common_cpu_device::ORB()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::ORW()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::ANDB()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::ANDX()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::XORB()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::XORW()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::ROL_BYTE()
{
	m_CarryVal = m_dst & 0x80;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void i8086_common_cpu_device::ROL_WORD()
{
	m_CarryVal = m_dst & 0x8000;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void i8086_common_cpu_device::ROR_BYTE()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) | (CF ? 0x80 : 0x00);
}

inline void i8086_common_cpu_device::ROR_WORD()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) + (CF ? 0x8000 : 0x0000);
}

inline void i8086_common_cpu_device::ROLC_BYTE()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFB(m_dst);
}

inline void i8086_common_cpu_device::ROLC_WORD()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFW(m_dst);
}

inline void i8086_common_cpu_device::RORC_BYTE()
{
	m_dst |= ( CF ? 0x100 : 0x00);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void i8086_common_cpu_device::RORC_WORD()
{
	m_dst |= ( CF ? 0x10000 : 0);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void i8086_common_cpu_device::SHL_BYTE(uint8_t c)
{
	while (c--)
		m_dst <<= 1;

	set_CFB(m_dst);
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHL_WORD(uint8_t c)
{
	while (c--)
		m_dst <<= 1;

	set_CFW(m_dst);
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void i8086_common_cpu_device::SHR_BYTE(uint8_t c)
{
	while (c--)
	{
		m_CarryVal = m_dst & 0x01;
		m_dst >>= 1;
	}

	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHR_WORD(uint8_t c)
{
	while (c--)
	{
		m_CarryVal = m_dst & 0x01;
		m_dst >>= 1;
	}

	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void i8086_common_cpu_device::SHRA_BYTE(uint8_t c)
{
	while (c--)
	{
		m_CarryVal = m_dst & 0x01;
		m_dst = ((int8_t) m_dst) >> 1;
	}

	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHRA_WORD(uint8_t c)
{
	while (c--)
	{
		m_CarryVal = m_dst & 0x01;
		m_dst = ((int16_t) m_dst) >> 1;
	}

	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}


inline void i8086_common_cpu_device::XchgAXReg(uint8_t reg)
{
	uint16_t tmp = m_regs.w[reg];

	m_regs.w[reg] = m_regs.w[AX];
	m_regs.w[AX] = tmp;
}


inline void i8086_common_cpu_device::IncWordReg(uint8_t reg)
{
	uint32_t tmp = m_regs.w[reg];
	uint32_t tmp1 = tmp+1;

	m_OverVal = (tmp == 0x7fff);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void i8086_common_cpu_device::DecWordReg(uint8_t reg)
{
	uint32_t tmp = m_regs.w[reg];
	uint32_t tmp1 = tmp-1;

	m_OverVal = (tmp == 0x8000);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void i8086_common_cpu_device::PUSH(uint16_t data)
{
	write_word(calc_addr(SS, m_regs.w[SP] - 2, 2, I8086_WRITE, false), data);
	m_regs.w[SP] -= 2;
}


inline uint16_t i8086_common_cpu_device::POP()
{
	uint16_t data = read_word(calc_addr(SS, m_regs.w[SP], 2, I8086_READ, false));

	m_regs.w[SP] += 2;
	return data;
}


inline void i8086_common_cpu_device::JMP(bool cond)
{
	int rel  = (int)((int8_t)fetch());

	if (cond)
	{
		m_ip += rel;
		CLK(JCC_T);
	}
	else
		CLK(JCC_NT);
}


inline void i8086_common_cpu_device::ADJ4(int8_t param1,int8_t param2)
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
	set_SZPF_Byte(m_regs.b[AL]);
}


inline void i8086_common_cpu_device::ADJB(int8_t param1, int8_t param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		m_regs.b[AL] += param1;
		m_regs.b[AH] += param2;
		m_AuxVal = 1;
		m_CarryVal = 1;
	}
	else
	{
		m_AuxVal = 0;
		m_CarryVal = 0;
	}
	m_regs.b[AL] &= 0x0F;
}
