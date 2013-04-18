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

class pc_fdc_family_device : public pc_fdc_interface {
public:
	pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	required_device<upd765a_device> fdc;

	virtual void setup_intrq_cb(line_cb cb);
	virtual void setup_drq_cb(line_cb cb);

	virtual DECLARE_ADDRESS_MAP(map, 8);

	virtual void tc_w(bool state);
	virtual UINT8 dma_r();
	virtual void dma_w(UINT8 data);
	virtual UINT8 do_dir_r();

	READ8_MEMBER(dor_r);
	WRITE8_MEMBER(dor_w);
	READ8_MEMBER(dir_r);
	WRITE8_MEMBER(ccr_w);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	line_cb intrq_cb, drq_cb;
	bool irq, drq, fdc_drq, fdc_irq;
	UINT8 dor;

	floppy_image_device *floppy[4];

	void irq_w(bool state);
	void drq_w(bool state);
	void check_irq();
	void check_drq();
};

class pc_fdc_xt_device : public pc_fdc_family_device {
public:
	pc_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
	WRITE8_MEMBER(dor_fifo_w);
};

class pc_fdc_at_device : public pc_fdc_family_device {
public:
	pc_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

extern const device_type PC_FDC_XT;
extern const device_type PC_FDC_AT;

#endif /* PC_FDC_H */
