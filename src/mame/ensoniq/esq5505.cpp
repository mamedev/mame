// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/***************************************************************************

    esq5505.cpp - Ensoniq ES5505 + ES5510 based synthesizers and samplers

    Ensoniq VFX, VFX-SD, EPS, EPS-16 Plus, SD-1, SD-1 32, SQ-1 and SQ-R (SQ-1 Plus,
    SQ-2, and KS-32 are known to also be this architecture).

    The Taito sound system in taito_en.cpp is directly derived from the 32-voice version
    of the SD-1.

    Driver by R. Belmont and Parduz with thanks to Christian Brunschen, and Phil Bennett

    Memory map:

    0x000000-0x007fff   work RAM low (64k for SQ-1 and later)
    0x200000-0x20001f   OTIS (5505) regs
    0x240000-0x2400ff   DMAC (68450) regs (EPS/EPS-16)
    0x260000-0x2601ff   ESP (5510) regs
    0x280000-0x28001f   DUART (68681) regs
    0x2C0000-0x2C0003   Floppy (WD1772) regs (VFX-SD, SD-1, and EPS/EPS-16)
    0x2e0000-0x2fffff   Expansion cartridge (VFX, VFX-SD, SD-1, SD-1 32 voice)
    0x300000-0x300003   EPS/EPS-16 SCSI (WD33C93, register at 300001, data at 300003)
    0x330000-0x37ffff   VFX-SD / SD-1 sequencer RAM
    0x340000-0x3bffff   EPS/EPS-16 sample RAM
    0xc00000-0xc3ffff   OS ROM
    0xff8000-0xffffff   work RAM hi (64k for SQ-1 and later)

    Note from es5700.pdf PLA equations:
    RAM if (A23/22/21 = 000 and FC is not 6) or (A23/22/21 = 111 and FC is not 7)
    ROM if (A23/22/21 = 110) or (A23/22/21 = 000 & FC is 6)

    Interrupts:
    5505 interrupts are on normal autovector IRQ 1
    DMAC interrupts (EPS only) are on autovector IRQ 2
    68681 uses custom vector 0x40 (address 0x100) level 3

    VFX / VFX-SD / SD-1 / SD-1 32 panel button codes:
    2 = PROGRAM CONTROL
    3 = WRITE
    4 = WAVE
    5 = SELECT VOICE
    6 = MIXER/SHAPER
    7 = EFFECT
    8 = COMPARE
    9 = COPY EFFECTS PARAMETERS
    10 = LFO
    11 = PITCH
    12 = ENV1
    13 = PITCH MOD
    14 = ENV2
    15 = FILTER
    16 = ENV3
    17 = OUTPUT
    18 = ERROR 20 (VFX) / SEQ. CONTROL
    19 = RECORD
    20 = MASTER
    21 = STORAGE
    22 = STOP/CONT
    23 = PLAY
    24 = MIDI
    25 = BUTTON 9
    26 = PSEL
    27 = STAT
    28 = EFFECT
    29 = SEQ?  (toggles INT0 / TRAX display)
    30 = TRACKS 1-6
    31 = TRACKS 7-12
    32 = ERROR 20 (VFX) / CLICK-REC
    33 = ERROR 20 (VFX) / LOCATE
    34 = BUTTON 8
    35 = BUTTON 7
    36 = VOLUME
    37 = PAN
    38 = TIMBRE
    39 = KEY ZONE
    40 = TRANSPOSE
    41 = RELEASE
    42 = SOFT TOP CENTER
    43 = SOFT TOP RIGHT
    44 = SOFT BOTTOM CENTER
    45 = SOFT BOTTOM RIGHT
    46 = BUTTON 3
    47 = BUTTON 4
    48 = BUTTON 5
    49 = BUTTON 6
    50 = SOFT BOTTOM LEFT
    51 = ERROR 202 (VFX) / SEQ.
    52 = CART
    53 = SOUNDS
    54 = PRESETS
    55 = BUTTON 0
    56 = BUTTON 1
    57 = BUTTON 2
    58 = SOFT TOP LEFT
    59 = ERROR 20 (VFX) / EDIT SEQUENCE
    60 = ERROR 20 (VFX) / EDIT SONG
    61 = ERROR 20 (VFX) / EDIT TRACK
    62 = DATA INCREMENT
    63 = DATA DECREMENT

    VFX / VFX-SD / SD-1 analog values: all values are 10 bits, left-justified within 16 bits.
    0 = Pitch Bend
    1 = Patch Select
    2 = Mod Wheel
    3 = Value, aka Data Entry Slider
    4 = Pedal / CV
    5 = Volume Slider
    6 = Battery
    7 = Voltage Reference

    SQ-1:
    4 = second digit of patch # becomes 2
    5 = first digit of patch # becomes 2
    6 = second digit of patch # becomes 4
    7 = first digit of patch # becomes 4
    8 = trk07 4volume=99
    12 = patch -1
    13 = patch +1
    14 = second digit of patch # becomes 5
    15 = first digit of patch # becomes 5
    20 = select sound?
    22 = second digit of patch # becomes 6
    23 = first digit of patch # becomes 6

    Ensoniq SQ-2 MIDI keyboard
    Ensoniq 1992
    PCB Layout by Guru


    PART NO.: 4001018001 REV B

       |--|  |--|  |--|    |--|                       |--|     |--|   |--|   |--|
    |--|J1|--|J2|--|J3|----|J4|-----------------------|J7|-----|J8|---|J9|---|J10--|
    | PHONE  RAUD  LAUD    PEDAL      3V_BATT         FOOT     MIDI   MIDI   MIDI  |
    |                      CV   74LS244 74LS244 74LS245        IN     OUT    THRU  |
    |                                                                              |
    |    TL072                  J5                                HP6N138   |---|  |
    |            7912       4051      CARTRIDGE       LOWER.U27  UPPER.U32  | 6 |LM2926
    |                 J15         |-------J6------|       62256       62256 | 8 |  |
    |                             |---------------|   74HC4053              | 0 |7805
    |            7812           7407                          ENSONIQ       | 0 |  |
    |    TL072            TL072               ENSONIQ         GLU           | 0 |  |
    |                                         SUPERGLU                      |---|  |
    |                                                                              |
    |                                            16MHz  30.47618MHz                |
    |                                                                         POWER|
    |TDA1541A        ROM3.U38   ROM1.U26    74HC74                LM339         J12|
    |                                                                              |
    |                    ROM2.U39   ROM0.U25              NCR6500/11  J13          |
    |                                                                              |
    |        OTISR2             74LS373      ESP        68681                      |
    |                                                                              |
    |7805    74HC74 74F139  6264      6264   ADJ-POT  ESPR6       LM339            |
    |                                                                              |
    |7915    74LS174 74HC161    74LS373    LM317T                 J11 J14          |
    |------------------------------------------------------------------------------|
    Note: All parts shown.

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/es5510/es5510.h"
#include "cpu/m68000/m68000.h"
#include "formats/esq16_dsk.h"
#include "imagedev/floppy.h"
#include "esqlcd.h"
#include "esqpanel.h"
#include "esqvfd.h"
#include "machine/hd63450.h"    // compatible with MC68450, which is what these really have
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"
#include "sound/es5506.h"
#include "sound/esqpump.h"
#include "emupal.h"
#include "speaker.h"

