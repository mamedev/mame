// license:BSD-3-Clause
// copyright-holders:Eric Anderson
#ifndef MAME_VECTORGRAPHIC_DUALMODEDISK_H
#define MAME_VECTORGRAPHIC_DUALMODEDISK_H

#pragma once

class vector_micropolis_image_device :
		public device_t,
		public device_image_interface
{
public:
	vector_micropolis_image_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	bool is_readable()  const noexcept override { return true; }
	bool is_writeable() const noexcept override { return true; }
	bool is_creatable() const noexcept override { return true; }
	const char *image_type_name() const noexcept override { return "micropolisimage"; }
	const char *image_brief_type_name() const noexcept override { return "flop"; }
	const char *image_interface() const noexcept override { return "micropolis_image"; }
	const char *file_extensions() const noexcept override { return "vgi"; }
	image_init_result call_load() override;
	bool is_reset_on_load() const noexcept override { return false; }

protected:
	// device_t implementation
	void device_start() override;
};

class vector_dualmode_device : public device_t
{
public:
	vector_dualmode_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	void device_start() override;
	void device_reset() override;
	void device_add_mconfig(machine_config &config) override;

private:
	required_device_array<vector_micropolis_image_device, 4> m_floppy;
	uint8_t m_ram[512];
	uint16_t m_cmar;
	uint8_t m_drive;
	uint8_t m_head;
	uint8_t m_track;
	uint8_t m_sector;
	bool m_read;
	emu_timer *m_motor_on_timer;
};

DECLARE_DEVICE_TYPE(MICROPOLIS_IMAGE, vector_micropolis_image_device)
DECLARE_DEVICE_TYPE(VECTOR_DUALMODE, vector_dualmode_device)

#endif // MAME_VECTORGRAPHIC_DUALMODEDISK_H