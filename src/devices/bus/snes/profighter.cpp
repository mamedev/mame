// license:BSD-3-Clause
// copyright-holders:

/***********************************************************************************************************

 Super Pro Fighter Q (3 variants)
 Super Pro Fighter X (MB03D)

 TODO: everything

 ***********************************************************************************************************/


#include "emu.h"
#include "profighter.h"

#include "snes_carts.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"

#include "formats/pc_dsk.h"


DEFINE_DEVICE_TYPE(SNS_PRO_FIGHTER_Q, sns_pro_fighter_q_device, "profighterq", "Super Pro. Fighter Q (variant 1)");
DEFINE_DEVICE_TYPE(SNS_PRO_FIGHTER_QA, sns_pro_fighter_qa_device, "profighterqa", "Super Pro. Fighter Q (variant 2)");
DEFINE_DEVICE_TYPE(SNS_PRO_FIGHTER_QB, sns_pro_fighter_qb_device, "profighterqb", "Super Pro. Fighter Q (variant 3)");
DEFINE_DEVICE_TYPE(SNS_PRO_FIGHTER_X, sns_pro_fighter_x_device, "profighterx", "Pro Fighter X (MB03D)");


static void profght_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}


/*
Super Pro. Fighter Q copier (variant 1)

Main components:

MCCS3201FN floppy disk controller
24 MHz XTAL (for the FDC)
ST10198P CIC replacement
HM6264LP-15 standard SRAM
MCM60L256AF10 standard SRAM
*/

sns_pro_fighter_q_device::sns_pro_fighter_q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNS_PRO_FIGHTER_Q, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
{
}

ROM_START( profghtq )
	ROM_REGION( 0x4000, "program", 0 )
	ROM_LOAD( "u9", 0x0000, 0x4000, CRC(4f27e1fe) SHA1(ee11ac36b5e51476ab92c66135a2353e038c014b) )

	ROM_REGION( 0x1200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h.u1",  0x0000, 0x0117, CRC(3191765b) SHA1(3042c29cad1c6eebfa756514939da973b5856bd2) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce20v8h.u5",  0x0200, 0x0157, CRC(5fa3c6bf) SHA1(edb825a15256a3a0c2fefdb3d8bf2482fd630744) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce16v8h.u6",  0x0400, 0x0117, CRC(f9a94549) SHA1(48eab926a13d4015e91833ea4b2e3da8a461392e) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce20v8h.u7",  0x0600, 0x0157, CRC(b321697d) SHA1(902459c06e1e35b3ad052f9439fd54add6ad2e5b) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce16v8h.u12", 0x0800, 0x0117, CRC(40f86bce) SHA1(97edea87fc3bcfa296202fc106b1838ceffb1730) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce16v8h.u13", 0x0a00, 0x0117, CRC(691d8a86) SHA1(f61a93cd661035440461076ee42ec7bee98ce8ff) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "ami18cv8b.u14",  0x0c00, 0x0155, CRC(472405e4) SHA1(615196f1ab9819b2e22f5f48d5edd35d4db70098) ) // this is actually a recreation targeted for a PEEL18CV8 device
	ROM_LOAD( "palce16v8h.u15", 0x0e00, 0x0117, CRC(0b8a2c8f) SHA1(0cc9c9ed671f85a3ad8622b65f94b546bb13e6b6) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce16v8h.u16", 0x1000, 0x0117, CRC(70346ef5) SHA1(d9c0a7a2c466fa1fcc1680ee5dc3777714c05fe5) ) // this is actually a recreation targeted for a GAL16V8 device
ROM_END


void sns_pro_fighter_q_device::device_start()
{
}

void sns_pro_fighter_q_device::device_reset()
{

}

const tiny_rom_entry *sns_pro_fighter_q_device::device_rom_region() const
{
	return ROM_NAME( profghtq );
}

void sns_pro_fighter_q_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, "fdc", 24_MHz_XTAL); // actually MCCS3201FN, divider?
	FLOPPY_CONNECTOR(config, "fdc:0", profght_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	SNS_CART_SLOT(config, "cartslot", 0, snes_cart, nullptr);
}


/*
Super Pro. Fighter Q copier (variant 2)

Main components:

GM82C765B floppy disk controller
16 MHz XTAL (for the FDC)
74LS216 CIC replacement (not to be confused with the TTL)
MK4864N-120 standard SRAM
M5M5256FP CMOS Static RAM
6 x HY524800J-80 Fast Page DRAM (on a sub board)
*/

sns_pro_fighter_qa_device::sns_pro_fighter_qa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNS_PRO_FIGHTER_QA, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
{
}

