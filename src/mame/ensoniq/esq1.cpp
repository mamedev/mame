// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

    drivers/esq1.c

    Ensoniq ESQ-1 Digital Wave Synthesizer
    Ensoniq ESQ-M (rack-mount ESQ-1)
    Ensoniq SQ-80 Cross Wave Synthesizer
    Driver by R. Belmont and O. Galibert

    Map for ESQ-1 and ESQ-m:
    0000-1fff: OS RAM
    2000-3fff: Cartridge
    4000-5fff: SEQRAM
    6000-63ff: ES5503 DOC
    6400-67ff: MC2681 DUART
    6800-6fff: AD7524 (CV_MUX)
    7000-7fff: OS ROM low (banked)
    8000-ffff: OS ROM high (fixed)

    Map for SQ-80:
    0000-1fff: OS RAM
    2000-3fff: Cartridge
    4000-5fff: DOSRAM or SEQRAM (banked)
    6000-63ff: ES5503 DOC
    6400-67ff: MC2681 DUART
    6800-6bff: AD7524 (CV_MUX)
    6c00-6dff: Mapper (bit 0 only - determines DOSRAM or SEQRAM at 4000)
    6e00-6fff: WD1772 FDC (not present on ESQ1)
    7000-7fff: OS ROM low (banked)
    8000-ffff: OS ROM high (fixed)

    CV_MUX area:
    write to        output goes to
    $68f8   $00     D/A converter
    $68f0   -$08    Filter Frequency (FF)
    $68e8   -$10    Filter Resonance (Q)
    $68d8   -$20    Final DCA (ENV4)
    $68b8   -$40    Panning (PAN)
    $6878   -$80    Floppy (Motor/LED on - SQ-80 only)

ESQ1: 8x CEM3379 VC Signal Processor Filter/Mix/VCA, 1x CEM3360 Dual VCA, 4x SSM2300
SQ-80: 8x CEM3379 VC Signal Processor - Filter/Mix/VCA, 1x CEM3360 Dual VCA, 4x SSM2300


If SEQRAM is mapped at 4000, DUART port 2 determines the 32KB "master bank" and ports 0 and 1
determine which of the 4 8KB "sub banks" is visible.

Output ports 3 to 1 determine the 4kB page which should be shown at $7000 to $7fff.

IRQ sources are the DUART and the DRQ line from the FDC (SQ-80 only).
NMI is from the IRQ line on the FDC (again, SQ-80 only).

TODO:
    - VFD display
    - Keyboard
]    - Analog filters and VCA on the back end of the 5503 (inaccurate)
     - duart seems to keep interrupting even after MIDI xmit buffer becomes empty

