// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

    Akiko

    Used in the Amiga CD32

    - CD-ROM controller
    - Builtin 1KB NVRAM
    - Chunky to planar converter

***************************************************************************/

#pragma once

#ifndef __AKIKO_H__
#define __AKIKO_H__

#include "emu.h"
#include "cdrom.h"
#include "sound/cdda.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AKIKO_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, AKIKO, 0) \
	akiko_device::set_cputag(*device, _cputag);

#define MCFG_AKIKO_SCL_HANDLER(_devcb) \
	devcb = &akiko_device::set_scl_handler(*device, DEVCB_##_devcb);

#define MCFG_AKIKO_SDA_READ_HANDLER(_devcb) \
	devcb = &akiko_device::set_sda_read_handler(*device, DEVCB_##_devcb);

#define MCFG_AKIKO_SDA_WRITE_HANDLER(_devcb) \
	devcb = &akiko_device::set_sda_write_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> akiko_device

class akiko_device : public device_t
{
public:
	akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~akiko_device() {}

	// callbacks
	template<class _Object> static devcb_base &set_scl_handler(device_t &device, _Object object)
		{ return downcast<akiko_device &>(device).m_scl_w.set_callback(object); }

	template<class _Object> static devcb_base &set_sda_read_handler(device_t &device, _Object object)
		{ return downcast<akiko_device &>(device).m_sda_r.set_callback(object); }

	template<class _Object> static devcb_base &set_sda_write_handler(device_t &device, _Object object)
		{ return downcast<akiko_device &>(device).m_sda_w.set_callback(object); }

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	// inline configuration
	static void set_cputag(device_t &device, const char *tag);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// 1X CDROM sector time in msec (300KBps)
	static const int CD_SECTOR_TIME = (1000/((150*1024)/2048));

	// internal state
	address_space *m_space;

	// chunky to planar converter
	UINT32 m_c2p_input_buffer[8];
	UINT32 m_c2p_output_buffer[8];
	UINT32 m_c2p_input_index;
	UINT32 m_c2p_output_index;

	// i2c bus
	int m_i2c_scl_out;
	int m_i2c_scl_dir;
	int m_i2c_sda_out;
	int m_i2c_sda_dir;

	// cdrom
	UINT32 m_cdrom_status[2];
	UINT32 m_cdrom_address[2];
	UINT32 m_cdrom_track_index;
	UINT32 m_cdrom_lba_start;
	UINT32 m_cdrom_lba_end;
	UINT32 m_cdrom_lba_cur;
	UINT16 m_cdrom_readmask;
	UINT16 m_cdrom_readreqmask;
	UINT32 m_cdrom_dmacontrol;
	UINT32 m_cdrom_numtracks;
	UINT8 m_cdrom_speed;
	UINT8 m_cdrom_cmd_start;
	UINT8 m_cdrom_cmd_end;
	UINT8 m_cdrom_cmd_resp;

	cdda_device *m_cdda;
	cdrom_file *m_cdrom;

	UINT8 *m_cdrom_toc;

	emu_timer *m_dma_timer;
	emu_timer *m_frame_timer;

	int m_cdrom_is_device;

	void nvram_write(UINT32 data);
	UINT32 nvram_read();

	void c2p_write(UINT32 data);
	UINT32 c2p_read();

	void cdda_stop();
	void cdda_play(UINT32 lba, UINT32 num_blocks);
	void cdda_pause(int pause);
	UINT8 cdda_getstatus(UINT32 *lba);
	void set_cd_status(UINT32 status);

	TIMER_CALLBACK_MEMBER( frame_proc );
	TIMER_CALLBACK_MEMBER( dma_proc );

	void start_dma();
	void setup_response( int len, UINT8 *r1 );

	TIMER_CALLBACK_MEMBER( cd_delayed_cmd );
	void update_cdrom();

	// i2c interface
	devcb_write_line m_scl_w;
	devcb_read_line m_sda_r;
	devcb_write_line m_sda_w;

	const char *m_cputag;
};

// device type definition
extern const device_type AKIKO;

#endif
