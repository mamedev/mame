// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_dk.h: include file for 990_dk.c
*/

#ifndef MAME_BUS_TI99X_990_DK_H
#define MAME_BUS_TI99X_990_DK_H

#pragma once

DECLARE_DEVICE_TYPE(TI99X_FD800, fd800_legacy_device)

class fd800_legacy_device : public device_t
{
public:
	fd800_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cru_r(offs_t offset);
	void cru_w(offs_t offset, uint8_t data);
	auto int_cb() { return m_int_line.bind(); }

private:
	enum buf_mode_t
	{
		bm_off, bm_read, bm_write
	};

	static constexpr unsigned MAX_FLOPPIES = 4;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void set_interrupt_line();

	int     read_id(int unit, int head, int *cylinder_id, int *sector_id);
	int     find_sector(int unit, int head, int sector, int *data_id);
	int     do_seek(int unit, int cylinder, int head);
	int     do_restore(int unit);
	void    do_read();
	void    do_write();
	void    do_cmd();

	uint16_t m_recv_buf;
	uint16_t m_stat_reg;
	uint16_t m_xmit_buf;
	uint16_t m_cmd_reg;

	int m_interrupt_f_f;
	devcb_write_line m_int_line;

	uint8_t m_buf[128];
	int m_buf_pos;
	buf_mode_t m_buf_mode;
	int m_unit;
//  int m_head;
	int m_sector;
	/*int m_non_seq_mode;*/
//  int m_ddam;

	struct
	{
	//  legacy_floppy_image_device *img;
		int phys_cylinder;
		int log_cylinder[2];
		int seclen;
	} m_drv[MAX_FLOPPIES];
};

// LEGACY_FLOPPY_OPTIONS_EXTERN(fd800);

#endif // MAME_BUS_TI99X_990_DK_H
