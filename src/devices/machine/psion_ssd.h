// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/****************************************************************************

    Psion Solid State Disk emulation

****************************************************************************/

#ifndef MAME_MACHINE_PSION_SSD_H
#define MAME_MACHINE_PSION_SSD_H

#include "machine/psion_asic5.h"
#include "imagedev/memcard.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> psion_ssd_device

class psion_ssd_device : public device_t, public device_memcard_image_interface
{
public:
	// construction/destruction
	psion_ssd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~psion_ssd_device();

	// callbacks
	auto door_cb() { return m_door_cb.bind(); }

	uint8_t data_r();
	void data_w(uint16_t data);

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *create_args) override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "psion_ssd"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }
	virtual const char *image_type_name() const noexcept override { return "ssd"; }
	virtual const char *image_brief_type_name() const noexcept override { return "ssd"; }

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

private:
	optional_memory_region  m_region;
	required_device<psion_asic5_device> m_asic5;

	TIMER_CALLBACK_MEMBER(close_door);

	devcb_write_line m_door_cb;
	emu_timer *m_door_timer;

	std::unique_ptr<uint8_t[]> m_ssd_data;

	void set_info_byte(uint32_t size, uint8_t type = 0);
	uint32_t latched_addr();

	uint8_t m_info_byte;
	uint32_t m_port_latch;
	int m_mem_width;

	static constexpr uint8_t SSD_RAM    = 0;
	static constexpr uint8_t SSD_FLASH1 = 1;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_SSD, psion_ssd_device)

#endif // MAME_MACHINE_PSION_SSD_H
