//
// Simple program that touches all the existing cores to help ensure
// that everything builds cleanly.
//
// Compile with:
//
//   g++ --std=c++14 -I../../src buildall.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_opq.cpp ../../src/ymfm_opz.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o buildall.exe
//
// or:
//
//   clang --std=c++14 -I../../src buildall.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_opq.cpp ../../src/ymfm_opz.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o buildall.exe
//
// or:
//
//   cl -I..\..\src buildall.cpp ..\..\src\ymfm_misc.cpp ..\..\src\ymfm_opl.cpp ..\..\src\ymfm_opm.cpp ..\..\src\ymfm_opn.cpp ..\..\src\ymfm_opq.cpp ..\..\src\ymfm_opz.cpp ..\..\src\ymfm_adpcm.cpp ..\..\src\ymfm_pcm.cpp ..\..\src\ymfm_ssg.cpp /Od /Zi /std:c++14 /EHsc
//

#include <vector>

#include "ymfm_misc.h"
#include "ymfm_opl.h"
#include "ymfm_opm.h"
#include "ymfm_opn.h"
#include "ymfm_opq.h"
#include "ymfm_opz.h"


//-------------------------------------------------
//  main - program entry point
//-------------------------------------------------

template<typename ChipType>
class chip_wrapper : public ymfm::ymfm_interface
{
public:
	chip_wrapper() :
		m_chip(*this)
	{
		// reset
		m_chip.reset();

		// save/restore
		std::vector<uint8_t> buffer;
		{
			ymfm::ymfm_saved_state saver(buffer, true);
			m_chip.save_restore(saver);
		}
		{
			ymfm::ymfm_saved_state restorer(buffer, false);
			m_chip.save_restore(restorer);
		}

		// dummy read/write
		m_chip.read(0);
		m_chip.write(0, 0);

		// generate
		typename ChipType::output_data output[20];
		m_chip.generate(&output[0], ymfm::array_size(output));
	}

private:
	ChipType m_chip;
};


//-------------------------------------------------
//  main - program entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	// just keep adding chip variants here as they are implemented

	// ymfm_misc.h:
	chip_wrapper<ymfm::ym2149> test2149;

	// ymfm_opl.h:
	chip_wrapper<ymfm::ym3526> test3526;
	chip_wrapper<ymfm::y8950> test8950;
	chip_wrapper<ymfm::ym3812> test3812;
	chip_wrapper<ymfm::ymf262> test262;
	chip_wrapper<ymfm::ymf289b> test289b;
	chip_wrapper<ymfm::ymf278b> test278b;
	chip_wrapper<ymfm::ym2413> test2413;
	chip_wrapper<ymfm::ym2423> test2423;
	chip_wrapper<ymfm::ymf281> test281;
	chip_wrapper<ymfm::ds1001> test1001;

	// ymfm_opm.h:
	chip_wrapper<ymfm::ym2151> test2151;
	chip_wrapper<ymfm::ym2164> test2164;

	// ymfm_opn.h:
	chip_wrapper<ymfm::ym2203> test2203;
	chip_wrapper<ymfm::ym2608> test2608;
	chip_wrapper<ymfm::ymf288> test288;
	chip_wrapper<ymfm::ym2610> test2610;
	chip_wrapper<ymfm::ym2610b> test2610b;
	chip_wrapper<ymfm::ym2612> test2612;
	chip_wrapper<ymfm::ym3438> test3438;
	chip_wrapper<ymfm::ymf276> test276;

	// ymfm_opq.h:
	chip_wrapper<ymfm::ym3806> test3806;
	chip_wrapper<ymfm::ym3533> test3533;

	// ymfm_opz.h:
	chip_wrapper<ymfm::ym2414> test2414;

	printf("Done\n");

	return 0;
}
