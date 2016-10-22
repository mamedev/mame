// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#if 0
static inline void video_filter16(int *out_r, int *out_g, int *out_b, uint16_t* vbuff, uint8_t* hbuff, const uint32_t hres);
static inline void divot_filter16(uint8_t* r, uint8_t* g, uint8_t* b, uint16_t* fbuff, uint32_t fbuff_index);
static inline void restore_filter16(int32_t* r, int32_t* g, int32_t* b, uint16_t* fbuff, uint32_t fbuff_index, uint32_t hres);
static inline void divot_filter16_buffer(int32_t* r, int32_t* g, int32_t* b, color_t* vibuffer);
static inline void restore_filter16_buffer(int32_t* r, int32_t* g, int32_t* b, color_t* vibuff, uint32_t hres);
static inline void restore_two(color_t* filtered, color_t* neighbour);
static inline void video_max(uint32_t* Pixels, uint8_t* max, uint32_t* enb);
static inline uint32_t ge_two(uint32_t enb);

static inline void video_filter16(int *out_r, int *out_g, int *out_b, uint16_t* vbuff, uint8_t* hbuff, const uint32_t hres)
{
	color_t penumax, penumin, max, min;
	uint16_t pix = *vbuff;
	const uint8_t centercvg = (*hbuff & 3) + ((pix & 1) << 2) + 1;
	uint32_t numoffull = 1;
	uint32_t cvg;
	uint32_t backr[7], backg[7], backb[7];
	uint32_t invr[7], invg[7], invb[7];
	int32_t coeff;
	int32_t leftup = -hres - 2;
	int32_t leftdown = hres - 2;
	int32_t toleft = -2;
	uint32_t colr, colg, colb;
	uint32_t enb;
	uint32_t r = ((pix >> 8) & 0xf8) | (pix >> 13);
	uint32_t g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
	uint32_t b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);

	*out_r = *out_g = *out_b = 0;

	backr[0] = r;
	backg[0] = g;
	backb[0] = b;
	invr[0] = ~r;
	invg[0] = ~g;
	invb[0] = ~b;

	if (centercvg == 8)
	{
		*out_r = r;
		*out_g = g;
		*out_b = b;
		return;
	}

	for(int i = 0; i < 5; i++)
	{
		pix = vbuff[leftup ^ WORD_ADDR_XOR];
		cvg = hbuff[leftup ^ BYTE_ADDR_XOR] & 3;
		if(i & 1)
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = ((pix >> 8) & 0xf8) | (pix >> 13);
				backg[numoffull] = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				backb[numoffull] = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
				invr[numoffull] = ~backr[numoffull];
				invg[numoffull] = ~backg[numoffull];
				invb[numoffull] = ~backb[numoffull];
			}
			else
			{
				backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		leftup++;
	}

	for(int i = 0; i < 5; i++)
	{
		pix = vbuff[leftdown ^ WORD_ADDR_XOR];
		cvg = hbuff[leftdown ^ BYTE_ADDR_XOR] & 3;
		if (i&1)
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = ((pix >> 8) & 0xf8) | (pix >> 13);
				backg[numoffull] = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				backb[numoffull] = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
				invr[numoffull] = ~backr[numoffull];
				invg[numoffull] = ~backg[numoffull];
				invb[numoffull] = ~backb[numoffull];
			}
			else
			{
				backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		leftdown++;
	}

	for(int i = 0; i < 5; i++)
	{
		pix = vbuff[toleft ^ WORD_ADDR_XOR];
		cvg = hbuff[toleft ^ BYTE_ADDR_XOR] & 3;
		if (!(i&3))
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = ((pix >> 8) & 0xf8) | (pix >> 13);
				backg[numoffull] = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				backb[numoffull] = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
				invr[numoffull] = ~backr[numoffull];
				invg[numoffull] = ~backg[numoffull];
				invb[numoffull] = ~backb[numoffull];
			}
			else
			{
				backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		toleft++;
	}

	video_max(&backr[0], &max.i.r, &enb);
	for(int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backr[i] = 0;
		}
	}
	video_max(&backg[0], &max.i.g, &enb);
	for (int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backg[i] = 0;
		}
	}
	video_max(&backb[0], &max.i.b, &enb);
	for (int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backb[i] = 0;
		}
	}
	video_max(&invr[0], &min.i.r, &enb);
	for (int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backr[i] = 0;
		}
	}
	video_max(&invg[0], &min.i.g, &enb);
	for (int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backg[i] = 0;
		}
	}
	video_max(&invb[0], &min.i.b, &enb);
	for (int i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backb[i] = 0;
		}
	}

	video_max(&backr[0], &penumax.i.r, &enb);
	penumax.i.r = ge_two(enb) ? max.i.r : penumax.i.r;

	video_max(&backg[0], &penumax.i.g, &enb);
	penumax.i.g = ge_two(enb) ? max.i.g : penumax.i.g;

	video_max(&backb[0], &penumax.i.b, &enb);
	penumax.i.b = ge_two(enb) ? max.i.b : penumax.i.b;

	video_max(&invr[0], &penumin.i.r, &enb);
	penumin.i.r = ge_two(enb) ? min.i.r : penumin.i.r;

	video_max(&invg[0], &penumin.i.g, &enb);
	penumin.i.g = ge_two(enb) ? min.i.g : penumin.i.g;

	video_max(&invb[0], &penumin.i.b, &enb);
	penumin.i.b = ge_two(enb) ? min.i.b : penumin.i.b;

	penumin.i.r = ~penumin.i.r;
	penumin.i.g = ~penumin.i.g;
	penumin.i.b = ~penumin.i.b;

	colr = (uint32_t)penumin.i.r + (uint32_t)penumax.i.r - (r << 1);
	colg = (uint32_t)penumin.i.g + (uint32_t)penumax.i.g - (g << 1);
	colb = (uint32_t)penumin.i.b + (uint32_t)penumax.i.b - (b << 1);
	coeff = 8 - centercvg;
	colr = (((colr * coeff) + 4) >> 3) + r;
	colg = (((colg * coeff) + 4) >> 3) + g;
	colb = (((colb * coeff) + 4) >> 3) + b;

	*out_r = colr & 0xff;
	*out_g = colg & 0xff;
	*out_b = colb & 0xff;
	return;
}

