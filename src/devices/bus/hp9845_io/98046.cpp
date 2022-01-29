// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98046.cpp

    98046 module (data communications interface)

    Fun fact: I didn't need to dump the fw in 8048 MCU because,
    in a way, it's already "dumped" in the test sw. Basically, to
    test the correctness of the ROM content, the HP sw reads out the
    whole ROM and compares it to the known good image...

    Main reference for this module:
    HP, 98046B Interface Installation and Service

    Test software:
    HP, 98046-90449, 98046 Interface Test
    (see http://www.hpmuseum.net/display_item.php?sw=324)

*********************************************************************/

#include "emu.h"
#include "98046.h"

// Debugging

#include "logmacro.h"
#define LOG_IFS_MASK (LOG_GENERAL << 1)
#define LOG_IFS(...) LOGMASKED(LOG_IFS_MASK, __VA_ARGS__)
#define LOG_MCU_MASK (LOG_IFS_MASK << 1)
#define LOG_MCU(...) LOGMASKED(LOG_MCU_MASK, __VA_ARGS__)
#define LOG_CPU_MASK (LOG_MCU_MASK << 1)
#define LOG_CPU(...) LOGMASKED(LOG_CPU_MASK, __VA_ARGS__)
#define LOG_SIO_MASK (LOG_CPU_MASK << 1)
#define LOG_SIO(...) LOGMASKED(LOG_SIO_MASK, __VA_ARGS__)
//#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_MCU_MASK | LOG_CPU_MASK | LOG_SIO_MASK)

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// device type definition
DEFINE_DEVICE_TYPE(HP98046_IO_CARD, hp98046_io_card_device , "hp98046" , "HP98046 card")

hp98046_io_card_device::hp98046_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP98046_IO_CARD, tag, owner, clock)
	, device_hp9845_io_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_sio(*this, "sio")
	, m_rs232(*this, "rs232")
	, m_tx_brg(*this , "tx_brg")
	, m_rx_brg(*this , "rx_brg")
	, m_loopback_en(*this, "loop")
{
}

hp98046_io_card_device::~hp98046_io_card_device()
{
}

uint16_t hp98046_io_card_device::reg_r(address_space &space, offs_t offset)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
		// R4
		// Read from rxFIFO
		if (!rx_fifo_flag()) {
			m_rxfifo_irq = false;
			update_irq();
		}
		res = ~m_rx_fifo.dequeue() & 0x1ff;
		// Save bit 8 of new word at output of rx FIFO
		if (!m_rx_fifo.empty()) {
			m_rx_fifo_out_b8 = BIT(m_rx_fifo.peek() , 8);
		}
		update_flg();
		update_sts();
		break;

	case 1:
		// R5
		if (m_rxfifo_overrun) {
			BIT_SET(res , 15);
		}
		BIT_SET(res , 11);
		if (m_inten) {
			BIT_SET(res , 7);
		}
		if (!m_r6_r7_pending) {
			BIT_SET(res , 0);
		}
		break;

	case 2:
		// R6: not mapped
		break;

	case 3:
		// R7: not mapped
		break;

	default:
		break;
	}

	LOG_CPU("rd R%u=%04x\n" , offset + 4 , res);
	return res;
}

void hp98046_io_card_device::reg_w(address_space &space, offs_t offset, uint16_t data)
{
	LOG_CPU("wr R%u=%04x\n" , offset + 4 , data);

	switch (offset) {
	case 0:
		// R4
		// Write to txFIFO
		m_tx_fifo_in = (data ^ 0x00ff) & 0x1ff;
		m_tx_fifo_pending = true;
		load_tx_fifo();
		space.device().execute().yield();
		break;

	case 1:
		// R5
		if (BIT(data , 5)) {
			// 8048 reset
			m_cpu->pulse_input_line(INPUT_LINE_RESET , attotime::zero);
			m_inten = false;
		} else if (BIT(m_port_2 , 7)) {
			// When SIORST is active, inten always sets to 0
			m_inten = false;
		} else {
			m_inten = BIT(data , 7);
		}
		m_enoutint = BIT(data , 0);
		update_irq();
		break;

	case 2:
		// R6
	case 3:
		// R7
		m_r6_r7_select = offset == 3;
		m_r6_r7 = ~data & 0xff;
		set_r6_r7_pending(true);
		break;

	default:
		break;
	}
}