NOTES:
    Commands from KPC are all 2 bytes

    first byte: command code, bit 7 is 1 = press, 0 = release
    second byte is source: 00 = panel  01 = internal keyboard

    04 SEQ
    05 CART A
    06 CART B
    07 INT
    08 1 / SEQ 1
    09 2 / SEQ 2
    0A 3 / SEQ 3
    0B 4 / SONG
    0C COMPARE
    0D DATA UP
    0E DATA DOWN
    0F WRITE
    10 = UPPER 1 (buttons above display)
    11 = UPPER 2
    12 = UPPER 3
    13 = UPPER 4
    14 = UPPER 5
    15 = LOWER 1 (buttons below display)
    16 = LOWER 2
    17 = LOWER 3
    18 = LOWER 4
    19 = LOWER 5
    1a = LFO 1
    1b = ENV 2
    1c = MASTER
    1d = CREATE / ERASE
    1e = SELECT
    1f = RECORD
    20 = STORAGE
    21 = EDIT
    22 = MIX
    23 = STOP / CONT
    24 = MIDI
    25 = CONTROL
    26 = LOCATE
    27 = PLAY
    28 = OSC 1
    29 = OSC 2
    2A = OSC 3
    2B = ENV 1
    2C = DCA 1
    2D = DCA 2
    2E = DCA 3
    2F = LFO 2
    30 = LFO 3
    31 = FILTER
    32 = ENV 4
    33 = ENV 3
    34 = DCA 4
    35 = MODES
    36 = SPLIT / LAYER


    Analog filters (CEM3379):

    The analog part is relatively simple.  The digital part outputs 8
    voices, which are filtered, amplified, panned then summed
    together.

    The filtering stage is a 4-level lowpass filter with a loopback:


             +-[+]-<-[*-1]--------------------------+
             |  |                                   |
             ^ [*r]                                 |
             |  |                                   |
             |  v                                   ^
    input ---+-[+]--[LPF]---[LPF]---[LPF]---[LPF]---+--- output

    All 4 LPFs are identical, with a transconductance G:

    output = 1/(1+s/G)^4 * ( (1+r)*input - r*output)

    or

    output = input * (1+r)/((1+s/G)^4+r)

    to which the usual z-transform can be applied (see votrax.c)

    G is voltage controlled through the Vfreq input, with the formula (Vfreq in mV):

         G = 6060*exp(Vfreq/28.5)

    That gives a cutoff frequency (f=G/(2pi)) of 5Hz at 5mV, 964Hz at
    28.5mV and 22686Hz at 90mV.  The resistor ladder between the DAC
    and the input seem to map 0..255 into a range of -150.4mV to
    +83.6mV.

    The resonance is controlled through the Vq input pin, and is not
    well defined.  Reading between the lines the control seems linear
    and tops when then circuit is self-oscillation, at r=4.

    The amplification is exponential for a control voltage between 0
    to 0.2V from -100dB to -20dB, and then linear up to 5V at 0dB.  Or
    in other words:
         amp(Vca) = Vca < 0.2 ? 10**(-5+20*Vca) : Vca*0.1875 + 0.0625


    Finally the panning is not very described.  What is clear is that
    the control voltage at 2.5V gives a gain of -6dB, the max
    attenuation at 0/5V is -100dB.  The doc also says the gain is
    linear between 1V and 3.5V, which makes no sense since it's not
    symmetrical, and logarithmic afterwards, probably meaning
    exponential, otherwise the change between 0 and 1V would be
    minimal.  So we're going to do some assumptions:
        - 0-1V exponential from -100Db to -30dB
        - 1V-2.5V linear from -30dB to -6dB
        - 2.5V-5V is 1-amp at 2.5V-v

    Note that this may be incorrect, maybe to sum of squares should be
    constant, the half-point should be at -3dB and the linearity in dB
    space.


***************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/m6809/m6809.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"
#include "esqpanel.h"
#include "sound/es5503.h"
#include "speaker.h"


#define WD1772_TAG      "wd1772"

class esq1_filters : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	esq1_filters(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_vca(int channel, uint8_t value);
	void set_vpan(int channel, uint8_t value);
	void set_vq(int channel, uint8_t value);
	void set_vfc(int channel, uint8_t value);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct filter {
		uint8_t vca = 0, vpan = 0, vq = 0, vfc = 0;
		double amp = 0, lamp = 0, ramp = 0;
		double a[5]{}, b[5]{};
		double x[4]{}, y[4]{};
	};

	filter filters[8]{};

	sound_stream *stream = nullptr;

	void recalc_filter(filter &f);
};

DEFINE_DEVICE_TYPE(ESQ1_FILTERS, esq1_filters, "esq1_filters", "ESQ1 Filters stage")

