/*----------- defined in drivers/eolithsp.c -----------*/

void eolith_speedup_read(running_machine *machine);
void init_eolith_speedup(running_machine *machine);
INTERRUPT_GEN( eolith_speedup );
CUSTOM_INPUT( eolith_speedup_getvblank );
