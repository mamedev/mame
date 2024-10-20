// license:GPL-2.0+
// copyright-holders:smf
/*
  Fujitsu 32MB Flash Card
  -----------------------

  Front

  |----PCMCIA CONNECTOR-----|
  |                         |
  | HT04A MB624018 MB624019 |
  | AT28C16                 |
  |                         |
  | 29F017A.1L   29F017A.1U |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.2L   29F017A.2U |
  | 90PFTN       90PFTR     |
  |                         |
  | 29F017A.3L   29F017A.3U |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.4L   29F017A.4U |
  | 90PFTN       90PFTR     |
  |                         |
  |------------------SWITCH-|

  Back

  |----PCMCIA CONNECTOR-----|
  |                         |
  |                         |
  |                         |
  |                         |
  | 29F017A.5U   29F017A.5L |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.6U   29F017A.6L |
  | 90PFTN       90PFTR     |
  |                         |
  | 29F017A.7U   29F017A.7L |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.8U   29F017A.8L |
  | 90PFTN       90PFTR     |
  |                         |
  |-SWITCH------------------|

  Texas Instruments HT04A
  Fujitsu MB624018 CMOS GATE ARRAY
  Fujitsu MB624019 CMOS GATE ARRAY
  Atmel AT28C16 16K (2K x 8) Parallel EEPROM
  Fujitsu 29F017A-90PFTR 16M (2M x 8) BIT Flash Memory Reverse Pinout (Gachaga Champ card used 29F017-12PFTR instead)
  Fujitsu 29F017A-90PFTN 16M (2M x 8) BIT Flash Memory Standard Pinout

*/

#include "emu.h"
#include "linflash.h"

#define LOG_READ (1U << 1)
#define LOG_WRITE (1U << 2)

//#define VERBOSE (LOG_READ | LOG_WRITE)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREAD(...) LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGWRITE(...) LOGMASKED(LOG_WRITE, __VA_ARGS__)

linear_flash_pccard_device::linear_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor amap) :
	device_t(mconfig, type, tag, owner, clock),
	device_pccard_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_space_config("memory", ENDIANNESS_LITTLE, 16, 26, 0, amap),
	m_wp(0)
{
}

namespace {

static INPUT_PORTS_START(linflash)
	PORT_START("CONF")
	PORT_CONFNAME(0x01, 0x00, "Write Protect")  PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(linear_flash_pccard_device::update_wp))
	PORT_CONFSETTING(   0x01, DEF_STR(Yes))
	PORT_CONFSETTING(   0x00, DEF_STR(No))
INPUT_PORTS_END

}; // anonymous namespace

ioport_constructor linear_flash_pccard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(linflash);
}

void linear_flash_pccard_device::device_start()
{
	m_space = &space(0);
	m_cd1_cb(0);
	m_cd2_cb(0);

	// Correct for ID245P01, TODO: check other cards
	m_bvd1_cb(1);
	m_bvd2_cb(1);

	save_item(NAME(m_wp));

	m_wp_cb(m_wp);
}

device_memory_interface::space_config_vector linear_flash_pccard_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_space_config) };
}

uint16_t linear_flash_pccard_device::read_memory(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_space->read_word(offset * 2, mem_mask);
	LOGREAD("%s read_memory(%08x, %08x) %08x\n", machine().describe_context(), offset, mem_mask, data);
	return data;
}

void linear_flash_pccard_device::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGWRITE("%s write_memory(%08x, %08x, %08x)%s\n", machine().describe_context(), offset, data, mem_mask, m_wp ? " (write protected)" : "");
	if (!m_wp)
		m_space->write_word(offset * 2, data, mem_mask);
}

void linear_flash_pccard_device::update_wp(int state)
{
	if (m_wp != state)
	{
		m_wp = state;
		m_wp_cb(m_wp);
	}
}


template<unsigned N>
linear_flash_pccard_8bit_device<N>::linear_flash_pccard_8bit_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(linear_flash_pccard_8bit_device<N>::amap), this)),
	m_l(*this, "%ul", 1U),
	m_u(*this, "%uu", 1U)
{
}