#include <cstdarg>
#include <cstdio>


namespace {

#define GENERIC (0)
#define EPS     (1)
#define SQ1     (2)

#define KEYBOARD_HACK (1)   // turn on to play the SQ-1, SD-1, and SD-1 32-voice: Z and X are program up/down, A/S/D/F/G/H/J/K/L and Q/W/E/R/T/Y/U play notes

#if KEYBOARD_HACK
static int shift = 32;
#endif

#if 0
static void ATTR_PRINTF(1,2) print_to_stderr(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
}
#endif

class esq5505_state : public driver_device
{
public:
	esq5505_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_esp(*this, "esp")
		, m_pump(*this, "pump")
		, m_fdc(*this, "wd1772")
		, m_floppy_connector(*this, "wd1772:0")
		, m_panel(*this, "panel")
		, m_dmac(*this, "mc68450")
		, m_mdout(*this, "mdout")
		, m_rom(*this, "osrom")
		, m_ram(*this, "osram")
	{ }

	void sq1(machine_config &config);
	void vfx(machine_config &config);
	void vfxsd(machine_config &config);
	void eps(machine_config &config);
	void vfx32(machine_config &config);
	void ks32(machine_config &config);

	void init_eps();
	void init_common();
	void init_sq1();
	void init_denib();
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

	void esq5505_otis_irq(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<es5510_device> m_esp;
	required_device<esq_5505_5510_pump_device> m_pump;
	optional_device<wd1772_device> m_fdc;
	optional_device<floppy_connector> m_floppy_connector;
	required_device<esqpanel_device> m_panel;
	optional_device<hd63450_device> m_dmac;
	required_device<midi_port_device> m_mdout;
	required_region_ptr<uint16_t> m_rom;
	required_shared_ptr<uint16_t> m_ram;

	uint16_t lower_r(offs_t offset);
	void lower_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t analog_r();
	void analog_w(offs_t offset, uint16_t data);

	void duart_irq_handler(int state);
	void duart_tx_a(int state);
	void duart_tx_b(int state);
	void duart_output(uint8_t data);

	void es5505_clock_changed(u32 data);

	int m_system_type = 0;
	uint8_t m_duart_io = 0;
	uint8_t m_otis_irq_state = 0;
	uint8_t m_dmac_irq_state = 0;
	uint8_t m_duart_irq_state = 0;

	void update_irq_to_maincpu();

	static void floppy_formats(format_registration &fr);

	void eps_map(address_map &map) ATTR_COLD;
	void sq1_map(address_map &map) ATTR_COLD;
	void vfx_map(address_map &map) ATTR_COLD;
	void vfxsd_map(address_map &map) ATTR_COLD;

	void cpu_space_map(address_map &map) ATTR_COLD;
	void eps_cpu_space_map(address_map &map) ATTR_COLD;

	uint16_t m_analog_values[8];

	//dmac
	void dma_irq(int state);
};

void esq5505_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ESQIMG_FORMAT);
}

void esq5505_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff7, 0xfffff7).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

void esq5505_state::eps_cpu_space_map(address_map &map)
{
	cpu_space_map(map);
	map(0xfffff5, 0xfffff5).r(m_dmac, FUNC(hd63450_device::iack));
}

void esq5505_state::machine_start()
{
	m_otis_irq_state = 0;
	m_dmac_irq_state = 0;
	m_duart_irq_state = 0;
}

