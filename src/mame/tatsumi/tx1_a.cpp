// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy sound hardware

***************************************************************************/

#include "emu.h"

#include "tx1_a.h"
#include "cpu/z80/z80.h"
#include "tx1.h"
#include "video/resnet.h"
#include "speaker.h"


/*************************************
 *
 *  TX-1
 *
 *************************************/

/* RC oscillator: 1785Hz */
#define TX1_NOISE_CLOCK     (1/(1000.0e-12 * 560000.0))
#define TX1_PIT_CLOCK       (clock() / 16)
#define TX1_FRAC            30

#define TX1_SHUNT           (250.0)
#define TX1_R0              (180000.0 + TX1_SHUNT)
#define TX1_R1              (56000.0  + TX1_SHUNT)
#define TX1_R2              (22000.0  + TX1_SHUNT)
#define TX1_R               (100000.0 + TX1_SHUNT)
#define TX1_RI              (180000.0)

static const double tx1_engine_gains[16] =
{
	-( TX1_R )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT ) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI
};


DEFINE_DEVICE_TYPE(TX1_SOUND, tx1_sound_device, "tx1_sound", "TX-1 Custom Sound")
DEFINE_DEVICE_TYPE(TX1J_SOUND, tx1j_sound_device, "tx1j_sound", "TX-1 Custom Sound (Japan)")

tx1_sound_device::tx1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tx1_sound_device(mconfig, TX1_SOUND, tag, owner, clock)
{
}

tx1_sound_device::tx1_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_audiocpu(*this, "audio_cpu"),
	m_z80_ram(*this, "z80_ram"),
	m_ppi(*this, "ppi8255"),
	m_dsw(*this, "DSW"),
	m_steering(*this, "AN_STEERING"),
	m_accelerator(*this, "AN_ACCELERATOR"),
	m_brake(*this, "AN_BRAKE"),
	m_stream(nullptr),
	m_freq_to_step(0),
	m_step0(0),
	m_step1(0),
	m_step2(0),
	m_ay_outputa(0),
	m_ay_outputb(0),
	m_ppi_latch_a(0),
	m_ppi_latch_b(0),
	m_ts(0),
	m_pit0(0),
	m_pit1(0),
	m_pit2(0),
	m_noise_lfsra(0),
	m_noise_lfsrb(1),
	m_noise_lfsrc(0),
	m_noise_lfsrd(0),
	m_noise_counter(0),
	m_ym1_outputa(0),
	m_ym2_outputa(0),
	m_ym2_outputb(0)
{
}

tx1j_sound_device::tx1j_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tx1_sound_device(mconfig, TX1J_SOUND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tx1_sound_device::device_start()
{
	static const int r0[4] = { static_cast<int>(390e3), static_cast<int>(180e3), static_cast<int>(180e3), static_cast<int>(180e3) };
	static const int r1[3] = { static_cast<int>(180e3), static_cast<int>(390e3), static_cast<int>(56e3) };
	static const int r2[3] = { static_cast<int>(390e3), static_cast<int>(390e3), static_cast<int>(180e3) };

	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, machine().sample_rate());
	m_freq_to_step = (double)(1 << TX1_FRAC) / (double)machine().sample_rate();

	/* Compute the engine resistor weights */
	compute_resistor_weights(0, 10000, -1.0,
			4, &r0[0], m_weights0, 0, 0,
			3, &r1[0], m_weights1, 0, 0,
			3, &r2[0], m_weights2, 0, 0);

	std::fill(std::begin(m_eng0), std::end(m_eng0), 0.0);
	std::fill(std::begin(m_eng1), std::end(m_eng1), 0.0);
	std::fill(std::begin(m_eng2), std::end(m_eng2), 0.0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tx1_sound_device::device_reset()
{
	m_step0 = m_step1 = m_step2 = 0;
}

/* Main CPU and Z80 synchronisation */
void tx1_sound_device::z80_busreq_w(uint16_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_HALT, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}

/* Z80 can trigger its own interrupts */
void tx1_sound_device::z80_intreq_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

uint16_t tx1_sound_device::z80_shared_r(offs_t offset)
{
	return m_audiocpu->space(AS_PROGRAM).read_byte(offset);
}

void tx1_sound_device::z80_shared_w(offs_t offset, uint16_t data)
{
	m_audiocpu->space(AS_PROGRAM).write_byte(offset, data & 0xff);
}

/*
    (TODO) TS: Connected in place of dipswitch A bit 0
    Accessed on startup as some sort of acknowledgement
*/
void tx1_sound_device::ts_w(offs_t offset, uint8_t data)
{
//  TS = 1;
	m_z80_ram[offset] = data;
}

uint8_t tx1_sound_device::ts_r(offs_t offset)
{
//  TS = 1;
	return m_z80_ram[offset];
}

static uint8_t bit_reverse8(uint8_t val)
{
	val = ((val & 0xF0) >>  4) | ((val & 0x0F) <<  4);
	val = ((val & 0xCC) >>  2) | ((val & 0x33) <<  2);
	val = ((val & 0xAA) >>  1) | ((val & 0x55) <<  1);

	return val;
}

uint16_t tx1_sound_device::dipswitches_r()
{
	return (m_dsw->read() & 0xfffe) | m_ts;
}

// Tazmi TZ2103 custom 4-channel A/D converter @ 7.5 MHz
uint8_t buggyboy_sound_device::bb_analog_r(offs_t offset)
{
	if (offset == 0)
		return bit_reverse8(((m_accelerator->read() & 0xf) << 4) | (m_steering->read() & 0xf));
	else
		return bit_reverse8((m_brake->read() & 0xf) << 4);
}

uint8_t buggyboyjr_sound_device::bbjr_analog_r(offs_t offset)
{
	if (offset == 0)
		return ((m_accelerator->read() & 0xf) << 4) | (m_steering->read() & 0xf);
	else
		return (m_brake->read() & 0xf) << 4;
}

void tx1_sound_device::tx1_coin_cnt_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x80);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);
}

