#ifndef __RP5H01_H__
#define __RP5H01_H__

/* max simultaneous chips supported. change if you need more */
#define MAX_RP5H01	1

struct RP5H01_interface {
	int num;					/* number of chips */
	int region[MAX_RP5H01];		/* memory region where data resides */
	int offset[MAX_RP5H01];		/* memory offset within the above region where data resides */
};

int RP5H01_init( struct RP5H01_interface *interface );
void RP5H01_enable_w( int which, int data );				/* /CE */
void RP5H01_reset_w( int which, int data );				/* RESET */
void RP5H01_clock_w( int which, int data );				/* DATA CLOCK (active low) */
void RP5H01_test_w( int which, int data );				/* TEST */
int RP5H01_counter_r( int which );						/* COUNTER OUT */
int RP5H01_data_r( int which );							/* DATA */

/* direct-access stubs */
WRITE8_HANDLER( RP5H01_0_enable_w );
WRITE8_HANDLER( RP5H01_0_reset_w );
WRITE8_HANDLER( RP5H01_0_clock_w );
WRITE8_HANDLER( RP5H01_0_test_w );
READ8_HANDLER( RP5H01_0_counter_r );
READ8_HANDLER( RP5H01_0_data_r );

#endif /* __RP5H01_H__ */
