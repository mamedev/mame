// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega 9-pin controllers

    Unemulated Mega Drive peripherals:
    * Mega Modem (connects to EXP port on Mega Drive)
    * Sega Menacer (infrared wireless lightgun)
    * Konami Justifier (dual wired lightguns)
    * EA 4-Play (impractical - connects to both CTRL1 and CTRL2)

**********************************************************************/

#include "emu.h"
#include "controllers.h"

#include "diypaddle.h"
#include "hypershot.h"
#include "graphic.h"
#include "joypad.h"
#include "lphaser.h"
#include "md6bt.h"
#include "mdpad.h"
#include "mouse.h"
#include "multitap.h"
#include "paddle.h"
#include "rfu.h"
#include "rs232adapt.h"
#include "sports.h"
#include "sportsjp.h"
#include "teamplayer.h"
#include "xe1ap.h"


char const *const SMS_CTRL_OPTION_DIY_PADDLE  = "diypaddle";
char const *const SMS_CTRL_OPTION_HYPERSHOT   = "hypershot";
char const *const SMS_CTRL_OPTION_GRAPHIC     = "graphic";
char const *const SMS_CTRL_OPTION_LPHASER     = "lphaser";
char const *const SMS_CTRL_OPTION_MD_6BUTTON  = "md6button";
char const *const SMS_CTRL_OPTION_MD_PAD      = "mdpad";
char const *const SMS_CTRL_OPTION_MEGA_MOUSE  = "mouseus";
char const *const SMS_CTRL_OPTION_MULTITAP    = "multitap";
char const *const SMS_CTRL_OPTION_JOYPAD      = "mspad";
char const *const SMS_CTRL_OPTION_PADDLE      = "paddle";
char const *const SMS_CTRL_OPTION_RAPID_FIRE  = "rapidfire";
char const *const SMS_CTRL_OPTION_RS232       = "rs232";
char const *const SMS_CTRL_OPTION_SEGA_MOUSE  = "mouse";
char const *const SMS_CTRL_OPTION_SPORTS      = "sports";
char const *const SMS_CTRL_OPTION_SPORTS_JP   = "sportsjp";
char const *const SMS_CTRL_OPTION_TEAM_PLAYER = "teamplay";
char const *const SMS_CTRL_OPTION_XE1AP       = "xe1ap";



void sms_control_port_devices(device_slot_interface &device)
{
	device.option_add(SMS_CTRL_OPTION_DIY_PADDLE,  SMS_DIY_PADDLE);
	device.option_add(SMS_CTRL_OPTION_HYPERSHOT,   SMS_HYPERSHOT);
	device.option_add(SMS_CTRL_OPTION_GRAPHIC,     SMS_GRAPHIC);
	device.option_add(SMS_CTRL_OPTION_LPHASER,     SMS_LIGHT_PHASER);
	device.option_add(SMS_CTRL_OPTION_MD_6BUTTON,  SMS_MD6BUTTON);
	device.option_add(SMS_CTRL_OPTION_MD_PAD,      SMS_MDPAD);
	device.option_add(SMS_CTRL_OPTION_MEGA_MOUSE,  SMS_MEGAMOUSE);
	device.option_add(SMS_CTRL_OPTION_MULTITAP,    SMS_MULTITAP);
	device.option_add(SMS_CTRL_OPTION_JOYPAD,      SMS_JOYPAD);
	device.option_add(SMS_CTRL_OPTION_PADDLE,      SMS_PADDLE);
	device.option_add(SMS_CTRL_OPTION_RAPID_FIRE,  SMS_RAPID_FIRE);
	device.option_add(SMS_CTRL_OPTION_RS232,       SMS_RS232);
	device.option_add(SMS_CTRL_OPTION_SEGA_MOUSE,  SMS_SEGAMOUSE);
	device.option_add(SMS_CTRL_OPTION_SPORTS,      SMS_SPORTS_PAD);
	device.option_add(SMS_CTRL_OPTION_SPORTS_JP,   SMS_SPORTS_PAD_JP);
	device.option_add(SMS_CTRL_OPTION_TEAM_PLAYER, SMS_TEAM_PLAYER);
	device.option_add(SMS_CTRL_OPTION_XE1AP,       SMS_XE1AP);
}


void sms_control_port_passive_devices(device_slot_interface &device)
{
	device.option_add(SMS_CTRL_OPTION_HYPERSHOT,   SMS_HYPERSHOT);
	device.option_add(SMS_CTRL_OPTION_JOYPAD,      SMS_JOYPAD);
}
