/******************************************************************************

    compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

 ******************************************************************************/

/*-------------------------------------------------------------------------*/
/* Include files                                                           */
/*-------------------------------------------------------------------------*/

#include "includes/compis.h"

/*-------------------------------------------------------------------------*/
/* Defines, constants, and global variables                                */
/*-------------------------------------------------------------------------*/

/* Keyboard */
static const UINT8 compis_keyb_codes[6][16] = {
{0x39, 0x32, 0x29, 0x20, 0x17, 0x0e, 0x05, 0x56, 0x4d, 0x44, 0x08, 0x57, 0x59, 0x4e, 0x43, 0x3a},
{0x31, 0x28, 0x1f, 0x16, 0x0d, 0x04, 0x55, 0x4c, 0x4f, 0x58, 0x00, 0x07, 0xff, 0x42, 0x3b, 0x30},
{0x27, 0x1e, 0x15, 0x0c, 0x03, 0x54, 0x06, 0x50, 0x01, 0xfe, 0x38, 0x2f, 0x26, 0x1d, 0x14, 0x0b},
{0x02, 0x53, 0x4a, 0x48, 0x51, 0xfe, 0xfd, 0xfc, 0x40, 0xfc, 0xfd, 0x0f, 0x18, 0x21, 0x22, 0x34},
{0x10, 0x11, 0x1a, 0x23, 0x1b, 0x2d, 0x04, 0x0a, 0x13, 0x1c, 0x2a, 0x3c, 0x33, 0x19, 0x46, 0x2b},
{0x2c, 0x3e, 0x35, 0x12, 0x3f, 0x24, 0x25, 0x37, 0x2e, 0x45, 0x3d, 0x47, 0x36, 0x00, 0x00, 0x00}
};

enum COMPIS_KEYB_SHIFT
{
	KEY_CAPS_LOCK = 0xff,
	KEY_SHIFT = 0xfe,
	KEY_SUPER_SHIFT = 0xfd,
	KEY_CTRL = 0xfc
};

enum COMPIS_USART_STATES
{
	COMPIS_USART_STATUS_TX_READY = 0x01,
	COMPIS_USART_STATUS_RX_READY = 0x02,
	COMPIS_USART_STATUS_TX_EMPTY = 0x04
};

/* Compis interrupt handling */
enum COMPIS_INTERRUPTS
{
	COMPIS_NMI_KEYB     = 0x00, /* Default not used */
	COMPIS_INT_8251_TXRDY   = 0x01, /* Default not used */
	COMPIS_INT_8274     = 0x03
};

enum COMPIS_INTERRUPT_REQUESTS
{
	COMPIS_IRQ_SBX0_INT1     = 0x00,
	COMPIS_IRQ_SBX0_INT0     = 0x01, /* Default not used */
	COMPIS_IRQ_8251_RXRDY    = 0x02,
	COMPIS_IRQ_80150_SYSTICK = 0x03, /* Default not used */
	COMPIS_IRQ_ACK_J7    = 0x04,
	COMPIS_IRQ_SBX1_INT1     = 0x05, /* Default not used */
	COMPIS_IRQ_SBX1_INT0     = 0x06, /* Default not used */
	COMPIS_IRQ_80150_DELAY   = 0x07
};

/* Main emulation */

/*-------------------------------------------------------------------------*/
/*  Keyboard                                                               */
/*-------------------------------------------------------------------------*/
// TODO: make the low level keyboard emulation work
void compis_state::compis_keyb_update()
{
	UINT8 key_code;
	UINT8 key_status;
	UINT8 irow;
	UINT8 icol;
	UINT16 data;
	UINT16 ibit;
	static const char *const rownames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5" };

	key_code = 0;
	key_status = 0x80;

	for (irow = 0; irow < 6; irow++)
	{
		data = machine().root_device().ioport(rownames[irow])->read();
		if (data != 0xffff)
		{
			ibit = 1;
			for (icol = 0; icol < 16; icol++)
			{
				if (!(data & ibit))
				{
					switch(compis_keyb_codes[irow][icol])
					{
						case KEY_SHIFT:
							key_status |= 0x01;
							break;
						case KEY_CAPS_LOCK:
							key_status |= 0x02;
							break;
						case KEY_CTRL:
							key_status |= 0x04;
							break;
						case KEY_SUPER_SHIFT:
							key_status |= 0x08;
							break;
						default:
							key_code = compis_keyb_codes[irow][icol];
					}
				}
				ibit <<= 1;
			}
		}
	}
	if (key_code != 0)
	{
		m_compis.keyboard.key_code = key_code;
		m_compis.keyboard.key_status = key_status;
		m_compis.usart.bytes_sent = 1;
		m_uart->receive_character(key_code);
	}
}

void compis_state::compis_keyb_init()
{
	m_compis.keyboard.key_code = 0;
	m_compis.keyboard.key_status = 0x80;
	m_compis.usart.bytes_sent = 0;
}

/*-------------------------------------------------------------------------*/
/*  FDC iSBX-218A                                                          */
/*-------------------------------------------------------------------------*/
void compis_state::compis_fdc_reset()
{
	m_fdc->reset();
}

void compis_state::compis_fdc_tc(int state)
{
	m_fdc->tc_w(state);
}

void compis_state::fdc_irq(bool state)
{
	m_8259m->ir0_w(state);
}

void compis_state::fdc_drq(bool state)
{
	/* DMA request if iSBX-218A has DMA enabled */
	if (ioport("DSW1")->read() && state)
	{
		//compis_dma_drq(state, read);
	}
}

