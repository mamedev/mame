// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+ and HP49 G

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
#define HP48_MODULE_MASK_KNOWN   1
#define HP48_MODULE_CONFIGURED   2

/***************************************************************************
    GLOBAL VARIABLES & CONSTANTS
***************************************************************************/

/* current HP48 model */

#define HP48_G_SERIES ((m_model==HP48_G) || (m_model==HP48_GX) || (m_model==HP48_GP))
#define HP48_S_SERIES ((m_model==HP48_S) || (m_model==HP48_SX))
#define HP48_X_SERIES ((m_model==HP48_SX) || (m_model==HP48_GX))
#define HP48_GX_MODEL ((m_model==HP48_GX) || (m_model==HP48_GP))
#define HP49_G_MODEL  ((m_model==HP49_G))

static const char *const hp48_module_names[6] =
{ "HDW (I/O)", "NCE2 (RAM)", "CE1", "CE2", "NCE3", "NCE1 (ROM)" };

/* values returned by C=ID */
static const UINT8 hp48_module_mask_id[6] = { 0x00, 0x03, 0x05, 0x07, 0x01, 0x00 };
static const UINT8 hp48_module_addr_id[6] = { 0x19, 0xf4, 0xf6, 0xf8, 0xf2, 0x00 };





/***************************************************************************
    FUNCTIONS
***************************************************************************/

void hp48_state::hp48_pulse_irq( int irq_line)
{
	m_maincpu->set_input_line(irq_line, ASSERT_LINE );
	m_maincpu->set_input_line(irq_line, CLEAR_LINE );
}


/* ---------------- serial --------------------- */

#define RS232_DELAY attotime::from_usec( 300 )

/* end of receive event */
TIMER_CALLBACK_MEMBER(hp48_state::hp48_rs232_byte_recv_cb)
{
	LOG_SERIAL(( "%f hp48_rs232_byte_recv_cb: end of receive, data=%02x\n",
				machine().time().as_double(), param ));

	m_io[0x14] = param & 0xf; /* receive zone */
	m_io[0x15] = param >> 4;
	m_io[0x11] &= ~2; /* clear byte receiving */
	m_io[0x11] |= 1;  /* set byte received */

	/* interrupt */
	if ( m_io[0x10] & 2 )
	{
		hp48_pulse_irq( SATURN_IRQ_LINE );
	}
}

/* outside world initiates a receive event */
void hp48_state::hp48_rs232_start_recv_byte( UINT8 data )
{
	LOG_SERIAL(( "%f hp48_rs232_start_recv_byte: start receiving, data=%02x\n",
				machine().time().as_double(), data ));

	m_io[0x11] |= 2;  /* set byte receiving */

	/* interrupt */
	if ( m_io[0x10] & 1 )
	{
		hp48_pulse_irq( SATURN_IRQ_LINE );
	}

	/* schedule end of reception */
	machine().scheduler().timer_set( RS232_DELAY, timer_expired_delegate(FUNC(hp48_state::hp48_rs232_byte_recv_cb),this), data);
}


/* end of send event */
TIMER_CALLBACK_MEMBER(hp48_state::hp48_rs232_byte_sent_cb)
{
	//device_image_interface *xmodem = dynamic_cast<device_image_interface *>(machine().device("rs232_x"));
	//device_image_interface *kermit = dynamic_cast<device_image_interface *>(machine().device("rs232_k"));

	LOG_SERIAL(( "%f hp48_rs232_byte_sent_cb: end of send, data=%02x\n",
				machine().time().as_double(), param ));

	m_io[0x12] &= ~3; /* clear byte sending and buffer full */

	/* interrupt */
	if ( m_io[0x10] & 4 )
	{
		hp48_pulse_irq( SATURN_IRQ_LINE );
	}

	/* protocol action */
	//if ( xmodem && xmodem->exists() ) xmodem_receive_byte( &xmodem->device(), param );
	//else if ( kermit && kermit->exists() ) kermit_receive_byte( &kermit->device(), param );
}

/* CPU initiates a send event */
void hp48_state::hp48_rs232_send_byte(  )
{
	UINT8 data = HP48_IO_8(0x16); /* byte to send */

	LOG_SERIAL(( "%s %f hp48_rs232_send_byte: start sending, data=%02x\n",
				machine().describe_context(), machine().time().as_double(), data ));

	/* set byte sending and send buffer full */
	m_io[0x12] |= 3;

	/* schedule transmission */
	machine().scheduler().timer_set( RS232_DELAY, timer_expired_delegate(FUNC(hp48_state::hp48_rs232_byte_sent_cb),this), data);
}




/* ------ Saturn's IN / OUT registers ---------- */


