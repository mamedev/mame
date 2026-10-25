// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, R. Belmont, David Haywood
/*******************************************************************************

    Happy Fish 2 302-in-1

    Adapted from mini2440.cpp

    The Linux kernel and initrd boot to the game menu, and the joystick, coin
    and start inputs work.

    Copy protection is a FameG FS8806 on the I2C bus at address 0x62, emulated
    in machine/fs8806.cpp.  The 8-byte DES key the chip and the host share is
    stored in the Linux kernel image, which has the FS8806 library linked into
    it: InitFS8806Lib() passes it in at 0xc0193d02, and the routine that talks
    to the chip lives around 0xc002d700.

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/fs8806.h"
#include "machine/nandflash.h"
#include "machine/s3c2440.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define LOG_GPIO    (1U << 1)
#define LOG_ADC     (1U << 2)
#define LOG_INPUTS  (1U << 3)

#define VERBOSE     (0)
#include "logmacro.h"


namespace {

class hapyfish_state : public driver_device
{
public:
	hapyfish_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2440(*this, "s3c2440"),
		m_fs8806(*this, "fs8806"),
		m_nand(*this, "nand"),
		m_nand2(*this, "nand2"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_inputs(*this, "INPUT%u", 0U)
	{ }

	void hapyfish(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<s3c2440_device> m_s3c2440;
	required_device<fs8806_device> m_fs8806;
	required_device<samsung_k9lag08u0m_device> m_nand, m_nand2;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_ioport_array<6> m_inputs;

	uint32_t m_port[9];

	uint8_t m_i2c_sda_in = 1;

	bool m_nand_select = false;

	uint8_t m_input_select = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t s3c2440_gpio_port_r(offs_t offset);
	void s3c2440_gpio_port_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c2440_core_pin_r(offs_t offset);
	void s3c2440_nand_command_w(uint8_t data);
	void s3c2440_nand_address_w(uint8_t data);
	uint8_t s3c2440_nand_data_r();
	void s3c2440_nand_data_w(uint8_t data);
	void s3c2440_i2s_data_w(offs_t offset, uint16_t data);
	uint32_t s3c2440_adc_data_r();
	void fs8806_sda_w(int state) { m_i2c_sda_in = state; }

	void hapyfish_map(address_map &map) ATTR_COLD;
};

/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// GPIO

uint32_t hapyfish_state::s3c2440_gpio_port_r(offs_t offset)
{
	uint32_t data = m_port[offset];
	switch (offset)
	{
		case S3C2440_GPIO_PORT_A:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO A: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_B:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO B: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_C:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO C: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_D:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO D: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_E:
			data = m_i2c_sda_in << 15;
			LOGMASKED(LOG_GPIO, "%s: Read GPIO E: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_F:
			data = 0xff;
			if (m_input_select < 6)
			{
				data = m_inputs[m_input_select]->read();
			}
			LOGMASKED(LOG_GPIO, "%s: Read GPIO F: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_G:
			data = (data & ~(1 << 13)) | (1 << 13); // [nand] 1 = 2048 byte page  (if ncon = 1)
			data = (data & ~(1 << 14)) | (1 << 14); // [nand] 1 = 5 address cycle (if ncon = 1)
			data = (data & ~(1 << 15)) | (0 << 15); // [nand] 0 = 8-bit bus width (if ncon = 1)
			data = data | 0x8E9; // for "load Image of Linux..."
			LOGMASKED(LOG_GPIO, "%s: Read GPIO G: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_H:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO H: %08x\n", machine().describe_context(), data);
			break;

		case S3C2440_GPIO_PORT_J:
			LOGMASKED(LOG_GPIO, "%s: Read GPIO J: %08x\n", machine().describe_context(), data);
			break;
	}
	return data;
}

void hapyfish_state::s3c2440_gpio_port_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// tout2/gb2 -> uda1341ts l3mode
	// tout3/gb3 -> uda1341ts l3data
	// tclk0/gb4 -> uda1341ts l3clock

	//printf("%08x to GPIO @ %x\n", data, offset);

	m_port[offset] = data;
	switch (offset)
	{
		case S3C2440_GPIO_PORT_A:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO A: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_B:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO B: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_C:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO C: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_D:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO D: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_E:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO E: %08x & %08x\n", machine().describe_context(), data, mem_mask);

			// FS8806 I2C on bits 15 (SDA) and 14 (SCL)
			if (BIT(mem_mask, 14))
			{
				m_fs8806->scl_write(BIT(data, 14));
			}
			if (BIT(mem_mask, 15))
			{
				m_fs8806->sda_write(BIT(data, 15));
			}

			if (mem_mask & 0x7e0)
			{
				uint8_t input_row_mask = ~(((data & mem_mask) >> 5) & 0x7f);
				for (m_input_select = 0; m_input_select < 8 && !BIT(input_row_mask, m_input_select); m_input_select++);
				LOGMASKED(LOG_INPUTS, "%s: Input select: Port %d\n", machine().describe_context(), m_input_select);
			}
			break;

		case S3C2440_GPIO_PORT_F:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO F: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_G:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO G: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_H:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO H: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;

		case S3C2440_GPIO_PORT_J:
			LOGMASKED(LOG_GPIO, "%s: Write GPIO J: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (BIT(mem_mask, 11))
			{
				m_nand_select = BIT(data, 11);
			}
			break;
	}
}

// CORE

/*

OM[1:0] = 00: Enable NAND flash memory boot

NCON: NAND flash memory selection (Normal / Advance)
0: Normal NAND flash (256Words/512Bytes page size, 3/4 address cycle)
1: Advance NAND flash (1KWords/2KBytes page size, 4/5 address cycle)

*/

