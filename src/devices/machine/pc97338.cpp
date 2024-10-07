// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

National Semiconductor PC97338 Super I/O

TODO:
- PC87338 (no MTEST register, bunch of other small differences)
- Eventually inherit from PC87306, emulate what's in between;

**************************************************************************************************/

#include "emu.h"
#include "pc97338.h"

#define LOG_WARN        (1U << 1) // Show warnings

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)       LOGMASKED(LOG_WARN, __VA_ARGS__)

DEFINE_DEVICE_TYPE(PC97338, pc97338_device, "pc97338", "National Semiconductor PC97338 Super I/O")

pc97338_device::pc97338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC97338, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pc97338_device::config_map), this))
//  , m_kbdc(*this, "pc_kbdc")
//  , m_rtc(*this, "rtc")
	, m_pc_com(*this, "uart%d", 0U)
	, m_pc_lpt(*this, "lpta")
//  , m_gp20_reset_callback(*this)
//  , m_gp25_gatea20_callback(*this)
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
{ }

void pc97338_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	//m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);
}

void pc97338_device::device_reset()
{
	m_locked_state = 2;
}

device_memory_interface::space_config_vector pc97338_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void pc97338_device::device_add_mconfig(machine_config &config)
{
	PC_LPT(config, m_pc_lpt);
	m_pc_lpt->irq_handler().set(FUNC(pc97338_device::irq_parallel_w));

	NS16550(config, m_pc_com[0], XTAL(1'843'200));
	m_pc_com[0]->out_int_callback().set(FUNC(pc97338_device::irq_serial1_w));
	m_pc_com[0]->out_tx_callback().set(FUNC(pc97338_device::txd_serial1_w));
	m_pc_com[0]->out_dtr_callback().set(FUNC(pc97338_device::dtr_serial1_w));
	m_pc_com[0]->out_rts_callback().set(FUNC(pc97338_device::rts_serial1_w));

	NS16550(config, m_pc_com[1], XTAL(1'843'200));
	m_pc_com[1]->out_int_callback().set(FUNC(pc97338_device::irq_serial2_w));
	m_pc_com[1]->out_tx_callback().set(FUNC(pc97338_device::txd_serial2_w));
	m_pc_com[1]->out_dtr_callback().set(FUNC(pc97338_device::dtr_serial2_w));
	m_pc_com[1]->out_rts_callback().set(FUNC(pc97338_device::rts_serial2_w));
}

void pc97338_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// TODO: BADDR1/0 config pin controlled
		u16 superio_base = 0x002e;
		m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(pc97338_device::read)), write8sm_delegate(*this, FUNC(pc97338_device::write)));

		if (BIT(m_fer, 0))
			m_isa->install_device(0x378, 0x37f, read8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::write)));

		if (BIT(m_fer, 1))
			m_isa->install_device(0x3f8, 0x3ff, read8sm_delegate(*m_pc_com[0], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_pc_com[0], FUNC(ns16450_device::ins8250_w)));

		if (BIT(m_fer, 2))
			m_isa->install_device(0x2f8, 0x2ff, read8sm_delegate(*m_pc_com[1], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_pc_com[1], FUNC(ns16450_device::ins8250_w)));
	}
}

u8 pc97338_device::read(offs_t offset)
{
	if (m_locked_state)
	{
		if (!machine().side_effects_disabled())
			m_locked_state --;
		return (m_locked_state) ? 0x88 : 0x00;
	}

	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void pc97338_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_index = data;
	}
	else
	{
		// TODO: two writes, first one just unlocks?
		space().write_byte(m_index, data);
	}
}

void pc97338_device::config_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(pc97338_device::fer_r), FUNC(pc97338_device::fer_w));
	map(0x01, 0x01).rw(FUNC(pc97338_device::far_r), FUNC(pc97338_device::far_w));
//  map(0x02, 0x02) PTR Power and Test Register
//  map(0x03, 0x03) FCR Function Control Register
//  map(0x04, 0x04) PCR Parallel port Control Register
//  map(0x05, 0x05).rw(FUNC(pc97338_device::krr_r), FUNC(pc97338_device::krr_w));
//  map(0x06, 0x06) PMC Power Management Control Register
//  map(0x07, 0x07) TUP Tape, UART and Parallel Port Configuration Register
	// SID Super I/O Identification Register
	// bits 7-3 -> 01110 TL/C/12379-27
	// bits 2-0 -> <undefined>
	map(0x08, 0x08).lr8(
		NAME([] (offs_t offset) { return 0xb0; })
	);
//  map(0x09, 0x09) ASC Advanced Super I/O Configuration Register
//  map(0x0a, 0x0a) CS0LA Chip Select 0 Low Address
//  map(0x0b, 0x0b) CS0CF Chip Select 0 Configuration Address
//  map(0x0c, 0x0c) CS1LA Chip Select 1 Low Address
//  map(0x0d, 0x0d) CS1CF Chip Select 1 Configuration Address
//  map(0x0e, 0x0e) IRC InfraRed Configuration Register
//  map(0x0f, 0x0f) GPBA General Purpose I/O Port Base Address
//  map(0x10, 0x10) CS0HA Chip Select 0 High Address
//  map(0x11, 0x11) CS1HA Chip Select 1 High Address
//  map(0x12, 0x12) SCF0 Super I/O Configuration Register 0
//  map(0x18, 0x18) SCF1 Super I/O Configuration Register 1
//  map(0x19, 0x19) <missing in PC97338, LTPBA in '87306>