/* CPU sets OUT register (keyboard + beeper) */
WRITE32_MEMBER( hp48_state::hp48_reg_out )
{
	LOG(( "%s %f hp48_reg_out: %03x\n",
			machine().describe_context(), machine().time().as_double(), data ));

	/* bits 0-8: keyboard lines */
	m_out = data & 0x1ff;

	/* bits 9-10: unused */

	/* bit 11: beeper */
	m_dac->write_unsigned8((data & 0x800) ? 0x80 : 00 );
}

int hp48_state::hp48_get_in(  )
{
	int in = 0;

	/* regular keys */
	if ( (m_out >> 0) & 1 ) in |= ioport( "LINE0" )->read();
	if ( (m_out >> 1) & 1 ) in |= ioport( "LINE1" )->read();
	if ( (m_out >> 2) & 1 ) in |= ioport( "LINE2" )->read();
	if ( (m_out >> 3) & 1 ) in |= ioport( "LINE3" )->read();
	if ( (m_out >> 4) & 1 ) in |= ioport( "LINE4" )->read();
	if ( (m_out >> 5) & 1 ) in |= ioport( "LINE5" )->read();
	if ( (m_out >> 6) & 1 ) in |= ioport( "LINE6" )->read();
	if ( (m_out >> 7) & 1 ) in |= ioport( "LINE7" )->read();
	if ( (m_out >> 8) & 1 ) in |= ioport( "LINE8" )->read();

	/* on key */
	in |= ioport( "ON" )->read();

	return in;
}

/* CPU reads IN register (keyboard) */
READ32_MEMBER( hp48_state::hp48_reg_in )
{
	int in = hp48_get_in();
	LOG(( "%s %f hp48_reg_in: %04x\n",
			machine().describe_context(), machine().time().as_double(), in ));
	return in;
}

/* key detect */
void hp48_state::hp48_update_kdn( )
{
	int in = hp48_get_in();

	/* interrupt on raising edge */
	if ( in && !m_kdn )
	{
		LOG(( "%f hp48_update_kdn: interrupt\n", machine().time().as_double() ));
		m_io[0x19] |= 8;                                              /* service request */
		hp48_pulse_irq( SATURN_WAKEUP_LINE );
		hp48_pulse_irq( SATURN_IRQ_LINE );
	}

	m_kdn = (in!=0);
}

/* periodic keyboard polling, generates an interrupt */
TIMER_CALLBACK_MEMBER(hp48_state::hp48_kbd_cb)
{
	/* NMI for ON key */
	if ( ioport( "ON" )->read() )
	{
		LOG(( "%f hp48_kbd_cb: keyboard interrupt, on key\n",
				machine().time().as_double() ));
		m_io[0x19] |= 8;                                          /* set service request */
		hp48_pulse_irq( SATURN_WAKEUP_LINE );
		hp48_pulse_irq( SATURN_NMI_LINE );
		return;
	}

	/* regular keys */
	hp48_update_kdn();
}

/* RSI opcode */
WRITE_LINE_MEMBER( hp48_state::hp48_rsi )
{
	LOG(( "%s %f hp48_rsi\n", machine().describe_context(), machine().time().as_double() ));

	/* enables interrupts on key repeat
	   (normally, there is only one interrupt, when the key is pressed)
	*/
	m_kdn = 0;
}


/* ------------- annonciators ------------ */

void hp48_state::hp48_update_annunciators()
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
	output().set_value( "lshift0",   (markers & 0x81) == 0x81 );
	output().set_value( "rshift0",   (markers & 0x82) == 0x82 );
	output().set_value( "alpha0",    (markers & 0x84) == 0x84 );
	output().set_value( "alert0",    (markers & 0x88) == 0x88 );
	output().set_value( "busy0",     (markers & 0x90) == 0x90 );
	output().set_value( "transmit0", (markers & 0xb0) == 0xb0 );
}


/* ------------- I/O registers ----------- */

/* Some part of the I/O registers are simple r/w registers. We store them in hp48_io.
   Special cases are registers that:
   - have a different meaning on read and write
   - perform some action on read / write
 */

