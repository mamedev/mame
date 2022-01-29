// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+ and HP49 G

**********************************************************************/

#include "emu.h"
#include "includes/hp48.h"

#include "cpu/saturn/saturn.h"
#include "machine/nvram.h"

#include "screen.h"


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
static const uint8_t hp48_module_mask_id[6] = { 0x00, 0x03, 0x05, 0x07, 0x01, 0x00 };
static const uint8_t hp48_module_addr_id[6] = { 0x19, 0xf4, 0xf6, 0xf8, 0xf2, 0x00 };





/***************************************************************************
    FUNCTIONS
***************************************************************************/

void hp48_state::pulse_irq(int irq_line)
{
	m_maincpu->set_input_line(irq_line, ASSERT_LINE);
	m_maincpu->set_input_line(irq_line, CLEAR_LINE);
}


/* ---------------- serial --------------------- */

#define RS232_DELAY attotime::from_usec(300)

/* end of receive event */
TIMER_CALLBACK_MEMBER(hp48_state::rs232_byte_recv_cb)
{
	LOG_SERIAL(("%f hp48_state::rs232_byte_recv_cb: end of receive, data=%02x\n", machine().time().as_double(), param));

	m_io[0x14] = param & 0xf; /* receive zone */
	m_io[0x15] = param >> 4;
	m_io[0x11] &= ~2; /* clear byte receiving */
	m_io[0x11] |= 1;  /* set byte received */

	/* interrupt */
	if (m_io[0x10] & 2)
	{
		pulse_irq(SATURN_IRQ_LINE);
	}
}

/* outside world initiates a receive event */
void hp48_state::rs232_start_recv_byte(uint8_t data)
{
	LOG_SERIAL(("%f hp48_state::rs232_start_recv_byte: start receiving, data=%02x\n", machine().time().as_double(), data));

	m_io[0x11] |= 2;  /* set byte receiving */

	/* interrupt */
	if (m_io[0x10] & 1)
	{
		pulse_irq(SATURN_IRQ_LINE);
	}

	/* schedule end of reception */
	machine().scheduler().timer_set(RS232_DELAY, timer_expired_delegate(FUNC(hp48_state::rs232_byte_recv_cb),this), data);
}


/* end of send event */
TIMER_CALLBACK_MEMBER(hp48_state::rs232_byte_sent_cb)
{
	LOG_SERIAL(("%f hp48_state::rs232_byte_sent_cb: end of send, data=%02x\n", machine().time().as_double(), param));

	m_io[0x12] &= ~3; /* clear byte sending and buffer full */

	/* interrupt */
	if (m_io[0x10] & 4)
	{
		pulse_irq(SATURN_IRQ_LINE);
	}
}

/* CPU initiates a send event */
void hp48_state::rs232_send_byte()
{
	uint8_t data = HP48_IO_8(0x16); /* byte to send */

	LOG_SERIAL(("%s %f hp48_state::rs232_send_byte: start sending, data=%02x\n", machine().describe_context(), machine().time().as_double(), data));

	/* set byte sending and send buffer full */
	m_io[0x12] |= 3;

	/* schedule transmission */
	machine().scheduler().timer_set(RS232_DELAY, timer_expired_delegate(FUNC(hp48_state::rs232_byte_sent_cb),this), data);
}




/* ------ Saturn's IN / OUT registers ---------- */


/* CPU sets OUT register (keyboard + beeper) */
void hp48_state::reg_out(uint32_t data)
{
	LOG(("%s %f hp48_state::reg_out: %03x\n", machine().describe_context(), machine().time().as_double(), data));

	/* bits 0-8: keyboard lines */
	m_out = data & 0x1ff;

	/* bits 9-10: unused */

	/* bit 11: beeper */
	m_dac->write(BIT(data, 11));
}

