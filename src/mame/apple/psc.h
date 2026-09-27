// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_PSC_H
#define MAME_APPLE_PSC_H

#pragma once

#include "cpu/m68000/m68040.h"
#include "machine/6522via.h"
#include "machine/am79c940.h"
#include "machine/applefdintf.h"
#include "machine/ncr53c90.h"
#include "sound/dac.h"

#include "speaker.h"

class psc_device :  public device_t
{
public:
	// construction/destruction
	psc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	void set_scsi_device(ncr53c94_device *device) { m_ncr = device; }
	template <typename T> void set_mace_tag(T &&tag) { m_mace.set_tag(std::forward<T>(tag)); }

	// interface routines
	auto write_cb1() { return m_cb1.bind(); }   // ADB clock
	auto write_cb2() { return m_cb2.bind(); }   // ADB data

	auto read_pa1()  { return m_pa1.bind(); }   // ID bits
	auto read_pa2()  { return m_pa2.bind(); }
	auto read_pa4()  { return m_pa4.bind(); }
	auto read_pa6()  { return m_pa6.bind(); }

	virtual void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	void cb1_w(int state);  // ADB clock
	void cb2_w(int state);  // ADB data
	void scsi_irq_w(int state);
	void scc_irq_w(int state);
	void vbl_irq_w(int state);
	void fdc_irq_w(int state);

	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	template <u8 mask> void via2_irq_w(int state);

	void via_sync();

	void scsi_drq_w(int state);
	void enet_irq_w(int state);
	void enet_tx_drq_w(int state);
	void enet_rx_drq_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t via_in_b();
	virtual void via_out_b(uint8_t data);

	virtual u32 psc_regs_r(offs_t offset);
	virtual void psc_regs_w(offs_t offset, u32 data, u32 mem_mask);

	devcb_write_line m_cb1, m_cb2;
	devcb_write_line write_pb4, write_pb5;
	devcb_read_line read_pb3;
	devcb_read_line m_pa1, m_pa2, m_pa4, m_pa6;

	required_device<m68000_musashi_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<dac_16bit_r2r_device> m_dac_l, m_dac_r;

private:
	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, u16 data, u16 mem_mask);
	void recalc_via2_irqs();

	u16 dma_ctrl_r(offs_t offset);
	void dma_ctrl_w(offs_t offset, u16 data);
	u32 dma_set_r(offs_t offset);
	void dma_set_w(offs_t offset, u32 data, u32 mem_mask);
	void enet_dma_kick();
	bool enet_dma_ready(int channel) const;
	void enet_dma_complete(int channel);
	TIMER_CALLBACK_MEMBER(enet_dma_tick);
	void recalc_dma_irqs();
	void recalc_lv3();
	void recalc_lv4();
	void recalc_lv5();
	void recalc_lv6();

	uint8_t via_in_a();
	uint8_t via2_in_a();
	uint32_t scc_fake_r();
	uint32_t newage_fake_r();
	void via_out_a(uint8_t data);
	void field_interrupts();
	void via_out_cb1(int state);
	void via_out_cb2(int state);
	void via1_irq(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	TIMER_CALLBACK_MEMBER(singer_tick);

	u16 m_psc_regs[0x2000 / 4];

	// VIA2 (which is a very minimal pseudo-VIA, with pretty different semantics from the 'standard' ones) registers.
	// Yes, there are only 3.
	u8 m_slot_irqs, m_ifr, m_ier;

	emu_timer *m_6015_timer, *m_singer_timer;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;

	s32 m_drq, m_scsi_irq, m_fdc_irq;

	u32 m_audio_out_ptr;
	u32 m_audio_out_offset;
	u32 m_audio_out_length;

	u32 m_l3if, m_l3ier, m_l4if, m_l4ier, m_l5if, m_l5ier, m_l6if, m_l6ier;

	u16 m_dma_control[7];
	u32 m_dma_addr[7][2];
	u32 m_dma_cnt[7][2];
	u16 m_dma_cmdstat[7][2];
	u32 m_dma_irqstat;
	optional_device<am79c940_device> m_mace;
	emu_timer *m_enet_timer;
	bool m_enet_tx_drq, m_enet_rx_drq;
	u16 m_enet_rx_offset;
	u8 m_enet_rx_status;

	required_address_space m_space;
	ncr53c94_device *m_ncr;
};

// device type definition
DECLARE_DEVICE_TYPE(PSC, psc_device)

#endif // MAME_APPLE_PSC_H
