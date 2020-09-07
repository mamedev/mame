// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
/**********************************************************************

    Philips P2000T Mini Digital Cassette Recorder Port emulation

**********************************************************************/

#include "emu.h"
#include "mdcr.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(MDCR_PORT, mdcr_port_device, "mdcr_port", "Philips Mini DCR Port")

//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mdcr_port_interface - constructor
//-------------------------------------------------

device_mdcr_port_interface::device_mdcr_port_interface(const machine_config& mconfig,
                                                       device_t&             device)
: device_interface(device, "mdcr")
{
    m_slot = dynamic_cast<mdcr_port_device*>(device.owner());
}


//-------------------------------------------------
//  ~device_mdcr_port_interface - destructor
//-------------------------------------------------
device_mdcr_port_interface::~device_mdcr_port_interface() {}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mdcr_port_device - constructor
//-------------------------------------------------

mdcr_port_device::mdcr_port_device(const machine_config& mconfig,
                                   const char*           tag,
                                   device_t*             owner,
                                   uint32_t              clock)
: device_t(mconfig, MDCR_PORT, tag, owner, clock)
, device_single_card_slot_interface<device_mdcr_port_interface>(mconfig, *this)
, m_read_handler(*this)
, m_cart(nullptr)
{
}


//-------------------------------------------------
//  mdcr_port_device - destructor
//-------------------------------------------------
mdcr_port_device::~mdcr_port_device() {}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mdcr_port_device::device_start()
{
    m_cart = get_card_device();

    // resolve callbacks
    m_read_handler.resolve_safe();
}


READ_LINE_MEMBER(mdcr_port_device::rdc) { return m_cart != nullptr && m_cart->data_available(); }
READ_LINE_MEMBER(mdcr_port_device::rda) { return m_cart != nullptr && m_cart->data(); }
READ_LINE_MEMBER(mdcr_port_device::bet) { return m_cart != nullptr && m_cart->tape_start_or_end(); }
READ_LINE_MEMBER(mdcr_port_device::cip) { return m_cart != nullptr; }
READ_LINE_MEMBER(mdcr_port_device::wen) { return m_cart != nullptr; }

WRITE_LINE_MEMBER(mdcr_port_device::rev)
{
    m_rev = state;
    if (m_cart != nullptr && m_rev)
    {
        m_cart->rewind();
    }

    if (!m_rev && !m_fwd && m_cart)
    {
        m_cart->stop();
    }
}

WRITE_LINE_MEMBER(mdcr_port_device::fwd)
{
    m_fwd = state;
    if (m_cart != nullptr && m_fwd)
    {
        m_cart->forward();
    }

    if (!m_rev && !m_fwd && m_cart)
    {
        m_cart->stop();
    }
}

WRITE_LINE_MEMBER(mdcr_port_device::wda) { m_wda = state; }

WRITE_LINE_MEMBER(mdcr_port_device::wdc)
{
    if (m_cart != nullptr && state)
    {
        m_cart->write_bit(m_wda);
    };
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MDCR, mdcr_device, "mdcr", "Philips Mini DCR")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mdcr_device::device_add_mconfig(machine_config& config)
{
    CASSETTE(config, m_cassette);
    m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED
                                  | CASSETTE_SPEAKER_MUTED);
    m_cassette->set_interface("p2000_cass");

    // Tape speed of mdcr is between 12-20 ips (300-500 mm/s)
    m_cassette->set_speed((double) 18.75 / 1.875);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  mdcr_device - constructor
//-------------------------------------------------
mdcr_device::mdcr_device(const machine_config& mconfig,
                         device_type           type,
                         const char*           tag,
                         device_t*             owner,
                         uint32_t              clock)
: device_t(mconfig, type, tag, owner, clock)
, device_mdcr_port_interface(mconfig, *this)
, m_cassette(*this, "cassette")
, m_read_timer(nullptr)
{

}


void mdcr_device::device_start()
{
    m_read_timer = timer_alloc();
    m_read_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mdcr_device::device_timer(emu_timer& timer, device_timer_id id, int param, void* ptr)
{
    if (!m_recording && m_cassette->motor_on())
    {
        auto delay = abs(m_cassette->get_position() - m_last_tape_time);

        // Decode the signal using the fake phase decode circuit
        bool newBit = phd.signal((m_cassette->input() > +0.04), delay);
        if (newBit)
        {
            // Flip rdc
            m_rdc = !m_rdc;
            m_rda = phd.pullBit();
        }
    }
    m_last_tape_time = m_cassette->get_position();
}

mdcr_device::mdcr_device(const machine_config& mconfig,
                         const char*           tag,
                         device_t*             owner,
                         uint32_t              clock)
: mdcr_device(mconfig, MDCR, tag, owner, clock)
{
}

void mdcr_device::write_bit(bool bit)
{
    m_recording = true;
    m_cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
    m_cassette->output(bit ? +1.0 : -1.0);
    phd.reset();
}

void mdcr_device::rewind()
{
    m_fwd       = false;
    m_recording = false;
    m_cassette->set_motor(true);
    m_cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
    m_cassette->go_reverse();
}

void mdcr_device::forward()
{
    // A pulse of 1us < T < 20 usec should reset the phase decoder.
    constexpr double kResetPulse = 2.00e-05;
    auto             now         = machine().time().as_double();
    auto             pulse_delay = now - m_fwd_pulse_time;
    m_fwd_pulse_time             = now;

    if (pulse_delay < kResetPulse)
    {
        phd.reset();
    }

    m_fwd = true;
    m_cassette->set_motor(true);
    m_cassette->change_state(m_recording ? CASSETTE_RECORD : CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
    m_cassette->go_forward();
}

void mdcr_device::stop()
{
    m_cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
    m_cassette->set_motor(false);
}

bool mdcr_device::tape_start_or_end()
{
    auto pos = m_cassette->get_position();
    return m_cassette->motor_on() && (pos <= 0 || pos >= m_cassette->get_length());
}

bool mdcr_device::data_available()
{
    if (m_recording)
        return false;

    return m_fwd ? m_rdc : m_rda;
}

bool mdcr_device::data()
{
    return m_fwd ? m_rda : m_rdc;
}

//-------------------------------------------------
//  SLOT_INTERFACE( p2000_mdcr_devices )
//-------------------------------------------------

void p2000_mdcr_devices(device_slot_interface& device) { device.option_add("mdcr", MDCR); }
