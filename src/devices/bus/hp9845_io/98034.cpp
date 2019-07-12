// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98034.cpp

    98034 module (HPIB interface)

    TODO: Implement Parallel Poll response

    The main reference for this module is:
    HP 98034-90001, 98034 Installation and Service Manual

*********************************************************************/

#include "emu.h"
#include "98034.h"
#include "coreutil.h"

// Debugging
//#define VERBOSE 1
#include "logmacro.h"


#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

hp98034_io_card_device::hp98034_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp9845_io_card_device(mconfig , HP98034_IO_CARD , tag , owner , clock),
	  m_cpu(*this , "np"),
	  m_sw1(*this , "sw1"),
	  m_ieee488(*this , IEEE488_TAG)
{
}

hp98034_io_card_device::~hp98034_io_card_device()
{
}

static INPUT_PORTS_START(hp98034_port)
	PORT_HP9845_IO_SC(7)
	PORT_START("sw1")
	PORT_DIPNAME(0x1f , 0x15 , "HPIB address")
	PORT_DIPLOCATION("S1:1,2,3,4,5")
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
	PORT_DIPNAME(0x20 , 0x00 , "Sys. controller")
	PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(0x00 , DEF_STR(On))
	PORT_DIPSETTING(0x20 , DEF_STR(Off))
INPUT_PORTS_END

ioport_constructor hp98034_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98034_port);
}

void hp98034_io_card_device::device_start()
{
	save_item(NAME(m_dc));
	save_item(NAME(m_idr));
	save_item(NAME(m_odr));
	save_item(NAME(m_force_flg));
	save_item(NAME(m_mode_reg));
	save_item(NAME(m_clr_hpib));
	save_item(NAME(m_ctrl_out));
	save_item(NAME(m_data_out));
}

void hp98034_io_card_device::device_reset()
{
	hp9845_io_card_device::device_reset();

	m_idr = 0;
	m_odr = 0;
	m_force_flg = false;
	m_mode_reg = 0xff;
	m_clr_hpib = false;
	m_ctrl_out = 0;
	m_data_out = 0;
	update_dc();
}

READ16_MEMBER(hp98034_io_card_device::reg_r)
{
	uint16_t res = m_odr;

	if (offset == 1 || offset == 3) {
		// Reading from R5 or R7 forces bits 4&5 to 1
		res |= 0x30;
	}

	// Mode register
	// Bits Value
	// ==========
	// 7-4  1
	// 3-2  ~offset
	// 1-0  1
	m_mode_reg = (uint8_t)((offset << 2) ^ 0xff);
	m_force_flg = true;

	update_flg();
	// PPU pauses for an instant to let NP see FLG=0 immediately
	// There's a bug in Mass Memory opt. ROM because the PPU
	// doesn't wait for FLG from 98034 in a few (common) cases.
	// A magic combination of relative speeds between PPU and
	// NP always hides this bug in the real hw.
	space.device().execute().spin_until_time(attotime::from_usec(5));

	LOG("read R%u=%04x\n" , offset + 4 , res);
	return res;
}

WRITE16_MEMBER(hp98034_io_card_device::reg_w)
{
	m_idr = (uint8_t)data;

	// Mode register
	// Bits Value
	// ==========
	// 7-4  1
	// 3-2  ~offset
	// 1    0
	// 0    1
	m_mode_reg = (uint8_t)((offset << 2) ^ 0xfd);
	m_force_flg = true;

	update_flg();
	// PPU pauses for an instant to let NP see FLG=0 immediately
	// (see above)
	space.device().execute().spin_until_time(attotime::from_usec(5));
	LOG("write R%u=%04x\n" , offset + 4 , data);
}

WRITE8_MEMBER(hp98034_io_card_device::dc_w)
{
	if (data != m_dc) {
		//LOG("DC=%02x\n" , data);
		m_dc = data;
		update_dc();
	}
}

READ8_MEMBER(hp98034_io_card_device::dc_r)
{
	uint8_t res;

	if (m_force_flg) {
		// Force DC3 low
		res = 0xf7;
	} else {
		res = 0xff;
	}

	return res;
}

WRITE8_MEMBER(hp98034_io_card_device::hpib_data_w)
{
	m_data_out = data;
	update_data_out();
}

WRITE8_MEMBER(hp98034_io_card_device::hpib_ctrl_w)
{
	m_ctrl_out = data;
	update_ctrl_out();
}

READ8_MEMBER(hp98034_io_card_device::hpib_ctrl_r)
{
	uint8_t res = 0;

	if (!m_ieee488->dav_r()) {
		BIT_SET(res , 0);
	}
	if (!m_ieee488->nrfd_r()) {
		BIT_SET(res , 1);
	}
	if (!m_ieee488->ndac_r()) {
		BIT_SET(res , 2);
	}
	if (!m_ieee488->ifc_r()) {
		BIT_SET(res , 3);
	}
	if (!m_ieee488->atn_r()) {
		BIT_SET(res , 4);
	}
	if (!m_ieee488->srq_r()) {
		BIT_SET(res , 5);
	}
	if (!m_ieee488->ren_r()) {
		BIT_SET(res , 6);
	}
	if (!m_ieee488->eoi_r()) {
		BIT_SET(res , 7);
	}

	return res;
}

READ8_MEMBER(hp98034_io_card_device::hpib_data_r)
{
	return ~m_ieee488->read_dio();
}

READ8_MEMBER(hp98034_io_card_device::idr_r)
{
	return m_idr;
}

