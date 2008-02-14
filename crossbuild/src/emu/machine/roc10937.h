/**********************************************************************

    Rockwell 10937/10957 interface and emulation by J.Wallace
    OKI MSC1937 is a clone of this chip

**********************************************************************/

#ifndef ROC10937
#define ROC10937

#define MAX_ROCK_ALPHAS  3	  // max number of displays emulated

#define ROCKWELL10937 0	// Rockwell 10937
#define MSC1937 0 		// OKI MSC1937 clone of Rockwell 10937
#define ROCKWELL10957 1	// Rockwell 10957

void	ROC10937_init(  int id, int type,int reversed );		// setup a display

void	ROC10937_reset( int id);				// reset the alpha

void	ROC10937_shift_data(int id, int data);	// clock in a bit of data

int		ROC10937_newdata(   int id, int data);	// clock in 8 bits of data

UINT32	*ROC10937_get_segments(int id);			// get current segments displayed
UINT32  *ROC10937_set_outputs(int id);			// convert segments to standard for display
UINT32  *ROC10937_get_outputs(int id);			// get converted segments

char	*ROC10937_get_string( int id);			// get current string   displayed (not as accurate)

void	ROC10937_draw_16seg(int id);
void	ROC10937_draw_14seg(int id);
#endif

