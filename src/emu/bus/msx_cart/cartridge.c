
#include "emu.h"
#include "cartridge.h"
#include "ascii.h"
#include "crossblaim.h"
#include "fmpac.h"
#include "konami.h"
#include "korean.h"
#include "majutsushi.h"
#include "msx_audio.h"
#include "msxdos2.h"
#include "nomapper.h"
#include "rtype.h"
#include "superloderunner.h"


SLOT_INTERFACE_START(msx_cart)
	SLOT_INTERFACE_INTERNAL("nomapper", MSX_CART_NOMAPPER)
	SLOT_INTERFACE_INTERNAL("msxdos2", MSX_CART_MSXDOS2)
	SLOT_INTERFACE_INTERNAL("konami_scc", MSX_CART_KONAMI_SCC)
	SLOT_INTERFACE_INTERNAL("konami", MSX_CART_KONAMI)
	SLOT_INTERFACE_INTERNAL("ascii8", MSX_CART_ASCII8)
	SLOT_INTERFACE_INTERNAL("ascii16", MSX_CART_ASCII16)
	SLOT_INTERFACE_INTERNAL("gamemaster2", MSX_CART_GAMEMASTER2)
	SLOT_INTERFACE_INTERNAL("ascii8_sram", MSX_CART_ASCII8_SRAM)
	SLOT_INTERFACE_INTERNAL("ascii16_sram", MSX_CART_ASCII16_SRAM)
	SLOT_INTERFACE_INTERNAL("rtype", MSX_CART_RTYPE)
	SLOT_INTERFACE_INTERNAL("majutsushi", MSX_CART_MAJUTSUSHI)
	SLOT_INTERFACE_INTERNAL("fmpac", MSX_CART_FMPAC)
	SLOT_INTERFACE_INTERNAL("superloderunner", MSX_CART_SUPERLODERUNNER)
	SLOT_INTERFACE_INTERNAL("synthesizer", MSX_CART_SYNTHESIZER)
	SLOT_INTERFACE_INTERNAL("cross_blaim", MSX_CART_CROSSBLAIM)
//	SLOT_INTERFACE_INTERNAL("disk_rom", MSX_CART_DISK_ROM)
	SLOT_INTERFACE_INTERNAL("korean_80in1", MSX_CART_KOREAN_80IN1)
	SLOT_INTERFACE_INTERNAL("korean_90in1", MSX_CART_KOREAN_90IN1)
	SLOT_INTERFACE_INTERNAL("korean_126in1", MSX_CART_KOREAN_126IN1)
	SLOT_INTERFACE_INTERNAL("sound_snatcher", MSX_CART_SOUND_SNATCHER)
	SLOT_INTERFACE_INTERNAL("sound_sdsnatch", MSX_CART_SOUND_SDSNATCHER)
	SLOT_INTERFACE_INTERNAL("msx_audio", MSX_CART_MSX_AUDIO)
SLOT_INTERFACE_END


msx_cart_interface::msx_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}

void msx_cart_interface::rom_alloc(UINT32 size)
{
	m_rom.resize(size);
}

void msx_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}

void msx_cart_interface::sram_alloc(UINT32 size)
{
	m_sram.resize(size);
}
