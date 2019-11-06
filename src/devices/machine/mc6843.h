// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

#ifndef MAME_MACHINE_MC6843_H
#define MAME_MACHINE_MC6843_H

#pragma once

#include "imagedev/flopdrv.h"


class mc6843_device : public device_t
{
public:
	mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<int Id, typename T> void set_floppy_drive(T &&tag) { m_floppy[Id].set_tag(std::forward<T>(tag)); }
	template<typename T, typename U, typename V, typename W> void set_floppy_drives(T &&tag0, U &&tag1, V &&tag2, W &&tag3)
	{
		m_floppy[0].set_tag(std::forward<T>(tag0));
		m_floppy[1].set_tag(std::forward<U>(tag1));
		m_floppy[2].set_tag(std::forward<V>(tag2));
		m_floppy[3].set_tag(std::forward<W>(tag3));
	}

	auto irq() { return m_write_irq.bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_drive(int drive);
	void set_side(int side);
	void set_index_pulse(int index_pulse);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_CONT
	};

	optional_device_array<legacy_floppy_image_device, 4> m_floppy;

	devcb_write_line m_write_irq;

	/* registers */
	uint8_t m_CTAR;       /* current track */
	uint8_t m_CMR;        /* command */
	uint8_t m_ISR;        /* interrupt status */
	uint8_t m_SUR;        /* set-up */
	uint8_t m_STRA;       /* status */
	uint8_t m_STRB;       /* status */
	uint8_t m_SAR;        /* sector address */
	uint8_t m_GCR;        /* general count */
	uint8_t m_CCR;        /* CRC control */
	uint8_t m_LTAR;       /* logical address track (=track destination) */

	/* internal state */
	uint8_t  m_drive;
	uint8_t  m_side;
	uint8_t  m_data[128];   /* sector buffer */
	uint32_t m_data_size;   /* size of data */
	uint32_t m_data_idx;    /* current read/write position in data */
	uint32_t m_data_id;     /* chrd_id for sector write */
	uint8_t  m_index_pulse;

	/* trigger delayed actions (bottom halves) */
	emu_timer* m_timer_cont;

	legacy_floppy_image_device* floppy_image();
	void status_update();
	void cmd_end();
	void finish_STZ();
	void finish_SEK();
	int address_search(chrn_id* id);
	int address_search_read(chrn_id* id);
	void finish_RCR();
	void cont_SR();
	void cont_SW();

};

DECLARE_DEVICE_TYPE(MC6843, mc6843_device)

#endif // MAME_MACHINE_MC6843_H
