// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    psion_pack.h

****************************************************************************/

#ifndef MAME_PSION_PSION_PACK_H
#define MAME_PSION_PSION_PACK_H

#include "imagedev/memcard.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> datapack_device

class datapack_device : public device_t,
						public device_memcard_image_interface
{
public:
	// construction/destruction
	datapack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~datapack_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *create_args) override;

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "psion_pack"; }
	virtual const char *file_extensions() const noexcept override { return "opk"; }
	virtual const char *image_type_name() const noexcept override { return "datapack"; }
	virtual const char *image_brief_type_name() const noexcept override { return "dpak"; }
	virtual const util::option_guide &create_option_guide() const override;

	// specific implementation
	uint8_t data_r();
	void  data_w(uint8_t data);
	uint8_t control_r();
	void control_w(uint8_t data);

protected:
	// internal helper
	void update();

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

private:
	// internal device state
	uint8_t  m_id = 0;                //datapack ID
	uint8_t  m_size = 0;              //size in 8k blocks
	uint8_t  m_data = 0;              //data lines
	uint8_t  m_control = 0;           //control lines
	uint16_t m_counter = 0;           //address counter
	uint8_t  m_page = 0;              //active page (only for paged Datapack)
	uint8_t  m_segment = 0;           //active segment (only for segmented Datapack)
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_DATAPACK, datapack_device)

#endif // MAME_PSION_PSION_PACK_H
