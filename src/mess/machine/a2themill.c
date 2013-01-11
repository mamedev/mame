/*********************************************************************

    a2themill.c

    Implementation of the Stellation Two The Mill 6809 card

    The OS9 add-on changes the address mapping as follows:
    6809 0x0000-0xafff -> 6502 0x1000-0xbfff
    6809 0xb000-0xdfff -> 6502 0xd000-0xffff
    6809 0xe000-0xefff -> 6502 0xc000-0xcfff
    6809 0xf000-0xffff -> 6502 0x0000-0x0fff

    (reference: http://mirrors.apple2.org.za/ground.icaen.uiowa.edu/MiscInfo/Hardware/mill.6809 )

    ProDOS "Stellation The Mill Disk.po" requires Mill in slot 2; boot
    the disc and type "-DEMO1" and press Enter to launch the simple demo.

    The OS9 disk image available around the internet seems to be bad - the
    6809 boot vector is 0x4144 which maps to 6502 0x5144 and there's all
    zeros from 6502 1000-8fff.  There is valid 6809 code from 9000-BFFF
    at the point where it wants to boot the 6809, but I don't know what
    is supposed to be the entry point.

*********************************************************************/

#include "a2themill.h"
#include "includes/apple2.h"
#include "cpu/m6809/m6809.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define MILL_VERBOSE (0)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_THEMILL = &device_creator<a2bus_themill_device>;

#define M6809_TAG         "m6809"

static ADDRESS_MAP_START( m6809_mem, AS_PROGRAM, 8, a2bus_themill_device )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(dma_r, dma_w)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( a2themill )
	MCFG_CPU_ADD(M6809_TAG, M6809, 1021800)   // M6809 runs at ~1 MHz as per Stellation Two's print ads
	MCFG_CPU_PROGRAM_MAP(m6809_mem)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_themill_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2themill );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_themill_device::a2bus_themill_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_6809(*this, M6809_TAG),
	m_6502space(NULL)
{
	m_shortname = "a2themill";
}

a2bus_themill_device::a2bus_themill_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_THEMILL, "Stellation Two The Mill", tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_6809(*this, M6809_TAG),
	m_6502space(NULL)
{
	m_shortname = "a2themill";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_themill_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	save_item(NAME(m_bEnabled));
	save_item(NAME(m_flipAddrSpace));
	save_item(NAME(m_6809Mode));
	save_item(NAME(m_status));
}

void a2bus_themill_device::device_reset()
{
	m_bEnabled = false;
	m_6502space = NULL;
	m_flipAddrSpace = false;
	m_6809Mode = false;
	m_status = 0xc0;    // OS9 loader relies on this
	m_6809->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_6809->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

UINT8 a2bus_themill_device::read_c0nx(address_space &space, UINT8 offset)
{
	return m_status;
}

void a2bus_themill_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0: // 6502 IRQ
			if (data & 0x80)
			{
				m_status |= 0x01;
				lower_slot_irq();
			}
			else
			{
				m_status &= ~0x01;
				raise_slot_irq();
			}
			break;

		case 2: // 6809 reset
			if (data & 0x80)
			{
				// steal the 6502's address space
				m_6502space = &space;
				m_6809->reset();

				m_6809->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_6809->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

				m_bEnabled = true;
				m_status &= ~0x04;
			}
			else
			{
				m_6809->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				m_bEnabled = false;
				m_status |= 0x04;
			}
			break;

		case 1: // 6809 halt
			if (data & 0x80)    // release reset
			{
				m_status |= 0x02;
			}
			else
			{
				m_6809->reset();
				m_status &= ~0x02;
			}
			break;

		case 3: // 6809 NMI
			if (data & 0x80)
			{
				m_6809->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
				m_status |= 0x08;
			}
			else
			{
				m_6809->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
				m_status &= ~0x08;
			}
			break;

		case 4: // 6809 FIRQ
			if (data & 0x80)
			{
				m_6809->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
				m_status |= 0x10;
			}
			else
			{
				m_6809->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
				m_status &= ~0x10;
			}
			break;

		case 5: // 6809 IRQ
			if (data & 0x80)
			{
				m_6809->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
				m_status |= 0x20;
			}
			else
			{
				m_6809->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
				m_status &= ~0x20;
			}
			break;

		case 6:
			if (data & 0x80)    // enable ROM socket
			{
				m_status |= 0x40;
				printf("The Mill: on-board ROM socket enabled; because none of these ROMs are dumped, the 6809 will not run!\n");
			}
			else
			{
				m_status &= ~0x40;
			}
			break;

		case 7: // 6809 mapping
			if (data & 0x80)
			{
				m_status |= 0x80;
				m_flipAddrSpace = false;
			}
			else
			{
				m_status &= ~0x80;
				m_flipAddrSpace = true;
			}
			break;

		case 0xa:   // addresses >= 0x8 are direct status writes?  "Excel Flex 9" disc seems to indicate so.
			m_status = data;
			break;

		default:
			printf("The Mill: %02x to unhandled c0n%x\n", data, offset);
			break;
	}
}

READ8_MEMBER( a2bus_themill_device::dma_r )
{
	if (m_6502space)
	{
		if (m_6809Mode)
		{
			if (offset <= 0xafff)
			{
				return m_6502space->read_byte(offset+0x1000);
			}
			else if (offset <= 0xbfff)
			{
				return m_6502space->read_byte((offset&0xfff) + 0xd000);
			}
			else if (offset <= 0xcfff)
			{
				return m_6502space->read_byte((offset&0xfff) + 0xe000);
			}
			else if (offset <= 0xdfff)
			{
				return m_6502space->read_byte((offset&0xfff) + 0xf000);
			}
			else if (offset <= 0xefff)
			{
				return m_6502space->read_byte((offset&0xfff) + 0xc000);
			}
			else    // 6809 Fxxx -> 6502 ZP
			{
				return m_6502space->read_byte(offset&0xfff);
			}
		}
		else
		{
			if (m_flipAddrSpace)
			{
				return m_6502space->read_byte(offset^0x8000);
			}
			else
			{
				return m_6502space->read_byte(offset);
			}
		}
	}

	return 0xff;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( a2bus_themill_device::dma_w )
{
	if (m_6502space)
	{
		if (m_6809Mode)
		{
			if (offset <= 0xafff)
			{
				m_6502space->write_byte(offset+0x1000, data);
			}
			else if (offset <= 0xbfff)
			{
				m_6502space->write_byte((offset&0xfff) + 0xd000, data);
			}
			else if (offset <= 0xcfff)
			{
				m_6502space->write_byte((offset&0xfff) + 0xe000, data);
			}
			else if (offset <= 0xdfff)
			{
				m_6502space->write_byte((offset&0xfff) + 0xf000, data);
			}
			else if (offset <= 0xefff)
			{
				m_6502space->write_byte((offset&0xfff) + 0xc000, data);
			}
			else    // 6809 Fxxx -> 6502 ZP
			{
				m_6502space->write_byte(offset&0xfff, data);
			}
		}
		else
		{
			if (m_flipAddrSpace)
			{
				m_6502space->write_byte(offset^0x8000, data);
			}
			else
			{
				m_6502space->write_byte(offset, data);
			}
		}
	}
}

bool a2bus_themill_device::take_c800()
{
	return false;
}