WRITE8_MEMBER(hp48_state::hp48_io_w)
{
	LOG(( "%s %f hp48_io_w: off=%02x data=%x\n",
			space.machine().describe_context(), space.machine().time().as_double(), offset, data ));

	switch( offset )
	{
	/* CRC register */
	case 0x04: m_crc = (m_crc & 0xfff0) | data; break;
	case 0x05: m_crc = (m_crc & 0xff0f) | (data << 4); break;
	case 0x06: m_crc = (m_crc & 0xf0ff) | (data << 8); break;
	case 0x07: m_crc = (m_crc & 0x0fff) | (data << 12); break;

	/* annunciators */
	case 0x0b:
	case 0x0c:
		m_io[offset] = data;
		hp48_update_annunciators();
		break;

	/* cntrl ROM */
	case 0x29:
	{
		int old_cntrl = m_io[offset] & 8;
		m_io[offset] = data;
		if ( old_cntrl != (data & 8) )
		{
			hp48_apply_modules();
		}
		break;
	}

	/* timers */
	case 0x37: m_timer1 = data; break;
	case 0x38: m_timer2 = (m_timer2 & 0xfffffff0) | data; break;
	case 0x39: m_timer2 = (m_timer2 & 0xffffff0f) | (data << 4); break;
	case 0x3a: m_timer2 = (m_timer2 & 0xfffff0ff) | (data << 8); break;
	case 0x3b: m_timer2 = (m_timer2 & 0xffff0fff) | (data << 12); break;
	case 0x3c: m_timer2 = (m_timer2 & 0xfff0ffff) | (data << 16); break;
	case 0x3d: m_timer2 = (m_timer2 & 0xff0fffff) | (data << 20); break;
	case 0x3e: m_timer2 = (m_timer2 & 0xf0ffffff) | (data << 24); break;
	case 0x3f: m_timer2 = (m_timer2 & 0x0fffffff) | (data << 28); break;

	/* cards */
	case 0x0e:
		LOG(( "%s: card control write %02x\n", space.machine().describe_context(), data ));

		/* bit 0: software interrupt */
		if ( data & 1 )
		{
			LOG(( "%f hp48_io_w: software interrupt requested\n",
					space.machine().time().as_double() ));
			hp48_pulse_irq( SATURN_IRQ_LINE );
			data &= ~1;
		}

		/* XXX not implemented
		    bit 1: card test?
		 */

		m_io[0x0e] = data;
		break;

	case 0x0f:
		LOG(( "%s: card info write %02x\n", space.machine().describe_context(), data ));
		m_io[0x0f] = data;
		break;

	/* serial */
	case 0x13:
		m_io[0x11] &= ~4; /* clear error status */
		break;
	case 0x16:
		/* first nibble of sent data */
		m_io[offset] = data;
		break;
	case 0x17:
		/* second nibble of sent data */
		m_io[offset] = data;
		hp48_rs232_send_byte();
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
	                 on HP49 G, flash ROM write enable

	   - 0x1d: I/R output buffer
	*/

	default: m_io[offset] = data;
	}

}


