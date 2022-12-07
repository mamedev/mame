// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega DE-9 controllers

**********************************************************************/

#include "emu.h"
#include "controllers.h"

#include "graphic.h"
#include "joypad.h"
#include "lphaser.h"
#include "md6bt.h"
#include "mdpad.h"
#include "multitap.h"
#include "paddle.h"
#include "rfu.h"
#include "sports.h"
#include "sportsjp.h"


char const *const SMS_CTRL_OPTION_GRAPHIC    = "graphic";
char const *const SMS_CTRL_OPTION_LPHASER    = "lphaser";
char const *const SMS_CTRL_OPTION_MD_6BUTTON = "md6button";
char const *const SMS_CTRL_OPTION_MD_PAD     = "mdpad";
char const *const SMS_CTRL_OPTION_MULTITAP   = "multitap";
char const *const SMS_CTRL_OPTION_JOYPAD     = "mspad";
char const *const SMS_CTRL_OPTION_PADDLE     = "paddle";
char const *const SMS_CTRL_OPTION_RAPID_FIRE = "rapidfire";
char const *const SMS_CTRL_OPTION_SPORTS     = "sports";
char const *const SMS_CTRL_OPTION_SPORTS_JP  = "sportsjp";



void sms_control_port_devices(device_slot_interface &device)
{
	device.option_add(SMS_CTRL_OPTION_GRAPHIC,    SMS_GRAPHIC);
	device.option_add(SMS_CTRL_OPTION_LPHASER,    SMS_LIGHT_PHASER);
	device.option_add(SMS_CTRL_OPTION_MD_6BUTTON, SMS_MD6BUTTON);
	device.option_add(SMS_CTRL_OPTION_MD_PAD,     SMS_MDPAD);
	device.option_add(SMS_CTRL_OPTION_MULTITAP,   SMS_MULTITAP);
	device.option_add(SMS_CTRL_OPTION_JOYPAD,     SMS_JOYPAD);
	device.option_add(SMS_CTRL_OPTION_PADDLE,     SMS_PADDLE);
	device.option_add(SMS_CTRL_OPTION_RAPID_FIRE, SMS_RAPID_FIRE);
	device.option_add(SMS_CTRL_OPTION_SPORTS,     SMS_SPORTS_PAD);
	device.option_add(SMS_CTRL_OPTION_SPORTS_JP,  SMS_SPORTS_PAD_JP);
}


void sms_control_port_passive_devices(device_slot_interface &device)
{
	device.option_add(SMS_CTRL_OPTION_JOYPAD,     SMS_JOYPAD);
}
