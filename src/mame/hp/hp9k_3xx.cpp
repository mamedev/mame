// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  hp9k3xx.c: preliminary driver for HP9000 300 Series (aka HP9000/3xx)
  By R. Belmont

  TODO: Add DIO/DIO-II slot capability and modularize the video cards

  Currently supporting:

  310:
      MC68010 CPU @ 10 MHz
      HP custom MMU

  320:
      MC68020 CPU @ 16.67 MHz
      HP custom MMU
      MC68881 FPU

  330:
      MC68020 CPU @ 16.67 MHz
      MC68851 MMU
      MC68881 FPU

  340:
      MC68030 CPU @ 16.67 MHz w/built-in MMU
      MC68881 FPU

  360:
      MC68030 CPU @ 25 MHz w/built-in MMU
      MC68882 FPU

  370:
      MC68030 CPU @ 33 MHz w/built-in MMU
      MC68881 FPU

  380:
    MC68040 CPU @ 25 MHz w/built-in MMU and FPU

  382:
    MC68040 CPU @ 25? MHz w/built-in MMU and FPU
    Built-in VGA compatible video

  All models have an MC6840 PIT on IRQ6 clocked at 250 kHz.

  TODO:
    BBCADDR   0x420000
    RTC_DATA: 0x420001
    RTC_CMD:  0x420003
    HIL:      0x428000
    HPIB:     0x470000
    KBDNMIST: 0x478005
    DMA:      0x500000
    FRAMEBUF: 0x560000

    6840:     0x5F8001/3/5/7/9, IRQ 6

****************************************************************************/

#include "emu.h"
#include "logmacro.h"
#include "cpu/m68000/m68010.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68040.h"
#include "machine/6840ptm.h"
#include "bus/hp_dio/hp_dio.h"

#include "screen.h"
#include "softlist_dev.h"
#include "hp9k_3xx.lh"


namespace {

#define MAINCPU_TAG "maincpu"
#define PTM6840_TAG "ptm"

class hp9k3xx_state : public driver_device
{
public:
	hp9k3xx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_diag_led(*this, "led_diag_%u", 0U)
	{ }

	void hp9k310(machine_config &config);
	void hp9k320(machine_config &config);
	void hp9k330(machine_config &config);
	void hp9k332(machine_config &config);
	void hp9k340(machine_config &config);
	void hp9k360(machine_config &config);
	void hp9k370(machine_config &config);
	void hp9k380(machine_config &config);
	void hp9k382(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

private:
	void hp9k300(machine_config &config);
	required_device<m68000_musashi_device> m_maincpu;

	output_finder<8> m_diag_led;

	void set_bus_error(uint32_t address, bool write, uint16_t mem_mask);

	uint16_t buserror16_r(offs_t offset, uint16_t mem_mask = ~0);
	void buserror16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t buserror_r(offs_t offset, uint32_t mem_mask = ~0);
	void buserror_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void led_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hp9k310_map(address_map &map) ATTR_COLD;
	void hp9k320_map(address_map &map) ATTR_COLD;
	void hp9k330_map(address_map &map) ATTR_COLD;
	void hp9k332_map(address_map &map) ATTR_COLD;
	void hp9k360_map(address_map &map) ATTR_COLD;
	void hp9k370_map(address_map &map) ATTR_COLD;
	void hp9k380_map(address_map &map) ATTR_COLD;
	void hp9k382_map(address_map &map) ATTR_COLD;
	void hp9k3xx_common(address_map &map) ATTR_COLD;

	void add_dio16_bus(machine_config &mconfig);
	void add_dio32_bus(machine_config &mconfig);

	void dio_irq1_w(int state) { m_maincpu->set_input_line(M68K_IRQ_1, state); }
	void dio_irq2_w(int state) { m_maincpu->set_input_line(M68K_IRQ_2, state); }
	void dio_irq3_w(int state) { m_maincpu->set_input_line(M68K_IRQ_3, state); }
	void dio_irq4_w(int state) { m_maincpu->set_input_line(M68K_IRQ_4, state); }
	void dio_irq5_w(int state) { m_maincpu->set_input_line(M68K_IRQ_5, state); }
	void dio_irq6_w(int state) { m_maincpu->set_input_line(M68K_IRQ_6, state); }
	void dio_irq7_w(int state) { m_maincpu->set_input_line(M68K_IRQ_7, state); }

	bool m_bus_error = false;
	emu_timer *m_bus_error_timer = nullptr;
	TIMER_CALLBACK_MEMBER(bus_error_timeout);
};

// shared mappings for all 9000/3xx systems
void hp9k3xx_state::hp9k3xx_common(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(hp9k3xx_state::buserror_r), FUNC(hp9k3xx_state::buserror_w));
	map(0x00000000, 0x0001ffff).rom().region("maincpu", 0).w(FUNC(hp9k3xx_state::led_w));  // writes to 1fffc are the LED
	map(0x005f4000, 0x005f400f).ram(); // somehow coprocessor related - bootrom crashes if not present
	map(0x005f8000, 0x005f800f).rw(PTM6840_TAG, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask32(0x00ff00ff);

}

// 9000/310 - has onboard video that the graphics card used in other 3xxes conflicts with
void hp9k3xx_state::hp9k310_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(hp9k3xx_state::buserror16_r), FUNC(hp9k3xx_state::buserror16_w));
	map(0x000000, 0x01ffff).rom().region("maincpu", 0).w(FUNC(hp9k3xx_state::led_w));  // writes to 1fffc are the LED
	map(0x5f4000, 0x5f400f).ram(); // somehow coprocessor related - bootrom crashes if not present
	map(0x5f8000, 0x5f800f).rw(PTM6840_TAG, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x800000, 0xffffff).ram();
}

