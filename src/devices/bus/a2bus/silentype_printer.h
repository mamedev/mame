// license:BSD-3-Clause
// copyright-holders:
/*
 *  silentype printer
 *
 */
#include "bitmap_printer.h"
#include "machine/steppers.h"

#ifndef MAME_MACHINE_SILENTYPE_PRINTER_H
#define MAME_MACHINE_SILENTYPE_PRINTER_H

#pragma once

class silentype_printer_device : public device_t
{
public:
	silentype_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	silentype_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:

	DECLARE_READ_LINE_MEMBER( margin_switch_input ) { return (m_xpos <= 0); }

	void update_printhead(uint8_t data);
	void update_pf_stepper(uint8_t data);
	void update_cr_stepper(uint8_t data);

protected:
	// device-level overrides

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:

	uint8_t *m_rom;
	uint8_t m_ram[256];

	int m_xpos = 250;
	int m_ypos = 0;
	uint16_t m_shift_reg = 0;
	uint16_t m_parallel_reg = 0;
	int m_romenable = 0;  // start off disabled

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;


	int right_offset = 0;
	int left_offset = 3;

	double headtemp[7] = {0.0}; // initialize to zero - nan bugs
	int heattime = 3000;   // time in usec to hit max temp  (smaller numbers mean faster)
	int decaytime = 1000;  // time in usec to cool off

	int hstepperlast = 0;
	int vstepperlast = 0;
	int lastheadbits = 0;
	int xdirection;
	int newpageflag;

	int page_count=0;

	double last_update_time = 0.0;  // strange behavior if we don't initialize

 private:

	const int dpi=60;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide at 60 dpi
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high at 60 dpi
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int distfrombottom = 50;

	void adjust_headtemp(u8 pin_status, double time_elapsed,  double& temp);
	void darken_pixel(double headtemp, unsigned int& pixel);
	int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern);
	s32 ypos_coord(s32 ypos) { return ypos * 7 / 4 / 2; }
};

DECLARE_DEVICE_TYPE(SILENTYPE_PRINTER, silentype_printer_device)

#endif // MAME_MACHINE_SILENTYPE_PRINTER_H
