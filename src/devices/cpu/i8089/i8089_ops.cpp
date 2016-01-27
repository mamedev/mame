// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

    Opcode implementations

***************************************************************************/

#include "emu.h"
#include "i8089_channel.h"

#define UNIMPLEMENTED logerror("%s('%s'): unimplemented opcode: %s\n", shortname(), tag(), __FUNCTION__);

#define LWR(m, o) ((INT16)m_iop->read_word(m_r[m].t, o))
#define LBR(m, o) ((INT8)m_iop->read_byte(m_r[m].t, o))
#define SWR(m, o, d) (m_iop->write_word(m_r[m].t, o, d))
#define SBR(m, o, d) (m_iop->write_byte(m_r[m].t, o, d))

void i8089_channel::add_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w + LWR(m, o));
}

void i8089_channel::add_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) + m_r[r].w);
}

void i8089_channel::addb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w + LBR(m, o));
}

void i8089_channel::addb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) + m_r[r].w);
}

void i8089_channel::addbi_ri(int r, INT8 i)
{
	set_reg(r, m_r[r].w + i);
}

void i8089_channel::addbi_mi(int m, INT8 i, int o)
{
	SBR(m, o, LBR(m, o) + i);
}

void i8089_channel::addi_ri(int r, INT16 i)
{
	set_reg(r, m_r[r].w + i);
}

void i8089_channel::addi_mi(int m, INT16 i, int o)
{
	SWR(m, o, LWR(m, o) + i);
}

void i8089_channel::and_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w & LWR(m, o));
}

void i8089_channel::and_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) & m_r[r].w);
}

void i8089_channel::andb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w & (INT16)LBR(m, o));
}

void i8089_channel::andb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) & m_r[r].w);
}

void i8089_channel::andbi_ri(int r, INT8 i)
{
	set_reg(r, m_r[r].w & (INT16)i);
}

void i8089_channel::andbi_mi(int m, INT8 i, int o)
{
	SBR(m, o, LBR(m, o) & i);
}

void i8089_channel::andi_ri(int r, INT16 i)
{
	set_reg(r, m_r[r].w & i);
}

void i8089_channel::andi_mi(int m, INT16 i, int o)
{
	SWR(m, o, LWR(m, o) & i);
}

