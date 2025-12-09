// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 SuperAMS Memory Expansion Card.

    The card uses a 74LS612 memory mapper in all variants.
    It can be equipped with two unbuffered SRAM circuits of 128K,
    512K, or 2048K. The size of the RAM must be set by a switch on the board.

    SAMS organizes memory in 4 KiB blocks which are mapped into the address
    space by a memory mapper. The mapper can be configured via a sequence of
    addresses at 4000, 4002, ..., 401e, which correspond to memory locations
    0000-0fff, 1000-1fff, ..., f000-ffff. However, due to the memory layout of
    the TI-99/4A, only addresses 2000-3fff and a000-ffff can be used to map
    pages.

    Technical details:

    The card uses a 74LS612 mapper circuit which substitutes the first four bits
    of the logical address by up to 12 bits stored in the respective mapper
    register. The earlier cards were only able to store 8 bits in the mapper
    registers, thus limiting the expansion to 2^(12+8) = 1 MiB. This is mainly
    due to the 8-bit width of the external data bus.

    The latest version of the card uses a latch to expand the address prefix
    to 10 bits (4 MiB). When a 16-bit word is stored at a word address n,
    the TI console first sends the lower 8 bits to the address n+1, then the
    upper 8 bits to address n. The card picks up the bits written to n+1 and
    keeps them in a latch, which then delivers those bits to the mapper when
    the upper byte is written to n.

    Example:   LI  R0,>A002
               MOV R0,@>4000

    This stores the value 0x02 at the address 0x4001, which means it is held
    in the latch, and 0xa0 at the address 0x4000, which is routed to
    the lower 8 bit of the mapper register. At the same time, the latch
    delivers the rightmost two bits to the upper two register bits of the
    mapper. However, reading the full value out of the register is not
    supported.

    The SAMS does not decode AMA/AMB/AMC. This would be relevant for
    use in a Geneve system, but the SAMS card will not run with a Geneve
    anyway, since the word transfer address sequence is even/odd
    instead of odd/even which is assumed for the latch operation.

    History:

    According to a software distribution disk from the South West 99ers group,
    the predecessor of this card was the Asgard Expanded Memory System (AEMS).
    Although some documentation and software was available for it, it was never
    built. Instead, a simpler memory card called the Asgard Memory System (AMS)
    was built. The South West 99ers group built a better version of this card
    called the Super AMS. Any documentation and software containing a reference
    to the AEMS are applicable to either AMS or SAMS.

    Michael Zapf
    Updated June 2025

*****************************************************************************/

#include "emu.h"
#include "samsmem.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_SAMSMEM, bus::ti99::peb::sams_memory_expansion_device, "ti99_sams", "SuperAMS memory expansion card")

