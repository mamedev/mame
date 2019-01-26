// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

    Opcode implementations

***************************************************************************/

#include "emu.h"
#include "i8089.h"
#include "i8089_channel.h"

#define UNIMPLEMENTED logerror("%s('%s'): unimplemented opcode: %s\n", shortname(), tag(), __FUNCTION__);

#define LWR(m, o) ((int16_t)m_iop->read_word(m_r[m].t, o))
#define LBR(m, o) ((int8_t)m_iop->read_byte(m_r[m].t, o))
#define SWR(m, o, d) (m_iop->write_word(m_r[m].t, o, d))
#define SBR(m, o, d) (m_iop->write_byte(m_r[m].t, o, d))

void i8089_channel_device::add_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w + LWR(m, o));
}

void i8089_channel_device::add_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) + m_r[r].w);
}

void i8089_channel_device::addb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w + LBR(m, o));
}

void i8089_channel_device::addb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) + m_r[r].w);
}

void i8089_channel_device::addbi_ri(int r, int8_t i)
{
	set_reg(r, m_r[r].w + i);
}

void i8089_channel_device::addbi_mi(int m, int8_t i, int o)
{
	SBR(m, o, LBR(m, o) + i);
}

void i8089_channel_device::addi_ri(int r, int16_t i)
{
	set_reg(r, m_r[r].w + i);
}

void i8089_channel_device::addi_mi(int m, int16_t i, int o)
{
	SWR(m, o, LWR(m, o) + i);
}

void i8089_channel_device::and_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w & LWR(m, o));
}

void i8089_channel_device::and_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) & m_r[r].w);
}

void i8089_channel_device::andb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w & (int16_t)LBR(m, o));
}

void i8089_channel_device::andb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) & m_r[r].w);
}

void i8089_channel_device::andbi_ri(int r, int8_t i)
{
	set_reg(r, m_r[r].w & (int16_t)i);
}

void i8089_channel_device::andbi_mi(int m, int8_t i, int o)
{
	SBR(m, o, LBR(m, o) & i);
}

void i8089_channel_device::andi_ri(int r, int16_t i)
{
	set_reg(r, m_r[r].w & i);
}

void i8089_channel_device::andi_mi(int m, int16_t i, int o)
{
	SWR(m, o, LWR(m, o) & i);
}

