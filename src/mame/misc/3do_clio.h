// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol

#ifndef MAME_MISC_3DO_CLIO_H
#define MAME_MISC_3DO_CLIO_H

#pragma once

#include "cpu/dspp/dspp.h"
#include "screen.h"

class clio_device : public device_t
{
public:
	// construction/destruction
	clio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	auto firq_cb() { return m_firq_cb.bind(); }
	auto xbus_read_cb() { return m_xbus_read_cb.bind(); }
	auto xbus_write_cb() { return m_xbus_write_cb.bind(); }
	auto dacl_cb() { return m_dac_l.bind(); }
	auto dacr_cb() { return m_dac_r.bind(); }
	auto hsync_cb() { return m_hsync_cb.bind(); }
	auto vsync_cb() { return m_vsync_cb.bind(); }
	template <std::size_t Line> auto adb_out_cb() { return m_adb_out_cb[Line].bind(); }

	void xbus_int_w(int state);

//	void expansion_w(int state);
	void dply_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<screen_device> m_screen;
	required_device<dspp_device> m_dspp;
	devcb_write_line    m_firq_cb;
	devcb_write_line    m_vsync_cb;
	devcb_write_line    m_hsync_cb;
	devcb_read8         m_xbus_read_cb;
	devcb_write8        m_xbus_write_cb;
	devcb_write16       m_dac_l;
	devcb_write16       m_dac_r;
	devcb_write_line::array<4> m_adb_out_cb;

	uint32_t  m_revision = 0;       /* 03400000 */
	uint32_t  m_csysbits = 0;       /* 03400004 */
	uint32_t  m_vint0 = 0;          /* 03400008 */
	uint32_t  m_vint1 = 0;          /* 0340000c */
	uint32_t  m_audin = 0;          /* 03400020 */
	uint32_t  m_audout = 0;         /* 03400024 */
	uint32_t  m_cstatbits = 0;      /* 03400028 */
	uint32_t  m_wdog = 0;           /* 0340002c */
//	uint32_t  m_hcnt = 0;           /* 03400030 */
//	uint32_t  m_vcnt = 0;           /* 03400034 */
	uint32_t  m_seed = 0;           /* 03400038 */
	uint32_t  m_random = 0;         /* 0340004c */
	uint32_t  m_irq0 = 0;           /* 03400040 / 03400044 */
	uint32_t  m_irq0_enable = 0;    /* 03400048 / 0340004c */
	uint32_t  m_mode = 0;           /* 03400050 / 03400054 */
	uint32_t  m_badbits = 0;        /* 03400058 */
	uint32_t  m_irq1 = 0;           /* 03400060 / 03400064 */
	uint32_t  m_irq1_enable = 0;    /* 03400068 / 0340006c */
	uint32_t  m_hdelay = 0;         /* 03400080 */
	uint32_t  m_adbio = 0;          /* 03400084 */
	uint32_t  m_adbctl = 0;         /* 03400088 */
                                    /* Timers */
	uint32_t  m_timer_count[16]{};  /* 034001** & 8 */
	uint32_t  m_timer_backup[16]{}; /* 034001**+4 & 8 */
	uint64_t  m_timer_ctrl = 0;     /* 03400200 */
	uint32_t  m_slack = 0;          /* 03400220 */
							        /* DMA */
	uint32_t  m_dma_enable = 0;     /* 03400308 */
							        /* Expansion bus */
	uint32_t  m_expctl = 0;         /* 03400400/03400404 */
	uint32_t  m_type0_4 = 0;        /* 03400408 */
	uint32_t  m_dipir1 = 0;         /* 03400410 */
	uint32_t  m_dipir2 = 0;         /* 03400414 */
							        /* Bus signals */
//	uint32_t  m_avdisel = 0;        /* 03400500 - 0340053f */
//	uint32_t  m_avdipoll = 0;       /* 03400540 - 0340057f */
//	uint32_t  m_avdicmdstat = 0;    /* 03400580 - 034005bf */
//	uint32_t  m_avdidata = 0;       /* 034005c0 - 034005ff */
	uint32_t  m_sel;
	uint32_t  m_poll;
							        /* DSPP */
//	uint32_t  m_semaphore = 0;      /* 034017d0 */
//	uint32_t  m_semaack = 0;        /* 034017d4 */
//	uint32_t  m_dsppdma = 0;        /* 034017e0 */
//	uint32_t  m_dspprst0 = 0;       /* 034017e4 */
//	uint32_t  m_dspprst1 = 0;       /* 034017e8 */
//	uint32_t  m_dspppc = 0;         /* 034017f4 */
//	uint32_t  m_dsppnr = 0;         /* 034017f8 */
//	uint32_t  m_dsppgw = 0;         /* 034017fc */
//	uint32_t  m_dsppn[0x400]{};     /* 03401800 - 03401bff DSPP N stack (32bit writes) */
							        /* 03402000 - 034027ff DSPP N stack (16bit writes) */
//	uint32_t  m_dsppei[0x100]{};    /* 03403000 - 034030ff DSPP EI stack (32bit writes) */
							        /* 03403400 - 034035ff DSPP EI stack (16bit writes) */
//	uint32_t  m_dsppeo[0x1f]{};     /* 03403800 - 0340381f DSPP EO stack (32bit reads) */
							        /* 03403c00 - 03403c3f DSPP EO stack (16bit reads) */
//	uint32_t  m_dsppclkreload = 0;  /* 034039dc / 03403fbc */

	void request_fiq(uint32_t irq_req, uint8_t type);

	TIMER_CALLBACK_MEMBER( scan_timer_cb );
	TIMER_CALLBACK_MEMBER( system_timer_cb );
	TIMER_CALLBACK_MEMBER( dac_update_cb );
	emu_timer *m_scan_timer;
	emu_timer *m_dac_timer;
	emu_timer *m_system_timer;
};

DECLARE_DEVICE_TYPE(CLIO, clio_device)


#endif // MAME_MISC_3DO_CLIO_H
