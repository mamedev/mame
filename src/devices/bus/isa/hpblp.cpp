// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

 HP Basic Language Coprocessor (82321A)

***************************************************************************/

#include "emu.h"
#include "hpblp.h"

//#define VERBOSE 1
#include "logmacro.h"

#define BLP_TAG "blpcpu"
#define TMS_TAG "tms9914"

DEFINE_DEVICE_TYPE(HPBLP, isa8_hpblp_device, "hpblp", "HP Basic Language Coprocessor")

static INPUT_PORTS_START(hpblp)
	PORT_START("BLPPORT")
	PORT_DIPNAME(3, 0, "IO Address") PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(0, "250h-257h")
	PORT_DIPSETTING(1, "280h-287h")
	PORT_DIPSETTING(2, "330h-337h")
	PORT_DIPSETTING(3, "390h-397h")

	PORT_START("BLPIRQ")
	PORT_DIPNAME(7, 3, "Interrupt") PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(0x0, "IRQ 3")
	PORT_DIPSETTING(0x1, "IRQ 4")
	PORT_DIPSETTING(0x2, "IRQ 5")
	PORT_DIPSETTING(0x3, "IRQ 7")
	PORT_DIPSETTING(0x4, "IRQ 9")
INPUT_PORTS_END

ioport_constructor isa8_hpblp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hpblp);
}


isa8_hpblp_device::isa8_hpblp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HPBLP, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_maincpu(*this, BLP_TAG),
	m_tms9914(*this, TMS_TAG),
	m_ieee488(*this, IEEE488_TAG),
	m_timer_10ms(nullptr),
	m_reset(false)
{
}

void isa8_hpblp_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &isa8_hpblp_device::m68map);

	TMS9914(config, m_tms9914, XTAL(5'000'000));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->int_write_cb().set(FUNC(isa8_hpblp_device::gpib_irq));

	IEEE488(config, m_ieee488, 0);
	m_ieee488->eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	m_ieee488->dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	m_ieee488->nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	m_ieee488->ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	m_ieee488->ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	m_ieee488->srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	m_ieee488->atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	m_ieee488->ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));

	ieee488_slot_device &slot0(IEEE488_SLOT(config, "ieee0", 0));
	hp_ieee488_devices(slot0);
	slot0.set_default_option("hp9122c");

	bus::hp_dio::dio16_device &dio16(DIO16(config, "diobus", 0));
	dio16.set_program_space(m_maincpu, AS_PROGRAM);
	m_maincpu->reset_cb().set(dio16, FUNC(bus::hp_dio::dio16_device::reset_in));

	dio16.irq1_out_cb().set_inputline(m_maincpu, M68K_IRQ_1);
	dio16.irq2_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	dio16.irq3_out_cb().set_inputline(m_maincpu, M68K_IRQ_3);
	dio16.irq4_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	dio16.irq5_out_cb().set_inputline(m_maincpu, M68K_IRQ_5);
	dio16.irq6_out_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	dio16.irq7_out_cb().set_inputline(m_maincpu, M68K_IRQ_7);

	DIO32_SLOT(config, "sl0", 0, "diobus", dio16_cards, nullptr, false);
	DIO32_SLOT(config, "sl1", 0, "diobus", dio16_cards, nullptr, false);
}

offs_t isa8_hpblp_device::get_bus_address(offs_t offset, uint16_t mem_mask)
{
	if (mem_mask == 0xff)
		offset++;
	return offset;
}

bool isa8_hpblp_device::forward_to_host(offs_t offset)
{
	if (offset >= 0x200000 && offset <= 0x2fffff)
		return true;
	if (offset >= 0x420000 && offset <= 0x42ffff)
		return true;
	if (offset >= 0x510000 && offset <= 0x51ffff)
		return true;
	if (offset >= 0x520000 && offset <= 0x52ffff)
		return true;
	if (offset >= 0x538000 && offset <= 0x53ffff)
		return true;
	if (offset >= 0x5f2000 && offset <= 0x5f3fff)
		return true;
	if (offset >= 0x690000 && offset <= 0x69ffff)
		return true;
	if (offset >= 0x6f0000 && offset <= 0x6fffff)
		return true;
	if (offset >= 0x730000 && offset <= 0x73ffff)
		return true;
	if (offset >= 0x7a0000 && offset <= 0x7affff)
		return true;
	return false;
}

