/**********************************************************************

    Motorola 6821 PIA interface and emulation

    Notes:
        * pia_get_port_b_z_mask() gives the caller the bitmask
          that show which bits are high-impendance when
          reading port B, thus neither 0 or 1.
          pia_get_output_cb2_z() returns the same
          information for the CB2 pin
        * pia_set_port_a_z_mask allows the input callback to
          indicate which port A bits are disconnected.
          For these bit, the read operation will return the
          output buffer's contents
        * the 'alt' interface functions are used when the A0 and A1
          address bits are swapped
        * all 'int' data or return values are Boolean

**********************************************************************/

#ifndef PIA_6821
#define PIA_6821


#define MAX_PIA 8


/*------------- PIA interface structure -----------------*/

typedef struct _pia6821_interface pia6821_interface;
struct _pia6821_interface
{
	read8_handler in_a_func;
	read8_handler in_b_func;
	read8_handler in_ca1_func;
	read8_handler in_cb1_func;
	read8_handler in_ca2_func;
	read8_handler in_cb2_func;
	write8_handler out_a_func;
	write8_handler out_b_func;
	write8_handler out_ca2_func;
	write8_handler out_cb2_func;
	void (*irq_a_func)(int state);
	void (*irq_b_func)(int state);
};



/*------------------ Configuration -----------------------*/

void pia_config(int which, const pia6821_interface *intf);



/*----------------------- Reset --------------------------*/

void pia_reset(void);



/*-------------- CPU interface for PIA read --------------*/

UINT8 pia_read(int which, offs_t offset);
UINT8 pia_alt_read(int which, offs_t offset);
UINT8 pia_get_port_b_z_mask(int which);  /* see first note */



/*------------- CPU interface for PIA write --------------*/

void pia_write(int which, offs_t offset, UINT8 data);
void pia_alt_write(int which, offs_t offset, UINT8 data);
void pia_set_port_a_z_mask(int which, UINT8 data);  /* see second note */



/*------------- Device interface for port A --------------*/

void pia_set_input_a(int which, UINT8 data, UINT8 z_mask);
UINT8 pia_get_output_a(int which);



/*----------- Device interface for the CA1 pin -----------*/

void pia_set_input_ca1(int which, int data);



/*----------- Device interface for the CA2 pin -----------*/

void pia_set_input_ca2(int which, int data);
int pia_get_output_ca2(int which);



/*------------- Device interface for port A --------------*/

void pia_set_input_b(int which, UINT8 data);
UINT8 pia_get_output_b(int which);



/*----------- Device interface for the CB1 pin -----------*/

void pia_set_input_cb1(int which, int data);



/*----------- Device interface for the CB2 pin -----------*/

void pia_set_input_cb2(int which, int data);
int pia_get_output_cb2(int which);
int pia_get_output_cb2_z(int which);  /* see first note */



/*-- Convinience interface for retrieving the IRQ state --*/

int pia_get_irq_a(int which);
int pia_get_irq_b(int which);



/*---------- Standard 8-bit CPU interfaces, D0-D7 --------*/

READ8_HANDLER( pia_0_r );
READ8_HANDLER( pia_1_r );
READ8_HANDLER( pia_2_r );
READ8_HANDLER( pia_3_r );
READ8_HANDLER( pia_4_r );
READ8_HANDLER( pia_5_r );
READ8_HANDLER( pia_6_r );
READ8_HANDLER( pia_7_r );

WRITE8_HANDLER( pia_0_w );
WRITE8_HANDLER( pia_1_w );
WRITE8_HANDLER( pia_2_w );
WRITE8_HANDLER( pia_3_w );
WRITE8_HANDLER( pia_4_w );
WRITE8_HANDLER( pia_5_w );
WRITE8_HANDLER( pia_6_w );
WRITE8_HANDLER( pia_7_w );

READ8_HANDLER( pia_0_alt_r );
READ8_HANDLER( pia_1_alt_r );
READ8_HANDLER( pia_2_alt_r );
READ8_HANDLER( pia_3_alt_r );
READ8_HANDLER( pia_4_alt_r );
READ8_HANDLER( pia_5_alt_r );
READ8_HANDLER( pia_6_alt_r );
READ8_HANDLER( pia_7_alt_r );

WRITE8_HANDLER( pia_0_alt_w );
WRITE8_HANDLER( pia_1_alt_w );
WRITE8_HANDLER( pia_2_alt_w );
WRITE8_HANDLER( pia_3_alt_w );
WRITE8_HANDLER( pia_4_alt_w );
WRITE8_HANDLER( pia_5_alt_w );
WRITE8_HANDLER( pia_6_alt_w );
WRITE8_HANDLER( pia_7_alt_w );



/*---------- Standard 16-bit CPU interfaces, D0-D7 -------*/

