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
	next_state(const machine_config &mconfig, device_type type, const char *tag)
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

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void setup(uint32_t scr1, int size_x, int size_y, int skip, bool color);

	uint8_t io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t rom_map_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t scr2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void scr2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t scr1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t irq_status_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t irq_mask_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void irq_mask_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t event_counter_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t dsp_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t fdc_control_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void fdc_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dma_ctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dma_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dma_regs_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dma_regs_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t scsictrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void scsictrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t phy_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void phy_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t timer_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void timer_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t timer_ctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void timer_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ramdac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t scr1;
	uint32_t scr2;
	uint32_t irq_status;
	uint32_t irq_mask;
	int irq_level;
	required_shared_ptr<uint32_t> vram;
	uint8_t scsictrl, scsistat;

	uint32_t phy[2];

	attotime timer_tbase;
	uint16_t timer_vbase;
	uint32_t timer_data, timer_next_data;
	uint32_t timer_ctrl;
	emu_timer *timer_tm;

	uint32_t eventc_latch;

	void scc_irq(int state);
	void keyboard_irq(int state);
	void power_irq(int state);
	void nmi_irq(int state);

	void scsi_irq(int state);
	void scsi_drq(int state);

	void fdc_irq(int state);
	void fdc_drq(int state);

	void net_tx_irq(int state);
	void net_rx_irq(int state);
	void net_tx_drq(int state);
	void net_rx_drq(int state);

	void mo_irq(int state);
	void mo_drq(int state);

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void vblank_w(screen_device &screen, bool vblank_state);

protected:
	struct dma_slot {
		uint32_t start, limit, chain_start, chain_limit, current;
		uint8_t state;
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
	uint32_t esp;

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
	void dma_do_ctrl_w(int slot, uint8_t data);
	void dma_drq_w(int slot, bool state);
	void dma_read(int slot, uint8_t &val, bool &eof, bool &err);
	void dma_write(int slot, uint8_t val, bool eof, bool &err);
	void dma_check_update(int slot);
	void dma_check_end(int slot, bool eof);
	void dma_end(int slot);

public:
	void init_nexts2();
	void init_next();
	void init_nextsc();
	void init_nextst();
	void init_nextct();
	void init_nextstc();
	void init_nextctc();
	void init_nexts();
	required_device<cpu_device> m_maincpu;
};

#endif
