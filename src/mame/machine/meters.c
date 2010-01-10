///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Electro mechanical meters                                             //
//                                                                       //
// 23-07-2004: Re-Animator                                               //
//                                                                       //
// TODO: - meter ticks if the signal changes from high to low AND the    //
//         signal was high for at least 'reacttime' cycles               //
//         It should tick if the signal goes from low to high AND stays  //
//         high for at least xxx milliseconds                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "emu.h"
#include "meters.h"

// local vars /////////////////////////////////////////////////////////////

static struct
{
  long timestamp,	// time stamp in cycles after which timer will tick
	   reacttime,
	   count;		// mechmeter value
  int  state;		// state 0/1
} meter_info[MAXMECHMETERS];


static int number_mtr;

///////////////////////////////////////////////////////////////////////////

void Mechmtr_init(int number)
{
  int i;

  if ( number > MAXMECHMETERS ) number = MAXMECHMETERS;

  for ( i = 0; i < number; i++ )
  {
	meter_info[i].reacttime = METERREACTTIME;
	meter_info[i].state     = 0;
	meter_info[i].count     = 0;
	meter_info[i].timestamp = 0;
  }
  number_mtr = number;
}

///////////////////////////////////////////////////////////////////////////

int  MechMtr_GetNumberMeters(void)
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

int Mechmtr_update(int id, long cycles, int state)
{
  int res = 0;

  if ( id >= number_mtr ) return res;

  state = state?1:0;

  if ( meter_info[id].state != state )
  {	// meter state is changing
	meter_info[id].state = state;

	if ( state )
	{
	  meter_info[id].timestamp = cycles + meter_info[id].reacttime;
	}
	else
	{
	  if ( cycles > meter_info[id].timestamp )
	  { // meter has been active long enough
		res = 1;
		meter_info[id].count++;
	  }
	}
  }

  return res;
}