READ16_HANDLER( pia_0_lsb_r );
READ16_HANDLER( pia_1_lsb_r );
READ16_HANDLER( pia_2_lsb_r );
READ16_HANDLER( pia_3_lsb_r );
READ16_HANDLER( pia_4_lsb_r );
READ16_HANDLER( pia_5_lsb_r );
READ16_HANDLER( pia_6_lsb_r );
READ16_HANDLER( pia_7_lsb_r );

WRITE16_HANDLER( pia_0_lsb_w );
WRITE16_HANDLER( pia_1_lsb_w );
WRITE16_HANDLER( pia_2_lsb_w );
WRITE16_HANDLER( pia_3_lsb_w );
WRITE16_HANDLER( pia_4_lsb_w );
WRITE16_HANDLER( pia_5_lsb_w );
WRITE16_HANDLER( pia_6_lsb_w );
WRITE16_HANDLER( pia_7_lsb_w );

READ16_HANDLER( pia_0_lsb_alt_r );
READ16_HANDLER( pia_1_lsb_alt_r );
READ16_HANDLER( pia_2_lsb_alt_r );
READ16_HANDLER( pia_3_lsb_alt_r );
READ16_HANDLER( pia_4_lsb_alt_r );
READ16_HANDLER( pia_5_lsb_alt_r );
READ16_HANDLER( pia_6_lsb_alt_r );
READ16_HANDLER( pia_7_lsb_alt_r );

WRITE16_HANDLER( pia_0_lsb_alt_w );
WRITE16_HANDLER( pia_1_lsb_alt_w );
WRITE16_HANDLER( pia_2_lsb_alt_w );
WRITE16_HANDLER( pia_3_lsb_alt_w );
WRITE16_HANDLER( pia_4_lsb_alt_w );
WRITE16_HANDLER( pia_5_lsb_alt_w );
WRITE16_HANDLER( pia_6_lsb_alt_w );
WRITE16_HANDLER( pia_7_lsb_alt_w );



/*--------- Standard 16-bit CPU interfaces, D8-D15 -------*/

READ16_HANDLER( pia_0_msb_r );
READ16_HANDLER( pia_1_msb_r );
READ16_HANDLER( pia_2_msb_r );
READ16_HANDLER( pia_3_msb_r );
READ16_HANDLER( pia_4_msb_r );
READ16_HANDLER( pia_5_msb_r );
READ16_HANDLER( pia_6_msb_r );
READ16_HANDLER( pia_7_msb_r );

WRITE16_HANDLER( pia_0_msb_w );
WRITE16_HANDLER( pia_1_msb_w );
WRITE16_HANDLER( pia_2_msb_w );
WRITE16_HANDLER( pia_3_msb_w );
WRITE16_HANDLER( pia_4_msb_w );
WRITE16_HANDLER( pia_5_msb_w );
WRITE16_HANDLER( pia_6_msb_w );
WRITE16_HANDLER( pia_7_msb_w );

READ16_HANDLER( pia_0_msb_alt_r );
READ16_HANDLER( pia_1_msb_alt_r );
READ16_HANDLER( pia_2_msb_alt_r );
READ16_HANDLER( pia_3_msb_alt_r );
READ16_HANDLER( pia_4_msb_alt_r );
READ16_HANDLER( pia_5_msb_alt_r );
READ16_HANDLER( pia_6_msb_alt_r );
READ16_HANDLER( pia_7_msb_alt_r );

WRITE16_HANDLER( pia_0_msb_alt_w );
WRITE16_HANDLER( pia_1_msb_alt_w );
WRITE16_HANDLER( pia_2_msb_alt_w );
WRITE16_HANDLER( pia_3_msb_alt_w );
WRITE16_HANDLER( pia_4_msb_alt_w );
WRITE16_HANDLER( pia_5_msb_alt_w );
WRITE16_HANDLER( pia_6_msb_alt_w );
WRITE16_HANDLER( pia_7_msb_alt_w );



/*--------------- 8-bit A/B port interfaces -------------*/

WRITE8_HANDLER( pia_0_porta_w );
WRITE8_HANDLER( pia_1_porta_w );
WRITE8_HANDLER( pia_2_porta_w );
WRITE8_HANDLER( pia_3_porta_w );
WRITE8_HANDLER( pia_4_porta_w );
WRITE8_HANDLER( pia_5_porta_w );
WRITE8_HANDLER( pia_6_porta_w );
WRITE8_HANDLER( pia_7_porta_w );

WRITE8_HANDLER( pia_0_portb_w );
WRITE8_HANDLER( pia_1_portb_w );
WRITE8_HANDLER( pia_2_portb_w );
WRITE8_HANDLER( pia_3_portb_w );
WRITE8_HANDLER( pia_4_portb_w );
WRITE8_HANDLER( pia_5_portb_w );
WRITE8_HANDLER( pia_6_portb_w );
WRITE8_HANDLER( pia_7_portb_w );

