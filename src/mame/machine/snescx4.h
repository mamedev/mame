/***************************************************************************

    snescx4.h

    Code based on original work by zsKnight, anomie and Nach.
    This implementation is based on C++ "cx4*.cpp" by byuu
    (up to date with source v 0.49).

***************************************************************************/

#include "driver.h"

typedef struct
{
       UINT8 ram[0x0c00];
       UINT8 reg[0x0100];
       UINT32 r0, r1, r2,  r3,  r4,  r5,  r6,  r7,
                  r8, r9, r10, r11, r12, r13, r14, r15;

       INT16 C4WFXVal, C4WFYVal, C4WFZVal, C4WFX2Val, C4WFY2Val, C4WFDist, C4WFScale;
       INT16 C41FXVal, C41FYVal, C41FAngleRes, C41FDist, C41FDistVal;

       double tanval;
       double c4x, c4y, c4z;
       double c4x2, c4y2, c4z2;
} CX4;

static CX4 cx4;

UINT32 CX4_ldr(UINT8 r);
void   CX4_str(UINT8 r, UINT32 data);
void   CX4_mul(UINT32 x, UINT32 y, UINT32 *rl, UINT32 *rh);
UINT32 CX4_sin(UINT32 rx);
UINT32 CX4_cos(UINT32 rx);

void   CX4_transfer_data(running_machine *machine);
void   CX4_immediate_reg(UINT32 num);

void   CX4_op00_00(running_machine *machine);
void   CX4_op00_03(void);
void   CX4_op00_05(running_machine *machine);
void   CX4_op00_07(void);
void   CX4_op00_08(running_machine *machine);
void   CX4_op00_0b(running_machine *machine);
void   CX4_op00_0c(running_machine *machine);

void   CX4_op00(running_machine* machine);
void   CX4_op01(running_machine* machine);
void   CX4_op05(running_machine *machine);
void   CX4_op0d(running_machine *machine);
void   CX4_op10(void);
void   CX4_op13(void);
void   CX4_op15(running_machine *machine);
void   CX4_op1f(running_machine *machine);
void   CX4_op22(void);
void   CX4_op25(void);
void   CX4_op2d(running_machine *machine);
void   CX4_op40(void);
void   CX4_op54(void);
void   CX4_op5c(void);
void   CX4_op5e(void);
void   CX4_op60(void);
void   CX4_op62(void);
void   CX4_op64(void);
void   CX4_op66(void);
void   CX4_op68(void);
void   CX4_op6a(void);
void   CX4_op6c(void);
void   CX4_op6e(void);
void   CX4_op70(void);
void   CX4_op72(void);
void   CX4_op74(void);
void   CX4_op76(void);
void   CX4_op78(void);
void   CX4_op7a(void);
void   CX4_op7c(void);
void   CX4_op89(void);

UINT8 CX4_readb(UINT16 addr);
UINT16 CX4_readw(UINT16 addr);
UINT32 CX4_readl(UINT16 addr);

void CX4_writeb(running_machine *machine, UINT16 addr, UINT8 data);
void CX4_writew(running_machine *machine, UINT16 addr, UINT16 data);
void CX4_writel(running_machine *machine, UINT16 addr, UINT32 data);

void CX4_reset(void);

UINT8 CX4_read(UINT32 addr);
void CX4_write(running_machine *machine, UINT32 addr, UINT8 data);

void CX4_C4DrawLine(INT32 X1, INT32 Y1, INT16 Z1, INT32 X2, INT32 Y2, INT16 Z2, UINT8 Color);
