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
