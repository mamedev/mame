// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 Miniware M2009 Auto Dial/Answer modem Cartridge

**********************************************************************/

#include "emu.h"
#include "m2009.h"

#define LOG_IO     (1U << 1)
#define LOG_DIAL   (1U << 2)
#define LOG_RXDTXD (1U << 3)

#define VERBOSE ( LOG_GENERAL )
#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO, __VA_ARGS__)
#define LOGDIAL(...)  LOGMASKED(LOG_DIAL, __VA_ARGS__)
#define LOGTXDRXD(...)  LOGMASKED(LOG_RXDTXD, __VA_ARGS__)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_M2009, p2000_m2009_modem_device, "p2000_m2009modem", "P2000 Miniware M2009 Auto Dial/Answer modem")

// Viewdata settings as default
static DEVICE_INPUT_DEFAULTS_START( modem  )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

#define M2009_PHONE_HOOK   5
#define M2009_DIALING      6
#define M2009_DIAL_PULSE   7

//-------------------------------------------------
//  p2000_m2009_modem_device - constructor
//-------------------------------------------------
p2000_m2009_modem_device::p2000_m2009_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_M2009, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
    , m_scc(*this, "scc")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_m2009_modem_device::device_add_mconfig(machine_config &config)
{
    SCC8530N(config, m_scc, 2.4576_MHz_XTAL); 
    // TXD over channel B (75 baud)  - port 0x40 - 0x41
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set("modem", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set("modem", FUNC(rs232_port_device::write_rts));
    
    // RXD over channel A (1200 baud) - port 0x42 - 0x43
	rs232_port_device &rs232(RS232_PORT(config, "modem", default_rs232_devices, "null_modem"));
    rs232.rxd_handler().set(m_scc, FUNC(scc8530_device::rxa_w));
	rs232.cts_handler().set(m_scc, FUNC(scc8530_device::ctsa_w));
    
    rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(modem));
    rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(modem));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_m2009_modem_device::device_start()
{
    //m_slot->io_space().install_readwrite_handler(0x40, 0x43, read8sm_delegate(*m_scc, FUNC(z80scc_device::ab_dc_r)), write8sm_delegate(*m_scc, FUNC(z80scc_device::ab_dc_w)));

    m_slot->io_space().install_readwrite_handler(0x40, 0x40, read8smo_delegate(*this, FUNC(port_40_r)), write8smo_delegate(*this, FUNC(port_40_w)));
    m_slot->io_space().install_readwrite_handler(0x41, 0x41, read8smo_delegate(*this, FUNC(port_41_r)), write8smo_delegate(*this, FUNC(port_41_w)));
    m_slot->io_space().install_readwrite_handler(0x42, 0x42, read8smo_delegate(*this, FUNC(port_42_r)), write8smo_delegate(*this, FUNC(port_42_w)));
    m_slot->io_space().install_readwrite_handler(0x43, 0x43, read8smo_delegate(*this, FUNC(port_43_r)), write8smo_delegate(*this, FUNC(port_43_w)));
    
    m_slot->io_space().install_read_handler(0x44, 0x47, read8smo_delegate(*this, FUNC(port_44_r)));
    m_slot->io_space().install_write_handler(0x44, 0x47, write8smo_delegate(*this, FUNC(port_44_w)));
 
    m_dial_pulse_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m2009_dial_pulse_timer_cb), this));
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------
void p2000_m2009_modem_device::device_reset()
{
    m_port_44 = 0;
    m_scc->syncb_w(0);
    m2009_phone_on_hook();
}

void p2000_m2009_modem_device::port_40_w(uint8_t data)
{
    m_cha_reg = data;
    LOGTXDRXD("Port write 40: %02x\n", data);
    m_scc->ab_dc_w(0, data);
}

uint8_t p2000_m2009_modem_device::port_40_r()
{
    uint8_t data = m_scc->ab_dc_r(0);
    //  Emulated sync signal when dialing P2000 implementation expects a bit 4 (SYNCH/Hunt) toggle when the hook is taken off
    if ((m_port_44 & 0x20) && (m_cha_reg == 0x10))
    {
        if (m_sync_toggle)
        {
            m_sync_toggle = false;
            m_scc->syncb_w(1);
        }
        else
        {
            m_sync_toggle = true;
            m_scc->syncb_w(0);
        }
    }
    
    LOGTXDRXD("Port read 40: %02x\n", data);
    return data;
}

void p2000_m2009_modem_device::port_41_w(uint8_t data)
{
    LOGTXDRXD("Port write 41: %02x\n", data);
    m_scc->ab_dc_w(1, data);
}