uint32_t hapyfish_state::s3c2440_core_pin_r(offs_t offset)
{
	int data = 0;
	switch (offset)
	{
		case S3C2440_CORE_PIN_NCON : data = 1; break;
		case S3C2440_CORE_PIN_OM0  : data = 0; break;
		case S3C2440_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// NAND

void hapyfish_state::s3c2440_nand_command_w(uint8_t data)
{
	if (m_nand_select)
		m_nand->command_w(data);
	else
		m_nand2->command_w(data);
}

void hapyfish_state::s3c2440_nand_address_w(uint8_t data)
{
	if (m_nand_select)
		m_nand->address_w(data);
	else
		m_nand2->address_w(data);
}

uint8_t hapyfish_state::s3c2440_nand_data_r()
{
	if (m_nand_select)
		return m_nand->data_r();
	else
		return m_nand2->data_r();
}

void hapyfish_state::s3c2440_nand_data_w(uint8_t data)
{
	if (m_nand_select)
		m_nand->data_w(data);
	else
		m_nand2->data_w(data);
}

// I2S

void hapyfish_state::s3c2440_i2s_data_w(offs_t offset, uint16_t data)
{
	if (offset)
		m_ldac->write(data);
	else
		m_rdac->write(data);
}

// ADC

uint32_t hapyfish_state::s3c2440_adc_data_r()
{
	uint32_t data = 0;
	LOGMASKED(LOG_ADC, "%s: ADC data read: %08x\n", machine().describe_context(), data);
	return data;
}

// ...

void hapyfish_state::machine_start()
{
	m_nand_select = true; // select NAND #1 (S3C2440 bootloader will happen before machine_reset())
	m_input_select = 7;

	save_item(NAME(m_port));
	save_item(NAME(m_i2c_sda_in));
	save_item(NAME(m_nand_select));
	save_item(NAME(m_input_select));
}

void hapyfish_state::machine_reset()
{
	m_maincpu->reset();
	std::fill(std::begin(m_port), std::end(m_port), 0);
	m_nand_select = true; // select NAND #1
	m_i2c_sda_in = 1;
	m_input_select = 7;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void hapyfish_state::hapyfish_map(address_map &map)
{
	map(0x30000000, 0x37ffffff).ram();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void hapyfish_state::hapyfish(machine_config &config)
{
	ARM920T(config, m_maincpu, 100000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hapyfish_state::hapyfish_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1024, 768);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update("s3c2440", FUNC(s3c2440_device::screen_update));

	SPEAKER(config, "speaker", 2).front();
	UDA1341TS(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0); // uda1341ts.u12
	UDA1341TS(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1); // uda1341ts.u12

	S3C2440(config, m_s3c2440, 12000000);
	m_s3c2440->set_palette_tag("palette");
	m_s3c2440->set_screen_tag("screen");
	m_s3c2440->core_pin_r_callback().set(FUNC(hapyfish_state::s3c2440_core_pin_r));
	m_s3c2440->gpio_port_r_callback().set(FUNC(hapyfish_state::s3c2440_gpio_port_r));
	m_s3c2440->gpio_port_w_callback().set(FUNC(hapyfish_state::s3c2440_gpio_port_w));
	m_s3c2440->adc_data_r_callback().set(FUNC(hapyfish_state::s3c2440_adc_data_r));
	m_s3c2440->i2s_data_w_callback().set(FUNC(hapyfish_state::s3c2440_i2s_data_w));
	m_s3c2440->nand_command_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_command_w));
	m_s3c2440->nand_address_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_address_w));
	m_s3c2440->nand_data_r_callback().set(FUNC(hapyfish_state::s3c2440_nand_data_r));
	m_s3c2440->nand_data_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_data_w));

	FS8806(config, m_fs8806);
	m_fs8806->set_key(0x05598147'd51583c2ULL); // from InitFS8806Lib() in the kernel
	m_fs8806->sda_callback().set(FUNC(hapyfish_state::fs8806_sda_w));

	SAMSUNG_K9LAG08U0M(config, m_nand);
	m_nand->rnb_wr_callback().set(m_s3c2440, FUNC(s3c2440_device::frnb_w));

	SAMSUNG_K9LAG08U0M(config, m_nand2);
	m_nand2->rnb_wr_callback().set(m_s3c2440, FUNC(s3c2440_device::frnb_w));
}

static INPUT_PORTS_START( hapyfish )
	PORT_START("INPUT0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)      PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)    PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)    PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)   PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)          PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)          PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)          PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4)          PORT_PLAYER(1)

	PORT_START("INPUT1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)      PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)    PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)    PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)          PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)          PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)          PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4)          PORT_PLAYER(2)

	PORT_START("INPUT2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUT3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUT4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUT5")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Language ) ) PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Chinese ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("S2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("S2:3,4")
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( hapyfsh2 )
	ROM_REGION( 0x84000000, "nand", 0 )
	ROM_LOAD( "flash.u6",        0x00000000, 0x84000000, CRC(3aa364a2) SHA1(fe09f549a937ecaf8f7a859522a6635e272fe714) )

	ROM_REGION( 0x84000000, "nand2", 0 )
	ROM_LOAD( "flash.u28",        0x00000000, 0x84000000, CRC(f00a25cd) SHA1(9c33f8e26b84cea957d9c37fb83a686b948c6834) )
ROM_END

} // anonymous namespace

GAME( 201?, hapyfsh2, 0, hapyfish, hapyfish, hapyfish_state, empty_init, ROT0, "bootleg", "Happy Fish (V2 PCB, 302-in-1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
