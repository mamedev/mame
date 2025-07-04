// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    esqkt.cpp - Ensoniq KT-76, KT-88, and E-Prime
                Ensoniq TS-10 and TS-12

    Driver by R. Belmont

    Hardware:
        CPU: 68EC020-16 CPU
        Serial/timers: SCN2681
        Sound: 2xES5506
        Effects: ES5510

    Memory map:

    0x000000-0x07FFFF   OS ROM (additional RAM on TS)
    0x200000-0x20003F   Master ES5506
    0x240000-0x24003F   Slave ES5506
    0x280000-0x2801FF   ES5510
    0x300000-0x30000F   68681 DUART
    0x400000-0xAFFFFF   User sample RAM (TS only)
    0xFF0000-0xFFFFFF   OS RAM

    Ensoniq KT-76
    Ensoniq 1994

    This is a wavetable-based synth keyboard made by Ensoniq in 1994

    PCB Layout
    ----------

    KT-76
    |---------------------------------------------|
    |J12 J3      J11                         J10  |
    |                                  LM393 LM358|
    |                                     74HC4051|
    |ADM691                                       |
    |              62256                          |
    |                                             |
    |3V_BATTERY  KT76_0590_LO.U5          ROM0    |
    |                                             |
    |   68EC020    62256       OTTOR2     ROM1    |
    |                                             |
    |            KT76_690B_HI.U6          ROM2    |
    | PAL1                                        |
    |       HP_6N138                              |
    | PAL2                     OTTOR2           J6|
    |      7407                                   |
    |                                             |
    |          D41464 D41464                      |
    |                                             |
    | SCN2681  D41464 D41464                      |
    |                         137000402           |
    |                                             |
    |                                18.432MHz    |
    |       ESPR6                    16MHz        |
    |    POT                                      |
    |                                  R1136-11   |
    |                                             |
    |                                LM339 LM339  |
    |    J13             J5  J1            J2  J4 |
    |---------------------------------------------|
    Notes:
          J1         - connector for digital jacks
          J2         - connector for keyboard
          J3         - connector for LCD display
          J4         - connector for keyboard
          J5         - connector for power input
          J6         - connector for wave expansion
          J10        - connector for wheels/pressure
          J11        - connector for memory card
          J12        - connector for headphones
          J13        - connector for analog jacks
          68EC020    - Motorola MC68EC020FG16 CPU. Clock input 16MHz
          1370000402 - Unknown PLCC44 IC stamped with the Ensoniq logo. Likely CPLD or gate array.
          ESPR6      - Ensoniq ESPR6 (ES5510) sound chip
          OTTOR2     - Ensoniq OTTOR2 (ES5506) sound chip
          POT        - ESP adjustment pot
          KT76*      - 27C2048/27C210 EPROM
          ROM*       - 2M x8-bit SOP44 mask ROM
          R1136-11   - DIP40 IC manufactured by Rockwell - believed to be some type of MCU.
          D41464     - NEC D41464 64k x4-bit DRAM
          62256      - 32k x8-bit SRAM
          SCN2681    - Philips SCN2681 Dual Universal Asynchronous Receiver/Transmitter (DUART)
          HP_6N138   - HP/Agilent HP 6N138 Low Input Current High Gain Optocoupler
          PAL1       - MMI PAL20L8ACN stamped 'KT-76 MMU 6A0A'. Printing is faint so 0 could be a B or a D.
          PAL2       - MMI PAL20L8ACN stamped 'KT-76 BCU 73D6'

***************************************************************************/

#include "emu.h"
#include "esqpanel.h"

#include "bus/midi/midi.h"
#include "cpu/es5510/es5510.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/mc68681.h"
#include "sound/es5506.h"
#include "sound/esqpump.h"

#include "speaker.h"


namespace {

class esqkt_state : public driver_device
{
public:
	esqkt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_esp(*this, "esp")
		, m_pump(*this, "pump")
		, m_duart(*this, "duart")
		, m_sq1panel(*this, "sq1panel")
		, m_mdout(*this, "mdout")
	{ }