void buggyboy_sound_device::bb_coin_cnt_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
//  machine().bookkeeping().coin_counter_w(2, data & 0x04);
}

void tx1_sound_device::tx1_ppi_latch_w(uint8_t data)
{
	m_ppi_latch_a = ((m_brake->read() & 0xf) << 4) | (m_accelerator->read() & 0xf);
	m_ppi_latch_b = m_steering->read() & 0xf;
}

uint8_t tx1_sound_device::tx1_ppi_porta_r()
{
	return m_ppi_latch_a;
}

uint8_t tx1_sound_device::tx1_ppi_portb_r()
{
	// upper nibble goes to unpopulated DS.3
	return m_ppi_latch_b & 0xf;
}

void tx1_sound_device::pit8253_w(offs_t offset, uint8_t data)
{
	m_stream->update();

	// TODO: use pit8253_device
	if (offset < 3)
	{
		if (m_pit8253.idx[offset] == 0)
		{
			m_pit8253.counts[offset].as8bit.LSB = data;
			m_pit8253.idx[offset] = 1;
		}
		else
		{
			m_pit8253.counts[offset].as8bit.MSB = data;
			m_pit8253.idx[offset] = 0;
		}
	}
	else
	{
		int mode = (data >> 1) & 7;

		if (mode == 3)
		{
			int cntsel = (data >> 6) & 3;
			m_pit8253.idx[cntsel] = 0;
			m_pit8253.counts[cntsel].val = 0;
		}
		else
			osd_printf_debug("PIT8253: Unsupported mode %d.\n", mode);
	}
}

uint8_t tx1_sound_device::pit8253_r(offs_t offset)
{
	osd_printf_debug("PIT R: %x", offset);
	return 0;
}

/* Periodic Z80 interrupt */
INTERRUPT_GEN_MEMBER(tx1_sound_device::z80_irq)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

/***************************************************************************

    AY-8910 port mappings:

    Port A                      Port B
    =======                     ======

    0: Engine 0 gain #0         0: Engine 0 gain #8
    1: Engine 0 gain #1         1: Engine 0 gain #9
    2: Engine 0 gain #2         2: Engine 0 gain #10
    3: Engine 0 gain #3         3: Engine 0 gain #11
    4: Engine 0 gain #4         4: /Enable AY on speaker CR
    5: Engine 0 gain #5         5: /Enable Engines 1/2 on speakers LR/RR/CF
    6: Engine 0 gain #6         6: /Skid 0 enable
    7: Engine 0 gain #7         7: /Skid 1 enable

***************************************************************************/

void tx1_sound_device::ay8910_a_w(uint8_t data)
{
	m_stream->update();

	/* All outputs inverted */
	m_ay_outputa = ~data;
}

