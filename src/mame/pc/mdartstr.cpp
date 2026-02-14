// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Medalist Spectrum HW

https://www.youtube.com/watch?v=-kxk8UtTeIM

Error 033 is A20 line related (cfr. below)
Game looks PTS-DOS based

TODO:
- Requires a dartboard layout;
- Lamps;
- Ticket dispenser;
- Needs optional printer and FDC options (cfr. service mode "File");
- Sensor test (for out of board darts?), seems tied to an irq or NMI;
- "Dart Star" title GFX can glitch out if coin is inserted at wrong time (verify);

===================================================================================================

Spectrum Avanti (Medalist Dart Star)
Processor: Intel 386SX (25MHz) (NG80386SX25)

Chipset: CHIPS F82C836 (Single Chip 386sx / SCATsx)

System RAM: 2 x Alliance AS4C256K16F0-50JC (1 MB) [640K Main, 384K Extended]

Video controller: CHIPS F65535/A
VRAM: Alliance AS4C256K16F0-50JC (512 KB)

Additional chips:
2 x NEC D71055C
IDT 7202
Analog Devices AD7224KN
Samsung K6T1008C2E-DL70 near mem roms (mixed ROM/RAM disk?)

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
//#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/7200fifo.h"
#include "machine/at_keybc.h"
#include "machine/f82c836.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/spkrdev.h"
#include "video/pc_vga_chips.h"

#include "speaker.h"


/*
 * ISA16 Medalist ROM DISK
 * handles ROM, NVRAM, I/O and sound, all on the same board
 *
 */

class isa16_medalist_rom_disk : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_medalist_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_rom_tag(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_memory_region m_rom;
	required_ioport m_system;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport_array<4> m_in_target;
	required_ioport m_dsw;
	required_device<idt7202_device> m_fifo;
	required_device<ad7224_device> m_dac;
	required_device<nvram_device> m_nvram;

	u32 m_bank_address;
	bool m_nvram_select;
	std::unique_ptr<u8[]> m_nvram_ptr;
	emu_timer *m_dac_timer;
	TIMER_CALLBACK_MEMBER(dac_cb);
	u8 m_target_select;

	void io_map(address_map &map);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
};

DEFINE_DEVICE_TYPE(ISA16_MEDALIST_ROM_DISK, isa16_medalist_rom_disk, "isa16_medalist_rom_disk", "ISA16 Medalist Spectrum ROM DISK")

isa16_medalist_rom_disk::isa16_medalist_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_MEDALIST_ROM_DISK, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_system(*this, "SYSTEM")
	, m_in0(*this, "IN0")
	, m_in1(*this, "IN1")
	, m_in_target(*this, "TARGET_%u", 1U)
	, m_dsw(*this, "DSW")
	, m_fifo(*this, "fifo")
	, m_dac(*this, "dac")
	, m_nvram(*this, "nvram")
{
}

void isa16_medalist_rom_disk::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SPEAKER(config, "mono").front_center();

	IDT7202(config, m_fifo);
	// TODO: one byte off (speaker pops), may disconnect timer on FIFO empty?
	//m_fifo->ef_handler().set([this] (int state) {
	//	printf("%d\n", state);
	//	if (state)
	//		m_dac_timer->adjust(attotime::from_hz(11'025));
	//});

	AD7224(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.50);
}


void isa16_medalist_rom_disk::device_start()
{
	set_isa_device();
	save_item(NAME(m_nvram_select));
	save_item(NAME(m_bank_address));
	save_item(NAME(m_target_select));

	const u32 nvram_size = 0x20000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);

	m_dac_timer = timer_alloc(FUNC(isa16_medalist_rom_disk::dac_cb), this);
}

void isa16_medalist_rom_disk::device_reset()
{
	m_nvram_select = false;
	m_bank_address = 0;
	m_target_select = 0xff;
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);

	m_dac_timer->adjust(attotime::from_hz(11'025), 0, attotime::from_hz(11'025));
}

