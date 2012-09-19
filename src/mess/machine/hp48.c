/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+

**********************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "cpu/saturn/saturn.h"

#include "machine/nvram.h"

#include "includes/hp48.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/


#define VERBOSE          0
#define VERBOSE_SERIAL   0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)
#define LOG_SERIAL(x)  do { if (VERBOSE_SERIAL) logerror x; } while (0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* state field in hp48_module */
#define HP48_MODULE_UNCONFIGURED 0
#define	HP48_MODULE_MASK_KNOWN   1
#define HP48_MODULE_CONFIGURED   2

/***************************************************************************
    GLOBAL VARIABLES & CONSTANTS
***************************************************************************/

/* current HP48 model */

#define HP48_G_SERIES ((state->m_model==HP48_G) || (state->m_model==HP48_GX) || (state->m_model==HP48_GP))
#define HP48_S_SERIES ((state->m_model==HP48_S) || (state->m_model==HP48_SX))
#define HP48_X_SERIES ((state->m_model==HP48_SX) || (state->m_model==HP48_GX))
#define HP48_GX_MODEL ((state->m_model==HP48_GX) || (state->m_model==HP48_GP))

/* OUT register from SATURN (actually 12-bit) */

/* keyboard interrupt */

/* from highest to lowest priority: HDW, NCE2, CE1, CE2, NCE3, NCE1 */

static const char *const hp48_module_names[6] =
{ "HDW (I/O)", "NCE2 (RAM)", "CE1", "CE2", "NCE3", "NCE1 (ROM)" };

/* values returned by C=ID */
static const UINT8 hp48_module_mask_id[6] = { 0x00, 0x03, 0x05, 0x07, 0x01, 0x00 };
static const UINT8 hp48_module_addr_id[6] = { 0x19, 0xf4, 0xf6, 0xf8, 0xf2, 0x00 };

/* RAM/ROM extensions, GX/SX only (each UINT8 stores one nibble)
   port1: SX/GX: 32/128 KB
   port2: SX:32/128KB, GX:128/512/4096 KB
 */


/* CRC state */

/* timers state */

#ifdef CHARDEV
#include "devices/chardev.h"
#endif


/***************************************************************************
    FUNCTIONS
***************************************************************************/

static void hp48_apply_modules(hp48_state *state);


static void hp48_pulse_irq( running_machine &machine, int irq_line)
{
	machine.device("maincpu")->execute().set_input_line(irq_line, ASSERT_LINE );
	machine.device("maincpu")->execute().set_input_line(irq_line, CLEAR_LINE );
}


/* ---------------- serial --------------------- */

#define RS232_DELAY attotime::from_usec( 300 )

/* end of receive event */
static TIMER_CALLBACK( hp48_rs232_byte_recv_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	LOG_SERIAL(( "%f hp48_rs232_byte_recv_cb: end of receive, data=%02x\n",
		     machine.time().as_double(), param ));

	state->m_io[0x14] = param & 0xf; /* receive zone */
	state->m_io[0x15] = param >> 4;
	state->m_io[0x11] &= ~2; /* clear byte receiving */
	state->m_io[0x11] |= 1;  /* set byte received */

	/* interrupt */
	if ( state->m_io[0x10] & 2 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}
}

/* outside world initiates a receive event */
void hp48_rs232_start_recv_byte( running_machine &machine, UINT8 data )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	LOG_SERIAL(( "%f hp48_rs232_start_recv_byte: start receiving, data=%02x\n",
		     machine.time().as_double(), data ));

	state->m_io[0x11] |= 2;  /* set byte receiving */

	/* interrupt */
	if ( state->m_io[0x10] & 1 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}

	/* schedule end of reception */
	machine.scheduler().timer_set( RS232_DELAY, FUNC(hp48_rs232_byte_recv_cb), data);
}


/* end of send event */
static TIMER_CALLBACK( hp48_rs232_byte_sent_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	//device_image_interface *xmodem = dynamic_cast<device_image_interface *>(machine.device("rs232_x"));
	//device_image_interface *kermit = dynamic_cast<device_image_interface *>(machine.device("rs232_k"));

	LOG_SERIAL(( "%f hp48_rs232_byte_sent_cb: end of send, data=%02x\n",
		     machine.time().as_double(), param ));

	state->m_io[0x12] &= ~3; /* clear byte sending and buffer full */

	/* interrupt */
	if ( state->m_io[0x10] & 4 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}

	/* protocol action */
	//if ( xmodem && xmodem->exists() ) xmodem_receive_byte( &xmodem->device(), param );
	//else if ( kermit && kermit->exists() ) kermit_receive_byte( &kermit->device(), param );
//#ifdef CHARDEV
//  else chardev_out( state->m_chardev, param );
//#endif
}

/* CPU initiates a send event */
static void hp48_rs232_send_byte( running_machine &machine )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	UINT8 data = HP48_IO_8(0x16); /* byte to send */

	LOG_SERIAL(( "%05x %f hp48_rs232_send_byte: start sending, data=%02x\n",
		     machine.device("maincpu")->safe_pcbase(), machine.time().as_double(), data ));

	/* set byte sending and send buffer full */
	state->m_io[0x12] |= 3;

	/* schedule transmission */
	machine.scheduler().timer_set( RS232_DELAY, FUNC(hp48_rs232_byte_sent_cb), data);
}


