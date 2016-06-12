// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/***************************************************************************

    r65c02gk.c

    Rockwell 65c02gb, additional interrupts, interrupt bankswitching

***************************************************************************/

#include "emu.h"
#include "r65c02gk.h"

const device_type R65C02GK = &device_creator<r65c02gk_device>;

r65c02gk_device::r65c02gk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	r65c02_device(mconfig, R65C02GK, "R65C02GK", tag, owner, clock, "r65c02gk", __FILE__)
{
}

r65c02gk_device::r65c02gk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	r65c02_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

offs_t r65c02gk_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void r65c02gk_device::device_reset()
{
  r65c02_device::device_reset();
  interrupt_controller.Reset();
}

void r65c02gk_device::set_irq_bank(memory_bank *m_bank4000_, UINT8 *bank4000_normal_, UINT8 *bank4000_irq_) {
  m_bank4000 = m_bank4000_;  
  bank4000_normal=bank4000_normal_;
  bank4000_irq=bank4000_irq_;
}

void r65c02gk_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum) {
	case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
	  interrupt_controller.states[inputnum]=state;
	  irq_state= interrupt_controller.getlevel()>=0;
	  break;
	case R65C02GK_NMI_LINE: nmi_state = nmi_state || (state == ASSERT_LINE); break;
#if 0
	case APU_IRQ_LINE: apu_irq_state = state == ASSERT_LINE; break;
	case V_LINE:
		if(!v_state && state == ASSERT_LINE)
			P |= F_V;
		v_state = state == ASSERT_LINE;
		break;
#endif
	}
//      logerror("%.6f irq num:%d state:%d irq:%x level:%x\n",machine().time().as_double(), inputnum, state, irq_state, interrupt_controller.getlevel());
}

#if 0
void r65c02gk_device::prefetch()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	IR = mintf->read_decrypted(PC);
	sync = false;
	sync_w(CLEAR_LINE);

	if((nmi_state || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	} else
		PC++;
}

void r65c02gk_device::prefetch_wai()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	IR = mintf->read_decrypted(PC);
	sync = false;
	sync_w(CLEAR_LINE);

	if (irq_state || nmi_state)
	  PC++;
	if((nmi_state || ((irq_state || apu_irq_state) && !(P & F_I))) && !inhibit_interrupts) {
		irq_taken = true;
		IR = 0x00;
	}
}

void r65c02gk_device::prefetch_noirq()
{
	sync = true;
	sync_w(ASSERT_LINE);
	NPC = PC;
	IR = mintf->read_decrypted(PC);
	sync = false;
	sync_w(CLEAR_LINE);
	PC++;
}
#endif

#include "cpu/m6502/r65c02gk.hxx"
