// license:BSD-3-Clause
// copyright-holders:R. Belmont, R Justice

/*
    cricket.cpp - Street Electronics The Cricket enulation
    based on mboardd.cpp

    Similiar to the Mockingboard D, this device was a little box that attached to the IIc's modem port.
    It included:
    - Music via 2 x AY 8913
    - Speech via TMS5220
    - Clock support. This is somewhat limited, The cricket will maintain the time once set, and is not powered off.

    The microcontroller used in the Cricket is a MC6801 with a custom mask programmed rom
    Some information available here:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Peripherals/Speech/Street%20Electronics%20The%20Cricket/


    Ports hooked up as follows:

    Port 1:
    x-------  x  nc
    -x------  x  nc
    --x-----  x  nc
    ---x----  x  nc
    ----x---  x  nc
    -----x--  x  nc
    ------x-  O  LED
    -------x  O  *RESET (To both AY's)

    Port 2:
    ---x----  O  UART TX
    ----x---  I  UART RX
    -----x--  I  pullup to +5v \
    ------x-  I  pullup to +5v  MC6801 Mode 7
    -------x  I  pullup to +5v /

    Port 3:
    x-------  O  DSR (to IIc)
    -x------  I  DTR (from IIc)
    --x-----  O  *CS2 (AY-3-8913 U8)
    ---x----  O  *CS1 (AY-3-8913 U7)
    ----x---  O  BC1 (Both AY-3-8913's)
    -----x--  O  BDIR (Both AY-3-8913's)
    ------x-  O  *WS (TMS5220)
    -------x  O  *RS (TMS5220)
	
	SC1 (IS3) I  120Hz generated from the power pack input. Relies on the filtering being poor enough to extract the mains frequency. 

    Port 4:
    x-------  I/O  D7
    -x------  I/O  D6
    --x-----  I/O  D5
    ---x----  I/O  D4
    ----x---  I/O  D3
    -----x--  I/O  D2
    ------x-  I/O  D1
    -------x  I/O  D0

*/

#include "emu.h"
#include "cricket.h"

#include "cpu/m6800/m6801.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"
#include "speaker.h"


namespace {

ROM_START(cricket)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("03-801-00.bin", 0x000000, 0x000800, CRC(0a61e208) SHA1(9977a21522e28ae485009d3c31440fb4f7b9054b))
ROM_END

class cricket_device : public device_t, public device_rs232_port_interface
{
public:
	cricket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override;
    virtual void input_dtr(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;


private:
	required_device<m6801_cpu_device> m_mcu;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;
	required_device<tms5220c_device> m_tms;

    u8 p2_r() { return m_rx_state<<3 | M6801_MODE_7; };
	void p1_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	u8 p4_r();
	void p4_w(u8 data);
    TIMER_DEVICE_CALLBACK_MEMBER(clock_interrupt);

	u8 m_rx_state;
    u8 m_dtr_state;
    u8 m_data_r;
    u8 m_data_w;
};

cricket_device::cricket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_CRICKET, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_ay1(*this, "ay1")
	, m_ay2(*this, "ay2")
    , m_tms(*this, "tms")
    , m_rx_state(0)
    , m_dtr_state(0)
    , m_data_r(0)
    , m_data_w(0)
{
}

void cricket_device::device_add_mconfig(machine_config &config)
{
	M6801(config, m_mcu, 4.9152_MHz_XTAL);  // Crystal verified from visual inspection
	m_mcu->out_p1_cb().set(FUNC(cricket_device::p1_w));
	m_mcu->in_p2_cb().set(FUNC(cricket_device::p2_r));
	m_mcu->in_p3_cb().set(FUNC(cricket_device::p3_r));
	m_mcu->out_p3_cb().set(FUNC(cricket_device::p3_w));
	m_mcu->in_p4_cb().set(FUNC(cricket_device::p4_r));
	m_mcu->out_p4_cb().set(FUNC(cricket_device::p4_w));
	m_mcu->out_ser_tx_cb().set(FUNC(cricket_device::output_rxd));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8913(config, m_ay1, 4.9152_MHz_XTAL / 4);
	m_ay1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	AY8913(config, m_ay2, 4.9152_MHz_XTAL / 4);
	m_ay2->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
    TMS5220C(config, m_tms, 640000);
	m_tms->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_tms->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

    TIMER(config, "clock_interrupt").configure_periodic(FUNC(cricket_device::clock_interrupt), attotime::from_hz(120));
}

const tiny_rom_entry *cricket_device::device_rom_region() const
{
	return ROM_NAME(cricket);
}

void cricket_device::device_start()
{
    save_item(NAME(m_rx_state));
	save_item(NAME(m_dtr_state));
    save_item(NAME(m_data_r));
    save_item(NAME(m_data_w));
}

void cricket_device::device_reset()
{
    output_dcd(0);
	output_cts(0);
}

void cricket_device::input_txd(int state)
{
	m_rx_state = state ? 1 : 0;
}

void cricket_device::input_dtr(int state)
{
	m_dtr_state = state ? 1 : 0;
}

void cricket_device::p1_w(u8 data)
{
    if (!BIT(data, 0))
	{
		m_ay1->reset_w();
		m_ay2->reset_w();
	}
    // TODO: Front panel LED driven from bit 1. Flashes if clock is not setup 
}

u8 cricket_device::p3_r()
{
    return m_dtr_state << 6;
}

void cricket_device::p3_w(u8 data)
{
	if (!BIT(data, 5)) //AY1 *CS
	{
        if (BIT(data, 2)) // BDIR
        {
            if (BIT(data, 3)) // BC1
                m_ay1->address_w(m_data_w);
            else
                m_ay1->data_w(m_data_w);
        }
        else if (!BIT(data, 3))
            m_data_r = m_ay1->data_r();
    }
    
    if (!BIT(data, 4)) // AY2 *CS
    {
        if (BIT(data, 2)) // BDIR
        {
            if (BIT(data, 3)) // BC1
                m_ay2->address_w(m_data_w);
            else
                m_ay2->data_w(m_data_w);
        }
        else if(!BIT(data, 3))
            m_data_r = m_ay2->data_r();
    }
    
    if (!BIT(data, 0)) //5220 read
        m_data_r = m_tms->status_r();
    
    if (!BIT(data, 1)) //5220 write
        m_tms->data_w(m_data_w);

    output_dsr(!BIT(data, 7));
}

u8 cricket_device::p4_r()
{
	return m_data_r;
}

void cricket_device::p4_w(u8 data)
{
	m_data_w = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(cricket_device::clock_interrupt)
{
	m_mcu->pulse_input_line(M6801_IRQ1_LINE, attotime::from_usec(1)); // use IRQ1 as IS3 not currently implemented for m6801
}


} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_CRICKET, device_rs232_port_interface, cricket_device, "secricket", "Street Electronics The Cricket!")
