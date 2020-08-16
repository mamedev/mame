/*
 * types.h - Type definitions for VICE.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  André Fachat <a.fachat@physik.tu-chemnitz.de>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef _TYPES_H
#define _TYPES_H

/*#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif*/

#ifndef DWORD
   //#ifdef DWORD_IS_LONG
   #define DWORD unsigned long
   //#else
   //#define DWORD unsigned int
   //#endif
#endif

/* RGJ added for AppleWin */
#ifndef ULONG 
typedef unsigned long ULONG;
#endif

typedef signed char SIGNED_CHAR;
typedef signed short SWORD;
typedef signed int SDWORD;

typedef DWORD CLOCK;
/* Maximum value of a CLOCK.  */
#define CLOCK_MAX (~((CLOCK)0))
/*
#if defined(__GNUC__) && defined(__i386__) && !defined(NO_REGPARM)
#define REGPARM1 __attribute__((regparm(1)))
#define REGPARM2 __attribute__((regparm(2)))
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM1
#define REGPARM2
#define REGPARM3
#endif
*/
#endif