void esq5505_state::machine_reset()
{
	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;

	// Default analog values: all values are 10 bits, left-justified within 16 bits.
	m_analog_values[0] = 0x7fc0; // pitch mod: start in the center
	m_analog_values[1] = 0x0000; // patch select: nothing pressed.
	m_analog_values[2] = 0xffc0; // mod wheel: at the bottom, no modulation
	m_analog_values[3] = 0xccc0; // data entry: somewhere in the middle
	m_analog_values[4] = 0xffc0; // control voltage / pedal: full on.
	m_analog_values[5] = 0xffc0; // Volume control: full on.
	m_analog_values[6] = 0x7fc0; // Battery voltage: something reasonable.
	m_analog_values[7] = 0x5540; // vRef to check battery.

	// on VFX, bit 0 is 1 for 'cartridge present'.
	// on VFX-SD and later, bit 0 is2 1 for floppy present, bit 1 is 1 for cartridge present
	if (strcmp(machine().system().name, "vfx") == 0)
	{
		// todo: handle VFX cart-in when we support cartridges
		m_duart->ip0_w(ASSERT_LINE);
	}
	else
	{
		m_duart->ip1_w(CLEAR_LINE);

		if (floppy)
		{
			m_duart->ip0_w(CLEAR_LINE);
		}
		else
		{
			m_duart->ip0_w(ASSERT_LINE);
		}
	}
}

void esq5505_state::update_irq_to_maincpu()
{
	// printf("updating IRQ state: have OTIS=%d, DMAC=%d, DUART=%d\n", m_otis_irq_state, m_dmac_irq_state, m_duart_irq_state);
	if (m_duart_irq_state)
	{
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
	}
	else if (m_dmac_irq_state)
	{
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
	}
	else if (m_otis_irq_state)
	{
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
}

uint16_t esq5505_state::lower_r(offs_t offset)
{
	offset &= 0x7fff;

	if (!machine().side_effects_disabled() && m_maincpu->get_fc() == 0x6)  // supervisor mode = ROM
	{
		return m_rom[offset];
	}
	else
	{
		return m_ram[offset];
	}
}

void esq5505_state::lower_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= 0x7fff;

	if (offset < 0x4000)
	{
		if (m_maincpu->get_fc() != 0x6)  // if not supervisor mode, RAM
		{
			COMBINE_DATA(&m_ram[offset]);
		}
		else
		{
			logerror("Write to ROM: %x @ %x (fc=%x)\n", data, offset, m_maincpu->get_fc());
		}
	}
	else
	{
		COMBINE_DATA(&m_ram[offset]);
	}
}

void esq5505_state::vfx_map(address_map &map)
{
	map(0x000000, 0x007fff).rw(FUNC(esq5505_state::lower_r), FUNC(esq5505_state::lower_w));
	map(0x200000, 0x20001f).rw("otis", FUNC(es5505_device::read), FUNC(es5505_device::write));
	map(0x280000, 0x28001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x260000, 0x2601ff).rw(m_esp, FUNC(es5510_device::host_r), FUNC(es5510_device::host_w)).umask16(0x00ff);
	map(0xc00000, 0xc1ffff).rom().region("osrom", 0);
	map(0xff0000, 0xffffff).ram().share("osram");
}

void esq5505_state::vfxsd_map(address_map &map)
{
	map(0x000000, 0x00ffff).rw(FUNC(esq5505_state::lower_r), FUNC(esq5505_state::lower_w));
	map(0x200000, 0x20001f).rw("otis", FUNC(es5505_device::read), FUNC(es5505_device::write));
	map(0x280000, 0x28001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x260000, 0x2601ff).rw(m_esp, FUNC(es5510_device::host_r), FUNC(es5510_device::host_w)).umask16(0x00ff);
	map(0x2c0000, 0x2c0007).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x330000, 0x3bffff).ram(); // sequencer memory?
	map(0xc00000, 0xc3ffff).rom().region("osrom", 0);
	map(0xff0000, 0xffffff).ram().share("osram");
}

void esq5505_state::eps_map(address_map &map)
{
	map(0x000000, 0x007fff).rw(FUNC(esq5505_state::lower_r), FUNC(esq5505_state::lower_w));
	map(0x200000, 0x20001f).rw("otis", FUNC(es5505_device::read), FUNC(es5505_device::write));
	map(0x240000, 0x2400ff).rw(m_dmac, FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x280000, 0x28001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x2c0000, 0x2c0007).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x580000, 0x7fffff).ram();         // sample RAM?
	map(0xc00000, 0xc1ffff).rom().region("osrom", 0);
	map(0xff0000, 0xffffff).ram().share("osram");
}

void esq5505_state::sq1_map(address_map &map)
{
	map(0x000000, 0x03ffff).rw(FUNC(esq5505_state::lower_r), FUNC(esq5505_state::lower_w));
	map(0x200000, 0x20001f).rw("otis", FUNC(es5505_device::read), FUNC(es5505_device::write));
	map(0x260000, 0x2601ff).rw(m_esp, FUNC(es5510_device::host_r), FUNC(es5510_device::host_w)).umask16(0x00ff);
	map(0x280000, 0x28001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x330000, 0x3bffff).ram(); // sequencer memory?
	map(0xc00000, 0xc3ffff).rom().region("osrom", 0);
	map(0xff0000, 0xffffff).ram().share("osram");
}

void esq5505_state::esq5505_otis_irq(int state)
{
	m_otis_irq_state = (state != 0);
	update_irq_to_maincpu();
}

void esq5505_state::es5505_clock_changed(u32 data)
{
	m_pump->set_unscaled_clock(data);
}

void esq5505_state::analog_w(offs_t offset, uint16_t data)
{
	offset &= 0x7;
	m_analog_values[offset] = data;
}