	void kt(machine_config &config);
	void ts(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<es5510_device> m_esp;
	required_device<esq_5505_5510_pump_device> m_pump;
	required_device<scn2681_device> m_duart;
	required_device<esqpanel_device> m_sq1panel;
	required_device<midi_port_device> m_mdout;

	void duart_irq_handler(int state);
	void duart_tx_a(int state);
	void duart_tx_b(int state);
	void duart_output(u8 data);

	u16 *m_rom = nullptr, *m_ram = nullptr;
	u8 m_duart_io = 0;
	bool m_bCalibSecondByte = false; // only set to false on machine_reset()?

	void esq5506_otto_irq(int state);
	u16 esq5506_read_adc();
	void es5506_clock_changed(u32 data);
	void kt_map(address_map &map) ATTR_COLD;
	void ts_map(address_map &map) ATTR_COLD;
};

void esqkt_state::machine_start()
{
	m_rom = (u16 *)(void *)memregion("osrom")->base();
	m_ram = (u16 *)(void *)memshare("osram")->ptr();
	m_duart_io = 0;

	save_item(NAME(m_duart_io));
}

void esqkt_state::machine_reset()
{
	m_bCalibSecondByte = false;
	memcpy(m_ram, m_rom, 8);
}

void esqkt_state::kt_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("osrom", 0);
	map(0x200000, 0x20003f).rw("ensoniq1", FUNC(es5506_device::read), FUNC(es5506_device::write));
	map(0x240000, 0x24003f).rw("ensoniq2", FUNC(es5506_device::read), FUNC(es5506_device::write));
	map(0x280000, 0x2801ff).rw(m_esp, FUNC(es5510_device::host_r), FUNC(es5510_device::host_w));
	map(0x300000, 0x30001f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xff0000, 0xffffff).ram().share("osram");
}

void esqkt_state::ts_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share("osram");
	map(0x200000, 0x20001f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	// 280001/280003 = FDC uPD72065?
	// 2c0001 = ?
	map(0x300000, 0x30007f).rw("ensoniq", FUNC(es5506_device::read), FUNC(es5506_device::write)).umask16(0x00ff);
	map(0x380000, 0x3801ff).rw(m_esp, FUNC(es5510_device::host_r), FUNC(es5510_device::host_w)).umask16(0x00ff);
	map(0x400000, 0xbfffff).ram();  // user sample RAM
	map(0xc00000, 0xcfffff).rom().region("osrom", 0);
	map(0xff0000, 0xffffff).ram();
}

void esqkt_state::esq5506_otto_irq(int state)
{
	#if 0   // 5505/06 IRQ generation needs (more) work
	m_maincpu->set_input_line(1, state);
	#endif
}

u16 esqkt_state::esq5506_read_adc()
{
	switch ((m_duart_io & 7) ^ 7)
	{
		case 0:     // vRef to check battery
			return 0x5b00;

		case 2:     // battery voltage
			return 0x7f00;

		default: // pedal
			return 0;
	}

#ifdef UNUSED
	// Coverity 315636
	if (m_duart_io & 1)
	{
		return 0x5b00;              // vRef
	}
	else
	{
		return 0x7f00;              // vBattery
	}
#endif
}

void esqkt_state::es5506_clock_changed(u32 data)
{
	m_pump->set_unscaled_clock(data);
}

void esqkt_state::duart_irq_handler(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

void esqkt_state::duart_output(u8 data)
{
	m_duart_io = data;

//    printf("DUART output: %02x (PC=%x)\n", data, m_maincpu->pc());
}

void esqkt_state::duart_tx_a(int state)
{
	m_mdout->write_txd(state);
}

void esqkt_state::duart_tx_b(int state)
{
	m_sq1panel->rx_w(state);
}

void esqkt_state::kt(machine_config &config)
{
	M68EC020(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &esqkt_state::kt_map);

	ES5510(config, m_esp, 10_MHz_XTAL);
	m_esp->set_disable();

	auto &panel(ESQPANEL2X16_SQ1(config, "sq1panel"));
	panel.write_tx().set(m_duart, FUNC(scn2681_device::rx_b_w));

	SCN2681(config, m_duart, 4000000);
	m_duart->irq_cb().set(FUNC(esqkt_state::duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(esqkt_state::duart_tx_a));
	m_duart->b_tx_cb().set(FUNC(esqkt_state::duart_tx_b));
	m_duart->outport_cb().set(FUNC(esqkt_state::duart_output));
	m_duart->set_clocks(500000, 500000, 1000000, 1000000);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w)); // route MIDI Tx send directly to 68681 channel A Rx

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "speaker", 2).front();

	ESQ_5505_5510_PUMP(config, m_pump, 16_MHz_XTAL / (16 * 32));
	m_pump->set_esp(m_esp);
	m_pump->add_route(0, "speaker", 1.0, 0);
	m_pump->add_route(1, "speaker", 1.0, 1);

