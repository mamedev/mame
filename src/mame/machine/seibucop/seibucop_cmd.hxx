/* Main COP functionality */

// notes about tables:
// (TABLENOTE1)
// in all but one case the upload table position (5-bits) is the SAME as the upper 5-bits of the 'trigger value'
// the exception to this rule is program 0x18 uploads on zeroteam
//  in this case you can see that the 'trigger' value upper bits are 0x0f, this would be potentially interesting if it were used (but it isn't)
//  18 - c480 ( 18) (  480) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk)
//  18 - 7c80 ( 0f) (  480) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (zeroteam, xsedae)

// It is unknown if the lower 11 bits uploaded as part of the 'trigger' value to the table are used during execution.
// When the actual trigger is written these bits can be different to the upload and for the written value we know they give extended
// meanings to the commands (eg. signs swapped in operations - for program 0x01 (0905) Zero Team writes 0904 (lowest bit different to uploaded
// value) to negate the logic

/*

## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
00 - 0205 ( 00) (  205) :  (188, 282, 082, b8e, 98e, 000, 000, 000)  6     ffeb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
00 - 0105 ( 00) (  105) :  (180, 2e0, 0a0, 000, 000, 000, 000, 000)  6     fffb   (zeroteamsr)
*/
void raiden2cop_device::execute_0205(int offset, UINT16 data)
{	
	int ppos =        m_host_space->read_dword(cop_regs[0] + 0x04 + offset * 4);
	int npos = ppos + m_host_space->read_dword(cop_regs[0] + 0x10 + offset * 4);
	int delta = (npos >> 16) - (ppos >> 16);
#if LOG_Move0205
	// ...
#endif
	m_host_space->write_dword(cop_regs[0] + 4 + offset * 4, npos);
	cop_write_word(cop_regs[0] + 0x1e + offset * 4, cop_read_word(cop_regs[0] + 0x1e + offset * 4) + delta);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
01 - 0905 ( 01) (  105) :  (194, 288, 088, 000, 000, 000, 000, 000)  6     fbfb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
01 - 0b05 ( 01) (  305) :  (180, 2e0, 0a0, 182, 2e0, 0c0, 000, 000)  6     ffdb   (zeroteamsr)
*/

// triggered with 0904 0905

void raiden2cop_device::execute_0904(int offset, UINT16 data)
{
#if LOG_Move0905
	printf("cmd %04x: %08x %08x [%08x]\n",data, m_host_space->read_dword(cop_regs[0] + 16 + offset * 4),m_host_space->read_dword(cop_regs[0] + 0x28 + offset * 4),cop_regs[0]);
#endif
	
	if (data&0x0001)
		m_host_space->write_dword(cop_regs[0] + 16 + offset * 4, m_host_space->read_dword(cop_regs[0] + 16 + offset * 4) + m_host_space->read_dword(cop_regs[0] + 0x28 + offset * 4));
	else /* X Se Dae and Zero Team uses this variant */
		m_host_space->write_dword(cop_regs[0] + 16 + offset * 4, m_host_space->read_dword(cop_regs[0] + 16 + offset * 4) - m_host_space->read_dword(cop_regs[0] + 0x28 + offset * 4));
}




/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
02 - 138e ( 02) (  38e) :  (984, aa4, d82, aa2, 39b, b9a, b9a, b9a)  5     bf7f   (legionna, heatbrl)
02 - 138e ( 02) (  38e) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9a)  5     bf7f   (cupsoc, grainbow, godzilla, denjinmk)
02 - 130e ( 02) (  30e) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9a)  5     bf7f   (raiden2, raidendx, zeroteam, xsedae)
*/

// triggered with 130e, 138e
void raiden2cop_device::execute_130e(int offset, UINT16 data)
{
	// this can't be right, or bits 15-12 from mask have different meaning ...
	execute_338e(offset, data);
}

