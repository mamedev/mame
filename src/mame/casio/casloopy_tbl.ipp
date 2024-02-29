// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/*****************************************************************************

    Casio Loopy video priority tables

    TODO: Replace with equations

 ******************************************************************************/

const casloopy_state::Layer casloopy_state::s_m4_pri_2[256][16][10] =
{
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, NG, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, NG, NG, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, NG, NG, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, NG, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, NG, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, NG, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, NG, NG },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, NG, NG },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, NG, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, NG, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, NG, NG }
	}
};

const casloopy_state::Layer casloopy_state::s_m4_pri_4[256][16][10] =
{
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, NG, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, NG, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, NG, NG, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, NG, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, NG, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, NG, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, NG, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, NG, NG },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, NG, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, NG, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, NG, NG }
	}
};

const casloopy_state::Layer casloopy_state::s_m4_pri_6[256][16][10] =
{
	{
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }
	},
	{
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG },
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }
	},
	{
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, S0, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, S0, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }
	},
	{
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S0, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S0, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, S0, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S0, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S0, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S0, X0, B2, B3, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, S0, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, S0, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG },
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }
	},
	{
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG },
		{ X1, Y1, X0, S0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X1, Y1, B0, B1, X0, Y0, S0, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, S0, NG, NG, NG },
		{ X1, Y1, X0, B0, B1, Y0, S0, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }
	},
	{
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ X1, B2, B3, Y1, X0, S0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }
	},
	{
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, X0, S0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG },
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ X1, Y1, B2, B3, X0, Y0, S0, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, S0, NG, NG, NG },
		{ X1, Y1, X0, B2, B3, Y0, S0, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ X1, B0, B1, Y1, X0, S0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG }
	},
	{
		{ X1, Y1, S0, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S0, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S0, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S0, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, B0, B1, S0, B2, B3, X0, Y0, NG },
		{ X1, Y1, B2, B3, S0, B0, B1, X0, Y0, NG },
		{ X1, Y1, X0, B0, B1, S0, B2, B3, Y0, NG },
		{ X1, Y1, X0, B2, B3, S0, B0, B1, Y0, NG },
		{ X1, Y1, B0, B1, B2, B3, S0, X0, Y0, NG },
		{ X1, Y1, B2, B3, B0, B1, S0, X0, Y0, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, S0, Y0, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, S0, Y0, NG },
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, S0, NG },
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, S0, NG },
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, S0, NG },
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, S0, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ X1, B0, B1, Y1, X0, S0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG },
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG },
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG }
	},
	{
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG },
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }
	},
	{
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ X1, B2, B3, Y1, X0, S0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG },
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG },
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG },
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG },
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG },
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }
	},
	{
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, S0, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, S0, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }
	},
	{
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S0, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S0, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, S0, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S0, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S0, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S0, X0, B2, B3, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, S0, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, S0, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, S0, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG },
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG },
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG },
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }
	},
	{
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }
	},
	{
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, S0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, S0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }
	},
	{
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, B0, B1, S0, B2, B3, X0, Y0 },
		{ S1, X1, Y1, B2, B3, S0, B0, B1, X0, Y0 },
		{ S1, X1, Y1, X0, B0, B1, S0, B2, B3, Y0 },
		{ S1, X1, Y1, X0, B2, B3, S0, B0, B1, Y0 },
		{ S1, X1, Y1, B0, B1, B2, B3, S0, X0, Y0 },
		{ S1, X1, Y1, B2, B3, B0, B1, S0, X0, Y0 },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, S0, Y0 },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, S0, Y0 },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, S0 },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, S0 },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, S0 },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }
	},
	{
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }
	},
	{
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }
	},
	{
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }
	},
	{
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG },
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG },
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG }
	},
	{
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }
	},
	{
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }
	},
	{
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }
	},
	{
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, S0, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, S0, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }
	},
	{
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ S0, X1, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ S0, X1, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ X1, Y1, S0, S1, B0, B1, B2, B3, X0, Y0 },
		{ X1, Y1, S0, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, Y1, S0, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, Y1, S0, S1, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, S0, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, S0, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG },
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG }
	},
	{
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, S0, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG },
		{ X1, Y1, S1, X0, S0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, S0, NG, NG },
		{ X1, Y1, S1, B0, B1, X0, Y0, S0, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, S0, NG, NG },
		{ X1, Y1, S1, X0, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG }
	},
	{
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, S0 },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, S0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, S0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, S0 }
	},
	{
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ B2, B3, X1, Y1, S1, S0, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, S0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, S0 },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, S0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, S0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, S0 }
	},
	{
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S1, S0, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, X0, S0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG },
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, S0, NG, NG },
		{ X1, Y1, S1, B2, B3, X0, Y0, S0, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, S0, NG, NG },
		{ X1, Y1, S1, X0, B2, B3, Y0, S0, NG, NG }
	},
	{
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S1, S0, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, S0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 }
	},
	{
		{ X1, Y1, S0, S1, B0, B1, B2, B3, X0, Y0 },
		{ X1, Y1, S0, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, Y1, S0, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, Y1, S0, S1, X0, B2, B3, B0, B1, Y0 },
		{ X1, Y1, S1, B0, B1, S0, B2, B3, X0, Y0 },
		{ X1, Y1, S1, B2, B3, S0, B0, B1, X0, Y0 },
		{ X1, Y1, S1, X0, B0, B1, S0, B2, B3, Y0 },
		{ X1, Y1, S1, X0, B2, B3, S0, B0, B1, Y0 },
		{ X1, Y1, S1, B0, B1, B2, B3, S0, X0, Y0 },
		{ X1, Y1, S1, B2, B3, B0, B1, S0, X0, Y0 },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, S0, Y0 },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, S0, Y0 },
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, S0 },
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, S0 },
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, S0 },
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, S0 }
	},
	{
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S1, S0, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, S0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 },
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 },
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 }
	},
	{
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG },
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, S0 },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, S0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, S0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, S0 }
	},
	{
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ B2, B3, X1, Y1, S1, S0, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, S0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 },
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 },
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 }
	},
	{
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, S0 },
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, S0 },
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, S0 },
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, S0 }
	},
	{
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG },
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B0, B1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, S0, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG },
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, S0, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }
	},
	{
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ S0, X1, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ S0, X1, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 },
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 },
		{ X1, Y1, S0, S1, B0, B1, B2, B3, X0, Y0 },
		{ X1, Y1, S0, S1, B2, B3, B0, B1, X0, Y0 },
		{ X1, Y1, S0, S1, X0, B0, B1, B2, B3, Y0 },
		{ X1, Y1, S0, S1, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, S0, B0, B1, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 },
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, S0, B2, B3, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG },
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, S0, B2, B3, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 },
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }
	},
	{
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 },
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 },
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 },
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 },
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 },
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 },
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 },
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 },
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 },
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 },
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }
	},
	{
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }
	},
	{
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }
	},
	{
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG },
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, S0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, S0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG },
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }
	},
	{
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, B0, B1, S0, B2, B3, X0, Y0 },
		{ S1, X1, Y1, B2, B3, S0, B0, B1, X0, Y0 },
		{ S1, X1, Y1, X0, B0, B1, S0, B2, B3, Y0 },
		{ S1, X1, Y1, X0, B2, B3, S0, B0, B1, Y0 },
		{ S1, X1, Y1, B0, B1, B2, B3, S0, X0, Y0 },
		{ S1, X1, Y1, B2, B3, B0, B1, S0, X0, Y0 },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, S0, Y0 },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, S0, Y0 },
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, S0 },
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, S0 },
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, S0 },
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 },
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG },
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 },
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 },
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 },
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 },
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }
	},
	{
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG },
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG },
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 },
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 },
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 },
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 },
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 },
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 },
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG },
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	},
	{
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 },
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 },
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 },
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 },
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 },
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 },
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 },
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 },
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 },
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 },
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 },
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }
	}
};

