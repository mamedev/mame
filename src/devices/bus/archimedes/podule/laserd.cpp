// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Concepts LaserDirect

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_LaserDirect.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "laserd.h"
#include "machine/7200fifo.h"
#include "bus/centronics/ctronics.h"


namespace {

class arc_laserd_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

protected:
	arc_laserd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


// ======================> arc_lbp4_device

class arc_lbp4_device : public arc_laserd_device
{
public:
	// construction/destruction
	arc_lbp4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


void arc_laserd_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x0000, 0x3fff).lw8(NAME([this](u8 data) { m_rom_page = data ^ 0xff; })).umask32(0x000000ff);
}

void arc_laserd_device::memc_map(address_map &map)
{
	//map(0x0000, 0x0003).r("fifo0", FUNC(fifo7200_device::data_word_r)).umask32(0x0000ffff);
	//map(0x0000, 0x0003).w("cent_data_out", FUNC(output_latch_device::write)).umask32(0x000000ff);
	map(0x0004, 0x0007).r("cent_status_in", FUNC(input_buffer_device::read)).umask32(0x000000ff);
	map(0x0004, 0x0007).w("cent_ctrl_out", FUNC(output_latch_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( lbp4 )
//-------------------------------------------------

ROM_START( lbp4 )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("lbp-4.rom", 0x0000, 0x8000, CRC(f21b8089) SHA1(07503fa6d29d34cc4ceb972296c9bd178085e412))
ROM_END

const tiny_rom_entry *arc_lbp4_device::device_rom_region() const
{
	return ROM_NAME( lbp4 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_laserd_device::device_add_mconfig(machine_config &config)
{
	IDT7201(config, "fifo0");
	IDT7201(config, "fifo1");

	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));
	centronics.busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit1));
	centronics.perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	centronics.select_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit3));
	centronics.fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit6));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in");

	output_latch_device &cent_ctrl_out(OUTPUT_LATCH(config, "cent_ctrl_out"));
	cent_ctrl_out.bit_handler<4>().set(centronics, FUNC(centronics_device::write_strobe));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_laserd_device - constructor
//-------------------------------------------------

arc_laserd_device::arc_laserd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}

arc_lbp4_device::arc_lbp4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_laserd_device(mconfig, ARC_LBP4, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_laserd_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_laserd_device::device_reset()
{
	m_rom_page = 0xff;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_LBP4, device_archimedes_podule_interface, arc_lbp4_device, "arc_lbp4", "Computer Concepts LaserDirect (Canon LBP-4)")
//DEFINE_DEVICE_TYPE_PRIVATE(ARC_LBP8, device_archimedes_podule_interface, arc_lbp8_device, "arc_lbp8", "Computer Concepts LaserDirect (Canon LBP-8)")
//DEFINE_DEVICE_TYPE_PRIVATE(ARC_QUME, device_archimedes_podule_interface, arc_qume_device, "arc_qume", "Computer Concepts LaserDirect (Qume)")