uint16_t esq5505_state::analog_r()
{
	return m_analog_values[m_duart_io & 7];
}

void esq5505_state::duart_irq_handler(int state)
{
//    printf("\nDUART IRQ: state %d vector %d\n", state, vector);
	if (state == ASSERT_LINE)
	{
		m_duart_irq_state = 1;
	}
	else
	{
		m_duart_irq_state = 0;
	}
	update_irq_to_maincpu();
}

void esq5505_state::duart_output(uint8_t data)
{
	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;

	m_duart_io = data;

	/*
	    EPS:
	    bit 2 = SSEL

	    VFX:
	    bits 0/1/2 = analog sel
	    bit 6 = ESPHALT
	    bit 7 = SACK (?)

	    VFX-SD & SD-1 (32):
	    bits 0/1/2 = analog sel
	    bit 3 = SSEL (disk side)
	    bit 4 = DSEL (drive select?)
	    bit 6 = ESPHALT
	    bit 7 = SACK (?)
	*/

	if (data & 0x40)
	{
		if (!m_pump->get_esp_halted())
		{
			logerror("ESQ5505: Asserting ESPHALT\n");
			m_pump->set_esp_halted(true);
		}
	}
	else
	{
		if (m_pump->get_esp_halted())
		{
			logerror("ESQ5505: Clearing ESPHALT\n");
			m_pump->set_esp_halted(false);
		}
	}

	if (floppy)
	{
		if (m_system_type == EPS)
		{
			floppy->ss_w((data & 2)>>1);
		}
		else
		{
			floppy->ss_w(((data & 8)>>3)^1);
		}
	}

//    printf("DUART output: %02x (PC=%x)\n", data, m_maincpu->pc());
}

// MIDI send
void esq5505_state::duart_tx_a(int state)
{
	m_mdout->write_txd(state);
}

void esq5505_state::duart_tx_b(int state)
{
	m_panel->rx_w(state);
}

void esq5505_state::dma_irq(int state)
{
	if (state != CLEAR_LINE)
	{
		logerror("DMAC error, vector = %x\n", m_dmac->iack());
		m_dmac_irq_state = 1;
	}
	else
	{
		m_dmac_irq_state = 0;
	}

	update_irq_to_maincpu();
}

#if KEYBOARD_HACK
INPUT_CHANGED_MEMBER(esq5505_state::key_stroke)
{
	int val = (uint8_t)param;
	int cmp = 0x60;

	if (m_system_type == SQ1)
	{
		cmp = 10;
	}

	if (val < cmp)
	{
		if (oldval == 0 && newval == 1)
		{
			if (val == 0 && shift > 0)
			{
				shift -= 32;
				printf("New shift %d\n", shift);
			}
			else if (val == 1 && shift < 32)
			{
				shift += 32;
				printf("New shift %d\n", shift);
			}
			else if (val == 0x02)
			{
//              printf("Analog tests!\n");
				m_panel->xmit_char(54 | 0x80); m_panel->xmit_char(0); // Preset down
				m_panel->xmit_char(8 | 0x80);  m_panel->xmit_char(0); // Compare down
				m_panel->xmit_char(8);         m_panel->xmit_char(0); // Compare up
				m_panel->xmit_char(54);        m_panel->xmit_char(0); // Preset up
			}
		}
	}
	else
	{
		if (val < 20) val += shift;
		if (oldval == 0 && newval == 1)
		{
	//      printf("key pressed %d\n", val);
			m_panel->xmit_char(val);
			m_panel->xmit_char(0x00);
		}
		else if (oldval == 1 && newval == 0)
		{
	//        printf("key off %x\n", (uint8_t)param);
			m_panel->xmit_char(val&0x7f);
			m_panel->xmit_char(0x00);
		}
	}
}
#endif

void esq5505_state::vfx(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::vfx_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &esq5505_state::cpu_space_map);

	ES5510(config, m_esp, 10_MHz_XTAL);
	m_esp->set_disable();

	ESQPANEL2X40_VFX(config, m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(esq5505_state::analog_w));

	MC68681(config, m_duart, 4000000);
	m_duart->irq_cb().set(FUNC(esq5505_state::duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(esq5505_state::duart_tx_a));
	m_duart->b_tx_cb().set(FUNC(esq5505_state::duart_tx_b));
	m_duart->outport_cb().set(FUNC(esq5505_state::duart_output));
	m_duart->set_clocks(500000, 500000, 1000000, 1000000);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w)); // route MIDI Tx send directly to 68681 channel A Rx

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "speaker", 2).front();

	ESQ_5505_5510_PUMP(config, m_pump, 10_MHz_XTAL / (16 * 21));
	m_pump->set_esp(m_esp);
	m_pump->add_route(0, "speaker", 1.0, 0);
	m_pump->add_route(1, "speaker", 1.0, 1);

	auto &es5505(ES5505(config, "otis", 10_MHz_XTAL));
	es5505.sample_rate_changed().set(FUNC(esq5505_state::es5505_clock_changed));
	es5505.set_region0("waverom");  /* Bank 0 */
	es5505.set_region1("waverom2"); /* Bank 1 */
	es5505.set_channels(4);          /* channels */
	es5505.irq_cb().set(FUNC(esq5505_state::esq5505_otis_irq)); /* irq */
	es5505.read_port_cb().set(FUNC(esq5505_state::analog_r)); /* ADC */
	es5505.add_route(0, "pump", 1.0, 0);
	es5505.add_route(1, "pump", 1.0, 1);
	es5505.add_route(2, "pump", 1.0, 2);
	es5505.add_route(3, "pump", 1.0, 3);
	es5505.add_route(4, "pump", 1.0, 4);
	es5505.add_route(5, "pump", 1.0, 5);
	es5505.add_route(6, "pump", 1.0, 6);
	es5505.add_route(7, "pump", 1.0, 7);
}