#ifdef CHARDEV

static TIMER_CALLBACK( hp48_chardev_byte_recv_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	UINT8 data = chardev_in( state->m_chardev );

	LOG_SERIAL(( "%f hp48_chardev_byte_recv_cb: end of receive, data=%02x\n",
		     machine.time().as_double(), data ));

	state->m_io[0x14] = data & 0xf; /* receive zone */
	state->m_io[0x15] = data >> 4;
	state->m_io[0x11] &= ~2; /* clear byte receiving */
	state->m_io[0x11] |= 1;  /* set byte received */

	/* interrupt */
	if ( state->m_io[0x10] & 2 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}
}

static void hp48_chardev_start_recv_byte( running_machine &machine, chardev_err status )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	if ( status != CHARDEV_OK ) return;

	LOG_SERIAL(( "%f hp48_chardev_start_recv_byte: start receiving\n",
		     machine.time().as_double() ));

	state->m_io[0x11] |= 2;  /* set byte receiving */

	/* interrupt */
	if ( state->m_io[0x10] & 1 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}

	/* schedule end of reception */
	machine.scheduler().timer_set( RS232_DELAY, FUNC(hp48_chardev_byte_recv_cb));
}

static void hp48_chardev_ready_to_send( running_machine &machine )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	state->m_io[0x12] &= ~3;

	/* interrupt */
	if ( state->m_io[0x10] & 4 )
	{
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}
}

static const chardev_interface hp48_chardev_iface =
{ hp48_chardev_start_recv_byte, hp48_chardev_ready_to_send };

#endif


/* ------ Saturn's IN / OUT registers ---------- */


/* CPU sets OUT register (keyboard + beeper) */
void hp48_reg_out( device_t *device, int out )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	LOG(( "%05x %f hp48_reg_out: %03x\n",
	      device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double(), out ));

	/* bits 0-8: keyboard lines */
	state->m_out = out & 0x1ff;

	/* bits 9-10: unused */

	/* bit 11: beeper */
	device->machine().device<dac_device>("dac")->write_unsigned8((out & 0x800) ? 0x80 : 00 );
}

static int hp48_get_in( running_machine &machine )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	int in = 0;

	/* regular keys */
	if ( (state->m_out >> 0) & 1 ) in |= machine.root_device().ioport( "LINE0" )->read();
	if ( (state->m_out >> 1) & 1 ) in |= machine.root_device().ioport( "LINE1" )->read();
	if ( (state->m_out >> 2) & 1 ) in |= machine.root_device().ioport( "LINE2" )->read();
	if ( (state->m_out >> 3) & 1 ) in |= machine.root_device().ioport( "LINE3" )->read();
	if ( (state->m_out >> 4) & 1 ) in |= machine.root_device().ioport( "LINE4" )->read();
	if ( (state->m_out >> 5) & 1 ) in |= machine.root_device().ioport( "LINE5" )->read();
	if ( (state->m_out >> 6) & 1 ) in |= machine.root_device().ioport( "LINE6" )->read();
	if ( (state->m_out >> 7) & 1 ) in |= machine.root_device().ioport( "LINE7" )->read();
	if ( (state->m_out >> 8) & 1 ) in |= machine.root_device().ioport( "LINE8" )->read();

	/* on key */
	in |= machine.root_device().ioport( "ON" )->read();

	return in;
}

/* CPU reads IN register (keyboard) */
int hp48_reg_in( device_t *device )
{
	int in = hp48_get_in(device->machine());
	LOG(( "%05x %f hp48_reg_in: %04x\n",
	      device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double(), in ));
	return in;
}

/* key detect */
static void hp48_update_kdn( running_machine &machine )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	int in = hp48_get_in( machine );

	/* interrupt on raising edge */
	if ( in && !state->m_kdn )
	{
		LOG(( "%f hp48_update_kdn: interrupt\n", machine.time().as_double() ));
		state->m_io[0x19] |= 8;                                              /* service request */
		hp48_pulse_irq( machine, SATURN_WAKEUP_LINE );
		hp48_pulse_irq( machine, SATURN_IRQ_LINE );
	}

	state->m_kdn = (in!=0);
}

/* periodic keyboard polling, generates an interrupt */
static TIMER_CALLBACK( hp48_kbd_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	/* NMI for ON key */
	if ( machine.root_device().ioport( "ON" )->read() )
	{
		LOG(( "%f hp48_kbd_cb: keyboard interrupt, on key\n",
		      machine.time().as_double() ));
		state->m_io[0x19] |= 8;                                          /* set service request */
		hp48_pulse_irq( machine, SATURN_WAKEUP_LINE );
		hp48_pulse_irq( machine, SATURN_NMI_LINE );
		return;
	}

	/* regular keys */
	hp48_update_kdn( machine );
}

