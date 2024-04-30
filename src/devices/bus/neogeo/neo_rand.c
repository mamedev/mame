// ++===================================================================++
// ||           	 rand.c - Random number generator					||
// ++-------------------------------------------------------------------++
// || A C-program for MT19937: Integer version							||
// || genrand() generates one pseudorandom unsigned integer (32bit)		||
// || which is uniformly distributed among 0 to 2^32-1 for each			||
// || call. sgenrand(seed) set initial values to the working area		||
// || of 624 words. Before genrand(), sgenrand(seed) must be			||
// || called once. (seed is any 32-bit integer except for 0).			||
// || Coded by Takuji Nishimura, considering the suggestions by			||
// || Topher Cooper and Marc Rieffel in July-Aug. 1997.					||
// || This library is free software; you can redistribute it and/or		||
// || modify it under the terms of the GNU Library General Public		||
// || License as published by the Free Software Foundation; either		||
// || version 2 of the License, or (at your option) any later			||
// || version.															||
// || This library is distributed in the hope that it will be useful,	||
// || but WITHOUT ANY WARRANTY; without even the implied warranty of	||
// || MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.				||
// || See the GNU Library General Public License for more details.		||
// || You should have received a copy of the GNU Library General		||
// || Public License along with this library; if not, write to the		||
// || Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA		||
// || 02111-1307 USA													||
// || Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.			||
// || Any feedback is very welcome. For any question, comments,			||
// || see http://www.math.keio.ac.jp/matumoto/emt.html or email			||
// || matumoto@m...														||
// || minor modifications by M.Brent. Don't bug these other guys		||
// || if *I* broke it!													||
// ++===================================================================++

#include <stdlib.h>

typedef uint32_t DWORD;

// Period parameters
#define N			624
#define M 			397
#define MATRIX_A	0x9908b0df // constant vector a
#define UPPER_MASK	0x80000000 // most significant w-r bits
#define LOWER_MASK	0x7fffffff // least significant r bits

// Tempering parameters
#define TEMPERING_MASK_B		0x9d2c5680
#define TEMPERING_MASK_C		0xefc60000
#define TEMPERING_SHIFT_U(y)	(y >> 11)
#define TEMPERING_SHIFT_S(y)	(y << 7)
#define TEMPERING_SHIFT_T(y)	(y << 15)
#define TEMPERING_SHIFT_L(y)	(y >> 18)

static DWORD	mt[N];		// the array for the state vector */
static int		mti;

// Initialize with a default seed
void __attribute__ ((constructor)) rand_init(void)
{
	srand(4357);	// a default initial seed is used
}

// initializing the array with a NONZERO seed
void srand(DWORD seed)
{
// setting initial seeds to mt[N] using the generator Line 25 of Table 1 in
// [KNUTH 1981, The Art of Computer Programming Vol. 2 (2nd Ed.), pp102]
	mt[0] = seed & 0xffffffff;
	for(mti = 1;mti < N;mti++)
		mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

// generate a pseudo-random number
DWORD neo_rand(void)
{
	const DWORD	mag01[2] = { 0x0, MATRIX_A };
	DWORD 		y;
	int			kk;

	// mag01[x] = x * MATRIX_A for x=0,1

	if (mti >= N)	// generate N words at one time
	{
		for (kk = 0;kk < N-M;kk++)
		{
			y = (mt[kk] & UPPER_MASK)|(mt[kk+1] & LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		
		for (;kk<N-1;kk++)
		{
			y = (mt[kk] & UPPER_MASK)|(mt[kk+1] & LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		
		y = (mt[N-1] & UPPER_MASK)|(mt[0] & LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
		mti = 0;
	}
	
	y = mt[mti++];
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);
	
	return y;
}
