// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_JAZZ_H
#define MAME_INCLUDES_JAZZ_H

#pragma once

#include "cpu/mips/r4000.h"

// memory
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/28fxxx.h"

// various hardware
#include "machine/jazz_mct_adr.h"
#include "machine/dp83932c.h"
#include "machine/mc146818.h"
#include "machine/ins8250.h"
#include "machine/ncr5390.h"
#include "machine/upd765.h"
#include "machine/at_keybc.h"
#include "machine/pc_lpt.h"
#include "machine/i82357.h"

// video
#include "screen.h"
#include "video/ims_cvc.h"

// audio
#include "sound/spkrdev.h"
#include "speaker.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "bus/rs232/rs232.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"

class jazz_state : public driver_device
{
public:
	jazz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_vram(*this, "vram")
		, m_mct_adr(*this, "mct_adr")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr53c94")
		, m_fdc(*this, "fdc")
		, m_rtc(*this, "rtc")
		, m_nvram(*this, "nvram")
		, m_flash(*this, "flash")
		, m_kbdc(*this, "kbdc")
		, m_network(*this, "net")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "g364")
		, m_ace(*this, "ace%u", 0)
		, m_lpt(*this, "lpt")
		, m_isp(*this, "isp")
		, m_buzzer(*this, "buzzer")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void jazz_common_map(address_map &map);

	// machine config
	void jazz(machine_config &config);

public:
	void mmr4000be(machine_config &config);
	void mmr4000le(machine_config &config);

	void init_common();

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	// devices
	required_device<r4000_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<ram_device> m_vram;
	required_device<jazz_mct_adr_device> m_mct_adr;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_scsi;
	required_device<n82077aa_device> m_fdc;
	required_device<mc146818_device> m_rtc;
	required_device<nvram_device> m_nvram;
	required_device<amd_28f020_device> m_flash;
	required_device<ps2_keyboard_controller_device> m_kbdc;
	required_device<dp83932c_device> m_network;
	required_device<screen_device> m_screen;
	required_device<g364_device> m_ramdac;
	required_device_array<ns16550_device, 2> m_ace;
	required_device<pc_lpt_device> m_lpt;
	required_device<i82357_device> m_isp;
	required_device<speaker_sound_device> m_buzzer;

private:
	// machine state
	u8 m_led;
};

#endif // MAME_INCLUDES_JAZZ_H