void tx1_sound_device::ay8910_b_w(uint8_t data)
{
	m_stream->update();

	/* Only B3-0 are inverted */
	m_ay_outputb = data ^ 0xf;

	/* It'll do until we get quadrophonic speaker support! */
	double gain = BIT(m_ay_outputb, 4) ? 1.5 : 2.0;
	device_sound_interface *sound;
	interface(sound);
	sound->set_output_gain(0, gain);
	sound->set_output_gain(1, gain);
	sound->set_output_gain(2, gain);
}

/***************************************************************************

    Engine sounds are produced by three of these 4013B chains:

                +------------------
                |  +---------+
        +-----+ |  | +-----+ |
      +-|D 1 Q|-+  +-|D 2 Q|-|-----
      | |C  /Q|-o----|C  /Q|-+
      | +-----+ |    +-----+
      +---------+

     +----------+
     |  +-----+ |    +-----+
     |  |C 3 Q|-+    |C 4 Q|-------
    !&--|D  /Q|------|D  /Q|-+
     |  +-----+      +-----+ |
     +-----------------------+

     Common clocks omitted for clarity (all driven from an 8253 output pin).

         Player: ES0, ES1, ES2
     Opponent 2: ES2

 ***************************************************************************/

static inline void update_engine(int eng[4])
{
	int p0 = eng[0];
	int p1 = eng[1];
	int p2 = eng[2];
	int p3 = eng[3];

	eng[0] = !p0;
	if (p0 && !eng[0]) eng[1] = !p1;
	eng[2] = !(p2 && !p3);
	eng[3] = !p2;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tx1_sound_device::sound_stream_update(sound_stream &stream)
{
	uint32_t step_0, step_1, step_2;
	double /*gain_0, gain_1,*/ gain_2, gain_3;

	/* 8253 outputs for the player/opponent engine sounds. */
	step_0 = m_pit8253.counts[0].val ? (TX1_PIT_CLOCK / m_pit8253.counts[0].val * m_freq_to_step) : 0;
	step_1 = m_pit8253.counts[1].val ? (TX1_PIT_CLOCK / m_pit8253.counts[1].val * m_freq_to_step) : 0;
	step_2 = m_pit8253.counts[2].val ? (TX1_PIT_CLOCK / m_pit8253.counts[2].val * m_freq_to_step) : 0;

	//gain_0 = tx1_engine_gains[m_ay_outputa & 0xf];
	//gain_1 = tx1_engine_gains[m_ay_outputa >> 4];
	gain_2 = tx1_engine_gains[m_ay_outputb & 0xf];
	gain_3 = BIT(m_ay_outputb, 5) ? 1.0f : 1.5f;

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		if (m_step0 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng0);
			m_pit0 = combine_weights(m_weights0, m_eng0[0], m_eng0[1], m_eng0[2], m_eng0[3]);
			m_step0 &= ((1 << TX1_FRAC) - 1);
		}

		if (m_step1 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng1);
			m_pit1 = combine_weights(m_weights1, m_eng1[0], m_eng1[1], m_eng1[3]);
			m_step1 &= ((1 << TX1_FRAC) - 1);
		}

		if (m_step2 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng2);
			m_pit2 = combine_weights(m_weights2, m_eng2[0], m_eng2[1], m_eng2[3]);
			m_step2 &= ((1 << TX1_FRAC) - 1);
		}

		stream.put_int(0, sampindex, (m_pit0 + m_pit1)*gain_3 + 2*m_pit2*gain_2, 32768);
		stream.put_int(1, sampindex, (m_pit0 + m_pit1)*gain_3 + 2*m_pit2*gain_2, 32768);

		m_step0 += step_0;
		m_step1 += step_1;
		m_step2 += step_2;
	}
}

void tx1_sound_device::tx1_sound_prg(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x37ff).ram().mirror(0x800).share("z80_ram");
	map(0x4000, 0x4000).w(FUNC(tx1_sound_device::z80_intreq_w));
	map(0x5000, 0x5003).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6000, 0x6003).rw(FUNC(tx1_sound_device::pit8253_r), FUNC(tx1_sound_device::pit8253_w));
	map(0x7000, 0x7fff).w(FUNC(tx1_sound_device::tx1_ppi_latch_w));
	map(0xb000, 0xbfff).rw(FUNC(tx1_sound_device::ts_r), FUNC(tx1_sound_device::ts_w));
}

void tx1_sound_device::tx1_sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x41).w("aysnd", FUNC(ay8910_device::data_address_w));
}

