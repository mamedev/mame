/*----------- defined in drivers/eolithsp.c -----------*/

void eolith_speedup_read(void);
void init_eolith_speedup(running_machine *machine);
INTERRUPT_GEN( eolith_speedup );
UINT32 eolith_speedup_getvblank(void *param);
