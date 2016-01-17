// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    PC-style floppy disk controller emulation

**********************************************************************/

#ifndef PC_FDC_H
#define PC_FDC_H

#include "emu.h"
#include "machine/upd765.h"

#define MCFG_PC_FDC_XT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PC_FDC_XT, 0)

#define MCFG_PC_FDC_AT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PC_FDC_AT, 0)

#define MCFG_PC_FDC_INTRQ_CALLBACK(_write) \
	devcb = &pc_fdc_family_device::set_intrq_wr_callback(*device, DEVCB_##_write);

#define MCFG_PC_FDC_DRQ_CALLBACK(_write) \
	devcb = &pc_fdc_family_device::set_drq_wr_callback(*device, DEVCB_##_write);

class pc_fdc_family_device : public pc_fdc_interface {
public:
	pc_fdc_family_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<pc_fdc_family_device &>(device).intrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<pc_fdc_family_device &>(device).drq_cb.set_callback(object); }

	required_device<upd765a_device> fdc;

	virtual DECLARE_ADDRESS_MAP(map, 8) override;

	virtual void tc_w(bool state) override;
	virtual UINT8 dma_r() override;
	virtual void dma_w(UINT8 data) override;
	virtual UINT8 do_dir_r() override;

	READ8_MEMBER(dor_r);
	WRITE8_MEMBER(dor_w);
	READ8_MEMBER(dir_r);
	WRITE8_MEMBER(ccr_w);
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	bool irq, drq, fdc_drq, fdc_irq;
	devcb_write_line intrq_cb, drq_cb;
	UINT8 dor;

	floppy_image_device *floppy[4];

	void check_irq();
	void check_drq();
};

class pc_fdc_xt_device : public pc_fdc_family_device {
public:
	pc_fdc_xt_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8) override;
	WRITE8_MEMBER(dor_fifo_w);
};

class pc_fdc_at_device : public pc_fdc_family_device {
public:
	pc_fdc_at_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8) override;
};

extern const device_type PC_FDC_XT;
extern const device_type PC_FDC_AT;

#endif /* PC_FDC_H */
