// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Electro mechanical meters                                             //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "emu.h"
#include "meters.h"

// local vars /////////////////////////////////////////////////////////////

static struct
{
	long on,    // Activity of reel
		reacttime,
		count;      // mechmeter value
	int  state;     // state 0/1
	emu_timer *meter_timer;
} meter_info[MAXMECHMETERS];

static int number_mtr;

///////////////////////////////////////////////////////////////////////////
static TIMER_CALLBACK( meter_callback )
{
	meter_info[param].count++;
}

void MechMtr_config(running_machine &machine, int number)
{
	int i;

	if ( number > MAXMECHMETERS ) number = MAXMECHMETERS;

	for ( i = 0; i < number; i++ )
	{
		meter_info[i].reacttime = METERREACTTIME;
		meter_info[i].state     = 0;
		meter_info[i].count     = 0;
		meter_info[i].on        = 0;
		meter_info[i].meter_timer = machine.scheduler().timer_alloc(FUNC(meter_callback), (void*)(FPTR)i);
		meter_info[i].meter_timer->reset();
	}
	number_mtr = number;
}

///////////////////////////////////////////////////////////////////////////

int MechMtr_GetNumberMeters(void)
{
	return number_mtr;
}

///////////////////////////////////////////////////////////////////////////

void MechMtr_Setcount(int id, long count)
{
	if ( id >= number_mtr ) return;

	meter_info[id].count = count;
}

///////////////////////////////////////////////////////////////////////////

long MechMtr_Getcount(int id)
{
	long result = 0;

	if ( id < number_mtr ) result = meter_info[id].count;

	return result;
}

///////////////////////////////////////////////////////////////////////////

void MechMtr_ReactTime(int id, long cycles)
{
	if ( id >= number_mtr ) return;
	meter_info[id].reacttime = cycles;
}

///////////////////////////////////////////////////////////////////////////

int MechMtr_GetActivity(int id)
{
	return meter_info[id].on;
}

///////////////////////////////////////////////////////////////////////////

int MechMtr_update(int id, int state)
{
	int res = 0;

	if ( id >= number_mtr ) return res;

	state = state?1:0;

	if ( meter_info[id].state != state )
	{   // meter state is changing
	meter_info[id].state = state;

	if ( state )
	{
		meter_info[id].on =1;
		meter_info[id].meter_timer->adjust(attotime::from_seconds(meter_info[id].reacttime), id);
	}
	else
	{
		meter_info[id].on =0;
		meter_info[id].meter_timer->adjust(attotime::never, id);
	}
	}

	return meter_info[id].on;
}
