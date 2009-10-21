/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

/*----------- defined in machine/tecmosys.c -----------*/

void tecmosys_prot_init(running_machine *machine, int which);

READ16_HANDLER(tecmosys_prot_status_r);
WRITE16_HANDLER(tecmosys_prot_status_w);
READ16_HANDLER(tecmosys_prot_data_r);
WRITE16_HANDLER(tecmosys_prot_data_w);