	auto &es5506a(ES5506(config, "ensoniq1", 16_MHz_XTAL));
	es5506a.sample_rate_changed().set(FUNC(esqkt_state::es5506_clock_changed)); // TODO : Sync with 2 chips?
	es5506a.set_region0("waverom");  /* Bank 0 */
	es5506a.set_region1("waverom2"); /* Bank 1 */
	es5506a.set_region2("waverom3"); /* Bank 0 */
	es5506a.set_region3("waverom4"); /* Bank 1 */
	es5506a.set_channels(4);          /* channels */
	es5506a.irq_cb().set(FUNC(esqkt_state::esq5506_otto_irq)); /* irq */
	es5506a.read_port_cb().set(FUNC(esqkt_state::esq5506_read_adc)); /* ADC */
	es5506a.add_route(0, "pump", 1.0, 0);
	es5506a.add_route(1, "pump", 1.0, 1);
	es5506a.add_route(2, "pump", 1.0, 2);
	es5506a.add_route(3, "pump", 1.0, 3);
	es5506a.add_route(4, "pump", 1.0, 4);
	es5506a.add_route(5, "pump", 1.0, 5);
	es5506a.add_route(6, "pump", 1.0, 6);
	es5506a.add_route(7, "pump", 1.0, 7);

	auto &es5506b(ES5506(config, "ensoniq2", 16_MHz_XTAL));
	es5506b.set_region0("waverom");  /* Bank 0 */
	es5506b.set_region1("waverom2"); /* Bank 1 */
	es5506b.set_region2("waverom3"); /* Bank 0 */
	es5506b.set_region3("waverom4"); /* Bank 1 */
	es5506b.set_channels(4);          /* channels */
	es5506b.add_route(0, "speaker", 1.0, 0);
	es5506b.add_route(1, "speaker", 1.0, 1);
	es5506b.add_route(2, "speaker", 1.0, 0);
	es5506b.add_route(3, "speaker", 1.0, 1);
	es5506b.add_route(4, "speaker", 1.0, 0);
	es5506b.add_route(5, "speaker", 1.0, 1);
	es5506b.add_route(6, "speaker", 1.0, 0);
	es5506b.add_route(7, "speaker", 1.0, 1);
}

void esqkt_state::ts(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &esqkt_state::ts_map);
	ES5510(config, m_esp, 10_MHz_XTAL);
	m_esp->set_disable();

	auto &panel(ESQPANEL2X40_VFX(config, "sq1panel"));
	panel.write_tx().set(m_duart, FUNC(scn2681_device::rx_b_w));

	SCN2681(config, m_duart, 4000000);
	m_duart->irq_cb().set(FUNC(esqkt_state::duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(esqkt_state::duart_tx_a));
	m_duart->b_tx_cb().set(FUNC(esqkt_state::duart_tx_b));
	m_duart->outport_cb().set(FUNC(esqkt_state::duart_output));
	m_duart->set_clocks(500000, 500000, 1000000, 1000000);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w)); // route MIDI Tx send directly to 68681 channel A Rx

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "speaker", 2).front();

	ESQ_5505_5510_PUMP(config, m_pump, 16_MHz_XTAL / (16 * 32));
	m_pump->set_esp(m_esp);
	m_pump->add_route(0, "speaker", 1.0, 0);
	m_pump->add_route(1, "speaker", 1.0, 1);

	auto &es5506a(ES5506(config, "ensoniq", 16_MHz_XTAL));
	es5506a.sample_rate_changed().set(FUNC(esqkt_state::es5506_clock_changed));
	es5506a.set_region0("waverom");  /* Bank 0 */
	es5506a.set_region1("waverom2"); /* Bank 1 */
	es5506a.set_region2("waverom3"); /* Bank 0 */
	es5506a.set_region3("waverom4"); /* Bank 1 */
	es5506a.set_channels(4);          /* channels */
	es5506a.irq_cb().set(FUNC(esqkt_state::esq5506_otto_irq)); /* irq */
	es5506a.read_port_cb().set(FUNC(esqkt_state::esq5506_read_adc)); /* ADC */
	es5506a.add_route(0, "pump", 1.0, 0);
	es5506a.add_route(1, "pump", 1.0, 1);
	es5506a.add_route(2, "pump", 1.0, 2);
	es5506a.add_route(3, "pump", 1.0, 3);
	es5506a.add_route(4, "pump", 1.0, 4);
	es5506a.add_route(5, "pump", 1.0, 5);
	es5506a.add_route(6, "pump", 1.0, 6);
	es5506a.add_route(7, "pump", 1.0, 7);
}
static INPUT_PORTS_START( kt )
INPUT_PORTS_END

