#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"

#include "ibmsystem23.lh"

namespace system23
{
	class system23_state: public driver_device
	{
		public:
			system23_state(const machine_config &mconfig, device_type type, const char *tag)
				: driver_device(mconfig, type, tag),
				m_ppi8255(*this,"ppi8255"),
				m_diag_digits(*this, "digit%u", 0U)
			{

			}

			void system23(machine_config &config);

		protected:
			virtual void machine_start() override ATTR_COLD;
			virtual void machine_reset() override ATTR_COLD;

		private:
			required_device<i8255_device> m_ppi8255;
			output_finder<2> m_diag_digits;

			void diag_digits_w(uint8_t data);

			void system23_io(address_map &map) ATTR_COLD;
			void system23_mem(address_map &map) ATTR_COLD;
	};

	void system23_state::diag_digits_w(uint8_t data)
	{
		m_diag_digits[0] = data & 0x0f;
		m_diag_digits[1] = (data >> 4) & 0x0f;
	}

	void system23_state::system23_io(address_map &map)
	{
		map.unmap_value_high();
		map(0x43, 0x43).w(FUNC(diag_digits_w));
	}

	void system23_state::system23_mem(address_map &map)
	{
		map.unmap_value_high();
		map(0x0000, 0x3fff).rom().region("ros_unpaged",0);
	}

	void system23_state::system23(machine_config &config)
	{
		i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6_MHz_XTAL)); //frequency needs to be adjusted
		maincpu.set_addrmap(AS_PROGRAM, &system23_state::system23_mem);
		maincpu.set_addrmap(AS_IO, &system23_state::system23_io);

		config.set_perfect_quantum("maincpu");
		config.set_default_layout(layout_ibmsystem23);


	}

	ROM_START( system23 )
		ROM_REGION(0x4000, "ros_unpaged", 0)
		ROM_LOAD("02_61c9866a_4481186.bin", 0x0000, 0x2000, CRC(61c9866a) SHA1(3b51a6b72d2ccae2459ddb2e16fbd21b19dfa2b8) )
		ROM_LOAD("09_07843020_8493747.bin", 0x2000, 0x2000, CRC(07843020) SHA1(078405ad202e26b7bac7132b06682fb01270af63) )
	ROM_END


	COMP( 1981, system23, 0,      0,      system23, 0,     system23_state, empty_init, "IBM",   "IBM System/23 Datamaster", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

}
