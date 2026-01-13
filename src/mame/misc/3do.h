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

#include "3do_clio.h"

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
		m_clio(*this, "clio"),
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
		uint32_t  fence[4]{};      /* 03300200-0330023c (W); 03300200-0330027c (R) */
		uint32_t  mmu[64]{};        /* 03300300-033003fc */
		uint32_t  dma[32][4]{};     /* 03300400-033005fc */
		uint32_t  mult[40]{};       /* 03300600-0330069c */
		uint32_t  mult_control = 0;   /* 033007f0-033007f4 */
		uint32_t  mult_status = 0;    /* 033007f8 */
	};

	void madam_map(address_map &map);


	struct UNCLE {
		uint32_t  rev = 0;       /* 0340c000 */
		uint32_t  soft_rev = 0; /* 0340c004 */
		uint32_t  addr = 0;     /* 0340c008 */
	};

	void uncle_map(address_map &map);

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
	required_device<clio_device> m_clio;
	required_memory_bank m_bank1;

	SLOW2 m_slow2;
	MADAM m_madam;
	UNCLE m_uncle;
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
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_mem(address_map &map) ATTR_COLD;

	void m_slow2_init( void );
	void m_madam_init( void );
	void m_clio_init( void );
};


#endif // MAME_MISC_3DO_H
