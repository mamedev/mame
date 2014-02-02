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

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/es5510/es5510.h"
#include "sound/es5506.h"
#include "machine/n68681.h"
#include "machine/esqpanel.h"
#include "machine/serial.h"
#include "machine/midiinport.h"
#include "machine/midioutport.h"

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
	required_device<duartn68681_device> m_duart;
	required_device<esqpanel2x40_sq1_device> m_sq1panel;
	required_device<serial_port_device> m_mdout;

	virtual void machine_reset();

	DECLARE_READ32_MEMBER(lower_r);
	DECLARE_WRITE32_MEMBER(lower_w);

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart_tx_a);
	DECLARE_WRITE_LINE_MEMBER(duart_tx_b);
	DECLARE_READ8_MEMBER(duart_input);
	DECLARE_WRITE8_MEMBER(duart_output);

	UINT8 m_duart_io;
	bool  m_bCalibSecondByte;

private:
	UINT32  *m_rom, *m_ram;

public:
	DECLARE_DRIVER_INIT(kt);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
	DECLARE_WRITE_LINE_MEMBER(esq5506_otto_irq);
};

void esqkt_state::machine_reset()
{
	m_bCalibSecondByte = false;
}

READ32_MEMBER(esqkt_state::lower_r)
{
	offset &= 0x3fff;

	// get pointers when 68k resets
	if (!m_rom)
	{
		m_rom = (UINT32 *)memregion("osrom")->base();
		m_ram = (UINT32 *)memshare("osram")->ptr();
	}

	if (offset < 0x2000)
	{
		if (m68k_get_fc(m_maincpu) == 0x6)  // supervisor mode = ROM
		{
			return m_rom[offset];
		}
		else
		{
			return m_ram[offset];
		}
	}
	else
	{
		return m_ram[offset];
	}
}

WRITE32_MEMBER(esqkt_state::lower_w)
{
	offset &= 0x3fff;

	if (offset < 0x2000)
	{
		if (m68k_get_fc(m_maincpu) != 0x6)  // if not supervisor mode, RAM
		{
			COMBINE_DATA(&m_ram[offset]);
		}
		else
		{
			logerror("Write to ROM: %x @ %x (fc=%x)\n", data, offset, m68k_get_fc(m_maincpu));
		}
	}
	else
	{
		COMBINE_DATA(&m_ram[offset]);
	}
}

static ADDRESS_MAP_START( kt_map, AS_PROGRAM, 32, esqkt_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_REGION("osrom", 0)
	AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0xffffffff)
	AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8("ensoniq2", es5506_device, read, write, 0xffffffff)
	AM_RANGE(0x280000, 0x2801ff) AM_DEVREADWRITE8("esp", es5510_device, host_r, host_w, 0xffffffff)
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE8("duart", duartn68681_device, read, write, 0xffffffff)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

WRITE_LINE_MEMBER(esqkt_state::esq5506_otto_irq)
{
	#if 0   // 5505/06 IRQ generation needs (more) work
	m_maincpu->set_input_line(1, state);
	#endif
}

static READ16_DEVICE_HANDLER(esq5506_read_adc)
{
	esqkt_state *state = device->machine().driver_data<esqkt_state>();

	switch ((state->m_duart_io & 7) ^ 7)
	{
		case 0:     // vRef to check battery
			return 0x5b00;

		case 2:     // battery voltage
			return 0x7f00;

		default: // pedal
			return 0;
	}

	if (state->m_duart_io & 1)
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
};

READ8_MEMBER(esqkt_state::duart_input)
{
	UINT8 result = 0;   // DUART input lines are separate from the output lines

	return result;
}

WRITE8_MEMBER(esqkt_state::duart_output)
{
	m_duart_io = data;

//    printf("DUART output: %02x (PC=%x)\n", data, m_maincpu->pc());
}

WRITE_LINE_MEMBER(esqkt_state::duart_tx_a)
{
	m_mdout->tx(state);
}

WRITE_LINE_MEMBER(esqkt_state::duart_tx_b)
{
	m_sq1panel->rx_w(state);
}

static const es5506_interface es5506_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	1,          /* channels */
	DEVCB_DRIVER_LINE_MEMBER(esqkt_state,esq5506_otto_irq), /* irq */
	DEVCB_DEVICE_HANDLER(DEVICE_SELF, esq5506_read_adc)
};

static const es5506_interface es5506_2_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	1,          /* channels */
	DEVCB_NULL,
	DEVCB_NULL
};

static const esqpanel_interface esqpanel_config =
{
	DEVCB_DEVICE_LINE_MEMBER("duart", duartn68681_device, rx_b_w)
};

static SLOT_INTERFACE_START(midiin_slot)
	SLOT_INTERFACE("midiin", MIDIIN_PORT)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(midiout_slot)
	SLOT_INTERFACE("midiout", MIDIOUT_PORT)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( kt, esqkt_state )
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(kt_map)

	MCFG_CPU_ADD("esp", ES5510, XTAL_10MHz)
	MCFG_DEVICE_DISABLE()

	MCFG_ESQPANEL2x40_SQ1_ADD("sq1panel", esqpanel_config)

	MCFG_DUARTN68681_ADD("duart", 4000000)
	MCFG_DUARTN68681_IRQ_CALLBACK(WRITELINE(esqkt_state, duart_irq_handler))
	MCFG_DUARTN68681_A_TX_CALLBACK(WRITELINE(esqkt_state, duart_tx_a))
	MCFG_DUARTN68681_B_TX_CALLBACK(WRITELINE(esqkt_state, duart_tx_b))
	MCFG_DUARTN68681_INPORT_CALLBACK(READ8(esqkt_state, duart_input))
	MCFG_DUARTN68681_OUTPORT_CALLBACK(WRITE8(esqkt_state, duart_output))
	MCFG_DUARTN68681_SET_EXTERNAL_CLOCKS(500000, 500000, 1000000, 1000000)
	MCFG_DUARTN68681_SET_EXTERNAL_CLOCKS(500000, 500000, 1000000, 1000000)

	MCFG_SERIAL_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("duart", duartn68681_device, rx_a_w)) // route MIDI Tx send directly to 68681 channel A Rx

	MCFG_SERIAL_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
	MCFG_SOUND_ADD("ensoniq2", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_2_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( kt )
INPUT_PORTS_END

ROM_START( kt76 )
	ROM_REGION(0x80000, "osrom", 0)
	ROM_LOAD32_WORD( "kt76_162_lo.bin", 0x000000, 0x020000, CRC(1a1ab910) SHA1(dcc80db2297fd25993e090c2e5bb7f947319a8bf) )
	ROM_LOAD32_WORD( "kt76_162_hi.bin", 0x000002, 0x040000, CRC(de16d236) SHA1(c55fca86453e90e8c34a048bed45817063237370) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqkt_state, kt)
{
	m_duart_io = 0;
}

CONS( 1996, kt76, 0, 0, kt, kt, esqkt_state, kt, "Ensoniq", "KT-76", GAME_NOT_WORKING )