// 9000/320
void hp9k3xx_state::hp9k320_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xfff00000, 0xffffffff).ram();
}

// 9000/330 and 9000/340
void hp9k3xx_state::hp9k330_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xffc00000, 0xffffffff).ram();
}

// 9000/332, with built-in medium-res video
void hp9k3xx_state::hp9k332_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xffc00000, 0xffffffff).ram();
}

// 9000/360 - 16 MB RAM to run HP/UX
void hp9k3xx_state::hp9k360_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xff000000, 0xffffffff).ram();
}

// 9000/370 - with 48 MB RAM (max. configuration)
void hp9k3xx_state::hp9k370_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xfd000000, 0xffffffff).ram();
}

// 9000/380 - '040
void hp9k3xx_state::hp9k380_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xff800000, 0xffffffff).ram();
}

// 9000/382 - onboard VGA compatible video (where?)
void hp9k3xx_state::hp9k382_map(address_map &map)
{
	hp9k3xx_common(map);
	// main memory
	map(0xffc00000, 0xffffffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( hp9k330 )
INPUT_PORTS_END

void hp9k3xx_state::driver_start()
{
	m_diag_led.resolve();
}

void hp9k3xx_state::machine_start()
{
	m_bus_error_timer = timer_alloc(FUNC(hp9k3xx_state::bus_error_timeout), this);
	m_bus_error = false;
	save_item(NAME(m_bus_error));
}

TIMER_CALLBACK_MEMBER(hp9k3xx_state::bus_error_timeout)
{
	m_bus_error = false;
}

void hp9k3xx_state::led_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!(mem_mask & 0xff))
	   return;

	LOG("LED: %02x\n", data & 0xff);

	m_diag_led[0] = BIT(data, 0);
	m_diag_led[1] = BIT(data, 1);
	m_diag_led[2] = BIT(data, 2);
	m_diag_led[3] = BIT(data, 3);
	m_diag_led[4] = BIT(data, 4);
	m_diag_led[5] = BIT(data, 5);
	m_diag_led[6] = BIT(data, 6);
	m_diag_led[7] = BIT(data, 7);
}