bool hp98046_io_card_device::has_dual_sc() const
{
	return true;
}

void hp98046_io_card_device::device_add_mconfig(machine_config &config)
{
	I8048(config , m_cpu , 6_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM , &hp98046_io_card_device::cpu_program_map);
	m_cpu->set_addrmap(AS_IO , &hp98046_io_card_device::cpu_io_map);
	m_cpu->p1_in_cb().set(FUNC(hp98046_io_card_device::p1_r));
	m_cpu->p2_out_cb().set(FUNC(hp98046_io_card_device::p2_w));
	m_cpu->t1_in_cb().set([this] () { return int(!m_sio_int); });
	// Clock to SIO is actually provided by T0 output of CPU
	Z80SIO(config , m_sio , 0);
	m_sio->out_int_callback().set(FUNC(hp98046_io_card_device::sio_int_w));
	m_sio->out_txda_callback().set(FUNC(hp98046_io_card_device::sio_txd_w));
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(hp98046_io_card_device::rs232_rxd_w));
	m_rs232->dcd_handler().set(FUNC(hp98046_io_card_device::rs232_dcd_w));
	m_rs232->dsr_handler().set(FUNC(hp98046_io_card_device::rs232_dsr_w));
	m_rs232->cts_handler().set(FUNC(hp98046_io_card_device::rs232_cts_w));
	m_rs232->rxc_handler().set(FUNC(hp98046_io_card_device::rs232_rxc_w));
	m_rs232->txc_handler().set(FUNC(hp98046_io_card_device::rs232_txc_w));
	// There's just one 2.4576 MHz xtal, Tx BRG gets its clock from Rx BRG
	F4702(config , m_tx_brg , 2.4576_MHz_XTAL);
	m_tx_brg->s_callback().set([this]() { return m_txc_sel; });
	m_tx_brg->z_callback().set(FUNC(hp98046_io_card_device::txc_w));
	F4702(config , m_rx_brg , 2.4576_MHz_XTAL);
	m_rx_brg->s_callback().set([this]() { return m_rxc_sel; });
	m_rx_brg->z_callback().set(FUNC(hp98046_io_card_device::rxc_w));
	config.set_maximum_quantum(attotime::from_hz(307200));
}

static INPUT_PORTS_START(hp98046_port)
	PORT_START("SC")
	PORT_CONFNAME(0xf , 4 - HP9845_IO_FIRST_SC , "Select Codes")
	PORT_CONFSETTING(1 , "2 3")
	PORT_CONFSETTING(3 , "4 5")
	PORT_CONFSETTING(5 , "6 7")
	PORT_CONFSETTING(7 , "8 9")
	PORT_CONFSETTING(9 , "10 11")
	PORT_CONFSETTING(11 , "12 13")

	PORT_START("loop")
	PORT_CONFNAME(1 , 0 , "ESK loopback")
	PORT_CONFSETTING(0 , DEF_STR(Off))
	PORT_CONFSETTING(1 , DEF_STR(On))
INPUT_PORTS_END

ioport_constructor hp98046_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98046_port);
}

ROM_START(hp98046)
	ROM_REGION(0x400, "cpu" , 0)
	ROM_LOAD("1820-2431.bin" , 0 , 0x400 , CRC(e6a068d6) SHA1(d19c35b18fae52b841060ed879f860fd2cae3482))
ROM_END

const tiny_rom_entry *hp98046_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp98046);
}

void hp98046_io_card_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(1024);
	save_pointer(NAME(m_ram) , 1024);
}

void hp98046_io_card_device::device_reset()
{
	m_port_2 = 0;
	m_inten = false;
	m_enoutint = false;
	update_flg();
	update_sts();
	update_irq();
	m_loopback = m_loopback_en->read() != 0;
}