// This needs to be fixed for endianness.
static inline void divot_filter16(uint8_t* r, uint8_t* g, uint8_t* b, uint16_t* fbuff, uint32_t fbuff_index)
{
	uint8_t leftr, leftg, leftb, rightr, rightg, rightb;
	uint16_t leftpix, rightpix;
	uint16_t* next, *prev;
	uint32_t Lsw = fbuff_index & 1;
	next = (Lsw) ? (uint16_t*)(fbuff - 1) : (uint16_t*)(fbuff + 3);
	prev = (Lsw) ? (uint16_t*)(fbuff - 3) : (uint16_t*)(fbuff + 1);
	leftpix = *prev;
	rightpix = *next;

	//leftpix = *(fbuff - 1); //for BE targets
	//rightpix = *(fbuff + 1);

	leftr = ((leftpix >> 8) & 0xf8) | (leftpix >> 13);
	leftg = ((leftpix >> 3) & 0xf8) | ((leftpix >>  8) & 0x07);
	leftb = ((leftpix << 2) & 0xf8) | ((leftpix >>  3) & 0x07);
	rightr = ((rightpix >> 8) & 0xf8) | (rightpix >> 13);
	rightg = ((rightpix >> 3) & 0xf8) | ((rightpix >>  8) & 0x07);
	rightb = ((rightpix << 2) & 0xf8) | ((rightpix >>  3) & 0x07);
	if ((leftr >= *r && rightr >= leftr) || (leftr >= rightr && *r >= leftr))
	{
		*r = leftr; //left = median value
	}
	if ((rightr >= *r && leftr >= rightr) || (rightr >= leftr && *r >= rightr))
	{
		*r = rightr; //right = median, else *r itself is median
	}
	if ((leftg >= *g && rightg >= leftg) || (leftg >= rightg && *g >= leftg))
	{
		*g = leftg;
	}
	if ((rightg >= *g && leftg >= rightg) || (rightg >= leftg && *g >= rightg))
	{
		*g = rightg;
	}
	if ((leftb >= *b && rightb >= leftb) || (leftb >= rightb && *b >= leftb))
	{
		*b = leftb;
	}
	if ((rightb >= *b && leftb >= rightb) || (rightb >= leftb && *b >= rightb))
	{
		*b = rightb;
	}
}