esq1_filters::esq1_filters(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ESQ1_FILTERS, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void esq1_filters::set_vca(int channel, uint8_t value)
{
	if(filters[channel].vca != value) {
		stream->update();
		filters[channel].vca = value;
		recalc_filter(filters[channel]);
	}
}

void esq1_filters::set_vpan(int channel, uint8_t value)
{
	if(filters[channel].vpan != value) {
		stream->update();
		filters[channel].vpan = value;
		recalc_filter(filters[channel]);
	}
}

void esq1_filters::set_vq(int channel, uint8_t value)
{
	if(filters[channel].vq != value) {
		stream->update();
		filters[channel].vq = value;
		recalc_filter(filters[channel]);
	}
}

void esq1_filters::set_vfc(int channel, uint8_t value)
{
	if(filters[channel].vfc != value) {
		stream->update();
		filters[channel].vfc = value;
		recalc_filter(filters[channel]);
	}
}

void esq1_filters::recalc_filter(filter &f)
{
	// Filtering stage
	//   First let's establish the control values
	//   Some tuning may be required

	double vfc = -150.4 + (83.6+150.4)*f.vfc/255;
	double r = 4.0*f.vq/255;


	double g = 6060*exp(vfc/28.5);
	double zc = g/tan(g/2/44100);

/*  if(f.vfc) {
        double ff = g/(2*M_PI);
        double fzc = 2*M_PI*ff/tan(M_PI*ff/44100);
        fprintf(stderr, "%02x f=%f zc=%f zc1=%f\n", f.vfc, g/(2*M_PI), zc, fzc);
    }*/

	double gzc = zc/g;
	double gzc2 = gzc*gzc;
	double gzc3 = gzc2*gzc;
	double gzc4 = gzc3*gzc;
	double r1 = 1+r;

	f.a[0] = r1;
	f.a[1] = 4*r1;
	f.a[2] = 6*r1;
	f.a[3] = 4*r1;
	f.a[4] = r1;

	f.b[0] =    r1 + 4*gzc + 6*gzc2 + 4*gzc3 + gzc4;
	f.b[1] = 4*(r1 + 2*gzc          - 2*gzc3 - gzc4);
	f.b[2] = 6*(r1         - 2*gzc2          + gzc4);
	f.b[3] = 4*(r1 - 2*gzc          + 2*gzc3 - gzc4);
	f.b[4] =    r1 - 4*gzc + 6*gzc2 - 4*gzc3 + gzc4;

/*  if(f.vfc != 0)
        for(int i=0; i<5; i++)
            printf("a%d=%f\nb%d=%f\n",
                   i, f.a[i], i, f.b[i]);*/

	// Amplification stage
	double vca = f.vca*(5.0/255.0);
	f.amp = vca < 0.2 ? pow(10, -5+20*vca) : vca*0.1875 + 0.0625;

	// Panning stage
	//   Very approximative at best
	//   Left/right unverified
	double vpan = f.vpan*(5.0/255.0);
	double vref = vpan > 2.5 ? 2.5 - vpan : vpan;
	double pan_amp = vref < 1 ? pow(10, -5+3.5*vref) : vref*0.312 - 0.280;
	if(vref < 2.5) {
		f.lamp = pan_amp;
		f.ramp = 1-pan_amp;
	} else {
		f.lamp = 1-pan_amp;
		f.ramp = pan_amp;
	}
}

void esq1_filters::device_start()
{
	stream = stream_alloc(8, 2, 44100);
	for(auto & elem : filters)
		elem = filter();
	for(auto & elem : filters)
		recalc_filter(elem);
}

void esq1_filters::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
/*  if(0) {
        for(int i=0; i<8; i++)
            fprintf(stderr, " [%02x %02x %02x %02x]",
                    filters[i].vca,
                    filters[i].vpan,
                    filters[i].vq,
                    filters[i].vfc);
        fprintf(stderr, "\n");
    }*/

	for(int i=0; i<outputs[0].samples(); i++) {
		double l=0, r=0;
		for(int j=0; j<8; j++) {
			filter &f = filters[j];
			double x = inputs[j].get(i);
			double y = (x*f.a[0]
						+ f.x[0]*f.a[1] + f.x[1]*f.a[2] + f.x[2]*f.a[3] + f.x[3]*f.a[4]
						- f.y[0]*f.b[1] - f.y[1]*f.b[2] - f.y[2]*f.b[3] - f.y[3]*f.b[4]) / f.b[0];
			memmove(f.x+1, f.x, 3*sizeof(double));
			memmove(f.y+1, f.y, 3*sizeof(double));
			f.x[0] = x;
			f.y[0] = y;
			y = y * f.amp;
			l += y * f.lamp;
			r += y * f.ramp;
		}
		static double maxl = 0;
		if(l > maxl) {
			maxl = l;
//          fprintf(stderr, "%f\n", maxl);
		}

//      l *= 6553;
//      r *= 6553;
		l *= 2;
		r *= 2;
		outputs[0].put_clamp(i, l, 1.0);
		outputs[1].put_clamp(i, r, 1.0);
	}
}


