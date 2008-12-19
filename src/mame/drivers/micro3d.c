/*====================================================================*/
/*                         'micro3d.c'                                */
/*               Microprose 3D Hardware MAME driver                   */
/*                     Philip J Bennett 2005                          */
/*                                                                    */
/* o F-15 Strike Eagle                                                */
/* o B.O.T.S.S - Battle of the Solar System                           */
/* o Super Tank Attack                                                */
/*                                                                    */
/* Current issues:                                                    */
/* o No 3D/Am29000 emulation (communications code patched out)        */
/* o Analogue controls not hooked up                                  */
/* o Incomplete/crap/incorrect 68901 and 68681 emulation (eg. timers) */
/* o Video emulation needs to be checked over                         */
/* o No sound.                                                        */
/*                                                                    */
/* F15SE v2.2 was missing the math & sound boards, so those may need  */
/* to be redumped!!!                                                  */
/*                                                                    */
/* It complains about bad object ROM checksums in the MFR test?       */
/* BOTSS and STANKATK crash when entering TMS monitor mode ?          */
/*====================================================================*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/2151intf.h"
#include "sound/upd7759.h"
#include "sound/dac.h"

#define M68681_CLK 3686400
#define M68901_CLK 4000000
#define M68901_XTAL1 4000000
#define TIMERA 250
#define TIMERB 250
#define TIMERC 250
#define TIMERD 250
#define DEBUG 1

/*====================================================================*/
/* Each CPU (bar 80C31) communicates with a computer plugged into the */
/* corresponding monitor port.                                        */
/* Status information is reported, and tests can be performed.        */
/*                                                                    */
/* We print this useful stuff out to the text prompt.                 */
/*====================================================================*/

#ifdef DEBUG
#define HOST_MONITOR_DISPLAY 1
#define VGB_MONITOR_DISPLAY 0
#define DRMATH_MONITOR_DISPLAY 0
#endif

static UINT16 *micro3d_sprite_vram;
static UINT16 *m68681_base;
static UINT16 *m68901_base;

static struct {
    UINT16 MR1A;
    UINT16 MR2A;
    UINT16 SRA;
    UINT16 CSRA;
    UINT16 CRA;
    UINT16 RBA;
    UINT16 TBA;
    UINT16 IPCR;
    UINT16 ACR;
    UINT16 ISR;
    UINT16 IMR;
    UINT16 CUR;
    UINT16 CTUR;
    UINT16 CLR;
    UINT16 CTLR;
    UINT16 MR1B;
    UINT16 MR2B;
    UINT16 SRB;
    UINT16 CSRB;
    UINT16 CRB;
    UINT16 RBB;
    UINT16 TBB;
    UINT16 IVR;
    UINT16 IPORT;
    UINT16 OPCR;
    UINT16 OPR;

    int MRA_ptr;
    int MRB_ptr;
} M68681;

static UINT8 ti_uart[9];
static int ti_uart_mode_cycle=0;
static int ti_uart_sync_cycle=0;

static void m68901_int_gen(running_machine *machine, int source);

/* 68901 */
enum{   TMRB=0,TXERR,TBE,RXERR,RBF,TMRA,GPIP6,GPIP7,     // A Registers
    	GPIP0,GPIP1,GPIP2,GPIP3,TMRD,TMRC,GPIP4,GPIP5    // B Registers
};

/* TI UART */
enum{   RX=0,TX,STATUS,SYN1,SYN2,DLE,MODE1,MODE2,COMMAND
};


/* Probably wrong and a bit crap */
static int data_to_i8031(const device_config *device)
{
	mame_printf_debug("68k sent data: %x\n",M68681.TBB);
    M68681.SRB |=0x0400;                   // Data has been sent - TX ready for more.
    // Write to sound board
    if(M68681.IMR & 0x1000)
    {
    	cpu_set_input_line_and_vector(device->machine->cpu[0],3, HOLD_LINE, M68681.IVR);         // Generate an interrupt, if allowed.
    }
	return M68681.TBB;
}

static void data_from_i8031(const device_config *device, int data)
{
	M68681.RBB  = data<<8;                         // Put into receive buffer.
	M68681.SRB |= 0x0100;                          // Set Receiver B ready.
	if(M68681.IMR & 0x1000)
	{
		cpu_set_input_line_and_vector(device->machine->cpu[0],3, HOLD_LINE, M68681.IVR);    // Generate a receiver interrupt.
		mame_printf_debug("INTERRUPT!!!\n");
	}
	mame_printf_debug("8031 sent data: %x\n",data);
}


static void changecolor_BBBBBRRRRRGGGGGG(running_machine *machine,pen_t color,int data)
{
	palette_set_color_rgb(machine,color,pal5bit(data >> 6),pal5bit(data >> 1),pal5bit(data >> 11));
}

static WRITE16_HANDLER( paletteram16_BBBBBRRRRRGGGGGG_word_w )
{
	COMBINE_DATA(&paletteram16[offset]);
	changecolor_BBBBBRRRRRGGGGGG(space->machine,offset,paletteram16[offset]);
}


static void micro3d_scanline_update(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *src = &micro3d_sprite_vram[(params->rowaddr << 8) & 0x7fe00];
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = params->coladdr;
	int x;

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pix = src[coladdr++ & 0x1ff];

		if (pix & 0x80)
			dest[x + 0] = (pix & 0x7f) + 0xf00;
		else
			dest[x + 0] = 0;	/* 3D data */

		pix >>= 8;
		if (pix & 0x80)
			dest[x + 1] = (pix & 0x7f) + 0xf00;
		else
			dest[x + 1] = 0;	/* 3D data */
	}
}

static MACHINE_RESET( micro3d )
{
        i8051_set_serial_tx_callback(machine->cpu[2], data_from_i8031);
        i8051_set_serial_rx_callback(machine->cpu[2], data_to_i8031);
        ti_uart[STATUS]=1;
        M68681.SRA=0x0500;
        M68681.SRB=0x0500;
}

static DRIVER_INIT( stankatk )
{
       UINT16 *rom = (UINT16 *)memory_region(machine, "main");
       rom[0x1F543]=0x4E71;                                       /* 3ea86 - nop */
       rom[0x1F546]=0x4E71;                                       /* 3ea8c - nop */
       rom[0x1F596]=0x4E71;
       rom[0x2182B]=0x6006;      /* Skip AM2k object download */
       rom[0x21838]=0x4E71;
       rom[0x1F692]=0x4e71;
       rom[0x21CA8]=0x4e71;      /* Lie that trig table is intact */
       rom[0x21C60]=0x4e71;      /* Lie that tree is intact */
       rom[0x1F581]=0x4e71;
       rom[0x21905]=0x4E71;
}

static DRIVER_INIT( botss )
{
       UINT16 *rom = (UINT16 *)memory_region(machine, "main");
       rom[0x1FCC0]=0x4E71;         /* Eliminate startup Am29000 timeout */
       rom[0x1FBF0]=0x4E71;         /* Skip AM29k code version detect */
       rom[0x1FC88]=0x4E71;
       rom[0x23146]=0x4E71;
       rom[0x17F4F]=0x6006;         /* Skip download objects */
       rom[0x17F5B]=0x4E71;

}

static DRIVER_INIT( f15se )
{
//       UINT16 *rom = (UINT16 *)memory_region(machine, "main");
}

static DRIVER_INIT( f15se21 )
{
#if 1
       UINT16 *rom = (UINT16 *)memory_region(machine, "main");
       rom[0x2A8B3]=0x6006;                          //055166: 6606                     bne     5516e -> bra
       rom[0x2A8BF]=0x4E71;                          //05517E: 6704                     beq     55184 -> nop
       rom[0x28AD1]=0x4E71;                          //0515A2: 67F8                     beq     5159c -> nop
       rom[0x28A9E]=0x4E71;
       rom[0x28ABD]=0x4E71;
       rom[0x28C3B]=0x4E71;
#endif
}