READ8_MEMBER(hp48_state::hp48_io_r)
{
	UINT8 data = 0;

	switch( offset )
	{
	/* CRC register */
	case 0x04: data = m_crc & 0xf; break;
	case 0x05: data = (m_crc >> 4) & 0xf; break;
	case 0x06: data = (m_crc >> 8) & 0xf; break;
	case 0x07: data = (m_crc >> 12) & 0xf; break;

	/* battery test */
	case 0x08:
		data = 0;
		if ( m_io[0x9] & 8 ) /* test enable */
		{
			/* XXX not implemented:
			   bit 3: battery in port 2
			   bit 2: battery in port 1
			 */
			switch ( ioport( "BATTERY" )->read() )
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
		int cur_line = space.machine().first_screen()->vpos();
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
	case 0x37: data = m_timer1; break;
	case 0x38: data = m_timer2 & 0xf; break;
	case 0x39: data = (m_timer2 >> 4) & 0xf; break;
	case 0x3a: data = (m_timer2 >> 8) & 0xf; break;
	case 0x3b: data = (m_timer2 >> 12) & 0xf; break;
	case 0x3c: data = (m_timer2 >> 16) & 0xf; break;
	case 0x3d: data = (m_timer2 >> 20) & 0xf; break;
	case 0x3e: data = (m_timer2 >> 24) & 0xf; break;
	case 0x3f: data = (m_timer2 >> 28) & 0xf; break;

	/* serial */
	case 0x15:
	{
		/* second nibble of received data */

		//device_image_interface *xmodem = dynamic_cast<device_image_interface *>(space.machine().device("rs232_x"));
		//device_image_interface *kermit = dynamic_cast<device_image_interface *>(space.machine().device("rs232_k"));

		m_io[0x11] &= ~1;  /* clear byte received */
		data = m_io[offset];

		/* protocol action */
		//if ( xmodem && xmodem->exists() ) xmodem_byte_transmitted( &xmodem->device() );
		//else if ( kermit && kermit->exists() ) kermit_byte_transmitted( &kermit->device() );
		break;
	}

	/* cards */
	case 0x0e: /* detection */
		data = m_io[0x0e];
		LOG(( "%s: card control read %02x\n", space.machine().describe_context(), data ));
		break;
	case 0x0f: /* card info */
		data = 0;
		if ( HP48_G_SERIES )
		{
			if ( m_port_size[1] ) data |= 1;
			if ( m_port_size[0] ) data |= 2;
			if ( m_port_size[1] && m_port_write[1] ) data |= 4;
			if ( m_port_size[0] && m_port_write[0] ) data |= 8;
		}
		else
		{
			if ( m_port_size[0] ) data |= 1;
			if ( m_port_size[1] ) data |= 2;
			if ( m_port_size[0] && m_port_write[0] ) data |= 4;
			if ( m_port_size[1] && m_port_write[1] ) data |= 8;
		}
		LOG(( "%s: card info read %02x\n", space.machine().describe_context(), data ));
		break;


	default: data = m_io[offset];
	}

	LOG(( "%s %f hp48_io_r: off=%02x data=%x\n",
			space.machine().describe_context(), space.machine().time().as_double(), offset, data ));
	return data;
}


/* ---------- bank switcher --------- */

READ8_MEMBER(hp48_state::hp48_bank_r)
{
	/* HP48 GX
	   bit 0: ignored
	   bits 2-5: bank number
	   bit 6: enable
	*/

	/* HP49 G
	   bit 0: ignored
	   bits 1-2: select bank 0x00000-0x3ffff
	   bits 3-6: select bank 0x40000-0x7ffff
	 */
	offset &= 0x7e;
	if ( m_bank_switch != offset )
	{
		LOG(( "%s %f hp48_bank_r: off=%03x\n", space.machine().describe_context(), space.machine().time().as_double(), offset ));
		m_bank_switch = offset;
		hp48_apply_modules();
	}
	return 0;
}


WRITE8_MEMBER(hp48_state::hp49_bank_w)
{
	offset &= 0x7e;
	if ( m_bank_switch != offset )
	{
		LOG(( "%05x %f hp49_bank_w: off=%03x\n", space.device().safe_pcbase(), space.machine().time().as_double(), offset ));
		m_bank_switch = offset;
		hp48_apply_modules();
	}
}



/* ---------------- timers --------------- */

TIMER_CALLBACK_MEMBER(hp48_state::hp48_timer1_cb)
{
	if ( !(m_io[0x2f] & 1) ) return; /* timer enable */

	m_timer1 = (m_timer1 - 1) & 0xf;

	/* wake-up on carry */
	if ( (m_io[0x2e] & 4) && (m_timer1 == 0xf) )
	{
		LOG(( "wake-up on timer1\n" ));
		m_io[0x2e] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( SATURN_WAKEUP_LINE );
	}
	/* interrupt on carry */
	if ( (m_io[0x2e] & 2) && (m_timer1 == 0xf) )
	{
		LOG(( "generate timer1 interrupt\n" ));
		m_io[0x2e] |= 8; /* set service request */
		m_io[0x18] |= 4; /* set service request */
		hp48_pulse_irq( SATURN_NMI_LINE );
	}
}

TIMER_CALLBACK_MEMBER(hp48_state::hp48_timer2_cb)
{
	if ( !(m_io[0x2f] & 1) ) return; /* timer enable */

	m_timer2 = (m_timer2 - 1) & 0xffffffff;

	/* wake-up on carry */
	if ( (m_io[0x2f] & 4) && (m_timer2 == 0xffffffff) )
	{
		LOG(( "wake-up on timer2\n" ));
		m_io[0x2f] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( SATURN_WAKEUP_LINE );
	}
	/* interrupt on carry */
	if ( (m_io[0x2f] & 2) && (m_timer2 == 0xffffffff) )
	{
		LOG(( "generate timer2 interrupt\n" ));
		m_io[0x2f] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		hp48_pulse_irq( SATURN_NMI_LINE );
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


      controller    48 S series     48 G series         49 G series
                    (Clark CPU)     (York CPU)          (York CPU)

         HDW        32B I/O RAM    32B I/O RAM         32B I/O RAM
        NCE2         32KB RAM      32/128KB RAM        256KB RAM
         CE1           port1       bank switcher       bank switcher
         CE2           port2          port1             128KB RAM
        NCE3          unused          port2             128KB RAM
        NCE1         256KB ROM      512KB ROM         2MB flash ROM


   - NCE1 (ROM) cannot be configured, it is always visible at addresses
   00000-7ffff not covered by higher priority modules.

   - only the address of HDW (I/O) can be configured, its size is constant
   (64 nibbles)

   - other modules can have their address & size set

 */


/* remap all modules according to hp48_modules */
void hp48_state::hp48_apply_modules()
{
	int i;
	int nce3_enable = 1;
	address_space& space = m_maincpu->space(AS_PROGRAM);

	m_io_addr = 0x100000;

	/* NCE1 (ROM) is a bit special, so treat it separately */
	space.unmap_readwrite( 0, 0xfffff, 0, 0 );
	if ( HP49_G_MODEL )
	{
		int bank_lo = (m_bank_switch >> 5) & 3;
		int bank_hi = (m_bank_switch >> 1) & 15;
		LOG(( "hp48_apply_modules: low ROM bank is %i\n", bank_lo ));
		LOG(( "hp48_apply_modules: high ROM bank is %i\n", bank_hi ));
		space.install_read_bank( 0x00000, 0x3ffff, 0, 0x80000, "bank5" );
		space.install_read_bank( 0x40000, 0x7ffff, 0, 0x80000, "bank6" );
		if ( m_rom )
		{
			membank("bank5")->set_base( m_rom + bank_lo * 0x40000 );
			membank("bank6")->set_base( m_rom + bank_hi * 0x40000 );
		}
	}
	else if ( HP48_G_SERIES )
	{
		/* port 2 bank switch */
		if ( m_port_size[1] > 0 )
		{
			int off = (m_bank_switch << 16) % m_port_size[1];
			LOG(( "hp48_apply_modules: port 2 offset is %i\n", off ));
			m_modules[HP48_NCE3].data = m_port_data[1].get() + off;
		}

		/* ROM A19 (hi 256 KB) / NCE3 (port 2) control switch */
		if ( m_io[0x29] & 8 )
		{
			/* A19 */
			LOG(( "hp48_apply_modules: A19 enabled, NCE3 disabled\n" ));
			nce3_enable = 0;
			space.install_read_bank( 0, 0xfffff, 0, 0, "bank5" );
		}
		else
		{
			/* NCE3 */
			nce3_enable = m_bank_switch >> 6;
			LOG(( "hp48_apply_modules: A19 disabled, NCE3 %s\n", nce3_enable ? "enabled" : "disabled" ));
			space.install_read_bank( 0, 0x7ffff, 0, 0x80000, "bank5" );
		}
		if ( m_rom )
			membank("bank5")->set_base( m_rom );
	}
	else
	{
		space.install_read_bank( 0, 0x7ffff, 0, 0x80000, "bank5" );
		if ( m_rom )
			membank("bank5")->set_base( m_rom );
	}


	/* from lowest to highest priority */
	for ( i = 4; i >= 0; i-- )
	{
		UINT32 select_mask = m_modules[i].mask;
		UINT32 nselect_mask = ~select_mask & 0xfffff;
		UINT32 base = m_modules[i].base;
		UINT32 off_mask = m_modules[i].off_mask;
		UINT32 mirror = nselect_mask & ~off_mask;
		UINT32 end = base + (off_mask & nselect_mask);
		char bank[10];
		sprintf(bank,"bank%d",i);

		if ( m_modules[i].state != HP48_MODULE_CONFIGURED ) continue;

		if ( (i == 4) && !nce3_enable ) continue;

		/* our code assumes that the 20-bit select_mask is all 1s followed by all 0s */
		if ( nselect_mask & (nselect_mask+1) )
		{
			logerror( "hp48_apply_modules: invalid mask %05x for module %s\n",
					select_mask, hp48_module_names[i] );
			continue;
		}

		if (m_modules[i].data)
			space.install_read_bank( base, end, 0, mirror, bank );
		else
		{
			if (!m_modules[i].read.isnull())
				space.install_read_handler( base, end, 0, mirror, m_modules[i].read );
		}

		if (m_modules[i].isnop)
			space.nop_write(base, end, 0, mirror);
		else
		{
			if (m_modules[i].data)
			{
				space.install_write_bank( base, end, 0, mirror, bank );
			}
			else
			{
				if (!m_modules[i].write.isnull())
					space.install_write_handler( base, end, 0, mirror, m_modules[i].write );
			}
		}

		LOG(( "hp48_apply_modules: module %s configured at %05x-%05x, mirror %05x\n",
				hp48_module_names[i], base, end, mirror ));

		if ( m_modules[i].data )
		{
			membank( bank )->set_base( m_modules[i].data );
		}

		if ( i == 0 )
		{
			m_io_addr = base;
		}
	}
}


/* reset the configuration */
void hp48_state::hp48_reset_modules(  )
{
	int i;
	/* fixed size for HDW */
	m_modules[HP48_HDW].state = HP48_MODULE_MASK_KNOWN;
	m_modules[HP48_HDW].mask = 0xfffc0;
	/* unconfigure NCE2, CE1, CE2, NCE3 */
	for ( i = 1; i < 5; i++ )
	{
		m_modules[i].state = HP48_MODULE_UNCONFIGURED;
	}

	/* fixed configuration for NCE1 */
	m_modules[HP48_NCE1].state = HP48_MODULE_CONFIGURED;
	m_modules[HP48_NCE1].base = 0;
	m_modules[HP48_NCE1].mask = 0;

	hp48_apply_modules();
}


/* RESET opcode */
WRITE_LINE_MEMBER( hp48_state::hp48_mem_reset )
{
	LOG(( "%s %f hp48_mem_reset\n", machine().describe_context(), machine().time().as_double() ));
	hp48_reset_modules();
}


/* CONFIG opcode */
WRITE32_MEMBER( hp48_state::hp48_mem_config )
{
	int i;

	LOG(( "%s %f hp48_mem_config: %05x\n", machine().describe_context(), machine().time().as_double(), data ));

	/* find the highest priority unconfigured module (except non-configurable NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... first call sets the address mask */
		if ( m_modules[i].state == HP48_MODULE_UNCONFIGURED )
		{
			m_modules[i].mask = data & 0xff000;
			m_modules[i].state = HP48_MODULE_MASK_KNOWN;
			break;
		}

		/* ... second call sets the base address */
		if ( m_modules[i].state == HP48_MODULE_MASK_KNOWN )
		{
			m_modules[i].base = data & m_modules[i].mask;
			m_modules[i].state = HP48_MODULE_CONFIGURED;
			LOG(( "hp48_mem_config: module %s configured base=%05x, mask=%05x\n",
					hp48_module_names[i], m_modules[i].base, m_modules[i].mask ));
			hp48_apply_modules();
			break;
		}
	}
}


/* UNCFG opcode */
WRITE32_MEMBER( hp48_state::hp48_mem_unconfig )
{
	int i;
	LOG(( "%s %f hp48_mem_unconfig: %05x\n", machine().describe_context(), machine().time().as_double(), data ));

	/* find the highest priority fully configured module at address v (except NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... and unconfigure it */
		if ( m_modules[i].state == HP48_MODULE_CONFIGURED &&
				(m_modules[i].base == (data & m_modules[i].mask)) )
		{
			m_modules[i].state = i> 0 ? HP48_MODULE_UNCONFIGURED : HP48_MODULE_MASK_KNOWN;
			LOG(( "hp48_mem_unconfig: module %s\n", hp48_module_names[i] ));
			hp48_apply_modules();
			break;
		}
	}
}


/* C=ID opcode */
READ32_MEMBER( hp48_state::hp48_mem_id )
{
	int i;
	int data = 0; /* 0 = everything is configured */

	/* find the highest priority unconfigured module (except NCE1)... */
	for ( i = 0; i < 5; i++ )
	{
		/* ... mask need to be configured */
		if ( m_modules[i].state == HP48_MODULE_UNCONFIGURED )
		{
			data = hp48_module_mask_id[i] | (m_modules[i].mask & ~0xff);
			break;
		}

		/* ... address need to be configured */
		if ( m_modules[i].state == HP48_MODULE_MASK_KNOWN )
		{
			data = hp48_module_addr_id[i] | (m_modules[i].base & ~0x3f);
			break;
		}
	}

	LOG(( "%s %f hp48_mem_id = %02x\n",
			machine().describe_context(), machine().time().as_double(), data ));

	return data; /* everything is configured */
}



/* --------- CRC ---------- */

/* each memory read by the CPU updates the internal CRC state */
WRITE32_MEMBER( hp48_state::hp48_mem_crc )
{
	/* no CRC for I/O RAM */
	if ( offset >= m_io_addr && offset < m_io_addr + 0x40 ) return;

	m_crc = (m_crc >> 4) ^ (((m_crc ^ data) & 0xf) * 0x1081);
}



/* ------ utilities ------- */


/* decodes size bytes into 2*size nibbles (least significant first) */
void hp48_state::hp48_decode_nibble( UINT8* dst, UINT8* src, int size )
{
	int i;
	for ( i=size-1; i >= 0; i-- )
	{
		dst[2*i+1] = src[i] >> 4;
		dst[2*i] = src[i] & 0xf;
	}
}

/* inverse of hp48_decode_nibble  */
void hp48_state::hp48_encode_nibble( UINT8* dst, UINT8* src, int size )
{
	int i;
	for ( i=0; i < size; i++ )
	{
		dst[i] = (src[2*i] & 0xf) | (src[2*i+1] << 4);
	}
}



/* ----- card images ------ */
const device_type HP48_PORT = &device_creator<hp48_port_image_device>;

/* helper for load and create */
void hp48_port_image_device::hp48_fill_port()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	int size = state->m_port_size[m_port];
	LOG(( "hp48_fill_port: %s module=%i size=%i rw=%i\n", tag(), m_module, size, state->m_port_write[m_port] ));
	state->m_port_data[m_port] = make_unique_clear<UINT8[]>(2 * size);
	state->m_modules[m_module].off_mask = 2 * (( size > 128 * 1024 ) ? 128 * 1024 : size) - 1;
	state->m_modules[m_module].read     = read8_delegate();
	state->m_modules[m_module].write    = write8_delegate();
	state->m_modules[m_module].isnop    = state->m_port_write[m_port] ? 0 : 1;
	state->m_modules[m_module].data     = (void*)state->m_port_data[m_port].get();
	state->hp48_apply_modules();
}