namespace {

class esq1_state : public driver_device
{
public:
	esq1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_filters(*this, "filters"),
		m_fdc(*this, WD1772_TAG),
		m_panel(*this, "panel"),
		m_mdout(*this, "mdout"),
		m_es5503(*this, "es5503"),
		m_es5503_rom(*this, "es5503")
	{ }

	void sq80(machine_config &config);

	void esq1(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

private:
	required_device<cpu_device> m_maincpu;
	required_device<scn2681_device> m_duart;
	required_device<esq1_filters> m_filters;
	optional_device<wd1772_device> m_fdc;
	optional_device<esqpanel2x40_device> m_panel;
	optional_device<midi_port_device> m_mdout;
	required_device<es5503_device> m_es5503;
	required_region_ptr<uint8_t> m_es5503_rom;

	uint8_t wd1772_r(offs_t offset);
	void wd1772_w(offs_t offset, uint8_t data);
	uint8_t seqdosram_r(offs_t offset);
	void seqdosram_w(offs_t offset, uint8_t data);
	void mapper_w(uint8_t data);
	void analog_w(offs_t offset, uint8_t data);

	void duart_output(uint8_t data);

	uint8_t esq1_adc_read();

	uint8_t es5503_sample_r(offs_t offset);

	int m_mapper_state = 0;
	int m_seq_bank = 0;
	uint8_t m_seqram[0x10000]{};
	uint8_t m_dosram[0x2000]{};
	virtual void machine_reset() override ATTR_COLD;

	void send_through_panel(uint8_t data);
	void esq1_map(address_map &map) ATTR_COLD;
	void sq80_map(address_map &map) ATTR_COLD;
	void sq80_es5503_map(address_map &map) ATTR_COLD;

	bool kpc_calibrated = false;  // sq80 requires keyboard calibration acknowledgement
	int m_adc_target = 0;     // adc poll target (index into the table below)
	uint8_t m_adc_value[6] = { 0,0,128,0,0,0 }; // VALV,PEDV,PITV,MODV,FILV,BATV
};

uint8_t esq1_state::es5503_sample_r(offs_t offset)
{
	return m_es5503_rom[offset + (((m_es5503->get_channel_strobe() & 8)>>3) * 0x20000)];
}

void esq1_state::sq80_es5503_map(address_map &map)
{
	map(0x000000, 0x1ffff).r(FUNC(esq1_state::es5503_sample_r));
}

uint8_t esq1_state::esq1_adc_read()
{
	return m_adc_value[m_adc_target];
}

void esq1_state::machine_reset()
{
	// set default OSROM banking
	membank("osbank")->set_base(memregion("osrom")->base() );

	m_mapper_state = 1;
	m_seq_bank = 0;
	kpc_calibrated = false;
}

uint8_t esq1_state::wd1772_r(offs_t offset)
{
	return m_fdc->read(offset&3);
}

void esq1_state::wd1772_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset&3, data);
}

void esq1_state::mapper_w(uint8_t data)
{
	m_mapper_state = (data & 1);

//    printf("mapper_state = %d\n", data ^ 1);
}