uint8_t isa8_hpblp_device::status_val(offs_t offset)
{
	if (offset >= 0x200000 && offset <= 0x2fffff)
		return 0x30;
	if (offset >= 0x512000 && offset <= 0x517fff)
		return 0x70;
	if (offset >= 0x51ff00 && offset <= 0x51ffff)
		return 0x50;
	if (offset >= 0x538000 && offset <= 0x53ffff)
		return 0xb0;
	return 0xd0;
}

uint16_t isa8_hpblp_device::bus_r(offs_t offset, uint16_t mem_mask)
{
	offset <<= 1;

	if (machine().side_effects_disabled())
		return m_bus_data;

	if (!forward_to_host(offset)) {
		LOG("%s: %06x -> BERR\n", __func__, offset);
		m_maincpu->trigger_bus_error();
		return 0xffff;
	}

	if (m_ack_buscycle) {
		m_ack_buscycle = false;
		m_bus_read = false;
		m_bus_address = 0;
		LOG("%s: %06x %04x (%04x)\n", __func__, offset, m_bus_data, mem_mask);
		return m_bus_data;
	} else {
		m_bus_address = get_bus_address(offset, mem_mask);
		m_bus_mem_mask = mem_mask;
		m_bus_read = true;
	}
	m_maincpu->defer_access();
	return 0xffff;
}

void isa8_hpblp_device::bus_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;

	if (!forward_to_host(offset)) {
		LOG("%s: %06x -> BERR\n", __func__, offset);
		m_maincpu->trigger_bus_error();
		return;
	}

	if (m_ack_buscycle) {
		m_ack_buscycle = false;
		m_bus_write = false;
		m_bus_address = 0;
		return;
	}

	if (!m_bus_write) {
		m_bus_address = get_bus_address(offset, mem_mask);
		m_bus_mem_mask = mem_mask;
		m_bus_write = true;
		m_bus_data = data;
	}
	m_maincpu->defer_access();
}

void isa8_hpblp_device::gpib_w(offs_t offset, uint16_t data)
{
	if (offset & 0x08) {
		m_tms9914->write(offset & 0x07, data);
		return;
	}

	switch (offset) {
	case 0:
		m_tms9914->reset();
		break;

	case 1:
		if (data >> 8 == 0)
			m_gpib_reg1 = 0x84;
		else if (data >> 8 == 1)
			m_gpib_reg1 = 0xbf;
		else
			m_gpib_reg1 = 0xbe;
		break;
	default:
		break;
	}
}

uint16_t isa8_hpblp_device::gpib_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset & 0x8)
		return m_tms9914->read(offset & 0x07);

	switch(offset) {
	case 0: /* ID */
		data = 0x31;
		break;
	case 1:
		/* Int control */
		data = m_gpib_reg1;
		break;
	case 2:
		/* Address */
		data = 0xbe;
		break;
	default:
		break;
	}
	return data;
}

void isa8_hpblp_device::update_gpib_irq()
{
	if ((m_gpib_reg1 & 0x40) && (m_gpib_reg1 & 0x80))
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
	else
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
}

void isa8_hpblp_device::gpib_irq(int state)
{
	if (state) {
		m_gpib_reg1 |= 0x40;
	} else {
		m_gpib_reg1 &= ~0x40;
	}
	update_gpib_irq();
}

void isa8_hpblp_device::m68map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(isa8_hpblp_device::bus_r), FUNC(isa8_hpblp_device::bus_w));
	map(0x000000, 0x00ffff).rom().nopw();
	map(0x470000, 0x47001f).mirror(0x00ffe0).rw(FUNC(isa8_hpblp_device::gpib_r), FUNC(isa8_hpblp_device::gpib_w));
	map(0xc00000, 0xffffff).ram();
}

void isa8_hpblp_device::hpblp_interrupt(int state)
{
	switch(m_irq) {
	case 0:
		m_isa->irq3_w(state);
		break;
	case 1:
		m_isa->irq4_w(state);
		break;
	case 2:
		m_isa->irq5_w(state);
		break;
	case 3:
		m_isa->irq7_w(state);
		break;
	case 4:
		m_isa->irq2_w(state);
		break;
	default:
		break;
	}
}

