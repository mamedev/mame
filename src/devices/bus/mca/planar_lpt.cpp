// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM PS/2 Planar LPT.

    Bidirectional SPP parallel port.

***************************************************************************/

#include "emu.h"
#include "planar_lpt.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_PLANAR_LPT, mca16_planar_lpt_device, "mca16_planar_lpt", "IBM PS/2 Planar LPT")

// static void mca_com(device_slot_interface &device)
// {

// }

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_planar_lpt_device::device_add_mconfig(machine_config &config)
{
	PC_LPT(config, m_lpt);
    m_lpt->irq_handler().set([this] (int state)
	{
        if (m_is_mapped)
        {
            if (m_cur_irq == 7) m_mca->ireq_w<7>(state);
        }
	});
}

//-------------------------------------------------
//  mca16_planar_lpt_device - constructor
//-------------------------------------------------

mca16_planar_lpt_device::mca16_planar_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_planar_lpt_device(mconfig, MCA16_PLANAR_LPT, tag, owner, clock)
{
}

mca16_planar_lpt_device::mca16_planar_lpt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xffff),
	m_lpt(*this, "lpt")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_planar_lpt_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_planar_lpt_device::device_reset()
{
	m_cur_io_start = m_cur_io_end = 0;
    m_cur_irq = 0;
    m_is_mapped = 0;
}

void mca16_planar_lpt_device::enable()
{
    // LOG("%s\n", FUNCNAME);

    planar_remap(AS_IO, m_cur_io_start, m_cur_io_end);
    m_is_mapped = 1;
}

void mca16_planar_lpt_device::disable()
{
    // LOG("%s\n", FUNCNAME);

    m_mca->unmap_device(m_cur_io_start, m_cur_io_end);
    m_is_mapped = 0;
}

void mca16_planar_lpt_device::planar_remap(int space_id, offs_t start, offs_t end)
{
    // LOG("%s\n", FUNCNAME);

    if(space_id == AS_IO)
    {
        // LOG("%s mapped to %04X-%04X\n", FUNCNAME, start, end);
        if(m_is_mapped) m_mca->unmap_device(m_cur_io_start, m_cur_io_end);
        m_mca->install_device(start, end, 
            read8sm_delegate(*this, FUNC(mca16_planar_lpt_device::io8_r)),
            write8sm_delegate(*this, FUNC(mca16_planar_lpt_device::io8_w)));
        m_cur_io_start = start;
        m_cur_io_end = end;
        m_is_mapped = true;
    }
    else
    {
        fatalerror("Tried to unmap I/O device from RAM space\n");
    }
}

void mca16_planar_lpt_device::planar_remap_irq(uint8_t new_irq)
{
    // LOG("%s\n", FUNCNAME);
    m_cur_irq = new_irq;
}

uint8_t mca16_planar_lpt_device::io8_r(offs_t offset)
{
	// printf("planar lpt interface: assert card feedback r %p\n", m_mca);
	// assert_card_feedback();
	// printf("ok\n");
    return m_lpt->read(offset);
}

void mca16_planar_lpt_device::io8_w(offs_t offset, uint8_t data)
{
	// printf("planar lpt interface: assert card feedback w %p\n", m_mca);
	// assert_card_feedback();
	// printf("ok\n");
    m_lpt->write(offset, data);
}