void esq1_state::analog_w(offs_t offset, uint8_t data)
{
	if(!(offset & 8))
		m_filters->set_vfc(offset & 7, data);
	if(!(offset & 16))
		m_filters->set_vq(offset & 7, data);
	if(!(offset & 32))
		m_filters->set_vpan(offset & 7, data);
	if(!(offset & 64))
		m_filters->set_vca(offset & 7, data);
}

uint8_t esq1_state::seqdosram_r(offs_t offset)
{
	if (m_mapper_state)
	{
		return m_dosram[offset];
	}
	else
	{
		return m_seqram[offset + m_seq_bank];
	}
}

void esq1_state::seqdosram_w(offs_t offset, uint8_t data)
{
	if (m_mapper_state)
	{
		m_dosram[offset] = data;
	}
	else
	{
		m_seqram[offset + m_seq_bank] = data;
	}
}

void esq1_state::esq1_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();                 // OSRAM
	map(0x4000, 0x5fff).ram();                 // SEQRAM
	map(0x6000, 0x63ff).rw("es5503", FUNC(es5503_device::read), FUNC(es5503_device::write));
	map(0x6400, 0x640f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x6800, 0x68ff).w(FUNC(esq1_state::analog_w));
	map(0x7000, 0x7fff).bankr("osbank");
	map(0x8000, 0xffff).rom().region("osrom", 0x8000);  // OS "high" ROM is always mapped here
}

void esq1_state::sq80_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();                 // OSRAM
	map(0x4000, 0x5fff).rw(FUNC(esq1_state::seqdosram_r), FUNC(esq1_state::seqdosram_w));
	map(0x6000, 0x63ff).rw("es5503", FUNC(es5503_device::read), FUNC(es5503_device::write));
	map(0x6400, 0x640f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x6800, 0x68ff).w(FUNC(esq1_state::analog_w));
	map(0x6c00, 0x6dff).w(FUNC(esq1_state::mapper_w));
	map(0x6e00, 0x6fff).rw(FUNC(esq1_state::wd1772_r), FUNC(esq1_state::wd1772_w));
	map(0x7000, 0x7fff).bankr("osbank");
	map(0x8000, 0xffff).rom().region("osrom", 0x8000);  // OS "high" ROM is always mapped here
}

// from the schematics:
//
// DUART channel A is MIDI
// channel B is to the keyboard/display
// IP0 = tape in
// IP1 = sequencer expansion cartridge inserted
// IP2 = patch cartridge inserted
// IP3 & 4 are 0.5 MHz, IP 5 & 6 are 1 MHz (note 0.5 MHz / 16 = MIDI baud rate)
//
// OP0 = to display processor
// OP1/2/3 = bank select 0, 1, and 2
// OP4 = metronome low
// OP5 = metronome hi
// OP6/7 = tape out

void esq1_state::duart_output(uint8_t data)
{
	int bank = m_adc_target = ((data >> 1) & 0x7);
//  printf("DP [%02x]: %d mlo %d mhi %d tape %d\n", data, data&1, (data>>4)&1, (data>>5)&1, (data>>6)&3);
//  printf("%s [%02x] bank %d => offset %x\n", machine().describe_context().c_str(), data, bank, bank * 0x1000);
	membank("osbank")->set_base(memregion("osrom")->base() + (bank * 0x1000) );

	m_seq_bank = (data & 0x8) ? 0x8000 : 0x0000;
	m_seq_bank += ((data>>1) & 3) * 0x2000;
//    printf("seqram_bank = %x\n", state->m_seq_bank);
}

void esq1_state::send_through_panel(uint8_t data)
{
	m_panel->xmit_char(data);
}

INPUT_CHANGED_MEMBER(esq1_state::key_stroke)
{
	u8 offset = 0;
	if (strncmp(machine().basename().c_str(), "sq80", 4) == 0)
	 {
		if (!kpc_calibrated)
		 { // ack SQ80 keyboard calibration
			send_through_panel((u8)0xff);
			kpc_calibrated = true;
		}
		offset = 2; // SQ80 keycodes are offset by -2
	}

	if (oldval == 0 && newval == 1)
	{
		send_through_panel((u8)param - offset);
		send_through_panel((u8)0x00);
	}
	else if (oldval == 1 && newval == 0)
	{
		send_through_panel(((u8)param - offset)&0x7f);
		send_through_panel((u8)0x00);
	}
}