/* RSI opcode */
void hp48_rsi( device_t *device )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	LOG(( "%05x %f hp48_rsi\n", device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double() ));

	/* enables interrupts on key repeat
       (normally, there is only one interrupt, when the key is pressed)
    */
	state->m_kdn = 0;
}


/* ------------- annonciators ------------ */

static void hp48_update_annunciators(hp48_state *state)
{
	/* bit 0: left shift
       bit 1: right shift
       bit 2: alpha
       bit 3: alert
       bit 4: busy
       bit 5: transmit
       bit 7: master enable
    */
	int markers = HP48_IO_8(0xb);
	output_set_value( "lshift0",   (markers & 0x81) == 0x81 );
	output_set_value( "rshift0",   (markers & 0x82) == 0x82 );
	output_set_value( "alpha0",    (markers & 0x84) == 0x84 );
	output_set_value( "alert0",    (markers & 0x88) == 0x88 );
	output_set_value( "busy0",     (markers & 0x90) == 0x90 );
	output_set_value( "transmit0", (markers & 0xb0) == 0xb0 );
}


/* ------------- I/O registers ----------- */

/* Some part of the I/O registers are simple r/w registers. We store them in hp48_io.
   Special cases are registers that:
   - have a different meaning on read and write
   - perform some action on read / write
 */

static WRITE8_HANDLER ( hp48_io_w )
{
	hp48_state *state = space.machine().driver_data<hp48_state>();
	LOG(( "%05x %f hp48_io_w: off=%02x data=%x\n",
	      space.device().safe_pcbase(), space.machine().time().as_double(), offset, data ));

	switch( offset )
	{

	/* CRC register */
	case 0x04: state->m_crc = (state->m_crc & 0xfff0) | data; break;
	case 0x05: state->m_crc = (state->m_crc & 0xff0f) | (data << 4); break;
	case 0x06: state->m_crc = (state->m_crc & 0xf0ff) | (data << 8); break;
	case 0x07: state->m_crc = (state->m_crc & 0x0fff) | (data << 12); break;

	/* annunciators */
	case 0x0b:
	case 0x0c:
		state->m_io[offset] = data;
		hp48_update_annunciators(state);
		break;

	/* cntrl ROM */
	case 0x29:
	{
		int old_cntrl = state->m_io[offset] & 8;
		state->m_io[offset] = data;
		if ( old_cntrl != (data & 8) )
		{
			hp48_apply_modules(state);
		}
		break;
	}

	/* timers */
	case 0x37: state->m_timer1 = data; break;
	case 0x38: state->m_timer2 = (state->m_timer2 & 0xfffffff0) | data; break;
	case 0x39: state->m_timer2 = (state->m_timer2 & 0xffffff0f) | (data << 4); break;
	case 0x3a: state->m_timer2 = (state->m_timer2 & 0xfffff0ff) | (data << 8); break;
	case 0x3b: state->m_timer2 = (state->m_timer2 & 0xffff0fff) | (data << 12); break;
	case 0x3c: state->m_timer2 = (state->m_timer2 & 0xfff0ffff) | (data << 16); break;
	case 0x3d: state->m_timer2 = (state->m_timer2 & 0xff0fffff) | (data << 20); break;
	case 0x3e: state->m_timer2 = (state->m_timer2 & 0xf0ffffff) | (data << 24); break;
	case 0x3f: state->m_timer2 = (state->m_timer2 & 0x0fffffff) | (data << 28); break;

	/* cards */
	case 0x0e:
		LOG(( "%05x: card control write %02x\n", space.device().safe_pcbase(), data ));

		/* bit 0: software interrupt */
		if ( data & 1 )
		{
			LOG(( "%f hp48_io_w: software interrupt requested\n",
			      space.machine().time().as_double() ));
			hp48_pulse_irq( space.machine(), SATURN_IRQ_LINE );
			data &= ~1;
		}

		/* XXX not implemented
            bit 1: card test?
         */

		state->m_io[0x0e] = data;
		break;

	case 0x0f:
		LOG(( "%05x: card info write %02x\n", space.device().safe_pcbase(), data ));
		state->m_io[0x0f] = data;
		break;

	/* serial */
	case 0x13:
		state->m_io[0x11] &= ~4; /* clear error status */
		break;
	case 0x16:
		/* first nibble of sent data */
		state->m_io[offset] = data;
		break;
	case 0x17:
		/* second nibble of sent data */
		state->m_io[offset] = data;
		hp48_rs232_send_byte(space.machine());
		break;

	/* XXX not implemented:

       - 0x0d: RS232c speed:
          bits 0-2: speed
                000 = 1200 bauds
            010 = 2400 bauds
            100 = 4800 bauds
            110 = 9600 bauds
              bit 3: ?

           - 0x1a: I/R input
              bit 0: irq
          bit 1: irq enable
          bit 2: 1=RS232c mode 0=direct mode
          bit 3: receiving

       - 0x1c: I/R output control
          bit 0: buffer full
          bit 1: transmitting
          bit 2: irq enable on buffer empty
          bit 3: led on (direct mode)

       - 0x1d: I/R output buffer
    */

	default: state->m_io[offset] = data;
	}

}


