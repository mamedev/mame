// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "cartridge.h"
#include "arc.h"
#include "ascii.h"
#include "bm_012.h"
#include "crossblaim.h"
#include "disk.h"
#include "dooly.h"
#include "fmpac.h"
#include "fs_sr022.h"
#include "halnote.h"
#include "hfox.h"
#include "holy_quran.h"
#include "konami.h"
#include "korean.h"
#include "majutsushi.h"
#include "moonsound.h"
#include "msx_audio.h"
#include "msxdos2.h"
#include "nomapper.h"
#include "rtype.h"
#include "superloderunner.h"
#include "super_swangi.h"
#include "yamaha.h"

#include "bus/msx_slot/cartridge.h"


void msx_cart(device_slot_interface &device)
{
	device.option_add_internal("nomapper", MSX_CART_NOMAPPER);
	device.option_add_internal("msxdos2", MSX_CART_MSXDOS2);
	device.option_add_internal("konami_scc", MSX_CART_KONAMI_SCC);
	device.option_add_internal("konami", MSX_CART_KONAMI);
	device.option_add_internal("ascii8", MSX_CART_ASCII8);
	device.option_add_internal("ascii16", MSX_CART_ASCII16);
	device.option_add_internal("gamemaster2", MSX_CART_GAMEMASTER2);
	device.option_add_internal("ascii8_sram", MSX_CART_ASCII8_SRAM);
	device.option_add_internal("ascii16_sram", MSX_CART_ASCII16_SRAM);
	device.option_add_internal("rtype", MSX_CART_RTYPE);
	device.option_add_internal("majutsushi", MSX_CART_MAJUTSUSHI);
	device.option_add_internal("fmpac", MSX_CART_FMPAC);
	device.option_add_internal("fs_sr022", MSX_CART_FS_SR022);
	device.option_add_internal("superloderunner", MSX_CART_SUPERLODERUNNER);
	device.option_add_internal("synthesizer", MSX_CART_SYNTHESIZER);
	device.option_add_internal("cross_blaim", MSX_CART_CROSSBLAIM);
	device.option_add_internal("korean_80in1", MSX_CART_KOREAN_80IN1);
	device.option_add_internal("korean_90in1", MSX_CART_KOREAN_90IN1);
	device.option_add_internal("korean_126in1", MSX_CART_KOREAN_126IN1);
	device.option_add_internal("msxwrite", MSX_CART_MSXWRITE);
	device.option_add_internal("sound_snatcher", MSX_CART_SOUND_SNATCHER);
	device.option_add_internal("sound_sdsnatch", MSX_CART_SOUND_SDSNATCHER);
	device.option_add_internal("msxaud_hxmu900", MSX_CART_MSX_AUDIO_HXMU900);
	device.option_add_internal("msxaud_fsca1", MSX_CART_MSX_AUDIO_FSCA1);
	device.option_add_internal("msxaud_nms1205", MSX_CART_MSX_AUDIO_NMS1205);
	device.option_add_internal("super_swangi", MSX_CART_SUPER_SWANGI);
	device.option_add_internal("hfox", MSX_CART_HFOX);
	device.option_add_internal("keyboard_master", MSX_CART_KEYBOARD_MASTER);
	device.option_add_internal("holy_quran", MSX_CART_HOLY_QURAN);
	device.option_add_internal("dooly", MSX_CART_DOOLY);
	device.option_add_internal("halnote", MSX_CART_HALNOTE);
	device.option_add_internal("arc", MSX_CART_ARC);
	device.option_add_internal("disk_vy0010", MSX_CART_VY0010);
	device.option_add_internal("disk_fsfd1", MSX_CART_FSFD1);
	device.option_add_internal("disk_fsfd1a", MSX_CART_FSFD1A);
	device.option_add_internal("disk_fscf351", MSX_CART_FSCF351);
	device.option_add("bm_012", MSX_CART_BM_012);
	device.option_add("moonsound", MSX_CART_MOONSOUND);
}


msx_cart_interface::msx_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_exp(nullptr)
{
}

void msx_cart_interface::rom_alloc(uint32_t size)
{
	m_rom.resize(size);
	std::fill_n(m_rom.begin(), size, 0xff);
}

void msx_cart_interface::rom_vlm5030_alloc(uint32_t size)
{
	m_rom_vlm5030.resize(size);
	std::fill_n(m_rom_vlm5030.begin(), size, 0xff);
}

void msx_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
	std::fill_n(m_ram.begin(), size, 0x00);
}

void msx_cart_interface::sram_alloc(uint32_t size)
{
	m_sram.resize(size);
	std::fill_n(m_sram.begin(), size, 0x00);
}

WRITE_LINE_MEMBER(msx_cart_interface::irq_out)
{
	m_exp->irq_out(state);
}

address_space &msx_cart_interface::memory_space() const
{
	return m_exp->memory_space();
}

address_space &msx_cart_interface::io_space() const
{
	return m_exp->io_space();
}


// Several yamaha machines had 60 pin expansion slots. The pinouts of these slots was
// exactly the same as the regular 50 pin cartridge slots. The lowest 10 pins are simply
// not connected.
void msx_yamaha_60pin(device_slot_interface &device)
{
	device.option_add("sfg01", MSX_CART_SFG01);
	device.option_add("sfg05", MSX_CART_SFG05);
}