void esq5505_state::eps(machine_config &config)
{
	vfx(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::eps_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &esq5505_state::eps_cpu_space_map);

	m_duart->set_clock(10_MHz_XTAL / 2);

	ESQPANEL1X22(config.replace(), m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(esq5505_state::analog_w));

	WD1772(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, m_floppy_connector);
	m_floppy_connector->option_add("35dd", FLOPPY_35_DD);
	m_floppy_connector->set_default_option("35dd");
	m_floppy_connector->set_formats(esq5505_state::floppy_formats);

	HD63450(config, m_dmac, 10_MHz_XTAL);   // MC68450 compatible
	m_dmac->set_cpu_tag(m_maincpu);
	m_dmac->set_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2));
	m_dmac->set_burst_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_nsec(50), attotime::from_nsec(50));
	m_dmac->irq_callback().set(FUNC(esq5505_state::dma_irq));
	m_dmac->dma_read<0>().set(m_fdc, FUNC(wd1772_device::data_r));  // ch 0 = fdc, ch 1 = 340001 (ADC?)
	m_dmac->dma_write<0>().set(m_fdc, FUNC(wd1772_device::data_w));
}

void esq5505_state::vfxsd(machine_config &config)
{
	vfx(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::vfxsd_map);

	WD1772(config, m_fdc, 8000000);
	FLOPPY_CONNECTOR(config, m_floppy_connector);
	m_floppy_connector->option_add("35dd", FLOPPY_35_DD);
	m_floppy_connector->set_default_option("35dd");
	m_floppy_connector->set_formats(esq5505_state::floppy_formats);
}

// 32-voice machines with the VFX-SD type config
void esq5505_state::vfx32(machine_config &config)
{
	M68000(config, m_maincpu, 30.47618_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::vfxsd_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &esq5505_state::cpu_space_map);

	ES5510(config, m_esp, 10_MHz_XTAL);
	m_esp->set_disable();

	ESQPANEL2X40_VFX(config, m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(esq5505_state::analog_w));

	MC68681(config, m_duart,  4000000);
	m_duart->irq_cb().set(FUNC(esq5505_state::duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(esq5505_state::duart_tx_a));
	m_duart->b_tx_cb().set(FUNC(esq5505_state::duart_tx_b));
	m_duart->outport_cb().set(FUNC(esq5505_state::duart_output));
	m_duart->set_clocks(500000, 500000, 1000000, 1000000);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w)); // route MIDI Tx send directly to 68681 channel A Rx

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "speaker", 2).front();

	ESQ_5505_5510_PUMP(config, m_pump, 30.47618_MHz_XTAL / (2 * 16 * 32));
	m_pump->set_esp(m_esp);
	m_pump->add_route(0, "speaker", 1.0, 0);
	m_pump->add_route(1, "speaker", 1.0, 1);

	auto &es5505(ES5505(config, "otis", 30.47618_MHz_XTAL / 2));
	es5505.sample_rate_changed().set(FUNC(esq5505_state::es5505_clock_changed));
	es5505.set_region0("waverom");  /* Bank 0 */
	es5505.set_region1("waverom2"); /* Bank 1 */
	es5505.set_channels(4);          /* channels */
	es5505.irq_cb().set(FUNC(esq5505_state::esq5505_otis_irq)); /* irq */
	es5505.read_port_cb().set(FUNC(esq5505_state::analog_r)); /* ADC */
	es5505.add_route(0, "pump", 1.0, 0);
	es5505.add_route(1, "pump", 1.0, 1);
	es5505.add_route(2, "pump", 1.0, 2);
	es5505.add_route(3, "pump", 1.0, 3);
	es5505.add_route(4, "pump", 1.0, 4);
	es5505.add_route(5, "pump", 1.0, 5);
	es5505.add_route(6, "pump", 1.0, 6);
	es5505.add_route(7, "pump", 1.0, 7);

	WD1772(config, m_fdc, 8000000);
	FLOPPY_CONNECTOR(config, m_floppy_connector, "35dd", FLOPPY_35_DD, true, floppy_formats);
}

void esq5505_state::sq1(machine_config &config)
{
	vfx(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::sq1_map);

	ESQPANEL2X16_SQ1(config.replace(), m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(esq5505_state::analog_w));
}

void esq5505_state::ks32(machine_config &config)
{
	vfx32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq5505_state::sq1_map);

	ESQPANEL2X16_SQ1(config.replace(), m_panel);
	m_panel->write_tx().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_panel->write_analog().set(FUNC(esq5505_state::analog_w));
}

static INPUT_PORTS_START( vfx )
#if KEYBOARD_HACK
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x80)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x81)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x82)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x83)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x84)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x85)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x86)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x87)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x88)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x89)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x8f)

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x90)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x91)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x92)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x93)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x94)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x95)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x96)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x97)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x98)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x99)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0x9f)

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 0)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 1)

	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 2)
#endif
INPUT_PORTS_END

