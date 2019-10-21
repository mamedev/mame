// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*********************************************************************

    fmt_icmem.h

    FM Towns IC Memory Card

*********************************************************************/

#ifndef MAME_MACHINE_FMT_ICMEM_H
#define MAME_MACHINE_FMT_ICMEM_H

#pragma once


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class fmt_icmem_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	fmt_icmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual iodevice_t image_type() const override { return IO_MEMCARD; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "icm"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	// device-level overrides
	virtual void device_start() override;

	DECLARE_READ8_MEMBER(static_mem_read);
	DECLARE_WRITE8_MEMBER(static_mem_write);
	DECLARE_READ8_MEMBER(mem_read);
	DECLARE_WRITE8_MEMBER(mem_write);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(bank_r);
	DECLARE_WRITE8_MEMBER(bank_w);


protected:
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_writeprotect;
	std::unique_ptr<uint8_t[]> m_memcard_ram;
	bool m_change;
	bool m_attr_select;
	uint8_t m_detect;
	uint8_t m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(FMT_ICMEM, fmt_icmem_device)


#endif  // MAME_MACHINE_FMT_ICMEM_H
