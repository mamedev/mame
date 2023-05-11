// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega 9-pin controllers

**********************************************************************/
#ifndef MAME_BUS_SMS_CTRL_CONTROLLERS_H
#define MAME_BUS_SMS_CTRL_CONTROLLERS_H

#pragma once


extern char const *const SMS_CTRL_OPTION_DIY_PADDLE;
extern char const *const SMS_CTRL_OPTION_HYPERSHOT;
extern char const *const SMS_CTRL_OPTION_GRAPHIC;
extern char const *const SMS_CTRL_OPTION_LPHASER;
extern char const *const SMS_CTRL_OPTION_MD_6BUTTON;
extern char const *const SMS_CTRL_OPTION_MD_PAD;
extern char const *const SMS_CTRL_OPTION_MEGA_MOUSE;
extern char const *const SMS_CTRL_OPTION_MULTITAP;
extern char const *const SMS_CTRL_OPTION_JOYPAD;
extern char const *const SMS_CTRL_OPTION_PADDLE;
extern char const *const SMS_CTRL_OPTION_RAPID_FIRE;
extern char const *const SMS_CTRL_OPTION_RS232;
extern char const *const SMS_CTRL_OPTION_SEGA_MOUSE;
extern char const *const SMS_CTRL_OPTION_SPORTS;
extern char const *const SMS_CTRL_OPTION_SPORTS_JP;
extern char const *const SMS_CTRL_OPTION_TEAM_PLAYER;
extern char const *const SMS_CTRL_OPTION_XE1AP;

void sms_control_port_devices(device_slot_interface &device);
void sms_control_port_passive_devices(device_slot_interface &device);

#endif // MAME_BUS_SMS_CTRL_CONTROLLERS_H