void hp9k3xx_state::add_dio16_bus(machine_config &config)
{
	bus::hp_dio::dio16_device &dio16(DIO16(config, "diobus", 0));
	dio16.set_program_space(m_maincpu, AS_PROGRAM);
	m_maincpu->reset_cb().set(dio16, FUNC(bus::hp_dio::dio16_device::reset_in));

	dio16.irq1_out_cb().set(FUNC(hp9k3xx_state::dio_irq1_w));
	dio16.irq2_out_cb().set(FUNC(hp9k3xx_state::dio_irq2_w));
	dio16.irq3_out_cb().set(FUNC(hp9k3xx_state::dio_irq3_w));
	dio16.irq4_out_cb().set(FUNC(hp9k3xx_state::dio_irq4_w));
	dio16.irq5_out_cb().set(FUNC(hp9k3xx_state::dio_irq5_w));
	dio16.irq6_out_cb().set(FUNC(hp9k3xx_state::dio_irq6_w));
	dio16.irq7_out_cb().set(FUNC(hp9k3xx_state::dio_irq7_w));
}

void hp9k3xx_state::add_dio32_bus(machine_config &config)
{
	bus::hp_dio::dio32_device &dio32(DIO32(config, "diobus", 0));
	dio32.set_program_space(m_maincpu, AS_PROGRAM);

	dio32.irq1_out_cb().set(FUNC(hp9k3xx_state::dio_irq1_w));
	dio32.irq2_out_cb().set(FUNC(hp9k3xx_state::dio_irq2_w));
	dio32.irq3_out_cb().set(FUNC(hp9k3xx_state::dio_irq3_w));
	dio32.irq4_out_cb().set(FUNC(hp9k3xx_state::dio_irq4_w));
	dio32.irq5_out_cb().set(FUNC(hp9k3xx_state::dio_irq5_w));
	dio32.irq6_out_cb().set(FUNC(hp9k3xx_state::dio_irq6_w));
	dio32.irq7_out_cb().set(FUNC(hp9k3xx_state::dio_irq7_w));
}

void hp9k3xx_state::set_bus_error(uint32_t address, bool rw, uint16_t mem_mask)
{
	if (m_maincpu->m68851_buserror(address))
		return;

	if (m_bus_error)
		return;

	m_bus_error = true;
	m_maincpu->set_buserror_details(address, rw, m_maincpu->get_fc());
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_bus_error_timer->adjust(m_maincpu->cycles_to_attotime(16)); // let rmw cycles complete
}

uint16_t hp9k3xx_state::buserror16_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		set_bus_error((offset << 1) & 0xFFFFFF, true, mem_mask);
	return 0xffff;
}

void hp9k3xx_state::buserror16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		set_bus_error((offset << 1) & 0xFFFFFF, false, mem_mask);
}

uint32_t hp9k3xx_state::buserror_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		set_bus_error(offset << 2, false, mem_mask);
	return 0xffffffff;
}

void hp9k3xx_state::buserror_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		set_bus_error(offset << 2, false, mem_mask);
}

void hp9k3xx_state::hp9k300(machine_config &config)
{
	ptm6840_device &ptm(PTM6840(config, PTM6840_TAG, 250000)); // from oscillator module next to the 6840
	ptm.set_external_clocks(250000.0f, 0.0f, 250000.0f);
	ptm.o3_callback().set(PTM6840_TAG, FUNC(ptm6840_device::set_c2));
	ptm.irq_callback().set_inputline("maincpu", M68K_IRQ_6);

	SOFTWARE_LIST(config, "flop_list").set_original("hp9k3xx_flop");
	SOFTWARE_LIST(config, "cdrom_list").set_original("hp9k3xx_cdrom");
	SOFTWARE_LIST(config, "hdd_list").set_original("hp9k3xx_hdd");
	config.set_default_layout(layout_hp9k_3xx);
}

