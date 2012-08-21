/*****************************************************************************
 *
 * includes/amstr_pc.h
 *
 ****************************************************************************/

#ifndef AMSTR_PC_H_
#define AMSTR_PC_H_


#define PC200_MODE (machine.root_device().ioport("DSW0")->read() & 0x30)
#define PC200_MDA 0x30


/*----------- defined in machine/amstr_pc.c -----------*/

READ8_HANDLER( pc1640_port60_r );
WRITE8_HANDLER( pc1640_port60_w );

READ8_HANDLER( pc1640_mouse_x_r );
READ8_HANDLER( pc1640_mouse_y_r );
WRITE8_HANDLER( pc1640_mouse_x_w );
WRITE8_HANDLER( pc1640_mouse_y_w );

READ8_HANDLER( pc200_port378_r );
READ8_HANDLER( pc200_port278_r );
READ8_HANDLER( pc1640_port378_r );
READ8_HANDLER( pc1640_port3d0_r );
READ8_HANDLER( pc1640_port4278_r );
READ8_HANDLER( pc1640_port278_r );

INPUT_PORTS_EXTERN( amstrad_keyboard );


#endif /* AMSTR_PC_H_ */
