/*
    snescart.h

*/

#ifndef _SNESCART_H
#define _SNESCART_H

void snes_machine_stop(running_machine &machine);
void sufami_machine_stop(running_machine &machine);

MACHINE_CONFIG_EXTERN( snes_cartslot );
MACHINE_CONFIG_EXTERN( snesp_cartslot );
MACHINE_CONFIG_EXTERN( sufami_cartslot );


// add-on chips IO
void srtc_write(running_machine &machine, UINT16 addr, UINT8 data);
UINT8 srtc_read(address_space &space, UINT16 addr);
void srtc_init(running_machine &machine);
extern DECLARE_READ8_HANDLER(obc1_read);
extern DECLARE_WRITE8_HANDLER(obc1_write);
void obc1_init(running_machine &machine);
UINT8 CX4_read(UINT32 addr);
void CX4_write(running_machine &machine, UINT32 addr, UINT8 data);
UINT8 sdd1_mmio_read(address_space &space, UINT32 addr);
void sdd1_mmio_write(address_space &space, UINT32 addr, UINT8 data);
void sdd1_init(running_machine& machine);
UINT8 sdd1_read(running_machine& machine, UINT32 addr);
UINT8 spc7110_mmio_read(address_space &space, UINT32 addr);
void spc7110_mmio_write(running_machine &machine, UINT32 addr, UINT8 data);
UINT8 spc7110_bank7_read(address_space &space, UINT32 offset);
void spc7110_init(running_machine& machine);
void spc7110rtc_init(running_machine& machine);
UINT8 spc7110_ram_read(UINT32 offset);
void spc7110_ram_write(UINT32 offset, UINT8 data);


#endif /* _SNESCART_H */
