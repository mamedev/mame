// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/****************************************************************************

    Datacast Controller by Defence Products

    The PCB is named Datacast Controller, and the outer casing is customised
    for 'The Stock Exchange'. Maybe other ROM variants exist for this
    controller.

    Hardware:
    ---------
    CPU:    80186

    RAM:    HM62256 x 8
            PCF8582

    Video:  SAA5240 + 2K RAM - Teletext Decoder
            SAA5250 + 2K RAM - Teletext Acquisition and Control
            SAA5231          - Teletext Video Processor

    Serial: D8251 x 2 - USART
            AY-5-8116 - Baud Rate Generator
            MC145406  - Driver/Receiver

    Ports:  Printer
            Modem
            Monitor
            Keypad

    I2C Slave Devices:
    ------------------
      22  SAA5240
      A2  PCF8582
      C0  ?

    TODO:
    - implement keypad
    - complete SAA5240 emulation
    - feed SAA5240 with teletext packets

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i2cmem.h"
#include "machine/i8251.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "machine/mm74c922.h"
#include "video/saa5240.h"
#include "bus/rs232/rs232.h"
#include "imagedev/bitbngr.h"
#include "screen.h"


namespace {

class datacast_state : public driver_device
{
public:
	datacast_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_kb(*this, "74c923")
		, m_cct(*this, "saa5240")
		, m_ttd(*this , "ttd")
		, m_i2cmem(*this, "i2cmem")
		, m_usart(*this, "usart%u", 0)
		, m_dbrg(*this, "dbrg")
	{
	}

	void datacast(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t keypad_r();
	void keypad_w(uint8_t data);
	uint8_t i2c_r();
	void i2c_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;
	void saa5240_map(address_map &map) ATTR_COLD;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mm74c923_device> m_kb;
	required_device<saa5240a_device> m_cct;
	required_device<bitbanger_device> m_ttd;
	required_device<i2c_pcf8582_device> m_i2cmem;
	required_device_array<i8251_device, 2> m_usart;
	required_device<com8116_device> m_dbrg;

	uint8_t m_key_col = 0;
};


void datacast_state::machine_start()
{
}


void datacast_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0xc0000, 0xc0000).rw(FUNC(datacast_state::keypad_r), FUNC(datacast_state::keypad_w)).umask16(0x00ff);
	map(0xc0080, 0xc0080).lr8([]() { return 0xff; }, "dips"); // unknown purpose
	map(0xc0100, 0xc010f).noprw(); //.rw(m_saa5250, FUNC(saa5250_device::read), FUNC(saa5250_device::write)).umask16(0x00ff);
	map(0xc0180, 0xc0180).rw(FUNC(datacast_state::i2c_r), FUNC(datacast_state::i2c_w)).umask16(0x00ff);
	map(0xc0200, 0xc0203).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xc0280, 0xc0280).w(m_dbrg, FUNC(com8116_device::stt_str_w));
	map(0xc0300, 0xc0303).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf0000, 0xfffff).rom().region("rom", 0);
}

void datacast_state::saa5240_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).ram();
}


static INPUT_PORTS_START(datacast)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1")  PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2")  PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3")  PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4")  PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5")  PORT_CODE(KEYCODE_5)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Q")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("W")  PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E")  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R")  PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("T")  PORT_CODE(KEYCODE_T)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A")  PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("S")  PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D")  PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("G")  PORT_CODE(KEYCODE_G)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z")  PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("X")  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C")  PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("V")  PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B")  PORT_CODE(KEYCODE_B)
INPUT_PORTS_END


uint8_t datacast_state::keypad_r()
{
	uint8_t data = (m_kb->read() << 0) | (m_kb->da_r() << 5);
	logerror("keypad_r: %02x\n", data);
	return data;
}

void datacast_state::keypad_w(uint8_t data)
{
	m_key_col = data;

	logerror("keypad_w: %02x\n", data);
}


uint8_t datacast_state::i2c_r()
{
	return m_i2cmem->read_sda() &  m_cct->read_sda();
}

void datacast_state::i2c_w(uint8_t data)
{
	m_i2cmem->write_sda(BIT(data, 0));
	m_i2cmem->write_scl(BIT(data, 1));

	m_cct->write_sda(BIT(data, 0));
	m_cct->write_scl(BIT(data, 1));
}


void datacast_state::datacast(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &datacast_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(6_MHz_XTAL, 768, 0, 480, 312, 0, 250);
	m_screen->set_screen_update("saa5240", FUNC(saa5240a_device::screen_update));

	MM74C923(config, m_kb, 0);
	m_kb->x1_rd_callback().set_ioport("X1");
	m_kb->x2_rd_callback().set_ioport("X2");
	m_kb->x3_rd_callback().set_ioport("X3");
	m_kb->x4_rd_callback().set_ioport("X4");

	//SAA5231(config, m_saa5231, 13.875_MHz_XTAL);

	SAA5240A(config, m_cct, 6_MHz_XTAL);
	m_cct->set_addrmap(0, &datacast_state::saa5240_map);

	//SAA5250(config, m_saa5250, 0);
	//m_saa5250->set_addrmap(0, &datacast_state::saa5250_map); // 2K RAM

	I2C_PCF8582(config, m_i2cmem).set_e0(1);

	COM8116(config, m_dbrg, 5.0688_MHz_XTAL); // AY-5-8116
	m_dbrg->fr_handler().set(m_usart[0], FUNC(i8251_device::write_txc));
	m_dbrg->fr_handler().append(m_usart[0], FUNC(i8251_device::write_rxc));
	m_dbrg->ft_handler().set(m_usart[1], FUNC(i8251_device::write_txc));
	m_dbrg->ft_handler().append(m_usart[1], FUNC(i8251_device::write_rxc));

	input_merger_device &usartint(INPUT_MERGER_ANY_HIGH(config, "usartint"));
	usartint.output_handler().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	I8251(config, m_usart[0], 0);
	m_usart[0]->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_usart[0]->dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	m_usart[0]->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));
	m_usart[0]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, "null_modem"));
	modem.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	modem.dsr_handler().set(m_usart[0], FUNC(i8251_device::write_dsr));

	I8251(config, m_usart[1], 0);
	m_usart[1]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));
	m_usart[1]->dtr_handler().set("printer", FUNC(rs232_port_device::write_dtr));
	m_usart[1]->rts_handler().set("printer", FUNC(rs232_port_device::write_rts));
	m_usart[1]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<2>));
	m_usart[1]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<3>));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	printer.dsr_handler().set(m_usart[1], FUNC(i8251_device::write_dsr));

	// Teletext data is extracted from video signal by SAA5231.
	// Use bitbanger to read a T42 teletext stream.
	BITBANGER(config, m_ttd, 0);
}


ROM_START(datacast)
	ROM_REGION16_LE(0x10000, "rom", 0)
	ROM_LOAD16_BYTE("v29.ic9",  0x0000, 0x8000, CRC(650e6d34) SHA1(0c8709ce8220bda5da8fd41b81e531fc6f7b8ee4))
	ROM_LOAD16_BYTE("v29.ic10", 0x0001, 0x8000, CRC(bd39d683) SHA1(9d6cc2d3d6a26ca35e4e38beac64d31e345d6808))
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT     CLASS           INIT        COMPANY             FULLNAME                                    FLAGS
SYST(1987, datacast, 0,      0,      datacast,   datacast, datacast_state, empty_init, "Defence Products", "Datacast Controller (The Stock Exchange)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
