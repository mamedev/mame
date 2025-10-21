// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "options.h"

#include "amd98.h"
#include "pc9801_26.h"
#include "pc9801_55.h"
#include "pc9801_86.h"
#include "pc9801_118.h"
#include "mpu_pc98.h"
#include "pc9801_cbus.h"
#include "sb16_ct2720.h"
#include "sound.h"
#include "wavestar.h"


void pc98_cbus_devices(device_slot_interface &device)
{
	// official HW
//  PC-9801-14
	device.option_add("pc9801_26",  PC9801_26);
	device.option_add("pc9801_55u", PC9801_55U);
	device.option_add("pc9801_55l", PC9801_55L);
	device.option_add("pc9801_86",  PC9801_86);
	device.option_add("pc9801_118", PC9801_118);
	device.option_add("pc9801_spb", PC9801_SPEAKBOARD);
//  Spark Board
	device.option_add("amd98",      AMD98);
	device.option_add("mpu_pc98",   MPU_PC98);
	device.option_add("sb16",       SB16_CT2720);
	device.option_add("wavestar",   QVISION_WAVESTAR);

	// doujinshi HW
// MAD Factory / Doujin Hard (同人ハード)
// MAD Factory Chibi-Oto: an ADPCM override for -86
// MAD Factory Otomi-chan: "TORIE9211 MAD FACTORY" printed on proto PCB, just overrides for ADPCM for -86?
	device.option_add("otomichan_kai", OTOMICHAN_KAI);

	// internal sound options
	device.option_add_internal("sound_pc9821ce",  SOUND_PC9821CE);
	device.option_add_internal("sound_pc9821cx3", SOUND_PC9821CX3);
}

