#ifndef __AMIGACD_H__
#define __AMIGACD_H__


MACHINE_START( amigacd );
MACHINE_RESET( amigacd );

/* 6525tpi */
WRITE_LINE_DEVICE_HANDLER( amigacd_tpi6525_irq );


#endif /* __AMIGACD_H__ */
