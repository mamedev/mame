/***************************************************************************

  ISA 8 bit Generic Communication Card

***************************************************************************/

#include "com.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/ser_mouse.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
#include "machine/ins8250.h"

static const ins8250_interface genpc_com_interface[2]=
{
	{
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_rts),
		DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, isa8_com_device, pc_com_interrupt_1),
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_rts),
		DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, isa8_com_device, pc_com_interrupt_2),
		DEVCB_NULL,
		DEVCB_NULL
	}/*,
    {
        DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_txd),
        DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_dtr),
        DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_rts),
        DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, isa8_com_device, pc_com_interrupt_1),
        DEVCB_NULL,
        DEVCB_NULL
    },
    {
        DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_txd),
        DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_dtr),
        DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_rts),
        DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, isa8_com_device, pc_com_interrupt_2),
        DEVCB_NULL,
        DEVCB_NULL
    }*/
};

static SLOT_INTERFACE_START(isa_com)
	SLOT_INTERFACE("microsoft_mouse", MSFT_SERIAL_MOUSE)
	SLOT_INTERFACE("msystems_mouse", MSYSTEM_SERIAL_MOUSE)
	SLOT_INTERFACE("terminal", SERIAL_TERMINAL)
	SLOT_INTERFACE("null_modem", NULL_MODEM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( com_config )
	MCFG_INS8250_ADD( "uart_0", genpc_com_interface[0], XTAL_1_8432MHz )
	MCFG_INS8250_ADD( "uart_1", genpc_com_interface[1], XTAL_1_8432MHz )
	//MCFG_INS8250_ADD( "uart_2", genpc_com_interface[2], XTAL_1_8432MHz )
	//MCFG_INS8250_ADD( "uart_3", genpc_com_interface[3], XTAL_1_8432MHz )

	MCFG_RS232_PORT_ADD( "serport0", isa_com, "microsoft_mouse" )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", isa_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, cts_w))

	//MCFG_RS232_PORT_ADD( "serport2", isa_com, NULL )
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, rx_w))
	//MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dcd_w))
	//MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dsr_w))
	//MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, ri_w))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, cts_w))

	//MCFG_RS232_PORT_ADD( "serport3", isa_com, NULL )
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_2", ins8250_uart_device, rx_w))
	//MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_2", ins8250_uart_device, dcd_w))
	//MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_2", ins8250_uart_device, dsr_w))
	//MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_2", ins8250_uart_device, ri_w))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_2", ins8250_uart_device, cts_w))
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_COM = &device_creator<isa8_com_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_com_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( com_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

isa8_com_device::isa8_com_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8_COM, "Communications Adapter PC/XT", tag, owner, clock, "isa_com", __FILE__),
		device_isa8_card_interface(mconfig, *this)
{
}

isa8_com_device::isa8_com_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_isa8_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_com_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f8, 0x03ff, 0, 0, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_0")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_0")) );
	m_isa->install_device(0x02f8, 0x02ff, 0, 0, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_1")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_1")) );
//  m_isa->install_device(0x03e8, 0x03ef, 0, 0, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_2")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_2")) );
//  m_isa->install_device(0x02e8, 0x02ef, 0, 0, read8_delegate(FUNC(ins8250_device::ins8250_r), subdevice<ins8250_uart_device>("uart_3")), write8_delegate(FUNC(ins8250_device::ins8250_w), subdevice<ins8250_uart_device>("uart_3")) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_com_device::device_reset()
{
}

static MACHINE_CONFIG_FRAGMENT( com_at_config )
	MCFG_NS16450_ADD( "uart_0", genpc_com_interface[0], XTAL_1_8432MHz ) /* Verified: IBM P/N 6320947 Serial/Parallel card uses an NS16450N */
	MCFG_NS16450_ADD( "uart_1", genpc_com_interface[1], XTAL_1_8432MHz )
//  MCFG_NS16450_ADD( "uart_2", genpc_com_interface[2], XTAL_1_8432MHz )
//  MCFG_NS16450_ADD( "uart_3", genpc_com_interface[3], XTAL_1_8432MHz )
	MCFG_RS232_PORT_ADD( "serport0", isa_com, "microsoft_mouse" )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", isa_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart_1", ins8250_uart_device, cts_w))

//  MCFG_RS232_PORT_ADD( "serport2", isa_com, NULL )
//  MCFG_RS232_PORT_ADD( "serport3", isa_com, NULL )
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_COM_AT = &device_creator<isa8_com_at_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_com_at_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( com_at_config );
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

isa8_com_at_device::isa8_com_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_com_device(mconfig, ISA8_COM_AT, "Communications Adapter", tag, owner, clock, "isa_com_at", __FILE__)
{
}