void isa16_medalist_rom_disk::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// C: drive 0xc0000, 0xc7fff
		// (debug option, will fail for missing COMMAND.COM that is otherwise present in BIOS)
		// D: drive
		m_isa->install_memory(0xd0000, 0xdffff, read8sm_delegate(*this, FUNC(isa16_medalist_rom_disk::read)), write8sm_delegate(*this, FUNC(isa16_medalist_rom_disk::write)));
	}
	else if (space_id == AS_IO)
	{
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_medalist_rom_disk::io_map);
	}
}

u8 isa16_medalist_rom_disk::read(offs_t offset)
{
	if (m_nvram_select)
	{
		const u32 nvram_size = 0x20000;
		const u32 nvram_address = offset | m_bank_address;
		if (nvram_address < nvram_size)
			return m_nvram_ptr[nvram_address];

		return 0xff;
	}
	return m_rom->base()[offset | m_bank_address];
}

void isa16_medalist_rom_disk::write(offs_t offset, u8 data)
{
	if (m_nvram_select)
	{
		const u32 nvram_size = 0x20000;
		const u32 nvram_address = offset | m_bank_address;
		if (nvram_address < nvram_size)
			m_nvram_ptr[nvram_address] = data;
	}
}

TIMER_CALLBACK_MEMBER(isa16_medalist_rom_disk::dac_cb)
{
	if (!m_fifo->ef_r())
		return;

	const u8 data = m_fifo->data_byte_r();

//	printf("%02x %d\n", data, m_fifo->ef_r());

	m_dac->data_w(data);
}


void isa16_medalist_rom_disk::io_map(address_map &map)
{
	map(0x0300, 0x0301).lr16(
		NAME([this] (offs_t offset) {
			// target segments
			u16 res = 0xffff;

			for (int i = 0; i < 4; i++)
			{
				if (!BIT(m_target_select, i))
					res &= m_in_target[i]->read();
			}

			return res;
		})
	);
	// 302 reads (?)
	map(0x0302, 0x0303).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				logerror("$303 Unknown write %02x\n", data);
			}
			else
			{
				m_target_select = data;
				if ((data & 0xf0) != 0xf0)
					logerror("$302 Unknown write nibble %02x\n", data);
			}
		})
	);
	map(0x0308, 0x0309).lr8(
		NAME([this] (offs_t offset) -> u8 {
			if (offset)
			{
				if (!machine().side_effects_disabled())
					logerror("$309 Unknown read\n");
				return 0xff;
			}

			return m_in0->read();
		})
	);
	// $309 write: lamps (bit 0-3 buttons 1-4, bit 4 Player Change), bit 7 ticket dispenser
	map(0x030a, 0x030b).lr8(
		NAME([this] (offs_t offset) -> u8 {
			if (offset)
			{
				if (!machine().side_effects_disabled())
					logerror("$30b Unknown read\n");
				return 0xff;
			}

			return m_in1->read();
		})
	);
	map(0x0310, 0x0310).w(m_fifo, FUNC(idt7202_device::data_byte_w));
	map(0x0320, 0x0321).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			if (offset)
				return m_system->read();

			if (!machine().side_effects_disabled())
				logerror("$320 Unknown read\n");
			return 0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_bank_address = (data & 0xf) * 0x10000;
				m_nvram_select = false;
				if (BIT(data, 4))
					logerror("$321 Unknown bit 4 high (%02x)\n", data);
				// TODO: perhaps use address_map_bank
				const u32 rom_select = (data & 0xe0) >> 5;
				if (rom_select == 7)
					m_nvram_select = true;
				else
					m_bank_address += rom_select << 20;
			}
			else
			{
				logerror("$320 Unknown write %02x\n", data);
			}
		})
	);
	map(0x0322, 0x0323).lr8(
		NAME([this] (offs_t offset) -> u8 {
			if (offset)
			{
				if (!machine().side_effects_disabled())
					logerror("$323 Unknown read\n");
				return 0xff;
			}

			return m_dsw->read();
		})
	);
}

