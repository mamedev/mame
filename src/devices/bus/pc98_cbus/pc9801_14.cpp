// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC9801-14 sound board

TMS3631 sound chip (same as Siel DK-600 / Opera 6)

http://www.retropc.net/mm/archives/1530

TODO:
- enough to make it talk with the sound interface and not much else.

===================================================================================================

- Known games with PC9801-14 support
    gamepac1:flappy

**************************************************************************************************/

#include "emu.h"

#include "pc9801_14.h"

#include "speaker.h"


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(PC9801_14, pc9801_14_device, "pc9801_14", "NEC PC-9801-14")

pc9801_14_device::pc9801_14_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_14, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_ppi(*this, "ppi")
	, m_pit(*this, "pit")
//	, m_tms(*this, "tms")
{
}


void pc9801_14_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();

	PIT8253(config, m_pit);
	m_pit->set_clk<2>(1'996'800 / 8);
	m_pit->out_handler<2>().set([this] (int state) {
		m_bus->int_w<5>(state);
	});

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set([this](uint8_t data) { LOG("TMS3631: PA envelope 1 %02x\n", data); });
	m_ppi->out_pb_callback().set([this](uint8_t data) { LOG("TMS3631: PB envelope 2 %02x\n", data); });
//	m_ppi->in_pc_callback().set_constant(0x08);
	m_ppi->out_pc_callback().set([this](uint8_t data) { LOG("TMS3631: data %02x\n", data); });
}

static INPUT_PORTS_START( pc9801_14 )
	// TODO: dipswitches for int line selection (at least)
INPUT_PORTS_END

ioport_constructor pc9801_14_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_14 );
}


void pc9801_14_device::device_validity_check(validity_checker &valid) const
{
}

void pc9801_14_device::device_start()
{
}

void pc9801_14_device::device_reset()
{
	m_bus->install_device(0x0000, 0x01ff, *this, &pc9801_14_device::io_map);
}

void pc9801_14_device::io_map(address_map &map)
{
	map(0x0088, 0x008f).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	// TODO: identifier, should it be from PPI?
	map(0x008e, 0x008e).lr8(NAME([] () { return 0x08; }));
	map(0x0188, 0x0188).mirror(2).lw8(NAME([this] (offs_t offset, u8 data) { LOG("TMS3631 mask %02x\n", data); }));
	map(0x018c, 0x018c).lrw8(
		NAME([this] (offs_t offset) { return m_pit->read(2); }),
		NAME([this] (offs_t offset, u8 data) { m_pit->write(2, data); }));
	map(0x018e, 0x018e).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PC9801-14: read DSW\n");
			return 0x80; // dip switch
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pit->write(3, data);
		})
	);
}
