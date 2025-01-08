// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Raphaël Assenat's DIY SMS/Mark III Paddle Controller

    English description:
    https://www.raphnet.net/electronique/sms_paddle/index_en.php

    French description:
    https://www.raphnet.net/electronique/sms_paddle/index.php

    Firmware source:
    https://github.com/raphnet/sms_paddle

    TODO:
    * Add ATmega8 variant - need CPU core.

**********************************************************************/

#include "emu.h"
#include "diypaddle.h"

#include "cpu/avr8/avr8.h"


namespace {

ROM_START( sms_diy_paddle )
	ROM_REGION(0x4000, "u1", ROMREGION_ERASEFF)
	ROM_LOAD( "sms_paddle.bin", 0x0000, 0x0138, CRC(97c7d334) SHA1(dd9e20f2447c43dbe29a49bb3a4fd8049775acd3) )

	ROM_REGION(0x200, "eeprom", ROMREGION_ERASEFF)
ROM_END


INPUT_PORTS_START( sms_diy_paddle )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("PADDLE")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE) PORT_MINMAX(0, 1023) PORT_REVERSE PORT_SENSITIVITY(160) PORT_KEYDELTA(20) PORT_CENTERDELTA(0)

	PORT_START("MODE")
	PORT_CONFNAME( 0x01, 0x00, "Mode" )
	PORT_CONFSETTING(    0x00, "HPD-200" )
	PORT_CONFSETTING(    0x01, "Export paddle" )
INPUT_PORTS_END



class sms_diy_paddle_device : public device_t, public device_sms_control_interface
{
public:
	sms_diy_paddle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	virtual tiny_rom_entry const *device_rom_region() const override { return ROM_NAME(sms_diy_paddle); }
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_diy_paddle); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 portb_in();
	u8 portc_in();
	u8 portd_in();
	void portb_out(u8 data);
	void portc_out(u8 data);

	TIMER_CALLBACK_MEMBER(set_lines);
	TIMER_CALLBACK_MEMBER(set_portd);

	required_device<atmega168_device> m_mcu;
	required_ioport m_button;
	required_ioport m_paddle;
	required_ioport m_mode;

	u8 m_lines;
	u8 m_portb;
	u8 m_portc;
	u8 m_portd;
};


sms_diy_paddle_device::sms_diy_paddle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_DIY_PADDLE, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_mcu(*this, "u1"),
	m_button(*this, "BUTTON"),
	m_paddle(*this, "PADDLE"),
	m_mode(*this, "MODE"),
	m_lines(0x2f),
	m_portb(0xff),
	m_portc(0xff),
	m_portd(0xff)
{
}


u8 sms_diy_paddle_device::in_r()
{
	if (!machine().side_effects_disabled())
		machine().scheduler().add_quantum(attotime::from_usec(5), attotime::from_usec(100));

	// button connected to TL directly
	return m_lines | (BIT(m_button->read(), 0) << 4);
}


void sms_diy_paddle_device::out_w(u8 data, u8 mem_mask)
{
	// TH connected to PD2
	machine().scheduler().synchronize(
			timer_expired_delegate(FUNC(sms_diy_paddle_device::set_portd), this),
			0xfb | (BIT(data, 6) << 2));
	machine().scheduler().add_quantum(attotime::from_usec(1), attotime::from_usec(20));
}


void sms_diy_paddle_device::device_add_mconfig(machine_config &config)
{
	ATMEGA168(config, m_mcu, 8'000'000); // internally-generated clock
	m_mcu->set_addrmap(AS_PROGRAM, &sms_diy_paddle_device::program_map);
	m_mcu->set_addrmap(AS_DATA, &sms_diy_paddle_device::data_map);
	m_mcu->set_eeprom_tag("eeprom");
	m_mcu->set_low_fuses(0xe2);
	m_mcu->set_high_fuses(0xdd);
	m_mcu->set_extended_fuses(0xf9);
	m_mcu->gpio_in<atmega168_device::GPIOB>().set(FUNC(sms_diy_paddle_device::portb_in));
	m_mcu->gpio_in<atmega168_device::GPIOC>().set(FUNC(sms_diy_paddle_device::portc_in));
	m_mcu->gpio_in<atmega168_device::GPIOD>().set(FUNC(sms_diy_paddle_device::portd_in));
	m_mcu->gpio_out<atmega168_device::GPIOB>().set(FUNC(sms_diy_paddle_device::portb_out));
	m_mcu->gpio_out<atmega168_device::GPIOC>().set(FUNC(sms_diy_paddle_device::portc_out));
	m_mcu->adc_in<atmega168_device::ADC0>().set_ioport("PADDLE");
}


void sms_diy_paddle_device::device_start()
{
	m_lines = 0x2f;
	m_portb = 0xff;
	m_portc = 0xff;
	m_portd = 0xff;

	save_item(NAME(m_lines));
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
	save_item(NAME(m_portd));
}


void sms_diy_paddle_device::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
}


void sms_diy_paddle_device::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}


u8 sms_diy_paddle_device::portb_in()
{
	// PB0 looped back to PB1 via mode switch
	if (BIT(m_mode->read(), 0))
		return 0xfc | bitswap<2>(m_portb, 0, 0);
	else
		return 0xfe | BIT(m_portb, 0);
}


u8 sms_diy_paddle_device::portc_in()
{
	return (m_portc & 0x3e) | 0xc0 | ((m_paddle->read() > 0x1ff) ? 0x01 : 0x00);
}


u8 sms_diy_paddle_device::portd_in()
{
	return m_portd;
}


void sms_diy_paddle_device::portb_out(u8 data)
{
	m_portb = data;
}


void sms_diy_paddle_device::portc_out(u8 data)
{
	// PC1 -> TR, PC2 -> Up, PC3 -> Down, PC4 -> Left, PC5 -> Right
	if (data != m_portc)
	{
		m_portc = data;
		machine().scheduler().synchronize(
				timer_expired_delegate(FUNC(sms_diy_paddle_device::set_lines), this),
				(BIT(data, 1) << 5) | BIT(data, 2, 4));
		machine().scheduler().add_quantum(attotime::from_usec(1), attotime::from_usec(20));
	}
}


TIMER_CALLBACK_MEMBER(sms_diy_paddle_device::set_lines)
{
	m_lines = param & 0x2f;
}


TIMER_CALLBACK_MEMBER(sms_diy_paddle_device::set_portd)
{
	m_portd = param & 0xff;
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_DIY_PADDLE, device_sms_control_interface, sms_diy_paddle_device, "sms_diypaddle", "raphnet DIY SMS/Mark III Paddle Controller")