int hp48_state::get_in()
{
	int in = 0;

	/* regular keys */
	if (BIT(m_out, 0)) in |= ioport("LINE0")->read();
	if (BIT(m_out, 1)) in |= ioport("LINE1")->read();
	if (BIT(m_out, 2)) in |= ioport("LINE2")->read();
	if (BIT(m_out, 3)) in |= ioport("LINE3")->read();
	if (BIT(m_out, 4)) in |= ioport("LINE4")->read();
	if (BIT(m_out, 5)) in |= ioport("LINE5")->read();
	if (BIT(m_out, 6)) in |= ioport("LINE6")->read();
	if (BIT(m_out, 7)) in |= ioport("LINE7")->read();
	if (BIT(m_out, 8)) in |= ioport("LINE8")->read();

	/* on key */
	in |= ioport("ON")->read();

	return in;
}

/* CPU reads IN register (keyboard) */
uint32_t hp48_state::reg_in()
{
	int in = get_in();
	LOG(("%s %f hp48_state::reg_in: %04x\n", machine().describe_context(), machine().time().as_double(), in));
	return in;
}

/* key detect */
void hp48_state::update_kdn()
{
	int in = get_in();

	/* interrupt on raising edge */
	if (in && !m_kdn)
	{
		LOG(("%f hp48_state::update_kdn: interrupt\n", machine().time().as_double()));
		m_io[0x19] |= 8; // service request
		pulse_irq(SATURN_WAKEUP_LINE);
		pulse_irq(SATURN_IRQ_LINE);
	}

	m_kdn = (in != 0);
}

/* periodic keyboard polling, generates an interrupt */
TIMER_CALLBACK_MEMBER(hp48_state::kbd_cb)
{
	/* NMI for ON key */
	if (ioport( "ON" )->read())
	{
		LOG(("%f hp48_state::kbd_cb: keyboard interrupt, on key\n", machine().time().as_double()));
		m_io[0x19] |= 8; // set service request
		pulse_irq(SATURN_WAKEUP_LINE);
		pulse_irq(SATURN_NMI_LINE);
		return;
	}

	/* regular keys */
	update_kdn();
}

/* RSI opcode */
WRITE_LINE_MEMBER( hp48_state::rsi )
{
	LOG(("%s %f hp48_state::rsi\n", machine().describe_context(), machine().time().as_double()));

	// enables interrupts on key repeat (normally, there is only one interrupt, when the key is pressed)
	m_kdn = 0;
}


/* ------------- annonciators ------------ */

void hp48_state::update_annunciators()
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
	m_lshift0   = (markers & 0x81) == 0x81;
	m_rshift0   = (markers & 0x82) == 0x82;
	m_alpha0    = (markers & 0x84) == 0x84;
	m_alert0    = (markers & 0x88) == 0x88;
	m_busy0     = (markers & 0x90) == 0x90;
	m_transmit0 = (markers & 0xb0) == 0xb0;
}


/* ------------- I/O registers ----------- */

/* Some part of the I/O registers are simple r/w registers. We store them in hp48_io.
   Special cases are registers that:
   - have a different meaning on read and write
   - perform some action on read / write
 */