static READ8_HANDLER ( hp48_io_r )
{
	hp48_state *state = space.machine().driver_data<hp48_state>();
	UINT8 data = 0;

	switch( offset )
	{

	/* CRC register */
	case 0x04: data = state->m_crc & 0xf; break;
	case 0x05: data = (state->m_crc >> 4) & 0xf; break;
	case 0x06: data = (state->m_crc >> 8) & 0xf; break;
	case 0x07: data = (state->m_crc >> 12) & 0xf; break;

	/* battery test */
	case 0x08:
		data = 0;
		if ( state->m_io[0x9] & 8 ) /* test enable */
		{
			/* XXX not implemented:
               bit 3: battery in port 2
               bit 2: battery in port 1
             */
			switch ( state->ioport( "BATTERY" )->read() )
			{
			case 1: data = 2; break; /* low */
			case 2: data = 3; break; /* low | critical */
			}
		}
		break;

	/* remaining lines in main bitmap */
	case 0x28:
	case 0x29:
	{
		int last_line = HP48_IO_8(0x28) & 0x3f; /* last line of main bitmap before menu */
		int cur_line = space.machine().primary_screen->vpos();
		if ( last_line <= 1 ) last_line = 0x3f;
		data = ( cur_line >= 0 && cur_line <= last_line ) ? last_line - cur_line : 0;
		if ( offset == 0x29 )
		{
			data >>= 4;
			data |= HP48_IO_4(0x29) & 0xc;
		}
		else
		{
			data &= 0xf;
		}
		break;
	}

	/* timers */
	case 0x37: data = state->m_timer1; break;
	case 0x38: data = state->m_timer2 & 0xf; break;
	case 0x39: data = (state->m_timer2 >> 4) & 0xf; break;
	case 0x3a: data = (state->m_timer2 >> 8) & 0xf; break;
	case 0x3b: data = (state->m_timer2 >> 12) & 0xf; break;
	case 0x3c: data = (state->m_timer2 >> 16) & 0xf; break;
	case 0x3d: data = (state->m_timer2 >> 20) & 0xf; break;
	case 0x3e: data = (state->m_timer2 >> 24) & 0xf; break;
	case 0x3f: data = (state->m_timer2 >> 28) & 0xf; break;

	/* serial */
	case 0x15:
	{
		/* second nibble of received data */

		//device_image_interface *xmodem = dynamic_cast<device_image_interface *>(space.machine().device("rs232_x"));
		//device_image_interface *kermit = dynamic_cast<device_image_interface *>(space.machine().device("rs232_k"));

		state->m_io[0x11] &= ~1;  /* clear byte received */
		data = state->m_io[offset];

		/* protocol action */
		//if ( xmodem && xmodem->exists() ) xmodem_byte_transmitted( &xmodem->device() );
		//else if ( kermit && kermit->exists() ) kermit_byte_transmitted( &kermit->device() );
		break;
	}

	/* cards */
	case 0x0e: /* detection */
		data = state->m_io[0x0e];
		LOG(( "%05x: card control read %02x\n", space.device().safe_pcbase(), data ));
		break;
	case 0x0f: /* card info */
		data = 0;
		if ( HP48_G_SERIES )
		{
			if ( state->m_port_size[1] ) data |= 1;
			if ( state->m_port_size[0] ) data |= 2;
			if ( state->m_port_size[1] && state->m_port_write[1] ) data |= 4;
			if ( state->m_port_size[0] && state->m_port_write[0] ) data |= 8;
		}
		else
		{
			if ( state->m_port_size[0] ) data |= 1;
			if ( state->m_port_size[1] ) data |= 2;
			if ( state->m_port_size[0] && state->m_port_write[0] ) data |= 4;
			if ( state->m_port_size[1] && state->m_port_write[1] ) data |= 8;
		}
		LOG(( "%05x: card info read %02x\n", space.device().safe_pcbase(), data ));
		break;


	default: data = state->m_io[offset];
	}

	LOG(( "%05x %f hp48_io_r: off=%02x data=%x\n",
	      space.device().safe_pcbase(), space.machine().time().as_double(), offset, data ));
	return data;
}


/* ---------- GX's bank switcher --------- */

static READ8_HANDLER ( hp48_bank_r )
{
	hp48_state *state = space.machine().driver_data<hp48_state>();
	/* bit 0: ignored, bits 2-5: bank number, bit 6: enable */
	offset &= 0x7e;
	if ( state->m_bank_switch != offset )
	{
		LOG(( "%05x %f hp48_bank_r: off=%03x\n", space.device().safe_pcbase(), space.machine().time().as_double(), offset ));
		state->m_bank_switch = offset;
		hp48_apply_modules(state);
	}
	return 0;
}


/* ---------------- timers --------------- */

