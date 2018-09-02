// license:BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    Amiga Keyboard Matrix Ports

 ***************************************************************************/

#ifndef MAME_BUS_AMIGA_KEYBOARD_MATRIX_H
#define MAME_BUS_AMIGA_KEYBOARD_MATRIX_H

#pragma once

namespace bus { namespace amiga { namespace keyboard {

INPUT_PORTS_EXTERN(matrix_us);
INPUT_PORTS_EXTERN(matrix_de);
INPUT_PORTS_EXTERN(matrix_fr);
INPUT_PORTS_EXTERN(matrix_it);
INPUT_PORTS_EXTERN(matrix_se);
INPUT_PORTS_EXTERN(matrix_es);
INPUT_PORTS_EXTERN(matrix_dk);
INPUT_PORTS_EXTERN(matrix_ch);
INPUT_PORTS_EXTERN(matrix_no);
INPUT_PORTS_EXTERN(matrix_gb);

INPUT_PORTS_EXTERN(a1000_keypad);
INPUT_PORTS_EXTERN(remove_keypad);

} } } // namespace bus::amiga::keyboard

#endif // MAME_BUS_AMIGA_KEYBOARD_MATRIX_H
