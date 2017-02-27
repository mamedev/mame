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
#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

hp98034_io_card::hp98034_io_card(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp9845_io_card_device(mconfig , HP98034_IO_CARD , "HP98034 card" , tag , owner , clock , "hp98034" , __FILE__),
	  m_cpu(*this , "np"),
	  m_sw1(*this , "sw1"),
	  m_ieee488(*this , IEEE488_TAG)
{
}

hp98034_io_card::~hp98034_io_card()
{
}

static INPUT_PORTS_START(hp98034_port)
	MCFG_HP9845_IO_SC(7)
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

ioport_constructor hp98034_io_card::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98034_port);
}

void hp98034_io_card::device_start()
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

void hp98034_io_card::device_reset()
{
	hp9845_io_card_device::device_reset();
	install_readwrite_handler(read16_delegate(FUNC(hp98034_io_card::reg_r) , this) , write16_delegate(FUNC(hp98034_io_card::reg_w) , this));

	m_idr = 0;
	m_odr = 0;
	m_force_flg = false;
	m_mode_reg = 0xff;
	m_clr_hpib = false;
	m_ctrl_out = 0;
	m_data_out = 0;
	update_dc();
}

READ16_MEMBER(hp98034_io_card::reg_r)
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
	// PPU yields to let NP see FLG=0 immediately
	// (horrible race conditions lurking...)
	space.device().execute().yield();

	LOG(("read R%u=%04x\n" , offset + 4 , res));
	return res;
}

WRITE16_MEMBER(hp98034_io_card::reg_w)
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
	// PPU yields to let NP see FLG=0 immediately
	// (horrible race conditions lurking...)
	space.device().execute().yield();
	LOG(("write R%u=%04x\n" , offset + 4 , data));
}

WRITE8_MEMBER(hp98034_io_card::dc_w)
{
	if (data != m_dc) {
		//LOG(("DC=%02x\n" , data));
		m_dc = data;
		update_dc();
	}
}

READ8_MEMBER(hp98034_io_card::dc_r)
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

WRITE8_MEMBER(hp98034_io_card::hpib_data_w)
{
	m_data_out = data;
	update_data_out();
}

WRITE8_MEMBER(hp98034_io_card::hpib_ctrl_w)
{
	m_ctrl_out = data;
	update_ctrl_out();
}

READ8_MEMBER(hp98034_io_card::hpib_ctrl_r)
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

READ8_MEMBER(hp98034_io_card::hpib_data_r)
{
	return ~m_ieee488->dio_r();
}

READ8_MEMBER(hp98034_io_card::idr_r)
{
	return m_idr;
}

WRITE8_MEMBER(hp98034_io_card::odr_w)
{
	m_odr = data;
}

READ8_MEMBER(hp98034_io_card::mode_reg_r)
{
	return m_mode_reg;
}

WRITE8_MEMBER(hp98034_io_card::mode_reg_clear_w)
{
	m_mode_reg = 0xff;
	m_force_flg = false;
	update_flg();
}

READ8_MEMBER(hp98034_io_card::switch_r)
{
	return m_sw1->read() | 0xc0;
}

IRQ_CALLBACK_MEMBER(hp98034_io_card::irq_callback)
{
	int res = 0xff;

	if (irqline == 0 && !m_ieee488->ifc_r()) {
		BIT_CLR(res, 1);
	}

	return res;
}

WRITE_LINE_MEMBER(hp98034_io_card::ieee488_ctrl_w)
{
	update_clr_hpib();
}

void hp98034_io_card::update_dc(void)
{
	irq_w(!BIT(m_dc , 0));
	sts_w(BIT(m_dc , 4));
	update_flg();
	update_clr_hpib();
}

void hp98034_io_card::update_flg(void)
{
	flg_w(BIT(m_dc , 3) && !m_force_flg);
}

void hp98034_io_card::update_np_irq(void)
{
	m_cpu->set_input_line(0 , (!m_ieee488->ifc_r() || m_clr_hpib) && BIT(m_dc , HP_NANO_IE_DC));
}