uint8_t p2000_m2009_modem_device::port_41_r()
{
    LOGTXDRXD("Port read 41\n");
    return m_scc->ab_dc_r(1);
}

void p2000_m2009_modem_device::port_42_w(uint8_t data)
{
    LOGTXDRXD("Port write 42: %02x\n", data);
    m_chb_reg = data;
    m_scc->ab_dc_w(2, data);
}

uint8_t p2000_m2009_modem_device::port_42_r()
{
    uint8_t data = m_scc->ab_dc_r(2);
    LOGTXDRXD("Port read 42 %02x\n", data);
    return data;
}

void p2000_m2009_modem_device::port_43_w(uint8_t data)
{
    LOGTXDRXD("Port write 43: %02x\n", data);
    m_scc->ab_dc_w(3, data);
}

uint8_t p2000_m2009_modem_device::port_43_r()
{
    uint8_t data = m_scc->ab_dc_r(3);
    LOGTXDRXD("Port write 43: %02x\n", data);
    return data;
}

void p2000_m2009_modem_device::port_44_w(uint8_t data)
{
    LOGTXDRXD("Port read 43 %02x\n", data);
    LOGIO("Hook is %s \n", BIT(data, M2009_DIAL_PULSE) ? "off" : "on");
    LOGIO("Dialing %s \n", BIT(data, M2009_DIALING) ? "yes" : "no");
    LOGIO("Dial pulse is %s \n", BIT(data, M2009_DIAL_PULSE) ? "short" : "open");

    if (BIT(data, M2009_PHONE_HOOK) || BIT(data, M2009_DIALING))
    {
        // First time off hook?
        if (!BIT(m_port_44, M2009_PHONE_HOOK) && !BIT(m_port_44, M2009_DIALING))
        {
            LOG("-- Off hook --\n");
            // TODO: start dial tone sound
            m_dial_in_progress = false;
            m_number_cnt = 0;
            m_phone_number[m_number_cnt + 1] = 0;
        }

        // Dialing in progress?
        if (BIT(data, M2009_DIALING))
        {
            // Start Dialing ?
            if (!BIT(m_port_44, M2009_DIALING))
            {
                // Start of dial process first pulse
                if (!m_dial_in_progress)
                {
                    LOGDIAL("Start dialing new number\n");
                    m_dial_in_progress = true;
                   // TODO: stop dial tone sound
                }
                else
                {
                    LOGDIAL("Start dialing next digit\n");
                    m_number_cnt++;
                }
                m_phone_number[m_number_cnt % m_number_size] = '0';
                m_phone_number[(m_number_cnt % m_number_size) + 1] = 0;
            }
            
            // Rising edge of dial pulse
            if (BIT(data, M2009_DIAL_PULSE) && !BIT(m_port_44, M2009_DIAL_PULSE))
            {
                m_phone_number[m_number_cnt % m_number_size]++;
                if (m_phone_number[m_number_cnt % m_number_size] > '9')
                {
                    // Ten pusles makes a zero (prevent overflow > 10 ==> '#')
                    m_phone_number[m_number_cnt % m_number_size] =
                        (m_phone_number[m_number_cnt % m_number_size] == '0' ? '#' : '0');
                }
            }
        }
        else
        {
            if (m_dial_in_progress )
            {
                LOGDIAL("Number dialed so far %s\n", m_phone_number);
            }
            
        }
        m_dial_pulse_timer->adjust(attotime::from_msec(m_dialing_ready_delay));
    }
    else
    {
        // Hook was off now on phone --> so reset all dial/connections activities
        if (BIT(m_port_44, M2009_PHONE_HOOK))
        {
            m2009_phone_on_hook();
        }
    }
    
    m_port_44 = data;
}

uint8_t p2000_m2009_modem_device::port_44_r()
{
    LOGTXDRXD("Port read 44 %02x\n", m_port_44);
    return m_port_44;
}

TIMER_CALLBACK_MEMBER(p2000_m2009_modem_device::m2009_dial_pulse_timer_cb)
{
    LOG("Number dialed: %s\n", m_phone_number);
    // TODO: Stop modem connection sound
    m_scc->dcda_w(0);
    m_scc->dcdb_w(0);
}

void p2000_m2009_modem_device::m2009_phone_on_hook()
{
    // reset all dial/connections activities
    LOG("-- On hook --\n");
            
    // Stop dial process
    m_dial_pulse_timer->reset();
    m_number_cnt = 0;
    m_dial_in_progress = false;
          
    m_scc->dcda_w(1);  // Data carier detect
    m_scc->dcdb_w(1);  // Data carier detect
    // TODO: stop dial/connection tone sound
}


