// license:BSD-3-Clause
// copyright-holders: Jaume López

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/ram.h"

#include "ibmsystem23.lh"

namespace
{
	class system23_state: public driver_device
	{
		public:
			system23_state(const machine_config &mconfig, device_type type, const char *tag)
				: driver_device(mconfig, type, tag),
				m_maincpu(*this, "maincpu"),
				m_ppi_kbd(*this, "ppi_kbd"),
				m_ppi_diag(*this, "ppi_diag"),
				m_ppi_settings(*this, "ppi_settings"),
				m_dma(*this,"dma"),
				m_ram(*this, RAM_TAG),
				m_diag_digits(*this, "digit%u", 0U)

			{

			}

			void system23(machine_config &config);

		protected:
			virtual void machine_start() override ATTR_COLD;
			virtual void machine_reset() override ATTR_COLD;

		private:
			required_device<i8085a_cpu_device> m_maincpu;
			required_device<i8255_device> m_ppi_kbd;
			required_device<i8255_device> m_ppi_diag;
			required_device<i8255_device> m_ppi_settings;
			required_device<i8257_device> m_dma;
			required_device<ram_device> m_ram;
			output_finder<2> m_diag_digits;

			uint8_t m_bus_test_register = 0;
			uint8_t hex_seven_segment[16] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

			void diag_digits_w(uint8_t data);

			void cpu_test_register_w(uint8_t data);
			uint8_t cpu_test_register_r();

			uint8_t memory_settings_r();

			uint8_t sid_sod_connection;
			void sod_w(int state);
			int sid_r();

			void system23_io(address_map &map) ATTR_COLD;
			void system23_mem(address_map &map) ATTR_COLD;
	};

	void system23_state::diag_digits_w(uint8_t data)
	{
		m_diag_digits[0] = system23_state::hex_seven_segment[data & 0x0f];
		m_diag_digits[1] = system23_state::hex_seven_segment[(data >> 4) & 0x0f];
	}

	void system23_state::cpu_test_register_w(uint8_t data)
	{
		m_bus_test_register = data;
	}

	uint8_t system23_state::cpu_test_register_r()
	{
		return m_bus_test_register;
	}

	uint8_t system23_state::memory_settings_r()
	{
		return 0;	//Patch to make the memory test work while port 2e is being studied
	}

	int system23_state::sid_r()
	{
		return ASSERT_LINE; // Actually, SID is tied to VCC at the motherboard
	}

	void system23_state::sod_w(int state)
	{
		sid_sod_connection = state;
	}

	void system23_state::system23_io(address_map &map)
	{
		map.unmap_value_high();
		map(0x00, 0x0f).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
		map(0x2c, 0x2f).rw(m_ppi_settings, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x40, 0x43).rw(m_ppi_diag, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x4c, 0x4f).rw(m_ppi_kbd, FUNC(i8255_device::read), FUNC(i8255_device::write));
	}

	void system23_state::system23_mem(address_map &map)
	{
		map.unmap_value_high();
		map(0x0000, 0x3fff).rom().region("ros_unpaged",0);
		map(0x8000, 0xbfff).ram();
		//map(0xC000, 0xffff).ram();
	}

	void system23_state::system23(machine_config &config)
	{
		i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6.144_MHz_XTAL)); //frequency needs to be adjusted
		maincpu.set_addrmap(AS_PROGRAM, &system23_state::system23_mem);
		maincpu.set_addrmap(AS_IO, &system23_state::system23_io);
		maincpu.in_sid_func().set(FUNC(system23_state::sid_r));
		maincpu.out_sod_func().set(FUNC(system23_state::sod_w));

		I8255(config, m_ppi_kbd);
		m_ppi_kbd->in_pa_callback().set(FUNC(system23_state::cpu_test_register_r));
		m_ppi_kbd->out_pa_callback().set(FUNC(system23_state::cpu_test_register_w));

		I8255(config, m_ppi_diag);
		m_ppi_diag->out_pb_callback().set(FUNC(system23_state::diag_digits_w));

		I8255(config, m_ppi_settings);
		m_ppi_settings->in_pc_callback().set(FUNC(system23_state::memory_settings_r));

		I8257(config, m_dma, 6.144_MHz_XTAL / 2); //frequency needs to be adjusted

		RAM(config, m_ram).set_default_size("16k");

		config.set_perfect_quantum("maincpu");
		config.set_default_layout(layout_ibmsystem23);
	}

	void system23_state::machine_start()
	{
		m_diag_digits.resolve();
	}

	void system23_state::machine_reset()
	{

	}

	ROM_START( system23 )
		ROM_REGION(0x4000, "ros_unpaged", 0)
		ROM_LOAD("02_61c9866a_4481186.bin", 0x0000, 0x2000, CRC(61c9866a) SHA1(43f2bed5cc2374c7fde4632948329062e57e994b) )
		ROM_LOAD("09_07843020_8493747.bin", 0x2000, 0x2000, CRC(07843020) SHA1(828ca0199af1246f6caf58bcb785f791c3a7e34e) )
	ROM_END

}

COMP( 1981, system23, 0,      0,      system23, 0,     system23_state, empty_init, "IBM",   "IBM System/23 Datamaster", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
