// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat840k_hle.h

    High-level simulation of the Agat 840K floppy controller card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_AGAT840K_HLE_H
#define MAME_BUS_A2BUS_AGAT840K_HLE_H

#pragma once

#include "a2bus.h"
#include "imagedev/flopdrv.h"
#include "machine/i8255.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agat840k_hle_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	enum : u8
	{
		MXCSR_SYNC  = 0x40,
		MXCSR_TR    = 0x80
	};

	// construction/destruction
	a2bus_agat840k_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t d14_i_b();
	uint8_t d15_i_a();
	uint8_t d15_i_c();
	void d14_o_c(uint8_t data);
	void d15_o_b(uint8_t data);
	void d15_o_c(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(index_0_w);
	DECLARE_WRITE_LINE_MEMBER(index_1_w);

	void index_callback(int unit, int state);

protected:
	// construction/destruction
	a2bus_agat840k_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	enum
	{
		TIMER_ID_WAIT = 0,
		TIMER_ID_SEEK
	};

	required_device_array<legacy_floppy_image_device, 2> m_floppy_image;
	required_device<i8255_device> m_d14;
	required_device<i8255_device> m_d15;

private:
	legacy_floppy_image_device *m_floppy;
	bool m_side;

	std::unique_ptr<uint16_t[]> m_tracks[160];
	int m_count_read;
	int m_count_write;
	bool m_seen_magic;
	int m_current_track;

	u8 m_mxcs;
	int m_unit;
	int m_state;

	int m_seektime;
	int m_waittime;

	emu_timer *m_timer_wait = nullptr;
	emu_timer *m_timer_seek = nullptr;

	uint8_t *m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_AGAT840K_HLE, a2bus_agat840k_hle_device)

#endif  // MAME_BUS_A2BUS_AGAT840K_HLE_H