INPUT_PORTS_START( tx1_inputs )
	PORT_START("DSW")
	/* Dipswitch DS.2 is 6 switches but "maps" to switches 2 to 8 (at 6P according to the manual)  */
	PORT_DIPNAME( 0x000c, 0x0000, "Game Cost" )         PORT_DIPLOCATION("DS.2:1,2")
	PORT_DIPSETTING(      0x0000, "1 Coin Unit for 1 Credit" )
	PORT_DIPSETTING(      0x0004, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x0008, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x000c, "4 Coin Units for 1 Credit" )

	PORT_DIPNAME( 0x0010, 0x0000, "Left Coin Mechanism" )       PORT_DIPLOCATION("DS.2:3")
	PORT_DIPSETTING(      0x0000, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING(      0x0010, "1 Coin for 2 Coin Units" )

	PORT_DIPNAME( 0x0060, 0x0000, "Right Coin Mechanism" )      PORT_DIPLOCATION("DS.2:4,5")
	PORT_DIPSETTING(      0x0000, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING(      0x0020, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING(      0x0040, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING(      0x0060, "1 Coin for 6 Coin Units" )

	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DS.2:6") /* Manual states switches 6 to 8 unused (physically it's only 6 switches) */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/* Dipswitch DS.1 is 8 switches (at 8P according to the manual) */
	PORT_DIPNAME( 0x0700, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DS.1:1,2,3")
	PORT_DIPSETTING(      0x0000, "A (Easiest)" )
	PORT_DIPSETTING(      0x0100, "B" )
	PORT_DIPSETTING(      0x0200, "C" )
	PORT_DIPSETTING(      0x0300, "D" )
	PORT_DIPSETTING(      0x0400, "E" )
	PORT_DIPSETTING(      0x0500, "F" )
	PORT_DIPSETTING(      0x0600, "G" )
	PORT_DIPSETTING(      0x0700, "H (Hardest)" )

	PORT_DIPNAME( 0x1800, 0x1000, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("DS.1:4,5")
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0800, "B" )
	PORT_DIPSETTING(      0x1000, "C" )
	PORT_DIPSETTING(      0x1800, "D (Shortest)" )

	PORT_DIPNAME( 0xe000, 0xe000, "Bonus Adder" )       PORT_DIPLOCATION("DS.1:6,7,8")
	PORT_DIPSETTING(      0x0000, "No Bonus" )
	PORT_DIPSETTING(      0x2000, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x4000, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x6000, "4 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x8000, "5 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0xa000, "4 Coin Units for 2 Credit" )
	PORT_DIPSETTING(      0xc000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0xe000, "No Bonus" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(6)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("PPI_PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear Change") PORT_TOGGLE
INPUT_PORTS_END

ioport_constructor tx1_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tx1_inputs);
}

INPUT_PORTS_START( tx1j_inputs )
	PORT_INCLUDE(tx1_inputs)

	PORT_MODIFY("DSW")
	/* Dipswitch DS.2 is 6 switches but "maps" to switches 2 to 8 (at 6P according to the manual)  */
	PORT_DIPNAME( 0x001c, 0x0000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("DS.2:1,2,3") /* As silkscreened on the PCB */
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x00e0, 0x0000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("DS.2:4,5,6") /* As silkscreened on the PCB */
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_6C ) )

	/* Dipswitch DS.1 is 8 switches (at 8P according to the manual) */
	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DS.1:6,7,8")
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x6000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0xa000, "5" )
	PORT_DIPSETTING(      0xc000, "6" )
	PORT_DIPSETTING(      0xe000, "7" )
INPUT_PORTS_END

ioport_constructor tx1j_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tx1j_inputs);
}

void tx1_sound_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_audiocpu, TX1_PIXEL_CLOCK / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tx1_sound_device::tx1_sound_prg);
	m_audiocpu->set_addrmap(AS_IO, &tx1_sound_device::tx1_sound_io);
	m_audiocpu->set_periodic_int(DEVICE_SELF, FUNC(tx1_sound_device::z80_irq), attotime::from_hz(TX1_PIXEL_CLOCK / 4 / 2048 / 2));

	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(tx1_sound_device::tx1_ppi_porta_r));
	m_ppi->in_pb_callback().set(FUNC(tx1_sound_device::tx1_ppi_portb_r));
	m_ppi->in_pc_callback().set_ioport("PPI_PORTC");
	m_ppi->out_pc_callback().set(FUNC(tx1_sound_device::tx1_coin_cnt_w));
	m_ppi->tri_pc_callback().set_constant(0);

	SPEAKER(config, "frontleft", -0.2, 0.0, 1.0);
	SPEAKER(config, "frontright", 0.2, 0.0, 1.0);