static TIMER_CALLBACK( hp48_timer1_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	if ( !(state->m_io[0x2f] & 1) ) return; /* timer enable */

	state->m_timer1 = (state->m_timer1 - 1) & 0xf;

	/* wake-up on carry */
	if ( (state->m_io[0x2e] & 4) && (state->m_timer1 == 0xf) )
	{
		LOG(( "wake-up on timer1\n" ));
		state->m_io[0x2e] |= 8;                                      /* set service request */
		state->m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( machine, SATURN_WAKEUP_LINE );
	}
	/* interrupt on carry */
	if ( (state->m_io[0x2e] & 2) && (state->m_timer1 == 0xf) )
	{
		LOG(( "generate timer1 interrupt\n" ));
		state->m_io[0x2e] |= 8; /* set service request */
		state->m_io[0x18] |= 4; /* set service request */
		hp48_pulse_irq( machine, SATURN_NMI_LINE );
	}
}

static TIMER_CALLBACK( hp48_timer2_cb )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	if ( !(state->m_io[0x2f] & 1) ) return; /* timer enable */

	state->m_timer2 = (state->m_timer2 - 1) & 0xffffffff;

	/* wake-up on carry */
	if ( (state->m_io[0x2f] & 4) && (state->m_timer2 == 0xffffffff) )
	{
		LOG(( "wake-up on timer2\n" ));
		state->m_io[0x2f] |= 8;                                      /* set service request */
		state->m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( machine, SATURN_WAKEUP_LINE );
	}
	/* interrupt on carry */
	if ( (state->m_io[0x2f] & 2) && (state->m_timer2 == 0xffffffff) )
	{
		LOG(( "generate timer2 interrupt\n" ));
		state->m_io[0x2f] |= 8;                                      /* set service request */
		state->m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( machine, SATURN_NMI_LINE );
	}
}




/* --------- memory controller ----------- */

/*
   Clark (S series) and York (G series) CPUs have 6 daisy-chained modules


   <-- highest --------- priority ------------- lowest -->

   CPU ---------------------------------------------------
            |      |      |          |          |        |
           HDW    NCE2   CE1        CE2        NCE3     NCE1


   However, controller usage is different in both series:


      controller     S series        G series
                    (Clark CPU)     (York CPU)

         HDW         I/O RAM          I/O RAM
        NCE2           RAM              RAM
         CE1           port1        bank switcher
         CE2           port2           port1
        NCE3          unused           port2
        NCE1           ROM              ROM


   - NCE1 (ROM) cannot be configured, it is always visible at addresses
   00000-7ffff not covered by higher priority modules.

   - only the address of HDW (I/O) can be configured, its size is constant
   (64 nibbles)

   - other modules can have their address & size set

 */


/* remap all modules according to hp48_modules */
static void hp48_apply_modules(hp48_state *state)
{
	int i;
	int nce2_enable = 1;
	address_space& space = state->machine().device("maincpu")->memory().space(AS_PROGRAM);

	state->m_io_addr = 0x100000;

	if ( HP48_G_SERIES )
	{
		/* port 2 bank switch */
		if ( state->m_port_size[1] > 0 )
		{
			int off = (state->m_bank_switch << 16) % state->m_port_size[1];
			state->m_modules[4].data = state->m_port_data[1] + off;
		}

		/* ROM A19 (hi 256 KB) / NCE2 (port 2) control switch */
		if ( state->m_io[0x29] & 8 )
		{
			/* A19 */
			state->m_modules[5].off_mask = 0xfffff;
			nce2_enable = 0;
		}
		else
		{
			/* NCE2 */
			state->m_modules[5].off_mask = 0x7ffff;
			nce2_enable = state->m_bank_switch >> 6;
		}
	}

	/* S series ROM mapping compatibility */
	if ( HP48_S_SERIES || !(state->m_io[0x29] & 8) )
	{
		state->m_modules[5].off_mask = 0x7ffff;
	}
	else
	{
		state->m_modules[5].off_mask = 0xfffff;
	}

	/* from lowest to highest priority */
	for ( i = 5; i >= 0; i-- )
	{
		UINT32 select_mask = state->m_modules[i].mask;
		UINT32 nselect_mask = ~select_mask & 0xfffff;
		UINT32 base = state->m_modules[i].base;
		UINT32 off_mask = state->m_modules[i].off_mask;
		UINT32 mirror = nselect_mask & ~off_mask;
		UINT32 end = base + (off_mask & nselect_mask);
		char bank[10];
		sprintf(bank,"bank%d",i);

		if ( state->m_modules[i].state != HP48_MODULE_CONFIGURED ) continue;

		if ( (i == 4) && !nce2_enable ) continue;

		/* our code assumes that the 20-bit select_mask is all 1s followed by all 0s */
		if ( nselect_mask & (nselect_mask+1) )
		{
			logerror( "hp48_apply_modules: invalid mask %05x for module %s\n",
				  select_mask, hp48_module_names[i] );
			continue;
		}

		if (state->m_modules[i].data)
			space.install_read_bank( base, end, 0, mirror, bank );
		else
		{
			if (state->m_modules[i].read != NULL)
				space.install_legacy_read_handler( base, end, 0, mirror, state->m_modules[i].read, state->m_modules[i].read_name);
		}

		if (state->m_modules[i].isnop)
			space.nop_write(base, end, 0, mirror);
		else
		{
			if (state->m_modules[i].data)
				space.install_write_bank( base, end, 0, mirror, bank );
			else
			{
				if (state->m_modules[i].write != NULL)
					space.install_legacy_write_handler( base, end, 0, mirror, state->m_modules[i].write, state->m_modules[i].write_name );
			}
		}

		LOG(( "hp48_apply_modules: module %s configured at %05x-%05x, mirror %05x\n",
		      hp48_module_names[i], base, end, mirror ));

		if ( state->m_modules[i].data )
		{
			state->membank( bank )->set_base( state->m_modules[i].data );
		}

		if ( i == 0 )
		{
			state->m_io_addr = base;
		}
	}
}