static INPUT_PORTS_START( mdartstr )
	// BIOS do several checks with bit 4
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Reset?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Player Change button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("fifo", FUNC(idt7202_device::ff_r))

	// pressing a single and a double or triple segment at same time will make the matrix test
	// to detect in 4th column, inner vs outer resolver?
	PORT_START("TARGET_1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single Bull")
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -18")
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -18")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -18")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -12")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -12")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -12")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -5")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -5")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -5")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -20")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -20")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -20")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -1")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -1")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -1")

	PORT_START("TARGET_2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) // Single Bull mirror
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -4")
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -4")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -4")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -15")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -15")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -15")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -10")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -10")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -10")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -6")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -6")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -6")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -13")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -13")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -13")

	PORT_START("TARGET_3")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double Bull")
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -7")
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -7")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -7")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -2")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -2")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -2")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -17")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -17")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -17")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -3")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -3")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -3")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -19")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -19")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -19")

	PORT_START("TARGET_4")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) // Single Bull mirror
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -16")
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -16")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -16")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -9")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -9")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -9")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -14")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -14")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -14")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -11")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -11")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -11")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Triple -8")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double -8")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Single -8")

	// a Spectrum sheet claims that demo sounds are on SW3, and "Factory 2" pricing on SW8,
	// this may be for a different version tho (former is in service mode "board settings")
	PORT_START("DSW")
	PORT_SERVICE_DIPLOC(0x01, IP_ACTIVE_LOW, "SW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW:8" )
INPUT_PORTS_END

ioport_constructor isa16_medalist_rom_disk::device_input_ports() const
{
	return INPUT_PORTS_NAME(mdartstr);
}


/*
 *
 * ISA16 VGA bindings
 *
 */

class isa16_f65535_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_f65535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<f65535_vga_device> m_vga;
};


DEFINE_DEVICE_TYPE(ISA16_F65535, isa16_f65535_device, "f65535_isa16", "CT-65535 Integrated VGA card")

isa16_f65535_device::isa16_f65535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_F65535, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

void isa16_f65535_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(f65535_vga_device::screen_update));

	F65535_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(512*1024);
}

void isa16_f65535_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(f65535_vga_device::io_map));
}

void isa16_f65535_device::device_start()
{
	set_isa_device();
}

void isa16_f65535_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_f65535_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_w)));
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_f65535_device::io_isa_map);
}



