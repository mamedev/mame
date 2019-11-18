// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98032.cpp

    98032 module (GPIO interface)

    Main reference for this module:
    HP, 98032A 16-bit Interface Installation and Service Manual

*********************************************************************/

#include "emu.h"
#include "98032.h"
#include "hp9885.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

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
DEFINE_DEVICE_TYPE(HP98032_IO_CARD, hp98032_io_card_device, "hp98032" , "HP98032 card")
DEFINE_DEVICE_TYPE(HP98032_GPIO_SLOT , hp98032_gpio_slot_device , "hp98032_gpio_slot" , "HP98032 GPIO slot")
DEFINE_DEVICE_TYPE(HP98032_GPIO_LOOPBACK , hp98032_gpio_loopback_device , "hp98032_loopback" , "HP98032 loopback connector")

// +----------------------+
// |hp98032_io_card_device|
// +----------------------+

hp98032_io_card_device::hp98032_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP98032_IO_CARD, tag, owner, clock)
	, device_hp9845_io_interface(mconfig, *this)
	, m_gpio(*this, "gpio")
{
}

hp98032_io_card_device::~hp98032_io_card_device()
{
}

void hp98032_io_card_device::device_add_mconfig(machine_config &config)
{
	HP98032_GPIO_SLOT(config , m_gpio , 0);
	m_gpio->pflg_cb().set(FUNC(hp98032_io_card_device::pflg_w));
	m_gpio->psts_cb().set(FUNC(hp98032_io_card_device::psts_w));
	m_gpio->eir_cb().set(FUNC(hp98032_io_card_device::eir_w));
}

static INPUT_PORTS_START(hp98032_port)
	PORT_HP9845_IO_SC(2)
INPUT_PORTS_END

ioport_constructor hp98032_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98032_port);
}

void hp98032_io_card_device::device_start()
{
	save_item(NAME(m_output));
	save_item(NAME(m_input));
	save_item(NAME(m_int_en));
	save_item(NAME(m_dma_en));
	save_item(NAME(m_busy));
	save_item(NAME(m_pready));
	save_item(NAME(m_flag));
	save_item(NAME(m_auto_ah));
	save_item(NAME(m_eir));
}

void hp98032_io_card_device::device_reset()
{
	// m_output is not reset
	// m_input is not reset
	m_int_en = false;
	m_dma_en = false;
	m_busy = true;  // Force reset
	m_auto_ah = false;
	set_busy(false);
	update_irq();
	update_dmar();
	m_gpio->preset_w(1);
	m_gpio->preset_w(0);
}

READ16_MEMBER(hp98032_io_card_device::reg_r)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
		// R4
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_C)) {
			latch_input_LSB();
		}
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_B) &&
			m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_A)) {
			latch_input_MSB();
		}
		res = m_input & 0x00ff;
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_B)) {
			res |= (m_input & 0xff00);
		}
		// Set direction to input
		m_gpio->io_w(0);
		if (m_auto_ah) {
			start_hs();
		}
		break;

	case 1:
		// R5
		BIT_SET(res , 5);
		res |= (m_gpio->ext_status_r() & 3);
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_2)) {
			BIT_SET(res , 2);
		}
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_1)) {
			BIT_SET(res , 3);
		}
		if (m_dma_en) {
			BIT_SET(res , 6);
		}
		if (m_int_en) {
			BIT_SET(res , 7);
		}
		break;

	case 2:
		// R6
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_B) &&
			m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_C)) {
			latch_input_LSB();
		}
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_A)) {
			latch_input_MSB();
		}
		res = m_input & 0xff00;
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_B)) {
			res |= (m_input & 0x00ff);
		}
		m_dma_en = false;
		update_irq();
		update_dmar();
		break;

	case 3:
		// R7: not mapped
	default:
		break;
	}

	LOG("rd R%u=%04x\n" , offset + 4 , res);
	return res;
}