/* reset the configuration */
static void hp48_reset_modules( running_machine &machine )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	int i;
	/* fixed size for HDW */
	state->m_modules[0].state = HP48_MODULE_MASK_KNOWN;
	state->m_modules[0].mask = 0xfffc0;
	/* unconfigure NCE2, CE1, CE2, NCE3 */
	for ( i = 1; i < 5; i++ )
	{
		state->m_modules[i].state = HP48_MODULE_UNCONFIGURED;
	}

	/* fixed configuration for NCE1 */
	state->m_modules[5].state = HP48_MODULE_CONFIGURED;
	state->m_modules[5].base = 0;
	state->m_modules[5].mask = 0;

	hp48_apply_modules(state);
}


/* RESET opcode */
void hp48_mem_reset( device_t *device )
{
	LOG(( "%05x %f hp48_mem_reset\n", device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double() ));
	hp48_reset_modules(device->machine());
}


/* CONFIG opcode */
void hp48_mem_config( device_t *device, int v )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	int i;

	LOG(( "%05x %f hp48_mem_config: %05x\n", device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double(), v ));

	/* find the highest priority unconfigured module (except non-configurable NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... first call sets the address mask */
		if ( state->m_modules[i].state == HP48_MODULE_UNCONFIGURED )
		{
			state->m_modules[i].mask = v & 0xff000;
			state->m_modules[i].state = HP48_MODULE_MASK_KNOWN;
			break;
		}

		/* ... second call sets the base address */
		if ( state->m_modules[i].state == HP48_MODULE_MASK_KNOWN )
		{
			state->m_modules[i].base = v & state->m_modules[i].mask;
			state->m_modules[i].state = HP48_MODULE_CONFIGURED;
			LOG(( "hp48_mem_config: module %s configured base=%05x, mask=%05x\n",
			      hp48_module_names[i], state->m_modules[i].base, state->m_modules[i].mask ));
			hp48_apply_modules(state);
			break;
		}
	}
}


/* UNCFG opcode */
void hp48_mem_unconfig( device_t *device, int v )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	int i;
	LOG(( "%05x %f hp48_mem_unconfig: %05x\n", device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double(), v ));

	/* find the highest priority fully configured module at address v (except NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... and unconfigure it */
		if ( state->m_modules[i].state == HP48_MODULE_CONFIGURED &&
		     (state->m_modules[i].base == (v & state->m_modules[i].mask)) )
		{
			state->m_modules[i].state = i> 0 ? HP48_MODULE_UNCONFIGURED : HP48_MODULE_MASK_KNOWN;
			LOG(( "hp48_mem_unconfig: module %s\n", hp48_module_names[i] ));
			hp48_apply_modules(state);
			break;
		}
	}
}


/* C=ID opcode */
int  hp48_mem_id( device_t *device )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	int i;
	int data = 0; /* 0 = everything is configured */

	/* find the highest priority unconfigured module (except NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... mask need to be configured */
		if ( state->m_modules[i].state == HP48_MODULE_UNCONFIGURED )
		{
			data = hp48_module_mask_id[i] | (state->m_modules[i].mask & ~0xff);
			break;
		}

		/* ... address need to be configured */
		if ( state->m_modules[i].state == HP48_MODULE_MASK_KNOWN )
		{
			data = hp48_module_addr_id[i] | (state->m_modules[i].base & ~0x3f);
			break;
		}
	}

	LOG(( "%05x %f hp48_mem_id = %02x\n",
	      device->machine().device("maincpu")->safe_pcbase(), device->machine().time().as_double(), data ));

	return data; /* everything is configured */
}



/* --------- CRC ---------- */

/* each memory read by the CPU updates the internal CRC state */
void hp48_mem_crc( device_t *device, int addr, int data )
{
	hp48_state *state = device->machine().driver_data<hp48_state>();
	/* no CRC for I/O RAM */
	if ( addr >= state->m_io_addr && addr < state->m_io_addr + 0x40 ) return;

	state->m_crc = (state->m_crc >> 4) ^ (((state->m_crc ^ data) & 0xf) * 0x1081);
}



/* ------ utilities ------- */


/* decodes size bytes into 2*size nibbles (least significant first) */
static void hp48_decode_nibble( UINT8* dst, UINT8* src, int size )
{
	int i;
	for ( i=size-1; i >= 0; i-- )
	{
		dst[2*i+1] = src[i] >> 4;
		dst[2*i] = src[i] & 0xf;
	}
}

