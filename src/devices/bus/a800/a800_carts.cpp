// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese

#include "emu.h"
#include "a800_carts.h"

// (#num) denotes the canonical number from .car specs for unemulated/undumped variants
void a800_left(device_slot_interface &device)
{
	device.option_add_internal("a800_8k",       A800_ROM);
	device.option_add_internal("a800_8k_right", A800_ROM_RIGHT);
	device.option_add_internal("a800_16k",      A800_ROM_16KB);
	device.option_add_internal("a800_phoenix",  A800_ROM_PHOENIX);
	device.option_add_internal("a800_blizzard", A800_ROM_BLIZZARD_16KB);
	device.option_add_internal("a800_phoenix_ast2k", A800_ROM_PHOENIX_AST2K);
	device.option_add_internal("a800_bbsb",     A800_ROM_BBSB);
	device.option_add_internal("a800_oss8k",    A800_ROM_OSS8K);
	device.option_add_internal("a800_oss034m",  A800_ROM_OSS34);
	device.option_add_internal("a800_oss043m",  A800_ROM_OSS43);
	device.option_add_internal("a800_ossm091",  A800_ROM_OSS91);
	// OSS cart with subslot
//  device.option_add_internal("a800_mddos",    A800_ROM_MDDOS);
	device.option_add_internal("a800_williams", A800_ROM_WILLIAMS);
	device.option_add_internal("a800_diamond",  A800_ROM_DIAMOND);
	device.option_add_internal("a800_express",  A800_ROM_EXPRESS);
	device.option_add_internal("a800_turbo",    A800_ROM_TURBO);
	device.option_add_internal("a800_tlink2",   A800_ROM_TELELINK2);
	device.option_add_internal("a800_ultracart", A800_ROM_ULTRACART);
	device.option_add_internal("a800_blizzard_32kb", A800_ROM_BLIZZARD_32KB);
	device.option_add_internal("a800_adawliah", A800_ROM_ADAWLIAH);
	device.option_add_internal("a800_atrax",    A800_ROM_ATRAX);
	device.option_add_internal("a800_sparta",   A800_ROM_SPARTADOS);
	device.option_add_internal("a800_sparta_128kb",   A800_ROM_SPARTADOS_128KB);
	// (#48-#49 / #68) SDX 64KB/128KB variants
//  device.option_add_internal("a800_sdx_atrax_64kb", A800_ROM_SDX_ATRAX_64KB);
//  device.option_add_internal("a800_sdx_atrax_128kb", A800_ROM_SDX_ATRAX_128KB);
	// (#5) DB "Dave Bennett" homebrew cartridge, vaporware? cfr. https://forums.atariage.com/topic/307663-32k-db-cart/
//  device.option_add_internal("a800_db",       A800_ROM_DB);
	// (#47) "Atari Super Turbo" 32 KB Polish cart with exotic banking scheme (256 bytes at a time) that maps to CCTL
//  device.option_add_internal("a800_ast",      A800_ROM_AST);
	// TOOLBOX III / RAMBOX 2 Polish carts with additional 256KB RAM
//  device.option_add_internal("a800_jrc",      A800_ROM_JRC);
	// Czech cart, some kind of toolkit with bankswitch that rolls back after set time
//  device.option_add_internal("a800_cos32",    A800_ROM_COS32);

	// XEGS carts
	device.option_add_internal("xegs",          XEGS_ROM);
	// "XEGS demo cartridge", 4 games in 1 with binary counter applied at reset for each reboot
//  device.option_add_internal("xegs_demo",     XEGS_ROM_DEMO);
	// (#33-#38) Carts sold by Nir Dary in the '90s, has fixed last bank to RD5 and selectable RD4 bank. 32KB to 1MB ROM size options
//  device.option_add_internal("xegs_switch",   XEGS_ROM_SWITCHABLE);

	// flash carts
	device.option_add_internal("a800_corina",   A800_ROM_CORINA);
	device.option_add_internal("a800_corina_sram", A800_ROM_CORINA_SRAM);
	device.option_add(         "maxflash_128kb",  A800_MAXFLASH_128KB);
	device.option_add(         "maxflash_1mb",  A800_MAXFLASH_1MB);
	// (#61) MegaMax, has switch that toggles between Atarimax 1MB mode and 2MB
//  device.option_add(         "megamax",       A800_MEGAMAX);
	device.option_add(         "sic_128kb",     A800_SIC_128KB);
	device.option_add(         "sic_256kb",     A800_SIC_256KB);
	device.option_add(         "sic_512kb",     A800_SIC_512KB);
	// (#26-#32 / #63-#64) MegaCart 16KB up to 4MB variant too
//  device.option_add(         "megacart",      A800_MEGACART);
	// Atarimax MyIDE-II, 512 KB flash ROM + 512 KB RAM + CompactFlash i/f, requires DIY MyBIOS ROM installation into main system
//  device.option_add(         "myideii",       A800_MYIDE_II);
	// 512KB flash + CompactFlash, emulates a Sparta DOS X, has DS1305 RTC hooked up thru SPI bus, SIDE 2 is an upgraded variant with slightly different CCTL mapping
//  device.option_add(         "side1",         A800_SIDE1);
//  device.option_add(         "side2",         A800_SIDE2);
	// Upgrades SIDE 2 with SD card instead of CF, MX29LV640ET NOR flash chip, 2MB RAM, DMA with various OPs, ROM bank emulation modes with relocatable CCTL ...
//  device.option_add(         "side3",         A800_SIDE3);
	// STM32 coprocessor with SD card, cfr. https://github.com/robinhedwards/UnoCart
//  device.option_add(         "unocart",       A800_UNOCART);
	// earlier variant of above running on FPGA, cfr. https://github.com/robinhedwards/UltimateCart
//  device.option_add(         "ultimatecart",  A800_ULTIMATECART);
	// (#62 / #65) The!Cart, 32/64/128MB + 512KB RAM, emulation modes
//  device.option_add(         "thecart",       A800_THECART);

	// non-ROM types
	device.option_add(         "rtime8",        A800_RTIME8);
	// adds extra 65C816 coprocessor. No firmware, runs on code uploaded by main CPU, also two rev variants (V1 and V2)
	// NB: SDX documentation calls this Weronika, which just seems a Polish alias
//  device.option_add(         "veronica",      A800_VERONICA);
	// 128 or 256 KB additional RAM, schematics available at atarimax
//  device.option_add(         "ramcart",       A800_RAM_CART);
	// converts analog mono sound to digital by reading $d500 low 4 bitset
//  device.option_add(         "ad_converter",  A800_AD_CONVERTER);
	device.option_add(         "supercharger",  A800_SUPER_CHARGER);
	// The PILL! / Super PILL! / Super Cart with RD4/RD5 held high, cart copier thru own floppy based program
//  device.option_add(         "pill",          A800_PILL);
	// Thompson Proburner, EPROM burner
//  device.option_add(         "proburner",     A800_PROBURNER);
	// The Multiplexer! (MUX!), PBI bus between 1 master and 8 slaves
	// cfr. https://sdx.atari8.info/sdx_files/muxsdx.txt & http://realdos.net/mux.html
//  device.option_add(         "mux",           A800_MUX);
}

void a800_right(device_slot_interface &device)
{
	device.option_add_internal("a800_8k_right", A800_ROM_RIGHT);
}

void a5200_carts(device_slot_interface &device)
{
	device.option_add_internal("a5200_rom",     A5200_ROM);
	device.option_add_internal("a5200_2chips",  A5200_ROM_2CHIPS);
	device.option_add_internal("a5200_bbsb",    A5200_ROM_BBSB);
	device.option_add_internal("a5200_supercart", A5200_ROM_SUPERCART);
}
