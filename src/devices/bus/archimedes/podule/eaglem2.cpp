// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Wild Vision/Computer Concepts Eagle M2

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_Eagle.html

    Main components:
    - Am7202A  x 2 High Density First-In First-Out
    - M514221B x 4
    - SAA7191B Digital Multistandard ColourDecoder
    - SAA7186  Digital video scaler
    - SAA7197  Clock Generator Circuit
    - TDA8708B Video analog input interface
    - TDA8709A Video analog input interface
    - YMZ263B  Multimedia Audio & Game Interface Controller

    TODO:
    - everything, it's complex and not documented.

**********************************************************************/

#include "emu.h"
#include "eaglem2.h"
#include "machine/7200fifo.h"
#include "machine/saa7191.h"
#include "bus/midi/midi.h"
#include "imagedev/avivideo.h"
#include "speaker.h"


namespace {

class arc_eaglem2_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_eaglem2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE | feature::SOUND; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_device<saa7191_device> m_dmsd;
	required_device<idt7202_device> m_fifo_in;
	required_device<idt7202_device> m_fifo_out;
	required_device<avivideo_image_device> m_avivideo;

	u8 m_rom_page;
};


void arc_eaglem2_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x1f800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( eagle )
//-------------------------------------------------

ROM_START( eagle )
	ROM_REGION(0x20000, "podule_rom", 0)
	ROM_LOAD("eagle.rom", 0x0000, 0x20000, CRC(9d9fa2e9) SHA1(b3633b8e1d58f59f5894d14a737c69b6d9d5d46d))
ROM_END

const tiny_rom_entry *arc_eaglem2_device::device_rom_region() const
{
	return ROM_NAME( eagle );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_eaglem2_device::device_add_mconfig(machine_config &config)
{
	SAA7191(config, m_dmsd);

	IDT7202(config, m_fifo_in);  // AM7202A
	IDT7202(config, m_fifo_out); // AM7202A

	IMAGE_AVIVIDEO(config, m_avivideo);

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set("mdthru", FUNC(midi_port_device::write_txd));
	midiout_slot(MIDI_PORT(config, "mdout"));
	midiout_slot(MIDI_PORT(config, "mdthru"));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_eaglem2_device - constructor
//-------------------------------------------------

arc_eaglem2_device::arc_eaglem2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_EAGLEM2, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_dmsd(*this, "dmsd")
	, m_fifo_in(*this, "fifo_in")
	, m_fifo_out(*this, "fifo_out")
	, m_avivideo(*this, "srcavi")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_eaglem2_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_eaglem2_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_EAGLEM2, device_archimedes_podule_interface, arc_eaglem2_device, "arc_eaglem2", "Wild Vision/Computer Concepts Eagle M2")