static inline void divot_filter16_buffer(int* r, int* g, int* b, color_t* vibuffer)
{
	color_t leftpix = vibuffer[-1];
	color_t rightpix = vibuffer[1];
	color_t filtered = *vibuffer;

	*r = filtered.i.r;
	*g = filtered.i.g;
	*b = filtered.i.b;
	uint32_t leftr = leftpix.i.r;
	uint32_t leftg = leftpix.i.g;
	uint32_t leftb = leftpix.i.b;
	uint32_t rightr = rightpix.i.r;
	uint32_t rightg = rightpix.i.g;
	uint32_t rightb = rightpix.i.b;

	if ((leftr >= *r && rightr >= leftr) || (leftr >= rightr && *r >= leftr))
	{
		*r = leftr; //left = median value
	}
	if ((rightr >= *r && leftr >= rightr) || (rightr >= leftr && *r >= rightr))
	{
		*r = rightr; //right = median, else *r itself is median
	}
	if ((leftg >= *g && rightg >= leftg) || (leftg >= rightg && *g >= leftg))
	{
		*g = leftg;
	}
	if ((rightg >= *g && leftg >= rightg) || (rightg >= leftg && *g >= rightg))
	{
		*g = rightg;
	}
	if ((leftb >= *b && rightb >= leftb) || (leftb >= rightb && *b >= leftb))
	{
		*b = leftb;
	}
	if ((rightb >= *b && leftb >= rightb) || (rightb >= leftb && *b >= rightb))
	{
		*b = rightb;
	}

	filtered.i.r = *r;
	filtered.i.g = *g;
	filtered.i.b = *b;
}

// Fix me.
static inline void restore_filter16(int* r, int* g, int* b, uint16_t* fbuff, uint32_t fbuff_index, uint32_t hres)
{
	int32_t leftuppix = -hres - 1;
	int32_t leftdownpix = hres - 1;
	int32_t toleftpix = -1;
	uint8_t tempr, tempg, tempb;
	uint16_t pix;
	int i;

	uint8_t r5 = *r;
	uint8_t g5 = *g;
	uint8_t b5 = *b;
	r5 &= ~7;
	g5 &= ~7;
	b5 &= ~7;

	for (i = 0; i < 3; i++)
	{
		pix = fbuff[leftuppix ^ 1];
		tempr = ((pix >> 8) & 0xf8) | (pix >> 13);
		tempg = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
		tempb = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
		tempr &= ~7;
		tempg &= ~7;
		tempb &= ~7;
		if (tempr > r5)
		{
			*r += 1;
		}
		if (tempr < r5)
		{
			*r -= 1;
		}
		if (tempg > g5)
		{
			*g += 1;
		}
		if (tempg < g5)
		{
			*g -= 1;
		}
		if (tempb > b5)
		{
			*b += 1;
		}
		if (tempb < b5)
		{
			*b -= 1;
		}
		leftuppix++;
	}

	for (i = 0; i < 3; i++)
	{
		pix = fbuff[leftdownpix ^ 1];
		tempr = ((pix >> 8) & 0xf8) | (pix >> 13);
		tempg = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
		tempb = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
		tempr &= ~7;
		tempg &= ~7;
		tempb &= ~7;
		if (tempr > r5)
		{
			*r += 1;
		}
		if (tempr < r5)
		{
			*r -= 1;
		}
		if (tempg > g5)
		{
			*g += 1;
		}
		if (tempg < g5)
		{
			*g -= 1;
		}
		if (tempb > b5)
		{
			*b += 1;
		}
		if (tempb < b5)
		{
			*b -= 1;
		}
		leftdownpix++;
	}
	for(i = 0; i < 3; i++)
	{
		if (!(i & 1))
		{
			pix = fbuff[toleftpix ^ 1];
			tempr = ((pix >> 8) & 0xf8) | (pix >> 13);
			tempg = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
			tempb = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
			tempr &= ~7;
			tempg &= ~7;
			tempb &= ~7;
			if (tempr > r5)
			{
				*r += 1;
			}
			if (tempr < r5)
			{
				*r -= 1;
			}
			if (tempg > g5)
			{
				*g += 1;
			}
			if (tempg < g5)
			{
				*g -= 1;
			}
			if (tempb > b5)
			{
				*b += 1;
			}
			if (tempb < b5)
			{
				*b -= 1;
			}
		}
		toleftpix++;
	}
}

