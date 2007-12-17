/***************************************************************************

    Atari Cops'n Robbers hardware

***************************************************************************/

#include "driver.h"
#include "copsnrob.h"

static int gun_mask[] = {0x7e, 0x7d, 0x7b, 0x77, 0x6f, 0x5f, 0x3f};

// The gun control is a 7 position switch. I'm doing the following to
// emulate it:
//
// I read out the current gun position via the sprite image locations,
// and then decrement/increment it if the up/down keys are pressed.

READ8_HANDLER( copsnrob_gun_position_r )
{
    int keys, current_car_image, current_gun_pos = 0;

    // Determine which player we need
    switch (offset)
    {
    default:
    case 0x00:
        current_car_image = copsnrob_carimage[0];
        keys = input_port_4_r(0);
        break;
    case 0x04:
        current_car_image = copsnrob_carimage[1];
        keys = input_port_5_r(0);
        break;
    case 0x08:
        current_car_image = copsnrob_carimage[2];
        keys = input_port_6_r(0);
        break;
    case 0x0c:
        current_car_image = copsnrob_carimage[3];
        keys = input_port_7_r(0);
    }

    if (current_car_image < 7)
    {
        current_gun_pos = 6 - current_car_image;
    }
    else if (current_car_image < 14)
    {
        current_gun_pos = 13 - current_car_image;
    }

    // Gun up
    if ((keys & 0x01) && (current_gun_pos != 6))  current_gun_pos++;

    // Gun down
    if ((keys & 0x02) && (current_gun_pos != 0))  current_gun_pos--;

    return (keys & 0x80) | gun_mask[current_gun_pos];
}
