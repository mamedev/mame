// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_DISKSYS_H
#define MAME_BUS_NES_DISKSYS_H

#pragma once

#include "nxrom.h"
#include "imagedev/flopdrv.h"
#include "sound/rp2c33_snd.h"


// ======================> nes_disksys_device

class nes_disksys_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_disksys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_ex(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	void disk_flip_side();
	DECLARE_INPUT_CHANGED_MEMBER(flip_side_changed);

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_region_ptr<uint8_t> m_2c33_rom;

private:
	static void load_proc(device_image_interface &image, bool is_created);
	static void unload_proc(device_image_interface &image);

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) { irq_timer_tick(0); watchdog_tick(address); } }
	// DRAM refresh watchdog (NESdev, Family Computer Disk System, $4030): the 2C33 refreshes a DRAM row on every CPU cycle
	// that does not access PRG-RAM ($6000-$DFFF); a 7-bit refresh counter counts those cycles and a 12-bit CPU cycle timer
	// raises an IRQ ($4030.D1, enabled by $4023.D7) when it underflows before the refresh counter has wrapped again
	void watchdog_tick(offs_t address);
	void update_irq();
	void write_4025(uint8_t data);
	static void qd_side_to_fds(const uint8_t *qd, uint8_t *fds);
	// Brad Taylor's FDS technical reference documents a disk/RAM-adaptor transfer rate of 96.4 kHz +/-10%
	// (https://archive.nesdev.org/fdstech.txt); at NTSC's ~1.7898 MHz CPU clock that is ~18.57 CPU cycles per
	// bit, i.e. ~149 CPU cycles per 8-bit byte - matching the ~12 KB/s transfer rate independently confirmed on
	// the NESdev forum ("Using FDS Disk IRQ (not Timer IRQ) for frame timing") and already used by puNES as
	// drive_delay. The transfer IRQ is timed from this rate, not from the PPU hblank callback.
	TIMER_CALLBACK_MEMBER(transfer_tick);
	static constexpr unsigned TRANSFER_CYCLES = 149;

	// the disk in the drive (saved with the machine state, so a state keeps what games wrote to the disk), and the disk
	// as it was when it was put in the drive, to tell whether it was written to; both hold up to MAX_SIDES sides
	static constexpr unsigned SIDE_SIZE = 65500;
	static constexpr unsigned QD_SIDE_SIZE = 65536;   // a side of a .qd image, which has the CRCs the drive does not take
	static constexpr unsigned MAX_SIDES = 16;     // the limit nes_dsk.cpp applies when it identifies a disk image
	std::unique_ptr<uint8_t[]> m_fds_data;
	std::unique_ptr<uint8_t[]> m_fds_loaded;
	bool m_write_started;                     // writing: the gap is over, the start mark was written, block data follows
	bool m_write_pending_valid;               // writing: m_write_pending was written to $4024 but has not been clocked onto the disk yet
	uint8_t m_write_pending;
	required_device<legacy_floppy_image_device> m_disk;
	required_device<rp2c33_sound_device> m_sound;

	emu_timer *transfer_timer;

	void load_disk(device_image_interface &image);
	void unload_disk(device_image_interface &image);

	uint16_t m_irq_count, m_irq_count_latch;
	bool m_irq_enable, m_irq_repeat, m_irq_transfer;
	bool m_disk_reg_enable;
	bool m_sound_en;
	uint16_t m_wd_timer;                      // 12-bit CPU cycle timer, 4095 while the disk registers are disabled
	uint8_t m_wd_refresh;                     // 7-bit refresh counter (DRAM row), 0 while the disk registers are disabled
	bool m_wd_irq_enable;                     // $4023.D7

	uint8_t m_fds_scan;             // $4025.D0: the drive scans (0: the media pointer is held at the start of the disk)
	uint8_t m_fds_door_closed;
	uint8_t m_fds_current_side;
	uint32_t m_fds_head_position;
	uint32_t m_scan_bytes;          // bytes the head has passed since the scan started (time, not reads)
	uint8_t m_fds_status0;
	uint8_t m_read_mode;
	uint8_t m_drive_ready;
	uint8_t m_4026 = 0x7f;                    // $4026 as last written (external connector outputs)
	uint8_t m_read_data = 0;                  // $4031: the last byte transferred from the disk
	uint8_t m_4025;                           // $4025 as last written: the byte transfer flag depends on D1, D2, D5 and D6

	uint8_t m_fds_sides;
	int32_t m_fds_last_side;
	int32_t m_fds_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_DISKSYS, nes_disksys_device)

#endif // MAME_BUS_NES_DISKSYS_H
