// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "vme_cards.h"

#include "cp31.h"
#include "enp10.h"
#include "hcpu30.h"
#include "hk68v10.h"
#include "mvme120.h"
#include "mvme147.h"
#include "mvme180.h"
#include "mvme181.h"
#include "mvme187.h"
#include "mvme327a.h"
#include "mvme328.h"
#include "mvme350.h"
#include "mzr8105.h"
#include "mzr8300.h"
#include "smvme2000.h"
#include "sys68k_cpu1.h"
#include "sys68k_cpu20.h"
#include "sys68k_cpu30.h"
#include "sys68k_iscsi.h"
#include "sys68k_isio.h"
#include "tp881v.h"

void vme_cards(device_slot_interface &device)
{
	device.option_add("cp31",             VME_CP31);
	device.option_add("enp10",            VME_ENP10);
	device.option_add("hcpu30",           VME_HCPU30);
	device.option_add("hk68v10",          VME_HK68V10);
	device.option_add("mvme120",          VME_MVME120);
	device.option_add("mvme121",          VME_MVME121);
	device.option_add("mvme122",          VME_MVME122);
	device.option_add("mvme123",          VME_MVME123);
	device.option_add("mvme147",          VME_MVME147);
	device.option_add("mvme180",          VME_MVME180);
	device.option_add("mvme181",          VME_MVME181);
	device.option_add("mvme187",          VME_MVME187);
	device.option_add("mvme327a",         VME_MVME327A);
	device.option_add("mvme328",          VME_MVME328);
	device.option_add("mvme350",          VME_MVME350);
	device.option_add("mzr8105",          VME_MZR8105);
	device.option_add("mzr8300",          VME_MZR8300);
	device.option_add("smvme2000",        VME_SMVME2000);
	device.option_add("sys68k_cpu1",      VME_SYS68K_CPU1);
	device.option_add("sys68k_cpu20",     VME_SYS68K_CPU20);
	device.option_add("sys68k_cpu21",     VME_SYS68K_CPU21);
	device.option_add("sys68k_cpu21a",    VME_SYS68K_CPU21A);
	device.option_add("sys68k_cpu21b",    VME_SYS68K_CPU21B);
	device.option_add("sys68k_cpu21s",    VME_SYS68K_CPU21S);
	device.option_add("sys68k_cpu21ya",   VME_SYS68K_CPU21YA);
	device.option_add("sys68k_cpu21yb",   VME_SYS68K_CPU21YB);
	device.option_add("sys68k_cpu30",     VME_SYS68K_CPU30);
	device.option_add("sys68k_cpu30x",    VME_SYS68K_CPU30X);
	device.option_add("sys68k_cpu30xa",   VME_SYS68K_CPU30XA);
	device.option_add("sys68k_cpu30za",   VME_SYS68K_CPU30ZA);
	device.option_add("sys68k_cpu30be",   VME_SYS68K_CPU30BE);
	device.option_add("sys68k_cpu30lite", VME_SYS68K_CPU30LITE);
	device.option_add("sys68k_cpu33",     VME_SYS68K_CPU33);
	device.option_add("sys68k_iscsi1",    VME_SYS68K_ISCSI1),
	device.option_add("sys68k_isio1",     VME_SYS68K_ISIO1),
	device.option_add("tp881v",           VME_TP881V);
}
