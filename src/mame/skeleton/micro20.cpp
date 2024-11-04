// license:BSD-3-Clause
// copyright-holders: R. Belmont
/****************************************************************************

    micro20.cpp
    GMX Micro 20 single-board computer

    68020 + 68881 FPU

    800a5e = end of initial 68020 torture test
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "machine/mc68681.h"
#include "machine/msm58321.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/68230pit.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"


namespace {

#define MAINCPU_TAG "maincpu"
#define DUART_A_TAG "duarta"
#define DUART_B_TAG "duartb"
#define RTC_TAG     "rtc"
#define FDC_TAG     "fdc"
#define PIT_TAG     "pit"

class micro20_state : public driver_device
{
public:
	micro20_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_rom(*this, "bootrom"),
		m_mainram(*this, "mainram"),
		m_pit(*this, PIT_TAG),
		m_rtc(*this, RTC_TAG)
	{
	}

	void micro20(machine_config &config);

private:
	required_device<m68020_device> m_maincpu;
	required_memory_region m_rom;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<pit68230_device> m_pit;
	required_device<msm58321_device> m_rtc;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void m68k_reset_callback(int state);
	u32 buserror_r();

	TIMER_DEVICE_CALLBACK_MEMBER(micro20_timer);
	void h4_w(int state);
	void portb_w(u8 data);
	void portc_w(u8 data);

	void timerirq_w(int state)
	{
		m_maincpu->set_input_line(M68K_IRQ_4, state);
	}

	void micro20_map(address_map &map) ATTR_COLD;

	u8 m_tin;
	u8 m_h4;
};

void micro20_state::machine_start()
{
}

void micro20_state::machine_reset()
{
	u32 *pROM = (uint32_t *)m_rom->base();
	u32 *pRAM = (uint32_t *)m_mainram.target();

	pRAM[0] = pROM[2];
	pRAM[1] = pROM[3];
	m_maincpu->reset();

	m_tin = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(micro20_state::micro20_timer)
{
	m_pit->update_tin(m_tin ? ASSERT_LINE : CLEAR_LINE);
	if (!m_h4 && m_tin)
		m_maincpu->set_input_line(M68K_IRQ_6, HOLD_LINE);

	m_tin ^= 1;
}

void micro20_state::h4_w(int state)
{
	printf("h4_w: %d\n", state);
	m_h4 = state ^ 1;
}

void micro20_state::m68k_reset_callback(int state)
{
	// startup test explicitly checks if the m68k RESET opcode resets the 68230
	m_pit->reset();
}

void micro20_state::portb_w(u8 data)
{
	m_rtc->d0_w((data & 1) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d1_w((data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d2_w((data & 4) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d3_w((data & 8) ? ASSERT_LINE : CLEAR_LINE);
}

void micro20_state::portc_w(u8 data)
{
	// MSM58321 CS1 and CS2 are tied to /RST, inverted RESET.
	// So they're always high when the system is not reset.
	m_rtc->cs1_w(ASSERT_LINE);
	m_rtc->cs2_w(ASSERT_LINE);
	m_rtc->stop_w((data & 1) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->write_w((data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->read_w((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->address_write_w((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->test_w((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
}

u32 micro20_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xffff;
}
/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void micro20_state::micro20_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("mainram");
	map(0x00200000, 0x002fffff).r(FUNC(micro20_state::buserror_r));
	map(0x00800000, 0x0083ffff).rom().region("bootrom", 0);
	map(0xffff8000, 0xffff8000).rw(FDC_TAG, FUNC(wd1772_device::status_r), FUNC(wd1772_device::cmd_w));
	map(0xffff8001, 0xffff8001).rw(FDC_TAG, FUNC(wd1772_device::track_r), FUNC(wd1772_device::track_w));
	map(0xffff8002, 0xffff8002).rw(FDC_TAG, FUNC(wd1772_device::sector_r), FUNC(wd1772_device::sector_w));
	map(0xffff8003, 0xffff8003).rw(FDC_TAG, FUNC(wd1772_device::data_r), FUNC(wd1772_device::data_w));
	map(0xffff8080, 0xffff808f).rw(DUART_A_TAG, FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0xffff80a0, 0xffff80af).rw(DUART_B_TAG, FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0xffff80c0, 0xffff80df).rw(m_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void micro20_state::micro20(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 16.67_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &micro20_state::micro20_map);
	m_maincpu->reset_cb().set(FUNC(micro20_state::m68k_reset_callback));

	mc68681_device &duart_a(MC68681(config, DUART_A_TAG, 3.6864_MHz_XTAL));
	duart_a.a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(DUART_A_TAG, FUNC(mc68681_device::rx_a_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	MC68681(config, DUART_B_TAG, 3.6864_MHz_XTAL);

	WD1772(config, FDC_TAG, 16.67_MHz_XTAL / 2);

	PIT68230(config, m_pit, 16.67_MHz_XTAL / 2);
	m_pit->timer_irq_callback().set(FUNC(micro20_state::timerirq_w));
	m_pit->h4_out_callback().set(FUNC(micro20_state::h4_w));
	m_pit->pb_out_callback().set(FUNC(micro20_state::portb_w));
	m_pit->pc_out_callback().set(FUNC(micro20_state::portc_w));

	MSM58321(config, m_rtc, 32768_Hz_XTAL);
	m_rtc->set_default_24h(false);
	m_rtc->d0_handler().set(m_pit, FUNC(pit68230_device::pb0_w));
	m_rtc->d1_handler().set(m_pit, FUNC(pit68230_device::pb1_w));
	m_rtc->d2_handler().set(m_pit, FUNC(pit68230_device::pb2_w));
	m_rtc->d3_handler().set(m_pit, FUNC(pit68230_device::pb3_w));
	m_rtc->busy_handler().set(m_pit, FUNC(pit68230_device::pb7_w));

	TIMER(config, "timer").configure_periodic(FUNC(micro20_state::micro20_timer), attotime::from_hz(200));
}

static INPUT_PORTS_START( micro20 )
INPUT_PORTS_END

/***************************************************************************

  Machine driver(s)

***************************************************************************/


ROM_START( micro20 )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD32_BYTE( "d00-07_u6_6791.bin",  0x000003, 0x010000, CRC(63d66ea1) SHA1(c5dfbc4d81920e1d2e981c52c1af3d486d382a35) )
	ROM_LOAD32_BYTE( "d08-15_u8_0dc6.bin",  0x000002, 0x010000, CRC(d62ef21f) SHA1(2779d430b1a0b835807627e707d46547b29ef579) )
	ROM_LOAD32_BYTE( "d16-23_u10_e5b0.bin", 0x000001, 0x010000, CRC(cd7acf86) SHA1(db994ed714a1079fbb66616355e8f18d2d1a2005) )
	ROM_LOAD32_BYTE( "d24-31_u13_d115.bin", 0x000000, 0x010000, CRC(3646d943) SHA1(97ee54063e2fe49fef2ff68d0f2e39345a75eac5) )
ROM_END

} // anonymous namespace


COMP( 1984, micro20, 0, 0, micro20, micro20, micro20_state, empty_init, "GMX", "Micro 20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
