#ifndef TMP68301_H
#define TMP68301_H

// Machine init
MACHINE_START( tmp68301 );
MACHINE_RESET( tmp68301 );

// Hardware Registers
DECLARE_READ16_HANDLER( tmp68301_regs_r );
DECLARE_WRITE16_HANDLER( tmp68301_regs_w );

// Interrupts
void tmp68301_external_interrupt_0(running_machine &machine);
void tmp68301_external_interrupt_1(running_machine &machine);
void tmp68301_external_interrupt_2(running_machine &machine);

#endif