ROM_START( kt76 )
	ROM_REGION32_BE(0x80000, "osrom", 0)
	ROM_LOAD32_WORD_SWAP( "kt76_162_lo.bin", 0x000002, 0x020000, CRC(1a1ab910) SHA1(dcc80db2297fd25993e090c2e5bb7f947319a8bf) )
	ROM_LOAD32_WORD_SWAP( "kt76_162_hi.bin", 0x000000, 0x040000, CRC(de16d236) SHA1(c55fca86453e90e8c34a048bed45817063237370) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000401_rom0.u103", 0x000001, 0x200000, CRC(425047af) SHA1(9680d1fc222b29ba24f0fbf6136982bee87a60ef) )

	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000402_rom1.u102", 0x000001, 0x200000, CRC(64459185) SHA1(0fa20b16847fc02a384057fc3d385226eb3e7527) )

	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000403_rom2.u104", 0x000001, 0x200000, CRC(c2aacc5d) SHA1(7fab518ba92ddb23cdc4dcb04751b26d25c298c0) )

	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

ROM_START( ts10 )
	ROM_REGION16_BE(0x100000, "osrom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "v310", "V310" )
	ROMX_LOAD( "ts10_310h.bin", 0x000000, 0x040000, CRC(cc04aa4f) SHA1(56761d29680bc99cfd625af3f92db836dfacdf31), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "ts10_310l.bin", 0x000001, 0x080000, CRC(51df8987) SHA1(294cde504a36752041deb0c09741153a797f2f28), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v120", "V120" )
	ROMX_LOAD( "ts10_120l.bin", 0x000000, 0x040000, CRC(047a0aaa) SHA1(6fd592e1a4e6c50877f4266f90620d5f2bd578aa), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "ts10_120h.bin", 0x000001, 0x040000, CRC(3ae66382) SHA1(ce603931d67735e2a8942b6c0992c138923858a6), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v202", "V202" )
	ROMX_LOAD( "ts10_202l.bin", 0x000000, 0x040000, CRC(509f1c46) SHA1(4feab8c6c1b2e96cf15ae59c0148b7308fd1ee7f), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "ts10_202h.bin", 0x000001, 0x040000, CRC(cacedb2c) SHA1(56fdf842a302b41ae2849c0e05ed1f38f42f2e18), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v305", "V305" )
	ROMX_LOAD( "ts10_305l.bin", 0x000000, 0x080000, CRC(3df39031) SHA1(3f14b56c7ea0c28f6173a7caf145d18e6287e33e), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "ts10_305h.bin", 0x000001, 0x040000, CRC(77d547e7) SHA1(268ae44b07a02686c31639c1344d23179a672f87), ROM_SKIP(1) | ROM_BIOS(3) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom4", ROMREGION_ERASE00)
ROM_END

ROM_START( ts12 )
	ROM_REGION16_BE(0x100000, "osrom", 0)
	ROM_SYSTEM_BIOS( 0, "v310", "V310" )
	ROMX_LOAD( "ts12-v310-hig.bin", 0x000000, 0x040000, CRC(99823433) SHA1(bd39a5d27824988cb531d90b91685a2362a98b3e), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "ts12-v310-low.bin", 0x000001, 0x080000, CRC(7e64a659) SHA1(c567c2d349e5a58928a74ac22473896c6814bda8), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v305", "V305" )
	ROMX_LOAD( "ts12_305h.bin", 0x000000, 0x040000, CRC(7968ca06) SHA1(2460d4a824445425d0d733faad1f9b3829fa19ae), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "ts12_305l.bin", 0x000000, 0x080000, CRC(30b4bece) SHA1(41b2f1df99b8106c96d532702de93badea085ed7), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x400000, "waverom4", ROMREGION_ERASE00)
ROM_END

} // Anonymous namespace


CONS( 1993, ts10, 0, 0, ts, kt, esqkt_state, empty_init, "Ensoniq", "TS-10", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )
CONS( 1993, ts12, 0, 0, ts, kt, esqkt_state, empty_init, "Ensoniq", "TS-12", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )
CONS( 1996, kt76, 0, 0, kt, kt, esqkt_state, empty_init, "Ensoniq", "KT-76", MACHINE_IMPERFECT_SOUND )