static INPUT_PORTS_START( sq1 )
#if KEYBOARD_HACK
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 148) PORT_NAME("PITCH")  // 148=PITCH  (lo 1)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 149) PORT_NAME("CONTROL")  // 149=CONTROL  (hi 1)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w')  PORT_CHAR('W')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 132) PORT_NAME("ENV1")  // 132=ENV1        (lo 2)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 133) PORT_NAME("CLICK")  // 133=CLICK  (hi 2)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 182) PORT_NAME("LFO")  // 182=LFO      (lo 3)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 183) PORT_NAME("SONG")  // 183=SONG        (hi 3)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r')  PORT_CHAR('R')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 134) PORT_NAME("FILTER")  // 134=FILTER    (lo 4)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 135) PORT_NAME("SEQ")  // 135=SEQ      (hi 4)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t')  PORT_CHAR('T')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 142) PORT_NAME("ENV2")  // 142=ENV2        (lo 5)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 143) PORT_NAME("EVENT")  // 143=EVENT  (hi 5)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y')  PORT_CHAR('Y')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 150) PORT_NAME("AMP")  // 150=AMP      (lo 6)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 151) PORT_NAME("PARAM")  // 151=PARAM  (hi 6)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u')  PORT_CHAR('U')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 166) PORT_NAME("OUTPUT")  // 166=OUTPUT    (lo 7)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 167) PORT_NAME("MIX")  // 167=MIX      (hi 7)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i')  PORT_CHAR('I')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 158) PORT_NAME("P. EFFECT")  // 158=P.EFFECT   (lo 8)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 159) PORT_NAME("S. EFFECT")  // 159=S.EFFECT   (hi 8)
	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o')  PORT_CHAR('O')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 174) PORT_NAME("MIDI")  // 174=MIDI        (lo 9)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 175) PORT_NAME("SYSTEM")  // 175=SYSTEM    (hi 9)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p')  PORT_CHAR('P')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 164) PORT_NAME("WAVE")  // 164=WAVE        (lo 0)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')                  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 165) PORT_NAME("LOCATE")  // 165=LOCATE    (hi 0)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g')  PORT_CHAR('G')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 144) PORT_NAME("TRACK 1")  // 144=Track 1
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h')  PORT_CHAR('H')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 160) PORT_NAME("TRACK 2")  // 160=Track 2
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j')  PORT_CHAR('J')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 152) PORT_NAME("TRACK 3")  // 152=Track 3
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k')  PORT_CHAR('K')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 168) PORT_NAME("TRACK 4")  // 168=Track 4
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v')  PORT_CHAR('V')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 145) PORT_NAME("TRACK 5")  // 145=Track 5
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 161) PORT_NAME("TRACK 6")  // 161=Track 6
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n')  PORT_CHAR('N')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 136) PORT_NAME("TRACK 7")  // 136=Track 7
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m')  PORT_CHAR('M')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 176) PORT_NAME("TRACK 8")  // 176=Track 8
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)                                     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 172) PORT_NAME("ENTER")  // 172=ENTER
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)                                 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 173) PORT_NAME("COMPARE")  // 173=COMPARE
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                                      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 140) PORT_NAME("PROG DN")  // 140=ProgDn
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                                        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 141) PORT_NAME("PROG UP")  // 141=ProgUp
	PORT_START("KEY2")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                                     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 156) PORT_NAME("ROM/INT SELECT +")  // 156=ROM/INT Select  189=track +
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)                                      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 157) PORT_NAME("ROM/INT SELECT -")  // 157=ROM/INT Select  190=track -
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 180) PORT_NAME("SOUND SELECT")  // 180=SOUND Select
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 181) PORT_NAME("SOUND EDIT")  // 181=SOUND Edit
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s')  PORT_CHAR('S')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 184) PORT_NAME("SEQ EDIT")  // 184=SEQ Edit
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x')  PORT_CHAR('X')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esq5505_state::key_stroke), 153) PORT_NAME("SEQ SELECT")  // 153=SEQ Select
	PORT_START("KEY3")
#endif
INPUT_PORTS_END

#define ROM_LOAD16_BYTE_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1) | ROM_BIOS(bios))

