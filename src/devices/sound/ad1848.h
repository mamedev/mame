// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_SOUND_AD1848_H
#define MAME_SOUND_AD1848_H

#pragma once

#include "sound/dac.h"


class ad1848_device : public device_t
{
public:
	ad1848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_irq_cb.bind(); }
	auto drq() { return m_drq_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t dack_r();
	void dack_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

private:
	union {
		struct {
			uint8_t linp;
			uint8_t rinp;
			uint8_t laux1;
			uint8_t raux1;
			uint8_t laux2;
			uint8_t raux2;
			uint8_t lout;
			uint8_t rout;
			uint8_t dform;
			uint8_t iface;
			uint8_t pinc;
			uint8_t init;
			uint8_t misc;
			uint8_t mix;
			uint8_t ubase;
			uint8_t lbase;
		};
		uint8_t idx[15];
	} m_regs;
	uint8_t m_addr;
	uint8_t m_stat;
	uint16_t m_count;
	uint32_t m_samples;
	uint8_t m_sam_cnt;
	bool m_play, m_mce, m_trd, m_irq;
	devcb_write_line m_irq_cb;
	devcb_write_line m_drq_cb;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	emu_timer *m_timer;
};

DECLARE_DEVICE_TYPE(AD1848, ad1848_device)

#endif // MAME_SOUND_AD1848_H