READ8_HANDLER( pia_0_porta_r );
READ8_HANDLER( pia_1_porta_r );
READ8_HANDLER( pia_2_porta_r );
READ8_HANDLER( pia_3_porta_r );
READ8_HANDLER( pia_4_porta_r );
READ8_HANDLER( pia_5_porta_r );
READ8_HANDLER( pia_6_porta_r );
READ8_HANDLER( pia_7_porta_r );

READ8_HANDLER( pia_0_portb_r );
READ8_HANDLER( pia_1_portb_r );
READ8_HANDLER( pia_2_portb_r );
READ8_HANDLER( pia_3_portb_r );
READ8_HANDLER( pia_4_portb_r );
READ8_HANDLER( pia_5_portb_r );
READ8_HANDLER( pia_6_portb_r );
READ8_HANDLER( pia_7_portb_r );



/*--------- 1-bit CA1/CA2/CB1/CB2 port interfaces -------*/

WRITE8_HANDLER( pia_0_ca1_w );
WRITE8_HANDLER( pia_1_ca1_w );
WRITE8_HANDLER( pia_2_ca1_w );
WRITE8_HANDLER( pia_3_ca1_w );
WRITE8_HANDLER( pia_4_ca1_w );
WRITE8_HANDLER( pia_5_ca1_w );
WRITE8_HANDLER( pia_6_ca1_w );
WRITE8_HANDLER( pia_7_ca1_w );

WRITE8_HANDLER( pia_0_ca2_w );
WRITE8_HANDLER( pia_1_ca2_w );
WRITE8_HANDLER( pia_2_ca2_w );
WRITE8_HANDLER( pia_3_ca2_w );
WRITE8_HANDLER( pia_4_ca2_w );
WRITE8_HANDLER( pia_5_ca2_w );
WRITE8_HANDLER( pia_6_ca2_w );
WRITE8_HANDLER( pia_7_ca2_w );

WRITE8_HANDLER( pia_0_cb1_w );
WRITE8_HANDLER( pia_1_cb1_w );
WRITE8_HANDLER( pia_2_cb1_w );
WRITE8_HANDLER( pia_3_cb1_w );
WRITE8_HANDLER( pia_4_cb1_w );
WRITE8_HANDLER( pia_5_cb1_w );
WRITE8_HANDLER( pia_6_cb1_w );
WRITE8_HANDLER( pia_7_cb1_w );

WRITE8_HANDLER( pia_0_cb2_w );
WRITE8_HANDLER( pia_1_cb2_w );
WRITE8_HANDLER( pia_2_cb2_w );
WRITE8_HANDLER( pia_3_cb2_w );
WRITE8_HANDLER( pia_4_cb2_w );
WRITE8_HANDLER( pia_5_cb2_w );
WRITE8_HANDLER( pia_6_cb2_w );
WRITE8_HANDLER( pia_7_cb2_w );

READ8_HANDLER( pia_0_ca1_r );
READ8_HANDLER( pia_1_ca1_r );
READ8_HANDLER( pia_2_ca1_r );
READ8_HANDLER( pia_3_ca1_r );
READ8_HANDLER( pia_4_ca1_r );
READ8_HANDLER( pia_5_ca1_r );
READ8_HANDLER( pia_6_ca1_r );
READ8_HANDLER( pia_7_ca1_r );

READ8_HANDLER( pia_0_ca2_r );
READ8_HANDLER( pia_1_ca2_r );
READ8_HANDLER( pia_2_ca2_r );
READ8_HANDLER( pia_3_ca2_r );
READ8_HANDLER( pia_4_ca2_r );
READ8_HANDLER( pia_5_ca2_r );
READ8_HANDLER( pia_6_ca2_r );
READ8_HANDLER( pia_7_ca2_r );

READ8_HANDLER( pia_0_cb1_r );
READ8_HANDLER( pia_1_cb1_r );
READ8_HANDLER( pia_2_cb1_r );
READ8_HANDLER( pia_3_cb1_r );
READ8_HANDLER( pia_4_cb1_r );
READ8_HANDLER( pia_5_cb1_r );
READ8_HANDLER( pia_6_cb1_r );
READ8_HANDLER( pia_7_cb1_r );

READ8_HANDLER( pia_0_cb2_r );
READ8_HANDLER( pia_1_cb2_r );
READ8_HANDLER( pia_2_cb2_r );
READ8_HANDLER( pia_3_cb2_r );
READ8_HANDLER( pia_4_cb2_r );
READ8_HANDLER( pia_5_cb2_r );
READ8_HANDLER( pia_6_cb2_r );
READ8_HANDLER( pia_7_cb2_r );

#endif
