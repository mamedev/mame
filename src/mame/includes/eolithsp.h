/*----------- defined in drivers/eolithsp.c -----------*/

void eolith_speedup_read(address_space *space);
void init_eolith_speedup(running_machine &machine);
TIMER_DEVICE_CALLBACK( eolith_speedup );
CUSTOM_INPUT( eolith_speedup_getvblank );