void hp9k3xx_state::hp9k310(machine_config &config)
{
	hp9k300(config);

	M68010(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k310_map);

	add_dio16_bus(config);

	DIO16_SLOT(config, "sl0", 0, "diobus", dio16_cards, "human_interface", true);
	DIO16_SLOT(config, "sl1", 0, "diobus", dio16_cards, "98544", false);
	DIO16_SLOT(config, "sl2", 0, "diobus", dio16_cards, "98603b", false);
	DIO32_SLOT(config, "sl3", 0, "diobus", dio32_cards, "98643", false);
	DIO16_SLOT(config, "sl4", 0, "diobus", dio16_cards, "98644", false);
	DIO16_SLOT(config, "sl5", 0, "diobus", dio16_cards, nullptr, false);
}

void hp9k3xx_state::hp9k320(machine_config &config)
{
	M68020FPU(config, m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k320_map);

	hp9k300(config);
	add_dio32_bus(config);

	DIO32_SLOT(config, "sl0", 0, "diobus", dio32_cards, "human_interface", true);
	DIO32_SLOT(config, "sl1", 0, "diobus", dio32_cards, "98543", false);
	DIO32_SLOT(config, "sl2", 0, "diobus", dio32_cards, "98603b", false);
	DIO32_SLOT(config, "sl3", 0, "diobus", dio32_cards, "98644", false);
	DIO32_SLOT(config, "sl4", 0, "diobus", dio32_cards, "98620", false);
	DIO32_SLOT(config, "sl5", 0, "diobus", dio32_cards, "98265a", false);
	DIO32_SLOT(config, "sl7", 0, "diobus", dio32_cards, nullptr, false);
}

void hp9k3xx_state::hp9k330(machine_config &config)
{
	M68020PMMU(config, m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k330_map);

	hp9k300(config);
	add_dio32_bus(config);

	DIO32_SLOT(config, "sl0", 0, "diobus", dio16_cards, "human_interface", true);
	DIO32_SLOT(config, "sl1", 0, "diobus", dio16_cards, "98544", false);
	DIO32_SLOT(config, "sl2", 0, "diobus", dio16_cards, "98603b", false);
	DIO32_SLOT(config, "sl3", 0, "diobus", dio16_cards, "98644", false);
	DIO32_SLOT(config, "sl4", 0, "diobus", dio16_cards, nullptr, false);
}

void hp9k3xx_state::hp9k332(machine_config &config)
{
	M68020PMMU(config, m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k332_map);

	hp9k300(config);
	add_dio16_bus(config);

	DIO16_SLOT(config, "sl0", 0, "diobus", dio16_cards, "human_interface", true);
	DIO16_SLOT(config, "sl1", 0, "diobus", dio16_cards, "98603b", false);
	DIO16_SLOT(config, "sl2", 0, "diobus", dio16_cards, "98644", false);
	DIO16_SLOT(config, "sl3", 0, "diobus", dio16_cards, "98543", false);
	DIO16_SLOT(config, "sl4", 0, "diobus", dio16_cards, nullptr, false);
}

void hp9k3xx_state::hp9k340(machine_config &config)
{
	hp9k320(config);

	M68030(config.replace(), m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k330_map);
}

void hp9k3xx_state::hp9k360(machine_config &config)
{
	hp9k300(config);

	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k360_map);

	add_dio32_bus(config);

	DIO32_SLOT(config, "sl0", 0, "diobus", dio32_cards, "human_interface", true);
	DIO32_SLOT(config, "sl1", 0, "diobus", dio32_cards, "98550", false);
	DIO32_SLOT(config, "sl2", 0, "diobus", dio32_cards, "98644", false);
	DIO32_SLOT(config, "sl3", 0, "diobus", dio32_cards, "98620", false);
	DIO32_SLOT(config, "sl4", 0, "diobus", dio32_cards, "98265a", false);
	DIO32_SLOT(config, "sl5", 0, "diobus", dio32_cards, nullptr, false);
}


void hp9k3xx_state::hp9k370(machine_config &config)
{
	hp9k360(config);

	M68030(config.replace(), m_maincpu, 33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k370_map);
}

void hp9k3xx_state::hp9k380(machine_config &config)
{
	hp9k360(config);

	M68040(config.replace(), m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k380_map);
}

