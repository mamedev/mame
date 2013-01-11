#ifndef __SUPERFX_H__
#define __SUPERFX_H__


enum
{
	SUPERFX_PC = 1,

	SUPERFX_DREG,
	SUPERFX_SREG,

	SUPERFX_R0,
	SUPERFX_R1,
	SUPERFX_R2,
	SUPERFX_R3,
	SUPERFX_R4,
	SUPERFX_R5,
	SUPERFX_R6,
	SUPERFX_R7,
	SUPERFX_R8,
	SUPERFX_R9,
	SUPERFX_R10,
	SUPERFX_R11,
	SUPERFX_R12,
	SUPERFX_R13,
	SUPERFX_R14,
	SUPERFX_R15,

	SUPERFX_PBR,
	SUPERFX_SFR,
	SUPERFX_ROMBR,
	SUPERFX_RAMBR,
	SUPERFX_CBR,
	SUPERFX_SCBR,
	SUPERFX_SCMR,
	SUPERFX_COLR,
	SUPERFX_POR,
	SUPERFX_BRAMR,
	SUPERFX_VCR,
	SUPERFX_CFGR,
	SUPERFX_CLSR,

	SUPERFX_ROMCL,
	SUPERFX_ROMDR,

	SUPERFX_RAMCL,
	SUPERFX_RAMAR,
	SUPERFX_RAMDR,
	SUPERFX_RAMADDR,
};

#define SUPERFX_SFR_IRQ     0x8000  // Interrupt Flag
#define SUPERFX_SFR_B       0x1000  // WITH Flag
#define SUPERFX_SFR_IH      0x0800  // Immediate Higher 8-bit Flag
#define SUPERFX_SFR_IL      0x0400  // Immediate Lower 8-bit Flag
#define SUPERFX_SFR_ALT     0x0300  // ALT Mode, both bits
#define SUPERFX_SFR_ALT0    0x0000  // ALT Mode, no bits
#define SUPERFX_SFR_ALT1    0x0100  // ALT Mode, bit 0
#define SUPERFX_SFR_ALT2    0x0200  // ALT Mode, bit 1
#define SUPERFX_SFR_ALT3    0x0300  // ALT Mode, both bits (convenience dupe)
#define SUPERFX_SFR_R       0x0040  // ROM R14 Read Flag
#define SUPERFX_SFR_G       0x0020  // GO Flag
#define SUPERFX_SFR_OV      0x0010  // Overflow Flag
#define SUPERFX_SFR_S       0x0008  // Sign Flag
#define SUPERFX_SFR_CY      0x0004  // Carry Flag
#define SUPERFX_SFR_Z       0x0002  // Zero Flag

#define SUPERFX_POR_OBJ         0x10
#define SUPERFX_POR_FREEZEHIGH  0x08
#define SUPERFX_POR_HIGHNIBBLE  0x04
#define SUPERFX_POR_DITHER      0x02
#define SUPERFX_POR_TRANSPARENT 0x01

#define SUPERFX_SCMR_HT_MASK    0x24
#define SUPERFX_SCMR_HT0        0x00
#define SUPERFX_SCMR_HT1        0x04
#define SUPERFX_SCMR_HT2        0x20
#define SUPERFX_SCMR_HT3        0x24
#define SUPERFX_SCMR_RON        0x10
#define SUPERFX_SCMR_RAN        0x08
#define SUPERFX_SCMR_MD         0x03

#define SUPERFX_CFGR_IRQ    0x80    // IRQ
#define SUPERFX_CFGR_MS0    0x20    // MS0

struct superfx_config
{
	devcb_write_line    out_irq_func;           /* IRQ changed callback */
};
#define SUPERFX_CONFIG(name) const superfx_config (name) =

DECLARE_LEGACY_CPU_DEVICE(SUPERFX, superfx);

CPU_DISASSEMBLE( superfx );
extern offs_t superfx_dasm_one(char *buffer, offs_t pc, UINT8 op, UINT8 param0, UINT8 param1, UINT16 alt);

UINT8 superfx_mmio_read(device_t *cpu, UINT32 addr);
void superfx_mmio_write(device_t *cpu, UINT32 addr, UINT8 data);
void superfx_add_clocks(device_t *cpu, INT32 clocks);
int superfx_access_ram(device_t *cpu);
int superfx_access_rom(device_t *cpu);

#endif /* __SUPERFX_H__ */
