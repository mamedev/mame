// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* SHA3/4 DMA Controller */

/* bit definitions */
#define CHCR_SSA    0xe0000000
#define CHCR_STC    0x10000000
#define CHCR_DSA    0x0e000000
#define CHCR_DTC    0x01000000
#define CHCR_DS     0x00080000
#define CHCR_RL     0x00040000
#define CHCR_AM     0x00020000
#define CHCR_AL     0x00010000
#define CHCR_DM     0x0000c000
#define CHCR_SM     0x00003000
#define CHCR_RS     0x00000f00
#define CHCR_TM     0x00000080
#define CHCR_TS     0x00000070
#define CHCR_IE     0x00000004
#define CHCR_TE     0x00000002
#define CHCR_DE     0x00000001

#define DMAOR_DDT   0x8000
#define DMAOR_PR    0x0300
#define DMAOR_COD   0x0010
#define DMAOR_AE    0x0004
#define DMAOR_NMIF  0x0002
#define DMAOR_DME   0x0001

TIMER_CALLBACK( sh4_dmac_callback );

void sh4_handle_sar0_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_sar1_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_sar2_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_sar3_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dar0_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dar1_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dar2_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dar3_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dmatcr0_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dmatcr1_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dmatcr2_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dmatcr3_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_chcr0_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_chcr1_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_chcr2_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_chcr3_addr_w(UINT32 data, UINT32 mem_mask);
void sh4_handle_dmaor_addr_w(UINT32 data, UINT32 mem_mask);
UINT32 sh4_handle_sar0_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_sar1_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_sar2_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_sar3_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dar0_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dar1_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dar2_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dar3_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dmatcr0_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dmatcr1_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dmatcr2_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dmatcr3_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_chcr0_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_chcr1_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_chcr2_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_chcr3_addr_r(UINT32 mem_mask);
UINT32 sh4_handle_dmaor_addr_r(UINT32 mem_mask);