void hp98034_io_card::update_data_out(void)
{
	if (m_clr_hpib) {
		m_data_out = 0;
	}
	m_ieee488->dio_w(~m_data_out);
}

void hp98034_io_card::update_ctrl_out(void)
{
	if (m_clr_hpib) {
		m_ieee488->dav_w(1);
		m_ieee488->nrfd_w(1);
		m_ieee488->eoi_w(1);
		m_ieee488->ndac_w(0);
	} else {
		m_ieee488->dav_w(BIT(m_dc , 2));
		m_ieee488->nrfd_w(BIT(m_dc , 1));
		m_ieee488->eoi_w(!BIT(m_ctrl_out , 4));
		m_ieee488->ndac_w(BIT(m_dc , 6));
	}
	m_ieee488->srq_w(!BIT(m_ctrl_out , 0));
	m_ieee488->ren_w(!BIT(m_ctrl_out , 1));
	m_ieee488->atn_w(!BIT(m_ctrl_out , 2));
	m_ieee488->ifc_w(!BIT(m_ctrl_out , 3));
}

void hp98034_io_card::update_clr_hpib(void)
{
	m_clr_hpib = !m_ieee488->atn_r() && BIT(m_dc , 5);
	update_data_out();
	update_ctrl_out();
	update_np_irq();
	LOG(("clr_hpib %d\n" , m_clr_hpib));
}

ROM_START(hp98034)
	ROM_REGION(0x400 , "np" , 0)
	ROM_LOAD("1816-1242.bin" , 0 , 0x400 , CRC(301a9f5f) SHA1(3d7c1ace38c4d3178fdbf764c044535d9f6ac94f))
ROM_END

static ADDRESS_MAP_START(np_program_map , AS_PROGRAM , 8 , hp98034_io_card)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000 , 0x3ff) AM_ROM AM_REGION("np" , 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(np_io_map , AS_IO , 8 , hp98034_io_card)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0 , 0) AM_WRITE(hpib_data_w)
	AM_RANGE(1 , 1) AM_WRITE(hpib_ctrl_w)
	AM_RANGE(2 , 2) AM_READ(hpib_ctrl_r)
	AM_RANGE(3 , 3) AM_READ(hpib_data_r)
	AM_RANGE(4 , 4) AM_READ(idr_r)
	AM_RANGE(5 , 5) AM_WRITE(odr_w)
	AM_RANGE(6 , 6) AM_READWRITE(mode_reg_r , mode_reg_clear_w)
	AM_RANGE(7 , 7) AM_READ(switch_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT(hp98034)
// Clock for NP is generated by a RC oscillator. Manual says its typical frequency
// is around 2 MHz.
	MCFG_CPU_ADD("np" , HP_NANOPROCESSOR , 2000000)
	MCFG_CPU_PROGRAM_MAP(np_program_map)
	MCFG_CPU_IO_MAP(np_io_map)
	MCFG_HP_NANO_DC_CHANGED(WRITE8(hp98034_io_card , dc_w))
	MCFG_HP_NANO_READ_DC_CB(READ8(hp98034_io_card , dc_r))
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(hp98034_io_card , irq_callback)

	MCFG_IEEE488_SLOT_ADD("ieee_dev" , 0 , hp_ieee488_devices , nullptr)
	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_IFC_CALLBACK(WRITELINE(hp98034_io_card , ieee488_ctrl_w))
	MCFG_IEEE488_ATN_CALLBACK(WRITELINE(hp98034_io_card , ieee488_ctrl_w))
MACHINE_CONFIG_END

const tiny_rom_entry *hp98034_io_card::device_rom_region() const
{
	return ROM_NAME(hp98034);
}

machine_config_constructor hp98034_io_card::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(hp98034);
}

// device type definition
const device_type HP98034_IO_CARD = device_creator<hp98034_io_card>;
