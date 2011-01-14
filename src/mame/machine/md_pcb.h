#ifndef __GENESIS_PCB_H
# define __GENESIS_PCB_H

/* PCB */
enum
  {
    STD_ROM = 0,

    /* Sega PCB */
    SEGA_5878, SEGA_6584A, SEGA_5921, SEGA_6278A, SEGA_6658A,

    /* Codemasters PCB (J-Carts and SEPROM) */
    CM_JCART, CM_JCART_SEPROM
  };

int md_get_pcb_id(const char *pcb);

#endif

