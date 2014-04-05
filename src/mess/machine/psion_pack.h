// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    psion_pack.h

****************************************************************************/

#ifndef __PSION_PACK_H__
#define __PSION_PACK_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> datapack_device

class datapack_device : public device_t,
						public device_image_interface
{
public:
	// construction/destruction
	datapack_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~datapack_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_create(int format_type, option_resolution *create_args);
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return "psion_pack"; }
	virtual const char *file_extensions() const { return "opk"; }
	virtual const option_guide *create_option_guide() const;

	// specific implementation
	UINT8 data_r();
	void  data_w(UINT8 data);
	UINT8 control_r();
	void control_w(UINT8 data);

protected:
	// internal helper
	void update();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

private:
	// internal device state
	UINT8  m_id;                //datapack ID
	UINT8  m_size;              //size in 8k blocks
	UINT8  m_data;              //data lines
	UINT8  m_control;           //control lines
	UINT16 m_counter;           //address counter
	UINT8  m_page;              //active page (only for paged Datapack)
	UINT8  m_segment;           //active segment (only for segmented Datapack)
};


// device type definition
extern const device_type PSION_DATAPACK;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PSION_DATAPACK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PSION_DATAPACK, 0)
#endif /* __PSION_PACK_H__ */