WRITE8_MEMBER(hp98034_io_card_device::odr_w)
{
	m_odr = data;
}

READ8_MEMBER(hp98034_io_card_device::mode_reg_r)
{
	return m_mode_reg;
}

WRITE8_MEMBER(hp98034_io_card_device::mode_reg_clear_w)
{
	m_mode_reg = 0xff;
	m_force_flg = false;
	update_flg();
}

READ8_MEMBER(hp98034_io_card_device::switch_r)
{
	return m_sw1->read() | 0xc0;
}

IRQ_CALLBACK_MEMBER(hp98034_io_card_device::irq_callback)
{
	int res = 0xff;

	if (irqline == 0 && !m_ieee488->ifc_r()) {
		BIT_CLR(res, 1);
	}

	return res;
}

WRITE_LINE_MEMBER(hp98034_io_card_device::ieee488_ctrl_w)
{
	update_clr_hpib();
}

void hp98034_io_card_device::update_dc()
{
	irq_w(!BIT(m_dc , 0));
	sts_w(BIT(m_dc , 4));
	update_flg();
	update_clr_hpib();
}

void hp98034_io_card_device::update_flg()
{
	flg_w(BIT(m_dc , 3) && !m_force_flg);
}

void hp98034_io_card_device::update_np_irq()
{
	m_cpu->set_input_line(0 , (!m_ieee488->ifc_r() || m_clr_hpib) && BIT(m_dc , HP_NANO_IE_DC));
}

void hp98034_io_card_device::update_data_out()
{
	if (m_clr_hpib) {
		m_data_out = 0;
	}
	m_ieee488->write_dio(~m_data_out);
}

void hp98034_io_card_device::update_ctrl_out()
{
	if (m_clr_hpib) {
		m_ieee488->host_dav_w(1);
		m_ieee488->host_nrfd_w(1);
		m_ieee488->host_eoi_w(1);
		m_ieee488->host_ndac_w(0);
	} else {
		m_ieee488->host_dav_w(BIT(m_dc , 2));
		m_ieee488->host_nrfd_w(BIT(m_dc , 1));
		m_ieee488->host_eoi_w(!BIT(m_ctrl_out , 4));
		m_ieee488->host_ndac_w(BIT(m_dc , 6));
	}
	m_ieee488->host_srq_w(!BIT(m_ctrl_out , 0));
	m_ieee488->host_ren_w(!BIT(m_ctrl_out , 1));
	m_ieee488->host_atn_w(!BIT(m_ctrl_out , 2));
	m_ieee488->host_ifc_w(!BIT(m_ctrl_out , 3));
}

void hp98034_io_card_device::update_clr_hpib()
{
	m_clr_hpib = !m_ieee488->atn_r() && BIT(m_dc , 5);
	update_data_out();
	update_ctrl_out();
	update_np_irq();
	LOG("clr_hpib %d\n" , m_clr_hpib);
}

ROM_START(hp98034)
	ROM_REGION(0x400 , "np" , 0)
	ROM_LOAD("1816-1242.bin" , 0 , 0x400 , CRC(301a9f5f) SHA1(3d7c1ace38c4d3178fdbf764c044535d9f6ac94f))
ROM_END

void hp98034_io_card_device::np_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x3ff).rom().region("np", 0);
}

void hp98034_io_card_device::np_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0, 0).w(FUNC(hp98034_io_card_device::hpib_data_w));
	map(1, 1).w(FUNC(hp98034_io_card_device::hpib_ctrl_w));
	map(2, 2).r(FUNC(hp98034_io_card_device::hpib_ctrl_r));
	map(3, 3).r(FUNC(hp98034_io_card_device::hpib_data_r));
	map(4, 4).r(FUNC(hp98034_io_card_device::idr_r));
	map(5, 5).w(FUNC(hp98034_io_card_device::odr_w));
	map(6, 6).rw(FUNC(hp98034_io_card_device::mode_reg_r), FUNC(hp98034_io_card_device::mode_reg_clear_w));
	map(7, 7).r(FUNC(hp98034_io_card_device::switch_r));
}

const tiny_rom_entry *hp98034_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp98034);
}

void hp98034_io_card_device::device_add_mconfig(machine_config &config)
{
	// Clock for NP is generated by a RC oscillator. Manual says its typical frequency
	// is around 2 MHz.
	HP_NANOPROCESSOR(config, m_cpu, 2000000);
	m_cpu->set_addrmap(AS_PROGRAM, &hp98034_io_card_device::np_program_map);
	m_cpu->set_addrmap(AS_IO, &hp98034_io_card_device::np_io_map);
	m_cpu->dc_changed().set(FUNC(hp98034_io_card_device::dc_w));
	m_cpu->read_dc().set(FUNC(hp98034_io_card_device::dc_r));
	m_cpu->set_irq_acknowledge_callback(FUNC(hp98034_io_card_device::irq_callback));

	IEEE488_SLOT(config , "ieee_dev" , 0 , hp_ieee488_devices , nullptr);
	IEEE488_SLOT(config , "ieee_rem" , 0 , remote488_devices , nullptr);
	IEEE488(config, m_ieee488);
	m_ieee488->ifc_callback().set(FUNC(hp98034_io_card_device::ieee488_ctrl_w));
	m_ieee488->atn_callback().set(FUNC(hp98034_io_card_device::ieee488_ctrl_w));
}

// device type definition
DEFINE_DEVICE_TYPE(HP98034_IO_CARD, hp98034_io_card_device, "hp98034", "HP98034 card")
