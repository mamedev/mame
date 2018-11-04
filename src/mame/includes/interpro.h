// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_INTERPRO_H
#define MAME_INCLUDES_INTERPRO_H

#pragma once

#include "cpu/clipper/clipper.h"
#include "machine/cammu.h"

#include "machine/interpro_ioga.h"
#include "machine/interpro_mcga.h"
#include "machine/interpro_sga.h"
#include "machine/interpro_arbga.h"

#include "machine/ram.h"
#include "machine/28fxxx.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/i82586.h"

#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

#include "bus/rs232/rs232.h"

#include "bus/interpro/sr/sr.h"
#include "bus/interpro/sr/sr_cards.h"
#include "bus/interpro/keyboard/keyboard.h"

#include "formats/pc_dsk.h"

#define INTERPRO_CPU_TAG           "cpu"
#define INTERPRO_MMU_TAG           "mmu"
#define INTERPRO_MCGA_TAG          "mcga"
#define INTERPRO_SGA_TAG           "sga"
#define INTERPRO_FDC_TAG           "fdc"
#define INTERPRO_ARBGA_TAG         "arbga"
#define INTERPRO_SCC1_TAG          "scc1"
#define INTERPRO_SCC2_TAG          "scc2"
#define INTERPRO_SERIAL_PORT0_TAG  "serial0"
#define INTERPRO_SERIAL_PORT1_TAG  "serial1"
#define INTERPRO_SERIAL_PORT2_TAG  "serial2"
#define INTERPRO_KEYBOARD_PORT_TAG "kbd"
#define INTERPRO_RTC_TAG           "rtc"
#define INTERPRO_SCSI_TAG          "scsi"
#define INTERPRO_SCSI_ADAPTER_TAG  "host"
#define INTERPRO_SCSI_DEVICE_TAG   INTERPRO_SCSI_TAG ":7:" INTERPRO_SCSI_ADAPTER_TAG
#define INTERPRO_ETH_TAG           "eth"
#define INTERPRO_IOGA_TAG          "ioga"

#define INTERPRO_IDPROM_TAG        "idprom"
#define INTERPRO_EPROM_TAG         "eprom"
#define INTERPRO_FLASH_TAG         "flash"

#define INTERPRO_SRBUS_TAG         "sr"