static INPUT_PORTS_START( stankatk )
	/* Ports A and B */
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0001, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
//  PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x0002, DEF_STR(Off) )
//  PORT_DIPSETTING(    0x0000, DEF_STR(On) )

//  PORT_BITX(0x0002, IP_ACTIVE_LOW, 0, "????ht", KEYCODE_K, IP_JOY_NONE )

	PORT_DIPNAME( 0x0004, 0x0000, "Shared Memory Handshake Test")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0004, DEF_STR(On) )
	PORT_DIPNAME( 0x0008, 0x0000, "Dr. Math Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0008, DEF_STR(On) )
	PORT_DIPNAME( 0x0010, 0x0000, "Burn-in Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0010, DEF_STR(On))
	PORT_DIPNAME( 0x0020, 0x0000, "Manufacturing Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0020, DEF_STR(On) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0040, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0080, 0x0000, "Host Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0080, DEF_STR(On) )

//  PORT_BITX(0x0100, IP_ACTIVE_LOW, 0, "Blue Lower Right", KEYCODE_A, IP_JOY_NONE )
//  PORT_BITX(0x0200, IP_ACTIVE_LOW, 0, "Blue Upper Left", KEYCODE_B, IP_JOY_NONE )
//  PORT_BITX(0x0800, IP_ACTIVE_LOW, 0, "Blue Lower Left", KEYCODE_C, IP_JOY_NONE )
//  PORT_BITX(0x1000, IP_ACTIVE_LOW, 0, "Yellow Upper Right", KEYCODE_E, IP_JOY_NONE )
//  PORT_BITX(0x2000, IP_ACTIVE_LOW, 0, "Yellow Lower Right", KEYCODE_F, IP_JOY_NONE )

//  PORT_BITX(0x000f, IP_ACTIVE_HIGH, 0, "Yeft", KEYCODE_K, IP_JOY_NONE )
	PORT_SERVICE(0x0400, IP_ACTIVE_LOW)


	/* C and D ports */
	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
//  PORT_BITX( 0x0800, IP_ACTIVE_LOW, 0, "Unk2", KEYCODE_S, IP_JOY_NONE )

//  PORT_BITX( 0x0010, IP_ACTIVE_LOW, 0, "Yellow Upper Right", KEYCODE_Y, IP_JOY_NONE )
//  PORT_BITX( 0x0020, IP_ACTIVE_LOW, 0, "Yellow Lower Right", KEYCODE_X, IP_JOY_NONE )
//  PORT_BITX( 0x0040, IP_ACTIVE_LOW, 0, "Unk2", KEYCODE_R, IP_JOY_NONE )
//  PORT_BITX( 0x0080, IP_ACTIVE_LOW, 0, "Unkn3", KEYCODE_T, IP_JOY_NONE )

//  PORT_BITX( 0x0001, IP_ACTIVE_HIGH, 0, "Blue Top Right", KEYCODE_Z, IP_JOY_NONE )  // ??
//  PORT_BITX( 0x0002, IP_ACTIVE_HIGH, 0, "Blue Top Right", KEYCODE_Z, IP_JOY_NONE )
//  PORT_BITX( 0x0004, IP_ACTIVE_LOW, 0, "Blue Top Right", KEYCODE_D, IP_JOY_NONE )
//  PORT_BITX( 0x0008, IP_ACTIVE_HIGH, 0, "Blue Top Right", KEYCODE_Z, IP_JOY_NONE )

//  PORT_BITX( 0x1000, IP_ACTIVE_LOW, 0, "Unk2", KEYCODE_I,Y IP_JOY_NONE )
//  PORT_BITX( 0x2000, IP_ACTIVE_LOW, 0, "Blue Trigger", KEYCODE_H, IP_JOY_NONE )
//  PORT_BITX( 0x4000, IP_ACTIVE_LOW, 0, "Unkn3", KEYCODE_J, IP_JOY_NONE )
//  PORT_BITX( 0x8000, IP_ACTIVE_LOW, 0, "Yellow Trigger", KEYCODE_G, IP_JOY_NONE )


	PORT_START("VGB")
	PORT_DIPNAME( 0x0008, 0x0008, "VGB Monitor Mode")
	PORT_DIPSETTING(	0x0008, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )

INPUT_PORTS_END

static INPUT_PORTS_START( botss )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0001, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0002, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0004, 0x0000, "Shared Memory Handshake Test")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0004, DEF_STR(On) )
	PORT_DIPNAME( 0x0008, 0x0000, "Dr. Math Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0008, DEF_STR(On) )
	PORT_DIPNAME( 0x0010, 0x0000, "Burn-in Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0010, DEF_STR(On))
	PORT_DIPNAME( 0x0020, 0x0000, "Manufacturing Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0020, DEF_STR(On) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0040, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0080, 0x0000, "Host Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0080, DEF_STR(On) )
//  PORT_BITX(0x0100, IP_ACTIVE_LOW, IPT_START1, "Start", IP_KEY_DEFAULT, IP_JOY_DEFAULT )
//  PORT_BITX(0x0200, IP_ACTIVE_LOW, 0, "Shield", KEYCODE_A, IP_JOY_NONE )
	PORT_SERVICE(0x0400, IP_ACTIVE_LOW)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	/* C and D ports */
	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
//  PORT_BITX(0x1000, IP_ACTIVE_LOW, 0, "Trigger", KEYCODE_B, IP_JOY_NONE )
//  PORT_BITX(0x2000, IP_ACTIVE_LOW, 0, "Blaster", KEYCODE_C, IP_JOY_NONE )
//  PORT_BITX(0x0002, IP_ACTIVE_LOW, 0, "Throttle Up", KEYCODE_D, IP_JOY_NONE )
//  PORT_BITX(0x0008, IP_ACTIVE_LOW, 0, "Throttle Down", KEYCODE_F, IP_JOY_NONE )

	PORT_START("VGB")
	PORT_DIPNAME( 0x0008, 0x0008, "VGB Monitor Mode")
	PORT_DIPSETTING(	0x0008, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )

INPUT_PORTS_END


static INPUT_PORTS_START( f15se )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0001, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0002, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0004, 0x0000, "Shared Memory Handshake Test")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0004, DEF_STR(On) )
	PORT_DIPNAME( 0x0008, 0x0000, "Dr. Math Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0008, DEF_STR(On) )
	PORT_DIPNAME( 0x0010, 0x0000, "Burn-in Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0010, DEF_STR(On))
	PORT_DIPNAME( 0x0020, 0x0000, "Manufacturing Tests")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0020, DEF_STR(On) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x0040, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )
	PORT_DIPNAME( 0x0080, 0x0000, "Host Monitor Mode")
	PORT_DIPSETTING(	0x0000, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0080, DEF_STR(On) )
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE(0x0400, IP_ACTIVE_LOW)

	/* C and D ports */
	PORT_START("IN1")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 )

	/* Analogue Inputs? */

	PORT_START("VGB")
	PORT_DIPNAME( 0x0008, 0x0008, "VGB Monitor Mode")
	PORT_DIPSETTING(	0x0008, DEF_STR(Off) )
	PORT_DIPSETTING(	0x0000, DEF_STR(On) )

	/* Sound PCB test button */
	PORT_START("SOUND")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Sound PCB Test") PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END


static void tms_interrupt(const device_config *device, int state)
{
   m68901_int_gen(device->machine, GPIP4);
}

static INTERRUPT_GEN( micro3d_vblank )
{
   m68901_int_gen(device->machine, GPIP7);
}


/* 68901 related gubbins. Re-write me please */

