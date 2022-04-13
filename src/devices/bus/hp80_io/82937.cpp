// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82937.cpp

    82937 module (HPIB interface)

    TODO: Implement Parallel Poll response

    Thanks to Tim Nye & Everett Kaser for dumping the 8049 ROM

    Main reference for this module is:
    HP 82937-90007, oct 80, HP82937A HP-IB Installation and theory
    of operation manual

*********************************************************************/

#include "emu.h"
#include "82937.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Bits in U3 (m_latch)
constexpr unsigned LATCH_CA_BIT = 5; // Controller Active
constexpr unsigned LATCH_TA_BIT = 4; // Talker Active
constexpr unsigned LATCH_EN_IFC_INT_BIT = 3; // Enable IFC interrupt
constexpr unsigned LATCH_EN_REN_INT_BIT = 2; // Enable REN interrupt
constexpr unsigned LATCH_EN_ATN_INT_BIT = 1; // Enable ATN interrupt
constexpr unsigned LATCH_EN_NDAC_BIT = 0;    // Enable NDAC

// Bits on P1 port of 8049
constexpr unsigned P1_IFC_BIT = 7;
constexpr unsigned P1_REN_BIT = 6;
constexpr unsigned P1_SRQ_BIT = 5;
constexpr unsigned P1_ATN_BIT = 4;
constexpr unsigned P1_EOI_BIT = 3;
constexpr unsigned P1_DAV_BIT = 2;
constexpr unsigned P1_NDAC_BIT = 1;
constexpr unsigned P1_NRFD_BIT = 0;

hp82937_io_card_device::hp82937_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP82937_IO_CARD , tag , owner , clock),
	  device_hp80_io_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_translator(*this , "xlator"),
	  m_sw1(*this , "sw1"),
	  m_ieee488(*this , IEEE488_TAG)
{
}

hp82937_io_card_device::~hp82937_io_card_device()
{
}

void hp82937_io_card_device::install_read_write_handlers(address_space& space , uint16_t base_addr)
{
	space.install_readwrite_handler(base_addr, base_addr + 1, read8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_r)), write8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_w)));
}

void hp82937_io_card_device::inten()
{
	m_translator->inten();
}

void hp82937_io_card_device::clear_service()
{
	m_translator->clear_service();
}

WRITE_LINE_MEMBER(hp82937_io_card_device::reset_w)
{
	m_cpu->set_input_line(INPUT_LINE_RESET , state);
	if (state) {
		// When reset is asserted, clear state
		device_reset();
	}
}

READ_LINE_MEMBER(hp82937_io_card_device::t0_r)
{
	return m_iatn;
}

uint8_t hp82937_io_card_device::p1_r()
{
	uint8_t res = 0;

	if (BIT(m_sw1->read() , 5)) {
		// System controller
		BIT_SET(res , P1_IFC_BIT);
		BIT_SET(res , P1_REN_BIT);
	} else {
		// Not system controller
		if (m_ieee488->ifc_r()) {
			BIT_SET(res , P1_IFC_BIT);
		}
		if (m_ieee488->ren_r()) {
			BIT_SET(res , P1_REN_BIT);
		}
	}
	if (!BIT(m_latch , LATCH_CA_BIT) || m_ieee488->srq_r()) {
		BIT_SET(res , P1_SRQ_BIT);
	}
	if (m_iatn) {
		BIT_SET(res , P1_ATN_BIT);
	}
	bool ndac = !BIT(m_latch , LATCH_EN_NDAC_BIT) || m_iatn;
	if (m_talker_out) {
		BIT_SET(res , P1_EOI_BIT);
		BIT_SET(res , P1_DAV_BIT);
		if (ndac && m_ieee488->ndac_r()) {
			BIT_SET(res , P1_NDAC_BIT);
		}
		if (m_ieee488->nrfd_r()) {
			BIT_SET(res , P1_NRFD_BIT);
		}
	} else {
		if (m_ieee488->eoi_r()) {
			BIT_SET(res , P1_EOI_BIT);
		}
		if (m_ieee488->dav_r()) {
			BIT_SET(res , P1_DAV_BIT);
		}
		if (ndac) {
			BIT_SET(res , P1_NDAC_BIT);
		}
		BIT_SET(res , P1_NRFD_BIT);
	}

	return res;
}

void hp82937_io_card_device::p1_w(uint8_t data)
{
	update_signals();
	update_data_out();
}