uint8_t isa8_hpblp_device::get_bus_address_partial(int shift)
{
	offs_t addr;

	// The value read from the bus address registers (isa reg 2,3,4) is
	// either the address during a waiting 68k bus cycle, or, if the
	// processor is running the current bus address which might be a data
	// read/write or program fetch. The DOS software checks whether these
	// register changes during normal operation. Therefore just invert
	// the address on every read if the CPU is not waiting for the host.

	if (!m_bus_read && !m_bus_write)
		m_bus_address ^= 0xffffff;

	addr = m_bus_address;
	return (addr >> shift) & 0xff;
}

uint8_t isa8_hpblp_device::addrl_r(offs_t offset)
{
	return get_bus_address_partial(0);
}

uint8_t isa8_hpblp_device::addrm_r(offs_t offset)
{
	return get_bus_address_partial(8);
}

uint8_t isa8_hpblp_device::addrh_r(offs_t offset)
{
	return get_bus_address_partial(16);
}

uint8_t isa8_hpblp_device::datal_r(offs_t offset)
{
	return m_bus_data & 0xff;
}

uint8_t isa8_hpblp_device::datah_r(offs_t offset)
{
	return m_bus_data >> 8;
}

void isa8_hpblp_device::datal_w(offs_t offset, uint8_t data)
{
	m_bus_data &= ~0xff;
	m_bus_data |= data;
}

void isa8_hpblp_device::datah_w(offs_t offset, uint8_t data)
{
	m_bus_data &= ~0xff00;
	m_bus_data |= (data << 8);
}

uint8_t isa8_hpblp_device::status_r(offs_t offset)
{
	uint8_t ret;

	// bits:
	// 7-5: type of buscycle (exact encoding unknown)
	// 4 - INT status
	// 2 - CPU RW#

	if (m_bus_read || m_bus_write)
		ret = status_val(m_bus_address);
	else
		ret = 0xe4;

	if (m_bus_read)
		ret |= 0x04;

	if (!m_irq_state)
		ret |= 0x10;
	return ret;
}

void isa8_hpblp_device::irq_w(offs_t offset, uint8_t data)
{
	hpblp_interrupt(1);
}

uint8_t isa8_hpblp_device::reg5_r(offs_t offset)
{
	// state bits:
	// 3-0: OS running
	// 0: BOOTROM
	// 1: HPBASIC
	// 2: PASCAL
	// 4: Is a 82324A card
	return m_reg5;
}

uint8_t isa8_hpblp_device::reg6_r(offs_t offset)
{
	return m_reg6;
}

void isa8_hpblp_device::reg2_w(offs_t offset, uint8_t data)
{
	// bit 0 - CPU RESET# line
	m_reset = data & 1;
	if (m_reset)
		device_reset();
}

// reg 3: force BERR# on current bus cycle
void isa8_hpblp_device::reg3_w(offs_t offset, uint8_t data)
{
	m_ack_buscycle = false;
	m_bus_read = false;
	m_bus_write = false;
	m_maincpu->trigger_bus_error();
	machine().scheduler().synchronize();
}

// reg 4: clear pending 10ms timer interrupt to host
void isa8_hpblp_device::reg4_w(offs_t offset, uint8_t data)
{
	hpblp_interrupt(0);
}

// reg 5: OS status
void isa8_hpblp_device::reg5_w(offs_t offset, uint8_t data)
{
	m_reg5 = data;
}