void hp98046_io_card_device::cpu_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000 , 0x3ff).rom();
	map(0x400 , 0x7ff).r(FUNC(hp98046_io_card_device::ram_r));
}

void hp98046_io_card_device::cpu_io_map(address_map &map)
{
	map(0 , 0xff).rw(FUNC(hp98046_io_card_device::cpu_r) , FUNC(hp98046_io_card_device::cpu_w));
}

uint8_t hp98046_io_card_device::ram_r(offs_t offset)
{
	return m_ram[ offset ];
}

uint8_t hp98046_io_card_device::cpu_r(offs_t offset)
{
	if (BIT(m_port_2 , 2)) {
		return m_ram[ (offset & 0xff) | (uint16_t(m_port_2 & 3) << 8) ];
	} else if (BIT(offset , 2)) {
		uint8_t res = ~0;

		switch (offset & 3) {
		case 0:
			// xxxx'x100: read from tx FIFO
			res = uint8_t(m_tx_fifo.dequeue());
			load_tx_fifo();
			update_flg();
			update_irq();
			break;

		case 1:
			// xxxx'x101: read HS
			res = get_hs_input();
			break;

		case 2:
			// xxxx'x110: clear FIFOs
			m_tx_fifo.clear();
			m_rx_fifo.clear();
			load_tx_fifo();
			update_flg();
			update_sts();
			update_irq();
			break;

		case 3:
			// xxxx'x111: read r6/r7
			res = m_r6_r7;
			set_r6_r7_pending(false);
			break;
		}
		LOG_MCU("CPU R @%02x=%02x\n" , offset , res);
		return res;
	} else {
		uint8_t res = m_sio->cd_ba_r(offset & 3);
		LOG_SIO("SIO R @%u=%02x\n" , offset & 3 , res);
		return res;
	}
}

void hp98046_io_card_device::cpu_w(offs_t offset, uint8_t data)
{
	if (BIT(m_port_2 , 2)) {
		m_ram[ (offset & 0xff) | (uint16_t(m_port_2 & 3) << 8) ] = data;
	} else if (BIT(offset , 2)) {
		LOG_MCU("CPU W @%02x=%02x\n" , offset , data);
		switch (offset & 3) {
		case 0:
			// xxxx'x100: write to rx FIFO
			if (BIT(offset , 6)) {
				if (m_rx_fifo.full()) {
					m_rxfifo_overrun = true;
				}
				uint16_t w = data;
				if (BIT(offset , 7)) {
					BIT_SET(w , 8);
				}
				// If enqueuing first word, store bit 8
				if (m_rx_fifo.empty()) {
					m_rx_fifo_out_b8 = BIT(w , 8);
				}
				m_rx_fifo.enqueue(w);
			}
			if (rx_fifo_flag()) {
				m_rxfifo_irq = true;
			} else {
				// Logic of A1U21A 'LS109 JK FF (J=A3 K/=A4)
				switch (offset & 0x18) {
				case 0x00:
					m_rxfifo_irq = false;
					break;

				case 0x08:
					m_rxfifo_irq = !m_rxfifo_irq;
					break;

				case 0x10:
					break;

				case 0x18:
					m_rxfifo_irq = true;
					break;
				}
			}
			update_flg();
			update_sts();
			update_irq();
			break;

		case 1:
			// xxxx'x101: write HS
			m_hs_out = data;
			update_hs_out();
			break;

		case 2:
			// xxxx'x110: clear rx FIFO overrun
			m_rxfifo_overrun = false;
			update_sts();
			break;

		case 3:
			// xxxx'x111: write to BRGs
			m_rxc_sel = (data >> 4) & 0xf;
			m_txc_sel = data & 0xf;
			break;
		}
	} else {
		LOG_SIO("%.6f SIO W @%u=%02x\n" , machine().time().as_double() , offset & 3 , data);
		m_sio->cd_ba_w(offset & 3 , data);
	}
}