void hp48_state::io_w(offs_t offset, uint8_t data)
{
	LOG(("%s %f hp48_state::io_w: off=%02x data=%x\n", machine().describe_context(), machine().time().as_double(), offset, data));

	switch (offset)
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
		update_annunciators();
		break;

	/* cntrl ROM */
	case 0x29:
	{
		int old_cntrl = m_io[offset] & 8;
		m_io[offset] = data;
		if (old_cntrl != (data & 8))
		{
			apply_modules();
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
		LOG(("%s: card control write %02x\n", machine().describe_context(), data));

		/* bit 0: software interrupt */
		if (data & 1)
		{
			LOG(("%f hp48_state::io_w: software interrupt requested\n", machine().time().as_double()));
			pulse_irq(SATURN_IRQ_LINE);
			data &= ~1;
		}

		/* XXX not implemented
		    bit 1: card test?
		 */

		m_io[0x0e] = data;
		break;

	case 0x0f:
		LOG(("%s: card info write %02x\n", machine().describe_context(), data));
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
		rs232_send_byte();
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


uint8_t hp48_state::io_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	/* CRC register */
	case 0x04: data = m_crc & 0xf; break;
	case 0x05: data = (m_crc >> 4) & 0xf; break;
	case 0x06: data = (m_crc >> 8) & 0xf; break;
	case 0x07: data = (m_crc >> 12) & 0xf; break;

	/* battery test */
	case 0x08:
		data = 0;
		if (m_io[0x9] & 8) /* test enable */
		{
			/* XXX not implemented:
			   bit 3: battery in port 2
			   bit 2: battery in port 1
			 */
			switch (ioport("BATTERY")->read())
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
		int cur_line = m_screen->vpos();
		if (last_line <= 1) last_line = 0x3f;
		data = (cur_line >= 0 && cur_line <= last_line) ? last_line - cur_line : 0;
		if (offset == 0x29)
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
		m_io[0x11] &= ~1;  /* clear byte received */
		data = m_io[offset];
		break;
	}

	/* cards */
	case 0x0e: /* detection */
		data = m_io[0x0e];
		LOG(( "%s: card control read %02x\n", machine().describe_context(), data ));
		break;
	case 0x0f: /* card info */
		data = 0;
		if ( HP48_G_SERIES )
		{
			if (m_port[1].found() && m_port[1]->port_size() > 0)
			{
				data |= 1;
				if (m_port[1]->port_write())
					data |= 4;
			}
			if (m_port[0].found() && m_port[0]->port_size() > 0)
			{
				data |= 2;
				if (m_port[0]->port_write())
					data |= 8;
			}
		}
		else
		{
			if (m_port[0].found() && m_port[0]->port_size() > 0)
			{
				data |= 1;
				if (m_port[0]->port_write())
					data |= 4;
			}
			if (m_port[1].found() && m_port[1]->port_size() > 0)
			{
				data |= 2;
				if (m_port[1]->port_write())
					data |= 8;
			}
		}
		LOG(( "%s: card info read %02x\n", machine().describe_context(), data ));
		break;


	default: data = m_io[offset];
	}

	LOG(("%s %f hp48_state::io_r: off=%02x data=%x\n", machine().describe_context(), machine().time().as_double(), offset, data));
	return data;
}


/* ---------- bank switcher --------- */

uint8_t hp48_state::bank_r(offs_t offset)
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
		LOG(( "%s %f hp48_state::bank_r: off=%03x\n", machine().describe_context(), machine().time().as_double(), offset ));
		m_bank_switch = offset;
		apply_modules();
	}
	return 0;
}


void hp48_state::hp49_bank_w(offs_t offset, uint8_t data)
{
	offset &= 0x7e;
	if ( m_bank_switch != offset )
	{
		LOG(("%s %f hp49_bank_w: off=%03x\n", machine().describe_context(), machine().time().as_double(), offset));
		m_bank_switch = offset;
		apply_modules();
	}
}



/* ---------------- timers --------------- */

TIMER_CALLBACK_MEMBER(hp48_state::timer1_cb)
{
	if (!(m_io[0x2f] & 1)) return; /* timer enable */

	m_timer1 = (m_timer1 - 1) & 0xf;

	/* wake-up on carry */
	if ((m_io[0x2e] & 4) && (m_timer1 == 0xf))
	{
		LOG(("wake-up on timer1\n"));
		m_io[0x2e] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		pulse_irq(SATURN_WAKEUP_LINE);
	}
	/* interrupt on carry */
	if ((m_io[0x2e] & 2) && (m_timer1 == 0xf))
	{
		LOG(("generate timer1 interrupt\n"));
		m_io[0x2e] |= 8; /* set service request */
		m_io[0x18] |= 4; /* set service request */
		pulse_irq(SATURN_NMI_LINE);
	}
}

