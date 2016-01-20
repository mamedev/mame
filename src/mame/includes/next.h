// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once

#ifndef __NEXT__
#define __NEXT__

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nscsi_bus.h"
#include "machine/mccs1850.h"
#include "machine/8530scc.h"
#include "machine/nextkbd.h"
#include "machine/upd765.h"
#include "machine/ncr5390.h"
#include "machine/mb8795.h"
#include "machine/nextmo.h"
#include "imagedev/chd_cd.h"
#include "imagedev/harddriv.h"

class next_state : public driver_device
{
public:
	next_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			maincpu(*this, "maincpu"),
			rtc(*this, "rtc"),
			scc(*this, "scc"),
			keyboard(*this, "keyboard"),
			scsibus(*this, "scsibus"),
			scsi(*this, "scsibus:7:ncr5390"),
			net(*this, "net"),
			mo(*this, "mo"),
			fdc(*this, "fdc"),
			vram(*this, "vram"),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> maincpu;
	required_device<mccs1850_device> rtc;
	required_device<scc8530_t> scc;
	required_device<nextkbd_device> keyboard;
	required_device<nscsi_bus_device> scsibus;
	required_device<ncr5390_device> scsi;
	required_device<mb8795_device> net;
	required_device<nextmo_device> mo;
	optional_device<n82077aa_device> fdc; // 040 only

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void setup(UINT32 scr1, int size_x, int size_y, int skip, bool color);

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );
	DECLARE_READ32_MEMBER( rom_map_r );
	DECLARE_READ32_MEMBER( scr2_r );
	DECLARE_WRITE32_MEMBER( scr2_w );
	DECLARE_READ32_MEMBER( scr1_r );
	DECLARE_READ32_MEMBER( irq_status_r );
	DECLARE_READ32_MEMBER( irq_mask_r );
	DECLARE_WRITE32_MEMBER( irq_mask_w );
	DECLARE_READ32_MEMBER( event_counter_r );
	DECLARE_READ32_MEMBER( dsp_r );
	DECLARE_READ32_MEMBER( fdc_control_r );
	DECLARE_WRITE32_MEMBER( fdc_control_w );
	DECLARE_READ32_MEMBER( dma_ctrl_r );
	DECLARE_WRITE32_MEMBER( dma_ctrl_w );
	DECLARE_READ32_MEMBER( dma_regs_r );
	DECLARE_WRITE32_MEMBER( dma_regs_w );
	DECLARE_READ32_MEMBER( scsictrl_r );
	DECLARE_WRITE32_MEMBER( scsictrl_w );
	DECLARE_READ32_MEMBER( phy_r );
	DECLARE_WRITE32_MEMBER( phy_w );
	DECLARE_READ32_MEMBER( timer_data_r );
	DECLARE_WRITE32_MEMBER( timer_data_w );
	DECLARE_READ32_MEMBER( timer_ctrl_r );
	DECLARE_WRITE32_MEMBER( timer_ctrl_w );
	DECLARE_WRITE8_MEMBER( ramdac_w );

	UINT32 scr1;
	UINT32 scr2;
	UINT32 irq_status;
	UINT32 irq_mask;
	int irq_level;
	required_shared_ptr<UINT32> vram;
	UINT8 scsictrl, scsistat;

	UINT32 phy[2];

	attotime timer_tbase;
	UINT16 timer_vbase;
	UINT32 timer_data, timer_next_data;
	UINT32 timer_ctrl;
	emu_timer *timer_tm;

	UINT32 eventc_latch;

	DECLARE_WRITE_LINE_MEMBER(scc_irq);
	DECLARE_WRITE_LINE_MEMBER(keyboard_irq);
	DECLARE_WRITE_LINE_MEMBER(power_irq);
	DECLARE_WRITE_LINE_MEMBER(nmi_irq);

	DECLARE_WRITE_LINE_MEMBER(scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(scsi_drq);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);

	DECLARE_WRITE_LINE_MEMBER(net_tx_irq);
	DECLARE_WRITE_LINE_MEMBER(net_rx_irq);
	DECLARE_WRITE_LINE_MEMBER(net_tx_drq);
	DECLARE_WRITE_LINE_MEMBER(net_rx_drq);

	DECLARE_WRITE_LINE_MEMBER(mo_irq);
	DECLARE_WRITE_LINE_MEMBER(mo_drq);

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void vblank_w(screen_device &screen, bool vblank_state);

protected:
	struct dma_slot {
		UINT32 start, limit, chain_start, chain_limit, current;
		UINT8 state;
		bool supdate, restart, drq;
	};

	enum {
		// write bits
		DMA_SETENABLE    = 0x01,
		DMA_SETSUPDATE   = 0x02,
		DMA_SETREAD      = 0x04,
		DMA_CLRCOMPLETE  = 0x08,
		DMA_RESET        = 0x10,
		DMA_INITBUF      = 0x20,
		DMA_INITBUFTURBO = 0x40,

		// read bits
		DMA_ENABLE       = 0x01,
		DMA_SUPDATE      = 0x02,
		DMA_READ         = 0x04,
		DMA_COMPLETE     = 0x08,
		DMA_BUSEXC       = 0x10
	};

	static const char *dma_targets[0x20];
	static const int dma_irqs[0x20];
	static const bool dma_has_saved[0x20];
	static const int scsi_clocks[4];

	dma_slot dma_slots[0x20];
	UINT32 esp;

	int screen_sx, screen_sy, screen_skip;
	bool screen_color;
	bool vbl_enabled;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void timer_start();
	void timer_update();

	void irq_set(int id, bool raise);
	void irq_check();
	const char *dma_name(int slot);
	void dma_do_ctrl_w(int slot, UINT8 data);
	void dma_drq_w(int slot, bool state);
	void dma_read(int slot, UINT8 &val, bool &eof, bool &err);
	void dma_write(int slot, UINT8 val, bool eof, bool &err);
	void dma_check_update(int slot);
	void dma_check_end(int slot, bool eof);
	void dma_end(int slot);

public:
	DECLARE_DRIVER_INIT(nexts2);
	DECLARE_DRIVER_INIT(next);
	DECLARE_DRIVER_INIT(nextsc);
	DECLARE_DRIVER_INIT(nextst);
	DECLARE_DRIVER_INIT(nextct);
	DECLARE_DRIVER_INIT(nextstc);
	DECLARE_DRIVER_INIT(nextctc);
	DECLARE_DRIVER_INIT(nexts);
	required_device<cpu_device> m_maincpu;
};

#endif
