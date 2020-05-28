// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    psion_pack.h

****************************************************************************/

#ifndef MAME_MACHINE_PSION_PACK_H
#define MAME_MACHINE_PSION_PACK_H

#include "softlist_dev.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> datapack_device

class datapack_device : public device_t,
						public device_image_interface
{
public:
	// construction/destruction
	datapack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~datapack_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *create_args) override;

	virtual iodevice_t image_type() const noexcept override { return IO_CARTSLOT; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "psion_pack"; }
	virtual const char *file_extensions() const noexcept override { return "opk"; }
	virtual const util::option_guide &create_option_guide() const override;

	// specific implementation
	uint8_t data_r();
	void  data_w(uint8_t data);
	uint8_t control_r();
	void control_w(uint8_t data);

protected:
	// internal helper
	void update();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }

private:
	// internal device state
	uint8_t  m_id;                //datapack ID
	uint8_t  m_size;              //size in 8k blocks
	uint8_t  m_data;              //data lines
	uint8_t  m_control;           //control lines
	uint16_t m_counter;           //address counter
	uint8_t  m_page;              //active page (only for paged Datapack)
	uint8_t  m_segment;           //active segment (only for segmented Datapack)
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_DATAPACK, datapack_device)

#endif // MAME_MACHINE_PSION_PACK_H
