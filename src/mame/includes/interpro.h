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

#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/28fxxx.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/i82586.h"

#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "bus/rs232/rs232.h"

#include "bus/interpro/sr/sr.h"
#include "bus/interpro/sr/sr_cards.h"
#include "bus/interpro/keyboard/keyboard.h"
#include "bus/interpro/mouse/mouse.h"

#include "formats/pc_dsk.h"
#include "softlist.h"

#define INTERPRO_CPU_TAG           "cpu"
#define INTERPRO_MMU_TAG           "cammu"
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
#define INTERPRO_MOUSE_PORT_TAG    "mse"
#define INTERPRO_RTC_TAG           "rtc"
#define INTERPRO_SCSI_TAG          "scsi"
#define INTERPRO_SCSI_ADAPTER_TAG  "host"
#define INTERPRO_SCSI_DEVICE_TAG   INTERPRO_SCSI_TAG ":7:" INTERPRO_SCSI_ADAPTER_TAG
#define INTERPRO_ETH_TAG           "eth"
#define INTERPRO_IOGA_TAG          "ioga"

#define INTERPRO_NODEID_TAG        "nodeid"
#define INTERPRO_IDPROM_TAG        "idprom"
#define INTERPRO_EPROM_TAG         "eprom"
#define INTERPRO_FLASH_TAG         "flash"

#define INTERPRO_SLOT_TAG          "slot"

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
		, m_scc1(*this, INTERPRO_SCC1_TAG)
		, m_scc2(*this, INTERPRO_SCC2_TAG)
		, m_rtc(*this, INTERPRO_RTC_TAG)
		, m_scsibus(*this, INTERPRO_SCSI_TAG)
		, m_eth(*this, INTERPRO_ETH_TAG)
		, m_ioga(*this, INTERPRO_IOGA_TAG)
		, m_softlist(*this, "softlist")
		, m_diag_led(*this, "digit0")
	{
	}

	required_device<clipper_device> m_maincpu;
	required_device<ram_device> m_ram;

	required_device<interpro_mcga_device> m_mcga;
	required_device<interpro_sga_device> m_sga;
	required_device<upd765_family_device> m_fdc;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	required_device<mc146818_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<i82586_base_device> m_eth;
	required_device<interpro_ioga_device> m_ioga;

	required_device<software_list_device> m_softlist;

	void init_common();

	virtual DECLARE_READ32_MEMBER(unmapped_r);
	virtual DECLARE_WRITE32_MEMBER(unmapped_w);

	enum error_mask : u16
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
	DECLARE_READ16_MEMBER(error_r);

	DECLARE_WRITE16_MEMBER(led_w);

	enum status_mask : u16
	{
		STATUS_YELLOW_ZONE = 0x0001,
		STATUS_SRNMI       = 0x0002,
		STATUS_PWRLOSS     = 0x0004,
		STATUS_RED_ZONE    = 0x0008,
		STATUS_BP          = 0x00f0
	};
	DECLARE_READ16_MEMBER(status_r) { return m_status; }

	virtual DECLARE_READ16_MEMBER(ctrl1_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(ctrl1_w) = 0;
	virtual DECLARE_READ16_MEMBER(ctrl2_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(ctrl2_w) = 0;

	DECLARE_READ8_MEMBER(nodeid_r);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void ioga(machine_config &config);
	void interpro_serial(machine_config &config);
	void interpro(machine_config &config);
	static void interpro_scsi_adapter(device_t *device);
	static void interpro_cdrom(device_t *device);
	void interpro_boot_map(address_map &map);
	void interpro_common_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	output_finder<> m_diag_led;
	emu_timer *m_reset_timer;

	u16 m_error;
	u16 m_status;
	u16 m_led;
};

class emerald_state : public interpro_state
{
public:
	emerald_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_d_cammu(*this, INTERPRO_MMU_TAG "_d")
		, m_i_cammu(*this, INTERPRO_MMU_TAG "_i")
		, m_scsi(*this, INTERPRO_SCSI_DEVICE_TAG)
		, m_bus(*this, INTERPRO_SLOT_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(error_w) { m_error = data; }

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
		CTRL1_FLOPRDY    = 0x0002, // floppy ready enable?
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_ETHLOOP    = 0x0010, // ethernet loopback enable?
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable?
		CTRL1_ETHRMOD    = 0x0040, // remote modem configured (read)?
		CTRL1_CLIPRESET  = 0x0040, // hard reset (write)?
		CTRL1_FIFOACTIVE = 0x0080  // plotter fifo active?
	};
	DECLARE_READ16_MEMBER(ctrl1_r) override { return m_ctrl1; }
	DECLARE_WRITE16_MEMBER(ctrl1_w) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0001, // power supply voltage adjust?
		CRTL2_PWRENA    = 0x0002, // ?
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FRCPARITY = 0x0080, // ?

		CTRL2_WMASK     = 0x000f
	};
	DECLARE_READ16_MEMBER(ctrl2_r) override { return m_ctrl2; }
	DECLARE_WRITE16_MEMBER(ctrl2_w) override;

	required_device<cammu_c3_device> m_d_cammu;
	required_device<cammu_c3_device> m_i_cammu;
	required_device<ncr53c90a_device> m_scsi;
	required_device<srx_bus_device> m_bus;

	void emerald(machine_config &config);
	void ip6000(machine_config &config);
	void interpro_82586_map(address_map &map);
	void emerald_base_map(address_map &map);
	void emerald_main_map(address_map &map);
	void emerald_io_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u16 m_ctrl1;
	u16 m_ctrl2;
};

