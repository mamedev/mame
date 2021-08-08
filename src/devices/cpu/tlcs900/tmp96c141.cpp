// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Toshiba TMP96C141 microcontroller

    TMP96CM40 is the same as TMP96C141 with the addition of 32K of on-chip
    ROM. TMP96PM40 is similar but with 32K OTP. TMP96C041 is RAMless as well
    as ROMless.

****************************************************************************/

#include "emu.h"
#include "tmp96c141.h"
#include "dasm900.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TMP96C141, tmp96c141_device, "tmp96c141", "Toshiba TMP96C141")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  tmp96c141_device - constructor
//-------------------------------------------------

tmp96c141_device::tmp96c141_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tlcs900_device(mconfig, TMP96C141, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - device-specific startup
//-------------------------------------------------

void tmp96c141_device::device_config_complete()
{
	if (m_am8_16 == 0)
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(tmp96c141_device::internal_mem), this));
	else
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 8, 24, 0, address_map_constructor(FUNC(tmp96c141_device::internal_mem), this));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmp96c141_device::device_start()
{
	tlcs900_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmp96c141_device::device_reset()
{
	tlcs900_device::device_reset();
}


//**************************************************************************
//  INTERNAL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  internal_mem - memory map for internal RAM and
//  I/O registers
//-------------------------------------------------

void tmp96c141_device::internal_mem(address_map &map)
{
	map(0x000000, 0x00007f).unmaprw();
	//map(0x000000, 0x000000).rw(FUNC(tmp96c141_device::p0_r), FUNC(tmp96c141_device::p0_w));
	//map(0x000001, 0x000001).rw(FUNC(tmp96c141_device::p1_r), FUNC(tmp96c141_device::p1_w));
	//map(0x000002, 0x000002).w(FUNC(tmp96c141_device::p0cr_w));
	//map(0x000004, 0x000004).w(FUNC(tmp96c141_device::p1cr_w));
	//map(0x000005, 0x000005).w(FUNC(tmp96c141_device::p1fc_w));
	//map(0x000006, 0x000006).rw(FUNC(tmp96c141_device::p2_r), FUNC(tmp96c141_device::p2_w));
	//map(0x000007, 0x000007).rw(FUNC(tmp96c141_device::p3_r), FUNC(tmp96c141_device::p3_w));
	//map(0x000008, 0x000008).w(FUNC(tmp96c141_device::p2cr_w));
	//map(0x000009, 0x000009).w(FUNC(tmp96c141_device::p2fc_w));
	//map(0x00000a, 0x00000a).w(FUNC(tmp96c141_device::p3cr_w));
	//map(0x00000b, 0x00000b).w(FUNC(tmp96c141_device::p3fc_w));
	//map(0x00000c, 0x00000c).rw(FUNC(tmp96c141_device::p4_r), FUNC(tmp96c141_device::p4_w));
	//map(0x00000d, 0x00000d).r(FUNC(tmp96c141_device::p5_r));
	//map(0x00000e, 0x00000e).w(FUNC(tmp96c141_device::p4cr_w));
	//map(0x000010, 0x000010).w(FUNC(tmp96c141_device::p4fc_w));
	//map(0x000012, 0x000012).rw(FUNC(tmp96c141_device::p6_r), FUNC(tmp96c141_device::p6_w));
	//map(0x000013, 0x000013).rw(FUNC(tmp96c141_device::p7_r), FUNC(tmp96c141_device::p7_w));
	//map(0x000014, 0x000014).w(FUNC(tmp96c141_device::p6cr_w));
	//map(0x000015, 0x000015).w(FUNC(tmp96c141_device::p6fc_w));
	//map(0x000016, 0x000016).w(FUNC(tmp96c141_device::p7cr_w));
	//map(0x000017, 0x000017).w(FUNC(tmp96c141_device::p7fc_w));
	//map(0x000018, 0x000018).rw(FUNC(tmp96c141_device::p8_r), FUNC(tmp96c141_device::p8_w));
	//map(0x000019, 0x000019).rw(FUNC(tmp96c141_device::p9_r), FUNC(tmp96c141_device::p9_w));
	//map(0x00001a, 0x00001a).w(FUNC(tmp96c141_device::p8cr_w));
	//map(0x00001b, 0x00001b).w(FUNC(tmp96c141_device::p8fc_w));
	//map(0x00001c, 0x00001c).w(FUNC(tmp96c141_device::p9cr_w));
	//map(0x00001d, 0x00001d).w(FUNC(tmp96c141_device::p9fc_w));
	//map(0x000020, 0x000020).rw(FUNC(tmp96c141_device::trun_r), FUNC(tmp96c141_device::trun_w));
	//map(0x000022, 0x000023).w(FUNC(tmp96c141_device::treg01_w));
	//map(0x000024, 0x000024).w(FUNC(tmp96c141_device::tmod_w));
	//map(0x000025, 0x000025).rw(FUNC(tmp96c141_device::tffcr_r), FUNC(tmp96c141_device::tffcr_w));
	//map(0x000026, 0x000027).rw(FUNC(tmp96c141_device::treg23_r), FUNC(tmp96c141_device::treg23_w));
	//map(0x000028, 0x000029).rw(FUNC(tmp96c141_device::pwmod_r), FUNC(tmp96c141_device::pwmod_w));
	//map(0x00002a, 0x00002a).rw(FUNC(tmp96c141_device::pffcr_r), FUNC(tmp96c141_device::pffcr_w));
	//map(0x000030, 0x000033).w(FUNC(tmp96c141_device::treg45_w));
	//map(0x000034, 0x000037).r(FUNC(tmp96c141_device::cap12_r));
	//map(0x000038, 0x000038).rw(FUNC(tmp96c141_device::t4mod_r), FUNC(tmp96c141_device::t4mod_w));
	//map(0x000039, 0x000039).rw(FUNC(tmp96c141_device::t4ffcr_r), FUNC(tmp96c141_device::t4ffcr_w));
	//map(0x00003a, 0x00003a).rw(FUNC(tmp96c141_device::t45cr_r), FUNC(tmp96c141_device::t45cr_w));
	//map(0x000040, 0x000043).w(FUNC(tmp96c141_device::treg67_w));
	//map(0x000044, 0x000047).r(FUNC(tmp96c141_device::cap34_r));
	//map(0x000048, 0x000048).rw(FUNC(tmp96c141_device::t5mod_r), FUNC(tmp96c141_device::t5mod_w));
	//map(0x000049, 0x000049).rw(FUNC(tmp96c141_device::t5ffcr_r), FUNC(tmp96c141_device::t5ffcr_w));
	//map(0x00004c, 0x00004d).rw(FUNC(tmp96c141_device::pgreg_r), FUNC(tmp96c141_device::pgreg_w));
	//map(0x00004e, 0x00004e).rw(FUNC(tmp96c141_device::pg01cr_r), FUNC(tmp96c141_device::pg01cr_w));
	//map(0x000050, 0x000050).rw(FUNC(tmp96c141_device::sc0buf_r), FUNC(tmp96c141_device::sc0buf_w));
	//map(0x000051, 0x000051).rw(FUNC(tmp96c141_device::sc0cr_r), FUNC(tmp96c141_device::sc0cr_w));
	//map(0x000052, 0x000052).rw(FUNC(tmp96c141_device::sc0mod_r), FUNC(tmp96c141_device::sc0mod_w));
	//map(0x000053, 0x000053).rw(FUNC(tmp96c141_device::br0cr_r), FUNC(tmp96c141_device::br0cr_w));
	//map(0x000054, 0x000054).rw(FUNC(tmp96c141_device::sc1buf_r), FUNC(tmp96c141_device::sc1buf_w));
	//map(0x000055, 0x000055).rw(FUNC(tmp96c141_device::sc1cr_r), FUNC(tmp96c141_device::sc1cr_w));
	//map(0x000056, 0x000056).rw(FUNC(tmp96c141_device::sc1mod_r), FUNC(tmp96c141_device::sc1mod_w));
	//map(0x000057, 0x000057).rw(FUNC(tmp96c141_device::br1cr_r), FUNC(tmp96c141_device::br1cr_w));
	//map(0x000058, 0x000058).rw(FUNC(tmp96c141_device::ode_r), FUNC(tmp96c141_device::ode_w));
	//map(0x00005c, 0x00005c).rw(FUNC(tmp96c141_device::wdmod_r), FUNC(tmp96c141_device::wdmod_w));
	//map(0x00005d, 0x00005d).w(FUNC(tmp96c141_device::wdcr_w));
	//map(0x00005e, 0x00005e).rw(FUNC(tmp96c141_device::admod_r), FUNC(tmp96c141_device::admod_w));
	//map(0x000060, 0x000067).r(FUNC(tmp96c141_device::adreg_r));
	//map(0x000068, 0x00006a).w(FUNC(tmp96c141_device::bcs_w));
	//map(0x000070, 0x000078).rw(FUNC(tmp96c141_device::inte_r), FUNC(tmp96c141_device::inte_w));
	//map(0x00007b, 0x00007b).w(FUNC(tmp96c141_device::iimc_w));
	//map(0x00007c, 0x00007f).w(FUNC(tmp96c141_device::dmav_w));
	map(0x000080, 0x00047f).ram();
}


//**************************************************************************
//  EXECUTION CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  tlcs900_check_hdma -
//-------------------------------------------------

void tmp96c141_device::tlcs900_check_hdma()
{
}


//-------------------------------------------------
//  tlcs900_check_irqs -
//-------------------------------------------------

void tmp96c141_device::tlcs900_check_irqs()
{
}


//-------------------------------------------------
//  tlcs900_handle_ad -
//-------------------------------------------------

void tmp96c141_device::tlcs900_handle_ad()
{
}


//-------------------------------------------------
//  tlcs900_handle_timers -
//-------------------------------------------------

void tmp96c141_device::tlcs900_handle_timers()
{
}


//-------------------------------------------------
//  execute_set_input - called when a synchronized
//  input is changed
//-------------------------------------------------

void tmp96c141_device::execute_set_input(int inputnum, int state)
{
}

std::unique_ptr<util::disasm_interface> tmp96c141_device::create_disassembler()
{
	return std::make_unique<tmp96c141_disassembler>();
}
