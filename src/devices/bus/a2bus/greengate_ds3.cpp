// license:BSD-3-Clause
// copyright-holders:Robyn Powell
/*********************************************************************

    greengate_ds3.cpp

    Greengate DS:3 digital sampler card (UK, 1984)

    Four voice 8-bit sample playback from Apple II main memory, plus
    an 8-bit ADC for recording and a 64 key diode matrix keyboard.
    Hardware reverse engineered by Tube Time ("Goodgreat DS3"):
    https://github.com/schlae/goodgreat-ds3

    The card has no ROM. Everything is decoded from /IOSEL, so the
    software expects it in slot 5:

      $Cn00-$Cn1F  MC6844 DMA controller
      $Cn20-$Cn3F  6522 VIA (keyboard matrix, IRQ from the DMAC)
      $Cn40-$Cn5F  AM9513 timer (A0 = C/D)
      $Cn60-$Cn7F  ZN448 ADC result
      $C0nX        ZN448 start conversion (/DEVSEL drives /WR)

    AM9513 OUT1-OUT4 clock four 74LS74 latches that raise the 6844's
    TxRQ0-3. The 6844 steals a cycle (TSC mode, /DRQ1), reads a byte
    from the bus and the Tx AKA/AKB lines latch it into the matching
    ZN428 DAC, which also clears that request latch. OUT5 clocks the
    ADC. The 6844 /IRQ/DEND goes to the VIA CA1, the VIA drives /IRQ.

    TODO:
    - The 6502 is not actually held off during the stolen cycles
    - ADC has no audio input, it always converts silence
    - The DMA grant state machine (U2/U3/U4) is collapsed into an
      immediate grant

*********************************************************************/

#include "emu.h"
#include "greengate_ds3.h"

#include "machine/6522via.h"
#include "machine/am9513.h"
#include "machine/mc6844.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class a2bus_ds3_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	a2bus_ds3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, A2BUS_DS3, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_dmac(*this, "dmac"),
		m_via(*this, "via"),
		m_stc(*this, "stc"),
		m_dac(*this, "dac%u", 0U),
		m_keys(*this, "GROUP%u", 1U),
		m_adc_timer(nullptr),
		m_group(0xff),
		m_adc_data(0x80),
		m_stc_out(0)
	{
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual void reset_from_bus() override;

private:
	template <unsigned N> void stc_out_w(int state);
	template <unsigned N> void dac_w(uint8_t data);
	uint8_t dma_mem_r(offs_t offset) { return slot_dma_read(offset); }
	uint8_t keyboard_r();
	void adc_start();
	TIMER_CALLBACK_MEMBER(adc_done);

	required_device<mc6844_device> m_dmac;
	required_device<via6522_device> m_via;
	required_device<am9513_device> m_stc;
	required_device_array<dac_byte_interface, 4> m_dac;
	required_ioport_array<8> m_keys;

	emu_timer *m_adc_timer;
	uint8_t m_group;
	uint8_t m_adc_data;
	uint8_t m_stc_out;
};


// Keys are wired as eight groups of eight. The group (octave) lines are
// driven low by VIA port A, the note lines are read back on port B in
// this scrambled order: PB0-PB7 = note 4, 1, 3, 0, 2, 6, 7, 5
#define DS3_GROUP(g) \
	PORT_START("GROUP" #g) \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 1") \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 2") \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 3") \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 4") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 5") \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 6") \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 7") \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Keyboard Group " #g " Note 8")

INPUT_PORTS_START( ds3 )
	DS3_GROUP(1)
	DS3_GROUP(2)
	DS3_GROUP(3)
	DS3_GROUP(4)
	DS3_GROUP(5)
	DS3_GROUP(6)
	DS3_GROUP(7)
	DS3_GROUP(8)
INPUT_PORTS_END

void a2bus_ds3_device::reset_from_bus()
{
	// /RES from the slot goes to the 6844 and the 6522
	m_dmac->reset();
	m_via->reset();
}

ioport_constructor a2bus_ds3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ds3 );
}