template<unsigned N>
void linear_flash_pccard_8bit_device<N>::amap(address_map &map)
{
	map.global_mask((N * 0x00400000) - 1);

	for (size_t i = 0; i < N; i++)
	{
		map(0x00400000 * i, (0x00400000 * (i + 1)) - 1).rw(m_l[i], FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
		map(0x00400000 * i, (0x00400000 * (i + 1)) - 1).rw(m_u[i], FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	}
}


template<unsigned N>
linear_flash_pccard_16bit_device<N>::linear_flash_pccard_16bit_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(linear_flash_pccard_16bit_device<N>::amap), this)),
	m_flash(*this, "flash.%u", 1U)
{
}

template<unsigned N>
void linear_flash_pccard_16bit_device<N>::amap(address_map &map)
{
	map.global_mask((m_flash.size() * 0x00800000) - 1);

	for (size_t i = 0; i < N; i++)
		map(0x00800000 * i, (0x00800000 * (i + 1)) - 1).rw(m_flash[i], FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
}


template<unsigned N>
linear_flash_pccard_29f017a_device<N>::linear_flash_pccard_29f017a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_8bit_device<N>(mconfig, type, tag, owner, clock)
{
}

template<unsigned N>
void linear_flash_pccard_29f017a_device<N>::device_add_mconfig(machine_config &config)
{
	for (size_t i = 0; i < N; i++)
	{
		FUJITSU_29F016A(config, linear_flash_pccard_8bit_device<N>::m_l[i]);
		FUJITSU_29F016A(config, linear_flash_pccard_8bit_device<N>::m_u[i]);
	}
}


template<unsigned N>
linear_flash_pccard_lh28f016s_device<N>::linear_flash_pccard_lh28f016s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_8bit_device<N>(mconfig, type, tag, owner, clock)
{
}


template<unsigned N>
void linear_flash_pccard_lh28f016s_device<N>::device_add_mconfig(machine_config &config)
{
	for (size_t i = 0; i < N; i++)
	{
		SHARP_LH28F016S(config, linear_flash_pccard_8bit_device<N>::m_l[i]);
		SHARP_LH28F016S(config, linear_flash_pccard_8bit_device<N>::m_u[i]);
	}
}


template<unsigned N>
linear_flash_pccard_28f640j5_device<N>::linear_flash_pccard_28f640j5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_16bit_device<N>(mconfig, type, tag, owner, clock)
{
}

template<unsigned N>
void linear_flash_pccard_28f640j5_device<N>::device_add_mconfig(machine_config &config)
{
	for (size_t i = 0; i < N; i++)
		INTEL_28F640J5(config, linear_flash_pccard_16bit_device<N>::m_flash[i]);
}

fujitsu_16mb_flash_card_device::fujitsu_16mb_flash_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_29f017a_device(mconfig, FUJITSU_16MB_FLASH_CARD, tag, owner, clock)
{
}

fujitsu_32mb_flash_card_device::fujitsu_32mb_flash_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_29f017a_device(mconfig, FUJITSU_32MB_FLASH_CARD, tag, owner, clock)
{
}

id245p01_device::id245p01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_lh28f016s_device(mconfig, ID245P01, tag, owner, clock)
{
}

pm24276_device::pm24276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_28f640j5_device(mconfig, PM24276, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(FUJITSU_16MB_FLASH_CARD, fujitsu_16mb_flash_card_device, "fujitsu_16mb_flash_card", "Fujitsu 16MB Flash Card")
DEFINE_DEVICE_TYPE(FUJITSU_32MB_FLASH_CARD, fujitsu_32mb_flash_card_device, "fujitsu_32mb_flash_card", "Fujitsu 32MB Flash Card")
DEFINE_DEVICE_TYPE(ID245P01, id245p01_device, "id245p01", "Sharp ID245P01 32MB Linear Flash Memory Card")
DEFINE_DEVICE_TYPE(PM24276, pm24276_device, "pm24276", "PM24276 32MB Linear Flash Memory Card")