static TIMER_CALLBACK( timera_int )
{
//      timer_set(machine, attotime_mul(ATTOTIME_IN_HZ(M68901_CLK), ((m68901_base[0xf]>>8) & 0xff) * 200),0,timera_int);     // Set the timer again.
        timer_set(machine, ATTOTIME_IN_USEC(1000), NULL,0,timera_int);     // Set the timer again.
        m68901_int_gen(machine, TMRA);           // Fire an interrupt.
}

#ifdef UNUSED_FUNCTION
static TIMER_CALLBACK( timerb_int )
{
        timer_set(machine, attotime_mul(ATTOTIME_IN_HZ(M68901_CLK), ((m68901_base[0x10]>>8) & 0xff) * 200),0,timera_int);
        m68901_int_gen(machine, TMRB);           // Fire an interrupt.
}

static TIMER_CALLBACK( timerc_int )
{
        timer_set(machine, attotime_mul(ATTOTIME_IN_HZ(M68901_CLK), ((m68901_base[0x11]>>8) & 0xff) * 200),0,timera_int);
        m68901_int_gen(machine, TMRC);           // Fire an interrupt.
}
#endif

static TIMER_CALLBACK( timerd_int )
{
        timer_set(machine, ATTOTIME_IN_USEC(250), NULL,0,timerd_int);
        m68901_int_gen(machine, TMRD);           // Fire an interrupt.
}


/* Called by anything that generates a MFD interrupt */
/* Requires: MFD line number */

static void m68901_int_gen(running_machine *machine, int source)
{
// logerror("M68901 interrupt %d requested.\n",source);

int IEN_REG=3+(int)(source / 8);
int IPEND_REG=5+(int)(source / 8);       // Makes things a little clearer.
int IMASK_REG=9+(int)(source / 8);
int bit=1 << (source-8*(int)(source/8));

       if(m68901_base[IEN_REG] & (bit<<8))                // If interrupt is enabled by MFD, set interrupt pending bit.
       {
              m68901_base[IPEND_REG] = m68901_base[IPEND_REG] | (bit<<8);
            //  logerror("M68901 interrupt %d now pending (%d) \n",source, IPEND_REG);
       }

       if(m68901_base[IMASK_REG] & (bit<<8))              // If interrupt is not masked by MFD, trigger a 68k INT
       {
                cpu_set_input_line(machine->cpu[0],4, HOLD_LINE);
            //    logerror("M68901 interrupt %d serviced.\n",source);
       }
}

static WRITE16_HANDLER( m68901_w )
{

UINT8 value = (data>>8)& 0xff;
m68901_base[offset]=data;

switch(offset)
{

        case 0x0c:    mame_printf_debug("Timer A Control: %x\n",value);
                      break;
        case 0x0d:    mame_printf_debug("Timer B Control: %x\n",value);
                      break;
        case 0x0e:    mame_printf_debug("Timer C/D Control: %x\n",value);
                      break;

        case 0x0f:    mame_printf_debug("Timer A Data:%4x\n",value);                                                       // Timer A Data Register
                     timer_set(space->machine, ATTOTIME_IN_USEC(1000), NULL,0,timera_int);
                      break;

        case 0x10:    mame_printf_debug("Timer B Data:%4x\n",value);                                                           // Timer B Data Register
//                    timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(M68901_CLK), value * 200),0,timerb_int);
                      break;

        case 0x11:    mame_printf_debug("Timer C Data:%4x\n",value);                                                        // Timer C Data Register
//                    timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(M68901_CLK), value * 200),0,timerc_int);
                      break;

        case 0x12:    mame_printf_debug("Timer D Data:%4x\n",value);
                      timer_set(space->machine, ATTOTIME_IN_USEC(500), NULL,0,timerd_int);                 // Timer D Data Register
                      break;

}

}

/* I should really re-write all this. */

static WRITE16_HANDLER( m68681_w )
{

UINT8 value = (data>>8)& 0xff;

switch(offset)
{
      case 0x00:        break;                                // Mode Register A

      case 0x01:        M68681.CSRA = value;
                        break;
                                                              // Clock-select register A
      case 0x02:        M68681.CRA = value;
                        break;                                // Command Register A

      case 0x03:        M68681.TBA = value;                   // Fill transmit buffer
                        M68681.SRA |=0x0400;                  // Data has been sent - TX ready for more.
                        if(M68681.IMR & 1)   cpu_set_input_line_and_vector(space->machine->cpu[0],3, HOLD_LINE, M68681.IVR);         // Generate an interrupt, if allowed.

#if HOST_MONITOR_DISPLAY
                        mame_printf_debug("%c",value);                    // Port A - Monitor
#endif
                        break;

      case 0x04:        break;

      case 0x05:        //mame_printf_debug("IMR=%x\n",M68681.IMR);
                        M68681.IMR = value;                    // Interrupt Mask Reg
                        break;

      case 0x06:        break;

      case 0x07:        break;

      case 0x08:        break;

      case 0x09:        break;

      case 0x0a:        break;

      case 0x0b:        M68681.TBB = value;                   //  Fill transmit buffer
#if 0
                        M68681.SRB |=0x0400;                   // Data has been sent - TX ready for more.
                        // Write to sound board
                        if(M68681.IMR & 0x1000)
                        {
                        	cpu_set_input_line_and_vector(space->machine->cpu[0],3, HOLD_LINE, M68681.IVR);         // Generate an interrupt, if allowed.
                        }
                        cpu_set_input_line(space->machine->cpu[2], MCS51_RX_LINE, ASSERT_LINE);                      // Generate 8031 interrupt
                        mame_printf_debug("Sound board TX: %4X at PC=%4X\n",value,cpu_get_pc(space->cpu));
#endif
                        M68681.SRB &=~0x0400;                   // Data has been sent - TX ready for more.
                        cpu_set_input_line(space->machine->cpu[2], MCS51_RX_LINE, ASSERT_LINE);                      // Generate 8031 interrupt
                        mame_printf_debug("Sound board TX: %4X at PC=%4X\n",value,cpu_get_pc(space->cpu));
                        break;

      case 0x0c:        //mame_printf_debug("IVR: %d",value);
                        M68681.IVR = value;                   // Interrupt vector register.
                        break;

      case 0x0d:        break;
      case 0x0e:
                        set_led_status(1, data&0x80);         // Host Health LED
                        if(value & 0x20)
                        {
//                              cpunum_set_reset_line(2 CLEAR_LINE);
                              logerror("8031 running at:%x (val=%x).\n",cpu_get_pc(space->cpu),value);
                        }
                        break;

      case 0x0f:        set_led_status(1, ~(data&0x80));      // Host Health LED
                        if(value & 0x20)
                        {
//                              cpunum_set_reset_line(2, ASSERT_LINE);
                              logerror("8031 reset at:%x (val=%x).\n",cpu_get_pc(space->cpu),value);
                        }
                        break;

      default:          m68681_base[offset]=data;
}
}

static READ16_HANDLER( m68681_r )
{

switch(offset)
{
//      case 0x00:
        case 0x01:     return M68681.SRA;                              // Status Register A
//      case 0x02:     return

        case 0x03:     M68681.SRA^=0x100;
                       return M68681.RBA;                              // RX A - Monitor Port

        case 0x09:     return M68681.SRB;                              // Status Register B

        case 0x0b:     mame_printf_debug("\nHost received: %x\n",M68681.RBB);
        				M68681.SRB^=0x0100;                             // No longer have data.
        				//M68681.SRB &= ~0x0100;                             // No longer have data.
                       return M68681.RBB;
                                                                       // RX B - Monitor Port
}

return 0;
}


