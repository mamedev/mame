/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "mame.h"
#include "input.h"
#include "xmlfile.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_TYPE_INIT			0		/* opportunity to initialize things first */
#define CONFIG_TYPE_CONTROLLER		1		/* loading from controller file */
#define CONFIG_TYPE_DEFAULT			2		/* loading from default.cfg */
#define CONFIG_TYPE_GAME			3		/* loading from game.cfg */
#define CONFIG_TYPE_FINAL			4		/* opportunity to finish initialization */



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef void (*config_callback)(int config_type, xml_data_node *parentnode);



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

void config_init(running_machine *machine);
void config_register(const char *nodename, config_callback load, config_callback save);
int config_load_settings(void);
void config_save_settings(void);

#endif	/* __CONFIG_H__ */
