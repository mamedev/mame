// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "options.h"

#include "amd98.h"
#include "fdd_2dd.h"
#include "fdd_2hd.h"
#include "pc9801_02.h"
#include "pc9801_14.h"
#include "pc9801_26.h"
#include "pc9801_27.h"
#include "pc9801_55.h"
#include "pc9801_86.h"
#include "pc9801_118.h"
#include "mif201.h"
#include "mpu_pc98.h"
#include "sb16_ct2720.h"
#include "sound.h"
#include "speakboard.h"
#include "wavestar.h"

void pc98_cbus_devices(device_slot_interface &device)
{
	// official HW
	device.option_add("pc9801_14",  PC9801_14);
	device.option_add("pc9801_26",  PC9801_26);
	device.option_add("pc9801_27",  PC9801_27);
	device.option_add("pc9801_55u", PC9801_55U);
	device.option_add("pc9801_55l", PC9801_55L);
	device.option_add("pc9801_86",  PC9801_86);
	device.option_add("pc9801_118", PC9801_118);

//  Spark Board
	device.option_add("amd98",      AMD98);
	device.option_add("mpu_pc98",   MPU_PC98);
	device.option_add("sb16",       SB16_CT2720);
	device.option_add("speakboard", SPEAKBOARD);
	device.option_add("wavestar",   QVISION_WAVESTAR);

	// File Bay
	// for first gen only
	device.option_add("fdd_2dd",    FDD_2DD_BRIDGE);
	device.option_add("fdd_2hd",    FDD_2HD_BRIDGE);
	// PC-9801-08 (2dd, external unit)
	// PC-9801-15 (8' unit)

	// doujinshi HW
// MAD Factory / Doujin Hard (同人ハード)
// MAD Factory Chibi-Oto: an ADPCM override for -86
// MAD Factory Otomi-chan: "TORIE9211 MAD FACTORY" printed on proto PCB, just overrides for ADPCM for -86?
	device.option_add("otomichan_kai", OTOMICHAN_KAI);

	// internal sound options
	device.option_add_internal("sound_pc9821ce",  SOUND_PC9821CE);
	device.option_add_internal("sound_pc9821cx3", SOUND_PC9821CX3);
}

void pc98_cbus_ram_devices(device_slot_interface &device)
{
	device.option_add("128kb", PC9801_02_128KB);
	device.option_add("256kb", PC9801_02_256KB);
	device.option_add("384kb", PC9801_02_384KB);
	device.option_add("512kb", PC9801_02_512KB);
	device.option_add("640kb", PC9801_02_640KB);
}

// TODO: add just a subset for now, all needs to be verified if compatible with C-Bus.
void pc88va_cbus_devices(device_slot_interface &device)
{
	device.option_add("pc9801_27",  PC9801_27);
//	device.option_add("pc9801_55u", PC9801_55U);
//	device.option_add("pc9801_55l", PC9801_55L);
	device.option_add("mif_201",    MIF201);
	device.option_add("mpu_pc98",   MPU_PC98);
}

// https://man.openbsd.org/cbus.4
void luna88k2_cbus_devices(device_slot_interface &device)
{
	// Allied Telesis CentreCOM LA-98
	device.option_add("pc9801_86",  PC9801_86);
	// PCMCIA options
}

