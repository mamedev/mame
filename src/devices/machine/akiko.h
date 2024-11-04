// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

    Akiko

    Used in the Amiga CD32

    - CD-ROM controller
    - Builtin 1KB NVRAM
    - Chunky to planar converter

***************************************************************************/

#ifndef MAME_MACHINE_AKIKO_H
#define MAME_MACHINE_AKIKO_H

#pragma once

#include "cdrom.h"
#include "imagedev/cdromimg.h"
#include "sound/cdda.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> akiko_device

class akiko_device : public device_t
{
public:
	akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto mem_r_callback() { return m_mem_r.bind(); }
	auto mem_w_callback() { return m_mem_w.bind(); }
	auto int_callback() { return m_int_w.bind(); }
	auto scl_callback() { return m_scl_w.bind(); }
	auto sda_r_callback() { return m_sda_r.bind(); }
	auto sda_w_callback() { return m_sda_w.bind(); }

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void set_mute(bool mute);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// 1X CDROM sector time in msec (300KBps)
	static constexpr int CD_SECTOR_TIME = (1000/((150*1024)/2048));

	// chunky to planar converter
	uint32_t m_c2p_input_buffer[8];
	uint32_t m_c2p_output_buffer[8];
	uint32_t m_c2p_input_index;
	uint32_t m_c2p_output_index;

	// i2c bus
	int m_i2c_scl_out;
	int m_i2c_scl_dir;
	int m_i2c_sda_out;
	int m_i2c_sda_dir;

	// cdrom
	uint32_t m_cdrom_status[2];
	uint32_t m_cdrom_address[2];
	uint32_t m_cdrom_track_index;
	uint32_t m_cdrom_lba_start;
	uint32_t m_cdrom_lba_end;
	uint32_t m_cdrom_lba_cur;
	uint16_t m_cdrom_readmask;
	uint16_t m_cdrom_readreqmask;
	uint32_t m_cdrom_dmacontrol;
	uint32_t m_cdrom_numtracks;
	uint8_t m_cdrom_speed;
	uint8_t m_cdrom_cmd_start;
	uint8_t m_cdrom_cmd_end;
	uint8_t m_cdrom_cmd_resp;

	required_device<cdda_device> m_cdda;
	optional_device<cdrom_image_device> m_cdrom;

	std::unique_ptr<uint8_t[]> m_cdrom_toc;

	emu_timer *m_dma_timer;
	emu_timer *m_frame_timer;

	void nvram_write(uint32_t data);
	uint32_t nvram_read();

	uint8_t mem_r8(offs_t offset);
	void mem_w8(offs_t offset, uint8_t data);

	void c2p_write(uint32_t data);
	uint32_t c2p_read();

	void cdda_stop();
	void cdda_play(uint32_t lba, uint32_t num_blocks);
	void cdda_pause(int pause);
	uint8_t cdda_getstatus(uint32_t *lba);
	void set_cd_status(uint32_t status);

	TIMER_CALLBACK_MEMBER( frame_proc );
	TIMER_CALLBACK_MEMBER( dma_proc );

	void start_dma();
	void setup_response( int len, uint8_t *r1 );

	TIMER_CALLBACK_MEMBER( cd_delayed_cmd );
	void update_cdrom();

	// interface
	devcb_read16 m_mem_r;
	devcb_write16 m_mem_w;
	devcb_write_line m_int_w;
	devcb_write_line m_scl_w;
	devcb_read_line m_sda_r;
	devcb_write_line m_sda_w;
};

// device type definition
DECLARE_DEVICE_TYPE(AKIKO, akiko_device)

#endif // MAME_MACHINE_AKIKO_H
