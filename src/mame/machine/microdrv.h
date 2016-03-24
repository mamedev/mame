// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    microdrv.h

    MESS interface to the Sinclair Microdrive image abstraction code

*********************************************************************/

#pragma once

#ifndef __MICRODRV__
#define __MICRODRV__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDV_1 "mdv1"
#define MDV_2 "mdv2"


#define MCFG_MICRODRIVE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MICRODRIVE, 0)

#define MCFG_MICRODRIVE_COMMS_OUT_CALLBACK(_write) \
	devcb = &microdrive_image_device::set_comms_out_wr_callback(*device, DEVCB_##_write);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> microdrive_image_device

class microdrive_image_device : public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~microdrive_image_device();

	template<class _Object> static devcb_base &set_comms_out_wr_callback(device_t &device, _Object object) { return downcast<microdrive_image_device &>(device).m_write_comms_out.set_callback(object); }

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const override { return IO_CASSETTE; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *image_interface() const override { return "ql_cass"; }
	virtual const char *file_extensions() const override { return "mdv"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// specific implementation
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( comms_in_w );
	DECLARE_WRITE_LINE_MEMBER( erase_w );
	DECLARE_WRITE_LINE_MEMBER( read_write_w );
	DECLARE_WRITE_LINE_MEMBER( data1_w );
	DECLARE_WRITE_LINE_MEMBER( data2_w );
	DECLARE_READ_LINE_MEMBER ( data1_r );
	DECLARE_READ_LINE_MEMBER ( data2_r );
protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	devcb_write_line m_write_comms_out;

	int m_clk;
	int m_comms_in;
	int m_comms_out;
	int m_erase;
	int m_read_write;

	std::unique_ptr<UINT8[]> m_left;
	std::unique_ptr<UINT8[]> m_right;

	int m_bit_offset;
	int m_byte_offset;

	emu_timer *m_bit_timer;
};


// device type definition
extern const device_type MICRODRIVE;



#endif