ROM_START( profghtqa )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "u10", 0x0000, 0x8000, CRC(56132c4e) SHA1(6649b1f13a96d144907ebab65ce00ac168b0dbe5) )

	ROM_REGION( 0x1400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ami18cv8b.u2",   0x0000, 0x0155, CRC(4ee0c889) SHA1(a5d060c8892dd24cbc1c14bc937ad5c301831dea) ) // this is actually a recreation targeted for a PEEL18CV8 device
	ROM_LOAD( "palce16v8h.u3",  0x0200, 0x0117, CRC(17e897b9) SHA1(e4e9806187ffb51d4363ef9fd34d5b4a3e62e1d3) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce20v8h.u4",  0x0400, 0x0157, CRC(5fa3c6bf) SHA1(edb825a15256a3a0c2fefdb3d8bf2482fd630744) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce20v8h.u8",  0x0600, 0x0157, CRC(a9f5c9ed) SHA1(395f523b77d9d9ed4f20c124153697c7b301cfcc) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce16v8h.u13", 0x0800, 0x0117, CRC(c77839b0) SHA1(6370c3862caee28dd9cc543c42c1ee9e87bdd64d) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce16v8h.u14", 0x0a00, 0x0117, CRC(12c79d25) SHA1(9796270dd2897803686638bbd87aca1364e41933) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce16v8h.u15", 0x0c00, 0x0117, CRC(0b8a2c8f) SHA1(0cc9c9ed671f85a3ad8622b65f94b546bb13e6b6) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "ami18cv8b.u16",  0x0e00, 0x0155, CRC(481a6974) SHA1(aff914837a7d8ef804b0ac4049d6f23ab23a1cda) ) // this is actually a recreation targeted for a PEEL18CV8 device
	ROM_LOAD( "palce16v8h.u17", 0x1000, 0x0117, CRC(70346ef5) SHA1(d9c0a7a2c466fa1fcc1680ee5dc3777714c05fe5) ) // this is actually a recreation targeted for a GAL16V8 device
	ROM_LOAD( "palce16v8h.u18", 0x1200, 0x0117, CRC(a3bbb6b1) SHA1(bdf291d2e3601d7b48c348b1d99b8872b1b4fedd) ) // this is actually a recreation targeted for a GAL16V8 device
ROM_END


void sns_pro_fighter_qa_device::device_start()
{
}

void sns_pro_fighter_qa_device::device_reset()
{

}

const tiny_rom_entry *sns_pro_fighter_qa_device::device_rom_region() const
{
	return ROM_NAME( profghtqa );
}

void sns_pro_fighter_qa_device::device_add_mconfig(machine_config &config)
{
	WD37C65C(config, "fdc", 16_MHz_XTAL); // actually GM82C765B
	FLOPPY_CONNECTOR(config, "fdc:0", profght_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	SNS_CART_SLOT(config, "cartslot", 0, snes_cart, nullptr);
}


/*
Super Pro. Fighter Q copier (variant 3)

Main components:

ACCMicro 3201 floppy disk controller
24 MHz XTAL (for the FDC)
unmarked CIC replacement
D43256AGU-15LL CMOS Static RAM
4 x GM71C4400BJ70 + 4 x KM44C1000BLJ-6 Fast Page DRAM (on a sub board)
*/

sns_pro_fighter_qb_device::sns_pro_fighter_qb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNS_PRO_FIGHTER_QB, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
{
}

ROM_START( profghtqb ) // same program as profghtrq but different plds
	ROM_REGION( 0x4000, "program", 0 )
	ROM_LOAD( "u7", 0x0000, 0x4000, CRC(4f27e1fe) SHA1(ee11ac36b5e51476ab92c66135a2353e038c014b) )

	ROM_REGION( 0xe00, "plds", ROMREGION_ERASE00 ) // probably all recreations, but chip types match
	ROM_LOAD( "peel18cv8p.u2",  0x000, 0x155, CRC(e40bc5c0) SHA1(207437a41ae04bf4d5093775a6f2e602d0840d12) )
	ROM_LOAD( "peel18cv8p.u4",  0x200, 0x155, CRC(bd152ad4) SHA1(bc79994d63efa7ef6e3cea4bd6f7940b68c40176) )
	ROM_LOAD( "peel18cv8p.u5",  0x400, 0x155, CRC(b03c21a0) SHA1(dde7644cca7b383ab19e3598f30250a6695816fd) )
	ROM_LOAD( "gal16v8b.u9",    0x600, 0x117, CRC(c9d37428) SHA1(9c06739d34a25095c0b0420910b82252ab0d05f8) )
	ROM_LOAD( "peel18cv8p.u10", 0x800, 0x155, CRC(e0a3a486) SHA1(20162b8c174e73fb7d57d9a221eef87fb88005a5) )
	ROM_LOAD( "gal16v8b.u11",   0xa00, 0x117, CRC(0b8a2c8f) SHA1(0cc9c9ed671f85a3ad8622b65f94b546bb13e6b6) )
	ROM_LOAD( "gal16v8b.u12",   0xc00, 0x117, CRC(a1d23a1a) SHA1(d83dc1b50771f5d8b68293c85b0b94e257521bd3) )
ROM_END


void sns_pro_fighter_qb_device::device_start()
{
}

void sns_pro_fighter_qb_device::device_reset()
{

}

const tiny_rom_entry *sns_pro_fighter_qb_device::device_rom_region() const
{
	return ROM_NAME( profghtqb );
}

void sns_pro_fighter_qb_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, "fdc", 24_MHz_XTAL); // actually ACCMicro 3201, divider?
	FLOPPY_CONNECTOR(config, "fdc:0", profght_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	SNS_CART_SLOT(config, "cartslot", 0, snes_cart, nullptr);
}


/*
Pro Fighter X (MB03D) copier

Main components:

GM82C765B floppy disk controller
16 MHz XTAL (for the FDC)
96813 CIC replacement on one PCB, 265111 CIC replacement on another
DDP1 custom (GRAPHIC DDP1-1 on one PCB, GINGER DDP1 9407 on another)
MK4864N-120 standard SRAM, 8k x 8
MS62256L-10PC CMOS Static RAM on one PCB; CXK58257M-10LL Static RAM on another
2 x D421700G5-80L-7JD Fast Page DRAM (on a sub board)
*/

sns_pro_fighter_x_device::sns_pro_fighter_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNS_PRO_FIGHTER_X, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
{
}

ROM_START( profghtx )
	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "pfx ver1.20.u3", 0x00000, 0x10000, CRC(04635d41) SHA1(e2765907d03f62329db224c6570d65aea5df5154) )

	ROM_REGION( 0x1200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce20v8h.u7",  0x0000, 0x0157, CRC(2a46e9b7) SHA1(3c2aca7dfa943e00b4227194ce90dfa89290d1a6) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce20v8h.u10", 0x0200, 0x0157, CRC(738765b4) SHA1(1357ca21112b9d6da735a1070deeaa8097cf2a6e) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "peel18cv8b.u12", 0x0400, 0x0155, CRC(e1ee3a45) SHA1(983918d504c16b20ae185679ed489b167c993609) ) // this is actually a recreation
	ROM_LOAD( "palce16v8h.u13", 0x0600, 0x0117, CRC(611c95a9) SHA1(b138041dd99af0bed37f178d2733764309279ee6) ) // probably actual dump
	ROM_LOAD( "palce16v8h.u14", 0x0800, 0x0117, CRC(82d9c905) SHA1(70c16fb86a16319246eae02d9e5439bc65453174) ) // probably actual dump
	ROM_LOAD( "palce20v8h.u16", 0x0a00, 0x0157, CRC(3593d7e8) SHA1(47094c6469fed5f10bea9f0f993b0257fc39e12f) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce20v8h.u17", 0x0c00, 0x0157, CRC(f58539ae) SHA1(c5105f0dd9c7d0f966b84444c17fe798cfcdcdc1) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce20v8h.u18", 0x0e00, 0x0157, CRC(97d8652f) SHA1(6bff3088fed8253cfd2b57a101e91e0a0e66bcff) ) // this is actually a recreation targeted for a GAL20V8 device
	ROM_LOAD( "palce20v8h.u19", 0x1000, 0x0157, CRC(2e42ed8f) SHA1(51f461756668a51e4f9b800168412dab26cd5ca9) ) // this is actually a recreation targeted for a GAL20V8 device
ROM_END


void sns_pro_fighter_x_device::device_start()
{
}

void sns_pro_fighter_x_device::device_reset()
{

}

const tiny_rom_entry *sns_pro_fighter_x_device::device_rom_region() const
{
	return ROM_NAME( profghtx );
}

void sns_pro_fighter_x_device::device_add_mconfig(machine_config &config)
{
	WD37C65C(config, "fdc", 16_MHz_XTAL); // actually GM82C765B
	FLOPPY_CONNECTOR(config, "fdc:0", profght_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	SNS_CART_SLOT(config, "cartslot", 0, snes_cart, nullptr);
}