void raiden2cop_device::LEGACY_execute_130e_cupsoc(int offset, UINT16 data)
{
	int dy = m_host_space->read_dword(cop_regs[1] + 4) - m_host_space->read_dword(cop_regs[0] + 4);
	int dx = m_host_space->read_dword(cop_regs[1] + 8) - m_host_space->read_dword(cop_regs[0] + 8);
	
	cop_status = 7;

	if (!dx) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else 
	{
		cop_angle = (int)(atan(double(dy) / double(dx)) * 128.0 / M_PI);
		if (dx < 0)
			cop_angle += 0x80;
		
		cop_angle &= 0xff;
	}

	m_LEGACY_r0 = dy;
	m_LEGACY_r1 = dx;

	//printf("%d %d %f %04x\n",dx,dy,atan(double(dy)/double(dx)) * 128 / M_PI,cop_angle);

	if (data & 0x80)
		cop_write_byte(cop_regs[0] + (0x34), cop_angle);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
03 - 1905 ( 03) (  105) :  (994, a88, 088, 000, 000, 000, 000, 000)  6     fbfb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
04 - 2288 ( 04) (  288) :  (f8a, b8a, 388, b9c, b9a, a9a, 000, 000)  5     f5df   (legionna, heatbrl)
04 - 2288 ( 04) (  288) :  (f8a, b8a, 388, b9a, b9a, a9a, 000, 000)  5     f5df   (cupsoc, grainbow, godzilla, denjinmk)
04 - 2208 ( 04) (  208) :  (f8a, b8a, 388, b9a, b9a, a9a, 000, 000)  5     f5df   (raiden2, raidendx, zeroteam, xsedae)
*/

// also triggered with 0x2208
void raiden2cop_device::execute_2288(int offset, UINT16 data)
{
	int dx = m_host_space->read_word(cop_regs[0] + 0x12);
	int dy = m_host_space->read_word(cop_regs[0] + 0x16);

	if (!dy) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = (int)(atan(double(dx) / double(dy)) * 128 / M_PI);
		if (dy < 0)
			cop_angle += 0x80;
	}

	if (data & 0x0080) {
		m_host_space->write_byte(cop_regs[0] + 0x34, cop_angle);
	}
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
05 - 2a05 ( 05) (  205) :  (9af, a82, 082, a8f, 18e, 000, 000, 000)  6     ebeb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

void raiden2cop_device::execute_2a05(int offset, UINT16 data)
{
	int delta = m_host_space->read_word(cop_regs[1] + 0x1e + offset * 4);
	m_host_space->write_dword(cop_regs[0] + 4 + 2 + offset * 4, m_host_space->read_word(cop_regs[0] + 4 + 2 + offset * 4) + delta);
	m_host_space->write_dword(cop_regs[0] + 0x1e + offset * 4, m_host_space->read_word(cop_regs[0] + 0x1e + offset * 4) + delta);
}


/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
06 - 338e ( 06) (  38e) :  (984, aa4, d82, aa2, 39c, b9c, b9c, a9a)  5     bf7f   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx)
06 - 330e ( 06) (  30e) :  (984, aa4, d82, aa2, 39c, b9c, b9c, a9a)  5     bf7f   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_338e(int offset, UINT16 data)
{
	int dx = m_host_space->read_dword(cop_regs[1] + 4) - m_host_space->read_dword(cop_regs[0] + 4);
	int dy = m_host_space->read_dword(cop_regs[1] + 8) - m_host_space->read_dword(cop_regs[0] + 8);

	if (!dy) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = (int)(atan(double(dx) / double(dy)) * 128 / M_PI);
		if (dy < 0)
			cop_angle += 0x80;
		
		cop_angle &= 0xff;
	}

#if LOG_Phytagoras
	printf("cmd %04x: dx = %d dy = %d angle = %02x %04x\n",data,dx,dy,cop_angle);
#endif

	if (data & 0x0080) {
		// TODO: byte or word?
		cop_write_byte(cop_regs[0] + 0x34, cop_angle);
	}
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
07 - 3bb0 ( 07) (  3b0) :  (f9c, b9c, b9c, b9c, b9c, b9c, b9c, 99c)  4     007f   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx,
07 - 3b30 ( 07) (  330) :  (f9c, b9c, b9c, b9c, b9c, b9c, b9c, 99c)  4     007f   (zeroteam, xsedae)
*/

