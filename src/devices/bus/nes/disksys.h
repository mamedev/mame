// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_DISKSYS_H
#define __NES_DISKSYS_H

#include "nxrom.h"
#include "imagedev/flopdrv.h"


// ======================> nes_disksys_device

class nes_disksys_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_disksys_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER(read_ex) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_ex) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void disk_flip_side() override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

	static void load_proc(device_image_interface &image);
	static void unload_proc(device_image_interface &image);

private:
	UINT8 *m_2c33_rom;
	UINT8 *m_fds_data;    // here, we store a copy of the disk
	required_device<legacy_floppy_image_device> m_disk;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	void load_disk(device_image_interface &image);
	void unload_disk(device_image_interface &image);

	UINT16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_transfer;

	UINT8 m_fds_motor_on;
	UINT8 m_fds_door_closed;
	UINT8 m_fds_current_side;
	UINT32 m_fds_head_position;
	UINT8 m_fds_status0;
	UINT8 m_read_mode;
	UINT8 m_drive_ready;

	UINT8 m_fds_sides;
	int m_fds_last_side;
	int m_fds_count;
};



// device type definition
extern const device_type NES_DISKSYS;

#endif