//  SPEAKER(config, "rearleft", -0.2, 0.0, -0.5); /* Atari TX-1 TM262 manual shows 4 speakers (TX-1 Audio PCB Assembly A042016-01 A) */
//  SPEAKER(config, "rearright", 0.2, 0.0, -0.5);

	ay8910_device &aysnd(AY8910(config, "aysnd", TX1_PIXEL_CLOCK / 8));
	aysnd.port_a_write_callback().set(FUNC(tx1_sound_device::ay8910_a_w));
	aysnd.port_b_write_callback().set(FUNC(tx1_sound_device::ay8910_b_w));
	aysnd.add_route(ALL_OUTPUTS, "frontleft", 0.1);
	aysnd.add_route(ALL_OUTPUTS, "frontright", 0.1);

	this->add_route(0, "frontleft", 0.2);
	this->add_route(1, "frontright", 0.2);
}

/*************************************
 *
 *  Buggy Boy
 *
 *************************************/

#define BUGGYBOY_PIT_CLOCK      (clock() / 8)
#define BUGGYBOY_NOISE_CLOCK    (BUGGYBOY_PIT_CLOCK / 4)

#define BUGGYBOY_R1     47000.0
#define BUGGYBOY_R2     22000.0
#define BUGGYBOY_R3     10000.0
#define BUGGYBOY_R4     5600.0
#define BUGGYBOY_SHUNT  250.0

#define BUGGYBOY_R1S    (1.0/(1.0/BUGGYBOY_R1 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R2S    (1.0/(1.0/BUGGYBOY_R2 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R3S    (1.0/(1.0/BUGGYBOY_R3 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R4S    (1.0/(1.0/BUGGYBOY_R4 + 1.0/BUGGYBOY_SHUNT))

static const double bb_engine_gains[16] =
{
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
};

DEFINE_DEVICE_TYPE(BUGGYBOY_SOUND, buggyboy_sound_device, "buggyboy_sound", "Buggy Boy Custom Sound")
DEFINE_DEVICE_TYPE(BUGGYBOYJR_SOUND, buggyboyjr_sound_device, "buggyboyjr_sound", "Buggy Boy Jr. Custom Sound")

buggyboy_sound_device::buggyboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	buggyboy_sound_device(mconfig, BUGGYBOY_SOUND, tag, owner, clock)
{
}

buggyboy_sound_device::buggyboy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	tx1_sound_device(mconfig, type, tag, owner, clock),
	m_ym(*this, "ym%u", 1U)
{
}

