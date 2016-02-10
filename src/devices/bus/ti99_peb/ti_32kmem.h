// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 32K memory expansion
    See ti32kmem.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI32K__
#define __TI32K__

extern const device_type TI_32KMEM;

class ti_32k_expcard_device : public ti_expansion_card_device
{
public:
	ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { };
	DECLARE_WRITE8_MEMBER(cruwrite) override { };

protected:
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
private:
	UINT8*  m_ram_ptr;
};

#endif