// reg 6: seems to be used to trigger interrupts on the 68k.
// Only seen with keyboard irqs so far.
void isa8_hpblp_device::reg6_w(offs_t offset, uint8_t data)
{
	// bit 2-0: IPL?
	if (data == 6) {
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	} else {
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
	m_reg6 = data;
}

// reg 7: acknowledge current 68k bus cycle
void isa8_hpblp_device::reg7_w(address_space &space, offs_t offset, uint8_t data)
{
	if (m_reset) {
		// DOS software write to this reg during reset, so we need to ignore
		// this write in order to not acknowledge the first bus cycle early
		return;
	}

	m_bus_read = false;
	m_bus_write = false;
	m_ack_buscycle = true;
	// It looks like the DOS software assumes that the m68k is always faster starting
	// the next buscycle than the PC reading the data registers. So it seems they
	// "optimized" the code to skip checking the bus status registers before accessing
	// the data registers during a 32 bit instruction. (which will be split into 2x 16
	// bit bus transactions by 68k. Eat all cycles to make the scheduler switch to 68k
	// before the x86 CPU can read the registers again.
	machine().scheduler().synchronize();
}

void isa8_hpblp_device::isamap(address_map &map)
{
	map(0, 0).rw(FUNC(isa8_hpblp_device::datal_r), FUNC(isa8_hpblp_device::datal_w));
	map(1, 1).rw(FUNC(isa8_hpblp_device::datah_r), FUNC(isa8_hpblp_device::datah_w));
	map(2, 2).rw(FUNC(isa8_hpblp_device::addrl_r), FUNC(isa8_hpblp_device::reg2_w));
	map(3, 3).rw(FUNC(isa8_hpblp_device::addrm_r), FUNC(isa8_hpblp_device::reg3_w));
	map(4, 4).rw(FUNC(isa8_hpblp_device::addrh_r), FUNC(isa8_hpblp_device::reg4_w));
	map(5, 5).rw(FUNC(isa8_hpblp_device::reg5_r), FUNC(isa8_hpblp_device::reg5_w));
	map(6, 6).rw(FUNC(isa8_hpblp_device::reg6_r), FUNC(isa8_hpblp_device::reg6_w));
	map(7, 7).rw(FUNC(isa8_hpblp_device::status_r), FUNC(isa8_hpblp_device::reg7_w));
}

TIMER_CALLBACK_MEMBER(isa8_hpblp_device::timer10ms)
{
	if (m_reg6 & 2)
		hpblp_interrupt(1);
	m_timer_10ms->adjust(attotime::from_msec(10));
}

void isa8_hpblp_device::device_start()
{
	set_isa_device();
	m_installed = false;
	m_timer_10ms = timer_alloc(FUNC(isa8_hpblp_device::timer10ms), this);
	m_timer_10ms->adjust(attotime::from_msec(10));

	save_item(NAME(m_bus_address));
	save_item(NAME(m_bus_mem_mask));
	save_item(NAME(m_bus_data));
	save_item(NAME(m_gpib_reg1));
	save_item(NAME(m_reg5));
	save_item(NAME(m_reg6));
	save_item(NAME(m_ack_buscycle));
	save_item(NAME(m_bus_read));
	save_item(NAME(m_bus_write));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_reset));
}

void isa8_hpblp_device::device_reset()
{
	uint16_t port;

	if (!m_installed) {
		// MAME doesn't allow reading ioport at device_start
		m_irq = ioport("BLPIRQ")->read();
		switch(ioport("BLPPORT")->read()) {
		case 1:
			port = 0x280;
			break;
		case 2:
			port = 0x330;
			break;
		case 3:
			port = 0x390;
			break;
		default:
			port = 0x250;
			break;
		}
		m_isa->install_device(port, port + 7, *this, &isa8_hpblp_device::isamap);
		m_installed = true;
	}

	m_ack_buscycle = false;
	m_bus_read = false;
	m_bus_write = false;
	m_bus_address = 0;
	m_reg5 = 0;
	m_reg6 = 0;
	m_maincpu->reset();
	m_gpib_reg1 = 0xbe;
	m_tms9914->reset();
}

ROM_START(hpblp)
	ROM_REGION(0x20000, BLP_TAG, 0)
	ROM_LOAD16_BYTE( "1818-4576.bin", 0x000000, 0x008000, CRC(8beb6bdb) SHA1(9b620c577aea54841608d0d46f7e8077eef607c6))
	ROM_LOAD16_BYTE( "1818-4577.bin", 0x000001, 0x008000, CRC(e8a5c071) SHA1(9e13c0ef2aed6565c33795780b7102f2647cf2a0))
ROM_END

const tiny_rom_entry *isa8_hpblp_device::device_rom_region() const
{
	return ROM_NAME(hpblp);
}
