// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files
***************************************************************************/

#pragma once

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "xmlfile.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_VERSION          10

enum
{
	CONFIG_TYPE_INIT = 0,                   /* opportunity to initialize things first */
	CONFIG_TYPE_CONTROLLER,                 /* loading from controller file */
	CONFIG_TYPE_DEFAULT,                    /* loading from default.cfg */
	CONFIG_TYPE_GAME,                   /* loading from game.cfg */
	CONFIG_TYPE_FINAL                   /* opportunity to finish initialization */
};



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef delegate<void (int, xml_data_node *)> config_saveload_delegate;



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

void config_init(running_machine &machine);
void config_register(running_machine &machine, const char *nodename, config_saveload_delegate load, config_saveload_delegate save);
int config_load_settings(running_machine &machine);
void config_save_settings(running_machine &machine);

#endif  /* __CONFIG_H__ */