uint8_t hp82937_io_card_device::dio_r()
{
	if (m_dio_out) {
		return 0xff;
	} else {
		return m_ieee488->dio_r();
	}
}

void hp82937_io_card_device::dio_w(uint8_t data)
{
	update_data_out();
}

uint8_t hp82937_io_card_device::switch_r()
{
	return m_sw1->read() | 0xc0;
}

void hp82937_io_card_device::latch_w(uint8_t data)
{
	LOG("latch=%02x\n" , data);
	m_latch = data;
	update_signals();
	update_data_out();
}

WRITE_LINE_MEMBER(hp82937_io_card_device::ieee488_ctrl_w)
{
	update_signals();
	update_data_out();
}

static INPUT_PORTS_START(hp82937_port)
	PORT_HP80_IO_SC(7)
	PORT_START("sw1")
	PORT_DIPNAME(0x1f , 0x15 , "HPIB address")
	PORT_DIPLOCATION("S1:7,6,5,4,3")
	PORT_DIPSETTING(0x00 , "0")
	PORT_DIPSETTING(0x01 , "1")
	PORT_DIPSETTING(0x02 , "2")
	PORT_DIPSETTING(0x03 , "3")
	PORT_DIPSETTING(0x04 , "4")
	PORT_DIPSETTING(0x05 , "5")
	PORT_DIPSETTING(0x06 , "6")
	PORT_DIPSETTING(0x07 , "7")
	PORT_DIPSETTING(0x08 , "8")
	PORT_DIPSETTING(0x09 , "9")
	PORT_DIPSETTING(0x0a , "10")
	PORT_DIPSETTING(0x0b , "11")
	PORT_DIPSETTING(0x0c , "12")
	PORT_DIPSETTING(0x0d , "13")
	PORT_DIPSETTING(0x0e , "14")
	PORT_DIPSETTING(0x0f , "15")
	PORT_DIPSETTING(0x10 , "16")
	PORT_DIPSETTING(0x11 , "17")
	PORT_DIPSETTING(0x12 , "18")
	PORT_DIPSETTING(0x13 , "19")
	PORT_DIPSETTING(0x14 , "20")
	PORT_DIPSETTING(0x15 , "21")
	PORT_DIPSETTING(0x16 , "22")
	PORT_DIPSETTING(0x17 , "23")
	PORT_DIPSETTING(0x18 , "24")
	PORT_DIPSETTING(0x19 , "25")
	PORT_DIPSETTING(0x1a , "26")
	PORT_DIPSETTING(0x1b , "27")
	PORT_DIPSETTING(0x1c , "28")
	PORT_DIPSETTING(0x1d , "29")
	PORT_DIPSETTING(0x1e , "30")
	PORT_DIPSETTING(0x1f , "31")
	PORT_DIPNAME(0x20 , 0x20 , "Sys. controller")
	PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING(0x00 , DEF_STR(Off))
	PORT_DIPSETTING(0x20 , DEF_STR(On))
INPUT_PORTS_END

ioport_constructor hp82937_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp82937_port);
}

void hp82937_io_card_device::device_start()
{
	save_item(NAME(m_dio_out));
	save_item(NAME(m_talker_out));
	save_item(NAME(m_iatn));
	save_item(NAME(m_latch));
}

void hp82937_io_card_device::device_reset()
{
	m_latch = 0;
	m_updating = false;
	update_signals();
	update_data_out();
}

void hp82937_io_card_device::update_data_out()
{
	m_ieee488->host_dio_w(m_dio_out ? m_cpu->p2_r() : 0xff);
}

