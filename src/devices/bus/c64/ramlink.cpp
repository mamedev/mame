// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD RAMLink emulation

**********************************************************************/

#include "emu.h"
#include "ramlink.h"

#include "georam.h"
#include "reu.h"

#include <algorithm>

#include "ramlink.lh"



namespace {

struct ramcard_config
{
	bool trace_cut;
	uint8_t simm[4];
};

constexpr ramcard_config RAMCARD[] =
{
	{ false, { 0, 0, 0, 0 } },
	{ false, { 1, 0, 0, 0 } },
	{ false, { 1, 1, 0, 0 } },
	{ false, { 1, 1, 1, 0 } },
	{ false, { 1, 1, 1, 1 } },
	{ true,  { 4, 0, 0, 0 } },
	{ true,  { 4, 1, 0, 0 } },
	{ true,  { 4, 4, 0, 0 } },
	{ true,  { 4, 4, 1, 0 } },
	{ true,  { 4, 4, 4, 0 } },
	{ true,  { 4, 4, 4, 1 } },
	{ true,  { 4, 4, 4, 4 } }
};

void ramlink_ramport_cards(device_slot_interface &device)
{
	device.option_add("georam", C64_GEORAM);
	device.option_add("reu1700", C64_REU1700);
	device.option_add("reu1750", C64_REU1750);
	device.option_add("reu1764", C64_REU1764);
}

void ramport_backup(device_t *device)
{
	device_nvram_interface *nvram;
	if (device->interface(nvram))
		nvram->nvram_enable_backup(true);
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_RAMLINK, c64_ramlink_cartridge_device, "c64_ramlink", "CMD RAMLink")


//-------------------------------------------------
//  ROM( c64_ramlink )
//-------------------------------------------------

ROM_START( c64_ramlink )
	ROM_REGION( 0x10000, "rom", 0 )
	ROM_DEFAULT_BIOS("v201")
	ROM_SYSTEM_BIOS( 0, "v140", "RL DOS v1.40" )
	ROMX_LOAD( "cmd_ramlink_dos_v140.bin", 0x0000, 0x10000, CRC(31d1c877) SHA1(fe536cb7a14c8ecbac55ef2799267855f1447d9f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v201", "RL DOS v2.01" )
	ROMX_LOAD( "ramlink1.bin", 0x0000, 0x8000, CRC(da953716) SHA1(adde7308c7155fc1942c98b674189e89f4965f91), ROM_BIOS(1) )
	ROMX_LOAD( "ramlink2.bin", 0x8000, 0x8000, CRC(ab8b6dda) SHA1(e35e776387bd8b9ab16bacff70c2a3f3ef9b65bd), ROM_BIOS(1) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "rl-701_037d.u2", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "rl-701_102b.u2", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "rl-701_017c.bin", 0x000, 0x001, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_ramlink_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_ramlink );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_ramlink_cartridge_device::device_add_mconfig(machine_config &config)
{
	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set([this] () { return cmdpar_data_r(); });
	m_ppi->out_pa_callback().set([this] (uint8_t data) { cmdpar_data_w(data); });
	m_ppi->in_pb_callback().set_constant(0xff);
	m_ppi->out_pb_callback().set(FUNC(c64_ramlink_cartridge_device::ppi_pb_w));
	m_ppi->in_pc_callback().set(FUNC(c64_ramlink_cartridge_device::ppi_pc_r));
	m_ppi->out_pc_callback().set([this] (uint8_t data) { m_rom_bank = data & 0x03; });

	RTC72421(config, m_rtc, 32.768_kHz_XTAL);

	C64_EXPANSION_SLOT(config, m_ramport, DERIVED_CLOCK(1, 1), ramlink_ramport_cards, nullptr);
	for (char const *option : { "georam", "reu1700", "reu1750", "reu1764" })
		m_ramport->set_option_machine_config(option, ramport_backup);
	m_ramport->irq_callback().set(FUNC(c64_ramlink_cartridge_device::irq_w<0>));
	m_ramport->nmi_callback().set(FUNC(c64_ramlink_cartridge_device::nmi_w<0>));
	m_ramport->reset_callback().set(FUNC(c64_ramlink_cartridge_device::reset_w<0>));
	m_ramport->cd_input_callback().set(FUNC(c64_ramlink_cartridge_device::dma_cd_r));
	m_ramport->cd_output_callback().set(FUNC(c64_ramlink_cartridge_device::dma_cd_w));
	m_ramport->dma_callback().set(FUNC(c64_ramlink_cartridge_device::dma_w<0>));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->irq_callback().set(FUNC(c64_ramlink_cartridge_device::irq_w<1>));
	m_exp->nmi_callback().set(FUNC(c64_ramlink_cartridge_device::nmi_w<1>));
	m_exp->reset_callback().set(FUNC(c64_ramlink_cartridge_device::reset_w<1>));
	m_exp->cd_input_callback().set(FUNC(c64_ramlink_cartridge_device::dma_cd_r));
	m_exp->cd_output_callback().set(FUNC(c64_ramlink_cartridge_device::dma_cd_w));
	m_exp->dma_callback().set(FUNC(c64_ramlink_cartridge_device::dma_w<1>));

	config.set_default_layout(layout_ramlink);
}


//-------------------------------------------------
//  INPUT_PORTS( c64_ramlink )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_ramlink_cartridge_device::switch_changed )
{
	m_enabled = BIT(m_switches->read(), 0);
	m_direct = BIT(m_switches->read(), 1);

	update_leds();
}

INPUT_CHANGED_MEMBER( c64_ramlink_cartridge_device::swap_changed )
{
	if (!newval)
		m_swap = BIT(m_swap, param) ? 0 : (1 << param);

	update_leds();
}

INPUT_CHANGED_MEMBER( c64_ramlink_cartridge_device::reset_changed )
{
	reset_w<2>(newval);
}

static INPUT_PORTS_START( c64_ramlink )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0xf0, 0xb0, "RAMCard" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x10, "1 MB" )
	PORT_CONFSETTING(    0x20, "2 MB" )
	PORT_CONFSETTING(    0x30, "3 MB" )
	PORT_CONFSETTING(    0x40, "4 MB (4x 1 MB SIMM)" )
	PORT_CONFSETTING(    0x50, "4 MB (1x 4 MB SIMM)" )
	PORT_CONFSETTING(    0x60, "5 MB" )
	PORT_CONFSETTING(    0x70, "8 MB" )
	PORT_CONFSETTING(    0x80, "9 MB" )
	PORT_CONFSETTING(    0x90, "12 MB" )
	PORT_CONFSETTING(    0xa0, "13 MB" )
	PORT_CONFSETTING(    0xb0, "16 MB" )

	PORT_START("SWITCHES")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_TOGGLE PORT_NAME("RAMLink ENABLE/DISABLE") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_ramlink_cartridge_device::switch_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("RAMLink NORMAL/DIRECT") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_ramlink_cartridge_device::switch_changed), 0)

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RAMLink SWAP 8") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_ramlink_cartridge_device::swap_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RAMLink SWAP 9") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_ramlink_cartridge_device::swap_changed), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RAMLink RESET") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_ramlink_cartridge_device::reset_changed), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_ramlink_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_ramlink );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ramlink_cartridge_device - constructor
//-------------------------------------------------

