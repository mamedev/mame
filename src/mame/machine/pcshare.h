#include "sound/custom.h"

/* flags for init_pc_common */
#define PCCOMMON_KEYBOARD_PC	0
#define PCCOMMON_KEYBOARD_AT	1
#define PCCOMMON_DMA8237_PC		0
#define PCCOMMON_DMA8237_AT		2
#define PCCOMMON_NEC765_RDY_NC	16

void init_pc_common(UINT32 flags);

void pc_keyboard(void);
UINT8 pc_keyb_read(void);
void pc_keyb_set_clock(int on);
void pc_keyb_clear(void);

READ8_HANDLER(pc_page_r);
WRITE8_HANDLER(pc_page_w);

READ16_HANDLER(pc_page16le_r);
WRITE16_HANDLER(pc_page16le_w);

READ8_HANDLER(at_page8_r);
WRITE8_HANDLER(at_page8_w);

READ32_HANDLER(at_page32_r);
WRITE32_HANDLER(at_page32_w);
