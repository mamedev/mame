// license: BSD-3-Clause
// copyright-holders: Charles MacDonald, Devin Hill, Fabio Dalla Libera

#ifndef MAME_BUS_SG1000_EXP_SK1100_SP400_H
#define MAME_BUS_SG1000_EXP_SK1100_SP400_H

#pragma once


#include "sk1100prn.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/m6805/m68705.h"
#include "machine/alps_dpg1302.h"



class sp400_printer_device : public device_t,
		     public device_sk1100_printer_port_interface
{
public:

	sp400_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual DECLARE_READ_LINE_MEMBER( output_fault ) override { return 1; }
	virtual DECLARE_READ_LINE_MEMBER( output_busy ) override { return m_busy; }

	const tiny_rom_entry * device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_data ) override{ m_data=state;}
	DECLARE_INPUT_CHANGED_MEMBER(misc_interaction);

private:

	void dser_map(address_map &map);
	void dser_p1_w(uint8_t v);
	uint8_t dser_p2_r();
	void dser_p2_w(uint8_t v);

	uint8_t mot_pa_r();
	void mot_pb_w(uint8_t v);
	uint8_t mot_pc_r();
	void mot_pc_w(uint8_t v);
	uint8_t mot_pd_r();
	void update_pen_state();
	uint32_t update_panel(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<i8035_device>   m_dsercpu; // "deserializer cpu"
	required_device<m6805_hmos_device> m_motcpu; // "motor cpu"

	int m_data, m_busy;
	int m_dserdata, m_dserstrobe;
	int m_motbusy, m_motPenUp, m_motPenDown;

	required_device<alps_dpg1302_plotter_device> m_plotter;
	required_ioport m_frontbuttons;
	required_ioport m_misc;
};


DECLARE_DEVICE_TYPE(SP400_PRINTER, sp400_printer_device)

#endif
