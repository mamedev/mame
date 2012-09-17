/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip implementation

    By MooglyGuy
    contact mooglyguy@gmail.com with licensing and usage questions.

**********************************************************************/

/*****************************************************************************************************************

                                                             P P P P P P P   P P P P P P P
                                                             E E E E E E E   J J J J J J J
                                                             1 2 3 4 5 6 7   0 1 2 3 4 5 6
                   D   D D D D                               / / / / / / /   / / / / / / /
                   3   4 5 6 7                             ! ! ! ! ! ! ! !   ! ! ! ! ! ! !
                   /   / / / /                       ! !   C C C C C C C C   C C C C C C C
                   P V P P P P     D D G D D D D T T L U V S S S S S S S S G S S S S S S S
                   B C B B B B D D 1 1 N 1 1 1 1 M C W W C A A A A B B B B N C C C C D D D
                   3 C 4 5 6 7 8 9 0 1 D 2 3 4 5 S K E E C 0 1 2 3 0 1 2 3 D 0 1 2 3 0 1 2
                   | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
              +-------------------------------------------------------------------------------+
              |                                                                               |
              |                                                                               |
              |                                                                               |
              |                                                                               |
              |                                                                               |
      D2/PB2--|                                                                               |--PJ7/!CSD3
      D1/PB1--|                                                                               |--VCC
      D0/PB0--|                                                                               |--PD0/!KBD0/!INT0
         TDO--|                                                                               |--PD1/!KBD1/!INT1
         TDI--|                                                                               |--PD2/!KBD2/!INT2
         GND--|                                                                               |--PD3/!KBD3/!INT3
         !OE--|                                                                               |--PD4/!KBD4/!INT4
    !UDS/PC1--|                                                                               |--PD5/!KBD5/!INT5
         !AS--|                                                                               |--PD6/!KBD6/!INT6
          A0--|                                                                               |--PD7/!KBD7/!INT7
        !LDS--|                                                                               |--GND
        R/!W--|                                                                               |--LD0
  !DTACK/PC5--|                                                                               |--LD1
      !RESET--|                                                                               |--LD2
         VCC--|                                                                               |--LD3
     !WE/PC6--|                                                                               |--LFRM
    !JTAGRST--|                                                                               |--LLP
       BBUSW--|                                  MC68328PV                                    |--LCLK
          A1--|                                   TOP VIEW                                    |--LACD
          A2--|                                                                               |--VCC
          A3--|                                                                               |--PK0/SPMTXD0
          A4--|                                                                               |--PK1/SPMRXD0
          A5--|                                                                               |--PK2/SPMCLK0
          A6--|                                                                               |--PK3/SPSEN
         GND--|                                                                               |--PK4/SPSRXD1
          A7--|                                                                               |--PK5/SPSCLK1
          A8--|                                                                               |--PK6/!CE2
          A9--|                                                                               |--PK7/!CE1
         A10--|                                                                               |--GND
         A11--|                                                                               |--PM0/!CTS
         A12--|                                                                               |--PM1/!RTS
         A13--|                                                                               |--PM2/!IRQ6
         A14--|                                                                               |--PM3/!IRQ3
         VCC--|                                                                               |--PM4/!IRQ2
         A15--|                                                                               |--PM5/!IRQ1
     A16/PA0--|                                                                               |--PM6/!PENIRQ
              |                                                                               |
              |   _                                                                           |
              |  (_)                                                                          |
              |\                                                                              |
              | \                                                                             |
              +-------------------------------------------------------------------------------+
                   | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
                   P P P P P G P P P P P P P P V P P P P P P P P G P P P V C G P P P E X P
                   A A A A A N A A F F F F F F C F F G G G G G G N G G C C L N C M L X T L
                   1 2 3 4 5 D 6 7 0 1 2 3 4 5 C 6 7 7 6 5 4 3 2 D 1 0 0 C K D 4 7 L T A L
                   / / / / /   / / / / / / / /   / / / / / / / /   / / /   O   / / G A L V
                   A A A A A   A A A A A A A A   A A R T ! T ! P   R T M       ! U N L   C
                   1 1 1 2 2   2 2 2 2 2 2 2 2   3 3 T I T I T W   X X O       I A D     C
                   7 8 9 0 1   2 3 4 5 6 7 8 9   0 1 C N O N O M   D D C       R R
                                                     O 1 U 2 U O       L       Q T
                                                         T   T         K       7 G
                                                         1   2                   P
                                                                                 I
                                                                                 O

                   Figure 12-1. MC68328 144-Lead Plastic Thin-Quad Flat Pack Pin Assignment

                      Source: MC68328 (DragonBall)(tm) Integrated Processor User's Manual

*****************************************************************************************************************/

#ifndef __MC68328_H_
#define __MC68328_H_

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct mc68328_interface
{
    const char *m68k_cpu_tag;

    devcb_write8  out_port_a_func;    /* 8-bit output */
    devcb_write8  out_port_b_func;    /* 8-bit output */
    devcb_write8  out_port_c_func;    /* 8-bit output */
    devcb_write8  out_port_d_func;    /* 8-bit output */
    devcb_write8  out_port_e_func;    /* 8-bit output */
    devcb_write8  out_port_f_func;    /* 8-bit output */
    devcb_write8  out_port_g_func;    /* 8-bit output */
    devcb_write8  out_port_j_func;    /* 8-bit output */
    devcb_write8  out_port_k_func;    /* 8-bit output */
    devcb_write8  out_port_m_func;    /* 8-bit output */

    devcb_read8   in_port_a_func;     /* 8-bit input */
    devcb_read8   in_port_b_func;     /* 8-bit input */
    devcb_read8   in_port_c_func;     /* 8-bit input */
    devcb_read8   in_port_d_func;     /* 8-bit input */
    devcb_read8   in_port_e_func;     /* 8-bit input */
    devcb_read8   in_port_f_func;     /* 8-bit input */
    devcb_read8   in_port_g_func;     /* 8-bit input */
    devcb_read8   in_port_j_func;     /* 8-bit input */
    devcb_read8   in_port_k_func;     /* 8-bit input */
    devcb_read8   in_port_m_func;     /* 8-bit input */

    devcb_write8  out_pwm_func;       /* 1-bit output */

    devcb_write16 out_spim_func;      /* 16-bit output */
    devcb_read16  in_spim_func;       /* 16-bit input */
    void (*spim_xch_trigger)( device_t *device );    /* SPIM exchange trigger */
};
#define MC68328_INTERFACE(name) const mc68328_interface (name)=

#define MC68328_TAG "dragonball"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MC68328_ADD(_intrf) \
    MCFG_DEVICE_ADD("dragonball", MC68328, 0) \
    MCFG_DEVICE_CONFIG(_intrf)

/*----------- defined in machine/mc68328.c -----------*/

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

DECLARE_WRITE16_DEVICE_HANDLER( mc68328_w );
DECLARE_READ16_DEVICE_HANDLER(  mc68328_r );


/***************************************************************************
    EXTERNAL I/O LINES
***************************************************************************/

void mc68328_set_penirq_line(device_t *device, int state);
void mc68328_set_port_d_lines(device_t *device, UINT8 state, int bit);

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

class mc68328_device : public device_t
{
public:
	mc68328_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mc68328_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type MC68328;


/*----------- defined in video/mc68328.c -----------*/

/***************************************************************************
    VIDEO INTERFACE
***************************************************************************/

PALETTE_INIT( mc68328 );
VIDEO_START( mc68328 );
SCREEN_UPDATE_IND16( mc68328 );

#endif // __MC68328_H_