c64_ramlink_cartridge_device::c64_ramlink_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_RAMLINK, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_cmd_parallel_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ppi(*this, "i8255"),
	m_rtc(*this, "rtc"),
	m_ramport(*this, "ramport"),
	m_exp(*this, "exp"),
	m_rom(*this, "rom"),
	m_config(*this, "CONFIG"),
	m_switches(*this, "SWITCHES"),
	m_leds(*this, "led%u", 0U),
	m_ram(*this, "ram", 0x2000, ENDIANNESS_LITTLE),
	m_card(*this, "card", 0x100'0000, ENDIANNESS_LITTLE),
	m_enabled(true),
	m_direct(false),
	m_ramcard(0),
	m_on(false),
	m_dos(false),
	m_trap(false),
	m_io1_mode(3),
	m_ram_page(0),
	m_card_addr(0),
	m_rom_bank(3),
	m_ppi_pb(0xff),
	m_mapped(false),
	m_swap(0),
	m_c128(false),
	m_boot(true),
	m_mmu_cr(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ramlink_cartridge_device::device_start()
{
	std::fill(std::begin(m_irq), std::end(m_irq), 0);
	std::fill(std::begin(m_nmi), std::end(m_nmi), 0);
	std::fill(std::begin(m_dma), std::end(m_dma), 0);
	std::fill(std::begin(m_reset), std::end(m_reset), 1);
	std::fill(std::begin(m_mmu_pcr), std::end(m_mmu_pcr), 0);

	save_item(NAME(m_enabled));
	save_item(NAME(m_direct));
	save_item(NAME(m_ramcard));
	save_item(NAME(m_on));
	save_item(NAME(m_dos));
	save_item(NAME(m_trap));
	save_item(NAME(m_io1_mode));
	save_item(NAME(m_ram_page));
	save_item(NAME(m_card_addr));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ppi_pb));
	save_item(NAME(m_mapped));
	save_item(NAME(m_swap));
	save_item(NAME(m_c128));
	save_item(NAME(m_boot));
	save_item(NAME(m_mmu_cr));
	save_item(NAME(m_mmu_pcr));
	save_item(NAME(m_irq));
	save_item(NAME(m_nmi));
	save_item(NAME(m_dma));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ramlink_cartridge_device::device_reset()
{
	m_enabled = BIT(m_switches->read(), 0);
	m_direct = BIT(m_switches->read(), 1);
	m_ramcard = m_config->read() >> 4;

	switch_off();

	m_io1_mode = 3;
	m_ram_page = 0;
	m_card_addr = 0;
	m_mapped = false;
	m_swap = 0;

	m_c128 = false;
	m_boot = true;
	m_mmu_cr = 0;
	std::fill(std::begin(m_mmu_pcr), std::end(m_mmu_pcr), 0);
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void c64_ramlink_cartridge_device::nvram_default()
{
	std::fill_n(m_ram.target(), m_ram.bytes(), 0);
	std::fill_n(m_card.target(), m_card.bytes(), 0);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool c64_ramlink_cartridge_device::nvram_read(util::read_stream &file)
{
	auto const [err1, actual1] = read(file, m_ram.target(), m_ram.bytes());
	if (err1 || (actual1 != m_ram.bytes()))
		return false;

	auto const [err2, actual2] = read(file, m_card.target(), m_card.bytes());
	return !err2 && (actual2 == m_card.bytes());
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool c64_ramlink_cartridge_device::nvram_write(util::write_stream &file)
{
	auto const [err1, actual1] = write(file, m_ram.target(), m_ram.bytes());
	if (err1)
		return false;

	auto const [err2, actual2] = write(file, m_card.target(), m_card.bytes());
	return !err2;
}


//-------------------------------------------------
//  switch_off - $DF7F
//-------------------------------------------------

void c64_ramlink_cartridge_device::switch_off()
{
	m_on = false;
	m_dos = false;
	m_trap = false;

	update_leds();
}


//-------------------------------------------------
//  update_leds -
//-------------------------------------------------

void c64_ramlink_cartridge_device::update_leds()
{
	m_leds[0] = m_enabled && BIT(m_ppi_pb, 4);
	m_leds[1] = m_enabled && BIT(m_ppi_pb, 3);
	m_leds[2] = active();
	m_leds[3] = m_enabled && BIT(m_swap, 0);
	m_leds[4] = m_enabled && BIT(m_swap, 1);
}


//-------------------------------------------------
//  card_installed -
//-------------------------------------------------

bool c64_ramlink_cartridge_device::card_installed() const
{
	return RAMCARD[m_ramcard].simm[0] != 0;
}


//-------------------------------------------------
//  card_offset - RAMCard SIMM decoding, ~0 for
//  an empty socket
//-------------------------------------------------

offs_t c64_ramlink_cartridge_device::card_offset(offs_t offset) const
{
	ramcard_config const &card = RAMCARD[m_ramcard];
	uint32_t const addr = m_card_addr | (offset & 0xff);

	if (!card.trace_cut)
	{
		int const socket = BIT(addr, 20, 2);

		return card.simm[socket] ? ((socket << 20) | (addr & 0xf'ffff)) : ~offs_t(0);
	}

	int const socket = BIT(addr, 22, 2);

	switch (card.simm[socket])
	{
	case 4: return (socket << 22) | (addr & 0x3f'ffff);
	case 1: return (socket << 22) | (addr & 0xf'ffff);
	default: return ~offs_t(0);
	}
}


//-------------------------------------------------
//  io1_internal - I/O1 serviced by internal RAM
//  or RAMCard
//-------------------------------------------------

bool c64_ramlink_cartridge_device::io1_internal() const
{
	return active() && ((m_io1_mode == 0) || ((m_io1_mode == 1) && card_installed()));
}


//-------------------------------------------------
//  io2_decoded - I/O2 address decoded by RAMLink
//-------------------------------------------------

bool c64_ramlink_cartridge_device::io2_decoded(offs_t offset, int rw) const
{
	if (!m_enabled)
		return false;

	uint8_t const addr = offset & 0xff;

	if (!rw && ((addr == 0x7e) || (addr == 0x7f)))
		return true;

	if (!m_on)
		return false;

	if (rw)
		return ((addr >= 0x40) && (addr <= 0x43)) || ((addr >= 0xb0) && (addr <= 0xbf));

	return ((addr >= 0x20) && (addr <= 0x22)) || ((addr >= 0x40) && (addr <= 0x43)) || (addr == 0x60) || (addr == 0x70) ||
		((addr >= 0x80) && (addr <= 0xa3)) || ((addr >= 0xb0) && (addr <= 0xbf)) || ((addr >= 0xc0) && (addr <= 0xc3));
}


//-------------------------------------------------
//  cable_access - 8255 access
//-------------------------------------------------

bool c64_ramlink_cartridge_device::cable_access(offs_t offset, int io2) const
{
	return !io2 && active() && ((offset & 0xfc) == 0x40);
}


//-------------------------------------------------
//  c128_kernal - 128 mode KERNAL decoding, leaving
//  the MMU and the regional tables to the C128
//-------------------------------------------------

bool c64_ramlink_cartridge_device::c128_kernal(offs_t offset) const
{
	if ((offset >= 0xff00) && (offset < 0xff10))
		return false;

	if (offset >= 0xfff0)
		return false;

	if (m_on)
		return true;

	return !(((offset >= 0xeb00) && (offset < 0xed00)) || ((offset >= 0xf7a0) && (offset < 0xf7b0)) ||
		((offset >= 0xfd00) && (offset < 0xff00)) || ((offset >= 0xff50) && (offset < 0xff60)));
}


//-------------------------------------------------
//  kernal_mapped - C64 mode Ultimax mapping, which
//  overrides 8K and 16K pass-thru cartridges but
//  yields to an Ultimax one
//-------------------------------------------------

bool c64_ramlink_cartridge_device::kernal_mapped(offs_t offset, int sphi2, int ba, int rw, int game, int exrom) const
{
	if (!m_enabled || m_c128 || !sphi2 || !ba || !rw || (!game && exrom))
		return false;

	if ((offset >= 0x8000) && (offset < 0xc000))
		return m_dos;

	if (offset >= 0xe000)
		return (m_on && ((offset < 0xff00) || ((offset >= 0xff10) && (offset < 0xfff0)))) || m_slot->hiram();

	return false;
}


//-------------------------------------------------
//  mmu_snoop - follow the C128 MMU configuration
//-------------------------------------------------

void c64_ramlink_cartridge_device::mmu_snoop(offs_t offset, uint8_t data)
{
	if ((offset >= 0xd500) && (offset < 0xd506))
	{
		if (BIT(m_mmu_cr, 0))
			return;

		if (offset == 0xd500)
			m_mmu_cr = data & 0x7f;
		else if (offset < 0xd505)
			m_mmu_pcr[offset - 0xd501] = data & 0x7f;
		else if (BIT(data, 6))
			m_c128 = false;
	}
	else if (offset == 0xff00)
	{
		m_mmu_cr = data & 0x7f;

		if (m_boot)
			m_c128 = true;
	}
	else if ((offset > 0xff00) && (offset < 0xff05))
	{
		m_mmu_cr = m_mmu_pcr[offset - 0xff01];
	}
}


//-------------------------------------------------
//  ramport_strobes - I/O1/I/O2 on the RAM port
//-------------------------------------------------

void c64_ramlink_cartridge_device::ramport_strobes(offs_t offset, int rw, int io1, int io2, int &rp_io1, int &rp_io2) const
{
	bool const ext_io1 = !io1 && !io1_internal();
	bool const trap = !rw && active() && m_trap && ((offset & 0xff) == 0x21);

	rp_io1 = !(ext_io1 && (m_direct || (active() && (m_io1_mode == 2))));
	rp_io2 = !(!io2 && ((!io2_decoded(offset, rw) && (m_direct || active())) || trap));
}


//-------------------------------------------------
//  exp_strobes - I/O1/I/O2 on the pass-thru port
//-------------------------------------------------

void c64_ramlink_cartridge_device::exp_strobes(offs_t offset, int io1, int io2, int &exp_io1, int &exp_io2) const
{
	bool const ext_io1 = !io1 && !io1_internal();

	exp_io1 = !(ext_io1 && !m_direct && !(active() && (m_io1_mode == 2)));
	exp_io2 = !(!io2 && !m_direct);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_ramlink_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (cable_access(offset, io2) && !cmdpar_sample_ready())
		return data;

	int rp_io1, rp_io2, exp_io1, exp_io2;
	ramport_strobes(offset, 1, io1, io2, rp_io1, rp_io2);
	exp_strobes(offset, io1, io2, exp_io1, exp_io2);

	if ((offset == 0xfffc) && !machine().side_effects_disabled())
		m_boot = false;

	data = m_ramport->cd_r(offset, data, sphi2, ba, 1, 1, rp_io1, rp_io2);
	data = m_exp->cd_r(offset, data, sphi2, ba, m_mapped ? 1 : roml, m_mapped ? 1 : romh, exp_io1, exp_io2);

	if (c128_mode())
	{
		if (sphi2 && ba)
		{
			uint8_t const hi = BIT(m_mmu_cr, 4, 2);
			uint8_t const mid = BIT(m_mmu_cr, 2, 2);

			if ((offset >= 0xe000) && (hi != 3) && c128_kernal(offset))
				data = m_rom[(m_on ? 0xc000 : 0xe000) | (offset & 0x1fff)];
			else if ((offset >= 0x8000) && (offset < 0xc000) && m_dos && ((mid == 1) || (mid == 2)))
				data = m_rom[(m_rom_bank << 14) | (offset & 0x3fff)];
		}
	}
	else if (m_mapped)
	{
		if (!roml)
		{
			data = m_rom[(m_rom_bank << 14) | (offset & 0x1fff)];
		}
		else if ((offset >= 0xa000) && (offset < 0xc000))
		{
			data = m_rom[(m_rom_bank << 14) | 0x2000 | (offset & 0x1fff)];
		}
		else if (!romh)
		{
			bool const switched = m_on && ((offset < 0xff00) || ((offset >= 0xff10) && (offset < 0xfff0)));

			data = m_rom[(switched ? 0x8000 : 0xa000) | (offset & 0x1fff)];
		}
	}

	if (!io1 && io1_internal())
	{
		if (m_io1_mode == 0)
		{
			data = m_ram[m_ram_page | (offset & 0xff)];
		}
		else
		{
			offs_t const addr = card_offset(offset);

			if (addr != ~offs_t(0))
				data = m_card[addr];
		}
	}
	else if (!io2 && active())
	{
		uint8_t const addr = offset & 0xff;

		if ((addr >= 0x40) && (addr <= 0x43))
			data = m_ppi->read(addr & 0x03);
		else if ((addr >= 0xb0) && (addr <= 0xbf))
			data = m_rtc->read(addr & 0x0f);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ramlink_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (cable_access(offset, io2) && !cmdpar_sample_ready())
		return;

	int rp_io1, rp_io2, exp_io1, exp_io2;
	ramport_strobes(offset, 0, io1, io2, rp_io1, rp_io2);
	exp_strobes(offset, io1, io2, exp_io1, exp_io2);

	m_ramport->cd_w(offset, data, sphi2, ba, 1, 1, rp_io1, rp_io2);
	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, exp_io1, exp_io2);

	if (!m_dma[0] && !m_dma[1])
		mmu_snoop(offset, data);

	if (!io1 && io1_internal())
	{
		if (m_io1_mode == 0)
		{
			m_ram[m_ram_page | (offset & 0xff)] = data;
		}
		else
		{
			offs_t const addr = card_offset(offset);

			if (addr != ~offs_t(0))
				m_card[addr] = data;
		}
	}
	else if (!io2 && m_enabled)
	{
		uint8_t const addr = offset & 0xff;

		if (addr == 0x7e)
		{
			m_on = true;
			update_leds();
		}
		else if (addr == 0x7f)
		{
			switch_off();
		}
		else if (m_on)
		{
			if (addr == 0x20)
				m_trap = false;
			else if (addr == 0x22)
				m_trap = true;
			else if ((addr >= 0x40) && (addr <= 0x43))
				m_ppi->write(addr & 0x03, data);
			else if (addr == 0x60)
				m_dos = true;
			else if (addr == 0x70)
				m_dos = false;
			else if ((addr >= 0x80) && (addr <= 0x9f))
				m_ram_page = (addr & 0x1f) << 8;
			else if (addr == 0xa0)
				m_card_addr = (m_card_addr & 0xff0000) | (data << 8);
			else if (addr == 0xa1)
				m_card_addr = (m_card_addr & 0x00ff00) | (data << 16);
			else if ((addr >= 0xb0) && (addr <= 0xbf))
				m_rtc->write(addr & 0x0f, data);
			else if ((addr >= 0xc0) && (addr <= 0xc3))
				m_io1_mode = addr & 0x03;
		}
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_ramlink_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	int const game = m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
	int const exrom = m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());

	m_mapped = kernal_mapped(offset, sphi2, ba, rw, game, exrom);

	return m_mapped ? 0 : game;
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_ramlink_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	int const game = m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
	int const exrom = m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());

	m_mapped = kernal_mapped(offset, sphi2, ba, rw, game, exrom);

	return m_mapped ? 1 : exrom;
}


//-------------------------------------------------
//  host bus signals to both ports
//-------------------------------------------------

void c64_ramlink_cartridge_device::c64_ba_w(int state)
{
	m_ramport->ba_w(state);
	m_exp->ba_w(state);
}

void c64_ramlink_cartridge_device::c64_phi2_w(int state)
{
	m_ramport->phi2_w(state);
	m_exp->phi2_w(state);
}

void c64_ramlink_cartridge_device::c64_irq_w(int state)
{
	m_ramport->host_irq_w(state);
	m_exp->host_irq_w(state);
}

void c64_ramlink_cartridge_device::c64_nmi_w(int state)
{
	m_ramport->host_nmi_w(state);
	m_exp->host_nmi_w(state);
}


//-------------------------------------------------
//  port signals to the host bus
//-------------------------------------------------

template <unsigned N>
void c64_ramlink_cartridge_device::irq_w(int state)
{
	bool const old = m_irq[0] || m_irq[1];

	m_irq[N] = state ? 1 : 0;

	bool const irq = m_irq[0] || m_irq[1];

	if (irq != old)
		m_slot->irq_w(irq);
}

template <unsigned N>
void c64_ramlink_cartridge_device::nmi_w(int state)
{
	bool const old = m_nmi[0] || m_nmi[1];

	m_nmi[N] = state ? 1 : 0;

	bool const nmi = m_nmi[0] || m_nmi[1];

	if (nmi != old)
		m_slot->nmi_w(nmi);
}

template <unsigned N>
void c64_ramlink_cartridge_device::dma_w(int state)
{
	bool const old = m_dma[0] || m_dma[1];

	m_dma[N] = state ? 1 : 0;

	bool const dma = m_dma[0] || m_dma[1];

	if (dma != old)
		m_slot->dma_w(dma);
}

template <unsigned N>
void c64_ramlink_cartridge_device::reset_w(int state)
{
	bool const old = m_reset[0] && m_reset[1] && m_reset[2];

	m_reset[N] = state ? 1 : 0;

	bool const reset = m_reset[0] && m_reset[1] && m_reset[2];

	if (reset != old)
		m_slot->reset_w(reset);
}

uint8_t c64_ramlink_cartridge_device::dma_cd_r(offs_t offset)
{
	return m_slot->dma_cd_r(offset);
}

void c64_ramlink_cartridge_device::dma_cd_w(offs_t offset, uint8_t data)
{
	m_slot->dma_cd_w(offset, data);
}


//-------------------------------------------------
//  ppi_pb_w -
//-------------------------------------------------

void c64_ramlink_cartridge_device::ppi_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       128/_64 MODE
	    1       SWAP 8
	    2       SWAP 9
	    3       ERROR LED
	    4       ACTIVITY LED
	    5       _PATN
	    6       _PCLK
	    7       _PRDY

	*/

	m_ppi_pb = data;

	update_leds();

	cmdpar_lines_w((BIT(data, 7) ? PRDY : 0) | (BIT(data, 6) ? PCLK : 0) | (BIT(data, 5) ? PATN : 0) | PEXT);
}


//-------------------------------------------------
//  ppi_pc_r -
//-------------------------------------------------

uint8_t c64_ramlink_cartridge_device::ppi_pc_r()
{
	/*

	    bit     description

	    0       ROM A14
	    1       ROM A15
	    2
	    3
	    4       _SWAP 8
	    5       _SWAP 9
	    6       PCLK
	    7       PRDY

	*/

	uint8_t const lines = cmdpar_lines_r();

	return ((lines & PRDY) ? 0x00 : 0x80) | ((lines & PCLK) ? 0x00 : 0x40) | (BIT(m_swap, 1) ? 0x00 : 0x20) | (BIT(m_swap, 0) ? 0x00 : 0x10) | 0x0f;
}
