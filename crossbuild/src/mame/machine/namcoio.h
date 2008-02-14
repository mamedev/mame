#ifndef NAMCOIO_H
#define NAMCOIO_H

enum
{
	NAMCOIO_NONE = 0,
	NAMCOIO_50XX,
	NAMCOIO_50XX_2,
	NAMCOIO_51XX,
	NAMCOIO_52XX,
	NAMCOIO_53XX_DIGDUG,
	NAMCOIO_53XX_POLEPOS,
	NAMCOIO_54XX,
	NAMCOIO_56XX,
	NAMCOIO_58XX,
	NAMCOIO_59XX,
	NAMCOIO_62XX
};

#define MAX_NAMCOIO 8
#define MAX_06XX 2


struct namcoio_interface
{
	read8_handler in[4];	/* read handlers for ports A-D */
	write8_handler out[2];	/* write handlers for ports A-B */
};


void namco_06xx_init(int chipnum, int cpu,
	int type0, const struct namcoio_interface *intf0,
	int type1, const struct namcoio_interface *intf1,
	int type2, const struct namcoio_interface *intf2,
	int type3, const struct namcoio_interface *intf3);
READ8_HANDLER( namco_06xx_0_data_r );
READ8_HANDLER( namco_06xx_1_data_r );
WRITE8_HANDLER( namco_06xx_0_data_w );
WRITE8_HANDLER( namco_06xx_1_data_w );
READ8_HANDLER( namco_06xx_0_ctrl_r );
READ8_HANDLER( namco_06xx_1_ctrl_r );
WRITE8_HANDLER( namco_06xx_0_ctrl_w );
WRITE8_HANDLER( namco_06xx_1_ctrl_w );


READ8_HANDLER( namcoio_r );
WRITE8_HANDLER( namcoio_w );
void namcoio_init(int chipnum, int type, const struct namcoio_interface *intf);
void namcoio_set_reset_line(int chipnum, int state);
void namcoio_set_irq_line(int chipnum, int state);


#endif
