#ifndef __TAITOSND_H__
#define __TAITOSND_H__


/* MASTER (8bit bus) control functions */
WRITE8_HANDLER( taitosound_port_w );
WRITE8_HANDLER( taitosound_comm_w );
READ8_HANDLER( taitosound_comm_r );


/* SLAVE (8bit bus) control functions ONLY */
WRITE8_HANDLER( taitosound_slave_port_w );
READ8_HANDLER( taitosound_slave_comm_r );
WRITE8_HANDLER( taitosound_slave_comm_w );


#endif /*__TAITOSND_H__*/