WRITE16_MEMBER(hp98032_io_card_device::reg_w)
{
	LOG("wr R%u=%04x\n" , offset + 4 , data);

	switch (offset) {
	case 0:
		// R4
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_F)) {
			m_output = data;
		} else {
			m_output = (m_output & 0xff00) | (data & 0x00ff);
		}
		m_gpio->output_w(m_output);
		// Set direction to output
		m_gpio->io_w(1);
		if (m_auto_ah) {
			start_hs();
		}
		break;

	case 1:
		// R5
		m_gpio->ext_control_w(data & 3);
		if (BIT(data , 5)) {
			// Reset pulse
			device_reset();
		} else {
			m_auto_ah = BIT(data , 4);
			m_dma_en = BIT(data , 6);
			m_int_en = BIT(data , 7);
			update_irq();
			update_dmar();
		}
		break;

	case 2:
		// R6
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_F)) {
			m_output = data;
		} else {
			m_output = (m_output & 0x00ff) | (data & 0xff00);
		}
		m_gpio->output_w(m_output);
		// Set direction to output
		m_gpio->io_w(1);
		if (m_auto_ah) {
			start_hs();
		}
		m_dma_en = false;
		update_irq();
		update_dmar();
		break;

	case 3:
		// R7
		start_hs();
		break;

	default:
		break;
	}
}

WRITE_LINE_MEMBER(hp98032_io_card_device::pflg_w)
{
	bool prev_pready = m_pready;
	m_pready = state;
	if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_4)) {
		m_pready = !m_pready;
	}
	LOG("pready = %d\n" , m_pready);
	if (!prev_pready && m_pready) {
		// Going to ready state
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_9)) {
			latch_input_MSB();
		}
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_D)) {
			latch_input_LSB();
		}
	} else if (prev_pready && !m_pready) {
		// Going to not ready state
		set_busy(false);
	}
	if (prev_pready != m_pready) {
		update_flag();
	}
}

WRITE_LINE_MEMBER(hp98032_io_card_device::psts_w)
{
	bool sts = !state;
	if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_5)) {
		sts = !sts;
	}
	LOG("sts = %d\n" , sts);
	sts_w(sts);
}

WRITE_LINE_MEMBER(hp98032_io_card_device::eir_w)
{
	m_eir = state;
	LOG("eir = %d\n" , m_eir);
	update_irq();
}

void hp98032_io_card_device::start_hs()
{
	set_busy(true);
}

void hp98032_io_card_device::set_busy(bool state)
{
	LOG("busy = %d\n" , state);
	if (m_busy && !state) {
		// BUSY -> !BUSY
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_8)) {
			latch_input_MSB();
		}
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_E)) {
			latch_input_LSB();
		}
	}
	if (m_busy != state) {
		m_busy = state;
		update_flag();
		bool pctl = m_busy;
		if (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_3)) {
			pctl = !pctl;
		}
		LOG("pctl = %d\n" , pctl);
		m_gpio->pctl_w(pctl);
	}
}

void hp98032_io_card_device::update_flag()
{
	bool new_flag = !m_busy && (m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_6) || m_pready);
	if (new_flag != m_flag) {
		m_flag = new_flag;
		LOG("flag = %d\n" , m_flag);
		update_irq();
		update_dmar();
		flg_w(m_flag);
	}
}

void hp98032_io_card_device::update_irq()
{
	bool irq = m_int_en && (m_eir || (!m_dma_en && m_flag));
	LOG("irq = %d\n" , irq);
	irq_w(irq);
}

void hp98032_io_card_device::update_dmar()
{
	bool dmar = m_gpio->is_jumper_present(hp98032_gpio_slot_device::JUMPER_7) && m_flag && m_dma_en;
	LOG("dmar = %d\n" , dmar);
	dmar_w(dmar);
}

void hp98032_io_card_device::latch_input_MSB()
{
	m_input = (m_input & 0x00ff) | (m_gpio->input_r() & 0xff00);
}

void hp98032_io_card_device::latch_input_LSB()
{
	m_input = (m_input & 0xff00) | (m_gpio->input_r() & 0x00ff);
}

// +------------------------+
// |hp98032_gpio_slot_device|
// +------------------------+

hp98032_gpio_slot_device::hp98032_gpio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP98032_GPIO_SLOT , tag , owner , clock)
	, device_single_card_slot_interface<device_hp98032_gpio_interface>(mconfig , *this)
	, m_pflg_handler(*this)
	, m_psts_handler(*this)
	, m_eir_handler(*this)
{
	option_reset();
	option_add("loopback" , HP98032_GPIO_LOOPBACK);
	option_add("hp9885" , HP9885);
	set_default_option(nullptr);
	set_fixed(false);
}

hp98032_gpio_slot_device::~hp98032_gpio_slot_device()
{
}

uint16_t hp98032_gpio_slot_device::get_jumpers() const
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		return card->get_jumpers();
	} else {
		return 0;
	}
}