void hp82937_io_card_device::update_signals()
{
	// Avoid recursive re-enter when writing to IEEE488 signals
	if (m_updating) {
		return;
	}
	m_updating = true;
	bool ctrl_active = BIT(m_latch , LATCH_CA_BIT);
	uint8_t p1 = m_cpu->p1_r();
	m_iatn = BIT(p1 , P1_ATN_BIT);
	if (ctrl_active) {
		m_ieee488->host_atn_w(m_iatn);
		m_ieee488->host_srq_w(1);
	} else {
		m_ieee488->host_atn_w(1);
		m_iatn = m_iatn && m_ieee488->atn_r();
		m_ieee488->host_srq_w(BIT(p1 , P1_SRQ_BIT));
	}
	m_talker_out = (ctrl_active && !m_iatn) || (BIT(m_latch , LATCH_TA_BIT) && m_iatn);
	if (m_talker_out) {
		m_ieee488->host_nrfd_w(1);
		m_ieee488->host_dav_w(BIT(p1 , P1_DAV_BIT));
		m_ieee488->host_eoi_w(BIT(p1 , P1_EOI_BIT));
		m_ieee488->host_ndac_w(1);

	} else {
		m_ieee488->host_nrfd_w(BIT(p1 , P1_NRFD_BIT));
		m_ieee488->host_dav_w(1);
		m_ieee488->host_eoi_w(1);
		bool ndac = BIT(p1 , P1_NDAC_BIT);
		if (BIT(m_latch , LATCH_EN_NDAC_BIT) && !m_iatn) {
			ndac = false;
		}
		m_ieee488->host_ndac_w(ndac);
	}
	bool iren = BIT(p1 , P1_REN_BIT);
	if (BIT(m_sw1->read() , 5)) {
		// System controller
		m_ieee488->host_ren_w(iren);
		m_ieee488->host_ifc_w(BIT(p1 , P1_IFC_BIT));
	} else {
		// Not system controller
		m_ieee488->host_ren_w(1);
		iren = iren && m_ieee488->ren_r();
		m_ieee488->host_ifc_w(1);
	}
	bool not_u8_1 = m_iatn || m_ieee488->eoi_r();
	m_dio_out = not_u8_1 && m_talker_out;
	bool irq = (BIT(m_latch , LATCH_EN_IFC_INT_BIT) && !m_ieee488->ifc_r()) ||
		(BIT(m_latch , LATCH_EN_REN_INT_BIT) && iren) ||
		(BIT(m_latch , LATCH_EN_ATN_INT_BIT) && !m_iatn);
	m_cpu->set_input_line(MCS48_INPUT_IRQ , irq);
	m_updating = false;
}

ROM_START(hp82937)
	ROM_REGION(0x800 , "cpu" , 0)
	ROM_LOAD("1820-2437.bin" , 0 , 0x800 , CRC(687d1559) SHA1(44dfc8c3f431cd37a270b094f1db751214009214))
ROM_END

void hp82937_io_card_device::cpu_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).rw("xlator", FUNC(hp_1mb5_device::uc_r), FUNC(hp_1mb5_device::uc_w));
	map(0x03, 0x03).rw(FUNC(hp82937_io_card_device::switch_r), FUNC(hp82937_io_card_device::latch_w));
}

const tiny_rom_entry *hp82937_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp82937);
}

void hp82937_io_card_device::device_add_mconfig(machine_config &config)
{
	I8049(config, m_cpu, XTAL(11'000'000));
	m_cpu->set_addrmap(AS_IO, &hp82937_io_card_device::cpu_io_map);
	m_cpu->t0_in_cb().set(FUNC(hp82937_io_card_device::t0_r));
	m_cpu->t1_in_cb().set("xlator", FUNC(hp_1mb5_device::int_r));
	m_cpu->p1_in_cb().set(FUNC(hp82937_io_card_device::p1_r));
	m_cpu->p1_out_cb().set(FUNC(hp82937_io_card_device::p1_w));
	m_cpu->p2_in_cb().set(FUNC(hp82937_io_card_device::dio_r));
	m_cpu->p2_out_cb().set(FUNC(hp82937_io_card_device::dio_w));

	HP_1MB5(config, m_translator, 0);
	m_translator->irl_handler().set(FUNC(hp82937_io_card_device::irl_w));
	m_translator->halt_handler().set(FUNC(hp82937_io_card_device::halt_w));
	m_translator->reset_handler().set(FUNC(hp82937_io_card_device::reset_w));

	ieee488_slot_device &ieee_dev(IEEE488_SLOT(config, "ieee_dev", 0));
	hp_ieee488_devices(ieee_dev);
	ieee488_slot_device &ieee_rem(IEEE488_SLOT(config, "ieee_rem", 0));
	remote488_devices(ieee_rem);

	IEEE488(config, m_ieee488, 0);
	m_ieee488->ifc_callback().set(FUNC(hp82937_io_card_device::ieee488_ctrl_w));
	m_ieee488->atn_callback().set(FUNC(hp82937_io_card_device::ieee488_ctrl_w));
	m_ieee488->ren_callback().set(FUNC(hp82937_io_card_device::ieee488_ctrl_w));
	m_ieee488->eoi_callback().set(FUNC(hp82937_io_card_device::ieee488_ctrl_w));
}

// device type definition
DEFINE_DEVICE_TYPE(HP82937_IO_CARD, hp82937_io_card_device, "hp82937", "HP82937 card")
