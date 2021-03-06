// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 Centronics Interface Cartridges

**********************************************************************/

#include "emu.h"
#include "centronics.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_CENTRONICS, p2000_mw102_centronics_device, "p2000_centronics", "P2000 MW102 Centronics Interface")
DEFINE_DEVICE_TYPE(P2000_M2003,      p2000_m2003_centronics_device, "p2000_m2003", "P2000 Miniware M2003 Centronics Interface")
DEFINE_DEVICE_TYPE(P2000_P2GGCENT,   p2000_p2gg_centronics_device,  "p2000_p2ggcent", "P2000 P2000gg Centronics Interface")

//**************************************************************************
//  P2000 MW102 Centronics Interface Cartridge
//**************************************************************************
//-------------------------------------------------
//  p2000_mw102_centronics_device - constructor
//-------------------------------------------------
p2000_mw102_centronics_device::p2000_mw102_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_CENTRONICS, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
    , m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_mw102_centronics_device::device_add_mconfig(machine_config &config)
{
    CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(p2000_mw102_centronics_device::centronics_busy_w));
    m_centronics->perror_handler().set(FUNC(p2000_mw102_centronics_device::centronics_paper_empty_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_mw102_centronics_device::device_start()
{
    // Centronics handler
    m_slot->io_space().install_write_handler(0x4e, 0x4e, write8smo_delegate(*this, FUNC(p2000_mw102_centronics_device::port_4e_w)));
    m_slot->io_space().install_readwrite_handler(0x4f, 0x4f, read8smo_delegate(*this, FUNC(p2000_mw102_centronics_device::port_4f_r)), write8smo_delegate(*this, FUNC(p2000_mw102_centronics_device::port_4f_w)));
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------
void p2000_mw102_centronics_device::device_reset()
{
    /* busy = 1, paper empty = 1 */
    m_centronics_status = 0x03;
    m_centronics->write_strobe(1);
}

//-------------------------------------------------
//  Centronics - data register
//-------------------------------------------------
void p2000_mw102_centronics_device::port_4e_w(uint8_t data)
{
    m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

/* -------------------------------------------------
     Centronics - set status register
       BIT
        0    strobe
        1    init
------------------------------------------------- */
void p2000_mw102_centronics_device::port_4f_w(uint8_t data)
{
    m_centronics->write_strobe(BIT(data, 0));
    m_centronics->write_init(BIT(data, 1));
}

/* -------------------------------------------------
     Centronics - get status register
       BIT
        0    busy
        1    paper empty
------------------------------------------------- */
WRITE_LINE_MEMBER(p2000_mw102_centronics_device::centronics_busy_w)
 {
    if (state)
        m_centronics_status |= 0x1;
     else
        m_centronics_status &= ~0x1;
 }

WRITE_LINE_MEMBER(p2000_mw102_centronics_device::centronics_paper_empty_w)
 {
    if (state)
        m_centronics_status |= 0x2;
     else
        m_centronics_status &= ~0x2;
}

uint8_t p2000_mw102_centronics_device::port_4f_r()
{
    return m_centronics_status;
}

//**************************************************************************
//  P2000 P2000gg Centronics Interface Cartridge
//**************************************************************************
//-------------------------------------------------
//  p2000_p2gg_centronics_device - constructor
//-------------------------------------------------
p2000_p2gg_centronics_device::p2000_p2gg_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_CENTRONICS, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
    , m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_p2gg_centronics_device::device_add_mconfig(machine_config &config)
{
    CENTRONICS(config, m_centronics, centronics_devices, "printer");
    m_centronics->ack_handler().set(FUNC(p2000_p2gg_centronics_device::centronics_ack_w));
	m_centronics->busy_handler().set(FUNC(p2000_p2gg_centronics_device::centronics_busy_w));
    m_centronics->perror_handler().set(FUNC(p2000_p2gg_centronics_device::centronics_paper_empty_w));
    m_centronics->select_handler().set(FUNC(p2000_p2gg_centronics_device::centronics_select_w));
    m_centronics->fault_handler().set(FUNC(p2000_p2gg_centronics_device::centronics_error_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_p2gg_centronics_device::device_start()
{
    // Centronics handler
    m_slot->io_space().install_write_handler(0x46, 0x46, write8smo_delegate(*this, FUNC(p2000_p2gg_centronics_device::port_46_w)));
    m_slot->io_space().install_read_handler(0x47, 0x47, read8smo_delegate(*this, FUNC(p2000_p2gg_centronics_device::port_47_r)));
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------
void p2000_p2gg_centronics_device::device_reset()
{
    /* ack = 1, busy = 1, paper empty = 1, printer on = 1, error = 0 , present = 1 */
    m_centronics_status = 0x2f;
    m_centronics->write_strobe(1);
}

//-------------------------------------------------
//  Centronics - data register
//-------------------------------------------------
void p2000_p2gg_centronics_device::port_46_w(uint8_t data)
{
    m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));

    m_centronics->write_strobe(1);
    m_centronics->write_strobe(0);
}


/* -------------------------------------------------
     Centronics - get status register
       BIT
        0    ack
        1    busy
        2    paper empty
`       3    select
        4    error
        5    Cartridge present (allways 1)
------------------------------------------------- */
WRITE_LINE_MEMBER(p2000_p2gg_centronics_device::centronics_ack_w)
{
    if (state)
        m_centronics_status |= 0x1;
     else
        m_centronics_status &= ~0x1;
 }

WRITE_LINE_MEMBER(p2000_p2gg_centronics_device::centronics_busy_w)
{
    if (state)
        m_centronics_status |= 0x2;
     else
        m_centronics_status &= ~0x2;
}

WRITE_LINE_MEMBER(p2000_p2gg_centronics_device::centronics_paper_empty_w)
{
    if (state)
        m_centronics_status |= 0x4;
     else
        m_centronics_status &= ~0x4;
}

WRITE_LINE_MEMBER(p2000_p2gg_centronics_device::centronics_select_w)
{
    if (state)
        m_centronics_status |= 0x8;
     else
        m_centronics_status &= ~0x8;
}

WRITE_LINE_MEMBER(p2000_p2gg_centronics_device::centronics_error_w)
{
    if (state)
        m_centronics_status |= 0x10;
     else
        m_centronics_status &= ~0x10;
}

uint8_t p2000_p2gg_centronics_device::port_47_r()
{
    return m_centronics_status & 0x20;
}

//**************************************************************************
//  P2000 M2003 Centronics Interface Cartridge
//**************************************************************************
//-------------------------------------------------
//  p2000_p2gg_centronics_device - constructor
//-------------------------------------------------
p2000_m2003_centronics_device::p2000_m2003_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: p2000_p2gg_centronics_device(mconfig, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_m2003_centronics_device::device_start()
{
    // Centronics handler
    m_slot->io_space().install_write_handler(0x46, 0x46, write8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_46_w)));
    m_slot->io_space().install_read_handler(0x47, 0x47, read8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_47_r)));
    m_slot->io_space().install_readwrite_handler(0x48, 0x48, read8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_48_r)), write8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_48_w)));
    m_slot->io_space().install_readwrite_handler(0x49, 0x49, read8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_49_r)), write8smo_delegate(*this, FUNC(p2000_m2003_centronics_device::port_49_w)));
}

//-------------------------------------------------
//  Centronics - data register
//-------------------------------------------------
void p2000_m2003_centronics_device::port_46_w(uint8_t data)
{
    m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}


//-------------------------------------------------
//  Accessing port 48 will set strobe signal
//-------------------------------------------------
void p2000_m2003_centronics_device::port_48_w(uint8_t data)
{
    m_centronics->write_strobe(1);
}

uint8_t p2000_m2003_centronics_device::port_48_r()
{
    m_centronics->write_strobe(1);
    return 0;
}

//-------------------------------------------------
//  Accessing port 49 will reset strobe signal
//-------------------------------------------------
void p2000_m2003_centronics_device::port_49_w(uint8_t data)
{
    m_centronics->write_strobe(0);
}

uint8_t p2000_m2003_centronics_device::port_49_r()
{
    m_centronics->write_strobe(0);
    return 0;
}