namespace bus::ti99::peb {

#define CRULATCH_TAG "crulatch"
#define MAPPER_TAG "mapper_u12"
#define RAM_U13_TAG "ram_u13"
#define RAM_U14_TAG "ram_u14"

static constexpr uint16_t SAMS_CRU_BASE = 0x1e00;

sams_memory_expansion_device::sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_SAMSMEM, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_crulatch_u8(*this, CRULATCH_TAG),
	m_mapper_u12(*this, MAPPER_TAG),
	m_ram_u13(*this, RAM_U13_TAG),
	m_ram_u14(*this, RAM_U14_TAG),
	m_select_bit(0x80),
	m_upper_bits(0)
{
}

line_state sams_memory_expansion_device::memsel(offs_t offset)
{
	return (((offset & 0xe000)==0x2000) || ((offset & 0xe000)==0xa000) || ((offset & 0xc000)==0xc000))?
		ASSERT_LINE : CLEAR_LINE;
}

line_state sams_memory_expansion_device::mapsel(offs_t offset)
{
	return (((offset & 0xe000)==0x4000) & (m_crulatch_u8->q0_r()==1))? ASSERT_LINE : CLEAR_LINE;
}

/*
    Memory read. The SAMS card has two address areas: The memory is at locations
    0x2000-0x3fff and 0xa000-0xffff, and the mapper area is at 0x4000-0x401f

    The smaller cards only use the even addresses in 4000-401f.

    The larger cards (beyond 1M) use the odd addresses to add more leading bits
    to the register value. This value is stored in another latch.

    4000: (D4 D5 D6 D7 D8 D9 D10 D11)
    4001: ( 0  0  0  0  0  0   0   0)     (up to 1024K)
    4001: ( 0  0  0  0  0  0  D2  D3)     (up to 4096K)
*/
void sams_memory_expansion_device::readz(offs_t offset, uint8_t *value)
{
	int mask = m_select_bit - 1;

	bool ram1_sel = ((m_mapper_u12->get_mapper_output((offset>>12)&0x0f) & m_select_bit)==0) && (memsel(offset)==ASSERT_LINE);
	bool ram2_sel = !ram1_sel && (memsel(offset)==ASSERT_LINE);

	offs_t prefix = ((m_mapper_u12->get_mapper_output((offset >> 12) & 0x0f)) & mask) << 12;

	if (mapsel(offset) == ASSERT_LINE)
		*value = m_mapper_u12->get_register((offset>>1) & 0xf) & 0xff;

	if (ram1_sel)
		*value = m_ram_u13->pointer()[prefix | (offset & 0x0fff)];

	if (ram2_sel)
		*value = m_ram_u14->pointer()[prefix | (offset & 0x0fff)];
}

void sams_memory_expansion_device::write(offs_t offset, uint8_t data)
{
	int mask = m_select_bit - 1;

	bool ram1_sel = ((m_mapper_u12->get_mapper_output((offset>>12)&0x0f) & m_select_bit)==0) && (memsel(offset)==ASSERT_LINE);
	bool ram2_sel = !ram1_sel && (memsel(offset)==ASSERT_LINE);

	offs_t prefix = ((m_mapper_u12->get_mapper_output((offset >> 12) & 0x0f)) & mask) << 12;

	if (mapsel(offset) == ASSERT_LINE)
	{
		// Actually, we should use a proper ls373 circuit emulation, but
		// for now, a simple variable may suffice.
		if (offset & 1)
			m_upper_bits = data & 0x03;
		else
			m_mapper_u12->set_register((offset>>1) & 0xf, ((m_upper_bits & 0x03) << 8) | data);
	}

	if (ram1_sel)
		m_ram_u13->pointer()[prefix | (offset & 0x0fff)] = data;

	if (ram2_sel)
		m_ram_u14->pointer()[prefix | (offset & 0x0fff)] = data;
}

/*
    CRU write. Turns on the mapper and allows to change it.
*/
void sams_memory_expansion_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==SAMS_CRU_BASE)
		m_crulatch_u8->write_bit((offset>>1) & 0x0f, data);
}

void sams_memory_expansion_device::reset_in(int state)
{
	m_crulatch_u8->clear_w(state);
}

void sams_memory_expansion_device::device_add_mconfig(machine_config &config)
{
	RAM(config, RAM_U13_TAG).set_default_size("2M").set_default_value(0);
	RAM(config, RAM_U14_TAG).set_default_size("2M").set_default_value(0);
	TTL74612(config, MAPPER_TAG);

	LS259(config, m_crulatch_u8);
	m_crulatch_u8->q_out_cb<1>().set(m_mapper_u12, FUNC(ttl74612_device::map_mode_w));
}

void sams_memory_expansion_device::device_start()
{
	save_item(NAME(m_upper_bits));
}

void sams_memory_expansion_device::device_reset()
{
	m_select_bit = 0x200 >> ((3-ioport("SAMSMEM")->read())*2);
}

INPUT_PORTS_START( samsmem )
	PORT_START( "SAMSMEM" )
	PORT_CONFNAME( 0x03, 0x02, "Memory circuits" )
		PORT_CONFSETTING( 0x01, "2 x 128K SRAM" )
		PORT_CONFSETTING( 0x02, "2 x 512K SRAM" )
		PORT_CONFSETTING( 0x03, "2 x 2048K SRAM" )
INPUT_PORTS_END

ioport_constructor sams_memory_expansion_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(samsmem);
}

} // end namespace bus::ti99::peb