// triggered with 0x39b0, 0x3b30, 0x3bb0

void raiden2cop_device::execute_3b30(int offset, UINT16 data)
{
	/* TODO: these are actually internally loaded via 0x130e command */
	int dx, dy;

	dx = m_host_space->read_dword(cop_regs[1] + 4) - m_host_space->read_dword(cop_regs[0] + 4);
	dy = m_host_space->read_dword(cop_regs[1] + 8) - m_host_space->read_dword(cop_regs[0] + 8);

	dx = dx >> 16;
	dy = dy >> 16;
	cop_dist = sqrt((double)(dx*dx + dy*dy));

#if LOG_Phytagoras
	printf("cmd %04x: dx = %d dy = %d dist = %08x \n",data,dx >> 16,dy >> 16,cop_dist);
#endif
	
	if (data & 0x0080)
		cop_write_word(cop_regs[0] + (data & 0x200 ? 0x3a : 0x38), cop_dist);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
08 - 42c2 ( 08) (  2c2) :  (f9a, b9a, b9c, b9c, b9c, 29c, 000, 000)  5     fcdd   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_42c2(int offset, UINT16 data)
{
	int div = cop_read_word(cop_regs[0] + (0x36));

#if LOG_Division
	printf("cmd %04x: div = %04x scale = %04x\n",data,div,cop_scale);
#endif

	if (!div)
	{
		cop_status |= 0x8000;
		cop_write_word(cop_regs[0] + (0x38), 0);
		return;
	}

	/* TODO: bits 5-6-15 */
	cop_status = 7;

#if LOG_Division
	printf("res = %04x dist %04x\n",(cop_dist << (5 - cop_scale)) / div,cop_dist);
	
//	if(div & 0x8000)
//		machine().debugger().debug_break();
#endif
	
	cop_write_word(cop_regs[0] + (0x38), (cop_dist << (5 - cop_scale)) / div);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
09 - 4aa0 ( 09) (  2a0) :  (f9a, b9a, b9c, b9c, b9c, 99b, 000, 000)  5     fcdd   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_4aa0(int offset, UINT16 data)
{
	int div = m_host_space->read_word(cop_regs[0] + (0x38));
	if (!div)
		div = 1;

	/* TODO: bits 5-6-15 */
	cop_status = 7;

	m_host_space->write_word(cop_regs[0] + (0x36), (cop_dist << (5 - cop_scale)) / div);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0a - 5105 ( 0a) (  105) :  (a80, 984, 082, 000, 000, 000, 000, 000)  5     fefb   (cupsoc, grainbow)
0a - 5205 ( 0a) (  205) :  (180, 2e0, 3a0, 0a0, 3a0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
0a - 5105 ( 0a) (  105) :  (180, 2e0, 0a0, 000, 000, 000, 000, 000)  6     fffb   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_5205(int offset, UINT16 data)
{
	m_host_space->write_dword(cop_regs[1], m_host_space->read_dword(cop_regs[0]));
}
/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0b - 5905 ( 0b) (  105) :  (9c8, a84, 0a2, 000, 000, 000, 000, 000)  5     fffb   (cupsoc, grainbow)
0b - 5a05 ( 0b) (  205) :  (180, 2e0, 3a0, 0a0, 3a0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
0b - 5a85 ( 0b) (  285) :  (180, 2e0, 0a0, 182, 2e0, 0c0, 3c0, 3c0)  6     ffdb   (zeroteam, xsedae)
*/
void raiden2cop_device::execute_5a05(int offset, UINT16 data)
{
	m_host_space->write_dword(cop_regs[1], m_host_space->read_dword(cop_regs[0]));
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0c - 6200 ( 0c) (  200) :  (380, 39a, 380, a80, 29a, 000, 000, 000)  8     f3e7   (legionn, heatbrla, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
0c - 6200 ( 0c) (  200) :  (3a0, 3a6, 380, aa0, 2a6, 000, 000, 000)  8     f3e7   (cupsoc)
*/
void raiden2cop_device::execute_6200(int offset, UINT16 data)
{
	int primary_reg = 0;
	int primary_offset = 0x34;

	UINT8 angle = cop_read_byte(cop_regs[primary_reg] + primary_offset);
	UINT16 flags = cop_read_word(cop_regs[primary_reg]);
	cop_angle_target &= 0xff;
	cop_angle_step &= 0xff;
	flags &= ~0x0004;
	int delta = angle - cop_angle_target;
	if (delta >= 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;
	if (delta < 0) {
		if (delta >= -cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle += cop_angle_step;
	}
	else {
		if (delta <= cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle -= cop_angle_step;
	}

	cop_write_word(cop_regs[primary_reg], flags);
	
	if (!m_host_endian)
		cop_write_byte(cop_regs[primary_reg] + primary_offset, angle);
	else // angle is a byte, but grainbow (cave mid-boss) is only happy with write-word, could be more endian weirdness, or it always writes a word?
		cop_write_word(cop_regs[primary_reg] + primary_offset, angle);

}


void raiden2cop_device::LEGACY_execute_6200(int offset, UINT16 data) // this is for cupsoc, different sequence, works on different registers
{
	int primary_reg = 1;
	int primary_offset = 0xc;

	UINT8 angle = cop_read_byte(cop_regs[primary_reg] + primary_offset);
	UINT16 flags = cop_read_word(cop_regs[primary_reg]);
	cop_angle_target &= 0xff;
	cop_angle_step &= 0xff;
	flags &= ~0x0004;
	int delta = angle - cop_angle_target;
	if (delta >= 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;
	if (delta < 0) {
		if (delta >= -cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle += cop_angle_step;
	}
	else {
		if (delta <= cop_angle_step) {
			angle = cop_angle_target;
			flags |= 0x0004;
		}
		else
			angle -= cop_angle_step;
	}

	cop_write_word(cop_regs[primary_reg], flags);

	if (!m_host_endian)
		cop_write_byte(cop_regs[primary_reg] + primary_offset, angle);
	else // angle is a byte, but grainbow (cave mid-boss) is only happy with write-word, could be more endian weirdness, or it always writes a word?
		cop_write_word(cop_regs[primary_reg] + primary_offset, angle);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0d - 6880 ( 0d) (  080) :  (b80, ba0, 000, 000, 000, 000, 000, 000)  a     fff3   (legionna, heatbrl, cupsoc, godzilla, denjinmk)
0d - 6980 ( 0d) (  180) :  (b80, ba0, 000, 000, 000, 000, 000, 000)  a     fff3   (grainbow, zeroteam, xsedae)
*/
void raiden2cop_device::LEGACY_execute_6980(int offset, UINT16 data)
{
	UINT8 offs;
	int abs_x, abs_y, rel_xy;

	offs = (offset & 3) * 4;

	/* TODO: I really suspect that following two are actually taken from the 0xa180 macro command then internally loaded */
	abs_x = m_host_space->read_word(cop_regs[0] + 8) - m_cop_sprite_dma_abs_x;
	abs_y = m_host_space->read_word(cop_regs[0] + 4) - m_cop_sprite_dma_abs_y;
	rel_xy = m_host_space->read_word(m_cop_sprite_dma_src + 4 + offs);

	//if(rel_xy & 0x0706)
	//  printf("sprite rel_xy = %04x\n",rel_xy);

	if (rel_xy & 1)
		m_host_space->write_word(cop_regs[4] + offs + 4, 0xc0 + abs_x - (rel_xy & 0xf8));
	else
		m_host_space->write_word(cop_regs[4] + offs + 4, (((rel_xy & 0x78) + (abs_x)-((rel_xy & 0x80) ? 0x80 : 0))));

	m_host_space->write_word(cop_regs[4] + offs + 6, (((rel_xy & 0x7800) >> 8) + (abs_y)-((rel_xy & 0x8000) ? 0x80 : 0)));
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0e - 7100 ( 0e) (  100) :  (b80, a80, b80, 000, 000, 000, 000, 000)  8     fdfd   (zeroteam, xsedae)
*/

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
0f - 7905 ( 0f) (  105) :  (1a2, 2c2, 0a2, 000, 000, 000, 000, 000)  6     fffb   (cupsoc, grainbow)
0f - 7e05 ( 0f) (  605) :  (180, 282, 080, 180, 282, 000, 000, 000)  6     fffb   (raidendx)
*/

void raiden2cop_device::execute_7e05(int offset, UINT16 data) // raidendx
{
	m_host_space->write_byte(0x470, m_host_space->read_byte(cop_regs[4]));
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
10 - 8100 ( 10) (  100) :  (b9a, b88, 888, 000, 000, 000, 000, 000)  7     fdfb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/

void raiden2cop_device::execute_8100(int offset, UINT16 data)
{
	int raw_angle = (cop_read_word(cop_regs[0] + (0x34)) & 0xff);
	double angle = raw_angle * M_PI / 128;
	double amp = (65536 >> 5)*(cop_read_word(cop_regs[0] + (0x36)) & 0xff);
	int res;
	// TODO: up direction needs to be doubled, happens on bootleg too, why is that?
	if (raw_angle == 0xc0)
		amp *= 2;
	res = int(amp*sin(angle)) << cop_scale;
	m_host_space->write_dword(cop_regs[0] + 16, res);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
11 - 8900 ( 11) (  100) :  (b9a, b8a, 88a, 000, 000, 000, 000, 000)  7     fdfb   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
*/
void raiden2cop_device::execute_8900(int offset, UINT16 data)
{
	int raw_angle = (cop_read_word(cop_regs[0] + (0x34)) & 0xff);
	double angle = raw_angle * M_PI / 128;
	double amp = (65536 >> 5)*(cop_read_word(cop_regs[0] + (0x36)) & 0xff);
	int res;
	// TODO: left direction needs to be doubled, happens on bootleg too, why is that?
	if (raw_angle == 0x80)
		amp *= 2;
	res = int(amp*cos(angle)) << cop_scale;
	m_host_space->write_dword(cop_regs[0] + 20, res);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
12 - 9180 ( 12) (  180) :  (b80, b94, b94, 894, 000, 000, 000, 000)  7     f8f7   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk)
12 - 9100 ( 12) (  100) :  (b80, b94, 894, 000, 000, 000, 000, 000)  7     fefb   (raiden2, raidendx)
12 - 9100 ( 12) (  100) :  (b80, b94, b94, 894, 000, 000, 000, 000)  7     f8f7   (zeroteam, xsedae)
*/
// Unused code suggests this may be an alternate sine function: the longword result at cop_regs[0] + 0x28 is doubled when the angle is 0xC0.

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
13 - 9980 ( 13) (  180) :  (b80, b96, b96, 896, 000, 000, 000, 000)  7     f8f7   (legionna, heatbrl)
13 - 9980 ( 13) (  180) :  (b80, b94, b94, 896, 000, 000, 000, 000)  7     f8f7   (cupsoc, grainbow, godzilla, denjinmk)
13 - 9900 ( 13) (  100) :  (b80, b94, 896, 000, 000, 000, 000, 000)  7     fefb   (raiden2, raidendx)
13 - 9900 ( 13) (  100) :  (b80, b94, b94, 896, 000, 000, 000, 000)  7     f8f7   (zeroteam, xsedae)
*/
// Unused code suggests this may be an alternate cosine function: the longword result at cop_regs[0] + 0x2C is doubled when the angle is 0x80.

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
14 - a180 ( 14) (  180) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     ffff   (legionna, cupsoc, godzilla, denjinmk)
14 - a100 ( 14) (  100) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     ffff   (heatbrl, zeroteam, xsedae)
14 - a180 ( 14) (  180) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     02ff   (grainbow)
14 - a100 ( 14) (  100) :  (b80, b82, b84, b86, 000, 000, 000, 000)  0     00ff   (raiden2, raidendx)
*/

// the last value (ffff / 02ff / 00ff depending on game) might be important here as they've been intentionally changed for the different games
void raiden2cop_device::execute_a100(int offset, UINT16 data)
{
	cop_collision_read_pos(0, cop_regs[0], data & 0x0080);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
15 - a980 ( 15) (  180) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     ffff   (legionna, cupsoc, godzilla, denjinmk)
15 - a900 ( 15) (  100) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     ffff   (heatbrl, zeroteam), xsedae
15 - a980 ( 15) (  180) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     02ff   (grainbow)
15 - a900 ( 15) (  100) :  (ba0, ba2, ba4, ba6, 000, 000, 000, 000)  f     00ff   (raiden2, raidendx)*/
void raiden2cop_device::execute_a900(int offset, UINT16 data)
{
	cop_collision_read_pos(1, cop_regs[1], data & 0x0080);
}


/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
16 - b100 ( 16) (  100) :  (b40, bc0, bc2, 000, 000, 000, 000, 000)  9     ffff   (legionna, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
16 - b080 ( 16) (  080) :  (b40, bc0, bc2, 000, 000, 000, 000, 000)  9     ffff   (heatbrl)
*/
void raiden2cop_device::execute_b100(int offset, UINT16 data)
{
	cop_collision_update_hitbox(data, 0, cop_regs[2]);
}




/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
17 - b900 ( 17) (  100) :  (b60, be0, be2, 000, 000, 000, 000, 000)  6     ffff   (legionna, cupsoc, grainbow, godzilla, denjinmk, raiden2, raidendx, zeroteam, xsedae)
17 - b880 ( 17) (  080) :  (b60, be0, be2, 000, 000, 000, 000, 000)  6     ffff   (heatbrl)
*/
void raiden2cop_device::execute_b900(int offset, UINT16 data)
{
	cop_collision_update_hitbox(data, 1, cop_regs[3]);
}



/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
18 - c480 ( 18) (  480) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (legionna, heatbrl, cupsoc, grainbow, godzilla, denjinmk)
18 - 7c80 ( 0f) (  480) :  (080, 882, 000, 000, 000, 000, 000, 000)  a     ff00   (zeroteam, xsedae)
*/

void raiden2cop_device::LEGACY_execute_c480(int offset, UINT16 data)
{
	UINT8 offs;

	offs = (offset & 3) * 4;

	m_host_space->write_word(cop_regs[4] + offs + 0, m_host_space->read_word(m_cop_sprite_dma_src + offs) + (m_cop_sprite_dma_param & 0x3f));
	//m_host_space->write_word(cop_regs[4] + offs + 2,m_host_space->read_word(m_cop_sprite_dma_src+2 + offs));

}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
19 - cb8f ( 19) (  38f) :  (984, aa4, d82, aa2, 39b, b9a, b9a, a9f)  5     bf7f   (cupsoc, grainbow)
*/

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1a - d104 ( 1a) (  104) :  (ac2, 9e0, 0a2, 000, 000, 000, 000, 000)  5     fffb   (cupsoc, grainbow)
*/
void raiden2cop_device::LEGACY_execute_d104(int offset, UINT16 data)
{
	UINT16 *ROM = (UINT16 *)m_host_space->machine().root_device().memregion("maincpu")->base();
	UINT32 rom_addr = (m_cop_rom_addr_hi << 16 | m_cop_rom_addr_lo);
	UINT16 rom_data = ROM[rom_addr / 2];

	/* writes to some unemulated COP registers, then puts the result in here, adding a parameter taken from ROM */
	//m_host_space->write_word(cop_regs[0]+(0x44 + offset * 4), rom_data);

	logerror("%04x%04x %04x %04x\n", m_cop_rom_addr_hi, m_cop_rom_addr_lo, m_cop_precmd, rom_data);
}
/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1b - dde5 ( 1b) (  5e5) :  (f80, aa2, 984, 0c2, 000, 000, 000, 000)  5     7ff7   (cupsoc, grainbow)
*/

void raiden2cop_device::LEGACY_execute_dde5(int offset, UINT16 data)
{
	UINT8 offs;
	int div;
	INT16 dir_offset;
	//              INT16 offs_val;

	/* TODO: [4-7] could be mirrors of [0-3] (this is the only command so far that uses 4-7 actually)*/
	/* ? 0 + [4] */
	/* sub32 4 + [5] */
	/* write16h 8 + [4] */
	/* addmem16 4 + [6] */

	// these two are obvious ...
	// 0xf x 16 = 240
	// 0x14 x 16 = 320
	// what are these two instead? scale factor? offsets? (edit: offsets to apply from the initial sprite data)
	// 0xfc69 ?
	// 0x7f4 ?
	//printf("%08x %08x %08x %08x %08x %08x %08x\n",cop_regs[0],cop_regs[1],cop_regs[2],cop_regs[3],cop_regs[4],cop_regs[5],cop_regs[6]);

	offs = (offset & 3) * 4;

	div = m_host_space->read_word(cop_regs[4] + offs);
	dir_offset = m_host_space->read_word(cop_regs[4] + offs + 8);
	//              offs_val = m_host_space.read_word(cop_regs[3] + offs);
	//420 / 180 = 500 : 400 = 30 / 50 = 98 / 18

	/* TODO: this probably trips a cop status flag */
	if (div == 0) { div = 1; }


	m_host_space->write_word((cop_regs[6] + offs + 4), ((m_host_space->read_word(cop_regs[5] + offs + 4) + dir_offset) / div));
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1c - e38e ( 1c) (  38e) :  (984, ac4, d82, ac2, 39b, b9a, b9a, a9a)  5     b07f   (cupsoc, grainbow)
1c - e105 ( 1c) (  105) :  (a88, 994, 088, 000, 000, 000, 000, 000)  5     06fb   (zeroteam, xsedae)
*/
// controls GK position, aligned to the ball position
void raiden2cop_device::LEGACY_execute_e30e(int offset, UINT16 data)
{	
	int dy = m_host_space->read_dword(cop_regs[2] + 4) - m_host_space->read_dword(cop_regs[0] + 4);
	int dx = m_host_space->read_dword(cop_regs[2] + 8) - m_host_space->read_dword(cop_regs[0] + 8);

	
	cop_status = 7;
	if (!dx)
	{
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = (int)(atan(double(dy) / double(dx)) * 128.0 / M_PI);
		if (dx < 0)
			cop_angle += 0x80;

		cop_angle &= 0xff;
	}
	
#if LOG_Phytagoras
	printf("cmd %04x: dx = %d dy = %d angle = %02x %04x\n",data,dx,dy,cop_angle);
#endif

	// TODO: byte or word?
	if (data & 0x0080)
		cop_write_byte(cop_regs[0] + 0x34, cop_angle);
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1d - eb8e ( 1d) (  38e) :  (984, ac4, d82, ac2, 39b, b9a, b9a, a9f)  5     b07f   (cupsoc, grainbow)
1d - ede5 ( 1d) (  5e5) :  (f88, a84, 986, 08a, 000, 000, 000, 000)  5     05f7   (zeroteam, xsedae)
*/

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1e - f105 ( 1e) (  105) :  (a88, 994, 088, 000, 000, 000, 000, 000)  5     fefb   (cupsoc, grainbow)
1e - f205 ( 1e) (  205) :  (182, 2e0, 3c0, 0c0, 3c0, 000, 000, 000)  6     fff7   (raiden2, raidendx)
1e - f790 ( 1e) (  790) :  (f80, b84, b84, b84, b84, b84, b84, b84)  4     00ff   (zeroteam, xsedae)
*/

void raiden2cop_device::execute_f205(int offset, UINT16 data)
{
	m_host_space->write_dword(cop_regs[2], m_host_space->read_dword(cop_regs[0]+4));
}

/*
## - trig (up5) (low11) :  (sq0, sq1, sq2, sq3, sq4, sq5, sq6, sq7)  valu  mask
1f - fc84 ( 1f) (  484) :  (182, 280, 000, 000, 000, 000, 000, 000)  6     00ff   (zeroteam, xsedae)
*/


/*
[:raiden2cop] COPDIS: f105 s=f0 f1=0 l=3 f2=05 5 fefb f0 a88 15.0.08 [:raiden2cop] sub32 10(r0)
[:raiden2cop] COPDIS: f105 s=f0 f1=0 l=3 f2=05 5 fefb f1 994 13.0.14 [:raiden2cop] write16h 28(r0)
[:raiden2cop] COPDIS: f105 s=f0 f1=0 l=3 f2=05 5 fefb f2 088 01.0.08 [:raiden2cop] addmem32 10(r0)
*/
// seibu cup soccer, before cosine (ball dribbling actually?)
void raiden2cop_device::execute_f105(int offset, UINT16 data)
{
	// ...
}

#ifdef UNUSED_COMMANDS

// For reference only, will be nuked at some point

void raiden2cop_device::LEGACY_execute_130e(int offset, UINT16 data)
{
	int dy =  m_host_space->read_dword(cop_regs[1] + 4) - m_host_space->read_dword(cop_regs[0] + 4);
	int dx = m_host_space->read_dword(cop_regs[1] + 8) - m_host_space->read_dword(cop_regs[0] + 8);

	cop_status = 7;
	if (!dx) {
		cop_status |= 0x8000;
		cop_angle = 0;
	}
	else {
		cop_angle = (int)(atan(double(dy) / double(dx)) * 128.0 / M_PI);
		if (dx < 0)
			cop_angle += 0x80;
	}

	m_LEGACY_r0 = dy;
	m_LEGACY_r1 = dx;

	if (data & 0x80)
		m_host_space->write_word(cop_regs[0] + (0x34 ^ 2), cop_angle);
}

void raiden2cop_device::LEGACY_execute_3b30(int offset, UINT16 data)
{
	int dy = m_LEGACY_r0;
	int dx = m_LEGACY_r1;

	dx >>= 16;
	dy >>= 16;
	cop_dist = sqrt((double)(dx*dx + dy*dy));

	if (data & 0x80)
		m_host_space->write_word(cop_regs[0] + (0x38), cop_dist);
}

void raiden2cop_device::LEGACY_execute_42c2(int offset, UINT16 data)
{
	int dy = m_LEGACY_r0;
	int dx = m_LEGACY_r1;
	int div = m_host_space->read_word(cop_regs[0] + (0x36 ^ 2));
	int res;
	int cop_dist_raw;

	// divide by zero?
	if (!div)
	{
		// No emulation error here: heatbrl specifically tests this
		cop_status |= 0x8000;
		res = 0;
	}
	else
	{
		/* TODO: calculation of this one should occur at 0x3b30/0x3bb0 I *think* */
		/* TODO: recheck if cop_scale still masks at 3 with this command */
		dx >>= 11 + cop_scale;
		dy >>= 11 + cop_scale;
		cop_dist_raw = sqrt((double)(dx*dx + dy*dy));

		res = cop_dist_raw;
		res /= div;

		cop_dist = (1 << (5 - cop_scale)) / div;

		/* TODO: bits 5-6-15 */
		cop_status = 7;
	}

	m_host_space->write_word(cop_regs[0] + (0x38 ^ 2), res);
}

// used by seibu cup soccer, not sure if right so left out
void raiden2cop_device::execute_5105(int offset, UINT16 data)
{
	int res = m_host_space->read_dword(cop_regs[0]) +  m_host_space->read_dword(cop_regs[0]+8);
	m_host_space->write_dword(cop_regs[0]+4, res);
}

/*
[:raiden2cop] COPDIS: 5905 s=58 f1=0 l=3 f2=05 5 fffb 58 9c8 13.2.08 [:raiden2cop] write16h 10(r2)
[:raiden2cop] COPDIS: 5905 s=58 f1=0 l=3 f2=05 5 fffb 59 a84 15.0.04 [:raiden2cop] sub32 8(r0)
[:raiden2cop] COPDIS: 5905 s=58 f1=0 l=3 f2=05 5 fffb 5a 0a2 01.1.02 [:raiden2cop] addmem32 4(r1)
*/
void raiden2cop_device::execute_5905(int offset, UINT16 data)
{
	int res = m_host_space->read_dword(cop_regs[2]+10 + offset*4) - m_host_space->read_dword(cop_regs[0]+8 + offset*4);
	m_host_space->write_dword(cop_regs[1]+4 + offset *4, res);
}

#endif