/* inverse of hp48_decode_nibble  */
static void hp48_encode_nibble( UINT8* dst, UINT8* src, int size )
{
	int i;
	for ( i=0; i < size; i++ )
	{
		dst[i] = (src[2*i] & 0xf) | (src[2*i+1] << 4);
	}
}



/* ----- card images ------ */

/* port information configurations */
const struct hp48_port_interface hp48sx_port1_config = { 0, 2,    128*1024 };
const struct hp48_port_interface hp48sx_port2_config = { 1, 3,    128*1024 };
const struct hp48_port_interface hp48gx_port1_config = { 0, 3,    128*1024 };
const struct hp48_port_interface hp48gx_port2_config = { 1, 4, 4*1024*1024 };

const device_type HP48_PORT = &device_creator<hp48_port_image_device>;

/* helper for load and create */
void hp48_port_image_device::hp48_fill_port()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	struct hp48_port_interface* conf = (struct hp48_port_interface*) static_config();
	int size = state->m_port_size[conf->port];
	LOG(( "hp48_fill_port: %s module=%i size=%i rw=%i\n", tag(), conf->module, size, state->m_port_write[conf->port] ));
	state->m_port_data[conf->port] = (UINT8*)malloc( 2 * size );
	memset( state->m_port_data[conf->port], 0, 2 * size );
	state->m_modules[conf->module].off_mask = 2 * (( size > 128 * 1024 ) ? 128 * 1024 : size) - 1;
	state->m_modules[conf->module].read     = NULL;
	state->m_modules[conf->module].write    = NULL;
	state->m_modules[conf->module].isnop    = 0;
	if (state->m_port_write[conf->port]) {
		state->m_modules[conf->module].isnop    = 1;
	}
	state->m_modules[conf->module].data     = state->m_port_data[conf->port];
	hp48_apply_modules(state);
}

/* helper for start and unload */
void hp48_port_image_device::hp48_unfill_port()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	struct hp48_port_interface* conf = (struct hp48_port_interface*) static_config();
	state->m_modules[conf->module].off_mask = 0x00fff;  /* 2 KB */
	state->m_modules[conf->module].read     = NULL;
	state->m_modules[conf->module].write    = NULL;
	state->m_modules[conf->module].data     = NULL;
}


bool hp48_port_image_device::call_load()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	struct hp48_port_interface* conf = (struct hp48_port_interface*)static_config();
	int size = length();
	if ( size == 0 ) size = conf->max_size; /* default size */

	/* check size */
	if ( (size < 32*1024) || (size > conf->max_size) || (size & (size-1)) )
	{
		logerror( "hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, conf->max_size );
		return IMAGE_INIT_FAIL;
	}

	state->m_port_size[conf->port] = size;
	state->m_port_write[conf->port] = !is_readonly();
	hp48_fill_port( );
	fread(state->m_port_data[conf->port], state->m_port_size[conf->port] );
	hp48_decode_nibble( state->m_port_data[conf->port], state->m_port_data[conf->port], state->m_port_size[conf->port] );
	return IMAGE_INIT_PASS;
}

bool hp48_port_image_device::call_create(int format_type, option_resolution *format_options)
{
	hp48_state *state = machine().driver_data<hp48_state>();
	struct hp48_port_interface* conf = (struct hp48_port_interface*) static_config();
	int size = conf->max_size;
	/* XXX defaults to max_size; get user-specified size instead */

	/* check size */
	/* size must be a power of 2 between 32K and max_size */
	if ( (size < 32*1024) || (size > conf->max_size) || (size & (size-1)) )
	{
		logerror( "hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, conf->max_size );
		return IMAGE_INIT_FAIL;
	}

	state->m_port_size[conf->port] = size;
	state->m_port_write[conf->port] = 1;
	hp48_fill_port();
	return IMAGE_INIT_PASS;
}

void hp48_port_image_device::call_unload()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	struct hp48_port_interface* conf = (struct hp48_port_interface*) static_config();
	LOG(( "hp48_port image unload: %s size=%i rw=%i\n",
	      tag(), state->m_port_size[conf->port] ,state->m_port_write[conf->port] ));
	if ( state->m_port_write[conf->port] )
	{
		hp48_encode_nibble( state->m_port_data[conf->port], state->m_port_data[conf->port], state->m_port_size[conf->port] );
		fseek( 0, SEEK_SET );
		fwrite( state->m_port_data[conf->port], state->m_port_size[conf->port] );
	}
	free( state->m_port_data[conf->port] );
	hp48_unfill_port();
	hp48_apply_modules(state);
}

void hp48_port_image_device::device_start()
{
	hp48_unfill_port();
}

hp48_port_image_device::hp48_port_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, HP48_PORT, "HP48 memory card", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{
}

void hp48_port_image_device::device_config_complete()
{
	update_names(HP48_PORT, "port", "p");
}

/***************************************************************************
    MACHINES
***************************************************************************/

DRIVER_INIT_MEMBER(hp48_state,hp48)
{
	int i;
	LOG(( "hp48: driver init called\n" ));
	for ( i = 0; i < 6; i++ )
	{
		m_modules[i].off_mask = 0x00fff;  /* 2 KB */
		m_modules[i].read     = NULL;
		m_modules[i].write    = NULL;
		m_modules[i].data     = NULL;
	}
	m_port_size[0] = 0;
	m_port_size[1] = 0;
}

