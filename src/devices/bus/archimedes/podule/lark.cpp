// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Wild Vision/Computer Concepts Lark A16

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_Lark.html

**********************************************************************/

#include "emu.h"
#include "lark.h"
#include "machine/7200fifo.h"
#include "machine/ins8250.h"
#include "sound/ad1848.h"
#include "bus/midi/midi.h"
#include "speaker.h"


namespace {

class arc_lark_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_lark_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_device<ad1848_device> m_ad1848;
	required_device<idt7202_device> m_fifo_in;
	required_device<idt7202_device> m_fifo_out;

	u8 status_r();
	void control_w(u8 data);

	void update_irqs();
	void set_irq(u8 mask, int state);
	void playback_drq(int state);

	u8 m_rom_page;
	u8 m_status;
	u8 m_ctrl;

	static constexpr u8 IRQ_MASTER   = 1 << 0;
	static constexpr u8 IRQ_AD1848   = 1 << 1;
	static constexpr u8 IRQ_16550    = 1 << 2;
	static constexpr u8 IRQ_FIFO_OUT = 1 << 6;
	static constexpr u8 IRQ_FIFO_IN  = 1 << 7;

	static constexpr u8 CTRL_FIFO_OUT_ENA = 1 << 3;
	static constexpr u8 CTRL_FIFO_IN_ENA  = 1 << 4;
	static constexpr u8 CTRL_RESET        = 1 << 7;
};


void arc_lark_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x1f800)]; })).umask32(0x000000ff);
	map(0x0000, 0x0000).lw8(NAME([](u8 data) { })); // XC3030 FPGA
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
	map(0x3000, 0x3000).rw(FUNC(arc_lark_device::status_r), FUNC(arc_lark_device::control_w));
	map(0x3400, 0x341f).rw("uart", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff);
	map(0x3c00, 0x3c0f).rw(m_ad1848, FUNC(ad1848_device::read), FUNC(ad1848_device::write)).umask32(0x000000ff);
}

void arc_lark_device::memc_map(address_map &map)
{
	map(0x0000, 0x0003).mirror(0x1ffc)
		.lr16(NAME([this]()
			{
				u16 data;
				data = m_fifo_in->data_byte_r() << 8;
				data |= m_fifo_in->data_byte_r();
				return data;
			}))
		.lw16(NAME([this](u16 data)
			{
				m_fifo_out->data_byte_w(data >> 8);
				m_fifo_out->data_byte_w(data & 0xff);

			})).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( lark )
//-------------------------------------------------

ROM_START( lark )
	ROM_REGION(0x20000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "110", "Lark A16 V1.10 (12 Jul 1994)")
	ROMX_LOAD("lark.rom", 0x0000, 0x20000, CRC(9bb6cb99) SHA1(f0b2ba52d0069dfe00a7ba66fba23e0eb2346d8c), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_lark_device::device_rom_region() const
{
	return ROM_NAME( lark );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_lark_device::device_add_mconfig(machine_config &config)
{
	AD1848(config, m_ad1848, 0);
	//m_ad1848->irq().set([this](int state) { set_irq(IRQ_AD1848, state); }); // not used/connected?
	m_ad1848->drq().set(FUNC(arc_lark_device::playback_drq));

	IDT7202(config, m_fifo_out); // AM7202A
	m_fifo_out->hf_handler().set([this](int state) { set_irq(IRQ_FIFO_OUT, state); });
	IDT7202(config, m_fifo_in); // AM7202A
	m_fifo_in->hf_handler().set([this](int state) { set_irq(IRQ_FIFO_IN, !state); });

	ns16550_device &uart(NS16550(config, "uart", DERIVED_CLOCK(1, 4)));
	uart.out_tx_callback().set("mdout", FUNC(midi_port_device::write_txd));
	uart.out_tx_callback().append("mdthru", FUNC(midi_port_device::write_txd));
	uart.out_int_callback().set([this](int state) { set_irq(IRQ_16550, state); });

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set("uart", FUNC(ns16550_device::rx_w));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));
	midiout_slot(MIDI_PORT(config, "mdout"));
	midiout_slot(MIDI_PORT(config, "mdthru"));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_lark_device - constructor
//-------------------------------------------------

arc_lark_device::arc_lark_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_LARK, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_ad1848(*this, "ad1848")
	, m_fifo_in(*this, "fifo_in")
	, m_fifo_out(*this, "fifo_out")
	, m_rom_page(0)
	, m_status(0)
	, m_ctrl(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_lark_device::device_start()
{
	save_item(NAME(m_rom_page));
	save_item(NAME(m_status));
	save_item(NAME(m_ctrl));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_lark_device::device_reset()
{
	m_rom_page = 0x00;
	m_status = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_lark_device::status_r()
{
	if (m_status ^ IRQ_FIFO_IN)
		return (m_status | IRQ_MASTER);
	else
		return m_status;
}

void arc_lark_device::control_w(u8 data)
{
	m_ctrl = data;

	update_irqs();

	if (m_ctrl & CTRL_RESET)
		m_fifo_out->reset();
}

void arc_lark_device::update_irqs()
{
	u8 irq_status = m_status ^ 0x80;

	if (!(m_ctrl & CTRL_FIFO_IN_ENA))
		irq_status &= 0x7f;

	set_pirq(irq_status ? ASSERT_LINE : CLEAR_LINE);
}

void arc_lark_device::set_irq(u8 mask, int state)
{
	if (state)
		m_status |= mask;
	else
		m_status &= ~mask;

	update_irqs();
}

void arc_lark_device::playback_drq(int state)
{
	if (state)
	{
		if (m_ctrl & CTRL_FIFO_OUT_ENA)
		{
			// 16-bit stereo data stream
			m_ad1848->dack_w(m_fifo_out->data_byte_r());
			m_ad1848->dack_w(m_fifo_out->data_byte_r());
			m_ad1848->dack_w(m_fifo_out->data_byte_r());
			m_ad1848->dack_w(m_fifo_out->data_byte_r());
		}
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_LARK, device_archimedes_podule_interface, arc_lark_device, "arc_lark", "Wild Vision/Computer Concepts Lark A16")
