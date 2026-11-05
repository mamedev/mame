// license:BSD-3-Clause
// copyright-holders:David Haywood
//
// A generic-looking S3C2410 SoC setup, with very little of note in the system.
// Presently decompresses and boots a stripped-down Linux 2.6.11 kernel,
// displays the initial logos and plays the initial boot sound, then goes
// to a black screen and hangs.
//
// It prints Linux console messages via the S3C2410's UART if UART_PRINTF
// is uncommented in src/devices/machine/s3c24xx.cpp.
//
// Potentially-useful information printed to the console include:
//
// USB Power Control, (c) 2005 Grey Innovation
// Battery Driver loaded for Digiblast
// hub 1-0:1.0: USB hub found
// hub 1-0:1.0: 2 ports detected
// cpia2: V4L-Driver for Vision CPiA2 based cameras v1.0.0
// usbcore: registered new driver cpia2
// s3c2410 usb gadget driver loaded. Version 1.3.6 Grey Innovation
// mice: PS/2 mouse device common for all mice
// Probing Grey-keypad...
//
// It appears that inputs are handled through the S3C2410's USB host
// hardware, although the exact nature of the OHCI device that it's
// presented with is unknown.
//
// It's not yet known why the emulated unit goes to a black screen
// after beginning to load a cartridge - it may be due to the relevant
// input device not being detected as connected over USB, or some
// sort of battery-level reporting that is not yet hooked up.
//
// As a historical note, the Linux kernel also starts a USB module,
// the "V4L-Driver for Vision CPiA2 based camera v1.0.0". It's known
// that there were plans at some point to release a TV tuner for the
// digiBLAST, and perhaps it was handled by having the TV tuner hardware
// masquerade as a camera over USB.

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"
#include "machine/i2cmem.h"
#include "machine/s3c24xx.h"
#include "sound/cs43l43.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define LOG_NAND	(1U << 1)
#define LOG_ADC		(1U << 2)
#define LOG_GPIO	(1U << 3)
#define LOG_I2C		(1U << 4)
#define VERBOSE ( LOG_GPIO | LOG_I2C )
#include "logmacro.h"

namespace {

class digiblast_state : public driver_device
{
public:
	digiblast_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2410(*this, "s3c2410"),
		m_i2cmem(*this, "i2cmem"),
		m_i2sdac(*this, "i2sdac"),
		m_system_memory(*this, "systememory"),
		m_cart(*this, "cartslot"),
		m_dac_sda(0)
	{
	}

