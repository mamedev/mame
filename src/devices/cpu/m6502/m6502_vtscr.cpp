// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

    m6502_vtscr.cpp

    6502 with VRT VTxx instruction scrambling
    
    Scrambling in newer NES-based VTxx systems (FC pocket, etc) seems to be
    enabled with a write of 5 to 0x411E, then is activated at the next jump?
    When enabled, opcodes are to be XORed with 0xA1
    
    Another form of scrambling is used in the VRT VT1682, this is not yet 
    implemented at all in MAME (it's used for the MiWi2 and InterAct consoles).
    This is simpler, permanently activated and consists of swapping opcode bits
    7 and 2.
    
    TODO: disassembly for this
***************************************************************************/

#include "emu.h"
#include "m6502_vtscr.h"

DEFINE_DEVICE_TYPE(M6502_VTSCR, m6502_vtscr, "m6502_vtscr", "M6502 with VTxx scrambling")

m6502_vtscr::m6502_vtscr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6502_VTSCR, tag, owner, clock)
{
}

m6502_vtscr::m6502_vtscr(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock)
{
}

void m6502_vtscr::set_next_scramble(bool scr)
{
	m_next_scramble = scr;
}

void m6502_vtscr::set_scramble(bool scr)
{
	m_next_scramble = scr;
	m_scramble_en = scr;

}


void m6502_vtscr::device_reset()
{
	m_scramble_en = false;
	m_next_scramble = false;

	m6502_device::device_reset();
}

bool m6502_vtscr::toggle_scramble(uint8_t op) {
	return (op == 0x4C) || (op == 0x00) || (op == 0x6C);
}


void m6502_vtscr::prefetch()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	IR = mintf->read_sync(PC);
	if(m_scramble_en)
		IR = descramble(IR);
		
	sync = false;
	sync_w(CLEAR_LINE);
	
	if((nmi_state || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	} else
		PC++;
		
	if(toggle_scramble(IR)) {
		m_scramble_en = m_next_scramble;
	}
}

void m6502_vtscr::prefetch_noirq()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	IR = mintf->read_sync(PC);
	if(m_scramble_en)
		IR = descramble(IR);
		
	sync = false;
	sync_w(CLEAR_LINE);
	PC++;
	
	if(toggle_scramble(IR)) {
		m_scramble_en = m_next_scramble;
	}
}

uint8_t m6502_vtscr::descramble(uint8_t op)
{
	return op ^ 0xA1;
}