static WRITE16_HANDLER( ti_uart_w )
{
// Need to initalise values to 0 on reset.
switch(offset)
{
        case 0x0: ti_uart[TX] = data;                  // Pointless really - nothing happens to it.
#if VGB_MONITOR_DISPLAY
                  mame_printf_debug("%c",data);

#endif
                  ti_uart[STATUS]|=1;                  // Transmit regiser empty.
                  break;

        case 0x1: if(!ti_uart_mode_cycle)
                  {
                        ti_uart[MODE1] = data;
                        ti_uart_mode_cycle=1;
                  }
                  else
                  {
                        ti_uart[MODE2] = data;
                        ti_uart_mode_cycle=0;
                  }
                  break;

        case 0x2: if(ti_uart_sync_cycle==0)
                            {
                                  ti_uart[SYN1] = data;
                                  ti_uart_mode_cycle=1;
                            }
                            else if(ti_uart_sync_cycle==1)
                            {
                                  ti_uart[SYN2] = data;
                                  ti_uart_mode_cycle=2;
                            }
                            else
                            {
                                 ti_uart[DLE] = data;
                                 ti_uart_mode_cycle=0;
                            }
                            break;

        case 0x3: ti_uart[COMMAND] = data;
                  ti_uart_mode_cycle = ti_uart_sync_cycle=0;
                  break;
}

}


static READ16_HANDLER( ti_uart_r )
{

switch(offset)
{
        case 0x0: ti_uart[STATUS]^=2;
                  return ti_uart[RX];

        case 0x1: if(!ti_uart_mode_cycle)
                            {
                                  ti_uart_mode_cycle=1;
                                  return ti_uart[MODE1];
                            }
                            else
                            {
                                  ti_uart_mode_cycle=0;
                                  return ti_uart[MODE2];
                            }
        case 0x2: return ti_uart[STATUS];

        case 0x3: ti_uart_mode_cycle = ti_uart_sync_cycle=0;
                  return ti_uart[COMMAND];

        default:  logerror("Unknown TI UART access.\n");
                  return 0;
}

}

#ifdef UNUSED_FUNCTION
static WRITE32_HANDLER( am_uart_w )
{
       mame_printf_debug("%c",data);
}

static READ32_HANDLER( am_uart_r )
{
       return 0xff;
}


static WRITE16_HANDLER( mystery_w )
{
       popmessage("Write to 900000: %x",data);
}

static WRITE16_HANDLER( mystery2_w )
{
       popmessage("TMS Write to e00000: %x",data);
}

static WRITE16_HANDLER( mystery3_w )
{
       popmessage("TMS Write to 2600000: %x",data);
}
#endif


static READ16_HANDLER( tms_host_r )
{
	return tms34010_host_r(space->machine->cpu[1], offset);
}