void esq1_state::esq1(machine_config &config)
{
	MC6809E(config, m_maincpu, 8_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq1_state::esq1_map);

	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq")); // open collector
	mainirq.output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	mainirq.output_handler().append_inputline(m_maincpu, M6809_FIRQ_LINE); // IRQ and FIRQ are tied together

	SCN2681(config, m_duart, 8_MHz_XTAL / 2);
	m_duart->set_clocks(8_MHz_XTAL / 16, 8_MHz_XTAL / 16, 8_MHz_XTAL / 8, 8_MHz_XTAL / 8);
	m_duart->irq_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_duart->a_tx_cb().set(m_mdout, FUNC(midi_port_device::write_txd));
	m_duart->b_tx_cb().set(m_panel, FUNC(esqpanel2x40_device::rx_w));
	m_duart->outport_cb().set(FUNC(esq1_state::duart_output));

	ESQPANEL2X40(config, m_panel);
	m_panel->write_tx().set(m_duart, FUNC(scn2681_device::rx_b_w));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w)); // route MIDI Tx send directly to 68681 channel A Rx

	midiout_slot(MIDI_PORT(config, "mdout"));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ESQ1_FILTERS(config, m_filters);
	m_filters->add_route(0, "lspeaker", 1.0);
	m_filters->add_route(1, "rspeaker", 1.0);

	ES5503(config, m_es5503, 8_MHz_XTAL);
	m_es5503->set_channels(8);
	m_es5503->irq_func().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_es5503->adc_func().set(FUNC(esq1_state::esq1_adc_read));
	m_es5503->add_route(0, "filters", 1.0, 0);
	m_es5503->add_route(1, "filters", 1.0, 1);
	m_es5503->add_route(2, "filters", 1.0, 2);
	m_es5503->add_route(3, "filters", 1.0, 3);
	m_es5503->add_route(4, "filters", 1.0, 4);
	m_es5503->add_route(5, "filters", 1.0, 5);
	m_es5503->add_route(6, "filters", 1.0, 6);
	m_es5503->add_route(7, "filters", 1.0, 7);
}

void esq1_state::sq80(machine_config &config)
{
	esq1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &esq1_state::sq80_map);

	m_es5503->set_addrmap(0, &esq1_state::sq80_es5503_map);
	m_es5503->irq_func().set_nop(); // not connected here

	WD1772(config, m_fdc, 8_MHz_XTAL);
	m_fdc->drq_wr_callback().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

static INPUT_PORTS_START( esq1 )
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x84) PORT_NAME("SEQ")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x85) PORT_NAME("CART A")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x86) PORT_NAME("CART B")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x87) PORT_NAME("INT")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x88) PORT_NAME("1 / SEQ 1")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x89) PORT_NAME("2 / SEQ 2")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8a) PORT_NAME("3 / SEQ 3")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8b) PORT_NAME("4 / SONG")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8c) PORT_NAME("COMPARE")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8e) PORT_NAME("DATA DOWN")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8d) PORT_NAME("DATA UP")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8f) PORT_NAME("WRITE")

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x90) PORT_NAME("UPPER 1")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x91) PORT_NAME("UPPER 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x92) PORT_NAME("UPPER 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x93) PORT_NAME("UPPER 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x99) PORT_NAME("UPPER 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x94) PORT_NAME("LOWER 1")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x95) PORT_NAME("LOWER 2")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x96) PORT_NAME("LOWER 3")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x97) PORT_NAME("LOWER 4")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x98) PORT_NAME("LOWER 5")


INPUT_PORTS_END