void i8089_channel_device::call(int m, int16_t d, int o)
{
	movp_mp(m, TP, o);
	set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::clr(int m, int b, int o)
{
	SBR(m, o, LBR(m, o) & ~(1<<b));
}

void i8089_channel_device::dec_r(int r)
{
	m_icount += 3;
	set_reg(r, m_r[r].w - 1);
}

void i8089_channel_device::dec_m(int m, int o)
{
	SWR(m, o, LWR(m, o) - 1);
}

void i8089_channel_device::decb(int m, int o)
{
	SBR(m, o, LBR(m, o) - 1);
}

// halt
void i8089_channel_device::hlt()
{
	movbi_mi(CP, 0x00, m_r[CP].w + 1);
	m_r[PSW].w &= ~(1 << 2);
	m_prio = PRIO_IDLE;
}

void i8089_channel_device::inc_r(int r)
{
	set_reg(r, m_r[r].w + 1);
}

void i8089_channel_device::inc_m(int m, int o)
{
	SWR(m, o, LWR(m, o) + 1);
}

void i8089_channel_device::incb(int m, int o)
{
	SBR(m, o, LBR(m, o) + 1);
}

void i8089_channel_device::jbt(int m, int b, int16_t d, int o)
{
	if(LBR(m, o) & (1<<b))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jmce(int m, int16_t d, int o)
{
	if(!((LBR(m, o) ^ (m_r[MC].w & 0xff)) & (m_r[MC].w >> 8)))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jmcne(int m, int16_t d, int o)
{
	if((LBR(m, o) ^ (m_r[MC].w & 0xff)) & (m_r[MC].w >> 8))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jnbt(int m, int b, int16_t d, int o)
{
	if(!(LBR(m, o) & (1<<b)))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jnz_r(int r, int16_t d)
{
	m_icount += 5;
	if(m_r[r].w & 0xffff)
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jnz_m(int m, int16_t d, int o)
{
	if(LWR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jnzb(int m, int16_t d, int o)
{
	if(LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jz_r(int r, int16_t d)
{
	if(!(m_r[r].w & 0xffff))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jz_m(int m, int16_t d, int o)
{
	if(!LWR(m, o))
		set_reg(TP, m_r[TP].w + d);
}

void i8089_channel_device::jzb(int m, int16_t d, int o)
{
	if(!LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
}


// load pointer from memory
void i8089_channel_device::lpd(int p, int m, int o)
{
	uint16_t offset = m_iop->read_word(m_r[m].t, o);
	uint16_t segment = m_iop->read_word(m_r[m].t, o + 2);

	set_reg(p, ((segment << 4) + offset) & 0xfffff, 0);
}

// load pointer from immediate data
void i8089_channel_device::lpdi(int p, int s, int o)
{
	set_reg(p, (s << 4) + (o & 0xffff), 0);
}

void i8089_channel_device::mov_mr(int m, int r, int o)
{
	SWR(m, o, m_r[r].w);
}

void i8089_channel_device::mov_rm(int r, int m, int o)
{
	set_reg(r, (int32_t)LWR(m, o), 1);
}

void i8089_channel_device::mov_mm(int m1, int m2, int o1, int o2)
{
	SWR(m2, o2, LWR(m1, o1));
}

// move register to memory byte
void i8089_channel_device::movb_mr(int m, int r, int o)
{
	m_iop->write_byte(m_r[m].t, o, m_r[r].w & 0xff);
}

// move memory byte to register
void i8089_channel_device::movb_rm(int r, int m, int o)
{
	uint8_t byte = m_iop->read_byte(m_r[m].t, o);
	set_reg(r, (BIT(byte, 7) ? 0xfff00 : 0x00000) | byte, 1);
}

// move memory byte to memory byte
void i8089_channel_device::movb_mm(int m1, int m2, int o1, int o2)
{
	uint8_t byte = m_iop->read_byte(m_r[m1].t, o1);
	m_iop->write_byte(m_r[m2].t, o2, byte);
}

// move immediate byte to register
void i8089_channel_device::movbi_ri(int r, int8_t i)
{
	set_reg(r, (BIT(i, 7) ? 0xfff00 : 0x00000) | (i & 0xff), 1);
}

// move immediate byte to memory byte
void i8089_channel_device::movbi_mi(int m, int8_t i, int o)
{
	m_iop->write_byte(m_r[m].t, o, i & 0xff);
}

// move immediate word to register
void i8089_channel_device::movi_ri(int r, int16_t i)
{
	set_reg(r, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff), 1);
}

// move immediate word to memory word
void i8089_channel_device::movi_mi(int m, int16_t i, int o)
{
	m_iop->write_word(m_r[m].t, o, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff));
}

// move pointer to memory (store)
void i8089_channel_device::movp_mp(int m, int p, int o)
{
	m_iop->write_word(m_r[m].t, o, m_r[p].w & 0xffff);
	m_iop->write_byte(m_r[m].t, o + 2, ((m_r[p].w >> 12) & 0xf0) | (m_r[p].t << 3));
}

// move memory to pointer (restore)
void i8089_channel_device::movp_pm(int p, int m, int o)
{
	uint16_t offset = m_iop->read_word(m_r[m].t, o);
	uint16_t segment = m_iop->read_byte(m_r[m].t, o + 2);

	set_reg(p, (((segment & 0xf0) << 12) + offset) & 0xfffff, segment >> 3 & 0x01);
}

// no operation
void i8089_channel_device::nop()
{
}

void i8089_channel_device::not_r(int r)
{
	set_reg(r, ~m_r[r].w);
}

void i8089_channel_device::not_m(int m, int o)
{
	SWR(m, o, ~LWR(m, o));
}

void i8089_channel_device::not_rm(int r, int m, int o)
{
	set_reg(r, ~(int32_t)LWR(m, o));
}

void i8089_channel_device::notb_m(int m, int o)
{
	SBR(m, o, ~LBR(m, o));
}

void i8089_channel_device::notb_rm(int r, int m, int o)
{
	set_reg(r, ~(int32_t)LBR(m, o));
}

void i8089_channel_device::or_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w | LWR(m, o));
}

void i8089_channel_device::or_mr(int m, int r, int o)
{
	SWR(m, o, LWR(m, o) | m_r[r].w);
}

void i8089_channel_device::orb_rm(int r, int m, int o)
{
	set_reg(r, m_r[r].w | (int16_t)LBR(m, o));
}

void i8089_channel_device::orb_mr(int m, int r, int o)
{
	SBR(m, o, LBR(m, o) | m_r[r].w);
}

void i8089_channel_device::orbi_ri(int r, int8_t i)
{
	set_reg(r, m_r[r].w | (int16_t)i);
}

void i8089_channel_device::orbi_mi(int m, int8_t i, int o)
{
	SBR(m, o, LBR(m, o) | i);
}

void i8089_channel_device::ori_ri(int r, int16_t i)
{
	set_reg(r, m_r[r].w | i);
}

void i8089_channel_device::ori_mi(int m, int16_t i, int o)
{
	SWR(m, o, LWR(m, o) | i);
}

void i8089_channel_device::setb(int m, int b, int o)
{
	SBR(m, o, LBR(m, o) | (1<<b));
}


// set interrupt service flip-flop
void i8089_channel_device::do_sintr()
{
	if (BIT(m_r[PSW].w, 4))
	{
		m_r[PSW].w |= 1 << 5;
		m_write_sintr(1);
	}
}

void i8089_channel_device::tsl(int m, int8_t i, int8_t d, int o)
{
	if(LBR(m, o))
		set_reg(TP, m_r[TP].w + d);
	else
		SBR(m, o, i);
}


// set source and destination logical widths
void i8089_channel_device::wid(int s, int d)
{
	m_r[PSW].w &= 0xfc;
	m_r[PSW].w |= d << 0;
	m_r[PSW].w |= s << 1;
}

// enter dma transfer mode after next instruction
void i8089_channel_device::xfer()
{
	m_xfer_pending = true;
}

void i8089_channel_device::invalid(int opc)
{
	logerror("%s('%s'): invalid opcode: %02x\n", shortname(), tag(), opc);
}