buggyboyjr_sound_device::buggyboyjr_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	buggyboy_sound_device(mconfig, BUGGYBOYJR_SOUND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buggyboy_sound_device::device_start()
{
	static const int resistors[4] = { 330000, 220000, 330000, 220000 };
	double aweights[4];
	static const int tmp[16] =
	{
		0x0, 0x1, 0xe, 0xf, 0x8, 0x9, 0x6, 0x7, 0xc, 0xd, 0xe, 0xf, 0x4, 0x5, 0x6, 0x7
	};

	compute_resistor_weights(0, 16384,  -1.0,
							4,  &resistors[0], aweights, 0, 0,
							0, nullptr, nullptr, 0, 0,
							0, nullptr, nullptr, 0, 0 );

	for (int i = 0; i < 16; i++)
		m_eng_voltages[i] = combine_weights(aweights, BIT(tmp[i], 0), BIT(tmp[i], 1), BIT(tmp[i], 2), BIT(tmp[i], 3));

	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, machine().sample_rate());
	m_freq_to_step = (double)(1 << 24) / (double)machine().sample_rate();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buggyboy_sound_device::device_reset()
{
	m_step0 = m_step1 = 0;

	/* Reset noise LFSR */
	m_noise_lfsra = 0;
	m_noise_lfsrb = 1;
	m_noise_lfsrc = 0;
	m_noise_lfsrd = 0;
}

/***************************************************************************

    YM-2149, IC24 port mappings:

    Port A                      Port B
    =======                     ======

    0: Engine 1 gain (FR) #0    0: *Coin Counter 1
    1: Engine 1 gain (FR) #1    1: *Coin Counter 2
    2: Engine 1 gain (FR) #2    2: *Coin Counter 3 (Unused)
    3: Engine 1 gain (FR) #3    3: *Engine 0 gain
    4: Engine 1 gain (FL) #0    4: Skid 0 enable
    5: Engine 1 gain (FL) #1    5: Skid 1 enable
    6: Engine 1 gain (FL) #2    6: Enable YM IC24 output on RR
    7: Engine 1 gain (FL) #3    7: Enable YM IC19 output on RL

    (* Buggy Boy Junior only)

    The engine sounds are generated by an 8253. There are two channels.

    #0 is the player's buggy
    #1 is the opponents' buggies

              +------------> GAIN[1] +--> FL
              |                      +--> FR
    8255 #0 --+--> BL
              +--> BR

    8255 #1 --+--> GAIN[2] ---> FL
              +--> GAIN[3] ---> FR


    [1] is used to amplify sound during tunnel.
    [2] and [3] are stereo fades

***************************************************************************/

void buggyboy_sound_device::ym1_a_w(uint8_t data)
{
	m_stream->update();
	m_ym1_outputa = data ^ 0xff;
}

void buggyboy_sound_device::ym2_a_w(uint8_t data)
{
	m_stream->update();
	m_ym2_outputa = data ^ 0xff;
}

void buggyboy_sound_device::ym2_b_w(uint8_t data)
{
	m_stream->update();

	m_ym2_outputb = data ^ 0xff;

	/*
	    Until we support > 2 speakers, double the gain of the front speakers

	    TODO: We do support more than 2 speakers but the output is downmixed to stereo.
	*/

	/* Rear left speaker */
	double gain = data & 0x80 ? 1.0 : 2.0;
	m_ym[0]->set_output_gain(0, gain);
	m_ym[0]->set_output_gain(1, gain);
	m_ym[0]->set_output_gain(2, gain);

	/* Rear right speaker */
	gain = data & 0x40 ? 1.0 : 2.0;
	m_ym[1]->set_output_gain(0, gain);
	m_ym[1]->set_output_gain(1, gain);
	m_ym[1]->set_output_gain(2, gain);
}

void buggyboyjr_sound_device::ym2_b_w(uint8_t data)
{
	buggyboy_sound_device::ym2_b_w(data);

	machine().bookkeeping().coin_counter_w(0, ~data & 0x01);
	machine().bookkeeping().coin_counter_w(1, ~data & 0x02);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void buggyboy_sound_device::sound_stream_update(sound_stream &stream)
{
	/* This is admittedly a bit of a hack job... */

	uint32_t step_0, step_1;
	int n1_en, n2_en;
	double gain0, gain1_l, gain1_r;

	/* 8253 outputs for the player/opponent buggy engine sounds. */
	step_0 = m_pit8253.counts[0].val ? (BUGGYBOY_PIT_CLOCK / m_pit8253.counts[0].val * m_freq_to_step) : 0;
	step_1 = m_pit8253.counts[1].val ? (BUGGYBOY_PIT_CLOCK / m_pit8253.counts[1].val * m_freq_to_step) : 0;

	if (!strcmp(machine().system().name, "buggyboyjr"))
		gain0 = BIT(m_ym2_outputb, 3) ? 1.0 : 2.0;
	else
		gain0 = BIT(m_ym1_outputa, 3) ? 1.0 : 2.0;

	n1_en = BIT(m_ym2_outputb, 4);
	n2_en = BIT(m_ym2_outputb, 5);

	gain1_l = bb_engine_gains[m_ym2_outputa >> 4] * 5;
	gain1_r = bb_engine_gains[m_ym2_outputa & 0xf] * 5;

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		s32 pit0, pit1, n1, n2;
		pit0 = m_eng_voltages[(m_step0 >> 24) & 0xf];
		pit1 = m_eng_voltages[(m_step1 >> 24) & 0xf];

		/* Calculate the tyre screech noise source */
		for (int i = 0; i < BUGGYBOY_NOISE_CLOCK / machine().sample_rate(); ++i)
		{
			/* CD4006 is a 4-4-1-4-4-1 shift register */
			int p13 = BIT(m_noise_lfsra, 3);
			int p12 = BIT(m_noise_lfsrb, 4);
			int p10 = BIT(m_noise_lfsrc, 3);
			int p8 = BIT(m_noise_lfsrd, 3);

			/* Update the register */
			m_noise_lfsra = p12 | ((m_noise_lfsra << 1) & 0xf);
			m_noise_lfsrb = (p8 ^ p12) | ((m_noise_lfsrb << 1) & 0x1f);
			m_noise_lfsrc = p13 | ((m_noise_lfsrc << 1) & 0xf);
			m_noise_lfsrd = p10 | ((m_noise_lfsrd << 1) & 0x1f);

			/* 4040 12-bit counter is clocked on the falling edge of Q13 */
			if (!BIT(m_noise_lfsrc, 3) && p10)
				m_noise_counter = (m_noise_counter + 1) & 0x0fff;
		}

		if (n1_en)
		{
			n1 = !BIT(m_noise_counter, 7-1) * 16000;
			if (BIT(m_noise_counter, 11-1)) n1 /=2;
		}
		else
			n1 = 8192;

		if (n2_en)
		{
			n2 = !BIT(m_noise_counter, 6-1) * 16000;
			if (BIT(m_noise_counter, 11-1)) n2 /=2;
		}
		else
			n2 = 8192;

		stream.put_int(0, sampindex, n1 + n2 + (pit0 * gain0) + (pit1 * gain1_l), 32768);
		stream.put_int(1, sampindex, n1 + n2 + (pit0 * gain0) + (pit1 * gain1_r), 32768);

		m_step0 += step_0;
		m_step1 += step_1;
	}
}

/* Buggy Boy Sound PCB TC033A */
void buggyboy_sound_device::buggyboy_sound_prg(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("z80_ram");
	map(0x6000, 0x6001).r(FUNC(buggyboy_sound_device::bb_analog_r));
	map(0x6800, 0x6803).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7000, 0x7003).rw(FUNC(buggyboy_sound_device::pit8253_r), FUNC(buggyboy_sound_device::pit8253_w));
	map(0x7800, 0x7800).w(FUNC(tx1_sound_device::z80_intreq_w));
	map(0xc000, 0xc7ff).rw(FUNC(tx1_sound_device::ts_r), FUNC(tx1_sound_device::ts_w));
}