uint16_t hp98032_gpio_slot_device::input_r() const
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		return card->input_r();
	} else {
		return 0;
	}
}

uint8_t hp98032_gpio_slot_device::ext_status_r() const
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		return card->ext_status_r();
	} else {
		return 0;
	}
}

void hp98032_gpio_slot_device::output_w(uint16_t data)
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		card->output_w(data);
	}
}

void hp98032_gpio_slot_device::ext_control_w(uint8_t data)
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		card->ext_control_w(data);
	}
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::pflg_w)
{
	m_pflg_handler(state);
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::psts_w)
{
	m_psts_handler(state);
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::eir_w)
{
	m_eir_handler(state);
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::pctl_w)
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		card->pctl_w(state);
	}
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::io_w)
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		card->io_w(state);
	}
}

WRITE_LINE_MEMBER(hp98032_gpio_slot_device::preset_w)
{
	device_hp98032_gpio_interface *card = get_card_device();
	if (card != nullptr) {
		card->preset_w(state);
	}
}

void hp98032_gpio_slot_device::device_start()
{
	m_pflg_handler.resolve_safe();
	m_psts_handler.resolve_safe();
	m_eir_handler.resolve_safe();
}

void hp98032_gpio_slot_device::device_reset()
{
	// When nothing is connected to GPIO, set input signals to 0
	if (get_card_device() == nullptr) {
		m_pflg_handler(0);
		m_psts_handler(0);
		m_eir_handler(0);
	}
}

// +-----------------------------+
// |device_hp98032_gpio_interface|
// +-----------------------------+

device_hp98032_gpio_interface::device_hp98032_gpio_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "hp98032gpio")
{
}

device_hp98032_gpio_interface::~device_hp98032_gpio_interface()
{
}

WRITE_LINE_MEMBER(device_hp98032_gpio_interface::pflg_w)
{
	hp98032_gpio_slot_device *slot = downcast<hp98032_gpio_slot_device*>(device().owner());
	slot->pflg_w(state);
}

WRITE_LINE_MEMBER(device_hp98032_gpio_interface::psts_w)
{
	hp98032_gpio_slot_device *slot = downcast<hp98032_gpio_slot_device*>(device().owner());
	slot->psts_w(state);
}

WRITE_LINE_MEMBER(device_hp98032_gpio_interface::eir_w)
{
	hp98032_gpio_slot_device *slot = downcast<hp98032_gpio_slot_device*>(device().owner());
	slot->eir_w(state);
}

// +----------------------------+
// |hp98032_gpio_loopback_device|
// +----------------------------+

hp98032_gpio_loopback_device::hp98032_gpio_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP98032_GPIO_LOOPBACK, tag, owner, clock)
	, device_hp98032_gpio_interface(mconfig, *this)
{
}

hp98032_gpio_loopback_device::~hp98032_gpio_loopback_device()
{
}

uint16_t hp98032_gpio_loopback_device::get_jumpers() const
{
	return hp98032_gpio_slot_device::JUMPER_1 |
		hp98032_gpio_slot_device::JUMPER_2 |
		hp98032_gpio_slot_device::JUMPER_4 |
		hp98032_gpio_slot_device::JUMPER_7 |
		hp98032_gpio_slot_device::JUMPER_A |
		hp98032_gpio_slot_device::JUMPER_B |
		hp98032_gpio_slot_device::JUMPER_C |
		hp98032_gpio_slot_device::JUMPER_F;
}

uint16_t hp98032_gpio_loopback_device::input_r() const
{
	return m_output;
}

uint8_t hp98032_gpio_loopback_device::ext_status_r() const
{
	uint8_t res = m_ext_control;
	if (m_io) {
		BIT_SET(res , 0);
	}
	return res;
}

void hp98032_gpio_loopback_device::output_w(uint16_t data)
{
	m_output = data;
}

void hp98032_gpio_loopback_device::ext_control_w(uint8_t data)
{
	m_ext_control = data;
}

WRITE_LINE_MEMBER(hp98032_gpio_loopback_device::pctl_w)
{
	pflg_w(state);
}

WRITE_LINE_MEMBER(hp98032_gpio_loopback_device::io_w)
{
	m_io = state;
}

WRITE_LINE_MEMBER(hp98032_gpio_loopback_device::preset_w)
{
	eir_w(state);
}

void hp98032_gpio_loopback_device::device_start()
{
}

void hp98032_gpio_loopback_device::device_reset()
{
	psts_w(0);
}