ROM_START( vfx )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "vfx210b-low.bin",  0x000001, 0x010000, CRC(c51b19cd) SHA1(2a125b92ffa02ae9d7fb88118d525491d785e87e) )
	ROM_LOAD16_BYTE( "vfx210b-high.bin", 0x000000, 0x010000, CRC(59853be8) SHA1(8e07f69d53f80885d15f624e0b912aeaf3212ee4) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "u14.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
	ROM_LOAD16_BYTE( "u15.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
	ROM_LOAD( "u16.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

ROM_START( vfxsd )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_SYSTEM_BIOS( 0, "v200", "V200" )
	ROMX_LOAD( "vfxsd_200_lower.bin", 0x000001, 0x010000, CRC(7bd31aea) SHA1(812bf73c4861a5d963f128def14a4a98171c93ad), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "vfxsd_200_upper.bin", 0x000000, 0x010000, CRC(9a40efa2) SHA1(e38a2a4514519c1573361cb1526139bfcf94e45a), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v133", "V133" )
	ROMX_LOAD( "vfxsd_133_lower.bin", 0x000001, 0x010000, CRC(65407fcf) SHA1(83952a19f6f9ae7886ac828d8bd5ea7fee8d0fe3), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "vfxsd_133_upper.bin", 0x000000, 0x010000, CRC(150fcd18) SHA1(e42cf08604b52fab248d15d761de7d835a076940), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "u57.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
	ROM_LOAD16_BYTE( "u58.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)

	ROM_REGION(0x80000, "nibbles", 0)
	ROM_LOAD( "u60.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

ROM_START( sd1 )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "sd1_21_300b_lower.bin", 0x000001, 0x020000, CRC(a1358a0c) SHA1(64ac5358aa46da37ca4195002cf358554e00878a) )
	ROM_LOAD16_BYTE( "sd1_21_300b_upper.bin", 0x000000, 0x010000, CRC(465ba463) SHA1(899b0e83d0788c8d49c7b09ccf0b4a92b528c6e9) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // BS=0 region (12-bit)
	ROM_LOAD16_BYTE( "u34.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
	ROM_LOAD16_BYTE( "u35.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD16_WORD_SWAP( "u38.bin", 0x000000, 0x100000, CRC(a904190e) SHA1(e4fd4e1130906086fb4182dcb8b51269969e2836) )
	ROM_LOAD16_WORD_SWAP( "u37.bin", 0x100000, 0x100000, CRC(d706cef3) SHA1(24ba35248509e9ca45110e2402b8085006ea0cfc) )

	ROM_REGION(0x80000, "nibbles", 0)
	ROM_LOAD( "u36.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

// note: all known 4.xx BIOSes are for the 32-voice SD-1 and play out of tune on 21-voice h/w
ROM_START( sd132 )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_SYSTEM_BIOS(0, "410", "SD-1 v4.10")
	ROM_LOAD16_BYTE_BIOS(0, "sd1_410_lo.bin", 0x000001, 0x020000, CRC(faa613a6) SHA1(60066765cddfa9d3b5d09057d8f83fb120f4e65e) )
	ROM_LOAD16_BYTE_BIOS(0, "sd1_410_hi.bin", 0x000000, 0x010000, CRC(618c0aa8) SHA1(74acf458aa1d04a0a7a0cd5855c49e6855dbd301) )
	ROM_SYSTEM_BIOS(1, "402", "SD-1 v4.02")
	ROM_LOAD16_BYTE_BIOS(1, "sd1_32_402_lo.bin", 0x000001, 0x020000, CRC(5da2572b) SHA1(cb6ddd637ed13bfeb40a99df56000479e63fc8ec) )
	ROM_LOAD16_BYTE_BIOS(1, "sd1_32_402_hi.bin", 0x000000, 0x010000, CRC(fc45c210) SHA1(23b81ebd9176112e6eae0c7c75b39fcb1656c953) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // BS=0 region (12-bit)
	ROM_LOAD16_BYTE( "u34.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
	ROM_LOAD16_BYTE( "u35.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD16_WORD_SWAP( "u38.bin", 0x000000, 0x100000, CRC(a904190e) SHA1(e4fd4e1130906086fb4182dcb8b51269969e2836) )
	ROM_LOAD16_WORD_SWAP( "u37.bin", 0x100000, 0x100000, CRC(d706cef3) SHA1(24ba35248509e9ca45110e2402b8085006ea0cfc) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
	ROM_LOAD( "u36.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END


ROM_START( sq1 )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "sq1lo.bin",    0x000001, 0x010000, CRC(b004cf05) SHA1(567b0dae2e35b06e39da108f9c041fd9bc38fa35) )
	ROM_LOAD16_BYTE( "sq1up.bin",    0x000000, 0x010000, CRC(2e927873) SHA1(06a948cb71fa254b23f4b9236f29035d10778da1) )

	ROM_REGION(0x200000, "waverom", 0)
	ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
	ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

ROM_START( sqrack )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "sqr-102-lower.bin", 0x000001, 0x010000, CRC(186c85ad) SHA1(801c5cf82823ce31a88688fbee4c11ea5ffdbc10) )
	ROM_LOAD16_BYTE( "sqr-102-upper.bin", 0x000000, 0x010000, CRC(088c9d31) SHA1(30627f21d893888b6159c481bea08e3eedd21902) )

	ROM_REGION(0x200000, "waverom", 0)
	ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
	ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

ROM_START( sq2 )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "sq232_2.03_9193_lower.u27", 0x000001, 0x010000, CRC(e37fbc2c) SHA1(4a74f3540756745073c8768b384905db03da47c0) )
	ROM_LOAD16_BYTE( "sq232_2.03_cbcd_upper.u32", 0x000000, 0x020000, CRC(5a7dc228) SHA1(d25adecc0dbba93a094c49fae105dcc7aad317f1) )

	ROM_REGION(0x200000, "waverom", 0)
	ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
	ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD( "rom2.u39",     0x000000, 0x100000, CRC(8d1b5e91) SHA1(12991083a6c574133a1a799813fa4573a33d2297) )
	ROM_LOAD( "rom3.u38",     0x100000, 0x100000, CRC(cb9875ce) SHA1(82021bdc34953e9be97d45746a813d7882250ae0) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

ROM_START( eps )
	ROM_REGION16_BE(0x20000, "osrom", 0)
	ROM_LOAD16_BYTE( "eps-l.bin",    0x000001, 0x008000, CRC(382beac1) SHA1(110e31edb03fcf7bbde3e17423b21929e5b32db2) )
	ROM_LOAD16_BYTE( "eps-h.bin",    0x000000, 0x008000, CRC(d8747420) SHA1(460597751386eb5f08465699b61381c4acd78065) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // EPS-16 has no ROM sounds

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
ROM_END

ROM_START( eps16p )
	ROM_REGION16_BE(0x20000, "osrom", 0)
	ROM_LOAD16_BYTE( "eps16plus-100f-lower.u27", 0x000001, 0x010000, CRC(78568d3f) SHA1(ac737e093f422e109e8f06d44548629a12d6418c) )
	ROM_LOAD16_BYTE( "eps16plus-100f-upper.u28", 0x000000, 0x010000, CRC(1264465f) SHA1(71604da091bd90a32f0d93698d70b9e114ec1697) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // EPS-16 Plus has no ROM sounds

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
ROM_END

ROM_START( ks32 )
	ROM_REGION16_BE(0x40000, "osrom", 0)
	ROM_SYSTEM_BIOS(0, "301", "KS-32 v3.01")
	ROM_LOAD16_BYTE_BIOS(0, "ks32v301lower.bin", 0x000001, 0x010000, CRC(de37e49c) SHA1(fd5a25e23ff217a5926daae4f57dd966d392d26d) )
	ROM_LOAD16_BYTE_BIOS(0, "ks32v301upper.bin", 0x000000, 0x020000, CRC(d8249b32) SHA1(8712cf2c63a31c00e98fa42e518093cac51f5214) )

	ROM_SYSTEM_BIOS(1, "30", "KS-32 v3.0")
	ROM_LOAD16_BYTE_BIOS(1, "ks32v3pt00lower.bin", 0x000001, 0x010000, CRC(c347708e) SHA1(637e1a5c0a62f4d5726363bdb782448ca9637afc) )
	ROM_LOAD16_BYTE_BIOS(1, "ks32v3pt00upper.bin", 0x000000, 0x020000, CRC(8c56c88f) SHA1(4424f39f74f067f15030b8d4a90d9ace8ea14677) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
	ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD( "rom2.u39",     0x000000, 0x100000, CRC(8d1b5e91) SHA1(12991083a6c574133a1a799813fa4573a33d2297) )
	ROM_LOAD( "rom3.u38",     0x100000, 0x100000, CRC(cb9875ce) SHA1(82021bdc34953e9be97d45746a813d7882250ae0) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

void esq5505_state::init_common()
{
	m_system_type = GENERIC;
	m_duart_io = 0;

	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;
	if (floppy)
	{
		m_fdc->set_floppy(floppy);
		floppy->ss_w(0);
	}
}

void esq5505_state::init_eps()
{
	init_common();
	m_system_type = EPS;
}

void esq5505_state::init_sq1()
{
	init_common();
	m_system_type = SQ1;
#if KEYBOARD_HACK
	shift = 60;
#endif
}

void esq5505_state::init_denib()
{
	uint8_t *pNibbles = (uint8_t *)memregion("nibbles")->base();
	uint8_t *pBS0L = (uint8_t *)memregion("waverom")->base();
	uint8_t *pBS0H = pBS0L + 0x100000;

	init_common();

	// create the 12 bit samples by patching in the nibbles from the nibble ROM
	// low nibbles go with the lower ROM, high nibbles with the upper ROM
	for (int i = 0; i < 0x80000; i++)
	{
		*pBS0L = (*pNibbles & 0x0f) << 4;
		*pBS0H = (*pNibbles & 0xf0);
		pBS0L += 2;
		pBS0H += 2;
		pNibbles++;
	}
}

} // Anonymous namespace


CONS( 1988, eps,    0,   0, eps,   vfx, esq5505_state, init_eps,    "Ensoniq", "EPS",             MACHINE_NOT_WORKING )  // custom VFD: one alphanumeric 22-char row, one graphics-capable row (alpha row can also do bar graphs)
CONS( 1989, vfx,    0,   0, vfx,   vfx, esq5505_state, init_denib,  "Ensoniq", "VFX",             MACHINE_NOT_WORKING )  // 2x40 VFD
CONS( 1989, vfxsd,  0,   0, vfxsd, vfx, esq5505_state, init_denib,  "Ensoniq", "VFX-SD",          MACHINE_NOT_WORKING )  // 2x40 VFD
CONS( 1990, eps16p, eps, 0, eps,   vfx, esq5505_state, init_eps,    "Ensoniq", "EPS-16 Plus",     MACHINE_NOT_WORKING )  // custom VFD: one alphanumeric 22-char row, one graphics-capable row (alpha row can also do bar graphs)
CONS( 1990, sd1,    0,   0, vfxsd, vfx, esq5505_state, init_denib,  "Ensoniq", "SD-1 (21 voice)", MACHINE_NOT_WORKING )  // 2x40 VFD
CONS( 1990, sq1,    0,   0, sq1,   sq1, esq5505_state, init_sq1,    "Ensoniq", "SQ-1",            MACHINE_NOT_WORKING )  // 2x16 LCD
CONS( 1990, sqrack, sq1, 0, sq1,   sq1, esq5505_state, init_sq1,    "Ensoniq", "SQ-Rack",         MACHINE_NOT_WORKING )  // 2x16 LCD
CONS( 1991, sq2,    0,   0, ks32,  sq1, esq5505_state, init_sq1,    "Ensoniq", "SQ-2",            MACHINE_NOT_WORKING )  // 2x16 LCD
CONS( 1991, sd132,  sd1, 0, vfx32, vfx, esq5505_state, init_denib,  "Ensoniq", "SD-1 (32 voice)", MACHINE_NOT_WORKING )  // 2x40 VFD
CONS( 1992, ks32,   sq2, 0, ks32,  sq1, esq5505_state, init_sq1,    "Ensoniq", "KS-32",           MACHINE_NOT_WORKING)                       // 2x16 LCD
