// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***********************************************************************************************************************************

Skeleton driver for R&D Co., Ltd. "GALPRO" programmer.
PCB label: "R&D co.,ltd RD-0053 GALPRO"

Type '?' at the prompt for a list of commands. Type 'super' to access additional test and diagnostic commands.
(Note: the 'chk' command in super mode outputs Japanese text and expects a ShiftJIS-compatible terminal)

************************************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/tmpz84c011.h"
#include "machine/i8251.h"

namespace {

class galpro_state : public driver_device
{
public:
	galpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_rs232(*this, "rs232")
	{ }

	void galpro(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void zc0_w(int state);
	template<int Num> void port_w(u8 data);
	template<int Num> void pin_state_w(u8 data);
	void vie_w(u8 data);

	required_device<tmpz84c011_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<rs232_port_device> m_rs232;

	u8 m_clock_in, m_clock_out;
	u8 m_pin_state[3];
};

/**************************************************************************/
void galpro_state::mem_map(address_map &map)
{
	// power-on test copies the ROM area over itself to populate RAM
	map(0x0000, 0x7fff).mirror(0x8000).ram();
	map(0x0000, 0x7fff).rom();
}

/**************************************************************************/
void galpro_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x80, 0x81).mirror(0x3e).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xc0, 0xff).w(FUNC(galpro_state::vie_w));
}


/**************************************************************************/
static const z80_daisy_config daisy_chain[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};

/**************************************************************************/
void galpro_state::galpro(machine_config &config)
{
	TMPZ84C011(config, m_maincpu, 9.8304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &galpro_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &galpro_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->zc0_callback().set(FUNC(galpro_state::zc0_w));
	m_maincpu->out_pa_callback().set(FUNC(galpro_state::pin_state_w<0>));
	m_maincpu->out_pb_callback().set(FUNC(galpro_state::pin_state_w<1>));
	m_maincpu->out_pc_callback().set(FUNC(galpro_state::pin_state_w<2>));
	m_maincpu->out_pd_callback().set(FUNC(galpro_state::port_w<3>));
	m_maincpu->out_pe_callback().set(FUNC(galpro_state::port_w<4>));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->cts_handler().set(m_uart, FUNC(i8251_device::write_cts));
	m_rs232->rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));

	I8251(config, m_uart);
	m_uart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_uart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_uart->rxrdy_handler().set(m_maincpu, FUNC(tmpz84c011_device::trg2));
}

/**************************************************************************/
void galpro_state::machine_start()
{
	m_clock_in = m_clock_out = 0;
	m_pin_state[0] = m_pin_state[1] = m_pin_state[2] = 0;

	save_item(NAME(m_clock_in));
	save_item(NAME(m_clock_out));
	save_item(NAME(m_pin_state));
}

/**************************************************************************/
void galpro_state::zc0_w(int state)
{
	if (state && !m_clock_in)
	{
		m_clock_out ^= 1;
		m_uart->write_txc(m_clock_out);
		m_uart->write_rxc(m_clock_out);
	}

	m_clock_in = state;
}

/**************************************************************************/
template<int Num>
void galpro_state::port_w(u8 data)
{
	logerror("%s: port %c write %02X\n", machine().describe_context(), 'A' + Num, data);
}

/**************************************************************************/
template<int Num>
void galpro_state::pin_state_w(u8 data)
{
	for (int i = 0; i < 8; i++)
		if (BIT(data ^ m_pin_state[Num], i))
			logerror("%s: GAL pin %u = %u\n", machine().describe_context(), 8*Num + i + 1, BIT(data, i));

	m_pin_state[Num] = data;
}

/**************************************************************************/
void galpro_state::vie_w(u8 data)
{
	// DAC controls programming voltage in 0.1V steps
	logerror("%s: Vie = %.1fV\n", machine().describe_context(), data / 10.0);
}

static INPUT_PORTS_START( galpro )
INPUT_PORTS_END

ROM_START( galpro )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "galpro v1.03.u1", 0x0000, 0x8000, CRC(d366543f) SHA1(d1585d53c842285950494ce1ddd3064fb2c31c15) )

	ROM_REGION(0x104, "pld", ROMREGION_ERASE00)
	ROM_LOAD( "epl16p8bd.u2", 0x000, 0x104, NO_DUMP )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY          FULLNAME  FLAGS
SYST( 1989, galpro, 0,      0,      galpro,  galpro, galpro_state, empty_init, "R&D Co., Ltd.", "GALPRO", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
