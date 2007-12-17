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

#ifndef INC_MMTR
#define INC_MMTR

#define MAXMECHMETERS 8

#define METERREACTTIME 30000	// number of cycles meter has to be active to tick

void Mechmtr_init(     int number);

int  MechMtr_GetNumberMeters(void);

void MechMtr_Setcount( int id, long count);

long MechMtr_Getcount( int id);

void MechMtr_ReactTime(int id, long cycles);

int  Mechmtr_update(   int id, long cycles, int state); // returns 1 if meter ticked

#endif
