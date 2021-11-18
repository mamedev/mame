// license:BSD-3-Clause
// copyright-holders:
/*
 *  silentype printer
 *
 */
#include "machine/bitmap_printer.h"
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
	DECLARE_READ_LINE_MEMBER( serial_data ) { return 0; } // should return shift register on read (unimplemented)

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

	const int dpi = 60;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide at 60 dpi
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high at 60 dpi
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int distfrombottom = 50;

	int m_xpos = PAPER_WIDTH / 2 * 2;  // middle of paper (position in half steps)
	int m_ypos = 0;

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	int right_offset = 0;
	int left_offset = 3;


	// The silentype printer has a thermal printhead that can overheat if continuously powered.
	// A 555 timer is triggered upon activation of any of the printhead drive lines.
	// The timer will clear all of the printhead drive lines after 10ms (10000 usec) to
	// protect the printhead from damage and to prevent a fire hazard.

	// It's possible that the bidirectional capability of the interface was to verify that this
	// failsafe protection circuit was functioning correctly as the shift register would be cleared after
	// 10 ms.  Values could be written to the shift register and then read back out after a delay.
	// If the shift register values could still be read back after 10 ms, then the protection circuit would
	// not be functioning properly since those bits should be cleared.

	double headtemp[7] = {0.0}; // initialize to zero - avoid nan bugs
	int heattime = 3000;   // time in usec to hit max temp  (smaller numbers mean faster)
	int decaytime = 1000;  // time in usec to cool off

	int lastheadbits = 0;
	int xdirection = 0;
	int newpageflag = 0;

	int page_count = 0;

	double last_update_time = 0.0;  // strange behavior if we don't initialize

	void adjust_headtemp(u8 pin_status, double time_elapsed,  double& temp);
	void darken_pixel(double headtemp, unsigned int& pixel);
	int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern);
	s32 x_pixel_coord(s32 xpos) { return xpos / 2; }  // x position in half steps
	s32 y_pixel_coord(s32 ypos) { return ypos * 7 / 4 / 2; }  // y position given in half steps, full step is 7/4 pixels
	void update_head_pos() 
	{
		m_bitmap_printer->setheadpos(x_pixel_coord(m_xpos), y_pixel_coord(m_ypos));
	}


};

DECLARE_DEVICE_TYPE(SILENTYPE_PRINTER, silentype_printer_device)

#endif // MAME_MACHINE_SILENTYPE_PRINTER_H
