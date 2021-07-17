// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 MSX Mouse Cartridge (Stichting Computer Creatief)

**********************************************************************/

#include "emu.h"
#include "mouse.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define MOUSE_RESET -1
#define MOUSE_X_HIGH_NIB 0
#define MOUSE_X_LOW_NIB  1
#define MOUSE_Y_HIGH_NIB 2
#define MOUSE_Y_LOW_NIB  3
#define MOUSE_BUTTON_L   4
#define MOUSE_BUTTON_R   5

#define BUTTON_LEFT  0x00000010
#define BUTTON_RIGHT 0x00000001

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_MOUSE, p2000_mouse_device, "p2kmouse", "P2000 MSX Mouse Interface")

INPUT_PORTS_START( p2000_mouse )
	PORT_START("mouse_b")  // mouse buttons
	PORT_BIT( BUTTON_LEFT, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( BUTTON_RIGHT, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("mouse_x")  // X-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(15) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mouse_y")  // Y-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(15) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_mouse_device::device_add_mconfig(machine_config &config)
{
    /* PIO device */
    Z80PIO(config, m_mousepio, 2500000);
    m_mousepio->in_pa_callback().set(FUNC(p2000_mouse_device::pa_r_cb));
    m_mousepio->out_pa_callback().set(FUNC(p2000_mouse_device::pa_w_cb));
    m_mousepio->in_pb_callback().set(FUNC(p2000_mouse_device::pb_r_cb));
    m_mousepio->out_pb_callback().set(FUNC(p2000_mouse_device::pb_w_cb));

    TIMER(config, "timer").configure_periodic(FUNC(p2000_mouse_device::mouse_timer_cb), attotime::from_hz(6));
}

ioport_constructor p2000_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( p2000_mouse );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p2000_mouse_device - constructor
//-------------------------------------------------
p2000_mouse_device::p2000_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_MOUSE, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_mousepio(*this, "pio")
        , m_io_mouse_b(*this, "mouse_b")
        , m_io_mouse_x(*this, "mouse_x")
        , m_io_mouse_y(*this, "mouse_y")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_mouse_device::device_start()
{
    LOG("Mouse device starting\n");
    m_slot->io_space().install_readwrite_handler(0x44, 0x47, read8sm_delegate(*m_mousepio, FUNC(z80pio_device::read)), write8sm_delegate(*m_mousepio, FUNC(z80pio_device::write)));
    
    m_mouse_last_x = m_mouse_last_y = 0;
    m_mouse_b = m_mouse_x = m_mouse_y = 0;
    m_mouse_status = MOUSE_RESET;

	save_item(NAME(m_mouse_b));
	save_item(NAME(m_mouse_last_x));
	save_item(NAME(m_mouse_last_y));
    save_item(NAME(m_mouse_status));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//  -------------------- Mouse CPU output/input ports ---------------------- 
//  
//  Mouse channel
//    0x44  channel A data address  
//    0x46  channel A control address
//  
//  Free:
//    0x45  channel B data address 
//    0x47  channel B control address 
//

//-------------------------------------------------
//  Update mouse position timer
//-------------------------------------------------
TIMER_DEVICE_CALLBACK_MEMBER(p2000_mouse_device::mouse_timer_cb) 
{
	m_mouse_b = m_io_mouse_b->read();
    if (m_mouse_status != MOUSE_RESET) 
    {
        // Reset last position when P2000 retrieveed position - movement is relative to last read
        m_mouse_last_x = m_mouse_x;
        m_mouse_last_y = m_mouse_y;
    }
    m_mouse_x = m_io_mouse_x->read();
	m_mouse_y = m_io_mouse_y->read();
    m_mouse_status = MOUSE_RESET;
    
    LOG("Mouse X: %d Y: %d but: %04x \n", m_mouse_x, m_mouse_y, m_mouse_b);
}

//-------------------------------------------------
//  PIO channel A data read
//-------------------------------------------------
uint8_t p2000_mouse_device::pa_r_cb() 
{
    uint8_t data = 0;
    int x = m_mouse_last_x - m_mouse_x;
    int y = m_mouse_last_y - m_mouse_y;

    switch (++m_mouse_status)
    {
        case MOUSE_X_HIGH_NIB:
            data = ((x >>  4) & 0x0f);
            break;
        case MOUSE_X_LOW_NIB:
            data = (x & 0x0f);
            break;
            
        case MOUSE_Y_HIGH_NIB:
            data = ((y >> 4) & 0x0f);
            break;

        case MOUSE_Y_LOW_NIB:
            data = (y & 0x0f);
            break;

        case MOUSE_BUTTON_L:
            data = (m_mouse_b & BUTTON_LEFT ? 0 : 0xff);
            break;

        case MOUSE_BUTTON_R:
            data = (m_mouse_b & BUTTON_RIGHT ? 0 : 0xff);
            break;

        default:
            data = 0xff;
            break;
    }
    LOG("p2000_mouse_device::pa_r_cb %02x\n", data);
    return data;
}

//-------------------------------------------------
//  PIO channel A data write
//-------------------------------------------------
void p2000_mouse_device::pa_w_cb(uint8_t data) 
{
    LOG("p2000_mouse_device::pa_w_cb %02x\n", data);
    if (data == 0xff && m_mouse_status >= MOUSE_Y_LOW_NIB) 
    {
        LOG("Reset mouse status\n", data);
        // No button status asked
        m_mouse_status = MOUSE_RESET;
    }
}

//-------------------------------------------------
//  PIO channel B data read
//-------------------------------------------------
uint8_t p2000_mouse_device::pb_r_cb() 
{
    LOG("p2000_mouse_device::pb_r_cb %02x\n", m_channel_b_data);
    return m_channel_b_data;
}

//-------------------------------------------------
//  PIO channel B data write
//-------------------------------------------------
void p2000_mouse_device::pb_w_cb(uint8_t data) 
{
    LOG("p2000_mouse_device::pb_w_cb %02x\n", data);
    m_channel_b_data = data;
}