static WRITE16_HANDLER( tms_host_w )
{
	tms34010_host_w(space->machine->cpu[1], offset, data);
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( reset_slave )
{
        if(data & 0x0020)
        {
//          cpu_set_input_line(1, CLEAR_LINE);
                logerror("TMS Running\n");
	}
	else
	{
//                cpu_set_input_line(1, ASSERT_LINE);
        	logerror("TMS Reset\n");
        }
}
#endif



/* The DS1215 phantom timer IC is mapped to the lower 32kB SRAM address space */
/* Serial Read/write operations are conducted via the 0th data bit */

/* Reset peripherals flip-flop MIGHT be 900000 */

static ADDRESS_MAP_START( hostmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x13ffff) AM_ROM					/* 68000 Code ROM and Dr. Math object data */
	AM_RANGE(0x200000, 0x20ffff) AM_RAM					/* Battery-backed SRAM (64kB) & DS1215 */
	AM_RANGE(0x800000, 0x83ffff) AM_RAM					/* 68000/AM29000 shared RAM (256kB) */
	AM_RANGE(0x900000, 0x900001) AM_NOP //WRITE(mystery_w)  /* ??????? 16-bit write here. rset? */
	AM_RANGE(0x920000, 0x920001) AM_READ_PORT("IN1")	/* User inputs C and D */
	AM_RANGE(0x940000, 0x940001) AM_READ_PORT("IN0")	/* User inputs A and B */
	AM_RANGE(0x960000, 0x960001) AM_NOP					/* Lamps */
	AM_RANGE(0x980000, 0x980001) AM_RAM					/* ADC0844 */
	AM_RANGE(0x9a0000, 0x9a0007) AM_READWRITE(tms_host_r, tms_host_w)	/* TMS34010 Interface */
	AM_RANGE(0x9c0000, 0x9c0001) AM_RAM					/* ????? Write: 80, A0 and 00 (8-bit high byte) */
	AM_RANGE(0x9e0000, 0x9e00cf) AM_RAM_WRITE(m68901_w) AM_BASE(&m68901_base)	/* 68901 Multifunction Peripheral */
	AM_RANGE(0xa00000, 0xa000cf) AM_READWRITE(m68681_r, m68681_w) AM_BASE(&m68681_base)	/* 68681 UART */
	AM_RANGE(0xa20000, 0xa20001) AM_RAM					/* XY joystick input - sign? */
	AM_RANGE(0xa40002, 0xa40003) AM_RAM					/* XY joystick input - actual values */
ADDRESS_MAP_END


// 2600000: Write 1E and 9E?
static ADDRESS_MAP_START( vgbmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_BASE(&micro3d_sprite_vram)	/* 2 Banks */
	AM_RANGE(0x00800000, 0x00bfffff) AM_RAM							/* 512kB Main DRAM */
	AM_RANGE(0x00c00000, 0x00c0000f) AM_READ_PORT("VGB")			/* TI Monitor Mode switch */
	AM_RANGE(0x00e00000, 0x00e0000f) AM_RAM //WRITE(mystery2_w)     /* CREGCLK ??? byte write here. */
	AM_RANGE(0x02000000, 0x0200ffff) AM_RAM_WRITE(paletteram16_BBBBBRRRRRGGGGGG_word_w) AM_BASE(&paletteram16) // AM_RANGE(0x02010000, 0x027fffff) AM_RAM       /* ??????????? Mirror of VRAM??? */
	AM_RANGE(0x02600000, 0x0260000f) AM_RAM							/* XFER3dk???? 16-bit write */
	AM_RANGE(0x02c00000, 0x02c0003f) AM_READ(ti_uart_r)				/* SCN UART */
	AM_RANGE(0x02e00000, 0x02e0003f) AM_WRITE(ti_uart_w)
	AM_RANGE(0x03800000, 0x03dfffff) AM_ROM AM_REGION("gfx1", 0)		/* 2D Graphics ROMs */
	AM_RANGE(0x03e00000, 0x03ffffff) AM_ROM AM_REGION("user1", 0)		/* 128kB Program ROM */
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)		/* 128kB Program ROM (Mirror - for interrupt vectors) */
ADDRESS_MAP_END


/*====================================================================*/
/* The 8031 ports are mapped as follows:                              */
/*                                                                    */
/* P1.0 = SHMUX-A  O         P3.0                                     */
/* P1.1 = SHMUX-B  O         P3.1                                     */
/* P1.2 = SHMUX-C  O         P3.2 = SPCHBANK O                        */
/* P1.3 = SHMUX-E  O         P3.3 = SP-BUSY- I                        */
/* P1.4 = VOLDQ    O         P3.4 = RESET    O                        */
/* P1.5 = VOLCK    O         P3.5 = WDO      O                        */
/* P1.6 = VOLRST   O         P3.6                                     */
/* P1.7 = SELFTEST I         P3.7                                     */
/*====================================================================*/

static UINT8 port_latch[4];
static WRITE8_HANDLER(sound_io_w)
{
	port_latch[offset] = data;
	switch(offset)
	{
        case 0x01:
        	break;
        case 0x03:
        	upd7759_set_bank_base(0, (data & 0x4) ? 0x20000 : 0);
        	upd7759_0_reset_w(space,0,(data & 0x10) ? 0 : 1);
	}
}

static READ8_HANDLER(sound_io_r)
{
	switch(offset)
	{
	        case 0x01:  return (port_latch[offset] & 0x7f) | input_port_read_safe(space->machine, "SOUND", 0);		/* Test push switch */
	        case 0x03:  return (port_latch[offset] & 0xf7) | (upd7759_0_busy_r(space,0) ? 0x08 : 0);
	        default:    return 0;
	}

}

static WRITE8_HANDLER( upd7759_port_start_w)
{
	upd7759_0_start_w(space, offset, 0);
	upd7759_0_port_w(space, offset, data);
	upd7759_0_start_w(space, offset, 1);
}

static ADDRESS_MAP_START( soundmem_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                    // 32kB Code EPROM
ADDRESS_MAP_END


/* FFXX - 00XX    */

static ADDRESS_MAP_START( soundmem_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM                                  /* 2Kb RAM */
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(ym2151_register_port_0_w)
	AM_RANGE(0xfd01, 0xfd01) AM_READWRITE(ym2151_status_port_0_r,ym2151_data_port_0_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(upd7759_port_start_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(dac_0_data_w)   /* DAC A - used for S&H, special effects? */
	AM_RANGE(0xff01, 0xff01) AM_WRITE(dac_1_data_w)   /* DAC B - 'SPEECH' */
	/* ports */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(sound_io_r,sound_io_w)
ADDRESS_MAP_END


static const tms34010_config vgb_config =
{
	FALSE,							/* halt on reset ????? - check this */
	"main",							/* the screen operated on */
	40000000/8,						/* pixel clock */
	4,								/* pixels per clock */
	micro3d_scanline_update,		/* scanline updater */
	tms_interrupt,					/* Generate interrupt */
	NULL,
	NULL
};


static MACHINE_DRIVER_START( micro3d )
	MDRV_CPU_ADD("main", M68000, 12000000 )
	MDRV_CPU_PROGRAM_MAP(hostmem,0)
	MDRV_CPU_VBLANK_INT("main", micro3d_vblank)

 	MDRV_CPU_ADD("vgb", TMS34010, 40000000)
	MDRV_CPU_CONFIG(vgb_config)
	MDRV_CPU_PROGRAM_MAP(vgbmem,0)

	MDRV_CPU_ADD("audio", I8051, 11059000)
	MDRV_CPU_PROGRAM_MAP(soundmem_prg,0)
	MDRV_CPU_IO_MAP(soundmem_io,0)

	MDRV_MACHINE_RESET(micro3d)
	MDRV_INTERLEAVE(50)

	MDRV_PALETTE_LENGTH(32768)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(40000000/8*4, 192*4, 0, 144*4, 434, 0, 400)

	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD("ym", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.0)	/* muted - I don't think it's directly used */

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.15)
MACHINE_DRIVER_END


ROM_START( botss )
	/* HOST PCB (MPG DW-00011C-0011-02) */
	ROM_REGION( 0x140000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "300hst.67", 0x000001, 0x20000, CRC(7f74362a) SHA1(41611ba8e6eb5d6b3dfe88e1cede7d9fb5472e40) ) // rev 1.1
	ROM_LOAD16_BYTE( "301hst.91", 0x000000, 0x20000, CRC(a8100d1e) SHA1(69d3cac6f67563c0796560f7b874d7660720027d) ) // rev 1.1
	ROM_LOAD16_BYTE( "302hst.68", 0x040001, 0x20000, CRC(af865ee4) SHA1(f00bce49401431bc749208399329d9f92457186b) ) // rev 1.1
	ROM_LOAD16_BYTE( "303hst.92", 0x040000, 0x20000, CRC(15182619) SHA1(e95dcce11c0651c8e85fc0c658029f48eea35fb8) ) // rev 1.1
	ROM_LOAD16_BYTE( "104hst.69", 0x080001, 0x20000, CRC(72a607ca) SHA1(1afc85380be12c429808c48f1502736a4c8b98e5) )
	ROM_LOAD16_BYTE( "105hst.93", 0x080000, 0x20000, CRC(f37680ae) SHA1(51f1ee805b7d1b2b078c612c572e12846de623b9) )
	ROM_LOAD16_BYTE( "106hst.70", 0x0c0001, 0x20000, CRC(57a1c728) SHA1(2bdc831be739ada0f4f4adec7974da453878db0e) )
	ROM_LOAD16_BYTE( "107hst.94", 0x0c0000, 0x20000, CRC(4c9e16af) SHA1(1f8acc9bb85fe1bf459b4358b9bf9cf9847e6a36) )
	ROM_LOAD16_BYTE( "108hst.71", 0x100001, 0x20000, CRC(cfc0333e) SHA1(9f290769129a61189870faef45c3f061eb7b5c07) )
	ROM_LOAD16_BYTE( "109hst.95", 0x100000, 0x20000, CRC(6c595d1e) SHA1(89fdc30166ba1e9706798547195bdf6875a02e96) )

	/* DR MATH PCB (MPG 010-00003-003) */
	ROM_REGION( 0x40000, "user2", 0 )
	ROM_LOAD( "014dth.153", 0x000000, 0x20000, CRC(0eee0557) SHA1(8abe52cad31e59cf814fd9f64f4e42ddb4aa8c93) )
	ROM_LOAD( "015dth.154", 0x020000, 0x20000, CRC(68564122) SHA1(410d2db74e574774b2eadd7fdf891feef5d8a93f) )
	ROM_LOAD( "016dth.167", 0x000000, 0x20000, CRC(60c6cb26) SHA1(0e2bf65793715e12d8fd7f87fd3336a9d00ee7e6) )
	ROM_LOAD( "017dth.160", 0x020000, 0x20000, CRC(d8b89379) SHA1(aa08e111c1505a4ad55b14659f8e21fd39cfcb16) )
	ROM_LOAD( "118dth.135", 0x000000, 0x08000, CRC(2903e682) SHA1(027ed6524e9d4490632f10aeb22150c2fbc4eec2) )
	ROM_LOAD( "119dth.115", 0x020000, 0x08000, CRC(9c9dbac1) SHA1(4c66971884190598e128684ece2e15a1c80b94ed) )
	ROM_LOAD( "120dth.108", 0x000000, 0x08000, CRC(dafa173a) SHA1(a19980b92a5e74ebe395be36313701fdb527a46a) )
	ROM_LOAD( "121dth.127", 0x020000, 0x08000, CRC(198a636b) SHA1(356b8948aafb98cb5e6ee7b5ad6ea9e5998265e5) )
	ROM_LOAD( "122dth.134", 0x000000, 0x08000, CRC(bf60c487) SHA1(5ce80e89d9a24b627b0e97bf36a4e71c2eff4324) )
	ROM_LOAD( "123dth.114", 0x020000, 0x08000, CRC(04ba6ed1) SHA1(012be71c6b955beda2bd0ff376dcaab51b226723) )
	ROM_LOAD( "124dth.107", 0x000000, 0x08000, CRC(220db5d3) SHA1(3bfbe0eb97282c4ce449fd44e8e141de74f08eb0) )
	ROM_LOAD( "125dth.126", 0x020000, 0x08000, CRC(b0dccf4a) SHA1(e8bfd622c006985b724cdbd3ad14c33e9ed27c6c) )

	ROM_REGION16_LE( 0x40000, "user1", 0 )
	ROM_LOAD16_BYTE( "101vgb.101", 0x000000, 0x20000, CRC(6aada23d) SHA1(85dbf9b20e4f17cb21922637763654d6cae80dfd) )
	ROM_LOAD16_BYTE( "104vgb.97",  0x000001, 0x20000, CRC(715cac9d) SHA1(2aa0c563dc1fe4d02fa1ecbaed16f720f899fdc4) )

	/* VGB - Video Graphics PCB  (MPG DW-010-00002-002) */
	ROM_REGION16_LE( 0xc0000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "105vgb.124", 0x000000, 0x20000, CRC(5482e0c4) SHA1(492afac1862f2899cd734d1e57ca978ed6a906d5) )
	ROM_LOAD16_BYTE( "106vgb.121", 0x000001, 0x20000, CRC(a55e5d19) SHA1(86fbcb425103ae9fff381357339af349848fc3f2) )

	ROM_LOAD16_BYTE( "107vgb.130", 0x040000, 0x20000, CRC(006487b6) SHA1(f8bc6abad13df099da1708bd22f239703e407b21) )
	ROM_LOAD16_BYTE( "108vgb.133", 0x040001, 0x20000, CRC(e4587ba1) SHA1(1323b4be5a526ae182ee38e96fccd263a4cecc37) )

	ROM_LOAD16_BYTE( "103vgb.114", 0x080000, 0x20000, CRC(4e486e70) SHA1(04ee16cfadd43dbe9ed5bd8330c21a718d63a8f4) )
	ROM_LOAD16_BYTE( "102vgb.108", 0x080001, 0x20000, CRC(441e8490) SHA1(6cfe30cea3fa297b71e881fbddad6d65a96e4386) )

	/*SND - Sound PCB           (MPG 010-00018-002) */
	ROM_REGION( 0x10000, "audio", 0 )
	ROM_LOAD( "14-001snd.2", 0x000000, 0x08000, CRC(307fcb6d) SHA1(0cf63a39ac8920be6532974311804529d7218545) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "13-001snd.17", 0x000000, 0x40000, CRC(015a0b17) SHA1(f229c9aa59f0e6b25b818f9513997a8685e33982) )
ROM_END


ROM_START( stankatk )
	ROM_REGION( 0x140000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "lo_u67",    0x000001, 0x20000, CRC(97aabac0) SHA1(12a0719d3332a63e912161200b0a942c27c1f5da) )
	ROM_LOAD16_BYTE( "le_u91",    0x000000, 0x20000, CRC(977f90d9) SHA1(530fa5c32b1f28e2b90d20d98cc453cb290c0ad2) )
//  ROM_LOAD16_BYTE( "host.u67",  0x000001, 0x20000, CRC(e79d9548) SHA1(84a4c181be81fff7d5e61faa32f6929fce8b62d0) ) // alt (bad?) rom
//  ROM_LOAD16_BYTE( "host.u91",  0x000000, 0x20000, CRC(ea19d0e1) SHA1(c11e8c6bff4746e3536fc19d3a40c89ab198059f) ) // alt (bad?) rom
	ROM_LOAD16_BYTE( "ho_u68",    0x040001, 0x20000, CRC(8f76f4ac) SHA1(f6c1d4c933a373b153eee7d9f3016c985acaa281) )
	ROM_LOAD16_BYTE( "he_u92",    0x040000, 0x20000, CRC(1ea1db7c) SHA1(ecaa1bd3d70489a5ba0d96c6935c2959f57467b2) )
//  ROM_LOAD16_BYTE( "host.u92",  0x040000, 0x20000, CRC(220e5a39) SHA1(d9de87bc56182c8412c11bff9616164159ced9fb) ) // alt (bad?) rom
	ROM_LOAD16_BYTE( "b00_o.u69", 0x080001, 0x20000, CRC(393718e5) SHA1(f956f8bd946f53a032af16011dc69f66fb3f095c) )
	ROM_LOAD16_BYTE( "b00_e.u93", 0x080000, 0x20000, CRC(aedea0ef) SHA1(a81c3518c7a1e21f2fa2ad29c30346f727069257) )
	ROM_LOAD16_BYTE( "b01_o.u70", 0x0c0001, 0x20000, CRC(e895167d) SHA1(677cbf1be32c1f0c76a0e1527db66eb037d7e9df) )
	ROM_LOAD16_BYTE( "b01_e.u94", 0x0c0000, 0x20000, CRC(823bba4d) SHA1(6668e972b1435aac43f9b21cc40fc3adec0d285f) )
	ROM_LOAD16_BYTE( "host.71",   0x100001, 0x20000, CRC(cfc0333e) SHA1(9f290769129a61189870faef45c3f061eb7b5c07) ) // same as botss
	ROM_LOAD16_BYTE( "host.95",   0x100000, 0x20000, CRC(6c595d1e) SHA1(89fdc30166ba1e9706798547195bdf6875a02e96) ) // same as botss

	ROM_REGION( 0x40000, "user2", 0 )
	ROM_LOAD( "pb0o_u.153", 0x000000, 0x20000, CRC(bcd7ddad) SHA1(3982756b6f0821df77918dd0d00807a90dbfb595) )
	ROM_LOAD( "pb0e_u.154", 0x020000, 0x20000, CRC(d84e7c71) SHA1(2edb13c1f96f35c7934dad380e06035335ccbb48) )
	ROM_LOAD( "pb1o_u.167", 0x000000, 0x20000, CRC(e4a65313) SHA1(f2df5cc87aa388d3273705562ab2d7c937a0a866) )
	ROM_LOAD( "pb1e_u.160", 0x020000, 0x20000, CRC(9d9d1395) SHA1(9d937eac8d7e7bea40a69b596ba2c01753b97565) )

	ROM_LOAD( "s24o_u.135", 0x000000, 0x08000, CRC(f89bab5f) SHA1(e79e71d0a5e7ba933952c5d41f6afb633da06e8a) )
	ROM_LOAD( "s08o_u.115", 0x020000, 0x08000, CRC(af1eae4a) SHA1(44f272b472f546ffff7d8f82e29c5d80b472b1c3) )
	ROM_LOAD( "s00o_u.108", 0x000000, 0x08000, CRC(9cadc977) SHA1(e95f60d9df422511bae6a6c4a20f813d77a894a4) )
	ROM_LOAD( "s16o_u.127", 0x020000, 0x08000, CRC(53ba1a3f) SHA1(333734fff41b98abfa7b2904692cb128ab1f90a3) )
	ROM_LOAD( "s24e_u.134", 0x000000, 0x08000, CRC(0a41756b) SHA1(8681aaf8eeda7acdff967a773290c4b2c17cbe30) )
	ROM_LOAD( "s08e_u.114", 0x020000, 0x08000, CRC(765da5d7) SHA1(d489581bd12d7fca42570ee7a12d922be2528c1e) )
	ROM_LOAD( "s00e_u.107", 0x000000, 0x08000, CRC(558918cc) SHA1(7e61639ab4af88f888f4aa481dd01db7de3829da) )
	ROM_LOAD( "s16e_u.126", 0x020000, 0x08000, CRC(d24654cd) SHA1(88d3624f23c669dc902136c822b1f4732104c9c1) )

	ROM_REGION16_LE( 0x40000, "user1", 0 )
	ROM_LOAD16_BYTE( "3el_u101", 0x000000, 0x20000, CRC(130e1a18) SHA1(c31af5c5a403da588142ccbea79d3aa253ac6519) )
	ROM_LOAD16_BYTE( "3eh_u97",  0x000001, 0x20000, CRC(0fdcab16) SHA1(afc21747e1624f3ab87b289b5f4a498141062445) )

	ROM_REGION16_LE( 0xC0000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "38l_u124", 0x000000, 0x20000, CRC(4e084daa) SHA1(f65f51d8d7c6b46aa844b37b212dab11c786d856) )
	ROM_LOAD16_BYTE( "38h_u121", 0x000001, 0x20000, CRC(3628c8c1) SHA1(760eda076ec46af5b954548036da5230a5c86371) )

	ROM_LOAD16_BYTE( "3al_u130", 0x040000, 0x20000, CRC(8a5386e3) SHA1(361f6abdb88cf51d5ec5ce6882986296dd274d3b) )
	ROM_LOAD16_BYTE( "3ah_u133", 0x040001, 0x20000, CRC(7e674ac1) SHA1(81f1d87e62faf94a44aca7e41a32edf5c7c145ec) )

	ROM_LOAD16_BYTE( "3cl_u114", 0x080000, 0x20000, CRC(bc04b0e6) SHA1(d08fddd52f2c1a565a80f5d4ff8b07f1c5f01a01) )
	ROM_LOAD16_BYTE( "3ch_u108", 0x080001, 0x20000, CRC(7cb688af) SHA1(6be495ae0ed74739f62de65386810864c9ffaaee) )

	ROM_REGION( 0x08000, "audio", 0 )
	ROM_LOAD( "sound.u2", 0x000000, 0x08000, CRC(77190a90) SHA1(a36a5a8457cc1c325e6318b083e5e271e163f7cb) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "sound.u17", 0x000000, 0x40000, CRC(d033ef6c) SHA1(0404473c87b5b52e39ab3824b159a2d98159bbea) )

ROM_END

ROM_START( f15se21 )	// rev 2.1 02/04/91
	/* HST - Host PCB            (MPG DW-00011C-0011-01)    */
	ROM_REGION( 0x140000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "500.hst", 0x000001, 0x20000, CRC(6c26806d) SHA1(7cfd2b3b92b0fc6627c92a2013a317ca5abc66a0) )
	ROM_LOAD16_BYTE( "501.hst", 0x000000, 0x20000, CRC(81f02bf7) SHA1(09976746fe4d9c88bd8840f6e7addb09226aa54b) )
	ROM_LOAD16_BYTE( "502.hst", 0x040001, 0x20000, CRC(1eb945e5) SHA1(aba3ff038f2ca0f1200be5710073825ce80e3656) )
	ROM_LOAD16_BYTE( "503.hst", 0x040000, 0x20000, CRC(21fcb974) SHA1(56f78ce652e2bf432fbba8cda8c800f02dad84bb) )

	ROM_LOAD16_BYTE( "004.hst", 0x080001, 0x20000, CRC(81671ce1) SHA1(51ff641ccbc9dea640a62944910abe73d796b062) )
	ROM_LOAD16_BYTE( "005.hst", 0x080000, 0x20000, CRC(bdaa7db5) SHA1(52cd832cdd44e609e8cd269469b806e2cd27d63d) )
	ROM_LOAD16_BYTE( "006.hst", 0x0c0001, 0x20000, CRC(8eedef6d) SHA1(a1b5b53afc9ff092d86e7c7d4e357807fae3ad85) )
	ROM_LOAD16_BYTE( "007.hst", 0x0c0000, 0x20000, CRC(36e06cba) SHA1(5ffee5da6f475978be10fa5e1a2c24f00497ea5f) )
	ROM_LOAD16_BYTE( "008.hst", 0x100001, 0x20000, CRC(d96fd4e2) SHA1(001af758da437e955b4ee914eabeb9739ebc4454) )
	ROM_LOAD16_BYTE( "009.hst", 0x100000, 0x20000, CRC(33e3b473) SHA1(66deda79ba94f0ed722b399b3fc6062dcdd1a6c9) )

	/* Video Graphics PCB  (MPG DW-010-00003-001) */
//  ROM_REGION( 0x40000, "vgb", 0 )         /* TMS34010 dummy region */

	// TI ROM version 1.0a, 28-November-90
	ROM_REGION16_LE( 0x40000, "user1", 0 )
	ROM_LOAD16_BYTE( "001.vgb", 0x000000, 0x20000, CRC(810c142d) SHA1(d37e5ecd716dda65d43cec7bca524c59d3dc9803) )
	ROM_LOAD16_BYTE( "004.vgb", 0x000001, 0x20000, CRC(b69e1260) SHA1(1a2b69ea7c96b0293b24d87ea46bd4b1d4c56a66) )

	ROM_REGION16_LE( 0xC0000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "005.vgb", 0x000000, 0x20000, CRC(7b1852f0) SHA1(d21525e59b3112313ea9783ac3dd988a4c1d5f87) )
	ROM_LOAD16_BYTE( "006.vgb", 0x000001, 0x20000, CRC(9d031636) SHA1(b7c7b57d547f2ce2eeb97126e961f3b5f35823f7) )
	ROM_LOAD16_BYTE( "007.vgb", 0x040000, 0x20000, CRC(15326070) SHA1(ec4484d4515694742d3fd3b944f342f052463988) )
	ROM_LOAD16_BYTE( "008.vgb", 0x040001, 0x20000, CRC(ca0e86d8) SHA1(a7b4b02d100a7875d5a184cdb76d507e926d1ca3) )
	ROM_LOAD16_BYTE( "003.vgb", 0x080000, 0x20000, CRC(4d8e8f54) SHA1(d8a23b5fd00ab919dc6d63fc72824d1293073813) )
	ROM_LOAD16_BYTE( "002.vgb", 0x080001, 0x20000, CRC(f6488e31) SHA1(d2f9304cc59f5523007592ae76ddd56107cc29e8) )

	/*  DTH - Dr Math PCB         (MPG 010-00002-001) */
	ROM_REGION32_BE( 0x80000, "user2", 0 )
//  ROM_LOAD("drmath.rom", 0x00000, 0x40000, CRC(28dd9107) SHA1(3d571de81109e805cc0b133eefc3ea419e6a337b) )         // Cheat

	ROM_LOAD( "120.dth", 0x000000, 0x08000, CRC(5fb9836d) SHA1(d511aa9f02972a7f475c82c6f57d1f3fd4f118fa) )
	ROM_LOAD( "119.dth", 0x008000, 0x08000, CRC(b1c966e5) SHA1(9703bb1f9bdf6a779b59daebb39df2926727fa76) )
	ROM_LOAD( "121.dth", 0x000006, 0x08000, CRC(392e5c43) SHA1(455cf3bb3c16217e58d6eea51d8f49a5bed1955e) )
	ROM_LOAD( "118.dth", 0x000007, 0x08000, CRC(cc895c20) SHA1(140ef47536914fe1441778e759894c2cdd893276) )
	ROM_LOAD( "014.dth", 0x000000, 0x20000, CRC(5ca7713f) SHA1(ac7b9629684b99ecfb1945176b06eb6be284ba93) )
	ROM_LOAD( "015.dth", 0x000001, 0x20000, CRC(beae31bb) SHA1(1ab80a6b99eea6d5bf9b1bce58ecca13042c77a6) )
	ROM_LOAD( "016.dth", 0x000002, 0x20000, CRC(5db4f677) SHA1(25a6fe4c562e4fa4225aa4687dd41920b614e591) )
	ROM_LOAD( "017.dth", 0x000003, 0x20000, CRC(47f9a868) SHA1(7c8a9355893e4a3f3846fd05e0237ffd1404ffee) )
	ROM_LOAD( "122.dth", 0x000000, 0x08000, CRC(9d2032cf) SHA1(8430816756ea92bbe86b94eaa24a6071bf0ef879) )
	ROM_LOAD( "123.dth", 0x020000, 0x08000, CRC(54d5544f) SHA1(d039ee39991b947a7483111359ab245fc104e060) )
	ROM_LOAD( "124.dth", 0x000000, 0x08000, CRC(7be96646) SHA1(a6733f75c0404282d71e8c1a287546ef4d9d42ad) )
	ROM_LOAD( "125.dth", 0x020000, 0x08000, CRC(7718487c) SHA1(609106f55601f84095b64ce2484107779da89149) )


	/* SND - Sound PCB           (MPG 010-00018-002) */
	ROM_REGION( 0x08000, "audio", 0 )
	ROM_LOAD( "4-001.snd", 0x000000, 0x08000, CRC(705685a9) SHA1(311f7cac126a19e8bd555ebf31ff4ec4680ddfa4) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "3-001.snd", 0x000000, 0x40000, CRC(af84b635) SHA1(844e5987a66e9e3ab2d2fe05b93a4da3512776bb) )
ROM_END

ROM_START( f15se )	// rev 2.2 02/25/91
	/* HST - Host PCB            (MPG DW-00011C-0011-01)    */
	ROM_REGION( 0x140000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "host.u67", 0x000001, 0x20000, CRC(8f495ceb) SHA1(90998ad67e76928ed1a6cae56038b98d1aa2e7b0) )
	ROM_LOAD16_BYTE( "host.u91", 0x000000, 0x20000, CRC(dfae5ec3) SHA1(29306eed5047e39a0a2350e61ab7126a84cb710b) )
	ROM_LOAD16_BYTE( "host.u68", 0x040001, 0x20000, CRC(685fc355) SHA1(5bfe015a8deccb66e3317154d715f490f00ace74) )
	ROM_LOAD16_BYTE( "host.u92", 0x040000, 0x20000, CRC(8f7bb2eb) SHA1(1923d55d66da0fbc158b4f90bcc98c88955953ea) )

        ROM_LOAD16_BYTE( "004.hst", 0x080001, 0x20000, CRC(81671ce1) SHA1(51ff641ccbc9dea640a62944910abe73d796b062) )
	ROM_LOAD16_BYTE( "005.hst", 0x080000, 0x20000, CRC(bdaa7db5) SHA1(52cd832cdd44e609e8cd269469b806e2cd27d63d) )
	ROM_LOAD16_BYTE( "host.u70",0x0c0001, 0x20000, CRC(251e92d2) SHA1(a20279089af1f738ba912f90a4d048d4e58795fe)  )
	ROM_LOAD16_BYTE( "007.hst", 0x0c0000, 0x20000, CRC(36e06cba) SHA1(5ffee5da6f475978be10fa5e1a2c24f00497ea5f) )
	ROM_LOAD16_BYTE( "008.hst", 0x100001, 0x20000, CRC(d96fd4e2) SHA1(001af758da437e955b4ee914eabeb9739ebc4454) )
	ROM_LOAD16_BYTE( "009.hst", 0x100000, 0x20000, CRC(33e3b473) SHA1(66deda79ba94f0ed722b399b3fc6062dcdd1a6c9) )

	/* Video Graphics PCB  (MPG DW-010-00003-001) */
//  ROM_REGION( 0x40000, "vgb", 0 )         /* TMS34010 dummy region */

	// TI ROM version 1.0c, 28-November-90
	ROM_REGION16_LE( 0x40000, "user1", 0 )
	ROM_LOAD16_BYTE( "vgb_u101.bin", 0x000000, 0x20000, CRC(e99fac71) SHA1(98d1d2134fabc1bad637cbe42cbe9cdc20b32126) )
	ROM_LOAD16_BYTE( "vgb_u097.bin", 0x000001, 0x20000, CRC(78b9b7c7) SHA1(4bce993dd3aea126e3a9d42ee8c68b8ab47fdba7) )

	ROM_REGION16_LE( 0xC0000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "005.vgb", 0x000000, 0x20000, CRC(7b1852f0) SHA1(d21525e59b3112313ea9783ac3dd988a4c1d5f87) )
	ROM_LOAD16_BYTE( "006.vgb", 0x000001, 0x20000, CRC(9d031636) SHA1(b7c7b57d547f2ce2eeb97126e961f3b5f35823f7) )
	ROM_LOAD16_BYTE( "007.vgb", 0x040000, 0x20000, CRC(15326070) SHA1(ec4484d4515694742d3fd3b944f342f052463988) )
	ROM_LOAD16_BYTE( "008.vgb", 0x040001, 0x20000, CRC(ca0e86d8) SHA1(a7b4b02d100a7875d5a184cdb76d507e926d1ca3) )
	ROM_LOAD16_BYTE( "003.vgb", 0x080000, 0x20000, CRC(4d8e8f54) SHA1(d8a23b5fd00ab919dc6d63fc72824d1293073813) )
	ROM_LOAD16_BYTE( "002.vgb", 0x080001, 0x20000, CRC(f6488e31) SHA1(d2f9304cc59f5523007592ae76ddd56107cc29e8) )

	/*  DTH - Dr Math PCB         (MPG 010-00002-001) */
	ROM_REGION32_BE( 0x80000, "user2", 0 )
//  ROM_LOAD("drmath.rom", 0x00000, 0x40000, CRC(28dd9107) SHA1(3d571de81109e805cc0b133eefc3ea419e6a337b) )         // Cheat

	ROM_LOAD( "120.dth", 0x000000, 0x08000, CRC(5fb9836d) SHA1(d511aa9f02972a7f475c82c6f57d1f3fd4f118fa) )
	ROM_LOAD( "119.dth", 0x008000, 0x08000, CRC(b1c966e5) SHA1(9703bb1f9bdf6a779b59daebb39df2926727fa76) )
	ROM_LOAD( "121.dth", 0x000006, 0x08000, CRC(392e5c43) SHA1(455cf3bb3c16217e58d6eea51d8f49a5bed1955e) )
	ROM_LOAD( "118.dth", 0x000007, 0x08000, CRC(cc895c20) SHA1(140ef47536914fe1441778e759894c2cdd893276) )
	ROM_LOAD( "014.dth", 0x000000, 0x20000, CRC(5ca7713f) SHA1(ac7b9629684b99ecfb1945176b06eb6be284ba93) )
	ROM_LOAD( "015.dth", 0x000001, 0x20000, CRC(beae31bb) SHA1(1ab80a6b99eea6d5bf9b1bce58ecca13042c77a6) )
	ROM_LOAD( "016.dth", 0x000002, 0x20000, CRC(5db4f677) SHA1(25a6fe4c562e4fa4225aa4687dd41920b614e591) )
	ROM_LOAD( "017.dth", 0x000003, 0x20000, CRC(47f9a868) SHA1(7c8a9355893e4a3f3846fd05e0237ffd1404ffee) )
	ROM_LOAD( "122.dth", 0x000000, 0x08000, CRC(9d2032cf) SHA1(8430816756ea92bbe86b94eaa24a6071bf0ef879) )
	ROM_LOAD( "123.dth", 0x020000, 0x08000, CRC(54d5544f) SHA1(d039ee39991b947a7483111359ab245fc104e060) )
	ROM_LOAD( "124.dth", 0x000000, 0x08000, CRC(7be96646) SHA1(a6733f75c0404282d71e8c1a287546ef4d9d42ad) )
	ROM_LOAD( "125.dth", 0x020000, 0x08000, CRC(7718487c) SHA1(609106f55601f84095b64ce2484107779da89149) )


	/* SND - Sound PCB           (MPG 010-00018-002) */
	ROM_REGION( 0x08000, "audio", 0 )
	ROM_LOAD( "4-001.snd", 0x000000, 0x08000, CRC(705685a9) SHA1(311f7cac126a19e8bd555ebf31ff4ec4680ddfa4) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "3-001.snd", 0x000000, 0x40000, CRC(af84b635) SHA1(844e5987a66e9e3ab2d2fe05b93a4da3512776bb) )
ROM_END

GAME( 1991, f15se,    0,     micro3d, f15se, f15se, ROT0, "Microprose", "F-15 Strike Eagle (rev. 2.2 02/25/91)", GAME_NOT_WORKING )
GAME( 1991, f15se21 , f15se, micro3d, f15se, f15se21, ROT0, "Microprose", "F-15 Strike Eagle (rev. 2.1 02/04/91)", GAME_NOT_WORKING )
GAME( 1992, botss,    0, micro3d, botss, botss, ROT0, "Microprose", "Battle of the Solar System (rev. 1.1)", GAME_NOT_WORKING )
GAME( 1992, stankatk, 0, micro3d, stankatk, stankatk, ROT0, "Microprose", "Super Tank Attack (prototype rev. 4/21/92 )", GAME_NOT_WORKING )
