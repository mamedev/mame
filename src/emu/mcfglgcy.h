// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mcfglgcy.h

    Legacy machine configuration helpers.

***************************************************************************/

#pragma once

#ifndef __MCFGLGCY_H__
#define __MCFGLGCY_H__

// core functions
#define MCFG_NVRAM_HANDLER(_func) \
	config.m_nvram_handler = NVRAM_HANDLER_NAME(_func);
#define MCFG_MEMCARD_HANDLER(_func) \
	config.m_memcard_handler = MEMCARD_HANDLER_NAME(_func);

#endif  /* __MCFGLGCY_H__ */