WRITE8_MEMBER(compis_state::fdc_mon_w)
{
	m_mon = data & 1;

	m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(m_mon);
	m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(m_mon);
}

READ8_MEMBER(compis_state::fdc_mon_r)
{
	return m_mon;
}
/*-------------------------------------------------------------------------*/
/* Bit 0: J5-4                                                             */
/* Bit 1: J5-5                                                     */
/* Bit 2: J6-3 Cassette read                                               */
/* Bit 3: J2-6 DSR / S8-4 Test                                             */
/* Bit 4: J4-6 DSR / S8-3 Test                                             */
/* Bit 5: J7-11 Centronics BUSY                                            */
/* Bit 6: J7-13 Centronics SELECT                              */
/* Bit 7: Tmr0                                                     */
/*-------------------------------------------------------------------------*/
READ8_MEMBER( compis_state::compis_ppi_port_b_r )
{
	UINT8 data;

	/* DIP switch - Test mode */
	data = ioport("DSW0")->read();

	/* Centronics busy */
	data |= m_centronics->busy_r() << 5;
	data |= m_centronics->vcc_r() << 6;

	return data;
}

/*-------------------------------------------------------------------------*/
/* Bit 0: J5-1                                                         */
/* Bit 1: J5-2                                                         */
/* Bit 2: Select: 1=time measure, DSR from J2/J4 pin 6. 0=read cassette    */
/* Bit 3: Datex: Tristate datex output (low)                   */
/* Bit 4: V2-5 Floppy motor on/off                                     */
/* Bit 5: J7-1 Centronics STROBE                                       */
/* Bit 6: V2-4 Floppy Soft reset                               */
/* Bit 7: V2-3 Floppy Terminal count                                   */
/*-------------------------------------------------------------------------*/
WRITE8_MEMBER( compis_state::compis_ppi_port_c_w )
{
	/* Centronics Strobe */
	m_centronics->strobe_w(BIT(data, 5));

	/* FDC Reset */
	if (BIT(data, 6))
		compis_fdc_reset();

	/* FDC Terminal count */
	compis_fdc_tc(BIT(data, 7));
}

I8255A_INTERFACE( compis_ppi_interface )
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write),
	DEVCB_DRIVER_MEMBER(compis_state, compis_ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(compis_state, compis_ppi_port_c_w)
};


/*-------------------------------------------------------------------------*/
/*  PIT 8253                                                               */
/*-------------------------------------------------------------------------*/

const struct pit8253_interface compis_pit8253_config =
{
	{
		/* Timer0 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL },
		/* Timer1 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL },
		/* Timer2 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL }
	}
};

const struct pit8253_interface compis_pit8254_config =
{
	{
		/* Timer0 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL },
		/* Timer1 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL },
		/* Timer2 */
		{4770000/4, DEVCB_NULL, DEVCB_NULL }
	}
};

/*-------------------------------------------------------------------------*/
/*  OSP PIT 8254                                                           */
/*-------------------------------------------------------------------------*/

READ16_MEMBER( compis_state::compis_osp_pit_r )
{
	return m_8254->read(space, offset);
}

WRITE16_MEMBER( compis_state::compis_osp_pit_w )
{
	m_8254->write(space, offset, data);
}


/*-------------------------------------------------------------------------*/
/*  USART 8251                                                             */
/*-------------------------------------------------------------------------*/
const i8251_interface compis_usart_interface=
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir2_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER( compis_state::compis_usart_r )
{
	switch (offset)
	{
		case 0x00:
		{
			UINT8 data = m_uart->data_r(space, 0);
			if(m_compis.usart.bytes_sent == 1)
			{
				m_compis.usart.bytes_sent = 2;
				m_uart->receive_character(m_compis.keyboard.key_status);
			}
			return data;
		}
		case 0x01:
			return m_uart->status_r(space, 0);
			break;
		default:
			logerror("USART Unknown Port Read %04X\n", offset);
			break;
	}
	return 0;
}

WRITE8_MEMBER( compis_state::compis_usart_w )
{
	switch (offset)
	{
		case 0x00:
			m_uart->data_w(space, 0, data);
			break;
		case 0x01:
			m_uart->control_w(space, 0, data);
			break;
		default:
			logerror("USART Unknown Port Write %04X = %04X\n", offset, data);
			break;
	}
}

/*-------------------------------------------------------------------------*/
/* Name: compis                                                            */
/* Desc: Driver - Init                                                     */
/*-------------------------------------------------------------------------*/

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

READ8_MEMBER(compis_state::compis_irq_callback)
{
	return m_8259m->inta_r();
}


DRIVER_INIT_MEMBER(compis_state,compis)
{
	memset (&m_compis, 0, sizeof (m_compis) );
}

void compis_state::machine_start()
{
	m_fdc->setup_intrq_cb(i8272a_device::line_cb(FUNC(compis_state::fdc_irq), this));
	m_mon = true;
}
/*-------------------------------------------------------------------------*/
/* Name: compis                                                            */
/* Desc: Machine - Init                                                    */
/*-------------------------------------------------------------------------*/
void compis_state::machine_reset()
{
	/* FDC */
	compis_fdc_reset();

	/* Keyboard */
	compis_keyb_init();
}

/*-------------------------------------------------------------------------*/
/* Name: compis                                                            */
/* Desc: Interrupt - Vertical Blanking Interrupt                           */
/*-------------------------------------------------------------------------*/
INTERRUPT_GEN_MEMBER(compis_state::compis_vblank_int)
{
//  compis_gdc_vblank_int();
	compis_keyb_update();
}
