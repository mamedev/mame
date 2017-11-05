// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_DISKSYS_H
#define MAME_BUS_NES_DISKSYS_H

#pragma once

#include "nxrom.h"
#include "imagedev/flopdrv.h"


// ======================> nes_disksys_device

class nes_disksys_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_disksys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_ex) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_ex) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void disk_flip_side() override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_region_ptr<uint8_t> m_2c33_rom;

private:
	static void load_proc(device_image_interface &image, bool is_created);
	static void unload_proc(device_image_interface &image);

	std::unique_ptr<uint8_t[]> m_fds_data;    // here, we store a copy of the disk
	required_device<legacy_floppy_image_device> m_disk;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	void load_disk(device_image_interface &image);
	void unload_disk(device_image_interface &image);

	uint16_t m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_transfer;

	uint8_t m_fds_motor_on;
	uint8_t m_fds_door_closed;
	uint8_t m_fds_current_side;
	uint32_t m_fds_head_position;
	uint8_t m_fds_status0;
	uint8_t m_read_mode;
	uint8_t m_drive_ready;

	uint8_t m_fds_sides;
	int m_fds_last_side;
	int m_fds_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_DISKSYS, nes_disksys_device)

#endif // MAME_BUS_NES_DISKSYS_H
