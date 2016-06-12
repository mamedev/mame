// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/***************************************************************************

    r65c02gk.h

    gameking 65c02 with bitwise instructions and wai instruction, additional 
	interrupt vectors, special vector address base


***************************************************************************/

#ifndef __R65C02GK_H__
#define __R65C02GK_H__

#include "r65c02.h"

class r65c02gk_device : public r65c02_device {
public:
	r65c02gk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	r65c02gk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	enum {
	  IRQ_GK3A=INPUT_LINE_IRQ0,
	  IRQ_GK3B,
	  IRQ_TIMERUSER=INPUT_LINE_IRQ6,
	  IRQ_INPUTS, //449C,
	  IRQ_USER, //=INPUT_LINE_IRQ8,
	  IRQ_TIMER, //=INPUT_LINE_IRQ9,
	  IRQ_6606,
	  R65C02GK_IRQ_LINE12 = IRQ_GK3A/*R65C02GK_IRQ_LINE0*/+12,
	  R65C02GK_NMI_LINE = INPUT_LINE_NMI // >12
//	R65C02GK_SET_OVERFLOW = m6502_device::V_LINE,
	};

	void set_irq_bank(memory_bank *m_bank4000, UINT8 *bank4000_normal, UINT8 *bank4000_irq);
protected:
      UINT8 *bank4000_normal, *bank4000_irq;
    virtual void device_reset() override;
    virtual void execute_set_input(int inputnum, int state) override;
    memory_bank *m_bank4000;

#if 0    
    void prefetch();
    void prefetch_noirq();
    void prefetch_wai();
#endif    

    struct InterruptController { // nearly nothing known about it, only enough to get bios working
    bool states[13];
    int getlevel() { for (int i=12; i>=0; --i) { if (states[i]) return i; } return -1; }
    void Reset() { memset(states, 0, sizeof(states)); }
  } interrupt_controller;

  #define O(o) void o ## _full(); void o ## _partial()

	O(brk_c_imp);
	O(rti_imp);
	O(wai_imp);

	//   exceptions
	O(reset);
  
};


extern const device_type R65C02GK;

#endif