void hp9k3xx_state::hp9k382(machine_config &config)
{
	hp9k360(config);

	M68040(config.replace(), m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k3xx_state::hp9k382_map);
}

ROM_START( hp9k310 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-3771.bin", 0x000001, 0x008000, CRC(b9e4e3ad) SHA1(ed6f1fad94a15d95362701dbe124b52877fc3ec4) )
	ROM_LOAD16_BYTE( "1818-3772.bin", 0x000000, 0x008000, CRC(a3665919) SHA1(ec1bc7e5b7990a1b09af947a06401e8ed3cb0516) )

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000000, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

ROM_START( hp9k320 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "5061-6538.bin", 0x000001, 0x004000, CRC(d6aafeb1) SHA1(88c6b0b2f504303cbbac0c496c26b85458ac5d63) )
	ROM_LOAD16_BYTE( "5061-6539.bin", 0x000000, 0x004000, CRC(a7ff104c) SHA1(c640fe68314654716bd41b04c6a7f4e560036c7e) )
	ROM_LOAD16_BYTE( "5061-6540.bin", 0x008001, 0x004000, CRC(4f6796d6) SHA1(fd254897ac1afb8628f40ea93213f60a082c8d36) )
	ROM_LOAD16_BYTE( "5061-6541.bin", 0x008000, 0x004000, CRC(39d32998) SHA1(6de1bda75187b0878c03c074942b807cf2924f0e) )
ROM_END

ROM_START( hp9k330 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) )
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) )
ROM_END

ROM_START( hp9k332 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4796.bin", 0x000000, 0x010000, CRC(8a7642da) SHA1(7ba12adcea85916d18b021255391bec806c32e94) )
	ROM_LOAD16_BYTE( "1818-4797.bin", 0x000001, 0x010000, CRC(98129eb1) SHA1(f3451a854060f1be1bee9f17c5c198b4b1cd61ac) )
ROM_END

ROM_START( hp9k340 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) )
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) )

ROM_END

ROM_START( hp9k360 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4796.bin", 0x000000, 0x010000, CRC(8a7642da) SHA1(7ba12adcea85916d18b021255391bec806c32e94) )
	ROM_LOAD16_BYTE( "1818-4797.bin", 0x000001, 0x010000, CRC(98129eb1) SHA1(f3451a854060f1be1bee9f17c5c198b4b1cd61ac) )
ROM_END


ROM_START( hp9k370 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) )
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) )
ROM_END

ROM_START( hp9k380 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_WORD_SWAP( "1818-5062_98754_9000-380_27c210.bin", 0x000000, 0x020000, CRC(500a0797) SHA1(4c0a3929e45202a2689e353657e5c4b58ff9a1fd) )
ROM_END

ROM_START( hp9k382 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_WORD_SWAP( "1818-5468_27c1024.bin", 0x000000, 0x020000, CRC(d1d9ef13) SHA1(6bbb17b9adad402fbc516dc2f3143e9c38ceef8e) )

	ROM_REGION( 0x2000, "unknown", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD( "1818-5282_8ce61e951207_28c64.bin", 0x000000, 0x002000, CRC(740442f3) SHA1(ab65bd4eec1024afb97fc2dd3bd3f017e90f49ae) )
ROM_END

} // Anonymous namespace


/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME      FLAGS */
COMP( 1985, hp9k310, 0,       0,      hp9k310, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/310", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1985, hp9k320, 0,       0,      hp9k320, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/320", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1987, hp9k330, 0,       0,      hp9k330, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/330", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1987, hp9k332, 0,       0,      hp9k332, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/332", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1989, hp9k340, hp9k330, 0,      hp9k340, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/340", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1988, hp9k360, hp9k330, 0,      hp9k360, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/360", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1988, hp9k370, hp9k330, 0,      hp9k370, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/370", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1991, hp9k380, 0,       0,      hp9k380, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/380", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1991, hp9k382, 0,       0,      hp9k382, hp9k330, hp9k3xx_state, empty_init, "Hewlett-Packard", "HP9000/382", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