class interpro_state : public driver_device
{
public:
	interpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, INTERPRO_CPU_TAG)
		, m_ram(*this, RAM_TAG)
		, m_mcga(*this, INTERPRO_MCGA_TAG)
		, m_sga(*this, INTERPRO_SGA_TAG)
		, m_fdc(*this, INTERPRO_FDC_TAG)
		, m_arbga(*this, INTERPRO_ARBGA_TAG)
		, m_scc1(*this, INTERPRO_SCC1_TAG)
		, m_scc2(*this, INTERPRO_SCC2_TAG)
		, m_rtc(*this, INTERPRO_RTC_TAG)
		, m_scsibus(*this, INTERPRO_SCSI_TAG)
		, m_scsi(*this, INTERPRO_SCSI_DEVICE_TAG)
		, m_eth(*this, INTERPRO_ETH_TAG)
		, m_ioga(*this, INTERPRO_IOGA_TAG)
	{
	}

	required_device<clipper_device> m_maincpu;
	required_device<ram_device> m_ram;

	required_device<interpro_mcga_device> m_mcga;
	required_device<interpro_sga_device> m_sga;
	required_device<upd765_family_device> m_fdc;
	required_device<interpro_arbga_device> m_arbga;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	required_device<mc146818_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c90a_device> m_scsi;
	required_device<i82586_base_device> m_eth;
	required_device<interpro_ioga_device> m_ioga;

	DECLARE_DRIVER_INIT(common);

	enum sreg_error_mask
	{
		ERROR_BPID4    = 0x0001,
		ERROR_SRXMMBE  = 0x0002,
		ERROR_SRXHOG   = 0x0004,
		ERROR_SRXNEM   = 0x0008,
		ERROR_SRXVALID = 0x0010,
		ERROR_CBUSNMI  = 0x0020,
		ERROR_CBUSBG   = 0x00c0,
		ERROR_BG       = 0x0070,
		ERROR_BUSHOG   = 0x0080
	};
	DECLARE_READ16_MEMBER(sreg_error_r);

	DECLARE_WRITE16_MEMBER(sreg_led_w);

	enum sreg_status_mask
	{
		STATUS_YELLOW_ZONE = 0x0001,
		STATUS_SRNMI       = 0x0002,
		STATUS_PWRLOSS     = 0x0004,
		STATUS_RED_ZONE    = 0x0008,
		STATUS_BP          = 0x00f0
	};
	DECLARE_READ16_MEMBER(sreg_status_r) { return m_sreg_status; }

	enum sreg_ctrl1_mask
	{
		CTRL1_FLOPLOW    = 0x0001, // 0 = 5.25" floppy selected
		CTRL1_FLOPRDY    = 0x0002, // 1 = plotter fifo empty?
		CTRL1_LEDENA     = 0x0004, // 0 = led display disabled
		CTRL1_LEDDP      = 0x0008, // 0 = led right decimal point disabled
		CTRL1_ETHLOOP    = 0x0010, // 1 = mmbe enabled
		CTRL1_ETHDTR     = 0x0020, // 0 = modem dtr pin activated
		CTRL1_ETHRMOD    = 0x0040, // 0 = sytem configured for remote modems
		CTRL1_FIFOACTIVE = 0x0080  // 0 = plotter fifos reset
	};
	DECLARE_READ16_MEMBER(sreg_ctrl1_r) { return m_sreg_ctrl1; }
	DECLARE_WRITE16_MEMBER(sreg_ctrl1_w);

	enum sreg_ctrl2_mask
	{
		CTRL2_PWRUP     = 0x0003, // 3 = power supply voltage adjusted up by 5%
		CTRL2_HOLDOFF   = 0x0004, // 0 = power supply will shut down 0.33 seconds after switch is turned off
		CTRL2_EXTNMIENA = 0x0008, // 0 = power nmi disabled
		CTRL2_COLDSTART = 0x0010, // 1 = cold start flag
		CTRL2_RESET     = 0x0020, // 0 = soft reset
		CTRL2_BUSENA    = 0x0040, // 0 = clear bus grant error
		CTRL2_FLASHEN   = 0x0080, // 0 = flash eprom writes disabled

		CTRL2_MASK      = 0x004d
	};
	DECLARE_READ16_MEMBER(sreg_ctrl2_r) { return m_sreg_ctrl2; }
	virtual DECLARE_WRITE16_MEMBER(sreg_ctrl2_w);
	DECLARE_READ16_MEMBER(sreg_ctrl3_r) { return m_sreg_ctrl3; }
	DECLARE_WRITE16_MEMBER(sreg_ctrl3_w) { m_sreg_ctrl3 = data; }

	DECLARE_READ8_MEMBER(nodeid_r);

	DECLARE_READ32_MEMBER(unmapped_r);
	DECLARE_WRITE32_MEMBER(unmapped_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void ioga(machine_config &config);
	void interpro_serial1(machine_config &config);
	void interpro_serial2(machine_config &config);
	void interpro(machine_config &config);
	static void interpro_scsi_adapter(device_t *device);
	void c300_data_map(address_map &map);
	void c300_insn_map(address_map &map);
	void c400_data_map(address_map &map);
	void c400_insn_map(address_map &map);
	void interpro_boot_map(address_map &map);
	void interpro_common_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	u16 m_sreg_error;
	u16 m_sreg_status;
	u16 m_sreg_led;
	u16 m_sreg_ctrl1;
	u16 m_sreg_ctrl2;

	u16 m_sreg_ctrl3;
};

class turquoise_state : public interpro_state
{
public:
	turquoise_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_d_cammu(*this, INTERPRO_MMU_TAG "_d")
		, m_i_cammu(*this, INTERPRO_MMU_TAG "_i")
	{
	}

	DECLARE_DRIVER_INIT(turquoise);

	DECLARE_WRITE8_MEMBER(sreg_error_w) { m_sreg_error = data; }

	required_device<cammu_c3_device> m_d_cammu;
	required_device<cammu_c3_device> m_i_cammu;
	void turquoise(machine_config &config);
	void ip2000(machine_config &config);
	void interpro_82586_map(address_map &map);
	void turquoise_base_map(address_map &map);
	void turquoise_main_map(address_map &map);
	void turquoise_io_map(address_map &map);
};

class sapphire_state : public interpro_state
{
public:
	sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_mmu(*this, INTERPRO_MMU_TAG)
		, m_flash_lo(*this, INTERPRO_FLASH_TAG "_lo")
		, m_flash_hi(*this, INTERPRO_FLASH_TAG "_hi")
	{
	}

	DECLARE_DRIVER_INIT(sapphire);

	virtual DECLARE_WRITE16_MEMBER(sreg_ctrl2_w) override;

	required_device<cammu_c4_device> m_mmu;

	required_device<intel_28f010_device> m_flash_lo;
	required_device<intel_28f010_device> m_flash_hi;
	void sapphire(machine_config &config);
	void ip2500(machine_config &config);
	void ip2400(machine_config &config);
	void ip2800(machine_config &config);
	void ip2700(machine_config &config);
	void interpro_82596_map(address_map &map);
	void sapphire_base_map(address_map &map);
	void sapphire_main_map(address_map &map);
	void sapphire_io_map(address_map &map);
};

#endif // MAME_INCLUDES_INTERPRO_H