class turquoise_state : public interpro_state
{
public:
	turquoise_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_d_cammu(*this, INTERPRO_MMU_TAG "_d")
		, m_i_cammu(*this, INTERPRO_MMU_TAG "_i")
		, m_kbd_port(*this, INTERPRO_KEYBOARD_PORT_TAG)
		, m_mse_port(*this, INTERPRO_MOUSE_PORT_TAG)
		, m_scsi(*this, INTERPRO_SCSI_DEVICE_TAG)
		, m_bus(*this, INTERPRO_SLOT_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(error_w) { m_error = data; }

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
		CTRL1_FLOPRDY    = 0x0002, // floppy ready enable?
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_ETHLOOP    = 0x0010, // ethernet loopback enable?
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable?
		CTRL1_ETHRMOD    = 0x0040, // remote modem configured (read)?
		CTRL1_CLIPRESET  = 0x0040, // hard reset (write)?
		CTRL1_FIFOACTIVE = 0x0080  // plotter fifo active?
	};
	DECLARE_READ16_MEMBER(ctrl1_r) override { return m_ctrl1; }
	DECLARE_WRITE16_MEMBER(ctrl1_w) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0001, // power supply voltage adjust?
		CRTL2_PWRENA    = 0x0002, // ?
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FRCPARITY = 0x0080, // ?

		CTRL2_WMASK     = 0x000f
	};
	DECLARE_READ16_MEMBER(ctrl2_r) override { return m_ctrl2; }
	DECLARE_WRITE16_MEMBER(ctrl2_w) override;

	required_device<cammu_c3_device> m_d_cammu;
	required_device<cammu_c3_device> m_i_cammu;
	required_device<interpro_keyboard_port_device> m_kbd_port;
	required_device<interpro_mouse_port_device> m_mse_port;
	required_device<ncr53c90a_device> m_scsi;
	required_device<cbus_bus_device> m_bus;

	void turquoise(machine_config &config);
	void ip2000(machine_config &config);
	void interpro_82586_map(address_map &map);
	void turquoise_base_map(address_map &map);
	void turquoise_main_map(address_map &map);
	void turquoise_io_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u16 m_ctrl1;
	u16 m_ctrl2;
};

class sapphire_state : public interpro_state
{
public:
	sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_mmu(*this, INTERPRO_MMU_TAG)
		, m_scsi(*this, INTERPRO_SCSI_DEVICE_TAG)
		, m_arbga(*this, INTERPRO_ARBGA_TAG)
		, m_flash_lsb(*this, INTERPRO_FLASH_TAG "_lsb")
		, m_flash_msb(*this, INTERPRO_FLASH_TAG "_msb")
	{
	}

	virtual DECLARE_READ32_MEMBER(unmapped_r) override;
	virtual DECLARE_WRITE32_MEMBER(unmapped_w) override;

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
								   // unused
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_MMBE       = 0x0010, // mmbe enable
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable
		CTRL1_ETHRMOD    = 0x0040, // 0 = sytem configured for remote modems
		CTRL1_FIFOACTIVE = 0x0080  // 0 = plotter fifos reset
	};
	DECLARE_READ16_MEMBER(ctrl1_r) override { return m_ctrl1; }
	DECLARE_WRITE16_MEMBER(ctrl1_w) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0003, // power supply voltage adjust
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FLASHEN   = 0x0080, // flash eprom write enable
	};
	DECLARE_READ16_MEMBER(ctrl2_r) override { return m_ctrl2; }
	DECLARE_WRITE16_MEMBER(ctrl2_w) override;

	required_device<cammu_c4_device> m_mmu;
	required_device<ncr53c94_device> m_scsi;
	required_device<interpro_arbga_device> m_arbga;
	required_device<intel_28f010_device> m_flash_lsb;
	required_device<intel_28f010_device> m_flash_msb;

	void sapphire(machine_config &config);

	void interpro_82596_map(address_map &map);
	void sapphire_base_map(address_map &map);
	void sapphire_main_map(address_map &map);
	void sapphire_io_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u16 m_ctrl1;
	u16 m_ctrl2;
};

class cbus_sapphire_state : public sapphire_state
{
public:
	cbus_sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: sapphire_state(mconfig, type, tag)
		, m_kbd_port(*this, INTERPRO_KEYBOARD_PORT_TAG)
		, m_mse_port(*this, INTERPRO_MOUSE_PORT_TAG)
		, m_bus(*this, INTERPRO_SLOT_TAG)
	{
	}

	void cbus_sapphire(machine_config &config);

	void ip2500(machine_config &config);
	void ip2400(machine_config &config);
	void ip2700(machine_config &config);
	void ip2800(machine_config &config);

protected:
	required_device<interpro_keyboard_port_device> m_kbd_port;
	required_device<interpro_mouse_port_device> m_mse_port;
	required_device<cbus_bus_device> m_bus;
};

class srx_sapphire_state : public sapphire_state
{
public:
	srx_sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: sapphire_state(mconfig, type, tag)
		, m_bus(*this, INTERPRO_SLOT_TAG)
	{
	}

	void srx_sapphire(machine_config &config);

	void ip6400(machine_config &config);
	void ip6700(machine_config &config);
	void ip6800(machine_config &config);

protected:
	required_device<srx_bus_device> m_bus;
};

#endif // MAME_INCLUDES_INTERPRO_H