/* helper for start and unload */
void hp48_port_image_device::hp48_unfill_port()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	LOG(( "hp48_unfill_port\n" ));
	state->m_modules[m_module].off_mask = 0x00fff;  /* 2 KB */
	state->m_modules[m_module].read     = read8_delegate();
	state->m_modules[m_module].write    = write8_delegate();
	state->m_modules[m_module].data     = nullptr;
	state->m_modules[m_module].isnop    = 1;
	state->m_port_size[m_port]          = 0;
}


bool hp48_port_image_device::call_load()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	int size = length();
	if ( size == 0 ) size = m_max_size; /* default size */
	LOG(( "hp48_port_image load: size=%i\n", size ));

	/* check size */
	if ( (size < 32*1024) || (size > m_max_size) || (size & (size-1)) )
	{
		logerror( "hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, m_max_size );
		return IMAGE_INIT_FAIL;
	}

	state->m_port_size[m_port] = size;
	state->m_port_write[m_port] = !is_readonly();
	hp48_fill_port( );
	fread(state->m_port_data[m_port].get(), state->m_port_size[m_port] );
	state->hp48_decode_nibble( state->m_port_data[m_port].get(), state->m_port_data[m_port].get(), state->m_port_size[m_port] );
	return IMAGE_INIT_PASS;
}