TIMER_CALLBACK_MEMBER(hp48_state::timer2_cb)
{
	if (!(m_io[0x2f] & 1)) return; /* timer enable */

	m_timer2 = (m_timer2 - 1) & 0xffffffff;

	/* wake-up on carry */
	if ((m_io[0x2f] & 4) && (m_timer2 == 0xffffffff))
	{
		LOG(("wake-up on timer2\n"));
		m_io[0x2f] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		pulse_irq(SATURN_WAKEUP_LINE);
	}
	/* interrupt on carry */
	if ((m_io[0x2f] & 2) && (m_timer2 == 0xffffffff))
	{
		LOG(("generate timer2 interrupt\n"));
		m_io[0x2f] |= 8;                                      /* set service request */
		m_io[0x18] |= 4;                                      /* set service request */
		pulse_irq(SATURN_NMI_LINE);
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
void hp48_state::apply_modules()
{
	int nce3_enable = 1;
	address_space& space = m_maincpu->space(AS_PROGRAM);

	m_io_addr = 0x100000;

	/* NCE1 (ROM) is a bit special, so treat it separately */
	space.unmap_readwrite(0, 0xfffff);
	if (HP49_G_MODEL)
	{
		int bank_lo = (m_bank_switch >> 5) & 3;
		int bank_hi = (m_bank_switch >> 1) & 15;
		LOG(("hp48_state::apply_modules: low ROM bank is %i\n", bank_lo));
		LOG(("hp48_state::apply_modules: high ROM bank is %i\n", bank_hi));
		if (m_rom)
		{
			space.install_rom(0x00000, 0x3ffff, 0x80000, m_rom + bank_lo * 0x40000);
			space.install_rom(0x40000, 0x7ffff, 0x80000, m_rom + bank_hi * 0x40000);
		}
	}
	else if (HP48_G_SERIES)
	{
		/* port 2 bank switch */
		if (m_port[1].found() && m_port[1]->port_size() > 0)
		{
			int off = (m_bank_switch << 16) % m_port[1]->port_size();
			LOG(("hp48_state::apply_modules: port 2 offset is %i\n", off));
			m_modules[HP48_NCE3].data = m_port[1]->port_data() + off;
		}

		/* ROM A19 (hi 256 KB) / NCE3 (port 2) control switch */
		if (m_io[0x29] & 8)
		{
			/* A19 */
			LOG(("hp48_state::apply_modules: A19 enabled, NCE3 disabled\n"));
			nce3_enable = 0;
			if (m_rom)
				space.install_rom(0, 0xfffff, m_rom);
		}
		else
		{
			/* NCE3 */
			nce3_enable = m_bank_switch >> 6;
			LOG(("hp48_apply_modules: A19 disabled, NCE3 %s\n", nce3_enable ? "enabled" : "disabled"));
			if (m_rom)
				space.install_rom(0, 0x7ffff, 0x80000, m_rom);
		}
	}
	else
	{
		if (m_rom)
			space.install_rom(0, 0x7ffff, 0x80000, m_rom);
	}


	/* from lowest to highest priority */
	for (int i = 4; i >= 0; i--)
	{
		uint32_t select_mask = m_modules[i].mask;
		uint32_t nselect_mask = ~select_mask & 0xfffff;
		uint32_t base = m_modules[i].base;
		uint32_t off_mask = m_modules[i].off_mask;
		uint32_t mirror = nselect_mask & ~off_mask;
		uint32_t end = base + (off_mask & nselect_mask);

		if (m_modules[i].state != HP48_MODULE_CONFIGURED) continue;
		if ((i == 4) && !nce3_enable) continue;

		/* our code assumes that the 20-bit select_mask is all 1s followed by all 0s */
		if (nselect_mask & (nselect_mask + 1))
		{
			logerror("hp48_apply_modules: invalid mask %05x for module %s\n", select_mask, hp48_module_names[i]);
			continue;
		}

		if (m_modules[i].data)
		{
			space.install_rom(base, end, mirror, m_modules[i].data);
		}
		else
		{
			if (!m_modules[i].read.isnull())
			{
				space.install_read_handler(base, end, 0, mirror, 0, m_modules[i].read);
			}
		}

		if (m_modules[i].isnop)
		{
			space.nop_write(base, end | mirror);
		}
		else
		{
			if (m_modules[i].data)
			{
				space.install_writeonly(base, end, mirror, m_modules[i].data);
			}
			else
			{
				if (!m_modules[i].write.isnull())
				{
					space.install_write_handler(base, end, 0, mirror, 0, m_modules[i].write);
				}
			}
		}

		LOG(("hp48_apply_modules: module %s configured at %05x-%05x, mirror %05x\n", hp48_module_names[i], base, end, mirror));

		if (i == 0)
		{
			m_io_addr = base;
		}
	}
}


/* reset the configuration */
void hp48_state::reset_modules()
{
	/* fixed size for HDW */
	m_modules[HP48_HDW].state = HP48_MODULE_MASK_KNOWN;
	m_modules[HP48_HDW].mask = 0xfffc0;
	/* unconfigure NCE2, CE1, CE2, NCE3 */
	for (int i = 1; i < 5; i++)
	{
		m_modules[i].state = HP48_MODULE_UNCONFIGURED;
	}

	/* fixed configuration for NCE1 */
	m_modules[HP48_NCE1].state = HP48_MODULE_CONFIGURED;
	m_modules[HP48_NCE1].base = 0;
	m_modules[HP48_NCE1].mask = 0;

	apply_modules();
}


/* RESET opcode */
WRITE_LINE_MEMBER( hp48_state::mem_reset )
{
	LOG(("%s %f hp48_state::mem_reset\n", machine().describe_context(), machine().time().as_double()));
	reset_modules();
}


/* CONFIG opcode */
void hp48_state::mem_config(uint32_t data)
{
	LOG(("%s %f hp48_state::mem_config: %05x\n", machine().describe_context(), machine().time().as_double(), data));

	/* find the highest priority unconfigured module (except non-configurable NCE1)... */
	for (int i = 0; i < 5; i++)
	{
		/* ... first call sets the address mask */
		if (m_modules[i].state == HP48_MODULE_UNCONFIGURED)
		{
			m_modules[i].mask = data & 0xff000;
			m_modules[i].state = HP48_MODULE_MASK_KNOWN;
			break;
		}

		/* ... second call sets the base address */
		if (m_modules[i].state == HP48_MODULE_MASK_KNOWN)
		{
			m_modules[i].base = data & m_modules[i].mask;
			m_modules[i].state = HP48_MODULE_CONFIGURED;
			LOG(("hp48_mem_config: module %s configured base=%05x, mask=%05x\n", hp48_module_names[i], m_modules[i].base, m_modules[i].mask));
			apply_modules();
			break;
		}
	}
}


/* UNCFG opcode */
void hp48_state::mem_unconfig(uint32_t data)
{
	LOG(("%s %f hp48_state::mem_unconfig: %05x\n", machine().describe_context(), machine().time().as_double(), data));

	/* find the highest priority fully configured module at address v (except NCE1)... */
	for (int i = 0; i < 5; i++)
	{
		/* ... and unconfigure it */
		if (m_modules[i].state == HP48_MODULE_CONFIGURED && (m_modules[i].base == (data & m_modules[i].mask)))
		{
			m_modules[i].state = i > 0 ? HP48_MODULE_UNCONFIGURED : HP48_MODULE_MASK_KNOWN;
			LOG(("hp48_mem_unconfig: module %s\n", hp48_module_names[i]));
			apply_modules();
			break;
		}
	}
}


/* C=ID opcode */
uint32_t hp48_state::mem_id()
{
	int data = 0; /* 0 = everything is configured */

	/* find the highest priority unconfigured module (except NCE1)... */
	for (int i = 0; i < 5; i++)
	{
		/* ... mask need to be configured */
		if (m_modules[i].state == HP48_MODULE_UNCONFIGURED)
		{
			data = hp48_module_mask_id[i] | (m_modules[i].mask & ~0xff);
			break;
		}

		/* ... address need to be configured */
		if (m_modules[i].state == HP48_MODULE_MASK_KNOWN)
		{
			data = hp48_module_addr_id[i] | (m_modules[i].base & ~0x3f);
			break;
		}
	}

	LOG(("%s %f mem_id = %02x\n", machine().describe_context(), machine().time().as_double(), data));

	return data; /* everything is configured */
}



/* --------- CRC ---------- */

/* each memory read by the CPU updates the internal CRC state */
void hp48_state::mem_crc(offs_t offset, uint32_t data)
{
	/* no CRC for I/O RAM */
	if (offset >= m_io_addr && offset < m_io_addr + 0x40) return;

	m_crc = (m_crc >> 4) ^ (((m_crc ^ data) & 0xf) * 0x1081);
}



/* ------ utilities ------- */


/* decodes size bytes into 2*size nibbles (least significant first) */
void hp48_state::decode_nibble(uint8_t* dst, uint8_t* src, int size)
{
	for (int i = size - 1; i >= 0; i--)
	{
		dst[2 * i + 1] = src[i] >> 4;
		dst[2 * i] = src[i] & 0xf;
	}
}

/* inverse of decode_nibble  */
void hp48_state::encode_nibble(uint8_t* dst, uint8_t* src, int size)
{
	for (int i = 0; i < size; i++)
	{
		dst[i] = (src[2 * i] & 0xf) | (src[2 * i + 1] << 4);
	}
}

/***************************************************************************
    MACHINES
***************************************************************************/

void hp48_state::init_hp48()
{
	LOG(( "hp48: driver init called\n" ));
	for (int i = 0; i < 6; i++)
	{
		m_modules[i].off_mask = 0x00fff;  // 2 KB
		m_modules[i].read     = read8sm_delegate(*this);
		m_modules[i].write    = write8sm_delegate(*this);
		m_modules[i].data     = nullptr;
		m_modules[i].isnop    = 0;
		m_modules[i].state    = 0;
		m_modules[i].base     = 0;
		m_modules[i].mask     = 0;
	}
	m_rom = nullptr;
}

void hp48_state::machine_reset()
{
	LOG(("hp48: machine reset called\n"));
	m_bank_switch = 0;
	m_cur_screen = 0;
	reset_modules();
	update_annunciators();
}

void hp48_state::base_machine_start(hp48_models model)
{
	LOG(( "hp48_state::machine_start: model %i\n", model ));

	m_model = model;

	/* internal RAM */
	uint32_t ram_size =
		HP49_G_MODEL  ? (512 * 1024) :
		HP48_GX_MODEL ? (128 * 1024) : (32 * 1024);

	m_allocated_ram = std::make_unique<uint8_t[]>(2 * ram_size);
	subdevice<nvram_device>("nvram")->set_base(m_allocated_ram.get(), 2 * ram_size);


	/* ROM load */
	uint32_t rom_size =
		HP49_G_MODEL  ? (2048 * 1024) :
		HP48_S_SERIES ?  (256 * 1024) : (512 * 1024);
	m_allocated_rom = std::make_unique<uint8_t[]>(2 * rom_size);
	decode_nibble(m_allocated_rom.get(), memregion("maincpu")->base(), rom_size);
	m_rom = m_allocated_rom.get();

	/* init state */
	std::fill_n(&m_allocated_ram[0], 2 * ram_size, 0);
	memset(m_io, 0, sizeof(m_io));
	m_out = 0;
	m_kdn = 0;
	m_crc = 0;
	m_timer1 = 0;
	m_timer2 = 0;
	m_bank_switch = 0;

	/* I/O RAM */
	m_modules[HP48_HDW].off_mask = 0x0003f;  /* 32 B */
	m_modules[HP48_HDW].read     = read8sm_delegate(*this, FUNC(hp48_state::io_r));
	m_modules[HP48_HDW].write    = write8sm_delegate(*this, FUNC(hp48_state::io_w));

	/* internal RAM */
	if (HP49_G_MODEL)
	{
		m_modules[HP48_NCE2].off_mask = 2 * 256 * 1024 - 1;
		m_modules[HP48_NCE2].data     = &m_allocated_ram[0];
		m_modules[HP48_CE2].off_mask  = 2 * 128 * 1024 - 1;
		m_modules[HP48_CE2].data      = &m_allocated_ram[2 * 256 * 1024];
		m_modules[HP48_NCE3].off_mask = 2 * 128 * 1024 - 1;
		m_modules[HP48_NCE3].data     = &m_allocated_ram[2 * (128+256) * 1024];
	}
	else
	{
		m_modules[HP48_NCE2].off_mask = 2 * ram_size - 1;
		m_modules[HP48_NCE2].data     = &m_allocated_ram[0];
	}

	/* bank switcher */
	if (HP48_G_SERIES)
	{
		m_modules[HP48_CE1].off_mask = 0x00fff;  /* 2 KB */
		m_modules[HP48_CE1].read     = read8sm_delegate(*this, FUNC(hp48_state::bank_r));
		m_modules[HP48_CE1].write    = HP49_G_MODEL ? write8sm_delegate(*this, FUNC(hp48_state::hp49_bank_w)) : write8sm_delegate(*this);
	}

	/* timers */
	m_1st_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hp48_state::timer1_cb), this));
	m_1st_timer->adjust(attotime::from_hz(16), 0, attotime::from_hz(16));

	m_2nd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hp48_state::timer2_cb), this));
	m_2nd_timer->adjust(attotime::from_hz(8192), 0, attotime::from_hz(8192));

	/* 1ms keyboard polling */
	m_kbd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hp48_state::kbd_cb), this));
	m_kbd_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));

	m_lshift0.resolve();
	m_rshift0.resolve();
	m_alpha0.resolve();
	m_alert0.resolve();
	m_busy0.resolve();
	m_transmit0.resolve();

	/* save state */
	save_item(NAME(m_out));
	save_item(NAME(m_kdn));
	save_item(NAME(m_io_addr));
	save_item(NAME(m_crc));
	save_item(NAME(m_timer1));
	save_item(NAME(m_timer2));
	save_item(NAME(m_bank_switch));
	for (int i = 0; i < 6; i++)
	{
		save_item(m_modules[i].state, "globals/m_modules[i].state", i);
		save_item(m_modules[i].base, "globals/m_modules[i].base", i);
		save_item(m_modules[i].mask, "globals/m_modules[i].mask", i);
	}
	save_item(NAME(m_io) );
	machine().save().register_postload(save_prepost_delegate(FUNC(hp48_state::update_annunciators), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(hp48_state::apply_modules), this));

}

MACHINE_START_MEMBER(hp48_state,hp48s)
{
	base_machine_start(HP48_S);
}

MACHINE_START_MEMBER(hp48_state,hp48sx)
{
	base_machine_start(HP48_SX);
}

MACHINE_START_MEMBER(hp48_state,hp48g)
{
	base_machine_start(HP48_G);
}

MACHINE_START_MEMBER(hp48_state,hp48gx)
{
	base_machine_start(HP48_GX);
}

MACHINE_START_MEMBER(hp48_state,hp48gp)
{
	base_machine_start(HP48_GP);
}

MACHINE_START_MEMBER(hp48_state,hp49g)
{
	base_machine_start(HP49_G);
}