uint8_t hp98046_io_card_device::p1_r()
{
	uint8_t res = 0;
	// b7: b8 of word @ txFIFO head
	if (BIT(m_tx_fifo.peek() , 8)) {
		BIT_SET(res , 7);
	}
	// b6: rxFIFO overrun
	if (!m_rxfifo_overrun) {
		BIT_SET(res , 6);
	}
	// b5: rxFIFO not empty
	if (!m_rx_fifo.empty()) {
		BIT_SET(res , 5);
	}
	// b4: R6(0)/R7(1)
	if (m_r6_r7_select) {
		BIT_SET(res , 4);
	}
	// b3: tx FIFO flag
	if (tx_fifo_flag()) {
		BIT_SET(res , 3);
	}
	// b2: tx FIFO not empty
	if (!m_tx_fifo.empty()) {
		BIT_SET(res , 2);
	}
	// b1: rx FIFO flag
	if (rx_fifo_flag()) {
		BIT_SET(res , 1);
	}
	// b0: rx FIFO not full
	if (!m_rx_fifo.full()) {
		BIT_SET(res , 0);
	}
	//LOG("p1=%02x\n" , res);
	return res;
}

void hp98046_io_card_device::p2_w(uint8_t data)
{
	LOG_MCU("p2=%02x\n" , data);
	uint8_t diff = data ^ m_port_2;
	m_port_2 = data;
	if (BIT(diff , 7)) {
		if (BIT(m_port_2 , 7)) {
			m_sio->reset();
			set_r6_r7_pending(true);
		}
		update_hs_out();
	}
	if (BIT(diff , 6)) {
		update_flg();
	}
	if (BIT(diff , 5)) {
		update_sts();
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::sio_int_w)
{
	if (m_sio_int != state) {
		LOG_SIO("SIO int=%d\n" , state);
	}
	m_sio_int = state;
}

WRITE_LINE_MEMBER(hp98046_io_card_device::sio_txd_w)
{
	m_sio->rxb_w(state);
	if (m_loopback) {
		m_sio->rxa_w(state);
	} else {
		m_rs232->write_txd(state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_rxd_w)
{
	if (!m_loopback) {
		m_sio->rxa_w(state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_dcd_w)
{
	if (!m_loopback) {
		m_sio->dcda_w(state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_dsr_w)
{
	if (!m_loopback) {
		m_sio->dcdb_w(state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_cts_w)
{
	if (!m_loopback) {
		m_sio->ctsa_w(state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_rxc_w)
{
	if (!m_loopback) {
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(hp98046_io_card_device::sync_rx_im_w) , this) , state);
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rs232_txc_w)
{
	if (!m_loopback) {
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(hp98046_io_card_device::sync_tx_im_w) , this) , state);
	}
}

bool hp98046_io_card_device::rx_fifo_flag() const
{
	return m_rx_fifo.queue_length() >= 16;
}

bool hp98046_io_card_device::tx_fifo_flag() const
{
	return m_tx_fifo.queue_length() >= 16;
}

void hp98046_io_card_device::update_flg()
{
	bool flg_e = !m_r6_r7_pending && !m_tx_fifo.full() && BIT(m_port_2 , 6);
	bool flg_o = !m_r6_r7_pending && !m_rx_fifo.empty();

	LOG_IFS("FLG e/o=%d/%d\n" , flg_e , flg_o);
	flg_w(flg_e);
	flg_nextsc_w(flg_o);
}

void hp98046_io_card_device::update_sts()
{
	bool sts_e = !BIT(m_port_2 , 5);
	bool sts_o = !m_rxfifo_overrun && m_rx_fifo_out_b8;

	LOG_IFS("STS e/o=%d/%d\n" , sts_e , sts_o);
	sts_w(sts_e);
	sts_nextsc_w(sts_o);
}

void hp98046_io_card_device::update_irq()
{
	bool irq = m_inten && !m_r6_r7_pending && (m_rxfifo_irq || (m_enoutint && !tx_fifo_flag()));
	bool irq_e = irq && !m_rxfifo_irq;
	bool irq_o = irq && m_rxfifo_irq;

	LOG_IFS("IRQ e/o=%d/%d\n" , irq_e , irq_o);
	irq_w(irq_e);
	irq_nextsc_w(irq_o);
}

void hp98046_io_card_device::update_hs_out()
{
	if (BIT(m_port_2 , 7)) {
		m_actual_hs_out = ~0;
	} else {
		m_actual_hs_out = m_hs_out;
	}
	if (m_loopback) {
		m_sio->ctsa_w(BIT(m_actual_hs_out , 4));
		m_sio->dcda_w(BIT(m_actual_hs_out , 3));
		m_sio->ctsb_w(BIT(m_actual_hs_out , 2));
		m_sio->dcdb_w(BIT(m_actual_hs_out , 1));
	} else {
		m_rs232->write_dtr(BIT(m_actual_hs_out , 5));
		m_rs232->write_rts(BIT(m_actual_hs_out , 4));
		// b3 (A2J1-13) not mapped
		// b2 (A2J1-15) not mapped (Data Rate Select)
		// b1 (A2J1-30) not mapped (Secondary RTS)
		// b0 (A2J1-24) not mapped
	}
}

void hp98046_io_card_device::load_tx_fifo()
{
	if (m_tx_fifo_pending && !m_tx_fifo.full()) {
		m_tx_fifo.enqueue(m_tx_fifo_in);
		m_tx_fifo_pending = false;
		update_flg();
		update_irq();
	}
}

void hp98046_io_card_device::set_r6_r7_pending(bool state)
{
	m_r6_r7_pending = state || BIT(m_port_2 , 7);
	m_cpu->set_input_line(MCS48_INPUT_IRQ , m_r6_r7_pending ? ASSERT_LINE : CLEAR_LINE);
	update_flg();
	update_irq();
}

uint8_t hp98046_io_card_device::get_hs_input() const
{
	uint8_t res = 0xc1;
	if (m_loopback) {
		// DTR looped back into RI
		if (BIT(m_actual_hs_out , 5)) {
			BIT_SET(res , 5);
		}
		// RTS looped back into CTS
		if (BIT(m_actual_hs_out , 4)) {
			BIT_SET(res , 4);
		}
		// A2J1-13 looped back into DCD
		if (BIT(m_actual_hs_out , 3)) {
			BIT_SET(res , 3);
		}
		// DRS looped back into SCD
		if (BIT(m_actual_hs_out , 2)) {
			BIT_SET(res , 2);
		}
		// SRTS looped back into DSR
		if (BIT(m_actual_hs_out , 1)) {
			BIT_SET(res , 1);
		}
	} else {
		if (m_rs232->ri_r()) {
			BIT_SET(res , 5);
		}
		if (m_rs232->cts_r()) {
			BIT_SET(res , 4);
		}
		if (m_rs232->dcd_r()) {
			BIT_SET(res , 3);
		}
		// SCD always 1
		BIT_SET(res , 2);
		if (m_rs232->dsr_r()) {
			BIT_SET(res , 1);
		}
	}
	return res;
}

WRITE_LINE_MEMBER(hp98046_io_card_device::rxc_w)
{
	if (m_last_rxc != bool(state)) {
		m_last_rxc = bool(state);
		m_sio->rxca_w(m_last_rxc);
		if (m_loopback) {
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(hp98046_io_card_device::sync_tx_im_w) , this) , state);
		}
	}
}

WRITE_LINE_MEMBER(hp98046_io_card_device::txc_w)
{
	if (m_last_txc != bool(state)) {
		m_last_txc = bool(state);
		m_sio->txca_w(m_last_txc);
		m_sio->txcb_w(m_last_txc);
		m_sio->rxcb_w(m_last_txc);
		if (m_loopback) {
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(hp98046_io_card_device::sync_rx_im_w) , this) , state);
		}
	}
}

TIMER_CALLBACK_MEMBER(hp98046_io_card_device::sync_rx_im_w)
{
	m_rx_brg->im_w(param);
}

TIMER_CALLBACK_MEMBER(hp98046_io_card_device::sync_tx_im_w)
{
	m_tx_brg->im_w(param);
}