static inline void restore_filter16_buffer(int32_t* r, int32_t* g, int32_t* b, color_t* vibuff, uint32_t hres)
{
	color_t filtered;
	color_t leftuppix, leftdownpix, leftpix;
	color_t rightuppix, rightdownpix, rightpix;
	color_t uppix, downpix;
	int32_t ihres = (int32_t)hres; //can't apply unary minus to unsigned

	leftuppix = vibuff[-ihres - 1];
	leftdownpix = vibuff[ihres - 1];
	leftpix = vibuff[-1];

	rightuppix = vibuff[-ihres + 1];
	rightdownpix = vibuff[ihres + 1];
	rightpix = vibuff[1];

	uppix = vibuff[-ihres];
	downpix = vibuff[ihres];
	filtered = *vibuff;

	restore_two(&filtered, &leftuppix);
	restore_two(&filtered, &uppix);
	restore_two(&filtered, &rightuppix);

	restore_two(&filtered, &leftpix);
	restore_two(&filtered, &rightpix);

	restore_two(&filtered, &leftdownpix);
	restore_two(&filtered, &downpix);
	restore_two(&filtered, &rightdownpix);

	*r = filtered.i.r;
	*g = filtered.i.g;
	*b = filtered.i.b;

	if(*r < 0) *r = 0;
	else if(*r > 255) *r = 255;
	if(*g < 0) *g = 0;
	else if(*g > 255) *g = 255;
	if(*b < 0) *b = 0;
	else if(*b > 255) *b = 255;
}

// This is wrong, only the 5 upper bits are compared.
static inline void restore_two(color_t* filtered, color_t* neighbour)
{
	if (neighbour->i.r > filtered->i.r)
	{
		filtered->i.r += 1;
	}
	if (neighbour->i.r < filtered->i.r)
	{
		filtered->i.r -= 1;
	}
	if (neighbour->i.g > filtered->i.g)
	{
		filtered->i.g += 1;
	}
	if (neighbour->i.g < filtered->i.g)
	{
		filtered->i.g -= 1;
	}
	if (neighbour->i.b > filtered->i.b)
	{
		filtered->i.b += 1;
	}
	if (neighbour->i.b < filtered->i.b)
	{
		filtered->i.b -= 1;
	}
}

static inline void video_max(uint32_t* Pixels, uint8_t* max, uint32_t* enb)
{
	int i;
	int pos = 0;
	*enb = 0;
	for(i = 0; i < 7; i++)
	{
		if (Pixels[i] > Pixels[pos])
		{
			*enb += (1 << i);
			pos = i;
		}
		else if (Pixels[i] < Pixels[pos])
		{
			*enb += (1 << i);
		}
		else
		{
			pos = i;
		}
	}
	*max = Pixels[pos];
}

static inline uint32_t ge_two(uint32_t enb)
{
	if(enb & 1)
	{
		if(enb & 2)
			return 1;
		if(enb & 4)
			return 1;
		if(enb & 8)
			return 1;
		if(enb & 16)
			return 1;
		if(enb & 32)
			return 1;
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 2)
	{
		if(enb & 4)
			return 1;
		if(enb & 8)
			return 1;
		if(enb & 16)
			return 1;
		if(enb & 32)
			return 1;
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 4)
	{
		if(enb & 8)
			return 1;
		if(enb & 16)
			return 1;
		if(enb & 32)
			return 1;
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 8)
	{
		if(enb & 16)
			return 1;
		if(enb & 32)
			return 1;
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 16)
	{
		if(enb & 32)
			return 1;
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 32)
	{
		if(enb & 64)
			return 1;
		if(enb & 128)
			return 1;
		return 0;
	}
	else if(enb & 64)
	{
		if(enb & 128)
			return 1;
		return 0;
	}
	return 0;
}
#endif