namespace {

class mdartstr_state : public driver_device
{
public:
	mdartstr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void mdartstr(machine_config &config);

private:
	required_device<i386sx_device> m_maincpu;
	required_device<f82c836a_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void romdisk_config(device_t *device);
};

void mdartstr_state::main_map(address_map &map)
{
//  map(0x000000, 0x09ffff).ram();
//  map(0x0a0000, 0x0bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
//  map(0x0c0000, 0x0c7fff).rom().region("bios", 0x48000); // VGA BIOS + virtual floppy ISA
//  map(0x0c8000, 0x0cffff).rom().region("bios", 0x18000);
//  map(0x0e0000, 0x0fffff).rom().region("bios", 0x20000);
//  map(0x100000, 0x15ffff).ram();
	// TODO: fc0000-fdffff has an optional memory hole in chipset at $4e bit 4
	map(0xfc0000, 0xffffff).rom().region("bios", 0x00000);
}

void mdartstr_state::main_io(address_map &map)
{
//  map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
}

static void pc_isa_onboard(device_slot_interface &device)
{
	device.option_add_internal("vga",  ISA16_F65535);
	device.option_add_internal("boot", ISA16_MEDALIST_ROM_DISK);
}

void mdartstr_state::romdisk_config(device_t *device)
{
	isa16_medalist_rom_disk &boot = *downcast<isa16_medalist_rom_disk *>(device);
	boot.set_rom_tag("romdisk");
}

void mdartstr_state::mdartstr(machine_config &config)
{
	I386SX(config, m_maincpu, 25'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mdartstr_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mdartstr_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("chipset", FUNC(f82c836a_device::int_ack_r));

	F82C836A(config, m_chipset, XTAL(25'000'000), "maincpu", "bios", "keybc", "ram", "isabus");
	m_chipset->hold().set([this] (int state) {
		// halt cpu
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

		// and acknowledge hold
		m_chipset->hlda_w(state);
	});
	m_chipset->nmi().set_inputline("maincpu", INPUT_LINE_NMI);
	m_chipset->intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	m_chipset->cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	m_chipset->a20m().set_inputline("maincpu", INPUT_LINE_A20);
	// isa dma
	m_chipset->ior().set([this] (offs_t offset) -> u16 {
		if (offset < 4)
			return m_isabus->dack_r(offset);
		else
			return m_isabus->dack16_r(offset);
	});
	m_chipset->iow().set([this] (offs_t offset, u16 data) {
		if (offset < 4)
			m_isabus->dack_w(offset, data);
		else
			m_isabus->dack16_w(offset, data);
	});
	m_chipset->tc().set([this] (offs_t offset, u8 data) { m_isabus->eop_w(offset, data); });
	// speaker
	m_chipset->spkr().set([this] (int state) { m_speaker->level_w(state); });

	// 640 + 384 KB
	RAM(config, "ram").set_default_size("1024K");

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(f82c836a_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(f82c836a_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(f82c836a_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(f82c836a_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(f82c836a_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(f82c836a_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(f82c836a_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(f82c836a_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(f82c836a_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(f82c836a_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(f82c836a_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(f82c836a_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(f82c836a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(f82c836a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(f82c836a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(f82c836a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(f82c836a_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(f82c836a_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(f82c836a_device::dreq7_w));

	// all on one backplane
	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa_onboard, "vga",     true);
	ISA16_SLOT(config, "board2", 0, "isabus", pc_isa_onboard, "boot",    true).set_option_machine_config("boot", romdisk_config);;

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(f82c836a_device::kbrst_w));
	// looks unconnected, the BIOS will just use fast A20 exclusively for driving the line
	keybc.gate_a20().set_nop();
//	keybc.gate_a20().set(m_chipset, FUNC(f82c836a_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(f82c836a_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// g80_1500 works with this, ms_naturl don't
	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, nullptr));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( mdartstr )
	ROM_REGION16_LE( 0x80000, "rawbios", 0 )
	// 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "system rev 1.0.bin", 0x000000, 0x080000, CRC(cdd36a31) SHA1(4ced7065e0923d9cb414b65d2a2d955da080c46b) )

	ROM_REGION16_LE( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_COPY( "rawbios", 0x40000, 0x00000, 0x40000 )

	ROM_REGION16_LE( 0x800000, "board2:romdisk", ROMREGION_ERASEFF )
	// TODO: actual socket position filename .ext
	ROM_LOAD("mem_0 rev 3.25.bin",  0x000000, 0x100000,  CRC(8fa930f1) SHA1(a898b180678086c730dc059f14ddd34a334625c7) )
	ROM_LOAD("mem_1 rev 3.25.bin",  0x100000, 0x100000, CRC(8145bd30) SHA1(70d6a1f7e2ca63431396fd923b6d7d2bdabd56e8) )
	// empty socket mem 2
	// empty socket mem 3
	// unsigned 11'025 Hz DAC samples in mem 4/5
	ROM_LOAD("mem_4 rev 2.0.bin",   0x400000, 0x100000, CRC(61adadd7) SHA1(b1705626e0c47ab213f85d74bc5148c0013e0da9) )
	ROM_LOAD("english rev 3.1.bin", 0x500000, 0x100000, CRC(72ed547b) SHA1(57b80dda3996cd75398c06e4bde92491d4d99c14) ) // mem 5 socket
	// empty socket mem 6
	// static RAM in mem 7
ROM_END

} // anonymous namespace

GAME( 2001, mdartstr, 0, mdartstr, 0, mdartstr_state, empty_init, ROT0, "Medalist Marketing", "Dart Star (Rev 3.25)",  MACHINE_NOT_WORKING | MACHINE_REQUIRES_ARTWORK )