const casloopy_state::Layer casloopy_state::s_m5_pri_6[256][16][10] =
{
	{
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, NG, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ X0, Y0, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ X0, Y0, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, Y1, NG, NG },
		{ X0, Y0, B0, B1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, B2, B3, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, Y1, NG, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, Y1, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, NG, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, NG, NG }
	},
	{
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, S0, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, S0, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, S0, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, S0, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, S0, NG, NG, NG, NG, NG },
		{ X0, Y0, X1, Y1, S0, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, S0, Y1, NG, NG, NG },
		{ X0, Y0, X1, S0, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, B0, B1, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, S0, Y1, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, S0, Y1, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X0, Y0, B0, B1, X1, Y1, S0, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, S0, NG, NG, NG },
		{ X0, Y0, X1, B0, B1, Y1, S0, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, S0, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, S0, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ B0, B1, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B0, B1, Y0, X1, Y1, S0, NG, NG, NG }
	},
	{
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, X1, S0, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, S0, Y1, NG, NG, NG },
		{ X0, Y0, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, B2, B3, S0, X1, Y1, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, S0, Y1, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, S0, Y1, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ X0, Y0, B2, B3, X1, Y1, S0, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, S0, NG, NG, NG },
		{ X0, Y0, X1, B2, B3, Y1, S0, NG, NG, NG }
	},
	{
		{ X0, Y0, S0, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S0, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, B0, B1, S0, B2, B3, X1, Y1, NG },
		{ X0, Y0, B2, B3, S0, B0, B1, X1, Y1, NG },
		{ X0, Y0, X1, B0, B1, S0, B2, B3, Y1, NG },
		{ X0, Y0, X1, B2, B3, S0, B0, B1, Y1, NG },
		{ X0, Y0, B0, B1, B2, B3, S0, X1, Y1, NG },
		{ X0, Y0, B2, B3, B0, B1, S0, X1, Y1, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, S0, Y1, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, S0, Y1, NG },
		{ X0, Y0, B0, B1, B2, B3, X1, Y1, S0, NG },
		{ X0, Y0, B2, B3, B0, B1, X1, Y1, S0, NG },
		{ X0, Y0, X1, B0, B1, B2, B3, Y1, S0, NG },
		{ X0, Y0, X1, B2, B3, B0, B1, Y1, S0, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ X0, B0, B1, Y0, X1, S0, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, S0, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, S0, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, S0, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, S0, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ X0, B0, B1, Y0, X1, S0, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, S0, X1, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, S0, Y1, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, S0, NG },
		{ B0, B1, X0, Y0, B2, B3, X1, Y1, S0, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, S0, NG },
		{ X0, B0, B1, Y0, X1, B2, B3, Y1, S0, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, S0, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ X0, B2, B3, Y0, X1, S0, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, S0, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, S0, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, S0, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, S0, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, S0, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, S0, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, S0, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, S0, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, S0, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, S0, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, S0, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ B2, B3, X0, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, S0, NG, NG, NG },
		{ X0, B2, B3, Y0, X1, Y1, S0, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ X0, B2, B3, Y0, X1, S0, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, S0, X1, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, S0, Y1, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, S0, NG },
		{ B2, B3, X0, Y0, B0, B1, X1, Y1, S0, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, S0, NG },
		{ X0, B2, B3, Y0, X1, B0, B1, Y1, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, S0, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, S0, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, S0, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, S0, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, S0, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, S0, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, X1, Y1, S0, NG },
		{ B2, B3, B0, B1, X0, Y0, X1, Y1, S0, NG },
		{ X0, B0, B1, B2, B3, Y0, X1, Y1, S0, NG },
		{ X0, B2, B3, B0, B1, Y0, X1, Y1, S0, NG }
	},
	{
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S0, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S0, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, S0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, S0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, S0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, S0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, S0, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S0, X1, B2, B3, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, S0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, S0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, S0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, S0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S0, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, S0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S0, X1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, S0, Y0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S0, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, S0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S0, X1, B2, B3, Y1, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S0, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S0, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, S0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, S0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S0, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S0, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, S0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, S0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, S0, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S0, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S0, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S0, X1, B2, B3, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, S0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S0, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ S0, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, S0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ B0, B1, S0, X0, Y0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, S0, Y0, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S0, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S0, X1, B2, B3, Y1, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, S0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S0, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, S0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, S0, X0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, S0, Y0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S0, X1, Y1, NG, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S0, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, S0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ B2, B3, S0, X0, Y0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, S0, Y0, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S0, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S0, X1, B0, B1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S0, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S0, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S0, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, S0, B2, B3, X0, Y0, X1, Y1, NG },
		{ B2, B3, S0, B0, B1, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, S0, B2, B3, Y0, X1, Y1, NG },
		{ X0, B2, B3, S0, B0, B1, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, S0, X0, Y0, X1, Y1, NG },
		{ B2, B3, B0, B1, S0, X0, Y0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, S0, Y0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, S0, Y0, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S0, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S0, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S0, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S0, X1, Y1, NG }
	},
	{
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, NG, NG, NG }
	},
	{
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, NG, NG, NG }
	},
	{
		{ X0, Y0, S1, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S1, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, S1, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S1, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, S1, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S1, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, Y1, NG },
		{ X0, Y0, S1, B0, B1, B2, B3, X1, Y1, NG },
		{ X0, Y0, S1, B2, B3, B0, B1, X1, Y1, NG },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, Y1, NG },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, Y1, NG }
	},
	{
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG }
	},
	{
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, NG }
	},
	{
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG }
	},
	{
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, NG, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, NG },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, NG },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, NG },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, NG }
	},
	{
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, S0, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, S0, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, S0, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, S0, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, S0, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, S0, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, S0, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, S0, Y1, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, S0, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, S0, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, S0, NG, NG, NG, NG },
		{ X0, Y0, S1, X1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S1, B0, B1, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, S0, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, S0, Y1, NG, NG },
		{ X0, Y0, S1, X1, S0, B0, B1, Y1, NG, NG },
		{ X0, Y0, S1, B0, B1, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, B0, B1, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, S0, Y1, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, S0, Y1, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, S0, NG, NG },
		{ X0, Y0, S1, B0, B1, X1, Y1, S0, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, S0, NG, NG },
		{ X0, Y0, S1, X1, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, S0, NG, NG }
	},
	{
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, S0, Y1, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ B0, B1, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B0, B1, Y0, S1, X1, Y1, S0, NG, NG }
	},
	{
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S1, S0, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S1, B2, B3, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, X1, S0, B2, B3, Y1, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, S0, Y1, NG, NG },
		{ X0, Y0, S1, B2, B3, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, B2, B3, S0, X1, Y1, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, S0, Y1, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, S0, Y1, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, S0, NG, NG },
		{ X0, Y0, S1, B2, B3, X1, Y1, S0, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, S0, NG, NG },
		{ X0, Y0, S1, X1, B2, B3, Y1, S0, NG, NG }
	},
	{
		{ X0, Y0, S0, S1, B0, B1, B2, B3, X1, Y1 },
		{ X0, Y0, S0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, Y0, S0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, Y0, S0, S1, X1, B2, B3, B0, B1, Y1 },
		{ X0, Y0, S1, B0, B1, S0, B2, B3, X1, Y1 },
		{ X0, Y0, S1, B2, B3, S0, B0, B1, X1, Y1 },
		{ X0, Y0, S1, X1, B0, B1, S0, B2, B3, Y1 },
		{ X0, Y0, S1, X1, B2, B3, S0, B0, B1, Y1 },
		{ X0, Y0, S1, B0, B1, B2, B3, S0, X1, Y1 },
		{ X0, Y0, S1, B2, B3, B0, B1, S0, X1, Y1 },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, S0, Y1 },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, S0, Y1 },
		{ X0, Y0, S1, B0, B1, B2, B3, X1, Y1, S0 },
		{ X0, Y0, S1, B2, B3, B0, B1, X1, Y1, S0 },
		{ X0, Y0, S1, X1, B0, B1, B2, B3, Y1, S0 },
		{ X0, Y0, S1, X1, B2, B3, B0, B1, Y1, S0 }
	},
	{
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S1, S0, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ X0, B0, B1, Y0, S1, X1, S0, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, S0 },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, S0 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, S0 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, S0 }
	},
	{
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S1, S0, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ X0, B0, B1, Y0, S1, X1, S0, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, S0, X1, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, S0, Y1 },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, S0 },
		{ B0, B1, X0, Y0, S1, B2, B3, X1, Y1, S0 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, S0 },
		{ X0, B0, B1, Y0, S1, X1, B2, B3, Y1, S0 }
	},
	{
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, S0, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ B2, B3, X0, Y0, S1, S0, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ X0, B2, B3, Y0, S1, X1, S0, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, S0 },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, S0 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, S0 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, S0 }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, S0 },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, S0 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, S0 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, S0 }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, S0 },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, S0 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, S0 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, S0 }
	},
	{
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, S0, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, S0, Y1, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ B2, B3, X0, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, S0, NG, NG },
		{ X0, B2, B3, Y0, S1, X1, Y1, S0, NG, NG }
	},
	{
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ B2, B3, X0, Y0, S1, S0, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ X0, B2, B3, Y0, S1, X1, S0, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, S0, X1, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, S0, Y1 },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, S0 },
		{ B2, B3, X0, Y0, S1, B0, B1, X1, Y1, S0 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, S0 },
		{ X0, B2, B3, Y0, S1, X1, B0, B1, Y1, S0 }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, S0 },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, S0 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, S0 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, S0 }
	},
	{
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, S0, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S1, S0, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, S0, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, S0, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S1, X1, Y1, S0 },
		{ B2, B3, B0, B1, X0, Y0, S1, X1, Y1, S0 },
		{ X0, B0, B1, B2, B3, Y0, S1, X1, Y1, S0 },
		{ X0, B2, B3, B0, B1, Y0, S1, X1, Y1, S0 }
	},
	{
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ S0, X0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ S0, X0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, S0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, S0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, S0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, S0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ X0, Y0, S0, S1, B0, B1, B2, B3, X1, Y1 },
		{ X0, Y0, S0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, Y0, S0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, Y0, S0, S1, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, S0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, S0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, S0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, S0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ S0, X0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, S0, Y0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG },
		{ X0, Y0, S0, S1, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B0, B1, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B0, B1, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B0, B1, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, S0, Y0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B0, B1, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B0, B1, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ S0, X0, Y0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, S0, Y0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, B2, B3, X1, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG },
		{ X0, Y0, S0, S1, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ S0, X0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ S0, X0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, S0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, S0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ S0, X0, Y0, S1, B0, B1, B2, B3, X1, Y1 },
		{ S0, X0, Y0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, S0, Y0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, S0, Y0, S1, X1, B2, B3, B0, B1, Y1 },
		{ X0, Y0, S0, S1, B0, B1, B2, B3, X1, Y1 },
		{ X0, Y0, S0, S1, B2, B3, B0, B1, X1, Y1 },
		{ X0, Y0, S0, S1, X1, B0, B1, B2, B3, Y1 },
		{ X0, Y0, S0, S1, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, S0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 }
	},
	{
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ S0, X0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ S0, B0, B1, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, S0, B0, B1, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ B0, B1, S0, X0, Y0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, S0, Y0, S1, X1, B2, B3, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ B0, B1, X0, Y0, S0, S1, B2, B3, X1, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 },
		{ X0, B0, B1, Y0, S0, S1, X1, B2, B3, Y1 }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, S0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, X0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ S0, B2, B3, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, S0, B2, B3, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, S0, X0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, S0, Y0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ B2, B3, X0, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG },
		{ X0, B2, B3, Y0, S0, S1, X1, Y1, NG, NG }
	},
	{
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, X0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ S0, B2, B3, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, S0, B2, B3, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ B2, B3, S0, X0, Y0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, S0, Y0, S1, X1, B0, B1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ B2, B3, X0, Y0, S0, S1, B0, B1, X1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 },
		{ X0, B2, B3, Y0, S0, S1, X1, B0, B1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S0, B0, B1, B2, B3, X0, Y0, S1, X1, Y1 },
		{ S0, B2, B3, B0, B1, X0, Y0, S1, X1, Y1 },
		{ S0, X0, B0, B1, B2, B3, Y0, S1, X1, Y1 },
		{ S0, X0, B2, B3, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, S0, B2, B3, X0, Y0, S1, X1, Y1 },
		{ B2, B3, S0, B0, B1, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, S0, B2, B3, Y0, S1, X1, Y1 },
		{ X0, B2, B3, S0, B0, B1, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, S0, X0, Y0, S1, X1, Y1 },
		{ B2, B3, B0, B1, S0, X0, Y0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, S0, Y0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, S0, Y0, S1, X1, Y1 },
		{ B0, B1, B2, B3, X0, Y0, S0, S1, X1, Y1 },
		{ B2, B3, B0, B1, X0, Y0, S0, S1, X1, Y1 },
		{ X0, B0, B1, B2, B3, Y0, S0, S1, X1, Y1 },
		{ X0, B2, B3, B0, B1, Y0, S0, S1, X1, Y1 }
	},
	{
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, S0, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, S0, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, S0, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, B0, B1, S0, B2, B3, X1, Y1 },
		{ S1, X0, Y0, B2, B3, S0, B0, B1, X1, Y1 },
		{ S1, X0, Y0, X1, B0, B1, S0, B2, B3, Y1 },
		{ S1, X0, Y0, X1, B2, B3, S0, B0, B1, Y1 },
		{ S1, X0, Y0, B0, B1, B2, B3, S0, X1, Y1 },
		{ S1, X0, Y0, B2, B3, B0, B1, S0, X1, Y1 },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, S0, Y1 },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, S0, Y1 },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, S0 },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, S0 },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, S0 },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, S0, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, S0, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, S0, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, S0, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S0, S1, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S0, S1, X0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S0, S1, X0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S0, S1, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S0, S1, X0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S0, S1, X0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, NG, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, NG },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, NG },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, NG },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, NG, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, NG },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, NG },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, NG },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, NG }
	},
	{
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, S0, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG },
		{ S1, X0, Y0, X1, Y1, S0, NG, NG, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, S0, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, S0, Y1, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, B0, B1, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B0, B1, Y1, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, S0, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B0, B1, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B0, B1, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, S0, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, S0, X1, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, S0, Y1, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, B2, B3, X1, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, S0, NG, NG },
		{ S1, X0, Y0, X1, B2, B3, Y1, S0, NG, NG }
	},
	{
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, B0, B1, S0, B2, B3, X1, Y1 },
		{ S1, X0, Y0, B2, B3, S0, B0, B1, X1, Y1 },
		{ S1, X0, Y0, X1, B0, B1, S0, B2, B3, Y1 },
		{ S1, X0, Y0, X1, B2, B3, S0, B0, B1, Y1 },
		{ S1, X0, Y0, B0, B1, B2, B3, S0, X1, Y1 },
		{ S1, X0, Y0, B2, B3, B0, B1, S0, X1, Y1 },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, S0, Y1 },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, S0, Y1 },
		{ S1, X0, Y0, B0, B1, B2, B3, X1, Y1, S0 },
		{ S1, X0, Y0, B2, B3, B0, B1, X1, Y1, S0 },
		{ S1, X0, Y0, X1, B0, B1, B2, B3, Y1, S0 },
		{ S1, X0, Y0, X1, B2, B3, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, S0, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 }
	},
	{
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, S0, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, S0, X1, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, S0, Y1 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, B0, B1, X0, Y0, B2, B3, X1, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 },
		{ S1, X0, B0, B1, Y0, X1, B2, B3, Y1, S0 }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, S0, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, S0, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, B2, B3, X0, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG },
		{ S1, X0, B2, B3, Y0, X1, Y1, S0, NG, NG }
	},
	{
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, S0, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, S0, X1, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, S0, Y1 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, B2, B3, X0, Y0, B0, B1, X1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 },
		{ S1, X0, B2, B3, Y0, X1, B0, B1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, S0, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, S0, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, X1, Y1, S0 },
		{ S1, B2, B3, B0, B1, X0, Y0, X1, Y1, S0 },
		{ S1, X0, B0, B1, B2, B3, Y0, X1, Y1, S0 },
		{ S1, X0, B2, B3, B0, B1, Y0, X1, Y1, S0 }
	},
	{
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S0, S1, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S0, S1, X0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S0, S1, X0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S0, S1, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, S0, X0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, S0, Y0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG },
		{ S1, X0, Y0, S0, X1, Y1, NG, NG, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B0, B1, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B0, B1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, S0, B0, B1, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B0, B1, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, S0, Y0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B0, B1, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B0, B1, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S0, S1, X0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, S0, X0, Y0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, S0, Y0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, B2, B3, X1, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG },
		{ S1, X0, Y0, S0, X1, B2, B3, Y1, NG, NG }
	},
	{
		{ S0, S1, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S0, S1, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S0, S1, X0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S0, S1, X0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, S0, X0, Y0, B0, B1, B2, B3, X1, Y1 },
		{ S1, S0, X0, Y0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, S0, Y0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, S0, Y0, X1, B2, B3, B0, B1, Y1 },
		{ S1, X0, Y0, S0, B0, B1, B2, B3, X1, Y1 },
		{ S1, X0, Y0, S0, B2, B3, B0, B1, X1, Y1 },
		{ S1, X0, Y0, S0, X1, B0, B1, B2, B3, Y1 },
		{ S1, X0, Y0, S0, X1, B2, B3, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S0, S1, X0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, S0, B0, B1, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, S0, B0, B1, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, B0, B1, S0, X0, Y0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, S0, Y0, X1, B2, B3, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, B0, B1, X0, Y0, S0, B2, B3, X1, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 },
		{ S1, X0, B0, B1, Y0, S0, X1, B2, B3, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S0, S1, X0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, S0, B2, B3, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, S0, B2, B3, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, S0, X0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, S0, Y0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, B2, B3, X0, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG },
		{ S1, X0, B2, B3, Y0, S0, X1, Y1, NG, NG }
	},
	{
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S0, S1, X0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, S0, B2, B3, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, S0, B2, B3, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, B2, B3, S0, X0, Y0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, S0, Y0, X1, B0, B1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, B2, B3, X0, Y0, S0, B0, B1, X1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 },
		{ S1, X0, B2, B3, Y0, S0, X1, B0, B1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	},
	{
		{ S0, S1, B0, B1, B2, B3, X0, Y0, X1, Y1 },
		{ S0, S1, B2, B3, B0, B1, X0, Y0, X1, Y1 },
		{ S0, S1, X0, B0, B1, B2, B3, Y0, X1, Y1 },
		{ S0, S1, X0, B2, B3, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, S0, B2, B3, X0, Y0, X1, Y1 },
		{ S1, B2, B3, S0, B0, B1, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, S0, B2, B3, Y0, X1, Y1 },
		{ S1, X0, B2, B3, S0, B0, B1, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, S0, X0, Y0, X1, Y1 },
		{ S1, B2, B3, B0, B1, S0, X0, Y0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, S0, Y0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, S0, Y0, X1, Y1 },
		{ S1, B0, B1, B2, B3, X0, Y0, S0, X1, Y1 },
		{ S1, B2, B3, B0, B1, X0, Y0, S0, X1, Y1 },
		{ S1, X0, B0, B1, B2, B3, Y0, S0, X1, Y1 },
		{ S1, X0, B2, B3, B0, B1, Y0, S0, X1, Y1 }
	}
};

