// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD HD disk drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_CMDHD_H
#define MAME_BUS_CBMIEC_CMDHD_H

#pragma once

#include "cbmiec.h"
#include "bus/nscsi/devices.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/harddriv.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/msm6242.h"
#include "machine/nscsi_bus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cmd_hd_device

class cmd_hd_device : public device_t, 
					  public device_cbm_iec_interface,
					  public nscsi_device_interface
{
public:
	// construction/destruction
	cmd_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( pbres_changed );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_srq(int state) override;
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

	// nscsi_device_interface overrides
	virtual void scsi_ctrl_changed() override;

private:
	enum
	{
		LED_PWR = 0,
		LED_ACT,
		LED_BSY,
		LED_ERR,
		LED_GEO,
		LED_SW8,
		LED_SW9,
		LED_WRP
	};

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<i8255_device> m_ppi;
	required_device<rtc72421_device> m_rtc;
	output_finder<8> m_leds;
	required_ioport m_pb;
	memory_view m_ram_view;
	required_region_ptr<uint8_t> m_rom;
	memory_share_creator<uint8_t> m_ram;

	uint8_t via0_pa_r();
	void via0_pa_w(uint8_t data);
	uint8_t via0_pb_r();
	void via0_pb_w(uint8_t data);
	void via0_cb1_w(int state);
	void via0_cb2_w(int state);

	uint8_t via1_pa_r();
	void via1_pa_w(uint8_t data);
	uint8_t via1_pb_r();
	void via1_pb_w(uint8_t data);
	void via1_ca2_w(int state);
	void via1_cb2_w(int state);

	uint8_t ppi_pa_r();
	void ppi_pa_w(uint8_t data);
	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);

	void ttl_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	bool m_fst_dir = 0;
	bool m_fst_clk = 0;
	bool m_fst_data = 0;
	bool m_atn_ack = 1;
	bool m_iec_atn = 1;
	bool m_iec_clk = 1;
	bool m_iec_data = 1;
	bool m_sb_reset_ena = 0;

	emu_timer *m_iec_sync_timer;
	TIMER_CALLBACK_MEMBER(iec_sync_tick);
	
	bool m_bdirin = 0;
	u8 m_sasi_out = 0;
	bool m_ack_ff = 0;

	emu_timer *m_ack_clear_timer;
	TIMER_CALLBACK_MEMBER(clear_ack_tick);

	bool m_romos = 1;
	bool m_wpram = 1;
};


// device type definition
DECLARE_DEVICE_TYPE(CMD_HD, cmd_hd_device)


#endif // MAME_BUS_CBMIEC_CMDHD_H