ROM_START( esq1 )
	ROM_REGION(0x10000, "osrom", 0)
	ROM_LOAD( "3p5lo.bin",    0x0000, 0x8000, CRC(ed001ad8) SHA1(14d1150bccdbc15d90567cf1812aacdb3b6ee882) )
	ROM_LOAD( "3p5hi.bin",    0x8000, 0x8000, CRC(332c572f) SHA1(ddb4f62807eb2ab29e5ac6b5d209d2ecc74cf806) )

	ROM_REGION(0x20000, "es5503", 0)
	ROM_LOAD( "esq1wavlo.bin", 0x0000, 0x8000, CRC(4d04ac87) SHA1(867b51229b0a82c886bf3b216aa8893748236d8b) )
	ROM_LOAD( "esq1wavhi.bin", 0x8000, 0x8000, CRC(94c554a3) SHA1(ed0318e5253637585559e8cf24c06d6115bd18f6) )
ROM_END

ROM_START( sq80 )
	ROM_REGION(0x10000, "osrom", 0)
	ROM_LOAD( "sq80rom.low",  0x0000, 0x008000, CRC(97ecd9a0) SHA1(cadff16ebbc15b52cf1d3335d22dc930d430a058) )
	ROM_LOAD( "sq80rom.hig",  0x8000, 0x008000, CRC(f83962b1) SHA1(e3e5cf41f15a37f8bf29b88fb1c85c0fca9ea912) )

	ROM_REGION(0x40000, "es5503", 0)
	ROM_LOAD( "2202.bin",     0x00000, 0x010000, CRC(dffd538c) SHA1(e90f6ff3a7804b54c8a3b1b574ec9c223a6c2bf9) )
	ROM_LOAD( "2203.bin",     0x20000, 0x010000, CRC(9be8cceb) SHA1(1ee4d7e6d2171b44e88e464071bdc4b800b69c4a) )
	ROM_LOAD( "2204.bin",     0x10000, 0x010000, CRC(4937c6f7) SHA1(4505efb9b28fe6d4bcc1f79e81a70bb215c399cb) )
	ROM_LOAD( "2205.bin",     0x30000, 0x010000, CRC(0f917d40) SHA1(1cfae9c80088f4c90b3c9e0b284c3b91f7ff61b9) )

	ROM_REGION(0x8000, "kpc", 0)    // 68HC11 keyboard/front panel processor
	ROM_LOAD( "sq80_kpc_150.bin", 0x000000, 0x008000, CRC(8170b728) SHA1(3ad68bb03948e51b20d2e54309baa5c02a468f7c) )
ROM_END

ROM_START( esqm )
	ROM_REGION(0x10000, "osrom", 0)
	ROM_LOAD( "1355500157_d640_esq-m_oshi.u14", 0x8000, 0x008000, CRC(ea6a7bae) SHA1(2830f8c52dc443b4ca469dc190b33e2ff15b78e1) )

	ROM_REGION(0x20000, "es5503", 0)
	ROM_LOAD( "esq1wavlo.bin", 0x0000, 0x8000, CRC(4d04ac87) SHA1(867b51229b0a82c886bf3b216aa8893748236d8b) )
	ROM_LOAD( "esq1wavhi.bin", 0x8000, 0x8000, CRC(94c554a3) SHA1(ed0318e5253637585559e8cf24c06d6115bd18f6) )
ROM_END

} // anonymous namespace


CONS( 1986, esq1, 0   , 0, esq1, esq1, esq1_state, empty_init, "Ensoniq", "ESQ-1 Digital Wave Synthesizer", MACHINE_NOT_WORKING )
CONS( 1986, esqm, esq1, 0, esq1, esq1, esq1_state, empty_init, "Ensoniq", "ESQ-M Digital Wave Synthesizer Module", MACHINE_NOT_WORKING )
CONS( 1988, sq80, 0,    0, sq80, esq1, esq1_state, empty_init, "Ensoniq", "SQ-80 Cross Wave Synthesizer", MACHINE_NOT_WORKING )
