// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_SPG290_CDSERVO_H
#define MAME_MACHINE_SPG290_CDSERVO_H

#pragma once

#include "imagedev/cdromimg.h"


class spg290_cdservo_device : public device_t
{
public:
	spg290_cdservo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	spg290_cdservo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cdrom_tag)
		: spg290_cdservo_device(mconfig, tag, owner, clock)
	{
		m_cdrom.set_tag(std::forward<T>(cdrom_tag));
	}

	void write(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t read(offs_t offset, uint32_t mem_mask);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto space_write_cb() { return m_space_write_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(cd_update);

private:
	void change_status();
	void add_qsub(int sector, uint8_t addrctrl, uint8_t track, uint8_t index, uint32_t rel_msf, uint32_t abs_msf, uint16_t crc16=0);
	void generate_qsub();
	void servo_cmd_r();
	void servo_cmd_w();

	optional_device<cdrom_image_device> m_cdrom;
	devcb_write_line m_irq_cb;
	devcb_write8 m_space_write_cb;

	emu_timer *m_cdtimer;

	uint32_t m_addr;
	uint32_t m_data;
	uint32_t m_buf_start;
	uint32_t m_buf_end;
	uint32_t m_buf_ptr;
	uint8_t  m_speed;
	uint8_t  m_seek_min;
	uint8_t  m_seek_sec;
	uint8_t  m_seek_frm;
	uint32_t m_seek_lba;
	uint32_t m_sector_size;
	uint32_t m_dsp_data;
	bool     m_frame_found;
	uint32_t m_control0;
	uint32_t m_control1;
	uint16_t m_skip;
	uint32_t m_dsp_regs[0x10];

	std::unique_ptr<uint32_t[]> m_dsp_memory;
	std::unique_ptr<uint8_t[]> m_qsub;
	uint32_t m_tot_sectors;
	int      m_cur_sector;
};

DECLARE_DEVICE_TYPE(SPG290_CDSERVO, spg290_cdservo_device)

#endif // MAME_MACHINE_SPG290_CDSERVO_H
