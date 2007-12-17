/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#ifndef __IREMGA20_H__
#define __IREMGA20_H__

struct IremGA20_interface {
	int region;					/* memory region of sample ROM(s) */
};

WRITE16_HANDLER( IremGA20_w );
READ16_HANDLER( IremGA20_r );

#endif /* __IREMGA20_H__ */
