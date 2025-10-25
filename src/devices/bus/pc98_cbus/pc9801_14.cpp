// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC9801-14 sound board

TMS3631 sound chip (same as Siel DK-600 / Opera 6)
4 octaves, 12 notes, 8 channels
CH1 and CH2 are fixed to center with no envelope
CH3-5 are fixed to the left
CH6-8 to the right
7 knobs on back panel, F2/F4/F8/F16 then L/R and VOL

References:
- http://www.retropc.net/mm/archives/1530
- https://yfl711.hateblo.jp/search?q=TMS3631%E5%88%B6%E5%BE%A1
- undocumented mem io_music.txt

TODO:
- enough to make it talk with the sound interface and not much else.

===================================================================================================

- Known games with PC9801-14 support
  gamepac1 - flappy

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
//  , m_tms(*this, "tms")
{
}



void pc9801_14_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	// 8253A
	PIT8253(config, m_pit);
	m_pit->set_clk<2>(1'996'800 / 8);
	m_pit->out_handler<2>().set([this] (int state) {
		m_bus->int_w<5>(state);
	});

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set([this](uint8_t data) { LOG("TMS3631: PA envelope 1 %02x\n", data); });
	m_ppi->out_pb_callback().set([this](uint8_t data) { LOG("TMS3631: PB envelope 2 %02x\n", data); });
//  m_ppi->in_pc_callback().set_constant(0x08);
	m_ppi->out_pc_callback().set([this](uint8_t data) { LOG("TMS3631: data %02x\n", data); });

	// TODO: TMS3631-RI104 & TMS3631-RI105
	// TMS3631(config, m_tms, 1'996'800);
	// m_tms->add_route(0, "speaker", 0.5, 0);
	// m_tms->add_route(1, "speaker", 0.5, 1);
}

ROM_START( pc9801_14 )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "vfz01_00.bin", 0x0001, 0x2000, CRC(3b227477) SHA1(85474b0550d58395ae9ca53658f93ad2f87fdd4d) )
	ROM_LOAD16_BYTE( "vfz02_00.bin", 0x0000, 0x2000, CRC(a386ab6b) SHA1(5b014c5de1b8e41a412cafd61d7e9d18abdeb6be) )
ROM_END

const tiny_rom_entry *pc9801_14_device::device_rom_region() const
{
	return ROM_NAME( pc9801_14 );
}


static INPUT_PORTS_START( pc9801_14 )
	// TODO: jumpers for int line and port selection
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
	// assumed, loads up in n88bas61 with switch.n88 setup
	m_bus->program_space().install_rom(
		0xcc000,
		0xcffff,
		memregion(this->subtag("sound_bios").c_str())->base()
	);

	m_bus->install_device(0x0000, 0x3fff, *this, &pc9801_14_device::io_map);
}

void pc9801_14_device::io_map(address_map &map)
{
	map(0x0088, 0x008f).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	// TODO: identifier + port select, mirrored over the full space?
	// --xx xx0A xxxx 1AA0
	// x = selects active port (jumpers)
	// - = don't care (mirrored?)
	// A = card select
	// 0x00 and 0xff values aren't valid
	map(0x008e, 0x008e).lr8(NAME([this] () { LOG("PC9801-14: read base port / identifier\n"); return 0x08; }));
	// mirror according to io_sound.txt
	map(0x0188, 0x0188).mirror(2).lw8(NAME([this] (offs_t offset, u8 data) { LOG("TMS3631 mask %02x\n", data); }));
	map(0x018c, 0x018c).lrw8(
		NAME([this] (offs_t offset) { return m_pit->read(2); }),
		NAME([this] (offs_t offset, u8 data) { m_pit->write(2, data); })
	);
	map(0x018e, 0x018e).lrw8(
		NAME([this] () {
			LOG("PC9801-14: read int line\n");
			// hardwire to INT5 for now
			return 0x80;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pit->write(3, data);
		})
	);
}
