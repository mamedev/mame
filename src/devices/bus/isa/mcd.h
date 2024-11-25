// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_MCD_H
#define MAME_BUS_ISA_MCD_H

#pragma once

#include "isa.h"
#include "imagedev/cdromimg.h"
#include "sound/cdda.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mcd_isa_device

class mcd_isa_device : public cdrom_image_device,
	public device_isa16_card_interface
{
public:
	// construction/destruction
	mcd_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint16_t dack16_r(int line) override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cdda_device> m_cdda;
	uint8_t data_r();
	uint8_t flag_r();
	void cmd_w(uint8_t data);
	void reset_w(uint8_t data);

	void map(address_map &map) ATTR_COLD;

	bool read_sector(bool first = false);

	bool m_change;
	bool m_newstat;
	bool m_data;
	uint8_t m_stat;
	uint8_t m_buf[2352];
	int m_buf_count;
	int m_buf_idx;
	uint8_t m_cmdbuf[16];
	int m_cmdbuf_count;
	int m_cmdrd_count;
	int m_cmdbuf_idx;
	uint8_t m_mode;
	uint8_t m_cmd;
	uint8_t m_conf;
	uint8_t m_irq;
	uint8_t m_dma;
	uint16_t m_dmalen;
	uint32_t m_readmsf;
	uint32_t m_readcount;
	bool m_locked;
	int m_drvmode;
	int m_curtoctrk;
	enum {
		STAT_CMD_CHECK = 0x01,
		STAT_PLAY_CDDA = 0x02,
		STAT_ERROR = 0x04,
		STAT_DISK_CDDA = 0x08,
		STAT_SPIN = 0x10,
		STAT_CHANGE = 0x20,
		STAT_READY = 0x40,
		STAT_OPEN = 0x80
	};
	enum {
		CMD_GET_INFO = 0x10,
		CMD_GET_Q = 0x20,
		CMD_GET_STAT = 0x40,
		CMD_SET_MODE = 0x50,
		CMD_SOFT_RESET = 0x60,
		CMD_STOPCDDA = 0x70,
		CMD_CONFIG = 0x90,
		CMD_SET_VOL = 0xae,
		CMD_READ1X = 0xc0,
		CMD_READ2X = 0xc1,
		CMD_GET_VER = 0xdc,
		CMD_STOP = 0xf0,
		CMD_EJECT = 0xf6,
		CMD_LOCK = 0xfe
	};
	enum {
		MODE_MUTE = 0x01,
		MODE_GET_TOC = 0x04,
		MODE_STOP = 0x08,
		MODE_ECC = 0x20,
		MODE_DATA = 0x40
	};
	enum {
		DRV_MODE_STOP,
		DRV_MODE_READ,
		DRV_MODE_CDDA
	};
	enum {
		FLAG_NODATA = 2,
		FLAG_NOSTAT = 4,
		FLAG_UNK = 8, //??
		FLAG_OPEN = 16
	};
	enum {
		IRQ_DATAREADY = 1,
		IRQ_DATACOMP = 2,
		IRQ_ERROR = 4
	};
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_MCD, mcd_isa_device)

#endif // MAME_BUS_ISA_MCD_H
