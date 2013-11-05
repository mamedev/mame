/***************************************************************************

    Intel 8089 I/O Processor

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    Opcode implementations

***************************************************************************/

#include "emu.h"
#include "i8089_channel.h"

#define UNIMPLEMENTED logerror("%s('%s'): unimplemented opcode: %s\n", shortname(), tag(), __FUNCTION__);

void i8089_channel::add_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::add_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::addb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::addb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::addbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::addbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::addi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::addi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::and_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::and_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::andb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::andb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::andbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::andbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::andi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::andi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::call(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::clr(int m, int b, int o) { UNIMPLEMENTED }
void i8089_channel::dec_r(int r) { UNIMPLEMENTED }
void i8089_channel::dec_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::decb(int m, int o) { UNIMPLEMENTED }

// halt
void i8089_channel::hlt()
{
	movbi_mi(CP, 0x00, 1);
	m_r[PSW].w &= ~(1 << 2);
}

void i8089_channel::inc_r(int r) { UNIMPLEMENTED }
void i8089_channel::inc_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::incb(int m, int o) { UNIMPLEMENTED }
void i8089_channel::jbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmce(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmcne(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmp(int d) { UNIMPLEMENTED }
void i8089_channel::jnbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jnz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::jnz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jnzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::jz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::lcall(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmce(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmcne(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmp(int d) { UNIMPLEMENTED }
void i8089_channel::ljnbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljnz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::ljnz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljnzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::ljz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljzb(int m, int d, int o) { UNIMPLEMENTED }

// load pointer from memory
void i8089_channel::lpd(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w + o + 2);

	set_reg(p, ((segment << 4) + offset) & 0xfffff, 0);
}

// load pointer from immediate data
void i8089_channel::lpdi(int p, int i, int o)
{
	set_reg(p, (o << 4) + (i & 0xffff), 0);
}

void i8089_channel::mov_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::mov_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::mov_mm(int m1, int m2, int o1, int o2) { UNIMPLEMENTED }

// move register to memory byte
void i8089_channel::movb_mr(int m, int r, int o)
{
	m_iop->write_byte(m_r[m].w + o, m_r[r].w & 0xff);
}

// move memory byte to register
void i8089_channel::movb_rm(int r, int m, int o)
{
	UINT8 byte = m_iop->read_byte(m_r[m].w + o);
	set_reg(r, (BIT(byte, 7) ? 0xfff00 : 0x00000) | byte, 1);
}

// move memory byte to memory byte
void i8089_channel::movb_mm(int m1, int m2, int o1, int o2)
{
	UINT8 byte = m_iop->read_byte(m_r[m2].w + o2);
	m_iop->write_byte(m_r[m1].w + o1, byte);
}

// move immediate byte to register
void i8089_channel::movbi_ri(int r, int i)
{
	set_reg(r, (BIT(i, 7) ? 0xfff00 : 0x00000) | (i & 0xff), 1);
}

// move immediate byte to memory byte
void i8089_channel::movbi_mi(int m, int i, int o)
{
	m_iop->write_byte(m_r[m].w + o, i & 0xff);
}

// move immediate word to register
void i8089_channel::movi_ri(int r, int i)
{
	set_reg(r, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff), 1);
}

// move immediate word to memory word
void i8089_channel::movi_mi(int m, int i, int o)
{
	m_iop->write_word(m_r[m].w + o, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff));
}

// move pointer to memory (store)
void i8089_channel::movp_mp(int m, int p, int o)
{
	m_iop->write_word(m_r[m].w + o, m_r[p].w & 0xffff);
	m_iop->write_word(m_r[m].w + o + 2, (m_r[p].w >> 12 & 0xf0) | (m_r[p].t << 3 & 0x01));
}

// move memory to pointer (restore)
void i8089_channel::movp_pm(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w + o + 2);

	set_reg(p, ((segment << 4) + offset) & 0xfffff, segment >> 3 & 0x01);
}

// no operation
void i8089_channel::nop()
{
}

void i8089_channel::not_r(int r) { UNIMPLEMENTED }
void i8089_channel::not_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::not_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::notb_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::notb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::or_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::or_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::orb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::orb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::orbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::orbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::ori_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::ori_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::setb(int m, int b, int o) { UNIMPLEMENTED }

// set interrupt service flip-flop
void i8089_channel::sintr()
{
	if (BIT(m_r[PSW].w, 4))
	{
		m_r[PSW].w |= 1 << 5;
		m_write_sintr(1);
	}
}

void i8089_channel::tsl(int m, int i, int d, int o) { UNIMPLEMENTED }

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