const casloopy_state::Layer casloopy_state::s_m0_pri_6[256][16][10] =
{
	{ // 0 - 0 0 0 0
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 0
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 1
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 2
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 3
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 4
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 5
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 6
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 7
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 8
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 9
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 10
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 11
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 12
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 13
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }, // 14
		{ X1, Y1, X0, Y0, NG, NG, NG, NG, NG, NG }  // 15
	},
	{ // 1 - 0 0 0 1
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 6
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 7
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 10
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }, // 14
		{ X1, B0, B1, Y1, X0, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 2 - 0 0 0 2
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 0
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 2
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 3
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 4
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 6
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 7
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 8
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 10
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 11
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 12
		{ X1, Y1, B0, B1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }, // 14
		{ X1, Y1, X0, B0, B1, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 3 - 0 0 0 3
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 2
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 3
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 6
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 7
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }, // 14
		{ X1, X0, B0, B1, Y1, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 4 - 0 0 1 0
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 6
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 7
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 10
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }, // 14
		{ X1, B2, B3, Y1, X0, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 5 - 0 0 1 1
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG }, // 2
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG }, // 6
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 9
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG }, // 10
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 13
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, NG, NG }, // 14
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, NG, NG }  // 15
	},
	{ // 6 - 0 0 1 2
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 4
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 6
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 7
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 8
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 9
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 10
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 11
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 12
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 13
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }, // 14
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, NG, NG }  // 15
	},
	{ // 7 - 0 0 1 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 2
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 6
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }  // 15
	},
	{ // 8 - 0 0 2 0
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 0
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 2
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 3
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 4
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 6
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 7
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 8
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 10
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 11
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 12
		{ X1, Y1, B2, B3, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }, // 14
		{ X1, Y1, X0, B2, B3, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 9 - 0 0 2 1
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 5
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 6
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 7
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 8
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 9
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 10
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 11
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 12
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 13
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }, // 14
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, NG, NG }  // 15
	},
	{ // 10 - 0 0 2 2
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG }, // 0
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG }, // 1
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG }, // 2
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG }, // 3
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG }, // 4
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG }, // 5
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG }, // 6
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG }, // 7
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG }, // 8
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG }, // 9
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG }, // 10
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG }, // 11
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, NG, NG }, // 12
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, NG, NG }, // 13
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, NG, NG }, // 14
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, NG, NG }  // 15
	},
	{ // 11 - 0 0 2 3
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, NG, NG }, // 2
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 3
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, NG, NG }, // 6
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 7
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 9
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, NG, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 11
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, NG, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }  // 15
	},
	{ // 12 - 0 0 3 0
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 2
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 3
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 7
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }, // 14
		{ X1, X0, B2, B3, Y1, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 13 - 0 0 3 1
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 2
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 6
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }  // 15
	},
	{ // 14 - 0 0 3 2
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 2
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 6
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 8
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 12
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }  // 15
	},
	{ // 15 - 0 0 3 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 2
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 6
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, NG, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, NG, NG }  // 15
	},
	{ // 16 - 0 1 0 0
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 6
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 7
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 10
		{ X1, S0, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 11
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 12
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 13
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 14
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 17 - 0 1 0 1
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG }, // 6
		{ X1, S0, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG }, // 10
		{ X1, B0, B1, S0, Y1, X0, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 13
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }, // 14
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 18 - 0 1 0 2
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 4
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 6
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 7
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 8
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 9
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 10
		{ X1, S0, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 11
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG }, // 12
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG }, // 13
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }, // 14
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }  // 15
	},
	{ // 19 - 0 1 0 3
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, S0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, S0, Y0, NG, NG, NG }, // 14
		{ X1, X0, B0, B1, Y1, S0, Y0, NG, NG, NG }  // 15
	},
	{ // 20 - 0 1 1 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, S0, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 6
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG }, // 10
		{ X1, B2, B3, S0, Y1, X0, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 13
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }, // 14
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 21 - 0 1 1 1
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 2
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, B0, B1, S0, B2, B3, Y1, X0, Y0, NG }, // 6
		{ X1, B2, B3, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, B0, B1, B2, B3, S0, Y1, X0, Y0, NG }, // 10
		{ X1, B2, B3, B0, B1, S0, Y1, X0, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 13
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG }, // 14
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }  // 15
	},
	{ // 22 - 0 1 1 2
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 0
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 2
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 3
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 4
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ X1, S0, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 6
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG }, // 7
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 8
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG }, // 10
		{ X1, B2, B3, S0, Y1, X0, B0, B1, Y0, NG }, // 11
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG }, // 12
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG }, // 13
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }, // 14
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }  // 15
	},
	{ // 23 - 0 1 1 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }  // 15
	},
	{ // 24 - 0 1 2 0
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 5
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 6
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 7
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 8
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 9
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 10
		{ X1, S0, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 11
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG }, // 12
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG }, // 13
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }, // 14
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }  // 15
	},
	{ // 25 - 0 1 2 1
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 1
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 2
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 3
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 5
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG }, // 6
		{ X1, S0, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 7
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 9
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG }, // 10
		{ X1, B0, B1, S0, Y1, X0, B2, B3, Y0, NG }, // 11
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 12
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 13
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }, // 14
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }  // 15
	},
	{ // 26 - 0 1 2 2
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ S0, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 4
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 5
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 6
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 7
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 8
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 9
		{ X1, S0, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 10
		{ X1, S0, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 11
		{ X1, Y1, S0, B0, B1, B2, B3, X0, Y0, NG }, // 12
		{ X1, Y1, S0, B2, B3, B0, B1, X0, Y0, NG }, // 13
		{ X1, Y1, S0, X0, B0, B1, B2, B3, Y0, NG }, // 14
		{ X1, Y1, S0, X0, B2, B3, B0, B1, Y0, NG }  // 15
	},
	{ // 27 - 0 1 2 3
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, B2, B3, Y0, NG }, // 6
		{ X1, X0, S0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, S0, Y1, B2, B3, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 13
		{ X1, X0, B0, B1, Y1, S0, B2, B3, Y0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }  // 15
	},
	{ // 28 - 0 1 3 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, S0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 13
		{ X1, X0, B2, B3, Y1, S0, Y0, NG, NG, NG }, // 14
		{ X1, X0, B2, B3, Y1, S0, Y0, NG, NG, NG }  // 15
	},
	{ // 29 - 0 1 3 1
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }  // 15
	},
	{ // 30 - 0 1 3 2
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ X1, X0, S0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, Y1, B0, B1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, S0, Y1, B0, B1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, X0, Y1, S0, Y0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 14
		{ X1, X0, B2, B3, Y1, S0, B0, B1, Y0, NG }  // 15
	},
	{ // 31 - 0 1 3 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }  // 15
	},
	{ // 32 - 0 2 0 0
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG }, // 6
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG }, // 7
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ X1, Y1, S0, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG }, // 10
		{ X1, Y1, X0, S0, Y0, NG, NG, NG, NG, NG }, // 11
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 12
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 13
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 14
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 33 - 0 2 0 1
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 0
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 1
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }, // 2
		{ X1, B0, B1, Y1, S0, X0, Y0, NG, NG, NG }, // 3
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 4
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 5
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG }, // 6
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG }, // 7
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG }, // 10
		{ X1, B0, B1, Y1, X0, S0, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }, // 14
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 34 - 0 2 0 2
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ X1, Y1, S0, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG }, // 4
		{ X1, Y1, S0, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG }, // 6
		{ X1, Y1, X0, S0, B0, B1, Y0, NG, NG, NG }, // 7
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG }, // 8
		{ X1, Y1, B0, B1, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG }, // 10
		{ X1, Y1, X0, B0, B1, S0, Y0, NG, NG, NG }, // 11
		{ X1, Y1, B0, B1, X0, Y0, S0, NG, NG, NG }, // 12
		{ X1, Y1, B0, B1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, Y1, X0, B0, B1, Y0, S0, NG, NG, NG }, // 14
		{ X1, Y1, X0, B0, B1, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 35 - 0 2 0 3
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, Y1, S0, Y0, NG, NG, NG }, // 6
		{ X1, X0, S0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, Y1, S0, Y0, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, Y1, S0, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }, // 14
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 36 - 0 2 1 0
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 0
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 1
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }, // 2
		{ X1, B2, B3, Y1, S0, X0, Y0, NG, NG, NG }, // 3
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 5
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG }, // 6
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG }, // 7
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG }, // 10
		{ X1, B2, B3, Y1, X0, S0, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }, // 14
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 37 - 0 2 1 1
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 1
		{ X1, B0, B1, B2, B3, Y1, S0, X0, Y0, NG }, // 2
		{ X1, B2, B3, B0, B1, Y1, S0, X0, Y0, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 5
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG }, // 6
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 9
		{ X1, B0, B1, B2, B3, Y1, X0, S0, Y0, NG }, // 10
		{ X1, B2, B3, B0, B1, Y1, X0, S0, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG }, // 14
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }  // 15
	},
	{ // 38 - 0 2 1 2
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG }, // 0
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG }, // 1
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }, // 2
		{ X1, B2, B3, Y1, S0, X0, B0, B1, Y0, NG }, // 3
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG }, // 4
		{ B2, B3, X1, Y1, S0, B0, B1, X0, Y0, NG }, // 5
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG }, // 6
		{ X1, B2, B3, Y1, X0, S0, B0, B1, Y0, NG }, // 7
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG }, // 8
		{ B2, B3, X1, Y1, B0, B1, S0, X0, Y0, NG }, // 9
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG }, // 10
		{ X1, B2, B3, Y1, X0, B0, B1, S0, Y0, NG }, // 11
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG }, // 12
		{ B2, B3, X1, Y1, B0, B1, X0, Y0, S0, NG }, // 13
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG }, // 14
		{ X1, B2, B3, Y1, X0, B0, B1, Y0, S0, NG }  // 15
	},
	{ // 39 - 0 2 1 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 4
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 6
		{ X1, X0, S0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, Y1, S0, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 40 - 0 2 2 0
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ X1, Y1, S0, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ X1, Y1, S0, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG }, // 5
		{ X1, Y1, X0, S0, B2, B3, Y0, NG, NG, NG }, // 6
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG }, // 7
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG }, // 8
		{ X1, Y1, B2, B3, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG }, // 10
		{ X1, Y1, X0, B2, B3, S0, Y0, NG, NG, NG }, // 11
		{ X1, Y1, B2, B3, X0, Y0, S0, NG, NG, NG }, // 12
		{ X1, Y1, B2, B3, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, Y1, X0, B2, B3, Y0, S0, NG, NG, NG }, // 14
		{ X1, Y1, X0, B2, B3, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 41 - 0 2 2 1
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 0
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 1
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }, // 2
		{ X1, B0, B1, Y1, S0, X0, B2, B3, Y0, NG }, // 3
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 4
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG }, // 5
		{ X1, B0, B1, Y1, X0, S0, B2, B3, Y0, NG }, // 6
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG }, // 7
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG }, // 8
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG }, // 9
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG }, // 10
		{ X1, B0, B1, Y1, X0, B2, B3, S0, Y0, NG }, // 11
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG }, // 12
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG }, // 13
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG }, // 14
		{ X1, B0, B1, Y1, X0, B2, B3, Y0, S0, NG }  // 15
	},
	{ // 42 - 0 2 2 2
		{ X1, Y1, S0, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ X1, Y1, S0, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ X1, Y1, S0, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ X1, Y1, S0, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ X1, Y1, B0, B1, S0, B2, B3, X0, Y0, NG }, // 4
		{ X1, Y1, B2, B3, S0, B0, B1, X0, Y0, NG }, // 5
		{ X1, Y1, X0, B0, B1, S0, B2, B3, Y0, NG }, // 6
		{ X1, Y1, X0, B2, B3, S0, B0, B1, Y0, NG }, // 7
		{ X1, Y1, B0, B1, B2, B3, S0, X0, Y0, NG }, // 8
		{ X1, Y1, B2, B3, B0, B1, S0, X0, Y0, NG }, // 9
		{ X1, Y1, X0, B0, B1, B2, B3, S0, Y0, NG }, // 10
		{ X1, Y1, X0, B2, B3, B0, B1, S0, Y0, NG }, // 11
		{ X1, Y1, B0, B1, B2, B3, X0, Y0, S0, NG }, // 12
		{ X1, Y1, B2, B3, B0, B1, X0, Y0, S0, NG }, // 13
		{ X1, Y1, X0, B0, B1, B2, B3, Y0, S0, NG }, // 14
		{ X1, Y1, X0, B2, B3, B0, B1, Y0, S0, NG }  // 15
	},
	{ // 43 - 0 2 2 3
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, X1, Y1, S0, B2, B3, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, Y1, S0, B2, B3, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, X1, Y1, B2, B3, S0, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, Y1, B2, B3, S0, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }, // 11
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 44 - 0 2 3 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, S0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, Y1, S0, Y0, NG, NG, NG }, // 7
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, X1, Y1, S0, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, Y1, S0, Y0, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, Y1, S0, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }, // 14
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 45 - 0 2 3 1
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, S0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, S0, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, S0, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, Y1, S0, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 46 - 0 2 3 2
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 47 - 0 2 3 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 48 - 0 3 0 0
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ X1, X0, S0, Y1, Y0, NG, NG, NG, NG, NG }, // 6
		{ X1, X0, S0, Y1, Y0, NG, NG, NG, NG, NG }, // 7
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ S0, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ X1, X0, S0, Y1, Y0, NG, NG, NG, NG, NG }, // 10
		{ X1, X0, S0, Y1, Y0, NG, NG, NG, NG, NG }, // 11
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 12
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 13
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }, // 14
		{ X1, Y1, X0, Y0, S0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 49 - 0 3 0 1
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S0, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, S0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }, // 14
		{ X1, B0, B1, Y1, X0, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 50 - 0 3 0 2
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S0, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, S0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }, // 14
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 51 - 0 3 0 3
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, S0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B0, B1, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }, // 14
		{ X1, X0, B0, B1, Y1, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 52 - 0 3 1 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S0, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, S0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12 -
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }, // 14
		{ X1, B2, B3, Y1, X0, Y0, S0, NG, NG, NG }  // 15
	},
	{ // 53 - 0 3 1 1
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 2
		{ S0, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, B0, B1, B2, B3, Y1, X0, Y0, S0, NG }, // 14
		{ X1, B2, B3, B0, B1, Y1, X0, Y0, S0, NG }  // 15
	},
	{ // 54 - 0 3 1 2
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 0
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 2
		{ S0, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 55 - 0 3 1 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 56 - 0 3 2 0
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ S0, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ S0, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, S0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12 -
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13 -
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }, // 14 -
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }  // 15 -
	},
	{ // 57 - 0 3 2 1
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 1
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 2
		{ S0, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 3
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, B2, B3, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 58 - 0 3 2 2
		{ S0, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ S0, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ S0, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 59 - 0 3 2 3
		{ S0, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, Y1, B2, B3, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, X0, Y1, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, X1, Y1, B2, B3, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, Y1, B2, B3, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 60 - 0 3 3 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S0, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ X1, X0, S0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 10
		{ X1, X0, B2, B3, S0, Y1, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 12 -
		{ B2, B3, X1, Y1, X0, Y0, S0, NG, NG, NG }, // 13 -
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }, // 14 -
		{ X1, X0, B2, B3, Y1, Y0, S0, NG, NG, NG }  // 15 -
	},
	{ // 61 - 0 3 3 1
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 62 - 0 3 3 2
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 63 - 0 3 3 3
		{ S0, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S0, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S0, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ X1, X0, B0, B1, S0, B2, B3, Y1, Y0, NG }, // 6
		{ X1, X0, B2, B3, S0, B0, B1, Y1, Y0, NG }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, X0, Y0, NG }, // 9
		{ X1, X0, B0, B1, B2, B3, S0, Y1, Y0, NG }, // 10
		{ X1, X0, B2, B3, B0, B1, S0, Y1, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, X0, Y0, S0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, X0, Y0, S0, NG }, // 13
		{ X1, X0, B0, B1, B2, B3, Y1, Y0, S0, NG }, // 14
		{ X1, X0, B2, B3, B0, B1, Y1, Y0, S0, NG }  // 15
	},
	{ // 64 - 1 0 0 0
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 7
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 65 - 1 0 0 1
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 66 - 1 0 0 2
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 4
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 7
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 8
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 11
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 12
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }  // 15
	},
	{ // 67 - 1 0 0 3 (3 0 0 3)
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 6
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 15
	},
	{ // 68 - 1 0 1 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 69 - 1 0 1 1
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 2
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 6
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 10
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }  // 15
	},
	{ // 70 - 1 0 1 2
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 0
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 2
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 3
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 4
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 7
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 11
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }  // 15
	},
	{ // 71 - 1 0 1 3 (3 0 1 3)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 72 - 1 0 2 0
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 7
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 8
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }  // 15
	},
	{ // 73 - 1 0 2 1
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 1
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 2
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 12
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }  // 15
	},
	{ // 74 - 1 0 2 2
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 4
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 7
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 8
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 11
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 12
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }  // 15
	},
	{ // 75 - 1 0 2 3 (3 0 2 3)
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 76 - 1 0 3 0 (3 0 3 0)
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 15
	},
	{ // 77 - 1 0 3 1 (3 0 3 1)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 78 - 1 0 3 2 (3 0 3 2)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 15
	},
	{ // 79 - 1 0 3 3 (3 0 3 3)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 80 - 1 1 0 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 7
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 81 - 1 1 0 1
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 6
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 10
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }  // 15
	},
	{ // 82 - 1 1 0 2
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 7
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 8
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 11
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 12
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }  // 15
	},
	{ // 83 - 1 1 0 3 (3 1 0 3)
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }  // 15
	},
	{ // 84 - 1 1 1 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG }, // 6
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 10
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }  // 15
	},
	{ // 85 - 1 1 1 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 }, // 2
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 }, // 6
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 }, // 10
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }  // 15
	},
	{ // 86 - 1 1 1 2
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 3
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 5
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 }, // 6
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 7
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 8
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 9
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 10
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 11
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 12
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 13
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }, // 14
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }  // 15
	},
	{ // 87 - 1 1 1 3 (3 1 1 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 88 - 1 1 2 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 7
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 8
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 11
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 12
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }  // 15
	},
	{ // 89 - 1 1 2 1
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 5
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 6
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 }, // 7
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 8
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 9
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 10
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 12
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 13
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }, // 14
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }  // 15
	},
	{ // 90 - 1 1 2 2
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 4
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 5
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 }, // 6
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 }, // 7
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 8
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 9
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 }, // 10
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 }, // 11
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 }, // 12
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 }, // 13
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 }, // 14
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }  // 15
	},
	{ // 91 - 1 1 2 3 (3 3 2 3)
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 92 - 1 1 3 0 (3 3 3 0)
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 93 - 1 1 3 1 (3 3 3 1)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 94 - 1 1 3 2 (3 1 3 2)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 95 - 1 1 3 3 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 96 - 1 2 0 0
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 0
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 1
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 2
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 7
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }  // 15
	},
	{ // 97 - 1 2 0 1
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 1
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }, // 2
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 98 - 1 2 0 2
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 0
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 1
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }, // 2
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 4
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 6
		{ S1, X1, Y1, X0, S0, B0, B1, Y0, NG, NG }, // 7
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 8
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 10
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }, // 14
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }  // 15
	},
	{ // 99 - 1 2 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, Y1, X0, S0, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 100 - 1 2 1 0
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 1
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }, // 2
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 101 - 1 2 1 1
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 1
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 }, // 2
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 5
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 }, // 6
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 9
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 }, // 10
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }  // 15
	},
	{ // 102 - 1 2 1 2
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 0
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 1
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }, // 2
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }, // 3
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 4
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 5
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 6
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 }, // 7
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 9
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 10
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 11
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 13
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }, // 14
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }  // 15
	},
	{ // 103 - 1 2 1 3
		{ S1, S0, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S1, S0, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S1, S0, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S1, S0, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 4
		{ S1, S0, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, S0, Y0 }, // 6
		{ S1, X1, X0, S0, B2, B3, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, S0, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, S0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 104 - 1 2 2 0
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 0
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 1
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }, // 2
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, Y1, X0, S0, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 7
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 8
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }  // 15
	},
	{ // 105 - 1 2 2 1
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 0
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 1
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }, // 2
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }, // 3
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 5
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 }, // 6
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 9
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 10
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 13
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }, // 14
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }  // 15
	},
	{ // 106 - 1 2 2 2
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S1, X1, Y1, B0, B1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, X1, Y1, B2, B3, S0, B0, B1, X0, Y0 }, // 5
		{ S1, X1, Y1, X0, B0, B1, S0, B2, B3, Y0 }, // 6
		{ S1, X1, Y1, X0, B2, B3, S0, B0, B1, Y0 }, // 7
		{ S1, X1, Y1, B0, B1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, X1, Y1, B2, B3, B0, B1, S0, X0, Y0 }, // 9
		{ S1, X1, Y1, X0, B0, B1, B2, B3, S0, Y0 }, // 10
		{ S1, X1, Y1, X0, B2, B3, B0, B1, S0, Y0 }, // 11
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, S0 }, // 13
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, S0 }, // 14
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, S0 }  // 15
	},
	{ // 107 - 1 2 2 3
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, Y1, S0, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, Y1, B2, B3, S0, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, S0, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, Y0, X0, S0 }, // 13
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 108 - 1 2 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, Y1, S0, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, S0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 109 - 1 2 3 1
		{ S1, S0, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S1, S0, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S1, S0, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S1, S0, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, S0, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, S0, B0, B1, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, S0, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, S0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 110 - 1 2 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 111 - 1 2 3 3 (3 2 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 112 - 1 3 0 0 (3 3 0 0)
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 7
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }  // 15
	},
	{ // 113 - 1 3 0 1 (3 3 0 1)
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 114 - 1 3 0 2 (3 3 0 2)
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 115 - 1 3 0 3 (3 3 0 3)
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 116 - 1 3 1 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 117 - 1 3 1 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 }, // 2
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }  // 15
	},
	{ // 118 - 1 3 1 2
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, B0, B1, Y0 }, // 7
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 13
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }, // 14
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }, // 15
	},
	{ // 119 - 1 3 1 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 120 - 1 3 2 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, Y1, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, Y1, B2, B3, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, Y1, B2, B3, Y0, S0, NG, NG }, // 15
	},
	{ // 121 - 1 3 2 1
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 122 - 1 3 2 2
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 123 - 1 3 2 3 (3 3 2 3)
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 124 - 1 3 3 0 (3 3 3 0)
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 125 - 1 3 3 1 (3 3 3 1)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 126 - 1 3 3 2 (3 3 3 2)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 127 - 1 3 3 3 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 128 - 2 0 0 0
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 6
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 7
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 10
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 11
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 12
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 13
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }, // 14
		{ X1, Y1, S1, X0, Y0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 129 - 2 0 0 1
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 0
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 1
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 2
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 3
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 4
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 5
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 6
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 7
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 8
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 9
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 10
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 11
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 12
		{ B0, B1, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 13
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }, // 14
		{ X1, B0, B1, Y1, S1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 130 - 2 0 0 2
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 4
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 6
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 7
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 8
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 9
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 10
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 11
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 12
		{ X1, Y1, S1, B0, B1, X0, Y0, NG, NG, NG }, // 13
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }, // 14
		{ X1, Y1, S1, X0, B0, B1, Y0, NG, NG, NG }  // 15
	},
	{ // 131 - 2 0 0 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 6
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 15

	},
	{ // 132 - 2 0 1 0
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 0
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 1
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 2
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 3
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 4
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 5
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 6
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 7
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 8
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 9
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 10
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 11
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 12
		{ B2, B3, X1, Y1, S1, X0, Y0, NG, NG, NG }, // 13
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }, // 14
		{ X1, B2, B3, Y1, S1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 133 - 2 0 1 1
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG }, // 0
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG }, // 1
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG }, // 2
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }, // 3
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG }, // 4
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG }, // 5
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG }, // 6
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }, // 7
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG }, // 8
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG }, // 9
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG }, // 10
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }, // 11
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, NG }, // 12
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, NG }, // 13
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, NG }, // 14
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, NG }  // 15
	},
	{ // 134 - 2 0 1 2
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 0
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 1
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 2
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 3
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 4
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 5
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 6
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 7
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 8
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 9
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 10
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 11
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 12
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, NG }, // 13
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }, // 14
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, NG }  // 15
	},
	{ // 135 - 2 0 1 3 (3 0 1 3)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 136 - 2 0 2 0
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 5
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 6
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 7
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 8
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 9
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 10
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 11
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 12
		{ X1, Y1, S1, B2, B3, X0, Y0, NG, NG, NG }, // 13
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }, // 14
		{ X1, Y1, S1, X0, B2, B3, Y0, NG, NG, NG }  // 15
	},
	{ // 137 - 2 0 2 1
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 0
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 1
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 2
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 3
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 4
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 5
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 6
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 7
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 8
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 9
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 10
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 11
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 12
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, NG }, // 13
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }, // 14
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, NG }  // 15
	},
	{ // 138 - 2 0 2 2
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG }, // 4
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG }, // 5
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG }, // 6
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG }, // 7
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG }, // 8
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG }, // 9
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG }, // 10
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG }, // 11
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, NG }, // 12
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, NG }, // 13
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, NG }, // 14
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, NG }  // 15
	},
	{ // 139 - 2 0 2 3 (3 0 2 3)
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 140 - 2 0 3 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
	},
	{ // 141 - 2 0 3 1
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 142 - 2 0 3 2
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 143 - 2 0 3 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 144 - 2 1 0 0
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 6
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 7
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 10
		{ X1, S0, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 11
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 12
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 13
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 14
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 145 - 2 1 0 1
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG }, // 1
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG }, // 2
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG }, // 3
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 4
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG }, // 5
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG }, // 6
		{ X1, S0, B0, B1, Y1, S1, X0, Y0, NG, NG }, // 7
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 8
		{ B0, B1, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 9
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG }, // 10
		{ X1, B0, B1, S0, Y1, S1, X0, Y0, NG, NG }, // 11
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 12
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 13
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }, // 14
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }  // 15
	},
	{ // 146 - 2 1 0 2
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 4
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 5
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 6
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 7
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 8
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 9
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 10
		{ X1, S0, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 11
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG }, // 12
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG }, // 13
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }, // 14
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }  // 15
	},
	{ // 147 - 2 1 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, S0, X0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }  // 15
	},
	{ // 148 - 2 1 1 0
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG }, // 1
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG }, // 2
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG }, // 3
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG }, // 4
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 5
		{ X1, S0, B2, B3, Y1, S1, X0, Y0, NG, NG }, // 6
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG }, // 7
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 8
		{ B2, B3, S0, X1, Y1, S1, X0, Y0, NG, NG }, // 9
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG }, // 10
		{ X1, B2, B3, S0, Y1, S1, X0, Y0, NG, NG }, // 11
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 12
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 13
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }, // 14
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }  // 15
	},
	{ // 149 - 2 1 1 1
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 }, // 1
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 }, // 2
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 }, // 3
		{ B0, B1, S0, B2, B3, X1, Y1, S1, X0, Y0 }, // 4
		{ B2, B3, S0, B0, B1, X1, Y1, S1, X0, Y0 }, // 5
		{ X1, B0, B1, S0, B2, B3, Y1, S1, X0, Y0 }, // 6
		{ X1, B2, B3, S0, B0, B1, Y1, S1, X0, Y0 }, // 7
		{ B0, B1, B2, B3, S0, X1, Y1, S1, X0, Y0 }, // 8
		{ B2, B3, B0, B1, S0, X1, Y1, S1, X0, Y0 }, // 9
		{ X1, B0, B1, B2, B3, S0, Y1, S1, X0, Y0 }, // 10
		{ X1, B2, B3, B0, B1, S0, Y1, S1, X0, Y0 }, // 11
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 }, // 12
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 }, // 13
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 }, // 14
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }  // 15
	},
	{ // 150 - 2 1 1 2
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 }, // 0
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 }, // 1
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 }, // 2
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 }, // 3
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 }, // 4
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 }, // 5
		{ X1, S0, B2, B3, Y1, S1, X0, B0, B1, Y0 }, // 6
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 }, // 7
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 }, // 8
		{ B2, B3, S0, X1, Y1, S1, B0, B1, X0, Y0 }, // 9
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 }, // 10
		{ X1, B2, B3, S0, Y1, S1, X0, B0, B1, Y0 }, // 11
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 }, // 12
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 }, // 13
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }, // 14
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }  // 15
	},
	{ // 151 - 2 1 1 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, X1, B2, B3, S1, X0, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ B2, B3, S0, S1, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 152 - 2 1 2 0
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 5
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 6
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 7
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 8
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 9
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 10
		{ X1, S0, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 11
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG }, // 12
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG }, // 13
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }, // 14
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }  // 15
	},
	{ // 153 - 2 1 2 1
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 }, // 0
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 }, // 1
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 }, // 2
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 }, // 3
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 }, // 4
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 }, // 5
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 }, // 6
		{ X1, S0, B0, B1, Y1, S1, X0, B2, B3, Y0 }, // 7
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 }, // 8
		{ B0, B1, S0, X1, Y1, S1, B2, B3, X0, Y0 }, // 9
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 }, // 10
		{ X1, B0, B1, S0, Y1, S1, X0, B2, B3, Y0 }, // 11
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 }, // 12
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 }, // 13
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }, // 14
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }  // 15
	},
	{ // 154 - 2 1 2 2
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S0, X1, Y1, S1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S0, X1, Y1, S1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 }, // 4
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 }, // 5
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 }, // 6
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 }, // 7
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 }, // 8
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 }, // 9
		{ X1, S0, Y1, S1, X0, B0, B1, B2, B3, Y0 }, // 10
		{ X1, S0, Y1, S1, X0, B2, B3, B0, B1, Y0 }, // 11
		{ X1, Y1, S0, S1, B0, B1, B2, B3, X0, Y0 }, // 12
		{ X1, Y1, S0, S1, B2, B3, B0, B1, X0, Y0 }, // 13
		{ X1, Y1, S0, S1, X0, B0, B1, B2, B3, Y0 }, // 14
		{ X1, Y1, S0, S1, X0, B2, B3, B0, B1, Y0 }  // 15
	},
	{ // 155 - 2 1 2 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 156 - 2 1 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 157 - 2 1 3 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 158 - 2 1 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 159 - 2 1 3 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 160 - 2 2 0 0
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 0
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 1
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 2
		{ X1, Y1, S0, S1, X0, Y0, NG, NG, NG, NG }, // 3
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG }, // 4
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG }, // 5
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG }, // 6
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG }, // 7
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG }, // 8
		{ X1, Y1, S1, S0, X0, Y0, NG, NG, NG, NG }, // 9
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG }, // 10
		{ X1, Y1, S1, X0, S0, Y0, NG, NG, NG, NG }, // 11
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ X1, Y1, S1, X0, Y0, S0, NG, NG, NG, NG }  // 15
	},
	{ // 161 - 2 2 0 1
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 0
		{ B0, B1, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 1
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }, // 2
		{ X1, B0, B1, Y1, S0, S1, X0, Y0, NG, NG }, // 3
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 4
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 5
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG }, // 6
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG }, // 7
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 8
		{ B0, B1, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 9
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG }, // 10
		{ X1, B0, B1, Y1, S1, X0, S0, Y0, NG, NG }, // 11
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG }, // 12
		{ B0, B1, X1, Y1, S1, X0, Y0, S0, NG, NG }, // 13
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG }, // 14
		{ X1, B0, B1, Y1, S1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 162 - 2 2 0 2
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG }, // 0
		{ X1, Y1, S0, S1, B0, B1, X0, Y0, NG, NG }, // 1
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }, // 2
		{ X1, Y1, S0, S1, X0, B0, B1, Y0, NG, NG }, // 3
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG }, // 4
		{ X1, Y1, S1, S0, B0, B1, X0, Y0, NG, NG }, // 5
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG }, // 6
		{ X1, Y1, S1, X0, S0, B0, B1, Y0, NG, NG }, // 7
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG }, // 8
		{ X1, Y1, S1, B0, B1, S0, X0, Y0, NG, NG }, // 9
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG }, // 10
		{ X1, Y1, S1, X0, B0, B1, S0, Y0, NG, NG }, // 11
		{ X1, Y1, S1, B0, B1, X0, Y0, S0, NG, NG }, // 12
		{ X1, Y1, S1, B0, B1, X0, Y0, S0, NG, NG }, // 13
		{ X1, Y1, S1, X0, B0, B1, Y0, S0, NG, NG }, // 14
		{ X1, Y1, S1, X0, B0, B1, Y0, S0, NG, NG }  // 15
	},
	{ // 163 - 2 2 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 164 - 2 2 1 0
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 0
		{ B2, B3, X1, Y1, S0, S1, X0, Y0, NG, NG }, // 1
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }, // 2
		{ X1, B2, B3, Y1, S0, S1, X0, Y0, NG, NG }, // 3
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 4
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 5
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG }, // 6
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG }, // 7
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 8
		{ B2, B3, X1, Y1, S1, S0, X0, Y0, NG, NG }, // 9
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG }, // 10
		{ X1, B2, B3, Y1, S1, X0, S0, Y0, NG, NG }, // 11
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG }, // 12
		{ B2, B3, X1, Y1, S1, X0, Y0, S0, NG, NG }, // 13
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG }, // 14
		{ X1, B2, B3, Y1, S1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 165 - 2 2 1 1
		{ B0, B1, B2, B3, X1, Y1, S0, S1, X0, Y0 }, // 0
		{ B2, B3, B0, B1, X1, Y1, S0, S1, X0, Y0 }, // 1
		{ X1, B0, B1, B2, B3, Y1, S0, S1, X0, Y0 }, // 2
		{ X1, B2, B3, B0, B1, Y1, S0, S1, X0, Y0 }, // 3
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 }, // 4
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 }, // 5
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 }, // 6
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 }, // 7
		{ B0, B1, B2, B3, X1, Y1, S1, S0, X0, Y0 }, // 8
		{ B2, B3, B0, B1, X1, Y1, S1, S0, X0, Y0 }, // 9
		{ X1, B0, B1, B2, B3, Y1, S1, X0, S0, Y0 }, // 10
		{ X1, B2, B3, B0, B1, Y1, S1, X0, S0, Y0 }, // 11
		{ B0, B1, B2, B3, X1, Y1, S1, X0, Y0, S0 }, // 12
		{ B2, B3, B0, B1, X1, Y1, S1, X0, Y0, S0 }, // 13
		{ X1, B0, B1, B2, B3, Y1, S1, X0, Y0, S0 }, // 14
		{ X1, B2, B3, B0, B1, Y1, S1, X0, Y0, S0 }  // 15
	},
	{ // 166 - 2 2 1 2
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 }, // 0
		{ B2, B3, X1, Y1, S0, S1, B0, B1, X0, Y0 }, // 1
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }, // 2
		{ X1, B2, B3, Y1, S0, S1, X0, B0, B1, Y0 }, // 3
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 }, // 4
		{ B2, B3, X1, Y1, S1, S0, B0, B1, X0, Y0 }, // 5
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 }, // 6
		{ X1, B2, B3, Y1, S1, X0, S0, B0, B1, Y0 }, // 7
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 }, // 8
		{ B2, B3, X1, Y1, S1, B0, B1, S0, X0, Y0 }, // 9
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 }, // 10
		{ X1, B2, B3, Y1, S1, X0, B0, B1, S0, Y0 }, // 11
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 }, // 12
		{ B2, B3, X1, Y1, S1, B0, B1, X0, Y0, S0 }, // 13
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 }, // 14
		{ X1, B2, B3, Y1, S1, X0, B0, B1, Y0, S0 }  // 15
	},
	{ // 167 - 2 2 1 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 168 - 2 2 2 0
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG }, // 0
		{ X1, Y1, S0, S1, B2, B3, X0, Y0, NG, NG }, // 1
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }, // 2
		{ X1, Y1, S0, S1, X0, B2, B3, Y0, NG, NG }, // 3
		{ X1, Y1, S1, S0, B2, B3, X0, Y0, NG, NG }, // 4
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG }, // 5
		{ X1, Y1, S1, X0, S0, B2, B3, Y0, NG, NG }, // 6
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG }, // 7
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG }, // 8
		{ X1, Y1, S1, B2, B3, S0, X0, Y0, NG, NG }, // 9
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG }, // 10
		{ X1, Y1, S1, X0, B2, B3, S0, Y0, NG, NG }, // 11
		{ X1, Y1, S1, B2, B3, X0, Y0, S0, NG, NG }, // 12
		{ X1, Y1, S1, B2, B3, X0, Y0, S0, NG, NG }, // 13
		{ X1, Y1, S1, X0, B2, B3, Y0, S0, NG, NG }, // 14
		{ X1, Y1, S1, X0, B2, B3, Y0, S0, NG, NG }  // 15
	},
	{ // 169 - 2 2 2 1
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 }, // 0
		{ B0, B1, X1, Y1, S0, S1, B2, B3, X0, Y0 }, // 1
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }, // 2
		{ X1, B0, B1, Y1, S0, S1, X0, B2, B3, Y0 }, // 3
		{ B0, B1, X1, Y1, S1, S0, B2, B3, X0, Y0 }, // 4
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 }, // 5
		{ X1, B0, B1, Y1, S1, X0, S0, B2, B3, Y0 }, // 6
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 }, // 7
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 }, // 8
		{ B0, B1, X1, Y1, S1, B2, B3, S0, X0, Y0 }, // 9
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 }, // 10
		{ X1, B0, B1, Y1, S1, X0, B2, B3, S0, Y0 }, // 11
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 }, // 12
		{ B0, B1, X1, Y1, S1, B2, B3, X0, Y0, S0 }, // 13
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 }, // 14
		{ X1, B0, B1, Y1, S1, X0, B2, B3, Y0, S0 }  // 15
	},
	{ // 170 - 2 2 2 2
		{ X1, Y1, S0, S1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ X1, Y1, S0, S1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ X1, Y1, S0, S1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ X1, Y1, S0, S1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ X1, Y1, S1, B0, B1, S0, B2, B3, X0, Y0 }, // 4
		{ X1, Y1, S1, B2, B3, S0, B0, B1, X0, Y0 }, // 5
		{ X1, Y1, S1, X0, B0, B1, S0, B2, B3, Y0 }, // 6
		{ X1, Y1, S1, X0, B2, B3, S0, B0, B1, Y0 }, // 7
		{ X1, Y1, S1, B0, B1, B2, B3, S0, X0, Y0 }, // 8
		{ X1, Y1, S1, B2, B3, B0, B1, S0, X0, Y0 }, // 9
		{ X1, Y1, S1, X0, B0, B1, B2, B3, S0, Y0 }, // 10
		{ X1, Y1, S1, X0, B2, B3, B0, B1, S0, Y0 }, // 11
		{ X1, Y1, S1, B0, B1, B2, B3, X0, Y0, S0 }, // 12
		{ X1, Y1, S1, B2, B3, B0, B1, X0, Y0, S0 }, // 13
		{ X1, Y1, S1, X0, B0, B1, B2, B3, Y0, S0 }, // 14
		{ X1, Y1, S1, X0, B2, B3, B0, B1, Y0, S0 }  // 15
	},
	{ // 171 - 2 2 2 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 172 - 2 2 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 173 - 2 2 3 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 174 - 2 2 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 175 - 2 2 3 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 176 - 2 3 0 0
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 7
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 15
	},
	{ // 177 - 2 3 0 1
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG }, // 0
		{ S0, B0, B1, X1, Y1, S1, X0, Y0, NG, NG }, // 1
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG }, // 2
		{ S0, X1, B0, B1, Y1, S1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 178 - 2 3 0 2
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, X1, Y1, S1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 179 - 2 3 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 180 - 2 3 1 0
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG }, // 0
		{ S0, B2, B3, X1, Y1, S1, X0, Y0, NG, NG }, // 1
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG }, // 2
		{ S0, X1, B2, B3, Y1, S1, X0, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 181 - 2 3 1 1
		{ S0, B0, B1, B2, B3, X1, Y1, S1, X0, Y0 }, // 0
		{ S0, B2, B3, B0, B1, X1, Y1, S1, X0, Y0 }, // 1
		{ S0, X1, B0, B1, B2, B3, Y1, S1, X0, Y0 }, // 2
		{ S0, X1, B2, B3, B0, B1, Y1, S1, X0, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }, // 15
	},
	{ // 182 - 2 3 1 2
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 }, // 0
		{ S0, B2, B3, X1, Y1, S1, B0, B1, X0, Y0 }, // 1
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 }, // 2
		{ S0, X1, B2, B3, Y1, S1, X0, B0, B1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 183 - 2 3 1 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 184 - 2 3 2 0
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, X1, Y1, S1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, X1, Y1, S1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 185 - 2 3 2 1
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 }, // 0
		{ S0, B0, B1, X1, Y1, S1, B2, B3, X0, Y0 }, // 1
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 }, // 2
		{ S0, X1, B0, B1, Y1, S1, X0, B2, B3, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 186 - 2 3 2 2
		{ S0, X1, Y1, S1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S0, X1, Y1, S1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S0, X1, Y1, S1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S0, X1, Y1, S1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 187 - 2 3 2 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 188 - 2 3 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 189 - 2 3 3 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 190 - 2 3 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 191 - 2 3 3 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, X1, S1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 192 - 3 0 0 0
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 0
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 3
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 4
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 7
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 8
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, NG, NG, NG, NG, NG }  // 15
	},
	{ // 193 - 3 0 0 1
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 194 - 3 0 0 2
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 0
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 3
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 4
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 7
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 8
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 11
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 12
		{ S1, X1, Y1, B0, B1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, B0, B1, Y0, NG, NG, NG }  // 15
	},
	{ // 195 - 3 0 0 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 6
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, NG, NG, NG }, // 15
	},
	{ // 196 - 3 0 1 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 2
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, Y0, NG, NG, NG }  // 15
	},
	{ // 197 - 3 0 1 1
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 2
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 6
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 10
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, NG }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, NG }  // 15
	},
	{ // 198 - 3 0 1 2
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 0
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 2
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 3
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 4
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 7
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 11
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, NG }  // 15
	},
	{ // 199 - 3 0 1 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 200 - 3 0 2 0
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 0
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 2
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 3
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 4
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 7
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 8
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, Y0, NG, NG, NG }  // 15
	},
	{ // 201 - 3 0 2 1
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 1
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 2
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 12
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, NG }  // 15
	},
	{ // 202 - 3 0 2 2
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 0
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 2
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 3
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 4
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 7
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 8
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }, // 11
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, NG }, // 12
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, NG }  // 15
	},
	{ // 203 - 3 0 2 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 204 - 3 0 3 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 0
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 1
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 5
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, NG, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, NG, NG, NG }, // 15
	},
	{ // 205 - 3 0 3 1 (3 0 3 3)
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 206 - 3 0 3 2
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, B0, B1, Y0, NG }, // 15
	},
	{ // 207 - 3 0 3 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 0
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 1
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 2
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 5
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 6
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, NG }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, NG }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, NG }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, NG }, // 15
	},
	{ // 208 - 3 1 0 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 7
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }  // 15
	},
	{ // 209 - 3 1 0 1
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 6
		{ S1, X1, S0, B0, B1, Y1, X0, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 10
		{ S1, X1, B0, B1, S0, Y1, X0, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, S0, X0, Y0, NG, NG }  // 15
	},
	{ // 210 - 3 1 0 2
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 7
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 8
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, B0, B1, Y0, NG, NG }, // 11
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 12
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, B0, B1, Y0, NG, NG }  // 15
	},
	{ // 211 - 3 1 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }  // 15
	},
	{ // 212 - 3 1 1 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, B2, B3, Y1, X0, Y0, NG, NG }, // 6
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 10
		{ S1, X1, B2, B3, S0, Y1, X0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, S0, X0, Y0, NG, NG }  // 15
	},
	{ // 213 - 3 1 1 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 }, // 2
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, B0, B1, S0, B2, B3, Y1, X0, Y0 }, // 6
		{ S1, X1, B2, B3, S0, B0, B1, Y1, X0, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, B0, B1, B2, B3, S0, Y1, X0, Y0 }, // 10
		{ S1, X1, B2, B3, B0, B1, S0, Y1, X0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, S0, X0, Y0 }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, S0, X0, Y0 }  // 15
	},
	{ // 214 - 3 1 1 2
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 3
		{ S1, S0, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 5
		{ S1, X1, S0, B2, B3, Y1, X0, B0, B1, Y0 }, // 6
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 7
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 8
		{ S1, B2, B3, S0, X1, Y1, B0, B1, X0, Y0 }, // 9
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 10
		{ S1, X1, B2, B3, S0, Y1, X0, B0, B1, Y0 }, // 11
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 12
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 13
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }, // 14
		{ S1, X1, B2, B3, Y1, S0, X0, B0, B1, Y0 }  // 15
	},
	{ // 215 - 3 1 1 3 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 216 - 3 1 2 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 5
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 7
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 8
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 9
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 10
		{ S1, X1, S0, Y1, X0, B2, B3, Y0, NG, NG }, // 11
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 12
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 13
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }, // 14
		{ S1, X1, Y1, S0, X0, B2, B3, Y0, NG, NG }  // 15
	},
	{ // 217 - 3 1 2 1
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, S0, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 5
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 6
		{ S1, X1, S0, B0, B1, Y1, X0, B2, B3, Y0 }, // 7
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 8
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 9
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 10
		{ S1, X1, B0, B1, S0, Y1, X0, B2, B3, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 12
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 13
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }, // 14
		{ S1, X1, B0, B1, Y1, S0, X0, B2, B3, Y0 }  // 15
	},
	{ // 218 - 3 1 2 2
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 4
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 5
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 }, // 6
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 }, // 7
		{ S1, S0, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 8
		{ S1, S0, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 9
		{ S1, X1, S0, Y1, X0, B0, B1, B2, B3, Y0 }, // 10
		{ S1, X1, S0, Y1, X0, B2, B3, B0, B1, Y0 }, // 11
		{ S1, X1, Y1, S0, B0, B1, B2, B3, X0, Y0 }, // 12
		{ S1, X1, Y1, S0, B2, B3, B0, B1, X0, Y0 }, // 13
		{ S1, X1, Y1, S0, X0, B0, B1, B2, B3, Y0 }, // 14
		{ S1, X1, Y1, S0, X0, B2, B3, B0, B1, Y0 }  // 15
	},
	{ // 219 - 3 1 2 3 (3 3 2 3)
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 220 - 3 1 3 0 (3 3 3 0)
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 221 - 3 1 3 1 (3 3 3 1)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 222 - 3 1 3 2 (3 3 3 2)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 223 - 3 1 3 3 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 224 - 3 2 0 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 7
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, X1, Y1, S0, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, Y1, X0, S0, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }  // 15
	},
	{ // 225 - 3 2 0 1
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 6
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 10
		{ S1, X1, B0, B1, Y1, X0, S0, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 226 - 3 2 0 2
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 4
		{ S1, X1, Y1, S0, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 6
		{ S1, X1, Y1, X0, S0, B0, B1, Y0, NG, NG }, // 7
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 8
		{ S1, X1, Y1, B0, B1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 10
		{ S1, X1, Y1, X0, B0, B1, S0, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B0, B1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }, // 14
		{ S1, X1, Y1, X0, B0, B1, Y0, S0, NG, NG }  // 15
	},
	{ // 227 - 3 2 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, Y1, S0, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, Y1, X0, S0, Y0, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 228 - 3 2 1 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 6
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 10
		{ S1, X1, B2, B3, Y1, X0, S0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B2, B3, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 229 - 3 2 1 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, B2, B3, Y1, X0, Y0 }, // 2
		{ S0, S1, X1, B2, B3, B0, B1, Y1, X0, Y0 }, // 3
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 4
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 5
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 }, // 6
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 }, // 7
		{ S1, B0, B1, B2, B3, X1, Y1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, X1, Y1, S0, X0, Y0 }, // 9
		{ S1, X1, B0, B1, B2, B3, Y1, X0, S0, Y0 }, // 10
		{ S1, X1, B2, B3, B0, B1, Y1, X0, S0, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, B0, B1, B2, B3, Y1, X0, Y0, S0 }, // 14
		{ S1, X1, B2, B3, B0, B1, Y1, X0, Y0, S0 }  // 15
	},
	{ // 230 - 3 2 1 2
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, X1, Y1, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 2
		{ S0, S1, X1, B2, B3, Y1, X0, B0, B1, Y0 }, // 3
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 4
		{ S1, B2, B3, X1, Y1, S0, B0, B1, X0, Y0 }, // 5
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 6
		{ S1, X1, B2, B3, Y1, X0, S0, B0, B1, Y0 }, // 7
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 8
		{ S1, B2, B3, X1, Y1, B0, B1, S0, X0, Y0 }, // 9
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 10
		{ S1, X1, B2, B3, Y1, X0, B0, B1, S0, Y0 }, // 11
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, X1, Y1, B0, B1, X0, Y0, S0 }, // 13
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }, // 14
		{ S1, X1, B2, B3, Y1, X0, B0, B1, Y0, S0 }  // 15
	},
	{ // 231 - 3 2 1 3 (3 3 1 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 232 - 3 2 2 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, X1, Y1, S0, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 5
		{ S1, X1, Y1, X0, S0, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 7
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 8
		{ S1, X1, Y1, B2, B3, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 10
		{ S1, X1, Y1, X0, B2, B3, S0, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }, // 14
		{ S1, X1, Y1, X0, B2, B3, Y0, S0, NG, NG }  // 15
	},
	{ // 233 - 3 2 2 1
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, B2, B3, Y0 }, // 3
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 5
		{ S1, X1, B0, B1, Y1, X0, S0, B2, B3, Y0 }, // 6
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 9
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 10
		{ S1, X1, B0, B1, Y1, X0, B2, B3, S0, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 13
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }, // 14
		{ S1, X1, B0, B1, Y1, X0, B2, B3, Y0, S0 }  // 15
	},
	{ // 234 - 3 2 2 2
		{ S0, S1, X1, Y1, B0, B1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, X1, Y1, B2, B3, B0, B1, X0, Y0 }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, B0, B1, Y0 }, // 3
		{ S1, X1, Y1, B0, B1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, X1, Y1, B2, B3, S0, B0, B1, X0, Y0 }, // 5
		{ S1, X1, Y1, X0, B0, B1, S0, B2, B3, Y0 }, // 6
		{ S1, X1, Y1, X0, B2, B3, S0, B0, B1, Y0 }, // 7
		{ S1, X1, Y1, B0, B1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, X1, Y1, B2, B3, B0, B1, S0, X0, Y0 }, // 9
		{ S1, X1, Y1, X0, B0, B1, B2, B3, S0, Y0 }, // 10
		{ S1, X1, Y1, X0, B2, B3, B0, B1, S0, Y0 }, // 11
		{ S1, X1, Y1, B0, B1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, X1, Y1, B2, B3, B0, B1, X0, Y0, S0 }, // 13
		{ S1, X1, Y1, X0, B0, B1, B2, B3, Y0, S0 }, // 14
		{ S1, X1, Y1, X0, B2, B3, B0, B1, Y0, S0 }  // 15
	},
	{ // 235 - 3 2 2 3
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, X1, Y1, S0, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, Y1, S0, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, X1, Y1, B2, B3, S0, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, Y1, B2, B3, S0, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, Y1, S0, Y0 }, // 11
		{ S1, B0, B1, X1, Y1, B2, B3, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, Y1, B2, B3, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 236 - 3 2 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, X1, Y1, S0, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, Y1, S0, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, Y1, S0, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 237 - 3 2 3 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 238 - 3 2 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 239 - 3 2 3 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 240 - 3 3 0 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 0
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 3
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 4
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 5
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 6
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 7
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 8
		{ S1, S0, X1, Y1, X0, Y0, NG, NG, NG, NG }, // 9
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 10
		{ S1, X1, X0, S0, Y1, Y0, NG, NG, NG, NG }, // 11
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 12
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 13
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }, // 14
		{ S1, X1, Y1, X0, Y0, S0, NG, NG, NG, NG }  // 15
	},
	{ // 241 - 3 3 0 1
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 2
		{ S0, S1, X1, B0, B1, Y1, X0, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }, // 14
		{ S1, X1, B0, B1, Y1, X0, Y0, S0, NG, NG }  // 15
	},
	{ // 242 - 3 3 0 2
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B0, B1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, X1, Y1, B0, B1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, Y0, NG, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 15
	},
	{ // 243 - 3 3 0 3
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B0, B1, Y1, Y0, NG, NG }, // 3
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, S0, B0, B1, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, S0, B0, B1, Y1, X0, Y0, NG }, // 7
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B0, B1, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B0, B1, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B0, B1, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B0, B1, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 244 - 3 3 1 0 (3 3 3 0)
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 245 - 3 3 1 1 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 246 - 3 3 1 2 (3 3 3 2)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 247 - 3 3 1 3 (3 3 3 3)
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 248 - 3 3 2 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 0
		{ S0, S1, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 2
		{ S0, S1, X1, Y1, X0, B2, B3, Y0, NG, NG }, // 3
		{ S1, S0, X1, Y1, B2, B3, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, Y1, B2, B3, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 12
		{ S1, X1, Y1, B2, B3, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, Y1, B2, B3, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, Y1, B2, B3, Y0, S0, NG, NG }, // 15
	},
	{ // 249 - 3 3 2 1 (3 3 2 3)
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 250 - 3 3 2 2 (3 3 2 3)
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 251 - 3 3 2 3
		{ S0, S1, B0, B1, X1, Y1, B2, B3, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, Y1, B2, B3, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, X1, Y1, B2, B3, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, Y1, B2, B3, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 252 - 3 3 3 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 0
		{ S0, S1, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 1
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 2
		{ S0, S1, X1, X0, B2, B3, Y1, Y0, NG, NG }, // 3
		{ S1, S0, B2, B3, X1, Y1, X0, Y0, NG, NG }, // 4
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 5
		{ S1, X1, X0, S0, B2, B3, Y1, Y0, NG, NG }, // 6
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 7
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 8
		{ S1, B2, B3, S0, X1, Y1, X0, Y0, NG, NG }, // 9
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 10
		{ S1, X1, X0, B2, B3, S0, Y1, Y0, NG, NG }, // 11
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 12
		{ S1, B2, B3, X1, Y1, X0, Y0, S0, NG, NG }, // 13
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }, // 14
		{ S1, X1, X0, B2, B3, Y1, Y0, S0, NG, NG }  // 15
	},
	{ // 253 - 3 3 3 1
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 254 - 3 3 3 2
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	},
	{ // 255 - 3 3 3 3
		{ S0, S1, B0, B1, B2, B3, X1, Y1, X0, Y0 }, // 0
		{ S0, S1, B2, B3, B0, B1, X1, Y1, X0, Y0 }, // 1
		{ S0, S1, X1, X0, B0, B1, B2, B3, Y1, Y0 }, // 2
		{ S0, S1, X1, X0, B2, B3, B0, B1, Y1, Y0 }, // 3
		{ S1, B0, B1, S0, B2, B3, X1, Y1, X0, Y0 }, // 4
		{ S1, B2, B3, S0, B0, B1, X1, Y1, X0, Y0 }, // 5
		{ S1, X1, X0, B0, B1, S0, B2, B3, Y1, Y0 }, // 6
		{ S1, X1, X0, B2, B3, S0, B0, B1, Y1, Y0 }, // 7
		{ S1, B0, B1, B2, B3, S0, X1, Y1, X0, Y0 }, // 8
		{ S1, B2, B3, B0, B1, S0, X1, Y1, X0, Y0 }, // 9
		{ S1, X1, X0, B0, B1, B2, B3, S0, Y1, Y0 }, // 10
		{ S1, X1, X0, B2, B3, B0, B1, S0, Y1, Y0 }, // 11
		{ S1, B0, B1, B2, B3, X1, Y1, X0, Y0, S0 }, // 12
		{ S1, B2, B3, B0, B1, X1, Y1, X0, Y0, S0 }, // 13
		{ S1, X1, X0, B0, B1, B2, B3, Y1, Y0, S0 }, // 14
		{ S1, X1, X0, B2, B3, B0, B1, Y1, Y0, S0 }  // 15
	}
};