/* Buggy Boy Jr Sound PCB TC043 */
void buggyboyjr_sound_device::buggybjr_sound_prg(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("z80_ram");
	map(0x5000, 0x5003).rw(FUNC(buggyboy_sound_device::pit8253_r), FUNC(buggyboy_sound_device::pit8253_w));
	map(0x6000, 0x6001).r(FUNC(buggyboyjr_sound_device::bbjr_analog_r));
	map(0x7000, 0x7000).w(FUNC(tx1_sound_device::z80_intreq_w));
	map(0xc000, 0xc7ff).rw(FUNC(tx1_sound_device::ts_r), FUNC(tx1_sound_device::ts_w));
}

/* Common */
void buggyboy_sound_device::buggyboy_sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).r(m_ym[0], FUNC(ay8910_device::data_r));
	map(0x40, 0x41).w(m_ym[0], FUNC(ay8910_device::data_address_w));
	map(0x80, 0x80).r(m_ym[1], FUNC(ay8910_device::data_r));
	map(0x80, 0x81).w(m_ym[1], FUNC(ay8910_device::data_address_w));
}

INPUT_PORTS_START( buggyboy_inputs )
	PORT_START("DSW")
	/* Dipswitch 0 is unconnected */
	PORT_DIPNAME( 0x0003, 0x0003, "Do not change DSW2 1&2" )    PORT_DIPLOCATION("SW2:1,2") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:3") /* Language of game instructions */
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change DSW2 4" )  PORT_DIPLOCATION("SW2:4") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0010, "Time Rank" )     PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D (Shortest)" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "A (Easy)")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D (Difficult)" )

	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )

	PORT_DIPNAME( 0x0700, 0x0700, "Do not change DSW1 1-3" )    PORT_DIPLOCATION("SW1:3,2,1") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_START("PPI_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear Change") PORT_TOGGLE
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("PPI_PORTC")
	PORT_DIPNAME( 0xff, 0x80, "Sound PCB Jumper" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(    0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(    0x80, "Buggy Boy/Tatsumi" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END

ioport_constructor buggyboy_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(buggyboy_inputs);
}

INPUT_PORTS_START( buggyboyjr_inputs )
	PORT_START("DSW")
	/* Dipswitch 0 is unconnected */
	PORT_DIPNAME( 0x0003, 0x0003, "Do not change DSW2 1&2" )    PORT_DIPLOCATION("SW2:1,2") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:3") /* Language of game instructions */
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change DSW2 4" )  PORT_DIPLOCATION("SW2:4") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0010, "Time Rank" )     PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D (Shortest)" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "A (Easy)")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D (Difficult)" )

	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0700, 0x0700, "Do not change DSW1 1-3" )    PORT_DIPLOCATION("SW1:3,2,1") /* Listed in manual as "Do Not Change" */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_START("YM2149_IC19_A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear Change") PORT_TOGGLE
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	/* Wire jumper setting on sound PCB */
	PORT_START("YM2149_IC19_B")
	PORT_DIPNAME( 0xff, 0x80, "Sound PCB Jumper" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(    0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(    0x80, "Buggy Boy/Tatsumi" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END

ioport_constructor buggyboyjr_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(buggyboyjr_inputs);
}

void buggyboy_sound_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_audiocpu, BUGGYBOY_ZCLK / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &buggyboy_sound_device::buggyboy_sound_prg);
	m_audiocpu->set_addrmap(AS_IO, &buggyboy_sound_device::buggyboy_sound_io);
	m_audiocpu->set_periodic_int(DEVICE_SELF, FUNC(buggyboy_sound_device::z80_irq), attotime::from_hz(BUGGYBOY_ZCLK / 2 / 4 / 2048));

	I8255A(config, m_ppi);
	/* Buggy Boy uses an 8255 PPI instead of YM2149 ports for inputs! */
	m_ppi->in_pa_callback().set_ioport("PPI_PORTA");
	m_ppi->out_pb_callback().set(FUNC(buggyboy_sound_device::bb_coin_cnt_w));
	m_ppi->tri_pb_callback().set_constant(0);
	m_ppi->in_pc_callback().set_ioport("PPI_PORTC");

	SPEAKER(config, "frontleft", -0.2, 0.0, 1.0);
	SPEAKER(config, "frontright", 0.2, 0.0, 1.0);
//  SPEAKER(config, "rearleft", -0.2, 0.0, -0.5); /* Atari TX-1 TM262 manual shows 4 speakers (TX-1 Audio PCB Assembly A042016-01 A) */
//  SPEAKER(config, "rearright", 0.2, 0.0, -0.5);

	YM2149(config, m_ym[0], BUGGYBOY_ZCLK / 4);
	m_ym[0]->port_a_write_callback().set(FUNC(buggyboy_sound_device::ym1_a_w));
	m_ym[0]->add_route(ALL_OUTPUTS, "frontleft", 0.15);

	YM2149(config, m_ym[1], BUGGYBOY_ZCLK / 4);
	m_ym[1]->port_a_write_callback().set(FUNC(buggyboy_sound_device::ym2_a_w));
	m_ym[1]->port_b_write_callback().set(FUNC(buggyboy_sound_device::ym2_b_w));
	m_ym[1]->add_route(ALL_OUTPUTS, "frontright", 0.15);

	this->add_route(0, "frontleft", 0.2);
	this->add_route(1, "frontright", 0.2);
}