void a2bus_ds3_device::device_add_mconfig(machine_config &config)
{
	// IC3_CLK (6844 E, 6522 PHI2) is the bus PHI0, which averages 65 cycles
	// per 456 7M periods because of the long cycle on each scan line. The
	// 6522 timers set the sequencer tempo, and the usual 1.0218 MHz
	// approximation plays 0.13% fast against a recording of real hardware.
	MC6844(config, m_dmac, DERIVED_CLOCK(65, 456));
	m_dmac->in_memr_callback().set(FUNC(a2bus_ds3_device::dma_mem_r));
	m_dmac->out_iow_callback<0>().set(FUNC(a2bus_ds3_device::dac_w<0>));
	m_dmac->out_iow_callback<1>().set(FUNC(a2bus_ds3_device::dac_w<1>));
	m_dmac->out_iow_callback<2>().set(FUNC(a2bus_ds3_device::dac_w<2>));
	m_dmac->out_iow_callback<3>().set(FUNC(a2bus_ds3_device::dac_w<3>));
	// the bus arbitration state machine grants the next 6502 cycle
	m_dmac->out_drq1_callback().set(m_dmac, FUNC(mc6844_device::dgrnt_w));
	// /IRQ//DEND
	m_dmac->out_int_callback().set(m_via, FUNC(via6522_device::write_ca1)).invert();

	MOS6522(config, m_via, DERIVED_CLOCK(65, 456));
	m_via->writepa_handler().set([this] (uint8_t data) { m_group = data; });
	m_via->readpb_handler().set(FUNC(a2bus_ds3_device::keyboard_r));
	m_via->irq_handler().set([this] (int state) {
		if (state)
			raise_slot_irq();
		else
			lower_slot_irq();
	});

	// clocked from the bus 7M signal
	AM9513(config, m_stc, DERIVED_CLOCK(1, 1));
	m_stc->out1_cb().set(FUNC(a2bus_ds3_device::stc_out_w<0>));
	m_stc->out2_cb().set(FUNC(a2bus_ds3_device::stc_out_w<1>));
	m_stc->out3_cb().set(FUNC(a2bus_ds3_device::stc_out_w<2>));
	m_stc->out4_cb().set(FUNC(a2bus_ds3_device::stc_out_w<3>));
	// OUT5 is the ZN448 conversion clock

	// four separate outputs on the original, mixed here
	SPEAKER(config, "speaker").front_center();
	for (auto &dac : m_dac)
		DAC_8BIT_R2R(config, dac).set_output_range(-1.0, 1.0).add_route(ALL_OUTPUTS, "speaker", 0.25); // ZN428E
}

void a2bus_ds3_device::device_start()
{
	m_adc_timer = timer_alloc(FUNC(a2bus_ds3_device::adc_done), this);

	save_item(NAME(m_group));
	save_item(NAME(m_adc_data));
	save_item(NAME(m_stc_out));
}

void a2bus_ds3_device::device_reset()
{
	// all AM9513 SRC and GATE inputs are tied high on the card
	m_stc->gate1_w(1);
	m_stc->gate2_w(1);
	m_stc->gate3_w(1);
	m_stc->gate4_w(1);
	m_stc->gate5_w(1);
	m_stc->source1_w(1);
	m_stc->source2_w(1);
	m_stc->source3_w(1);
	m_stc->source4_w(1);
	m_stc->source5_w(1);

	// the outputs are AC coupled downstream, start the DACs at mid-scale
	// rather than full negative so idle is silent
	for (auto &dac : m_dac)
		dac->write(0x80);

	m_adc_timer->adjust(attotime::never);
	m_via->write_cb2(1); // ZN448 /BUSY idle
}

template <unsigned N>
void a2bus_ds3_device::stc_out_w(int state)
{
	// rising edge of the timer output sets the request latch
	if (state && !BIT(m_stc_out, N))
		m_dmac->dreq_w<N>(ASSERT_LINE);

	m_stc_out = (m_stc_out & ~(1 << N)) | ((state ? 1 : 0) << N);
}

template <unsigned N>
void a2bus_ds3_device::dac_w(uint8_t data)
{
	// /DACENn latches the DAC and clears the matching request latch
	m_dac[N]->write(data);
	m_dmac->dreq_w<N>(CLEAR_LINE);
}

uint8_t a2bus_ds3_device::keyboard_r()
{
	uint8_t data = 0xff;
	for (int g = 0; g < 8; g++)
		if (!BIT(m_group, g))
			data &= m_keys[g]->read();
	return data;
}

void a2bus_ds3_device::adc_start()
{
	// ZN448 needs 9 clock periods, the clock comes from AM9513 OUT5
	// which isn't tracked, so assume a typical conversion time
	m_adc_data = 0x80;
	m_via->write_cb2(0);
	m_adc_timer->adjust(attotime::from_usec(10));
}

TIMER_CALLBACK_MEMBER(a2bus_ds3_device::adc_done)
{
	m_via->write_cb2(1);
}

uint8_t a2bus_ds3_device::read_c0nx(uint8_t offset)
{
	if (!machine().side_effects_disabled())
		adc_start();
	return get_open_bus();
}

void a2bus_ds3_device::write_c0nx(uint8_t offset, uint8_t data)
{
	adc_start();
}

uint8_t a2bus_ds3_device::read_cnxx(uint8_t offset)
{
	switch ((offset >> 5) & 3) // A7 not decoded
	{
		case 0: return m_dmac->read(offset & 0x1f);
		case 1: return m_via->read(offset & 0x0f);
		case 2: return m_stc->read8(offset & 0x01);
		case 3: return m_adc_data;
		default: return get_open_bus();
	}
}

void a2bus_ds3_device::write_cnxx(uint8_t offset, uint8_t data)
{
	switch ((offset >> 5) & 3) // A7 not decoded
	{
		case 0: m_dmac->write(offset & 0x1f, data); break;
		case 1: m_via->write(offset & 0x0f, data); break;
		case 2: m_stc->write8(offset & 0x01, data); break;
		default: break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_DS3, device_a2bus_card_interface, a2bus_ds3_device, "a2ds3", "Greengate DS:3 Digital Sampler")
