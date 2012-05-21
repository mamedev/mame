/* ASG 971222 -- rewrote this interface */
#pragma once

#ifndef __I86INTF_H__
#define __I86INTF_H__


#define INPUT_LINE_INT0			INPUT_LINE_IRQ0
#define INPUT_LINE_INT1			INPUT_LINE_IRQ1
#define INPUT_LINE_INT2			INPUT_LINE_IRQ2
#define INPUT_LINE_INT3			INPUT_LINE_IRQ3
#define INPUT_LINE_TEST			20    /* PJB 03/05 */
#define INPUT_LINE_DRQ0			21
#define INPUT_LINE_DRQ1			22
#define INPUT_LINE_TMRIN0		23
#define INPUT_LINE_TMRIN1		24


typedef struct _i80186_interface i80186_interface;
struct _i80186_interface
{
	devcb_write_line		out_tmrout0_func;
	devcb_write_line		out_tmrout1_func;
};
#define I80186_INTERFACE(name) const i80186_interface (name) =


enum
{
	I8086_IP,
	I8086_AX,
	I8086_CX,
	I8086_DX,
	I8086_BX,
	I8086_SP,
	I8086_BP,
	I8086_SI,
	I8086_DI,
	I8086_AL,
	I8086_CL,
	I8086_DL,
	I8086_BL,
	I8086_AH,
	I8086_CH,
	I8086_DH,
	I8086_BH,
	I8086_FLAGS,
	I8086_ES,
	I8086_CS,
	I8086_SS,
	I8086_DS,
	I8086_VECTOR,

	I8086_GENPC = STATE_GENPC,
	I8086_GENSP = STATE_GENSP,
	I8086_GENPCBASE = STATE_GENPCBASE
};

/* Public functions */
DECLARE_LEGACY_CPU_DEVICE(I8086, i8086);
DECLARE_LEGACY_CPU_DEVICE(I8088, i8088);
DECLARE_LEGACY_CPU_DEVICE(I80186, i80186);
DECLARE_LEGACY_CPU_DEVICE(I80188, i80188);

#endif /* __I86INTF_H__ */