void buggyboyjr_sound_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_audiocpu, BUGGYBOY_ZCLK / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &buggyboyjr_sound_device::buggybjr_sound_prg);
	m_audiocpu->set_addrmap(AS_IO, &buggyboyjr_sound_device::buggyboy_sound_io);
	m_audiocpu->set_periodic_int(DEVICE_SELF, FUNC(buggyboyjr_sound_device::z80_irq), attotime::from_hz(BUGGYBOY_ZCLK / 2 / 4 / 2048));

	SPEAKER(config, "frontleft", -0.2, 0.0, 1.0);
	SPEAKER(config, "frontright", 0.2, 0.0, 1.0);
//  SPEAKER(config, "rearleft", -0.2, 0.0, -0.5);
//  SPEAKER(config, "rearright", 0.2, 0.0, -0.5);

	YM2149(config, m_ym[0], BUGGYBOY_ZCLK / 4); /* YM2149 IC19 */
	m_ym[0]->port_a_read_callback().set_ioport("YM2149_IC19_A");
	m_ym[0]->port_b_read_callback().set_ioport("YM2149_IC19_B");
	m_ym[0]->add_route(ALL_OUTPUTS, "frontleft", 0.15);

	YM2149(config, m_ym[1], BUGGYBOY_ZCLK / 4); /* YM2149 IC24 */
	m_ym[1]->port_a_write_callback().set(FUNC(buggyboyjr_sound_device::ym2_a_w));
	m_ym[1]->port_b_write_callback().set(FUNC(buggyboyjr_sound_device::ym2_b_w));
	m_ym[1]->add_route(ALL_OUTPUTS, "frontright", 0.15);

	this->add_route(0, "frontleft", 0.2);
	this->add_route(1, "frontright", 0.2);
}
