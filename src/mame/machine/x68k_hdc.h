// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    X68000 Custom SASI HD controller

*/
#ifndef MAME_MACHINE_X68K_HDC_H
#define MAME_MACHINE_X68K_HDC_H

#pragma once


class x68k_hdc_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	x68k_hdc_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_HARDDISK; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "hdf"; }
	virtual const char *custom_instance_name() const override { return "sasihd"; }
	virtual const char *custom_brief_instance_name() const override { return "sasi"; }
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	DECLARE_WRITE16_MEMBER( hdc_w );
	DECLARE_READ16_MEMBER( hdc_r );
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	int m_phase;
	unsigned char m_status_port;  // read at 0xe96003
	unsigned char m_status;       // status phase output
	//unsigned char m_message;
	unsigned char m_command[10];
	unsigned char m_sense[4];
	int m_command_byte_count;
	int m_command_byte_total;
	int m_current_command;
	int m_transfer_byte_count;
	int m_transfer_byte_total;
	int m_msg;  // MSG
	int m_cd;   // C/D (Command/Data)
	int m_bsy;  // BSY
	int m_io;   // I/O
	int m_req;  // REQ
};

// device type definition
DECLARE_DEVICE_TYPE(X68KHDC, x68k_hdc_image_device)

#define MCFG_X68KHDC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X68KHDC, 0)

#endif // MAME_MACHINE_X68K_HDC_H
