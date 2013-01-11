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
	pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

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

#if 0
/* interface has been seperated, so that it can be used in the super i/o chip */

#define PC_FDC_STATUS_REGISTER_A 0
#define PC_FDC_STATUS_REGISTER_B 1
#define PC_FDC_DIGITAL_OUTPUT_REGISTER 2
#define PC_FDC_TAPE_DRIVE_REGISTER 3
#define PC_FDC_MAIN_STATUS_REGISTER 4
#define PC_FDC_DATA_RATE_REGISTER 4
#define PC_FDC_DATA_REGISTER 5
#define PC_FDC_RESERVED_REGISTER 6
#define PC_FDC_DIGITIAL_INPUT_REGISTER 7
#define PC_FDC_CONFIGURATION_CONTROL_REGISTER 8

/* main interface */
struct pc_fdc_interface
{
	void (*pc_fdc_interrupt)(running_machine&,int);
	void (*pc_fdc_dma_drq)(running_machine&,int);
	device_t *(*get_image)(running_machine&, int floppy_index);
	device_t *(*get_device)(running_machine&);
};

extern const upd765_interface pc_fdc_upd765_connected_interface;
extern const upd765_interface pc_fdc_upd765_not_connected_interface;
extern const upd765_interface pc_fdc_upd765_connected_1_drive_interface;
extern const upd765_interface pcjr_fdc_upd765_interface;

void pc_fdc_reset(running_machine &machine);
void pc_fdc_init(running_machine &machine, const struct pc_fdc_interface *iface);
void pc_fdc_set_tc_state(running_machine &machine, int state);
int pc_fdc_dack_r(running_machine &machine, address_space &space);
void pc_fdc_dack_w(running_machine &machine, address_space &space, int data);

DECLARE_READ8_HANDLER(pc_fdc_r);
DECLARE_WRITE8_HANDLER(pc_fdc_w);
DECLARE_WRITE8_HANDLER ( pcjr_fdc_w );

#endif
#endif /* PC_FDC_H */
