// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol
/*****************************************************************************
 *
 * includes/3do.h
 *
 ****************************************************************************/

#ifndef MAME_MISC_3DO_H
#define MAME_MISC_3DO_H

#include "machine/nvram.h"
#include "machine/timer.h"
#include "screen.h"


class _3do_state : public driver_device
{
public:
	_3do_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dram(*this, "dram", 0x200000, ENDIANNESS_BIG),
		m_vram(*this, "vram"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_bank1(*this, "bank1") { }

	void _3do(machine_config &config);
	void _3do_pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	struct SLOW2 {
		/* 03180000 - 0318003f - configuration group */
		/* 03180040 - 0318007f - diagnostic UART */

		uint8_t   cg_r_count = 0;
		uint8_t   cg_w_count = 0;
		uint32_t  cg_input = 0;
		uint32_t  cg_output = 0;
	};


	struct MADAM {
		uint32_t  revision = 0;       /* 03300000 */
		uint32_t  msysbits = 0;       /* 03300004 */
		uint32_t  mctl = 0;           /* 03300008 */
		uint32_t  sltime = 0;         /* 0330000c */
		uint32_t  abortbits = 0;      /* 03300020 */
		uint32_t  privbits = 0;       /* 03300024 */
		uint32_t  statbits = 0;       /* 03300028 */
		uint32_t  diag = 0;           /* 03300040 */

		uint32_t  ccobctl0 = 0;       /* 03300110 */
		uint32_t  ppmpc = 0;          /* 03300120 */

		uint32_t  regctl0 = 0;        /* 03300130 */
		uint32_t  regctl1 = 0;        /* 03300134 */
		uint32_t  regctl2 = 0;        /* 03300138 */
		uint32_t  regctl3 = 0;        /* 0330013c */
		uint32_t  xyposh = 0;         /* 03300140 */
		uint32_t  xyposl = 0;         /* 03300144 */
		uint32_t  linedxyh = 0;       /* 03300148 */
		uint32_t  linedxyl = 0;       /* 0330014c */
		uint32_t  dxyh = 0;           /* 03300150 */
		uint32_t  dxyl = 0;           /* 03300154 */
		uint32_t  ddxyh = 0;          /* 03300158 */
		uint32_t  ddxyl = 0;          /* 0330015c */

		uint32_t  pip[16]{};        /* 03300180-033001bc (W); 03300180-033001fc (R) */
		uint32_t  fence[16]{};      /* 03300200-0330023c (W); 03300200-0330027c (R) */
		uint32_t  mmu[64]{};        /* 03300300-033003fc */
		uint32_t  dma[32][4]{};     /* 03300400-033005fc */
		uint32_t  mult[40]{};       /* 03300600-0330069c */
		uint32_t  mult_control = 0;   /* 033007f0-033007f4 */
		uint32_t  mult_status = 0;    /* 033007f8 */
	};


	struct CLIO {
		screen_device *screen = nullptr;

		uint32_t  revision = 0;       /* 03400000 */
		uint32_t  csysbits = 0;       /* 03400004 */
		uint32_t  vint0 = 0;          /* 03400008 */
		uint32_t  vint1 = 0;          /* 0340000c */
		uint32_t  audin = 0;          /* 03400020 */
		uint32_t  audout = 0;         /* 03400024 */
		uint32_t  cstatbits = 0;      /* 03400028 */
		uint32_t  wdog = 0;           /* 0340002c */
		uint32_t  hcnt = 0;           /* 03400030 */
		uint32_t  vcnt = 0;           /* 03400034 */
		uint32_t  seed = 0;           /* 03400038 */
		uint32_t  random = 0;         /* 0340004c */
		uint32_t  irq0 = 0;           /* 03400040 / 03400044 */
		uint32_t  irq0_enable = 0;    /* 03400048 / 0340004c */
		uint32_t  mode = 0;           /* 03400050 / 03400054 */
		uint32_t  badbits = 0;        /* 03400058 */
		uint32_t  spare = 0;          /* 0340005c */
		uint32_t  irq1 = 0;           /* 03400060 / 03400064 */
		uint32_t  irq1_enable = 0;    /* 03400068 / 0340006c */
		uint32_t  hdelay = 0;         /* 03400080 */
		uint32_t  adbio = 0;          /* 03400084 */
		uint32_t  adbctl = 0;         /* 03400088 */
								/* Timers */
		uint32_t  timer_count[16]{};/* 034001** & 8 */
		uint32_t  timer_backup[16]{};   /* 034001**+4 & 8 */
		uint64_t  timer_ctrl = 0;     /* 03400200 */
		uint32_t  slack = 0;          /* 03400220 */
								/* DMA */
		uint32_t  dmareqdis = 0;      /* 03400308 */
								/* Expansion bus */
		uint32_t  expctl = 0;         /* 03400400/03400404 */
		uint32_t  type0_4 = 0;        /* 03400408 */
		uint32_t  dipir1 = 0;         /* 03400410 */
		uint32_t  dipir2 = 0;         /* 03400414 */
								/* Bus signals */
		uint32_t  sel = 0;            /* 03400500 - 0340053f */
		uint32_t  poll = 0;           /* 03400540 - 0340057f */
		uint32_t  cmdstat = 0;        /* 03400580 - 034005bf */
		uint32_t  data = 0;           /* 034005c0 - 034005ff */
								/* DSPP */
		uint32_t  semaphore = 0;      /* 034017d0 */
		uint32_t  semaack = 0;        /* 034017d4 */
		uint32_t  dsppdma = 0;        /* 034017e0 */
		uint32_t  dspprst0 = 0;       /* 034017e4 */
		uint32_t  dspprst1 = 0;       /* 034017e8 */
		uint32_t  dspppc = 0;         /* 034017f4 */
		uint32_t  dsppnr = 0;         /* 034017f8 */
		uint32_t  dsppgw = 0;         /* 034017fc */
		uint32_t  dsppn[0x400]{};   /* 03401800 - 03401bff DSPP N stack (32bit writes) */
								/* 03402000 - 034027ff DSPP N stack (16bit writes) */
		uint32_t  dsppei[0x100]{};  /* 03403000 - 034030ff DSPP EI stack (32bit writes) */
								/* 03403400 - 034035ff DSPP EI stack (16bit writes) */
		uint32_t  dsppeo[0x1f]{};   /* 03403800 - 0340381f DSPP EO stack (32bit reads) */
								/* 03403c00 - 03403c3f DSPP EO stack (32bit reads) */
		uint32_t  dsppclkreload = 0;  /* 034039dc / 03403fbc */
								/* UNCLE */
		uint32_t  unclerev = 0;       /* 0340c000 */
		uint32_t  uncle_soft_rev = 0; /* 0340c004 */
		uint32_t  uncle_addr = 0;     /* 0340c008 */
		uint32_t  uncle_rom = 0;      /* 0340c00c */
	};


	struct SVF {
		uint32_t  sport[512]{};
		uint32_t  color = 0;
	};

	struct DSPP {
		std::unique_ptr<uint16_t[]> N;
		std::unique_ptr<uint16_t[]> EI;
		std::unique_ptr<uint16_t[]> EO;
	};

	required_device<cpu_device> m_maincpu;
	memory_share_creator<uint32_t> m_dram;
	required_shared_ptr<uint32_t> m_vram;
	required_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_memory_bank m_bank1;

	SLOW2 m_slow2;
	MADAM m_madam;
	CLIO m_clio;
	SVF m_svf;
	DSPP m_dspp;
	uint8_t m_nvmem[0x8000]{};

//  uint8_t m_video_bits[512];
	uint8_t nvarea_r(offs_t offset);
	void nvarea_w(offs_t offset, uint8_t data);
	uint32_t slow2_r(offs_t offset);
	void slow2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t svf_r(offs_t offset);
	void svf_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t madam_r(offs_t offset);
	void madam_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t clio_r(offs_t offset);
	void clio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER( timer_x16_cb );

	void main_mem(address_map &map) ATTR_COLD;

	void m_slow2_init( void );
	void m_madam_init( void );
	void m_clio_init( void );

	void m_request_fiq(uint32_t irq_req, uint8_t type);
};


#endif // MAME_MISC_3DO_H