void i8089_channel::call(int m, INT16 d, int o)
{
	movp_mp(m, TP, o);
	set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::clr(int m, int b, int o)
{
	SBR(m, o, LBR(m, o) & ~(1<<b));
}

void i8089_channel::dec_r(int r)
{
	m_icount += 3;
	set_reg(r, m_r[r].w - 1);
}

void i8089_channel::dec_m(int m, int o)
{
	SWR(m, o, LWR(m, o) - 1);
}

void i8089_channel::decb(int m, int o)
{
	SBR(m, o, LBR(m, o) - 1);
}

// halt
void i8089_channel::hlt()
{
	movbi_mi(CP, 0x00, m_r[CP].w + 1);
	m_r[PSW].w &= ~(1 << 2);
	m_prio = PRIO_IDLE;
}

void i8089_channel::inc_r(int r)
{
	set_reg(r, m_r[r].w + 1);
}

void i8089_channel::inc_m(int m, int o)
{
	SWR(m, o, LWR(m, o) + 1);
}

void i8089_channel::incb(int m, int o)
{
	SBR(m, o, LBR(m, o) + 1);
}

void i8089_channel::jbt(int m, int b, INT16 d, int o)
{
	if(LBR(m, o) & (1<<b))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jmce(int m, INT16 d, int o)
{
	if(!((LBR(m, o) ^ (m_r[MC].w & 0xff)) & (m_r[MC].w >> 8)))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jmcne(int m, INT16 d, int o)
{
	if((LBR(m, o) ^ (m_r[MC].w & 0xff)) & (m_r[MC].w >> 8))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jnbt(int m, int b, INT16 d, int o)
{
	if(!(LBR(m, o) & (1<<b)))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jnz_r(int r, INT16 d)
{
	m_icount += 5;
	if(m_r[r].w & 0xffff)
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jnz_m(int m, INT16 d, int o)
{
	if(LWR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jnzb(int m, INT16 d, int o)
{
	if(LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jz_r(int r, INT16 d)
{
	if(!(m_r[r].w & 0xffff))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jz_m(int m, INT16 d, int o)
{
	if(!LWR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel::jzb(int m, INT16 d, int o)
{
	if(!LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
}


// load pointer from memory
void i8089_channel::lpd(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].t, o);
	UINT16 segment = m_iop->read_word(m_r[m].t, o + 2);

	set_reg(p, ((segment << 4) + offset) & 0xfffff, 0);
}

// load pointer from immediate data
void i8089_channel::lpdi(int p, int s, int o)
{
	set_reg(p, (s << 4) + (o & 0xffff), 0);
}

void i8089_channel::mov_mr(int m, int r, int o)
{
	SWR(m, o, m_r[r].w);
}

void i8089_channel::mov_rm(int r, int m, int o)
{
	set_reg(r, (INT32)LWR(m, o), 1);
}

void i8089_channel::mov_mm(int m1, int m2, int o1, int o2)
{
	SWR(m2, o2, LWR(m1, o1));
}

// move register to memory byte
void i8089_channel::movb_mr(int m, int r, int o)
{
	m_iop->write_byte(m_r[m].t, o, m_r[r].w & 0xff);
}

// move memory byte to register
void i8089_channel::movb_rm(int r, int m, int o)
{
	UINT8 byte = m_iop->read_byte(m_r[m].t, o);
	set_reg(r, (BIT(byte, 7) ? 0xfff00 : 0x00000) | byte, 1);
}

// move memory byte to memory byte
void i8089_channel::movb_mm(int m1, int m2, int o1, int o2)
{
	UINT8 byte = m_iop->read_byte(m_r[m1].t, o1);
	m_iop->write_byte(m_r[m2].t, o2, byte);
}

// move immediate byte to register
void i8089_channel::movbi_ri(int r, INT8 i)
{
	set_reg(r, (BIT(i, 7) ? 0xfff00 : 0x00000) | (i & 0xff), 1);
}

// move immediate byte to memory byte
void i8089_channel::movbi_mi(int m, INT8 i, int o)
{
	m_iop->write_byte(m_r[m].t, o, i & 0xff);
}

// move immediate word to register
void i8089_channel::movi_ri(int r, INT16 i)
{
	set_reg(r, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff), 1);
}

// move immediate word to memory word
void i8089_channel::movi_mi(int m, INT16 i, int o)
{
	m_iop->write_word(m_r[m].t, o, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff));
}

// move pointer to memory (store)
void i8089_channel::movp_mp(int m, int p, int o)
{
	m_iop->write_word(m_r[m].t, o, m_r[p].w & 0xffff);
	m_iop->write_byte(m_r[m].t, o + 2, ((m_r[p].w >> 12) & 0xf0) | (m_r[p].t << 3));
}

// move memory to pointer (restore)
void i8089_channel::movp_pm(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].t, o);
	UINT16 segment = m_iop->read_byte(m_r[m].t, o + 2);

	set_reg(p, (((segment & 0xf0) << 12) + offset) & 0xfffff, segment >> 3 & 0x01);
}

// no operation
void i8089_channel::nop()
{
}

void i8089_channel::not_r(int r)
{
	set_reg(r, ~m_r[r].w);
}

void i8089_channel::not_m(int m, int o)
{
	SWR(m, o, ~LWR(m, o));
}

void i8089_channel::not_rm(int r, int m, int o)
{
	set_reg(r, ~(INT32)LWR(m, o));
}

void i8089_channel::notb_m(int m, int o)
{
	SBR(m, o, ~LBR(m, o));
}

void i8089_channel::notb_rm(int r, int m, int o)
{
	set_reg(r, ~(INT32)LBR(m, o));
}

void i8089_channel::or_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w | LWR(m, o));
}

void i8089_channel::or_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) | m_r[r].w);
}

void i8089_channel::orb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w | (INT16)LBR(m, o));
}

void i8089_channel::orb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) | m_r[r].w);
}

void i8089_channel::orbi_ri(int r, INT8 i)
{
	set_reg(r, m_r[r].w | (INT16)i);
}

void i8089_channel::orbi_mi(int m, INT8 i, int o)
{
	SBR(m, o, LBR(m, o) | i);
}

void i8089_channel::ori_ri(int r, INT16 i)
{
	set_reg(r, m_r[r].w | i);
}

void i8089_channel::ori_mi(int m, INT16 i, int o)
{
	SWR(m, o, LWR(m, o) | i);
}

void i8089_channel::setb(int m, int b, int o)
{
	SBR(m, o, LBR(m, o) | (1<<b));
}


// set interrupt service flip-flop
void i8089_channel::sintr()
{
	if (BIT(m_r[PSW].w, 4))
	{
		m_r[PSW].w |= 1 << 5;
		m_write_sintr(1);
	}
}

void i8089_channel::tsl(int m, INT8 i, INT8 d, int o)
{
	if(LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
	else
		SBR(m, o, i);
}


// set source and destination logical widths
void i8089_channel::wid(int s, int d)
{
	m_r[PSW].w &= 0x3f;
	m_r[PSW].w |= d << 0;
	m_r[PSW].w |= s << 1;
}

// enter dma transfer mode after next instruction
void i8089_channel::xfer()
{
	m_xfer_pending = true;
}

void i8089_channel::invalid(int opc)
{
	logerror("%s('%s'): invalid opcode: %02x\n", shortname(), tag(), opc);
}
