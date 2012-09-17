/**********************************************************************

    PC-style floppy disk controller emulation

**********************************************************************/

#ifndef PC_FDC_H
#define PC_FDC_H

#include "emu.h"
#include "machine/upd765.h"

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
int	pc_fdc_dack_r(running_machine &machine, address_space &space);
void pc_fdc_dack_w(running_machine &machine, address_space &space, int data);

DECLARE_READ8_HANDLER(pc_fdc_r);
DECLARE_WRITE8_HANDLER(pc_fdc_w);
DECLARE_WRITE8_HANDLER ( pcjr_fdc_w );

#endif /* PC_FDC_H */