bool hp48_port_image_device::call_create(int format_type, option_resolution *format_options)
{
	hp48_state *state = machine().driver_data<hp48_state>();
	int size = m_max_size;
	LOG(( "hp48_port_image create: size=%i\n", size ));
	/* XXX defaults to max_size; get user-specified size instead */

	/* check size */
	/* size must be a power of 2 between 32K and max_size */
	if ( (size < 32*1024) || (size > m_max_size) || (size & (size-1)) )
	{
		logerror( "hp48: image size for %s should be a power of two between %i and %i\n", tag(), 32*1024, m_max_size );
		return IMAGE_INIT_FAIL;
	}

	state->m_port_size[m_port] = size;
	state->m_port_write[m_port] = 1;
	hp48_fill_port();
	return IMAGE_INIT_PASS;
}

void hp48_port_image_device::call_unload()
{
	hp48_state *state = machine().driver_data<hp48_state>();
	LOG(( "hp48_port image unload: %s size=%i rw=%i\n",
			tag(), state->m_port_size[m_port], state->m_port_write[m_port] ));
	if ( state->m_port_write[m_port] )
	{
		state->hp48_encode_nibble( state->m_port_data[m_port].get(), state->m_port_data[m_port].get(), state->m_port_size[m_port] );
		fseek( 0, SEEK_SET );
		fwrite( state->m_port_data[m_port].get(), state->m_port_size[m_port] );
	}
	state->m_port_data[m_port] = nullptr;
	hp48_unfill_port();
	state->hp48_apply_modules();
}

