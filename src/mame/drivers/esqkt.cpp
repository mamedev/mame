// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    esqkt.c - Ensoniq KT-76, KT-88, and E-Prime

    Driver by R. Belmont

    Hardware:
        CPU: 68EC020-16 CPU
        Serial/timers: SCN2681 (MC68681 clone)
        Sound: 2xES5506
        Effects: ES5510

    Memory map:

    0x000000-0x07FFFF   OS ROM
    0x200000-0x20003F   Master ES5506
    0x240000-0x24003F   Slave ES5506
    0x280000-0x2801FF   ES5510
    0x300000-0x30000F   68681 DUART
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

#include "bus/midi/midi.h"
#include "cpu/m68000/m68000.h"
#include "cpu/es5510/es5510.h"
#include "sound/es5506.h"
#include "machine/mc68681.h"
#include "machine/esqpanel.h"

class esqkt_state : public driver_device
{
public:
	esqkt_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_esp(*this, "esp"),
		m_duart(*this, "duart"),
		m_sq1panel(*this, "sq1panel"),
		m_mdout(*this, "mdout")
	{ }

	required_device<m68ec020_device> m_maincpu;
	required_device<es5510_device> m_esp;
	required_device<mc68681_device> m_duart;
	required_device<esqpanel2x40_sq1_device> m_sq1panel;
	required_device<midi_port_device> m_mdout;

	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart_tx_a);
	DECLARE_WRITE_LINE_MEMBER(duart_tx_b);
	DECLARE_WRITE8_MEMBER(duart_output);

	UINT8 m_duart_io;
	bool  m_bCalibSecondByte;

private:
	UINT32  *m_rom, *m_ram;

public:
	DECLARE_DRIVER_INIT(kt);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
	DECLARE_WRITE_LINE_MEMBER(esq5506_otto_irq);
	DECLARE_READ16_MEMBER(esq5506_read_adc);
};

void esqkt_state::machine_reset()
{
	m_bCalibSecondByte = false;
}

static ADDRESS_MAP_START( kt_map, AS_PROGRAM, 32, esqkt_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_REGION("osrom", 0)
	AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0xffffffff)
	AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8("ensoniq2", es5506_device, read, write, 0xffffffff)
	AM_RANGE(0x280000, 0x2801ff) AM_DEVREADWRITE8("esp", es5510_device, host_r, host_w, 0xffffffff)
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE8("duart", mc68681_device, read, write, 0xffffffff)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

WRITE_LINE_MEMBER(esqkt_state::esq5506_otto_irq)
{
	#if 0   // 5505/06 IRQ generation needs (more) work
	m_maincpu->set_input_line(1, state);
	#endif
}

READ16_MEMBER(esqkt_state::esq5506_read_adc)
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

	if (m_duart_io & 1)
	{
		return 0x5b00;              // vRef
	}
	else
	{
		return 0x7f00;              // vBattery
	}
}

WRITE_LINE_MEMBER(esqkt_state::duart_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

WRITE8_MEMBER(esqkt_state::duart_output)
{
	m_duart_io = data;

//    printf("DUART output: %02x (PC=%x)\n", data, m_maincpu->pc());
}

WRITE_LINE_MEMBER(esqkt_state::duart_tx_a)
{
	m_mdout->write_txd(state);
}

WRITE_LINE_MEMBER(esqkt_state::duart_tx_b)
{
	m_sq1panel->rx_w(state);
}

static MACHINE_CONFIG_START( kt, esqkt_state )
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(kt_map)

	MCFG_CPU_ADD("esp", ES5510, XTAL_10MHz)
	MCFG_DEVICE_DISABLE()

	MCFG_ESQPANEL2x40_SQ1_ADD("sq1panel")
	MCFG_ESQPANEL_TX_CALLBACK(DEVWRITELINE("duart", mc68681_device, rx_b_w))

	MCFG_MC68681_ADD("duart", 4000000)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(esqkt_state, duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(WRITELINE(esqkt_state, duart_tx_a))
	MCFG_MC68681_B_TX_CALLBACK(WRITELINE(esqkt_state, duart_tx_b))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(esqkt_state, duart_output))
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(500000, 500000, 1000000, 1000000)
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(500000, 500000, 1000000, 1000000)

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE("duart", mc68681_device, rx_a_w)) // route MIDI Tx send directly to 68681 channel A Rx

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(esqkt_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(esqkt_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
	MCFG_SOUND_ADD("ensoniq2", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
MACHINE_CONFIG_END

static INPUT_PORTS_START( kt )
INPUT_PORTS_END

ROM_START( kt76 )
	ROM_REGION(0x80000, "osrom", 0)
	ROM_LOAD32_WORD( "kt76_162_lo.bin", 0x000000, 0x020000, CRC(1a1ab910) SHA1(dcc80db2297fd25993e090c2e5bb7f947319a8bf) )
	ROM_LOAD32_WORD( "kt76_162_hi.bin", 0x000002, 0x040000, CRC(de16d236) SHA1(c55fca86453e90e8c34a048bed45817063237370) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000401_rom0.u103", 0x000001, 0x200000, CRC(425047af) SHA1(9680d1fc222b29ba24f0fbf6136982bee87a60ef) )

	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000402_rom1.u102", 0x000001, 0x200000, CRC(64459185) SHA1(0fa20b16847fc02a384057fc3d385226eb3e7527) )

	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "1351000403_rom2.u104", 0x000001, 0x200000, CRC(c2aacc5d) SHA1(7fab518ba92ddb23cdc4dcb04751b26d25c298c0) )

	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqkt_state, kt)
{
	m_duart_io = 0;
}

CONS( 1996, kt76, 0, 0, kt, kt, esqkt_state, kt, "Ensoniq", "KT-76", MACHINE_IMPERFECT_SOUND )
