/*
 * uPD4701
 *
 * Incremental Encoder Control
 *
 */

#include "driver.h"
#include "machine/upd4701.h"

struct uPD4701_chip
{
	int cs;
	int xy;
	int ul;
	int resetx;
	int resety;
	int latchx;
	int latchy;
	int startx;
	int starty;
	int x;
	int y;
	int switches;
	int latchswitches;
	int cf;
};

struct uPD4701_chip uPD4701[ UPD4701_MAXCHIP ];

#define MASK_SWITCHES ( 7 )
#define MASK_COUNTER ( 0xfff )

void uPD4701_init( int chip )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_init( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];

	c->cs = 1;
	c->xy = 0;
	c->ul = 0;
	c->resetx = 0;
	c->resety = 0;
	c->latchx = 0;
	c->latchy = 0;
	c->startx = 0;
	c->starty = 0;
	c->x = 0;
	c->y = 0;
	c->switches = 0;
	c->latchswitches = 0;
	c->cf = 1;

	state_save_register_item( "uPD4701", chip, c->cs );
	state_save_register_item( "uPD4701", chip, c->xy );
	state_save_register_item( "uPD4701", chip, c->ul );
	state_save_register_item( "uPD4701", chip, c->resetx );
	state_save_register_item( "uPD4701", chip, c->resety );
	state_save_register_item( "uPD4701", chip, c->latchx );
	state_save_register_item( "uPD4701", chip, c->latchy );
	state_save_register_item( "uPD4701", chip, c->startx );
	state_save_register_item( "uPD4701", chip, c->starty );
	state_save_register_item( "uPD4701", chip, c->x );
	state_save_register_item( "uPD4701", chip, c->y );
	state_save_register_item( "uPD4701", chip, c->switches );
	state_save_register_item( "uPD4701", chip, c->latchswitches );
	state_save_register_item( "uPD4701", chip, c->cf );
}

void uPD4701_ul_w( int chip, int ul )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_ul_w( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	c->ul = ul;
}

void uPD4701_xy_w( int chip, int xy )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_xy_w( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	c->xy = xy;
}

void uPD4701_cs_w( int chip, int cs )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_cs_w( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	if( cs != c->cs )
	{
		c->cs = cs;

		if( !c->cs )
		{
			c->latchx = ( c->x - c->startx ) & MASK_COUNTER;
			c->latchy = ( c->y - c->starty ) & MASK_COUNTER;

			c->latchswitches = ( ~c->switches ) & MASK_SWITCHES;
			if( c->latchswitches != 0 )
			{
				c->latchswitches |= 8;
			}

			c->cf = 1;
		}
	}
}

void uPD4701_resetx_w( int chip, int resetx )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_resetx_w( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	if( c->resetx != resetx )
	{
		c->resetx = resetx;

		if( c->resetx )
		{
			c->startx = c->x;
		}
	}
}

void uPD4701_resety_w( int chip, int resety )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_resety_w( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	if( c->resety != resety )
	{
		c->resety = resety;

		if( c->resety )
		{
			c->starty = c->y;
		}
	}
}

int uPD4701_d_r( int chip )
{
	int data;
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_d_r( %d ) invalid chip\n", chip );
		return 0;
	}

	c = &uPD4701[ chip ];
	if( c->cs )
	{
		return 0xff;
	}

	if( c->xy )
	{
		data = c->latchy;
	}
	else
	{
		data = c->latchx;
	}

	data |= c->latchswitches << 12;

	if( c->ul )
	{
		return data >> 8;
	}
	else
	{
		return data & 0xff;
	}
}

int uPD4701_sf_r( int chip )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_sf_r( %d ) invalid chip\n", chip );
		return 0;
	}

	c = &uPD4701[ chip ];
	if( ( c->switches & MASK_SWITCHES ) != MASK_SWITCHES )
	{
		return 0;
	}

	return 1;
}

int uPD4701_cf_r( int chip )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_cf_r( %d ) invalid chip\n", chip );
		return 0;
	}

	c = &uPD4701[ chip ];
	return c->cf;
}

void uPD4701_x_add( int chip, int dx )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_x_add( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	if( !c->resetx && dx != 0 )
	{
		c->x += dx;

		if( c->cs )
		{
			c->cf = 0;
		}
	}
}

void uPD4701_y_add( int chip, int dy )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_y_add( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	if( !c->resety && dy != 0 )
	{
		c->y += dy;

		if( c->cs )
		{
			c->cf = 0;
		}
	}
}

void uPD4701_switches_set( int chip, int switches )
{
	struct uPD4701_chip *c;

	if( chip < 0 || chip >= UPD4701_MAXCHIP )
	{
		logerror( "uPD4701_switches_set( %d ) invalid chip\n", chip );
		return;
	}

	c = &uPD4701[ chip ];
	c->switches = switches;
}