void hp48_port_image_device::device_start()
{
	LOG(("hp48_port_image start\n"));
	hp48_unfill_port();
}

hp48_port_image_device::hp48_port_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HP48_PORT, "HP48 memory card", tag, owner, clock, "hp48_port_image", __FILE__),
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
		m_modules[i].read     = read8_delegate();
		m_modules[i].write    = write8_delegate();
		m_modules[i].data     = nullptr;
		m_modules[i].isnop    = 0;
	}
	m_port_size[0] = 0;
	m_port_size[1] = 0;
	m_rom = nullptr;
}

void hp48_state::machine_reset()
{
	LOG(( "hp48: machine reset called\n" ));
	m_bank_switch = 0;
	hp48_reset_modules();
	hp48_update_annunciators();
}

void hp48_state::hp48_machine_start( hp48_models model )
{
	UINT8 *ram;
	int ram_size, rom_size, i;

	LOG(( "hp48_machine_start: model %i\n", model ));

	m_model = model;

	/* internal RAM */
	ram_size =
		HP49_G_MODEL  ? (512 * 1024) :
		HP48_GX_MODEL ? (128 * 1024) : (32 * 1024);

	ram = auto_alloc_array(machine(), UINT8, 2 * ram_size);
	machine().device<nvram_device>("nvram")->set_base(ram, 2 * ram_size);


	/* ROM load */
	rom_size =
		HP49_G_MODEL  ? (2048 * 1024) :
		HP48_S_SERIES ?  (256 * 1024) : (512 * 1024);
	m_rom = auto_alloc_array(machine(), UINT8, 2 * rom_size);
	hp48_decode_nibble( m_rom, memregion( "maincpu" )->base(), rom_size );

	/* init state */
	memset( ram, 0, 2 * ram_size );
	memset( m_io, 0, sizeof( m_io ) );
	m_out = 0;
	m_kdn = 0;
	m_crc = 0;
	m_timer1 = 0;
	m_timer2 = 0;
	m_bank_switch = 0;

	/* I/O RAM */
	m_modules[HP48_HDW].off_mask = 0x0003f;  /* 32 B */
	m_modules[HP48_HDW].read     = read8_delegate(FUNC(hp48_state::hp48_io_r),this);
	m_modules[HP48_HDW].write    = write8_delegate(FUNC(hp48_state::hp48_io_w),this);

	/* internal RAM */
	if ( HP49_G_MODEL )
	{
		m_modules[HP48_NCE2].off_mask = 2 * 256 * 1024 - 1;
		m_modules[HP48_NCE2].data     = ram;
		m_modules[HP48_CE2].off_mask  = 2 * 128 * 1024 - 1;
		m_modules[HP48_CE2].data      = ram + 2 * 256 * 1024;
		m_modules[HP48_NCE3].off_mask = 2 * 128 * 1024 - 1;
		m_modules[HP48_NCE3].data     = ram + 2 * (128+256) * 1024;
	}
	else
	{
		m_modules[HP48_NCE2].off_mask = 2 * ram_size - 1;
		m_modules[HP48_NCE2].data     = ram;
	}

	/* bank switcher */
	if ( HP48_G_SERIES )
	{
		m_modules[HP48_CE1].off_mask = 0x00fff;  /* 2 KB */
		m_modules[HP48_CE1].read     = read8_delegate(FUNC(hp48_state::hp48_bank_r),this);
		m_modules[HP48_CE1].write    = HP49_G_MODEL ? write8_delegate(FUNC(hp48_state::hp49_bank_w),this) : write8_delegate();
	}

	/* timers */
	machine().scheduler().timer_pulse(attotime::from_hz( 16 ), timer_expired_delegate(FUNC(hp48_state::hp48_timer1_cb),this));
	machine().scheduler().timer_pulse(attotime::from_hz( 8192 ), timer_expired_delegate(FUNC(hp48_state::hp48_timer2_cb),this));

	/* 1ms keyboard polling */
	machine().scheduler().timer_pulse(attotime::from_msec( 1 ), timer_expired_delegate(FUNC(hp48_state::hp48_kbd_cb),this));

	/* save state */
	save_item(NAME(m_out) );
	save_item(NAME(m_kdn) );
	save_item(NAME(m_io_addr) );
	save_item(NAME(m_crc) );
	save_item(NAME(m_timer1) );
	save_item(NAME(m_timer2) );
	save_item(NAME(m_bank_switch) );
	for ( i = 0; i < 6; i++ )
	{
		save_item(m_modules[i].state, "globals/m_modules[i].state", i);
		save_item(m_modules[i].base, "globals/m_modules[i].base", i);
		save_item(m_modules[i].mask, "globals/m_modules[i].mask", i);
	}
	save_item(NAME(m_io) );
	machine().save().register_postload( save_prepost_delegate(FUNC(hp48_state::hp48_update_annunciators), this ));
	machine().save().register_postload( save_prepost_delegate(FUNC(hp48_state::hp48_apply_modules), this ));

}


MACHINE_START_MEMBER(hp48_state,hp48s)
{
	hp48_machine_start( HP48_S );
}


MACHINE_START_MEMBER(hp48_state,hp48sx)
{
	hp48_machine_start( HP48_SX );
}


MACHINE_START_MEMBER(hp48_state,hp48g)
{
	hp48_machine_start( HP48_G );
}

MACHINE_START_MEMBER(hp48_state,hp48gx)
{
	hp48_machine_start( HP48_GX );
}

MACHINE_START_MEMBER(hp48_state,hp48gp)
{
	hp48_machine_start( HP48_GP );
}

MACHINE_START_MEMBER(hp48_state,hp49g)
{
	hp48_machine_start( HP49_G );
}