	void digiblast(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void main_map(address_map &map) ATTR_COLD;

	uint32_t adc_data_r(offs_t offset);
	void i2c_scl_w(int state);
	void i2c_sda_cb(int state);
	int i2c_sda_r();
	void i2c_sda_w(int state);
	uint32_t gpio_port_r(offs_t offset);
	void nand_command_w(uint8_t data);
	void nand_address_w(uint8_t data);
	uint8_t nand_data_r();
	void nand_data_w(uint8_t data);
	int nand_update_ecc_r();

	required_device<cpu_device> m_maincpu;
	required_device<s3c2410_device> m_s3c2410;
	required_device<i2cmem_device> m_i2cmem;
	required_device<cs43l43_device> m_i2sdac;
	required_shared_ptr<uint32_t> m_system_memory;

	required_device<generic_slot_device> m_cart;

	enum nand_mode_t
	{
		NAND_M_RESET,       // initial state
		NAND_M_READ_ID,     // read ID
		NAND_M_READ_PAGE    // read page data
	};

	struct nand_t
	{
		nand_mode_t mode{};
		int page_addr = 0;
		int byte_addr = 0;
		int addr_load_ptr = 0;
		uint8_t out_fifo[16];
		uint8_t out_read_idx = 0;
	};

	struct nand_t m_nand;
	int m_dac_sda;
};

void digiblast_state::main_map(address_map &map)
{
	map(0x30000000, 0x30ffffff).ram().share("systememory").mirror(0x02000000);
	map(0x40000000, 0x5fffffff).m(m_s3c2410, FUNC(s3c2410_device::map));
	map(0x80000000, 0x8fffffff).r("cartslot", FUNC(generic_slot_device::read_rom));
}

static INPUT_PORTS_START( digiblast )
INPUT_PORTS_END

void digiblast_state::machine_start()
{
	save_item(NAME(m_dac_sda));
}

void digiblast_state::machine_reset()
{
	m_dac_sda = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(digiblast_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

uint32_t digiblast_state::adc_data_r(offs_t offset)
{
	uint32_t data = 0;
	switch (offset)
	{
	default:
		data = 0x3ff;
		break;
	}
	LOGMASKED(LOG_ADC, "%s: adc_data_r(%d): %08x\n", machine().describe_context(), offset, data);
	return data;
}

void digiblast_state::i2c_scl_w(int state)
{
	LOGMASKED(LOG_I2C, "%s: i2c_scl_w: %d\n", machine().describe_context(), state);
	m_i2cmem->write_scl(state);
	m_i2sdac->scl_write(state);
}

void digiblast_state::i2c_sda_cb(int state)
{
	m_dac_sda = state;
}

int digiblast_state::i2c_sda_r()
{
	const bool nvram_state = m_i2cmem->read_sda();
	const bool state = nvram_state || m_dac_sda;
	LOGMASKED(LOG_I2C, "%s: i2c_sda_r: %d\n", machine().describe_context(), state);
	return state;
}

void digiblast_state::i2c_sda_w(int state)
{
	LOGMASKED(LOG_I2C, "%s: i2c_sda_w: %d\n", machine().describe_context(), state);
	m_i2cmem->write_sda(state);
	m_i2sdac->sda_write(state);
}

uint32_t digiblast_state::gpio_port_r(offs_t offset)
{
	uint32_t data = 0;
	switch (offset)
	{
		case s3c24xx_device::GPIO_PORT_A:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port A: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_B:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port B: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_C:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port C: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_D:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port D: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_E:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port E: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_F:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port F: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_G:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port G: %08x\n", machine().describe_context(), data);
			break;
		case s3c24xx_device::GPIO_PORT_H:
			LOGMASKED(LOG_GPIO, "%s: gpio_port_r port H: %08x\n", machine().describe_context(), data);
			break;
	}
	return data;
}

int digiblast_state::nand_update_ecc_r()
{
	return m_nand.mode == NAND_M_READ_ID ? 0 : 1;
}

void digiblast_state::nand_command_w(uint8_t data)
{
	struct nand_t &nand = m_nand;
	switch (data)
	{
		case 0xff:
			LOGMASKED(LOG_NAND, "nand_command_w: Reset\n");
			nand.mode = NAND_M_RESET;
			m_s3c2410->frnb_w(1);
			break;

		case 0x90:
			LOGMASKED(LOG_NAND, "nand_command_w: Read ID\n");
			nand.mode = NAND_M_READ_ID;
			break;

		case 0x00:
			LOGMASKED(LOG_NAND, "nand_command_w: Read Page\n");
			nand.mode = NAND_M_READ_PAGE;
			nand.page_addr = 0;
			nand.addr_load_ptr = 0;
			break;

		default:
			LOGMASKED(LOG_NAND, "nand_command_w: %02x\n", data);
			break;
	}
}

void digiblast_state::nand_address_w(uint8_t data)
{
	struct nand_t &nand = m_nand;
	switch (nand.mode)
	{
	case NAND_M_RESET:
		LOGMASKED(LOG_NAND, "nand_address_w: Unexpected write during Reset (%02x)\n", data);
		break;

	case NAND_M_READ_ID:
		LOGMASKED(LOG_NAND, "nand_address_w: Dummy byte for Read ID (%02x)\n", data);
		nand.out_fifo[0] = 0xff;
		nand.out_fifo[1] = 0x00;
		nand.out_fifo[2] = 0x76;
		nand.out_fifo[3] = 0x98;
		nand.out_read_idx = 4;
		break;

	case NAND_M_READ_PAGE:
		LOGMASKED(LOG_NAND, "nand_address_w: Read Page, Address Byte: %02x\n", data);
		if (nand.addr_load_ptr == 0)
		{
			nand.byte_addr = data;
		}
		else
		{
			nand.page_addr = (nand.page_addr & ~(0xFF << ((nand.addr_load_ptr - 1) * 8))) | (data << ((nand.addr_load_ptr - 1) * 8));
		}
		nand.addr_load_ptr++;
		if ((nand.mode == NAND_M_READ_PAGE) && (nand.addr_load_ptr == 4))
		{
			m_s3c2410->frnb_w(0);
			m_s3c2410->frnb_w(1);
		}
		break;

	default:
		LOGMASKED(LOG_NAND, "nand_address_w: Unknown, %02x\n", data);
		break;
	}
}

uint8_t digiblast_state::nand_data_r()
{
	struct nand_t &nand = m_nand;
	uint8_t data = 0;
	switch (nand.mode)
	{
	case NAND_M_RESET:
		LOGMASKED(LOG_NAND, "nand_data_r: Unexpected data read during Reset\n");
		break;

	case NAND_M_READ_ID:
		if (nand.out_read_idx == 0)
		{
			LOGMASKED(LOG_NAND, "nand_data_r: Unexpected data read when no data prepared\n", data);
			nand.mode = NAND_M_RESET;
			break;
		}
		nand.out_read_idx--;
		data = nand.out_fifo[nand.out_read_idx];
		LOGMASKED(LOG_NAND, "%s: nand_data_r: Data byte %d read: %02x\n", machine().describe_context(), (3 - nand.out_read_idx) + 1, data);
		if (nand.out_read_idx == 0)
			nand.mode = NAND_M_RESET;
		break;

	case NAND_M_READ_PAGE:
		if (nand.byte_addr < 0x200)
		{
			data = *(m_cart->get_rom_base() + nand.page_addr * 0x210 + nand.byte_addr);
		}
		else
		{
			if ((nand.byte_addr >= 0x200) && (nand.byte_addr < 0x204))
			{
				uint8_t mecc[4];
				m_s3c2410->nand_calculate_mecc(m_cart->get_rom_base() + nand.page_addr * 0x210, 0x200, mecc);
				data = mecc[nand.byte_addr-0x200];
			}
			else
			{
				data = 0xFF;
			}
		}
		nand.byte_addr++;
		if (nand.byte_addr == 0x210)
		{
			nand.byte_addr = 0;
			nand.page_addr++;
			if (nand.page_addr == 0x10000) nand.page_addr = 0;
		}
		break;
	}
	return data;
}

void digiblast_state::nand_data_w(uint8_t data)
{
}

void digiblast_state::digiblast(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &digiblast_state::main_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(455, 262);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update("s3c2410", FUNC(s3c2410_device::screen_update));

	PALETTE(config, "palette").set_entries(256);

	S3C2410(config, m_s3c2410, 12000000);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	m_s3c2410->om0_r_callback().set_constant(0); // OM[1:0] = 00: Enable NAND flash memory boot
	m_s3c2410->om1_r_callback().set_constant(0);
	m_s3c2410->ncon_r_callback().set_constant(1); // NCON = 1: Advance NAND flash (1KWords/2KBytes page size, 4/5 address cycle)
	m_s3c2410->adc_data_r_callback().set(FUNC(digiblast_state::adc_data_r));
	m_s3c2410->i2c_scl_w_callback().set(FUNC(digiblast_state::i2c_scl_w));
	m_s3c2410->i2c_sda_r_callback().set(FUNC(digiblast_state::i2c_sda_r));
	m_s3c2410->i2c_sda_w_callback().set(FUNC(digiblast_state::i2c_sda_w));
	m_s3c2410->i2s_data_w_callback().set(m_i2sdac, FUNC(cs43l43_device::write_i2s));
	m_s3c2410->i2s_clockchange_w_callback().set(m_i2sdac, FUNC(cs43l43_device::set_frequency));
	m_s3c2410->gpio_port_r_callback().set(FUNC(digiblast_state::gpio_port_r));
	m_s3c2410->nand_command_w_callback().set(FUNC(digiblast_state::nand_command_w));
	m_s3c2410->nand_address_w_callback().set(FUNC(digiblast_state::nand_address_w));
	m_s3c2410->nand_update_ecc_r_callback().set(FUNC(digiblast_state::nand_update_ecc_r));
	m_s3c2410->nand_data_r_callback().set(FUNC(digiblast_state::nand_data_r));
	m_s3c2410->nand_data_w_callback().set(FUNC(digiblast_state::nand_data_w));

	I2C_24C16(config, m_i2cmem, 0);
	m_i2cmem->set_address(0xa0);
	m_i2cmem->set_e0(0);
	m_i2cmem->set_e1(0);
	m_i2cmem->set_e2(0);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "digiblast_cart");
	m_cart->set_width(GENERIC_ROM8_WIDTH);
	m_cart->set_device_load(FUNC(digiblast_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("digiblast_cart");

	/* sound hardware */
	SPEAKER(config, "speakerl").front_left();
	SPEAKER(config, "speakerr").front_right();

	CS43L43(config, m_i2sdac, 0);
	m_i2sdac->sda_callback().set(FUNC(digiblast_state::i2c_sda_cb));
	m_i2sdac->add_route(0, "speakerl", 0.7);
	m_i2sdac->add_route(1, "speakerr", 0.7);
}

ROM_START( digiblst )
	//ROM_REGION( 0x4200000, "flash", ROMREGION_ERASEFF )
ROM_END

} // Anonymous namespace

CONS( 2005, digiblst,  0, 0, digiblast,  digiblast,  digiblast_state, empty_init, "Nikko Entertainment B.V. / Grey Innovation", "digiBLAST", MACHINE_NOT_WORKING )