//  map(0x1b, 0x1b) PNP0 Plug and Play Configuration 0 Register
//  map(0x1c, 0x1c) PNP1 Plug and Play Configuration 1 Register

//  map(0x40, 0x40) SCF2 Super I/O Configuration Register 2
//  map(0x41, 0x41) PNP2 PnP Configuration Register 2
//  map(0x42, 0x43) PBAL/PBAH Parallel Port Base Address Low/High Registers
//  map(0x44, 0x45) S1BAL/S1BAH SCC1 Base Address Low/High Registers
//  map(0x46, 0x47) S2BAL/S2BAH SCC2 Base Address Low/High Registers
//  map(0x48, 0x49) FBAL/FBAH FDC Base Address Low/High Registers
//  map(0x4a, 0x4b) SBAL/SBAH Super I/O Chip Base Address Low/High Registers
//  map(0x4c, 0x4e) SIRQ1/2/3 System IRQ Input Configuration Register
//  map(0x4f, 0x4f) PNP3 PnP Configuration Register 3

//  map(0x50, 0x50) SCF3 Super I/O Configuration Register 3
//  map(0x51, 0x51) CLK Clock Control Register
//  map(0x52, 0x52) MTEST Manufacturing Test Register (not present on '87338)
}

/*
 * [0x00] FER Function Enable Register
 * xx-- ---- <reserved in '97338, was IDE in '87306>
 * --x- ---- FDC address select
 * ---x ---- (0) x2 floppy drives (1) x4 floppy drives
 * ---- x--- FDC enable
 * ---- -x-- UART2 enable
 * ---- --x- UART1 enable
 * ---- ---x Parallel Port enable
 */
u8 pc97338_device::fer_r(offs_t offset)
{
	return m_fer;
}

void pc97338_device::fer_w(offs_t offset, u8 data)
{
	m_fer = data;
	remap(AS_IO, 0, 0x400);
}

/*
 * [0x01] FAR Function Address Register
 * xxxx ---- UART2 address select
 * xx-- xx-- UART1 address select
 * ---- --xx parallel address select
 */
u8 pc97338_device::far_r(offs_t offset)
{
	return m_far;
}

void pc97338_device::far_w(offs_t offset, u8 data)
{
	m_far = data;
	remap(AS_IO, 0, 0x400);
}

/*
 * Serial
 */

void pc97338_device::irq_serial1_w(int state)
{
	if (!(BIT(m_fer, 1)))
		return;
	request_irq(3, state ? ASSERT_LINE : CLEAR_LINE);
}

void pc97338_device::irq_serial2_w(int state)
{
	if (!(BIT(m_fer, 2)))
		return;
	request_irq(4, state ? ASSERT_LINE : CLEAR_LINE);
}

void pc97338_device::txd_serial1_w(int state)
{
	if (!(BIT(m_fer, 1)))
		return;
	m_txd1_callback(state);
}

void pc97338_device::txd_serial2_w(int state)
{
	if (!(BIT(m_fer, 2)))
		return;
	m_txd2_callback(state);
}

void pc97338_device::dtr_serial1_w(int state)
{
	if (!(BIT(m_fer, 1)))
		return;
	m_ndtr1_callback(state);
}

void pc97338_device::dtr_serial2_w(int state)
{
	if (!(BIT(m_fer, 2)))
		return;
	m_ndtr2_callback(state);
}

void pc97338_device::rts_serial1_w(int state)
{
	if (!(BIT(m_fer, 1)))
		return;
	m_nrts1_callback(state);
}

void pc97338_device::rts_serial2_w(int state)
{
	if (!(BIT(m_fer, 2)))
		return;
	m_nrts2_callback(state);
}

void pc97338_device::rxd1_w(int state)
{
	m_pc_com[0]->rx_w(state);
}

void pc97338_device::ndcd1_w(int state)
{
	m_pc_com[0]->dcd_w(state);
}

void pc97338_device::ndsr1_w(int state)
{
	m_pc_com[0]->dsr_w(state);
}

void pc97338_device::nri1_w(int state)
{
	m_pc_com[0]->ri_w(state);
}

void pc97338_device::ncts1_w(int state)
{
	m_pc_com[0]->cts_w(state);
}

void pc97338_device::rxd2_w(int state)
{
	m_pc_com[1]->rx_w(state);
}

void pc97338_device::ndcd2_w(int state)
{
	m_pc_com[1]->dcd_w(state);
}

void pc97338_device::ndsr2_w(int state)
{
	m_pc_com[1]->dsr_w(state);
}

void pc97338_device::nri2_w(int state)
{
	m_pc_com[1]->ri_w(state);
}

void pc97338_device::ncts2_w(int state)
{
	m_pc_com[1]->cts_w(state);
}

/*
 * Parallel
 */

void pc97338_device::irq_parallel_w(int state)
{
	if (!(BIT(m_fer, 0)))
		return;
	request_irq(5, state ? ASSERT_LINE : CLEAR_LINE);
}


void pc97338_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