void hp48_state::machine_reset()
{
	LOG(( "hp48: machine reset called\n" ));
	hp48_reset_modules( machine() );
	hp48_update_annunciators(this);
}

static void hp48_machine_start( running_machine &machine, hp48_models model )
{
	hp48_state *state = machine.driver_data<hp48_state>();
	UINT8* rom, *ram;
	int ram_size, rom_size, i;

	LOG(( "hp48_machine_start: model %i\n", model ));

	state->m_model = model;

	/* internal RAM */
	ram_size = HP48_GX_MODEL ? (128 * 1024) : (32 * 1024);

	ram = auto_alloc_array(machine, UINT8, 2 * ram_size);
	machine.device<nvram_device>("nvram")->set_base(ram, 2 * ram_size);


	/* ROM load */
	rom_size = HP48_S_SERIES ? (256 * 1024) : (512 * 1024);
	rom = auto_alloc_array(machine, UINT8, 2 * rom_size);
	hp48_decode_nibble( rom, state->memregion( "maincpu" )->base(), rom_size );

	/* init state */
	memset( ram, 0, 2 * ram_size );
	memset( state->m_io, 0, sizeof( state->m_io ) );
	state->m_out = 0;
	state->m_kdn = 0;
	state->m_crc = 0;
	state->m_timer1 = 0;
	state->m_timer2 = 0;
	state->m_bank_switch = 0;

	/* static module configuration */
	memset(state->m_modules,0,sizeof(state->m_modules)); // to put all on 0
	/* I/O RAM */
	state->m_modules[0].off_mask = 0x0003f;  /* 32 B */
	state->m_modules[0].read     = hp48_io_r;
	state->m_modules[0].read_name     = "hp48_io_r";
	state->m_modules[0].write    = hp48_io_w;
	state->m_modules[0].write_name    = "hp48_io_w";

	/* internal RAM */
	state->m_modules[1].off_mask = 2 * ram_size - 1;
	state->m_modules[1].read	 = NULL;
	state->m_modules[1].write	 = NULL;
	state->m_modules[1].data     = ram;

	if ( HP48_G_SERIES )
	{
		/* bank switcher */
		state->m_modules[2].off_mask = 0x00fff;  /* 2 KB */
		state->m_modules[2].read     = hp48_bank_r;
		state->m_modules[2].read_name    = "hp48_bank_r";
		state->m_modules[2].write    = NULL;
	}

	/* ROM */
	state->m_modules[5].off_mask = 2 * rom_size - 1;
	state->m_modules[5].read	 = NULL;
	state->m_modules[5].write    = NULL;
	state->m_modules[5].isnop    = 1;
	state->m_modules[5].data     = rom;

	/* timers */
	machine.scheduler().timer_pulse(attotime::from_hz( 16 ), FUNC(hp48_timer1_cb));
	machine.scheduler().timer_pulse(attotime::from_hz( 8192 ), FUNC(hp48_timer2_cb));

	/* 1ms keyboard polling */
	machine.scheduler().timer_pulse(attotime::from_msec( 1 ), FUNC(hp48_kbd_cb));

	/* save state */
	state->save_item(NAME(state->m_out) );
	state->save_item(NAME(state->m_kdn) );
	state->save_item(NAME(state->m_io_addr) );
	state->save_item(NAME(state->m_crc) );
	state->save_item(NAME(state->m_timer1) );
	state->save_item(NAME(state->m_timer2) );
	state->save_item(NAME(state->m_bank_switch) );
	for ( i = 0; i < 6; i++ )
	{
		state_save_register_item(machine, "globals", NULL, i, state->m_modules[i].state );
		state_save_register_item(machine, "globals", NULL, i, state->m_modules[i].base );
		state_save_register_item(machine, "globals", NULL, i, state->m_modules[i].mask );
	}
	state->save_item(NAME(state->m_io) );
	//state_save_register_global_pointer(machine,  machine.generic.nvram.u8, machine.generic.nvram_size );

	machine.save().register_postload( save_prepost_delegate(FUNC(hp48_update_annunciators), state ));
	machine.save().register_postload( save_prepost_delegate(FUNC(hp48_apply_modules), state ));

#ifdef CHARDEV
	/* direct I/O */
	state->m_chardev = chardev_open_pty( machine, &hp48_chardev_iface );
#endif
}


MACHINE_START_MEMBER(hp48_state,hp48s)
{
	hp48_machine_start( machine(), HP48_S );
}


MACHINE_START_MEMBER(hp48_state,hp48sx)
{
	hp48_machine_start( machine(), HP48_SX );
}


MACHINE_START_MEMBER(hp48_state,hp48g)
{
	hp48_machine_start( machine(), HP48_G );
}

MACHINE_START_MEMBER(hp48_state,hp48gx)
{
	hp48_machine_start( machine(), HP48_GX );
}

MACHINE_START_MEMBER(hp48_state,hp48gp)
{
	hp48_machine_start( machine(), HP48_GP );
}
