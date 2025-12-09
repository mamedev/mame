#include "emu.h"
#include "cpu/m6502/w65c02s.h"
#include "machine/6522via.h"


namespace
{
	class cr124_state : driver_device
	{
		public:

			cr124_state(const machine_config &mconfig, device_type type, const char *tag)
				: driver_device(mconfig, type, tag),
				m_maincpu(*this,"maincpu"),
				m_rom(*this,"rom"),
				m_ram(*this,"ram"),
				m_via1(*this,"via1"),
				m_via2(*this,"via2")
				{

				}

			void cr124(machine_config &config);

		protected:
			virtual void machine_start() override ATTR_COLD;
			virtual void machine_reset() override ATTR_COLD;

		private:
			required_device<w65c02s_device> m_maincpu;
			required_memory_region m_rom;
			required_memory_region m_ram;
			required_device<6522via_device> m_via1;
			required_device<6522via_device> m_via2;
	}

	//This is the constructor of the class

	void cr124_state::cr124(machine_config &config)
	{
		W65C02S(&config, m_maincpu, 1'000'000);
	}

	void cr124_state::machine_start()
	{

	}

	void cr124_state::machine_reset()
	{

	}

	ROM_START (cr124)
		ROM_SYSTEM_BIOS(0,"diagnostics","BP-124 Diagnostics (2025)")
	ROM_END

}

COMP( 1980, cr124, 0,      0,      cr124, cr124,     cr124_state, empty_init, "COMELTA",   "CR-124", MACHINE_NOT_WORKING)
//COMP( 1986, jb176, 0,      0,      jb176, jb176,     jb176_state, empty_init, "J.B. Electrónica Industrial",   "JB-176", MACHINE_NOT_WORKING)
//COMP( 1986, ep324, 0,      0,      ep324, ep324,     ep324_state, empty_init, "PYCMESA",   "EP-324", MACHINE_NOT_WORKING)
//COMP( 1980, bp124, 0,      0,      bp124, bp124,     bp124_state, empty_init, "Bits Passats",   "BP-124", MACHINE_NOT_WORKING)
