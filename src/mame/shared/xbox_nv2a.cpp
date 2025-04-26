// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#include "emu.h"
#include "xbox_nv2a.h"

#include "video/rgbutil.h"

#include "bitmap.h"

#include <bitset>
#include <cfloat>

#define LOG_NV2A_VERBOSE (1U << 1)

#ifdef LOG_NV2A
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif

#define LOG_OUTPUT_FUNC machine().logerror
#include "logmacro.h"


#define DEBUG_CHECKS // enable for debugging

char const *const vertex_program_disassembler::srctypes[] = { "??", "Rn", "Vn", "Cn" };
char const *const vertex_program_disassembler::scaops[] = { "NOP", "IMV", "RCP", "RCC", "RSQ", "EXP", "LOG", "LIT", "???", "???", "???", "???", "???", "???", "???", "???", "???" };
int const vertex_program_disassembler::scapar2[] = { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char const *const vertex_program_disassembler::vecops[] = { "NOP", "MOV", "MUL", "ADD", "MAD", "DP3", "DPH", "DP4", "DST", "MIN", "MAX", "SLT", "SGE", "ARL", "???", "???", "???" };
int const vertex_program_disassembler::vecpar2[] = { 0, 4, 6, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 4, 0, 0, 0 };
char const *const vertex_program_disassembler::vecouts[] = { "oPos", "???", "???", "oD0", "oD1", "oFog", "oPts", "oB0", "oB1", "oT0", "oT1", "oT2", "oT3" };
char const vertex_program_disassembler::compchar[] = { 'x', 'y', 'z', 'w' };

/*
Each vertex program instruction is a 128 bit word made of the fields:
d         f
w   b     i
o   i     e
r   t     l
d   s     d
+-+-----+-------
|0|31-0 |not used
+-+-----+-------
| |31-29|not used
| +-----+-------
| |28-25|scalar operation
| +-----+-------
| |24-21|vectorial operation
| +-----+-------
| |20-13|index for source constant C[]
| +-----+-------
| |12-9 |input vector index
| +-----+-------
|1|  8  |parameter A:sign
| +-----+-------
| | 7-6 |parameter A:swizzle x
| +-----+-------
| | 5-4 |parameter A:swizzle y
| +-----+-------
| | 3-2 |parameter A:swizzle z
| +-----+-------
| | 1-0 |parameter A:swizzle w
|-+-----+-------
| |31-28|parameter A:parameter Rn index
| +-----+-------
| |27-26|parameter A:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 25  |parameter B:sign
| +-----+-------
| |24-23|parameter B:swizzle x
| +-----+-------
| |22-21|parameter B:swizzle y
| +-----+-------
| |20-19|parameter B:swizzle z
| +-----+-------
|2|18-17|parameter B:swizzle w
| +-----+-------
| |16-13|parameter B:parameter Rn index
| +-----+-------
| |12-11|parameter B:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 10  |parameter C:sign
| +-----+-------
| | 9-8 |parameter C:swizzle x
| +-----+-------
| | 7-6 |parameter C:swizzle y
| +-----+-------
| | 5-4 |parameter C:swizzle z
| +-----+-------
| | 3-2 |parameter C:swizzle w
| +-----+-------
| | 1-0 |
|-+     |parameter C:parameter Rn index
| |31-30|
| +-----+-------
| |29-28|parameter C:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| |27-24|output Rn mask from vectorial operation
| +-----+-------
| |23-20|output Rn index from vectorial operation
| +-----+-------
| |19-16|output Rn mask from scalar operation
| +-----+-------
|3|15-12|output vector write mask
| +-----+-------
| | 11  |1:output is output vector 0:output is constant C[]
| +-----+-------
| |10-3 |output vector/constant index
| +-----+-------
| |  2  |0:output Rn from vectorial operation 1:output Rn from scalar operation
| +-----+-------
| |  1  |1:add a0x to index for source constant C[]
| +-----+-------
| |  0  |1:end of program
+-+-----+-------
Each vertex program instruction can generate up to three destination values using up to three source values.
The first possible destination is to Rn from a vectorial operation.
The second possible destination is to a vertex shader output or C[n] from a vectorial or scalar operation.
The third possible destination is to Rn from a scalar operation.
*/
void vertex_program_disassembler::decodefields(unsigned int *dwords, int offset, fields &decoded)
{
	unsigned int srcbits[3];
	int a;

	srcbits[0] = ((dwords[1 + offset] & 0x1ff) << 6) | (dwords[2 + offset] >> 26);
	srcbits[1] = (dwords[2 + offset] >> 11) & 0x7fff;
	srcbits[2] = ((dwords[2 + offset] & 0x7ff) << 4) | (dwords[3 + offset] >> 28);
	decoded.ScaOperation = (int)(dwords[1 + offset] >> 25) & 0xf;
	decoded.VecOperation = (int)(dwords[1 + offset] >> 21) & 0xf;
	decoded.SourceConstantIndex = (int)(dwords[1 + offset] >> 13) & 0xff;
	decoded.InputIndex = (int)(dwords[1 + offset] >> 9) & 0xf;
	for (a = 0; a < 3; a++)
	{
		decoded.src[a].Sign = (int)(srcbits[a] >> 14) & 1;
		decoded.src[a].SwizzleX = (int)(srcbits[a] >> 12) & 3;
		decoded.src[a].SwizzleY = (int)(srcbits[a] >> 10) & 3;
		decoded.src[a].SwizzleZ = (int)(srcbits[a] >> 8) & 3;
		decoded.src[a].SwizzleW = (int)(srcbits[a] >> 6) & 3;
		decoded.src[a].TempIndex = (int)(srcbits[a] >> 2) & 0xf;
		decoded.src[a].ParameterType = (int)(srcbits[a] >> 0) & 3;
	}

	decoded.VecTempWriteMask = (int)(dwords[3 + offset] >> 24) & 0xf;
	decoded.VecTempIndex = (int)(dwords[3 + offset] >> 20) & 0xf;
	decoded.ScaTempWriteMask = (int)(dwords[3 + offset] >> 16) & 0xf;
	decoded.OutputWriteMask = (int)(dwords[3 + offset] >> 12) & 0xf;
	decoded.OutputSelect = (int)(dwords[3 + offset] >> 11) & 0x1;
	decoded.OutputIndex = (int)(dwords[3 + offset] >> 3) & 0xff;
	decoded.MultiplexerControl = (int)(dwords[3 + offset] >> 2) & 0x1;
	decoded.Usea0x = (int)(dwords[3 + offset] >> 1) & 0x1;
	decoded.EndOfProgram = (int)(dwords[3 + offset] >> 0) & 0x1;
}

int vertex_program_disassembler::disassemble_mask(int mask, char *s)
{
	int l;

	*s = 0;
	if (mask == 15)
		return 0;
	s[0] = '.';
	l = 1;
	if ((mask & 8) != 0)
	{
		s[l] = 'x';
		l++;
	}
	if ((mask & 4) != 0)
	{
		s[l] = 'y';
		l++;
	}
	if ((mask & 2) != 0)
	{
		s[l] = 'z';
		l++;
	}
	if ((mask & 1) != 0)
	{
		s[l] = 'w';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_swizzle(sourcefields f, char *s)
{
	int t, l;

	t = 4;
	if (f.SwizzleW == 3)
	{
		t = t - 1;
		if (f.SwizzleZ == 2)
		{
			t = t - 1;
			if (f.SwizzleY == 1)
			{
				t = t - 1;
				if (f.SwizzleX == 0)
				{
					t = t - 1;
				}
			}
		}
	}
	*s = 0;
	if (t == 0)
		return 0;
	s[0] = '.';
	l = 1;
	if (t > 0)
	{
		s[l] = compchar[f.SwizzleX];
		l++;
	}
	if (t > 1)
	{
		s[l] = compchar[f.SwizzleY];
		l++;
	}
	if (t > 2)
	{
		s[l] = compchar[f.SwizzleZ];
		l++;
	}
	if (t > 3)
	{
		s[l] = compchar[f.SwizzleW];
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_source(sourcefields f, fields fi, char *s)
{
	int l;

	if (f.ParameterType == 0) {
		strcpy(s, ",???");
		return 4;
	}
	l = 0;
	if (f.Sign != 0) {
		s[l] = '-';
		l++;
	}
	if (f.ParameterType == 1) {
		s[l] = 'r';
		l = l + 1 + sprintf(s + l + 1, "%d", f.TempIndex);
	}
	else if (f.ParameterType == 2){
		s[l] = 'v';
		l = l + 1 + sprintf(s + l + 1, "%d", fi.InputIndex);
	}
	else
	{
		if (fi.Usea0x != 0)
		{
			if (fi.SourceConstantIndex >= 96) {
				strcpy(s + l, "c[");
				l = l + 2;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				strcpy(s + l, "+a0.x]");
				l = l + 6;
			}
			else {
				strcpy(s + l, "c[a0.x");
				l = l + 6;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				s[l] = ']';
				l++;
			}
		}
		else {
			strcpy(s + l, "c[");
			l = l + 2;
			l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
			s[l] = ']';
			l++;
		}
	}
	l = l + disassemble_swizzle(f, s + l);
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_output(fields f, char *s)
{
	int l;

	if (f.OutputSelect == 1) {
		strcpy(s, vecouts[f.OutputIndex]);
		return strlen(s);
	}
	else {
		strcpy(s, "c[");
		l = 2;
		l = l + sprintf(s + l, "%d", f.OutputIndex - 96);
		s[l] = ']';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::output_types(fields f, int *o)
{
	o[0] = o[1] = o[2] = o[3] = o[4] = o[5] = 0;
	if ((f.VecOperation > 0) && (f.VecTempWriteMask != 0))
		o[0] = 1;
	if ((f.VecOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 0))
		o[1] = 1;
	if ((f.ScaOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 1))
		o[2] = 1;
	if ((f.ScaOperation > 0) && (f.ScaTempWriteMask != 0))
		o[3] = 1;
	if (f.VecOperation == 13)
		o[4] = 1;
	if (f.EndOfProgram == 1)
		o[5] = 1;
	return o[0] + o[1] + o[2] + o[3] + o[4] + o[5];
}

int vertex_program_disassembler::disassemble(unsigned int *instruction, char *line)
{
	int b, p;
	char *c;

	if (state == 0) {
		decodefields(instruction, 0, f);
		output_types(f, o);
		state = 1;
	}
	if (o[0] != 0)
	{
		o[0] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", f.VecTempIndex);
		c = c + disassemble_mask(f.VecTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				c[0] = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[1] != 0)
	{
		o[1] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[2] != 0)
	{
		o[2] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[3] != 0)
	{
		if (f.VecOperation > 0)
			b = 1;
		else
			b = f.VecTempIndex;
		o[3] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", b);
		c = c + disassemble_mask(f.ScaTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[4] != 0)
	{
		o[4] = 0;
		c = line;
		c = c + sprintf(c, "MOV a0.x,");
		c = c + disassemble_source(f.src[0], f, c);
		*c = 0;
		return 1;
	}
	if (o[5] != 0)
	{
		o[5] = 0;
		strcpy(line, "END");
		return 1;
	}
	state = 0;
	return 0;
}

vertex_program_simulator::vertex_program_simulator()
{
	for (auto & elem : op)
		elem.modified = 0;
	initialize_constants();
}

void vertex_program_simulator::set_data(vertex_nv *in, vertex_nv *out)
{
	input = in;
	output = out;
}

void vertex_program_simulator::reset()
{
	ip = 0;
	a0x = 0;
	initialize_outputs();
	initialize_temps();
}

void vertex_program_simulator::decode_instruction(int address)
{
	instruction *i;

	i = &op[address];
	i->d.NegateA = i->i[1] & (1 << 8);
	i->d.ParameterTypeA = (i->i[2] >> 26) & 3;
	i->d.TempIndexA = (i->i[2] >> 28) & 15;
	i->d.SwizzleA[0] = (i->i[1] >> 6) & 3;
	i->d.SwizzleA[1] = (i->i[1] >> 4) & 3;
	i->d.SwizzleA[2] = (i->i[1] >> 2) & 3;
	i->d.SwizzleA[3] = (i->i[1] >> 0) & 3;
	i->d.NegateB = i->i[2] & (1 << 25);
	i->d.ParameterTypeB = (i->i[2] >> 11) & 3;
	i->d.TempIndexB = (i->i[2] >> 13) & 15;
	i->d.SwizzleB[0] = (i->i[2] >> 23) & 3;
	i->d.SwizzleB[1] = (i->i[2] >> 21) & 3;
	i->d.SwizzleB[2] = (i->i[2] >> 19) & 3;
	i->d.SwizzleB[3] = (i->i[2] >> 17) & 3;
	i->d.NegateC = i->i[2] & (1 << 10);
	i->d.ParameterTypeC = (i->i[3] >> 28) & 3;
	i->d.TempIndexC = ((i->i[2] & 3) << 2) + (i->i[3] >> 30);
	i->d.SwizzleC[0] = (i->i[2] >> 8) & 3;
	i->d.SwizzleC[1] = (i->i[2] >> 6) & 3;
	i->d.SwizzleC[2] = (i->i[2] >> 4) & 3;
	i->d.SwizzleC[3] = (i->i[2] >> 2) & 3;
	i->d.VecOperation = (VectorialOperation)((i->i[1] >> 21) & 15);
	i->d.ScaOperation = (ScalarOperation)((i->i[1] >> 25) & 15);
	i->d.OutputWriteMask = ((i->i[3] >> 12) & 15);
	i->d.MultiplexerControl = i->i[3] & 4; // 0 : output Rn from vectorial operation 4 : output Rn from scalar operation
	i->d.VecTempIndex = (i->i[3] >> 20) & 15;
	i->d.OutputIndex = (i->i[3] >> 3) & 255;
	i->d.OutputSelect = i->i[3] & 0x800;
	i->d.VecTempWriteMask = (i->i[3] >> 24) & 15;
	i->d.ScaTempWriteMask = (i->i[3] >> 16) & 15;
	i->d.InputIndex = (i->i[1] >> 9) & 15;
	i->d.SourceConstantIndex = (i->i[1] >> 13) & 255;
	i->d.Usea0x = i->i[3] & 2;
	i->d.EndOfProgram = i->i[3] & 1;
}

int vertex_program_simulator::step()
{
	int p1, p2;
	float tmp[3 * 4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float tmpv[4] = { 0, 0, 0, 0};
	float tmps[4] = { 0, 0, 0, 0};
	instruction::decoded *d;

#if 0 // useful while debugging to see what instrucion is being executed
	static int debugvpi = 0;
	char disbuffer[256];
	if (debugvpi) {
		char *pp;
		vertex_program_disassembler vdis;

		pp = disbuffer;
		while (vdis.disassemble(op[ip].i, pp) != 0) {
			pp = pp + strlen(pp);
			*pp = '\n';
			pp++;
			*pp = 0;
			printf("%s", disbuffer);
		}
	}
#endif

	if (op[ip].modified)
		decode_instruction(ip);
	d = &(op[ip].d);
	// prepare inputs
	//  input A
	generate_input(&tmp[0], d->NegateA, d->ParameterTypeA, d->TempIndexA, d->SwizzleA);
	//  input B
	generate_input(&tmp[4], d->NegateB, d->ParameterTypeB, d->TempIndexB, d->SwizzleB);
	//  input C
	generate_input(&tmp[8], d->NegateC, d->ParameterTypeC, d->TempIndexC, d->SwizzleC);
	// compute 2 instructions
	//  vectorial
	compute_vectorial_operation(tmpv, d->VecOperation, tmp);
	//  scalar
	compute_scalar_operation(tmps, d->ScaOperation, tmp);
	// assign destinations
	if (d->VecOperation != VecNOP) {
		if (d->VecOperation == VecARL)
			//o[4] = 1;
			a0x = (int)tmpv[0];
		else {
			if (d->VecTempWriteMask != 0) { // assign to Rn
				//o[0] = 1;
				assign_register(d->VecTempIndex, tmpv, d->VecTempWriteMask);
			}
			if ((d->OutputWriteMask != 0) && (d->MultiplexerControl == 0)) {
				//o[1] = 1;
				if (d->OutputSelect) { // assign to output
					assign_output(d->OutputIndex, tmpv, d->OutputWriteMask);
					// remeber, output position == r12
					if (d->OutputIndex == 0)
						for (p1 = 0; p1 < 4; p1++) {
							r_register[12].fv[p1] = output->attribute[d->OutputIndex].fv[p1];
						}
				}
				else { // assign to constant
					assign_constant(d->OutputIndex, tmpv, d->OutputWriteMask);
				}
			}
		}
	}
	if (d->ScaOperation != ScaNOP) {
		if (d->ScaTempWriteMask != 0) { // assign to Rn
			//o[3] = 1;
			if (d->VecOperation != VecNOP)
				p2 = 1;
			else
				p2 = d->VecTempIndex;
			assign_register(p2, tmps, d->ScaTempWriteMask);
		}
		if ((d->OutputWriteMask != 0) && (d->MultiplexerControl != 0)) { // assign to output
			//o[2] = 1;
			assign_output(d->OutputIndex, tmps, d->OutputWriteMask);
			// remeber, output position == r12
			if (d->OutputIndex == 0) {
				for (p1 = 0; p1 < 4; p1++) {
					r_register[12].fv[p1] = output->attribute[d->OutputIndex].fv[p1];
				}
			}
		}
	}
	return d->EndOfProgram;
}

void vertex_program_simulator::execute()
{
	int c;

	c = 0;
	do {
		c = step();
		ip++;
	} while (c == 0);
}

void vertex_program_simulator::jump(int address)
{
	ip = address;
}

void vertex_program_simulator::process(int address, vertex_nv *in, vertex_nv *out, int count)
{
#if 0 // useful while debugging to see what is being executed
	static int debugvps = 0;
	if (debugvps) {
		FILE *f;
		char *pp;
		vertex_program_disassembler vdis;
		char disbuffer[128];

		debugvps--;
		if ((f = fopen("vertexshader_debug.txt", "wb")) != nullptr) {
			jump(address);
			fprintf(f, "SHADER:\n");
			for (int t = 0; t < 128; t++) {
				pp = disbuffer;
				while (vdis.disassemble(op[ip + t].i, pp) != 0) {
					pp = pp + strlen(pp);
					*pp = '\n';
					pp++;
					*pp = 0;
				}
				fprintf(f, "%08X %08X %08X %s", op[ip + t].i[1], op[ip + t].i[2], op[ip + t].i[3], disbuffer);
				if (op[ip + t].i[3] & 1)
					break;
			}
			fprintf(f, "INPUTS:\n");
			for (int t = 0; t < 16; t++)
				fprintf(f, "v%d %f %f %f %f\n", t, in->attribute[t].fv[0], in->attribute[t].fv[1], in->attribute[t].fv[2], in->attribute[t].fv[3]);
			fprintf(f, "CONSTANTS:\n");
			for (int t = 0; t < 192; t++)
				fprintf(f, "c[%d] %f %f %f %f\n", t - 96, c_constant[t].fv[0], c_constant[t].fv[1], c_constant[t].fv[2], c_constant[t].fv[3]);
			fclose(f);
		}
	}
#endif
	set_data(in, out);
	while (count > 0) {
		reset();
		jump(address);
		execute();
		input++;
		output++;
		count--;
	}
	input = nullptr;
	output = nullptr;
}

int vertex_program_simulator::status()
{
	return ip;
}

void vertex_program_simulator::initialize_outputs()
{
	for (int n = 0; n < 16; n++) {
		output->attribute[n].fv[0] = output->attribute[n].fv[1] = output->attribute[n].fv[2] = 0;
		output->attribute[n].fv[3] = 1;
	}
}

void vertex_program_simulator::initialize_temps()
{
	for (auto & elem : r_register) {
		for (int m = 0; m < 4; m++)
			elem.fv[m] = 0;
	}
}

void vertex_program_simulator::initialize_constants()
{
	for (auto & elem : c_constant) {
		for (int m = 0; m < 4;m++)
			elem.fv[m] = 0;
	}
}

void vertex_program_simulator::generate_input(float t[4], int sign, int type, int temp, int swizzle[4])
{
	float sgn = 1;

	if (sign)
		sgn = -1;
	if (type == 1) {
		t[0] = sgn * r_register[temp].fv[swizzle[0]];
		t[1] = sgn * r_register[temp].fv[swizzle[1]];
		t[2] = sgn * r_register[temp].fv[swizzle[2]];
		t[3] = sgn * r_register[temp].fv[swizzle[3]];
	}
	else if (type == 2) {
		int InputIndex = op[ip].d.InputIndex;
		t[0] = sgn * input->attribute[InputIndex].fv[swizzle[0]];
		t[1] = sgn * input->attribute[InputIndex].fv[swizzle[1]];
		t[2] = sgn * input->attribute[InputIndex].fv[swizzle[2]];
		t[3] = sgn * input->attribute[InputIndex].fv[swizzle[3]];
	}
	else if (type == 3) {
		int SourceConstantIndex = op[ip].d.SourceConstantIndex;
		if (op[ip].d.Usea0x)
			SourceConstantIndex = SourceConstantIndex + a0x;
		t[0] = sgn * c_constant[SourceConstantIndex].fv[swizzle[0]];
		t[1] = sgn * c_constant[SourceConstantIndex].fv[swizzle[1]];
		t[2] = sgn * c_constant[SourceConstantIndex].fv[swizzle[2]];
		t[3] = sgn * c_constant[SourceConstantIndex].fv[swizzle[3]];
	}
}

void vertex_program_simulator::compute_vectorial_operation(float t_out[4], int instruction, float par_in[3 * 4])
{
	const int p1_A = 0;
	const int p2_B = 4;
	const int p3_C = 8;

	// t_out <= instruction(par_in)
	switch (instruction) {
	case 0: // "NOP"
		break;
	case 1: // "MOV"
		t_out[0] = par_in[p1_A + 0];
		t_out[1] = par_in[p1_A + 1];
		t_out[2] = par_in[p1_A + 2];
		t_out[3] = par_in[p1_A + 3];
		break;
	case 2: // "MUL"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0];
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1];
		t_out[2] = par_in[p1_A + 2] * par_in[p2_B + 2];
		t_out[3] = par_in[p1_A + 3] * par_in[p2_B + 3];
		break;
	case 3: // "ADD"
		t_out[0] = par_in[p1_A + 0] + par_in[p3_C + 0];
		t_out[1] = par_in[p1_A + 1] + par_in[p3_C + 1];
		t_out[2] = par_in[p1_A + 2] + par_in[p3_C + 2];
		t_out[3] = par_in[p1_A + 3] + par_in[p3_C + 3];
		break;
	case 4: // "MAD"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p3_C + 0];
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p3_C + 1];
		t_out[2] = par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p3_C + 2];
		t_out[3] = par_in[p1_A + 3] * par_in[p2_B + 3] + par_in[p3_C + 3];
		break;
	case 5: // "DP3"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 6: // "DPH"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p2_B + 3];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 7: // "DP4"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p1_A + 3] * par_in[p2_B + 3];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 8: // "DST"
		t_out[0] = 1.0;
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1];
		t_out[2] = par_in[p1_A + 2];
		t_out[3] = par_in[p2_B + 3];
		break;
	case 9: // "MIN"
		t_out[0] = fmin(par_in[p1_A + 0], par_in[p2_B + 0]);
		t_out[1] = fmin(par_in[p1_A + 1], par_in[p2_B + 1]);
		t_out[2] = fmin(par_in[p1_A + 2], par_in[p2_B + 2]);
		t_out[3] = fmin(par_in[p1_A + 3], par_in[p2_B + 3]);
		break;
	case 10: // "MAX"
		t_out[0] = fmax(par_in[p1_A + 0], par_in[p2_B + 0]);
		t_out[1] = fmax(par_in[p1_A + 1], par_in[p2_B + 1]);
		t_out[2] = fmax(par_in[p1_A + 2], par_in[p2_B + 2]);
		t_out[3] = fmax(par_in[p1_A + 3], par_in[p2_B + 3]);
		break;
	case 11: // "SLT"
		t_out[0] = (par_in[p1_A + 0] < par_in[p2_B + 0]) ? 1.0 : 0;
		t_out[1] = (par_in[p1_A + 1] < par_in[p2_B + 1]) ? 1.0 : 0;
		t_out[2] = (par_in[p1_A + 2] < par_in[p2_B + 2]) ? 1.0 : 0;
		t_out[3] = (par_in[p1_A + 3] < par_in[p2_B + 3]) ? 1.0 : 0;
		break;
	case 12: // "SGE"
		t_out[0] = (par_in[p1_A + 0] >= par_in[p2_B + 0]) ? 1.0 : 0;
		t_out[1] = (par_in[p1_A + 1] >= par_in[p2_B + 1]) ? 1.0 : 0;
		t_out[2] = (par_in[p1_A + 2] >= par_in[p2_B + 2]) ? 1.0 : 0;
		t_out[3] = (par_in[p1_A + 3] >= par_in[p2_B + 3]) ? 1.0 : 0;
		break;
	case 13: // "ARL"
		t_out[0] = par_in[p1_A + 0];
	}
}

void vertex_program_simulator::compute_scalar_operation(float t_out[4], int instruction, float par_in[3 * 4])
{
	//const int p1_A = 0;
	//const int p2_B = 4;
	const int p3_C = 8;
	union {
		float f;
		unsigned int i;
	} t;
	int e;

	// t_out <= instruction(par_in)
	switch (instruction) {
	case 0: // "NOP"
		break;
	case 1: // "IMV"
		t_out[0] = par_in[p3_C + 0];
		t_out[1] = par_in[p3_C + 1];
		t_out[2] = par_in[p3_C + 2];
		t_out[3] = par_in[p3_C + 3];
		break;
	case 2: // "RCP"
		if (par_in[p3_C + 0] == 0)
			t.f = std::numeric_limits<float>::infinity();
		else if (par_in[p3_C + 0] == 1.0f)
			t.f = 1.0f;
		else
			t.f = 1.0f / par_in[p3_C + 0];
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = t.f;
		break;
	case 3: // "RCC"
		t.f = par_in[p3_C + 0];
		if ((t.f < 0) && (t.f > -5.42101e-20f))
			t.f = -5.42101e-20f;
		else if ((t.f >= 0) && (t.f < 5.42101e-20f))
			t.f = 5.42101e-20f;
		if (t.f != 1.0f)
			t.f = 1.0f / t.f;
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = t.f;
		break;
	case 4: // "RSQ"
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = 1.0f / sqrtf(fabsf(par_in[p3_C + 0]));
		break;
	case 5: // "EXP"
		t_out[0] = pow(2, floor(par_in[p3_C + 0]));
		t_out[1] = par_in[p3_C + 0] - floorf(par_in[p3_C + 0]);
		t.f = pow(2, par_in[p3_C + 0]);
		t.i = t.i & 0xffffff00;
		t_out[2] = t.f;
		t_out[3] = 1.0;
		break;
	case 6: // "LOG"
		t_out[1] = frexp(par_in[p3_C + 0], &e)*2.0; // frexp gives mantissa as 0.5....1
		t_out[0] = e - 1;
#ifndef __OS2__
		t.f = log2(fabsf(par_in[p3_C + 0]));
#else
		static double log_2 = 0.0;
		if (log_2 == 0.0)
			log_2 = log(2);
		t.f = log(abs(par_in[p3_C + 0])) / log_2;
#endif
		t.i = t.i & 0xffffff00;
		t_out[2] = t.f;
		t_out[3] = 1.0;
		break;
	case 7: // "LIT"
		t_out[0] = 1.0;
		t_out[1] = fmax(0, fmin(par_in[p3_C + 0], 1.0f));
		t_out[2] = par_in[p3_C + 0] > 0 ? pow(fmax(par_in[p3_C + 1], 0), par_in[p3_C + 3]) : 0;
		t_out[3] = 1.0;
		break;
	}
}

void vertex_program_simulator::assign_output(int index, float t[4], int mask)
{
	for (int p1 = 0; p1 < 4; p1++) {
		if (mask & 8)
			output->attribute[index].fv[p1] = t[p1];
		mask = mask << 1;
	}
}

void vertex_program_simulator::assign_register(int index, float t[4], int mask)
{
	for (int p1 = 0; p1 < 4; p1++) {
		if (mask & 8)
			r_register[index].fv[p1] = t[p1];
		mask = mask << 1;
	}
}

void vertex_program_simulator::assign_constant(int index, float t[4], int mask)
{
	for (int p1 = 0; p1 < 4; p1++) {
		if (mask & 8)
			c_constant[index].fv[p1] = t[p1];
		mask = mask << 1;
	}
}

/*
 * Graphics
 */

uint32_t nv2a_renderer::dilate0(uint32_t value, int bits) // dilate first "bits" bits in "value"
{
	uint32_t x, m1, m2, m3;
	int a;

	x = value;
	for (a = 0; a < bits; a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + (x & m2) + ((x & m3) << 1);
	}
	return x;
}

uint32_t nv2a_renderer::dilate1(uint32_t value, int bits) // dilate first "bits" bits in "value"
{
	uint32_t x, m1, m2, m3;
	int a;

	x = value;
	for (a = 0; a < bits; a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + ((x & m2) << 1) + ((x & m3) << 1);
	}
	return x;
}

void nv2a_renderer::computedilated(void)
{
	int a, b;

	for (b = 0; b < 16; b++)
		for (a = 0; a < 2048; a++) {
			dilated0[b][a] = dilate0(a, b);
			dilated1[b][a] = dilate1(a, b);
		}
	for (b = 0; b < 16; b++)
		for (a = 0; a < 16; a++)
			dilatechose[(b << 4) + a] = (a < b ? a : b);
}

inline uint8_t *nv2a_renderer::direct_access_ptr(offs_t address)
{
#ifdef DEBUG_CHECKS
	if (address >= 512*1024*1024)
		machine().logerror("Bad address in direct_access_ptr !\n");
#endif
	return basemempointer + address;
}

nv2a_renderer::COMMAND nv2a_renderer::geforce_commandkind(uint32_t word)
{
	if ((word & 0x00000003) == 0x00000002)
		return COMMAND::CALL;
	if ((word & 0x00000003) == 0x00000001)
		return COMMAND::JUMP;
	if ((word & 0xE0030003) == 0x40000000)
		return COMMAND::NON_INCREASING;
	if ((word & 0xE0000003) == 0x20000000)
		return COMMAND::OLD_JUMP;
	if ((word & 0xFFFF0003) == 0x00030000)
		return COMMAND::LONG_NON_INCREASING;
	if ((word & 0xFFFFFFFF) == 0x00020000)
		return COMMAND::RETURN;
	if ((word & 0xFFFF0003) == 0x00010000)
		return COMMAND::SLI_CONDITIONAL;
	if ((word & 0xE0030003) == 0x00000000)
		return COMMAND::INCREASING;
	return COMMAND::INVALID;
}

uint32_t nv2a_renderer::geforce_object_offset(uint32_t handle)
{
	uint32_t h = ((((handle >> 11) ^ handle) >> 11) ^ handle) & 0x7ff;
	uint32_t o = (pfifo[0x210 / 4] & 0x1ff) << 8; // 0x1ff is not certain
	uint32_t e = o + h * 8; // at 0xfd000000+0x00700000
	uint32_t w;

	if (ramin[e / 4] != handle) {
		// this should never happen
		for (uint32_t aa = o / 4; aa < (sizeof(ramin) / 4); aa = aa + 2) {
			if (ramin[aa] == handle) {
				e = aa * 4;
			}
		}
	}
	w = ramin[e / 4 + 1];
	return (w & 0xffff) * 0x10; // 0xffff is not certain
}

void nv2a_renderer::geforce_read_dma_object(uint32_t handle, uint32_t &offset, uint32_t &size)
{
	//uint32_t objclass,pt_present,pt_linear,access,target,rorw;
	uint32_t dma_adjust, dma_frame;
	uint32_t o = geforce_object_offset(handle);

	o = o / 4;
	//objclass=ramin[o] & 0xfff;
	//pt_present=(ramin[o] >> 12) & 1;
	//pt_linear=(ramin[o] >> 13) & 1;
	//access=(ramin[o] >> 14) & 3;
	//target=(ramin[o] >> 16) & 3;
	dma_adjust = (ramin[o] >> 20) & 0xfff;
	size = ramin[o + 1];
	//rorw=ramin[o+2] & 1;
	dma_frame = ramin[o + 2] & 0xfffff000;
	offset = dma_frame + dma_adjust;
}

/*void debug(uint32_t *bmp, int width, int eight, float x1, float y1, float x2, float y2, uint32_t color)
{
int xx1,yy1,xx2,yy2;

    xx1=x1;
    xx2=x2;
    yy1=y1;
    yy2=y2;
    if (xx1 == xx2) {
        if (yy1 > yy2) {
            int t=yy1;
            yy1=yy2;
            yy2=t;
        }
        for (int y=yy1;y <= yy2;y++) {
            *(bmp+y*width+xx1) = color;
        }
    } else if (yy1 == yy2) {
        if (xx1 > xx2) {
            int t=xx1;
            xx1=xx2;
            xx2=t;
        }
        for (int x=xx1;x <= xx2;x++)
            *(bmp+yy1*width+x) = color;
    }
}*/

inline uint32_t convert_a4r4g4b4_a8r8g8b8(uint32_t a4r4g4b4)
{
	uint32_t a8r8g8b8;
	int ca, cr, cg, cb;

	cb = pal4bit(a4r4g4b4 & 0x000f);
	cg = pal4bit((a4r4g4b4 & 0x00f0) >> 4);
	cr = pal4bit((a4r4g4b4 & 0x0f00) >> 8);
	ca = pal4bit((a4r4g4b4 & 0xf000) >> 12);
	a8r8g8b8 = (ca << 24) | (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline uint32_t convert_a1r5g5b5_a8r8g8b8(uint32_t a1r5g5b5)
{
	uint32_t a8r8g8b8;
	int ca, cr, cg, cb;

	cb = pal5bit(a1r5g5b5 & 0x001f);
	cg = pal5bit((a1r5g5b5 & 0x03e0) >> 5);
	cr = pal5bit((a1r5g5b5 & 0x7c00) >> 10);
	ca = a1r5g5b5 & 0x8000 ? 0xff : 0;
	a8r8g8b8 = (ca << 24) | (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline uint32_t convert_r5g6b5_r8g8b8(uint32_t r5g6b5)
{
	uint32_t r8g8b8;
	int cr, cg, cb;

	cb = pal5bit(r5g6b5 & 0x001f);
	cg = pal6bit((r5g6b5 & 0x07e0) >> 5);
	cr = pal5bit((r5g6b5 & 0xf800) >> 11);
	r8g8b8 = (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return r8g8b8;
}

uint32_t nv2a_renderer::texture_get_texel(int number, int x, int y)
{
	uint32_t to, s, c, sa, ca;
	uint32_t a4r4g4b4, a1r5g5b5, r5g6b5;
	int bx, by;
	int color0, color1, color0m2, color1m2, alpha0, alpha1;
	uint32_t codes;
	uint64_t alphas;
	int cr, cg, cb;
	int sizeu, sizev;

	if (texture[number].rectangle == false) {
		sizeu = texture[number].sizes;
		sizev = texture[number].sizet;
	}
	else {
		sizeu = texture[number].rectwidth;
		sizev = texture[number].rectheight;
	}
	switch (texture[number].addrmodes) {
	default:
	case 1: // wrap
		x = x % sizeu;
		if (x < 0)
			x = sizeu + x;
		break;
	case 3: // clamp
		if (x < 0)
			x = 0;
		if (x >= sizeu)
			x = sizeu - 1;
		break;
	}
	switch (texture[number].addrmodet) {
	default:
	case 1: // wrap
		y = y % sizev;
		if (y < 0)
			y = sizev + y;
		break;
	case 3: // clamp
		if (y < 0)
			y = 0;
		if (y >= sizev)
			y = sizev - 1;
		break;
	}
	switch (texture[number].format) {
	case NV2A_TEX_FORMAT::A8R8G8B8:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		return *(((uint32_t *)texture[number].buffer) + to); // get texel color
	case NV2A_TEX_FORMAT::X8R8G8B8:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		return 0xff000000 | (*(((uint32_t*)texture[number].buffer) + to) & 0xffffff); // get texel color
	case NV2A_TEX_FORMAT::DXT1:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = bx + by*(sizeu >> 2);
		color0 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 0);
		color1 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 1);
		codes = *((uint32_t *)(((uint64_t *)texture[number].buffer) + to) + 1);
		s = (y << 3) + (x << 1);
		c = (codes >> s) & 3;
		c = c + (color0 > color1 ? 0 : 4);
		color0m2 = color0 << 1;
		color1m2 = color1 << 1;
		switch (c) {
		case 0:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit(((color0m2 & 0x003e) + (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0m2 & 0x0fc0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0m2 & 0x1f000) + color1) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 3:
			cb = pal5bit(((color1m2 & 0x003e) + (color0 & 0x001f)) / 3);
			cg = pal6bit(((color1m2 & 0x0fc0) + (color0 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color1m2 & 0x1f000) + color0) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 4:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 5:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 6:
			cb = pal5bit(((color0 & 0x001f) + (color1 & 0x001f)) / 2);
			cg = pal6bit(((color0 & 0x07e0) + (color1 & 0x07e0)) / 2 >> 5);
			cr = pal5bit(((color0 & 0xf800) + (color1 & 0xf800)) / 2 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		default:
			return 0xff000000;
		}
	case NV2A_TEX_FORMAT::DXT3:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(sizeu >> 2)) << 1;
		color0 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 4);
		color1 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 5);
		codes = *((uint32_t *)(((uint64_t *)texture[number].buffer) + to) + 3);
		alphas = *(((uint64_t *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) << 2;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 15;
		switch (c) {
		case 0:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	case NV2A_TEX_FORMAT::A4R4G4B4:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a4r4g4b4 = *(((uint16_t *)texture[number].buffer) + to); // get texel color
		return convert_a4r4g4b4_a8r8g8b8(a4r4g4b4);
	case NV2A_TEX_FORMAT::A8:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		c = *(((uint8_t*)texture[number].buffer) + to); // get texel color
		return c << 24;
	case NV2A_TEX_FORMAT::A1R5G5B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a1r5g5b5 = *(((uint16_t *)texture[number].buffer) + to); // get texel color
		return convert_a1r5g5b5_a8r8g8b8(a1r5g5b5);
	case NV2A_TEX_FORMAT::R5G6B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		r5g6b5 = *(((uint16_t *)texture[number].buffer) + to); // get texel color
		return 0xff000000 + convert_r5g6b5_r8g8b8(r5g6b5);
	case NV2A_TEX_FORMAT::R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((uint32_t *)(((uint8_t *)texture[number].buffer) + to));
	case NV2A_TEX_FORMAT::A8R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((uint32_t *)(((uint8_t *)texture[number].buffer) + to));
	case NV2A_TEX_FORMAT::DXT5:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(sizeu >> 2)) << 1;
		color0 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 4);
		color1 = *((uint16_t *)(((uint64_t *)texture[number].buffer) + to) + 5);
		codes = *((uint32_t *)(((uint64_t *)texture[number].buffer) + to) + 3);
		alpha0 = *((uint8_t *)(((uint64_t *)texture[number].buffer) + to) + 0);
		alpha1 = *((uint8_t *)(((uint64_t *)texture[number].buffer) + to) + 1);
		alphas = *(((uint64_t *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) * 3;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 7;
		ca = ca + (alpha0 > alpha1 ? 0 : 8);
		switch (ca) {
		case 0:
			ca = alpha0;
			break;
		case 1:
			ca = alpha1;
			break;
		case 2:
			ca = (6 * alpha0 + 1 * alpha1) / 7;
			break;
		case 3:
			ca = (5 * alpha0 + 2 * alpha1) / 7;
			break;
		case 4:
			ca = (4 * alpha0 + 3 * alpha1) / 7;
			break;
		case 5:
			ca = (3 * alpha0 + 4 * alpha1) / 7;
			break;
		case 6:
			ca = (2 * alpha0 + 5 * alpha1) / 7;
			break;
		case 7:
			ca = (1 * alpha0 + 6 * alpha1) / 7;
			break;
		case 8:
			ca = alpha0;
			break;
		case 9:
			ca = alpha1;
			break;
		case 10:
			ca = (4 * alpha0 + 1 * alpha1) / 5;
			break;
		case 11:
			ca = (3 * alpha0 + 2 * alpha1) / 5;
			break;
		case 12:
			ca = (2 * alpha0 + 3 * alpha1) / 5;
			break;
		case 13:
			ca = (1 * alpha0 + 4 * alpha1) / 5;
			break;
		case 14:
			ca = 0;
			break;
		case 15:
			ca = 255;
			break;
		}
		switch (c) {
		case 0:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	default:
		return 0xff00ff00;
	}
}

inline uint8_t *nv2a_renderer::read_pixel(int x, int y, int32_t c[4])
{
	uint32_t offset;
	uint32_t color;
	uint32_t *addr;
	uint16_t *addr16;
	uint8_t *addr8;

	if (type_rendertarget == NV2A_RT_TYPE::SWIZZLED)
		offset = (dilated0[dilate_rendertarget][x] + dilated1[dilate_rendertarget][y]) * bytespixel_rendertarget;
	else // type_rendertarget == LINEAR
		offset = pitch_rendertarget * y + x * bytespixel_rendertarget;
#ifdef DEBUG_CHECKS
	if (offset >= size_rendertarget)
	{
		machine().logerror("Bad offset computed in read_pixel !\n");
		offset = 0;
	}
#endif
	switch (colorformat_rendertarget) {
	case NV2A_COLOR_FORMAT::R5G6B5:
		addr16 = (uint16_t *)((uint8_t *)rendertarget + offset);
		color = *addr16;
		c[3] = 0xff;
		c[2] = pal5bit((color & 0xf800) >> 11);
		c[1] = pal6bit((color & 0x07e0) >> 5);
		c[0] = pal5bit(color & 0x1f);
		return (uint8_t *)addr16;
	case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
	case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
		addr = (uint32_t *)((uint8_t *)rendertarget + offset);
		color = *addr;

		c[3] = 0xff;
		c[2] = (color >> 16) & 255;
		c[1] = (color >> 8) & 255;
		c[0] = color & 255;
		return (uint8_t *)addr;
	case NV2A_COLOR_FORMAT::A8R8G8B8:
		addr = (uint32_t *)((uint8_t *)rendertarget + offset);
		color = *addr;
		c[3] = color >> 24;
		c[2] = (color >> 16) & 255;
		c[1] = (color >> 8) & 255;
		c[0] = color & 255;
		return (uint8_t *)addr;
	case NV2A_COLOR_FORMAT::B8:
		addr8 = (uint8_t *)rendertarget + offset;
		c[0] = *addr8;
		c[1] = c[2] = 0;
		c[3] = 0xff;
		return addr8;
	default:
		return nullptr;
	}
	return nullptr;
}

void nv2a_renderer::write_pixel(int x, int y, uint32_t color, int z)
{
	uint8_t *addr;
	uint32_t *daddr32;
	uint16_t *daddr16;
	uint32_t depthandstencil;
	int32_t c[4], fb[4], s[4], d[4], cc[4];
	uint32_t depth, stencil, stenc, stenv;
	uint32_t udepth;
	bool stencil_passed;
	bool depth_passed;

	if ((z > 0xffffff) || (z < 0) || (x < 0))
		return;
	udepth = (uint32_t)z;
	fb[3] = fb[2] = fb[1] = fb[0] = 0;
	addr = nullptr;
	if (color_mask != 0)
		addr = read_pixel(x, y, fb);
	if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z24S8) {
#ifdef DEBUG_CHECKS
		if (((pitch_depthbuffer / 4) * y + x) >= size_depthbuffer)
		{
			machine().logerror("Bad depthbuffer offset computed in write_pixel !\n");
			return;
		}
#endif
		daddr32 = depthbuffer + (pitch_depthbuffer / 4) * y + x;
		depthandstencil = *daddr32;
		depth = depthandstencil >> 8;
		stencil = depthandstencil & 255;
		daddr16 = nullptr;
	}
	else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z16) {
#ifdef DEBUG_CHECKS
		if (((pitch_depthbuffer / 2) * y + x) >= size_depthbuffer)
		{
			machine().logerror("Bad depthbuffer offset computed in write_pixel !\n");
			return;
		}
#endif
		daddr16 = (uint16_t *)depthbuffer + (pitch_depthbuffer / 2) * y + x;
		depthandstencil = *daddr16;
		depth = (depthandstencil << 8) | 0xff;
		stencil = 0;
		daddr32 = nullptr;
	}
	else {
		daddr32 = nullptr;
		daddr16 = nullptr;
		depth = 0xffffff;
		stencil = 0;
	}
	c[3] = color >> 24;
	c[2] = (color >> 16) & 255;
	c[1] = (color >> 8) & 255;
	c[0] = color & 255;
	cc[3] = blend_color >> 24;
	cc[2] = (blend_color >> 16) & 255;
	cc[1] = (blend_color >> 8) & 255;
	cc[0] = blend_color & 255;
	// ownership test and scissor test not done
	// alpha test
	if (alpha_test_enabled) {
		switch (alpha_func) {
			case NV2A_COMPARISON_OP::NEVER:
				return;
			case NV2A_COMPARISON_OP::ALWAYS:
			default:
				break;
			case NV2A_COMPARISON_OP::LESS:
				if (c[3] >= alpha_reference)
					return;
				break;
			case NV2A_COMPARISON_OP::LEQUAL:
				if (c[3] > alpha_reference)
					return;
				break;
			case NV2A_COMPARISON_OP::EQUAL:
				if (c[3] != alpha_reference)
					return;
				break;
			case NV2A_COMPARISON_OP::GEQUAL:
				if (c[3] < alpha_reference)
					return;
				break;
			case NV2A_COMPARISON_OP::GREATER:
				if (c[3] <= alpha_reference)
					return;
				break;
			case NV2A_COMPARISON_OP::NOTEQUAL:
				if (c[3] == alpha_reference)
					return;
				break;
		}
	}
	// stencil test
	stencil_passed = true;
	if (stencil_test_enabled) {
		stenc=stencil_mask & stencil_ref;
		stenv=stencil_mask & stencil;
		switch (stencil_func) {
		case NV2A_COMPARISON_OP::NEVER:
			stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::LESS:
			if (stenc >= stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::EQUAL:
			if (stenc != stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::LEQUAL:
			if (stenc > stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::GREATER:
			if (stenc <= stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::NOTEQUAL:
			if (stenc == stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::GEQUAL:
			if (stenc < stenv)
				stencil_passed = false;
			break;
		case NV2A_COMPARISON_OP::ALWAYS:
		default:
			break;
		}
		if (stencil_passed == false) {
			switch (stencil_op_fail) {
			case NV2A_STENCIL_OP::ZEROOP:
				stencil = 0;
				break;
			case NV2A_STENCIL_OP::INVERTOP:
				stencil = stencil ^ 255;
				break;
			case NV2A_STENCIL_OP::KEEP:
			default:
				break;
			case NV2A_STENCIL_OP::REPLACE:
				stencil = stencil_ref;
				break;
			case NV2A_STENCIL_OP::INCR:
				if (stencil < 255)
					stencil++;
				break;
			case NV2A_STENCIL_OP::DECR:
				if (stencil > 0)
					stencil--;
				break;
			case NV2A_STENCIL_OP::INCR_WRAP:
				if (stencil < 255)
					stencil++;
				else
					stencil = 0;
				break;
			case NV2A_STENCIL_OP::DECR_WRAP:
				if (stencil > 0)
					stencil--;
				else
					stencil = 255;
				break;
			}
			if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z24S8) {
				depthandstencil = (depth << 8) | stencil;
				*daddr32 = depthandstencil;
			}
			else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z16) {
				depthandstencil = depth >> 8;
				*daddr16 = (uint16_t)depthandstencil;
			}
			return;
		}
	}
	// depth buffer test
	depth_passed = true;
	if (depth_test_enabled) {
		switch (depth_function) {
			case NV2A_COMPARISON_OP::NEVER:
				depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::LESS:
				if (udepth >= depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::EQUAL:
				if (udepth != depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::LEQUAL:
				if (udepth > depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::GREATER:
				if (udepth <= depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::NOTEQUAL:
				if (udepth == depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::GEQUAL:
				if (udepth < depth)
					depth_passed = false;
				break;
			case NV2A_COMPARISON_OP::ALWAYS:
			default:
				break;
		}
		if (depth_passed == false) {
			switch (stencil_op_zfail) {
			case NV2A_STENCIL_OP::ZEROOP:
				stencil = 0;
				break;
			case NV2A_STENCIL_OP::INVERTOP:
				stencil = stencil ^ 255;
				break;
			case NV2A_STENCIL_OP::KEEP:
			default:
				break;
			case NV2A_STENCIL_OP::REPLACE:
				stencil = stencil_ref;
				break;
			case NV2A_STENCIL_OP::INCR:
				if (stencil < 255)
					stencil++;
				break;
			case NV2A_STENCIL_OP::DECR:
				if (stencil > 0)
					stencil--;
				break;
			case NV2A_STENCIL_OP::INCR_WRAP:
				if (stencil < 255)
					stencil++;
				else
					stencil = 0;
				break;
			case NV2A_STENCIL_OP::DECR_WRAP:
				if (stencil > 0)
					stencil--;
				else
					stencil = 255;
				break;
			}
			if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z24S8) {
				depthandstencil = (depth << 8) | stencil;
				*daddr32 = depthandstencil;
			}
			else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z16) {
				depthandstencil = depth >> 8;
				*daddr16 = (uint16_t)depthandstencil;
			}
			return;
		}
		switch (stencil_op_zpass) {
		case NV2A_STENCIL_OP::ZEROOP:
			stencil = 0;
			break;
		case NV2A_STENCIL_OP::INVERTOP:
			stencil = stencil ^ 255;
			break;
		case NV2A_STENCIL_OP::KEEP:
		default:
			break;
		case NV2A_STENCIL_OP::REPLACE:
			stencil = stencil_ref;
			break;
		case NV2A_STENCIL_OP::INCR:
			if (stencil < 255)
				stencil++;
			break;
		case NV2A_STENCIL_OP::DECR:
			if (stencil > 0)
				stencil--;
			break;
		case NV2A_STENCIL_OP::INCR_WRAP:
			if (stencil < 255)
				stencil++;
			else
				stencil = 0;
			break;
		case NV2A_STENCIL_OP::DECR_WRAP:
			if (stencil > 0)
				stencil--;
			else
				stencil = 255;
			break;
		}
	}
	// blending
	if (blending_enabled) {
		switch (blend_function_source) {
			case NV2A_BLEND_FACTOR::ZERO:
				s[3] = s[2] = s[1] = s[0] = 0;
				break;
			case NV2A_BLEND_FACTOR::ONE:
			default:
				s[3] = s[2] = s[1] = s[0] = 255;
				break;
			case NV2A_BLEND_FACTOR::DST_COLOR:
				s[3] = fb[3];
				s[2] = fb[2];
				s[1] = fb[1];
				s[0] = fb[0];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_DST_COLOR:
				s[3] = fb[3] ^ 255;
				s[2] = fb[2] ^ 255;
				s[1] = fb[1] ^ 255;
				s[0] = fb[0] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::SRC_ALPHA:
				s[3] = s[2] = s[1] = s[0] = c[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_SRC_ALPHA:
				s[3] = s[2] = s[1] = s[0] = c[3] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::DST_ALPHA:
				s[3] = s[2] = s[1] = s[0] = fb[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_DST_ALPHA:
				s[3] = s[2] = s[1] = s[0] = fb[3] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::CONSTANT_COLOR:
				s[3] = cc[3];
				s[2] = cc[2];
				s[1] = cc[1];
				s[0] = cc[0];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_CONSTANT_COLOR:
				s[3] = cc[3] ^ 255;
				s[2] = cc[2] ^ 255;
				s[1] = cc[1] ^ 255;
				s[0] = cc[0] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::CONSTANT_ALPHA:
				s[3] = s[2] = s[1] = s[0] = cc[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_CONSTANT_ALPHA:
				s[3] = s[2] = s[1] = s[0] = cc[3] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::SRC_ALPHA_SATURATE:
				s[3] = 255;
				if (c[3] < (fb[3] ^ 255))
					s[2] = c[3];
				else
					s[2] = fb[3];
				s[1] = s[0] = s[2];
				break;
		}
		switch (blend_function_destination) {
			case NV2A_BLEND_FACTOR::ZERO:
			default:
				d[3] = d[2] = d[1] = d[0] = 0;
				break;
			case NV2A_BLEND_FACTOR::ONE:
				d[3] = d[2] = d[1] = d[0] = 255;
				break;
			case NV2A_BLEND_FACTOR::SRC_COLOR:
				d[3] = c[3];
				d[2] = c[2];
				d[1] = c[1];
				d[0] = c[0];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_SRC_COLOR:
				d[3] = c[3] ^ 255;
				d[2] = c[2] ^ 255;
				d[1] = c[1] ^ 255;
				d[0] = c[0] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::SRC_ALPHA:
				d[3] = d[2] = d[1] = d[0] = c[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_SRC_ALPHA:
				d[3] = d[2] = d[1] = d[0] = c[3] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::DST_ALPHA:
				d[3] = d[2] = d[1] = d[0] = fb[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_DST_ALPHA:
				d[3] = d[2] = d[1] = d[0] = fb[3] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::CONSTANT_COLOR:
				d[3] = cc[3];
				d[2] = cc[2];
				d[1] = cc[1];
				d[0] = cc[0];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_CONSTANT_COLOR:
				d[3] = cc[3] ^ 255;
				d[2] = cc[2] ^ 255;
				d[1] = cc[1] ^ 255;
				d[0] = cc[0] ^ 255;
				break;
			case NV2A_BLEND_FACTOR::CONSTANT_ALPHA:
				d[3] = d[2] = d[1] = d[0] = cc[3];
				break;
			case NV2A_BLEND_FACTOR::ONE_MINUS_CONSTANT_ALPHA:
				d[3] = d[2] = d[1] = d[0] = cc[3] ^ 255;
				break;
		}
		switch (blend_equation) {
			case NV2A_BLEND_EQUATION::FUNC_ADD:
				c[3] = (c[3] * s[3] + fb[3] * d[3]) / 255;
				if (c[3] > 255)
					c[3] = 255;
				c[2] = (c[2] * s[2] + fb[2] * d[2]) / 255;
				if (c[2] > 255)
					c[2] = 255;
				c[1] = (c[1] * s[1] + fb[1] * d[1]) / 255;
				if (c[1] > 255)
					c[1] = 255;
				c[0] = (c[0] * s[0] + fb[0] * d[0]) / 255;
				if (c[0] > 255)
					c[0] = 255;
				break;
			case NV2A_BLEND_EQUATION::FUNC_SUBTRACT:
				c[3] = (c[3] * s[3] - fb[3] * d[3]) / 255;
				if (c[3] < 0)
					c[3] = 255;
				c[2] = (c[2] * s[2] - fb[2] * d[2]) / 255;
				if (c[2] < 0)
					c[2] = 255;
				c[1] = (c[1] * s[1] - fb[1] * d[1]) / 255;
				if (c[1] < 0)
					c[1] = 255;
				c[0] = (c[0] * s[0] - fb[0] * d[0]) / 255;
				if (c[0] < 0)
					c[0] = 255;
				break;
			case NV2A_BLEND_EQUATION::FUNC_REVERSE_SUBTRACT:
				c[3] = (fb[3] * d[3] - c[3] * s[3]) / 255;
				if (c[3] < 0)
					c[3] = 255;
				c[2] = (fb[2] * d[2] - c[2] * s[2]) / 255;
				if (c[2] < 0)
					c[2] = 255;
				c[1] = (fb[1] * d[1] - c[1] * s[1]) / 255;
				if (c[1] < 0)
					c[1] = 255;
				c[0] = (fb[0] * d[0] - c[0] * s[0]) / 255;
				if (c[0] < 0)
					c[0] = 255;
				break;
			case NV2A_BLEND_EQUATION::MIN:
				c[3] = s[3];
				if (d[3] < c[3])
					c[3] = d[3];
				c[2] = s[2];
				if (d[2] < c[2])
					c[2] = d[2];
				c[1] = s[1];
				if (d[1] < c[1])
					c[1] = d[1];
				c[0] = s[0];
				if (d[0] < c[0])
					c[0] = d[0];
				break;
			case NV2A_BLEND_EQUATION::MAX:
				c[3] = s[3];
				if (d[3] > c[3])
					c[3] = d[3];
				c[2] = s[2];
				if (d[2] > c[2])
					c[2] = d[2];
				c[1] = s[1];
				if (d[1] > c[1])
					c[1] = d[1];
				c[0] = s[0];
				if (d[0] > c[0])
					c[0] = d[0];
				break;
		}
	}
	// dithering not done
	// logical operation
	if (logical_operation_enabled) {
		switch (logical_operation) {
			case  NV2A_LOGIC_OP::CLEAR:
				c[3] = 0;
				c[2] = 0;
				c[1] = 0;
				c[0] = 0;
				break;
			case  NV2A_LOGIC_OP::AND:
				c[3] = c[3] & fb[3];
				c[2] = c[2] & fb[2];
				c[1] = c[1] & fb[1];
				c[0] = c[0] & fb[0];
				break;
			case  NV2A_LOGIC_OP::AND_REVERSE:
				c[3] = c[3] & (fb[3] ^ 255);
				c[2] = c[2] & (fb[2] ^ 255);
				c[1] = c[1] & (fb[1] ^ 255);
				c[0] = c[0] & (fb[0] ^ 255);
				break;
			case  NV2A_LOGIC_OP::COPY:
			default:
				break;
			case  NV2A_LOGIC_OP::AND_INVERTED:
				c[3] = (c[3] ^ 255) & fb[3];
				c[2] = (c[2] ^ 255) & fb[2];
				c[1] = (c[1] ^ 255) & fb[1];
				c[0] = (c[0] ^ 255) & fb[0];
				break;
			case  NV2A_LOGIC_OP::NOOP:
				c[3] = fb[3];
				c[2] = fb[2];
				c[1] = fb[1];
				c[0] = fb[0];
				break;
			case  NV2A_LOGIC_OP::XOR:
				c[3] = c[3] ^ fb[3];
				c[2] = c[2] ^ fb[2];
				c[1] = c[1] ^ fb[1];
				c[0] = c[0] ^ fb[0];
				break;
			case  NV2A_LOGIC_OP::OR:
				c[3] = c[3] | fb[3];
				c[2] = c[2] | fb[2];
				c[1] = c[1] | fb[1];
				c[0] = c[0] | fb[0];
				break;
			case  NV2A_LOGIC_OP::NOR:
				c[3] = (c[3] | fb[3]) ^ 255;
				c[2] = (c[2] | fb[2]) ^ 255;
				c[1] = (c[1] | fb[1]) ^ 255;
				c[0] = (c[0] | fb[0]) ^ 255;
				break;
			case  NV2A_LOGIC_OP::EQUIV:
				c[3] = (c[3] ^ fb[3]) ^ 255;
				c[2] = (c[2] ^ fb[2]) ^ 255;
				c[1] = (c[1] ^ fb[1]) ^ 255;
				c[0] = (c[0] ^ fb[0]) ^ 255;
				break;
			case  NV2A_LOGIC_OP::INVERT:
				c[3] = fb[3] ^ 255;
				c[2] = fb[2] ^ 255;
				c[1] = fb[1] ^ 255;
				c[0] = fb[0] ^ 255;
				break;
			case  NV2A_LOGIC_OP::OR_REVERSE:
				c[3] = c[3] | (fb[3] ^ 255);
				c[2] = c[2] | (fb[2] ^ 255);
				c[1] = c[1] | (fb[1] ^ 255);
				c[0] = c[0] | (fb[0] ^ 255);
				break;
			case  NV2A_LOGIC_OP::COPY_INVERTED:
				c[3] = c[3] ^ 255;
				c[2] = c[2] ^ 255;
				c[1] = c[1] ^ 255;
				c[0] = c[0] ^ 255;
				break;
			case  NV2A_LOGIC_OP::OR_INVERTED:
				c[3] = (c[3] ^ 255) | fb[3];
				c[2] = (c[2] ^ 255) | fb[2];
				c[1] = (c[1] ^ 255) | fb[1];
				c[0] = (c[0] ^ 255) | fb[0];
				break;
			case  NV2A_LOGIC_OP::NAND:
				c[3] = (c[3] & fb[3]) ^ 255;
				c[2] = (c[2] & fb[2]) ^ 255;
				c[1] = (c[1] & fb[1]) ^ 255;
				c[0] = (c[0] & fb[0]) ^ 255;
				break;
			case  NV2A_LOGIC_OP::SET:
				c[3] = 255;
				c[2] = 255;
				c[1] = 255;
				c[0] = 255;
				break;
		}
	}
	if (color_mask != 0) {
		uint32_t ct,ft,w;

		ct = ((uint32_t)c[3] << 24) | ((uint32_t)c[2] << 16) | ((uint32_t)c[1] << 8) | (uint32_t)c[0];
		ft = ((uint32_t)fb[3] << 24) | ((uint32_t)fb[2] << 16) | ((uint32_t)fb[1] << 8) | (uint32_t)fb[0];
		w = (ft & ~color_mask) | (ct & color_mask);

/* for debugging
        if (w == 0x94737d7b)
            x++;
*/
		switch (colorformat_rendertarget) {
		case NV2A_COLOR_FORMAT::R5G6B5:
			w = ((w >> 8) & 0xf800) + ((w >> 5) & 0x7e0) + ((w >> 3) & 0x1f);
			*((uint16_t *)addr) = (uint16_t)w;
			break;
		case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
		case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
			*((uint32_t *)addr) = w;
			break;
		case NV2A_COLOR_FORMAT::A8R8G8B8:
			*((uint32_t *)addr) = w;
			break;
		case NV2A_COLOR_FORMAT::B8:
			*addr = (uint8_t)w;
			break;
		default:
			return;
		}
	}
	if (depth_write_enabled)
		depth = udepth;
	if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z24S8) {
		depthandstencil = (depth << 8) | stencil;
		*daddr32 = depthandstencil;
	}
	else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT::Z16) {
		depthandstencil = depth >> 8;
		*daddr16 = (uint16_t)depthandstencil;
	}
}

void nv2a_renderer::render_color(int32_t scanline, const nv2a_rasterizer::extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x, lx;

	lx = limits_rendertarget.right();
	if ((extent.startx < 0) && (extent.stopx <= 0))
		return;
	if ((extent.startx > lx) && (extent.stopx > lx))
		return;
	x = extent.stopx - extent.startx; // number of pixels to draw (start inclusive, end exclusive)
	if (extent.stopx > lx)
		x = x - (extent.stopx - lx - 1);
	x--;
	while (x >= 0) {
		double zf;
		uint32_t a8r8g8b8;
		int z;
		int ca, cr, cg, cb;
		int xp = extent.startx + x; // x coordinate of current pixel

		z = (extent.param[(int)VERTEX_PARAMETER::PARAM_Z].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_Z].dpdx);
		zf = (extent.param[(int)VERTEX_PARAMETER::PARAM_1W].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_1W].dpdx);
		zf = 1.0f / zf;
		cb = std::clamp<int>(((extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_B].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_B].dpdx)) * zf * 255.0f, 0, 255);
		cg = std::clamp<int>(((extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_G].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_G].dpdx)) * zf * 255.0f, 0, 255);
		cr = std::clamp<int>(((extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_R].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_R].dpdx)) * zf * 255.0f, 0, 255);
		ca = std::clamp<int>(((extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_A].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_A].dpdx)) * zf * 255.0f, 0, 255);
		a8r8g8b8 = (ca << 24) | (cr << 16) | (cg << 8) | cb; // pixel color obtained by interpolating the colors of the vertices
		write_pixel(xp, scanline, a8r8g8b8, z);
		x--;
	}
}

void nv2a_renderer::render_texture_simple(int32_t scanline, const nv2a_rasterizer::extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int sizex, limitx;
	double mx, my;
	uint32_t a8r8g8b8;

	if (!objectdata.data->texture[0].enabled) {
		return;
	}
	limitx = limits_rendertarget.right();
	sizex = extent.stopx - extent.startx; // number of pixels to draw (start inclusive, end exclusive)
	if (extent.stopx > limitx)
		sizex = sizex - (extent.stopx - limitx - 1);
	if (objectdata.data->texture[0].rectangle == false) {
		// multply by 256 so that when converting to integer the lower 8 bits represent the fractional part
		mx = objectdata.data->texture[0].sizes * 256;
		my = objectdata.data->texture[0].sizet * 256;
	}
	else
		mx = my = 1 * 256;
	double zf = extent.param[(int)VERTEX_PARAMETER::PARAM_Z].start;
	double dwf = extent.param[(int)VERTEX_PARAMETER::PARAM_1W].start;
	double spf = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S].start;
	double tpf = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T].start;
	for (int posx = 0; posx < sizex; posx++) {
		int xp = extent.startx + posx; // x coordinate of current pixel
		double wf = 1.0f / dwf;
		double spf2 = spf * wf;
		double tpf2 = tpf * wf;
		//rpf = (extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R].dpdx) * wf;
		//qpf = (extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q].start + (double)x * extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q].dpdx) * wf;
		spf2 = spf2 * mx;
		tpf2 = tpf2 * my;
		int z = zf;
		int px = spf2; // x coordinate of texel in texture
		int py = tpf2; // y coordinate of texel in texture
		int pxfr = px & 255;
		int pyfr = py & 255;
		int pxi = px >> 8;
		int pyi = py >> 8;
		if (objectdata.data->bilinear_filter) {
			uint32_t c00 = texture_get_texel(0, pxi + 0, pyi + 0);
			uint32_t c01 = texture_get_texel(0, pxi + 0, pyi + 1);
			uint32_t c10 = texture_get_texel(0, pxi + 1, pyi + 0);
			uint32_t c11 = texture_get_texel(0, pxi + 1, pyi + 1);
			a8r8g8b8 = rgbaint_t::bilinear_filter(c00, c10, c01, c11, pxfr, pyfr);
		}
		else {
			a8r8g8b8 = texture_get_texel(0, pxi, pyi);
		}
		write_pixel(xp, scanline, a8r8g8b8, z);
		zf += extent.param[(int)VERTEX_PARAMETER::PARAM_Z].dpdx;
		dwf += extent.param[(int)VERTEX_PARAMETER::PARAM_1W].dpdx;
		spf += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S].dpdx;
		tpf += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T].dpdx;
	}
}

void nv2a_renderer::render_register_combiners(int32_t scanline, const nv2a_rasterizer::extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int sizex, limitx;
	float colorf[7][4];
	double mx[4], my[4];
	double spf[4], tpf[4], rpf[4], qpf[4];
	uint32_t color[6];
	uint32_t a8r8g8b8;
	int n;

	color[0] = color[1] = color[2] = color[3] = color[4] = color[5] = 0;
	limitx = limits_rendertarget.right();
	sizex = extent.stopx - extent.startx;
	if ((extent.startx < 0) && (extent.stopx <= 0))
		return;
	if ((extent.startx > limitx) && (extent.stopx > limitx))
		return;
	if (extent.stopx > limitx)
		sizex = sizex - (extent.stopx - limitx - 1);
	double zf = extent.param[(int)VERTEX_PARAMETER::PARAM_Z].start;
	double dwf = extent.param[(int)VERTEX_PARAMETER::PARAM_1W].start;
	for (n = 0; n < 4; n++) {
		if (objectdata.data->texture[n].rectangle == false) {
			mx[n] = objectdata.data->texture[n].sizes * 256;
			my[n] = objectdata.data->texture[n].sizet * 256;
		}
		else
			mx[n] = my[n] = 1 * 256;
		spf[n] = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S + n * 4].start;
		tpf[n] = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T + n * 4].start;
		rpf[n] = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R + n * 4].start;
		qpf[n] = extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q + n * 4].start;
	}
	double c00f = extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_R].start;
	double c01f = extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_G].start;
	double c02f = extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_B].start;
	double c03f = extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_A].start;
	double c10f = extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_R].start;
	double c11f = extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_G].start;
	double c12f = extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_B].start;
	double c13f = extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_A].start;
	for (int posx = 0; posx < sizex; posx++) {
		int xp = extent.startx + posx;
		// 1: fetch data
		// 1.1: interpolated color from vertices
		int z = zf;
		double wf = 1.0f / dwf;
		colorf[0][0] = c00f * wf;
		colorf[0][1] = c01f * wf;
		colorf[0][2] = c02f * wf;
		colorf[0][3] = c03f * wf;
		colorf[1][0] = c10f * wf;
		colorf[1][1] = c11f * wf;
		colorf[1][2] = c12f * wf;
		colorf[1][3] = c13f * wf;
		// 1.2: coordinates for each of the 4 possible textures
		for (n = 0; n < 4; n++) {
			colorf[n + 2][0] = spf[n] * wf;
			colorf[n + 2][1] = tpf[n] * wf;
			colorf[n + 2][2] = rpf[n] * wf;
			colorf[n + 2][3] = qpf[n] * wf;
		}
		// 1.3: fog
		combiner_argb8_float(fog_color, colorf[6]);
		colorf[6][3] = 1.0f; // should it be from the ofog output of the vertex shader ?
		// 1.4: colors from textures
		for (n = 0; n < 4; n++) {
			if (texture[n].mode == 1) {
				double spf2 = colorf[n + 2][0] * mx[n];
				double tpf2 = colorf[n + 2][1] * my[n];
				int px = spf2;
				int py = tpf2;
				int pxfr = px & 255;
				int pyfr = py & 255;
				int pxi = px >> 8;
				int pyi = py >> 8;
				if (objectdata.data->bilinear_filter) {
					uint32_t c00 = texture_get_texel(n, pxi + 0, pyi + 0);
					uint32_t c01 = texture_get_texel(n, pxi + 0, pyi + 1);
					uint32_t c10 = texture_get_texel(n, pxi + 1, pyi + 0);
					uint32_t c11 = texture_get_texel(n, pxi + 1, pyi + 1);
					a8r8g8b8 = rgbaint_t::bilinear_filter(c00, c10, c01, c11, pxfr, pyfr);
				}
				else {
					a8r8g8b8 = texture_get_texel(n, pxi, pyi);
				}
				combiner_argb8_float(a8r8g8b8, colorf[n + 2]);
			}
			else if (texture[n].mode == 4)
				; // nothing
			else
				combiner_argb8_float(0xff000000, colorf[n + 2]);
		}
		// 2: compute
		// 2.1: initialize
		combiner_initialize_registers(threadid, colorf);
		// 2.2: general combiner stages
		for (n = 0; n < combiner.setup.stages; n++) {
			// 2.2.1 initialize
			combiner_initialize_stage(threadid, n);
			// 2.2.2 map inputs
			combiner_map_stage_input(threadid, n);
			// 2.2.3 compute possible outputs
			combiner_compute_rgb_outputs(threadid, n);
			combiner_compute_alpha_outputs(threadid, n);
			// 2.2.4 map outputs to registers
			combiner_map_stage_output(threadid, n);
		}
		// 2.3: final combiner stage
		combiner_initialize_final(threadid);
		combiner_map_final_input(threadid);
		combiner_final_output(threadid);
		a8r8g8b8 = combiner_float_argb8(combiner.work[threadid].output);
		// 3: write pixel
		write_pixel(xp, scanline, a8r8g8b8, z);
		zf += extent.param[(int)VERTEX_PARAMETER::PARAM_Z].dpdx;
		dwf += extent.param[(int)VERTEX_PARAMETER::PARAM_1W].dpdx;
		for (n = 0; n < 4; n++) {
			spf[n] += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S + n * 4].dpdx;
			tpf[n] += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T + n * 4].dpdx;
			rpf[n] += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R + n * 4].dpdx;
			qpf[n] += extent.param[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q + n * 4].dpdx;
		}
		c00f += extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_R].dpdx;
		c01f += extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_G].dpdx;
		c02f += extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_B].dpdx;
		c03f += extent.param[(int)VERTEX_PARAMETER::PARAM_COLOR_A].dpdx;
		c10f += extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_R].dpdx;
		c11f += extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_G].dpdx;
		c12f += extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_B].dpdx;
		c13f += extent.param[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_A].dpdx;
	}
}

#if 0
const char *rc_mapping_str[] = {
	"UNSIGNED_IDENTITY",
	"UNSIGNED_INVERT",
	"EXPAND_NORMAL",
	"EXPAND_NEGATE",
	"HALF_BIAS_NORMAL",
	"HALF_BIAS_NEGATE",
	"SIGNED_IDENTITY",
	"SIGNED_NEGATE"
};

const char *rc_usage_rgb_str[] = {
	"RGB",
	"ALPHA"
};

const char *rc_usage_alpha_str[] = {
	"BLUE",
	"ALPHA"
};

const char *rc_variable_str[] = {
	"ZERO",
	"CONSTANT_COLOR0",
	"CONSTANT_COLOR1",
	"FOG",
	"PRIMARY_COLOR",
	"SECONDARY_COLOR",
	"???",
	"???",
	"TEXTURE0",
	"TEXTURE1",
	"TEXTURE2",
	"TEXTURE3",
	"SPARE0",
	"SPARE1",
	"SPARE0_PLUS_SECONDARY_COLOR",
	"E_TIMES_F"
};

const char *rc_bias_str[] = {
	"NONE",
	"BIAS_BY_NEGATIVE_ONE_HALF"
};

const char *rc_scale_str[] = {
	"NONE",
	"SCALE_BY_TWO",
	"SCALE_BY_FOUR",
	"SCALE_BY_ONE_HALF"
};

/* Dump the current setup of the register combiners */
void dumpcombiners(uint32_t *m)
{
	int a, b, n, v;

	n = m[0x1e60 / 4] & 0xf;
	printf("Combiners active: %d\n\r", n);
	for (a = 0; a < n; a++) {
		printf("Combiner %d\n\r", a + 1);
		printf(" RC_IN_ALPHA %08X\n\r", m[0x0260 / 4 + a]);
		for (b = 24; b >= 0; b = b - 8) {
			v = (m[0x0260 / 4 + a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r", 'A' + 3 - b / 8, rc_variable_str[v]);
			v = (m[0x0260 / 4 + a] >> (b + 4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - b / 8, rc_usage_alpha_str[v]);
			v = (m[0x0260 / 4 + a] >> (b + 5)) & 7;
			printf("  %c_MAPPING %s\n\r", 'A' + 3 - b / 8, rc_mapping_str[v]);
		}
		printf(" RC_IN_RGB %08X\n\r", m[0x0ac0 / 4 + a]);
		for (b = 24; b >= 0; b = b - 8) {
			v = (m[0x0ac0 / 4 + a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r", 'A' + 3 - b / 8, rc_variable_str[v]);
			v = (m[0x0ac0 / 4 + a] >> (b + 4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - b / 8, rc_usage_rgb_str[v]);
			v = (m[0x0ac0 / 4 + a] >> (b + 5)) & 7;
			printf("  %c_MAPPING %s\n\r", 'A' + 3 - b / 8, rc_mapping_str[v]);
		}
		printf(" RC_OUT_ALPHA %08X\n\r", m[0x0aa0 / 4 + a]);
		v = m[0x0aa0 / 4 + a] & 0xf;
		printf("  CD_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 15) & 1;
		printf("  BIAS %s\n\r", rc_bias_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 16) & 3;
		printf("  SCALE %s\n\r", rc_scale_str[v]);
		//v=(m[0x0aa0/4+a] >> 27) & 7;
		printf(" RC_OUT_RGB %08X\n\r", m[0x1e40 / 4 + a]);
		v = m[0x1e40 / 4 + a] & 0xf;
		printf("  CD_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 15) & 1;
		printf("  BIAS %s\n\r", rc_bias_str[v]);
		v = (m[0x1e40 / 4 + a] >> 16) & 3;
		printf("  SCALE %s\n\r", rc_scale_str[v]);
		//v=(m[0x1e40/4+a] >> 27) & 7;
		printf("\n\r");
	}
	printf("Combiner final %08X %08X\n\r", m[0x0288 / 4], m[0x028c / 4]);
	for (a = 24; a >= 0; a = a - 8) {
		n = (m[0x0288 / 4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r", 'A' + 3 - a / 8, rc_variable_str[n]);
		n = (m[0x0288 / 4] >> (a + 4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - a / 8, rc_usage_rgb_str[n]);
		n = (m[0x0288 / 4] >> (a + 5)) & 7;
		printf("  %c_MAPPING %s\n\r", 'A' + 3 - a / 8, rc_mapping_str[n]);
	}
	for (a = 24; a >= 8; a = a - 8) {
		n = (m[0x028c / 4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r", 'E' + 3 - a / 8, rc_variable_str[n]);
		n = (m[0x028c / 4] >> (a + 4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r", 'E' + 3 - a / 8, rc_usage_rgb_str[n]);
		n = (m[0x028c / 4] >> (a + 5)) & 7;
		printf("  %c_MAPPING %s\n\r", 'E' + 3 - a / 8, rc_mapping_str[n]);
	}
	n = (m[0x028c / 4] >> 7) & 1;
	printf(" color sum clamp: %d\n\r", n);
}
#endif

void nv2a_renderer::extract_packed_float(uint32_t data, float &first, float &second, float &third)
{
	float f1, f2, f3;
	int p1, p2, p3;

	p1 = data & 0x7ff;
	if (p1 & 0x400)
		f1 = (float)(p1 - 0x800) / 1023.0;
	else
		f1 = (float)p1 / 1023.0;
	p2 = (data >> 11) & 0x7ff;
	if (p2 & 0x400)
		f2 = (float)(p2 - 0x800) / 1023.0;
	else
		f2 = (float)p2 / 1023.0;
	p3 = (data >> 22) & 0x3ff;
	if (p3 & 0x200)
		f3 = (float)(p3 - 0x400) / 511.0;
	else
		f3 = (float)p3 / 511.0;
	first = f1;
	second = f2;
	third = f3;
}

void nv2a_renderer::read_vertex(address_space &space, offs_t address, vertex_nv &vertex, int attrib)
{
	uint32_t u;

	switch (vertexbuffer.type[attrib])
	{
	case 0x02: // none
		return;
	case 0x12: // float1
		vertex.attribute[attrib].iv[0] = space.read_dword(address + 0);
		vertex.attribute[attrib].fv[1] = 0;
		vertex.attribute[attrib].fv[2] = 0;
		vertex.attribute[attrib].fv[3] = 1.0;
		break;
	case 0x16: // normpacked3
		u = space.read_dword(address + 0);
		extract_packed_float(u, vertex.attribute[attrib].fv[0], vertex.attribute[attrib].fv[1], vertex.attribute[attrib].fv[2]);
		vertex.attribute[attrib].fv[3] = 1.0;
		break;
	case 0x22: // float2
		vertex.attribute[attrib].iv[0] = space.read_dword(address + 0);
		vertex.attribute[attrib].iv[1] = space.read_dword(address + 4);
		vertex.attribute[attrib].fv[2] = 0;
		vertex.attribute[attrib].fv[3] = 1.0;
		break;
	case 0x32: // float3
		vertex.attribute[attrib].iv[0] = space.read_dword(address + 0);
		vertex.attribute[attrib].iv[1] = space.read_dword(address + 4);
		vertex.attribute[attrib].iv[2] = space.read_dword(address + 8);
		vertex.attribute[attrib].fv[3] = 1.0;
		break;
	case 0x40: // d3dcolor
		u = space.read_dword(address + 0);
		// aarrggbb -> (rr, gg, bb, aa)
		vertex.attribute[attrib].fv[2] = (u & 0xff) / 255.0;
		u = u >> 8;
		vertex.attribute[attrib].fv[1] = (u & 0xff) / 255.0;
		u = u >> 8;
		vertex.attribute[attrib].fv[0] = (u & 0xff) / 255.0;
		u = u >> 8;
		vertex.attribute[attrib].fv[3] = (u & 0xff) / 255.0;
		break;
	case 0x42: // float4
		vertex.attribute[attrib].iv[0] = space.read_dword(address + 0);
		vertex.attribute[attrib].iv[1] = space.read_dword(address + 4);
		vertex.attribute[attrib].iv[2] = space.read_dword(address + 8);
		vertex.attribute[attrib].iv[3] = space.read_dword(address + 12);
		break;
	default:
		machine().logerror("Yet unsupported vertex data type %x\n\r", vertexbuffer.type[attrib]);
		return;
	}
}

/* Read vertices data from system memory. Method 0x1800 and 0x1808 */
int nv2a_renderer::read_vertices_0x180x(address_space &space, int destination, uint32_t address, int limit)
{
	uint32_t m, n;
	int a, b;

	n = destination;
	for (m = 0; m < limit; m++) {
		memcpy(&vertex_software[n], &persistvertexattr, sizeof(persistvertexattr));
		b = vertexbuffer.enabled;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, vertexbuffer.address[a] + vertex_indexes[indexesleft_first] * vertexbuffer.stride[a], vertex_software[n], a);
			}
			b = b >> 1;
		}
		n = (n + 1) & 1023;
		indexesleft_first = (indexesleft_first + 1) & 1023;
		indexesleft_count--;
	}
	return limit;
}

/* Read vertices data from system memory. Method 0x1810 */
int nv2a_renderer::read_vertices_0x1810(address_space &space, int destination, int offset, int limit)
{
	uint32_t m, n;
	int a, b;

	n = destination;
	for (m = 0; m < limit; m++) {
		memcpy(&vertex_software[n], &persistvertexattr, sizeof(persistvertexattr));
		b = vertexbuffer.enabled;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, vertexbuffer.address[a] + (m + offset) * vertexbuffer.stride[a], vertex_software[n], a);
			}
			b = b >> 1;
		}
		n = (n + 1) & 1023;
	}
	return m;
}

/* Read vertices data from system memory. Method 0x1818 */
int nv2a_renderer::read_vertices_0x1818(address_space &space, int destination, uint32_t address, int limit)
{
	uint32_t m, n, vwords;
	int a, b;

	n = destination;
	vwords = vertexbuffer.offset[16];
	for (m = 0; m < limit; m++) {
		memcpy(&vertex_software[n], &persistvertexattr, sizeof(persistvertexattr));
		b = vertexbuffer.enabled;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, address + vertexbuffer.offset[a] * 4, vertex_software[n], a);
			}
			b = b >> 1;
		}
		n = (n + 1) & 1023;
		address = address + vwords * 4;
	}
	return (int)(m*vwords);
}

void nv2a_renderer::compute_supersample_factors(float &horizontal, float &vertical)
{
	float mx, my;

	mx = 1;
	my = 1;
	switch (((antialias_control & 1) << 2) | antialiasing_rendertarget)
	{
	case 0:
		mx = my = 1;
		break;
	case 1:
		mx = 2; my = 1;
		break;
	case 2:
		mx = my = 2;
		break;
	case 4:
		mx = my = 1;
		break;
	case 5:
		mx = 2;
		my = 1;
		break;
	case 6:
		mx = 2;
		my = 2;
		break;
	default:
		mx = my = 1;
	}
	horizontal = mx;
	vertical = my;
}

void nv2a_renderer::convert_vertices(vertex_nv *source, nv2avertex_t *destination)
{
	vertex_nv vert;
	int u;
	float v[4];
	double c;

	// take each vertex with its attributes and obtain data for drawing
	// should use either the vertex program or transformation matrices
	if (vertex_pipeline == 4) {
		// transformation matrices
		// this part needs more testing
		for (int i = 0; i < 4; i++) {
			v[i] = 0;
			for (int j = 0; j < 4; j++)
				v[i] += matrix.composite[i][j] * source->attribute[0].fv[j];
		};
		destination->w = v[3];
		destination->x = (v[0] / v[3]) * supersample_factor_x; // source->attribute[0].fv[0];
		destination->y = (v[1] / v[3]) * supersample_factor_y; // source->attribute[0].fv[1];
		c = v[3];
		if (c == 0)
			c = FLT_MIN;
		c = 1.0f / c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_1W] = c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_Z] = v[2] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_R] = source->attribute[3].fv[0] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_G] = source->attribute[3].fv[1] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_B] = source->attribute[3].fv[2] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_A] = source->attribute[3].fv[3] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_R] = source->attribute[4].fv[0] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_G] = source->attribute[4].fv[1] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_B] = source->attribute[4].fv[2] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_A] = source->attribute[4].fv[3] * c;
		for (u = 0; u < 4; u++) {
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S + u * 4] = source->attribute[9 + u].fv[0] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T + u * 4] = source->attribute[9 + u].fv[1] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R + u * 4] = source->attribute[9 + u].fv[2] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q + u * 4] = source->attribute[9 + u].fv[3] * c;
		}
	}
	else {
		// vertex program
		// run vertex program
		vertexprogram.exec.process(vertexprogram.start_instruction, source, &vert, 1);
		// the output of the vertex program has the perspective divide, viewport scale and offset already applied
		// copy data for poly.h
		destination->w = vert.attribute[0].fv[3];
		destination->x = (vert.attribute[0].fv[0] - 0.53125f) * supersample_factor_x;
		destination->y = (vert.attribute[0].fv[1] - 0.53125f) * supersample_factor_y;
		c = destination->w;
		if (c == 0)
			c = FLT_MIN;
		c = 1.0f / c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_1W] = c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_Z] = vert.attribute[0].fv[2]; // already divided by w
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_R] = vert.attribute[3].fv[0] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_G] = vert.attribute[3].fv[1] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_B] = vert.attribute[3].fv[2] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_COLOR_A] = vert.attribute[3].fv[3] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_R] = vert.attribute[4].fv[0] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_G] = vert.attribute[4].fv[1] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_B] = vert.attribute[4].fv[2] * c;
		destination->p[(int)VERTEX_PARAMETER::PARAM_SECONDARY_COLOR_A] = vert.attribute[4].fv[3] * c;
		for (u = 0; u < 4; u++) {
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_S + u * 4] = vert.attribute[9 + u].fv[0] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_T + u * 4] = vert.attribute[9 + u].fv[1] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_R + u * 4] = vert.attribute[9 + u].fv[2] * c;
			destination->p[(int)VERTEX_PARAMETER::PARAM_TEXTURE0_Q + u * 4] = vert.attribute[9 + u].fv[3] * c;
		}
	}
}

void nv2a_renderer::clear_render_target(int what, uint32_t value)
{
	int xi, yi, xf, yf;
	int x, y;
	uint32_t color;
	uint8_t *addr;
	uint32_t mask;
	uint32_t offset;

	if (what == 0)
		return;
	mask = 0;
	switch (colorformat_rendertarget) {
	case NV2A_COLOR_FORMAT::R5G6B5:
		if (what & 1)
			mask = 0x1f;
		if (what & 2)
			mask = mask | 0x07e0;
		if (what & 4)
			mask = mask | 0xf800;
		break;
	case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
	case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
		for (x = 3; x >= 0; x--) {
			if (what & 8)
				mask = (mask << 8) | 255;
			what = what << 1;
		}
		break;
	case NV2A_COLOR_FORMAT::A8R8G8B8:
		for (x = 3; x >= 0; x--) {
			if (what & 8)
				mask = (mask << 8) | 255;
			what = what << 1;
		}
		break;
	case NV2A_COLOR_FORMAT::B8:
		if (what & 1)
			mask = 255;
		break;
	default:
		return;
	}
	xi = clear_rendertarget.left()*supersample_factor_x;
	yi = clear_rendertarget.top()*supersample_factor_y;
	xf = clear_rendertarget.right()*supersample_factor_x;
	yf = clear_rendertarget.bottom()*supersample_factor_y;
	if ((xi < limits_rendertarget.left()) && (xf < limits_rendertarget.left()))
		return;
	if ((xi > limits_rendertarget.right()) && (xf > limits_rendertarget.right()))
		return;
	if ((yi < limits_rendertarget.top()) && (yf < limits_rendertarget.top()))
		return;
	if ((yi > limits_rendertarget.bottom()) && (yf > limits_rendertarget.bottom()))
		return;
	if (xi < limits_rendertarget.left())
		xi = limits_rendertarget.left();
	if (xf > limits_rendertarget.right())
		xf = limits_rendertarget.right();
	if (yi < limits_rendertarget.top())
		yi = limits_rendertarget.top();
	if (yf > limits_rendertarget.bottom())
		yf = limits_rendertarget.bottom();
	if (type_rendertarget == NV2A_RT_TYPE::SWIZZLED)
		offset = (dilated0[dilate_rendertarget][xf] + dilated1[dilate_rendertarget][yf]) * bytespixel_rendertarget;
	else // type_rendertarget == LINEAR
		offset = pitch_rendertarget * yf + xf * bytespixel_rendertarget;
	addr = (uint8_t *)rendertarget + offset;
	if ((addr < basemempointer) || (addr > topmempointer))
	{
		machine().logerror("Bad memory pointer computed in clear_render_target !\n");
		return;
	}

	for (y = yi; y <= yf; y++)
		for (x = xi; x <= xf; x++) {
			if (type_rendertarget == NV2A_RT_TYPE::SWIZZLED)
				offset = (dilated0[dilate_rendertarget][x] + dilated1[dilate_rendertarget][y]) * bytespixel_rendertarget;
			else // type_rendertarget == LINEAR
				offset = pitch_rendertarget * y + x * bytespixel_rendertarget;
			switch (colorformat_rendertarget) {
			case NV2A_COLOR_FORMAT::R5G6B5:
				addr = (uint8_t *)rendertarget + offset;
				color = *((uint16_t *)addr);
				break;
			case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
			case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
				addr = (uint8_t *)rendertarget + offset;
				color = *((uint32_t *)addr);
				break;
			case NV2A_COLOR_FORMAT::A8R8G8B8:
				addr = (uint8_t *)rendertarget + offset;
				color = *((uint32_t *)addr);
				break;
			case NV2A_COLOR_FORMAT::B8:
				addr = (uint8_t *)rendertarget + offset;
				color = *addr;
				break;
			default:
				return;
			}
			color = (color & ~mask) | (value & mask);
			switch (colorformat_rendertarget) {
			case NV2A_COLOR_FORMAT::R5G6B5:
				*((uint16_t *)addr) = color;
				break;
			case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
			case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
				*((uint32_t *)addr) = color;
				break;
			case NV2A_COLOR_FORMAT::A8R8G8B8:
				*((uint32_t *)addr) = color;
				break;
			case NV2A_COLOR_FORMAT::B8:
				*addr = color;
				break;
			default:
				return;
			}
		}
	LOG("clearscreen (%d,%d)-(%d,%d)\n\r",xi,yi,xf,yf);
}

void nv2a_renderer::clear_depth_buffer(int what, uint32_t value)
{
	int xi, yi, xf, yf;
	int x, y;
	uint32_t color;
	uint8_t *addr;
	uint32_t mask;
	uint32_t offset;
	uint32_t bpp;

	if (what == 0)
		return;
	mask = 0;
	switch (depthformat_rendertarget) {
	case NV2A_RT_DEPTH_FORMAT::Z24S8:
		if (what & 1)
			mask = 0xffffff00;
		if (what & 2)
			mask = mask | 0xff;
		bpp = 4;
		break;
	case NV2A_RT_DEPTH_FORMAT::Z16:
		if (what & 1)
			mask = 0xffff;
		bpp = 2;
		break;
	default:
		return;
	}
	xi = clear_rendertarget.left()*supersample_factor_x;
	yi = clear_rendertarget.top()*supersample_factor_y;
	xf = clear_rendertarget.right()*supersample_factor_x;
	yf = clear_rendertarget.bottom()*supersample_factor_y;
	if ((xi < limits_rendertarget.left()) && (xf < limits_rendertarget.left()))
		return;
	if ((xi > limits_rendertarget.right()) && (xf > limits_rendertarget.right()))
		return;
	if ((yi < limits_rendertarget.top()) && (yf < limits_rendertarget.top()))
		return;
	if ((yi > limits_rendertarget.bottom()) && (yf > limits_rendertarget.bottom()))
		return;
	if (xi < limits_rendertarget.left())
		xi = limits_rendertarget.left();
	if (xf > limits_rendertarget.right())
		xf = limits_rendertarget.right();
	if (yi < limits_rendertarget.top())
		yi = limits_rendertarget.top();
	if (yf > limits_rendertarget.bottom())
		yf = limits_rendertarget.bottom();
	offset = pitch_depthbuffer * yf + xf * bpp;
	addr = (uint8_t *)depthbuffer + offset;
	if ((addr < basemempointer) || (addr > topmempointer))
	{
		machine().logerror("Bad memory pointer computed in clear_depth_buffer !\n");
		return;
	}

	for (y = yi; y <= yf; y++)
		for (x = xi; x <= xf; x++) {
			offset = pitch_depthbuffer * y + x * bpp;
			switch (depthformat_rendertarget) {
			case NV2A_RT_DEPTH_FORMAT::Z16:
				addr = (uint8_t *)depthbuffer + offset;
				color = *((uint16_t *)addr);
				break;
			case NV2A_RT_DEPTH_FORMAT::Z24S8:
				addr = (uint8_t *)depthbuffer + offset;
				color = *((uint32_t *)addr);
				break;
			default:
				return;
			}
			color = (color & ~mask) | (value & mask);
			switch (depthformat_rendertarget) {
			case NV2A_RT_DEPTH_FORMAT::Z16:
				addr = (uint8_t *)depthbuffer + offset;
				*((uint16_t *)addr) = color;
				break;
			case NV2A_RT_DEPTH_FORMAT::Z24S8:
				addr = (uint8_t *)depthbuffer + offset;
				*((uint32_t *)addr) = color;
				break;
			default:
				return;
			}
		}
}

uint32_t nv2a_renderer::render_triangle_culling(const rectangle &cliprect, nv2avertex_t &_v1, nv2avertex_t &_v2, nv2avertex_t &_v3)
{
	float areax2;
	NV2A_GL_CULL_FACE face = NV2A_GL_CULL_FACE::FRONT;

	//LOG("triangle culling %f,%f %f,%f %f,%f\n\r",_v1.x, _v1.y, _v2.x, _v2.y, _v3.x, _v3.y);
	if (backface_culling_enabled == false)
		return rasterizer.render_triangle<int(VERTEX_PARAMETER::ALL)>(cliprect, render_spans_callback, _v1, _v2, _v3);
	if (backface_culling_culled == NV2A_GL_CULL_FACE::FRONT_AND_BACK)
	{
		triangles_bfculled++;
		return 0;
	}
	areax2 = _v1.x*(_v2.y - _v3.y) + _v2.x*(_v3.y - _v1.y) + _v3.x*(_v1.y - _v2.y);
	if (areax2 == 0.0f) {
		triangles_bfculled++;
		return 0;
	}
	if (backface_culling_winding == NV2A_GL_FRONT_FACE::CCW)
	{
		if (-areax2 <= 0)
			face = NV2A_GL_CULL_FACE::BACK;
		else
			face = NV2A_GL_CULL_FACE::FRONT;
	} else
	{
		if (areax2 <= 0)
			face = NV2A_GL_CULL_FACE::BACK;
		else
			face = NV2A_GL_CULL_FACE::FRONT;
	}
	if (face == NV2A_GL_CULL_FACE::FRONT)
		if (backface_culling_culled == NV2A_GL_CULL_FACE::BACK)
			return rasterizer.render_triangle<int(VERTEX_PARAMETER::ALL)>(cliprect, render_spans_callback, _v1, _v2, _v3);
	if (face == NV2A_GL_CULL_FACE::BACK)
		if (backface_culling_culled == NV2A_GL_CULL_FACE::FRONT)
			return rasterizer.render_triangle<int(VERTEX_PARAMETER::ALL)>(cliprect, render_spans_callback, _v1, _v2, _v3);
	triangles_bfculled++;
	return 0;
}

int nv2a_renderer::clip_triangle_w(nv2avertex_t vi[3], nv2avertex_t *vo)
{
	int idx_prev, idx_curr;
	int neg_prev, neg_curr;
	double tfactor;
	int idx;
	const double wthreshold = 0.000001;

	idx_prev = 2;
	idx_curr = 0;
	idx = 0;
	neg_prev = vi[idx_prev].w < wthreshold ? 1 : 0;
	while (idx_curr < 3)
	{
		neg_curr = vi[idx_curr].w < wthreshold ? 1 : 0;
		if (neg_curr ^ neg_prev)
		{
			tfactor = (wthreshold - vi[idx_prev].w) / (vi[idx_curr].w - vi[idx_prev].w);
			// compute values for the new intermediate point
			vo[idx].x = ((vi[idx_curr].x - vi[idx_prev].x) * tfactor) + vi[idx_prev].x;
			vo[idx].y = ((vi[idx_curr].y - vi[idx_prev].y) * tfactor) + vi[idx_prev].y;
			vo[idx].w = ((vi[idx_curr].w - vi[idx_prev].w) * tfactor) + vi[idx_prev].w;
			for (int n = 0; n < (int)VERTEX_PARAMETER::PARAM_Z; n++)
				vo[idx].p[n] = ((vi[idx_curr].p[n] - vi[idx_prev].p[n]) * tfactor) + vi[idx_prev].p[n];
			vo[idx].p[(int)VERTEX_PARAMETER::PARAM_Z] = ((vi[idx_curr].p[(int)VERTEX_PARAMETER::PARAM_Z] - vi[idx_prev].p[(int)VERTEX_PARAMETER::PARAM_Z]) * tfactor) + vi[idx_prev].p[(int)VERTEX_PARAMETER::PARAM_Z];
			vo[idx].p[(int)VERTEX_PARAMETER::PARAM_1W] = 1.0f / vo[idx].w;
			idx++;
		}
		if (neg_curr == 0)
		{
			vo[idx].x = vi[idx_curr].x;
			vo[idx].y = vi[idx_curr].y;
			vo[idx].w = vi[idx_curr].w;
			for (int n = 0; n < (int)VERTEX_PARAMETER::PARAM_Z; n++)
				vo[idx].p[n] = vi[idx_curr].p[n];
			vo[idx].p[(int)VERTEX_PARAMETER::PARAM_Z] = vi[idx_curr].p[(int)VERTEX_PARAMETER::PARAM_Z];
			vo[idx].p[(int)VERTEX_PARAMETER::PARAM_1W] = 1.0f / vo[idx].w;
			idx++;
		}
		neg_prev = neg_curr;
		idx_prev = idx_curr;
		idx_curr++;
	}
	return idx;
}

uint32_t nv2a_renderer::render_triangle_clipping(const rectangle &cliprect, nv2avertex_t &_v1, nv2avertex_t &_v2, nv2avertex_t &_v3)
{
	nv2avertex_t *vp[3];
	nv2avertex_t vi[3];
	nv2avertex_t vo[8];
	int nv;
	double c;

	if ((_v1.w > 0) && (_v2.w > 0) && (_v3.w > 0))
		return render_triangle_culling(cliprect, _v1, _v2, _v3);
	if (enable_clipping_w == false)
		return 0;
	if ((_v1.w <= 0) && (_v2.w <= 0) && (_v3.w <= 0))
		return 0;
	// assign the elements of the pointer array
	vp[0] = &_v1;
	vp[1] = &_v2;
	vp[2] = &_v3;
	// go back to the state before perpective divide
	if (vertex_pipeline == 4)
	{
		for (int n = 0; n < 3; n++)
		{
			c = vp[n]->w;
			vi[n].w = c;
			vi[n].x = (vp[n]->x / (double)supersample_factor_x) * c;
			vi[n].y = (vp[n]->y / (double)supersample_factor_y) * c;
			for (int nn = 0; nn <= (int)VERTEX_PARAMETER::PARAM_Z; nn++)
				vi[n].p[nn] = vp[n]->p[nn] * c;
		}
	} else
	{
		for (int n = 0; n < 3; n++)
		{
			c = vp[n]->w;
			vi[n].w = c;
			// remove perspective correct interpolate
			for (int nn = 0; nn < (int)VERTEX_PARAMETER::PARAM_Z; nn++)
				vi[n].p[nn] = vp[n]->p[nn] * c;
			// remove supersample
			vi[n].x = (vp[n]->x / supersample_factor_x) + 0.53125f;
			vi[n].y = (vp[n]->y / supersample_factor_y) + 0.53125f;
			// remove translate
			vi[n].x = vi[n].x - matrix.translate[0];
			vi[n].y = vi[n].y - matrix.translate[1];
			vi[n].p[(int)VERTEX_PARAMETER::PARAM_Z] = vp[n]->p[(int)VERTEX_PARAMETER::PARAM_Z] - matrix.translate[2];
			// remove perspective divide
			vi[n].x = vi[n].x * c;
			vi[n].y = vi[n].y * c;
			vi[n].p[(int)VERTEX_PARAMETER::PARAM_Z] = vi[n].p[(int)VERTEX_PARAMETER::PARAM_Z] * c;
		}
	}
	// do the clipping
	nv = clip_triangle_w(vi, vo);
	// screen coordinates for the new points
	if (vertex_pipeline == 4)
	{
		for (int n = 0; n < nv; n++)
		{
			c = 1 / vo[n].w;
			vo[n].x = vo[n].x * (double)supersample_factor_x * c;
			vo[n].y = vo[n].y * (double)supersample_factor_y * c;
			for (int nn = 0; nn <= (int)VERTEX_PARAMETER::PARAM_Z; nn++)
				vo[n].p[nn] = vo[n].p[nn] * c;
		}
	} else
	{
		for (int n = 0; n < nv; n++)
		{
			c = 1 / vo[n].w;
			// apply perspective divide
			vo[n].x = vo[n].x * c;
			vo[n].y = vo[n].y * c;
			vo[n].p[(int)VERTEX_PARAMETER::PARAM_Z] = vo[n].p[(int)VERTEX_PARAMETER::PARAM_Z] * c;
			// apply translate
			vo[n].x = vo[n].x + matrix.translate[0];
			vo[n].y = vo[n].y + matrix.translate[1];
			vo[n].p[(int)VERTEX_PARAMETER::PARAM_Z] = vo[n].p[(int)VERTEX_PARAMETER::PARAM_Z] + matrix.translate[2];
			// apply supersample
			vo[n].x = (vo[n].x - 0.53125f) * supersample_factor_x;
			vo[n].y = (vo[n].y - 0.53125f) * supersample_factor_y;
			// apply perspective correct interpolate
			for (int nn = 0; nn < (int)VERTEX_PARAMETER::PARAM_Z; nn++)
				vo[n].p[nn] = vo[n].p[nn] * c;
		}
	}
	for (int n = 1; n <= (nv - 2); n++)
		render_triangle_culling(cliprect, vo[0], vo[n], vo[n + 1]);
	return 0;
}

void nv2a_renderer::assemble_primitive(int source, int count)
{
	uint32_t pc = primitives_count;
	vertex_nv *v;

	for (; count > 0; count--) {
		v = &vertex_software[source];
		if (primitive_type == NV2A_BEGIN_END::QUADS) {
			convert_vertices(v, vertex_xy + ((vertex_count + vertex_accumulated) & 1023));
			vertex_accumulated++;
			if (vertex_accumulated == 4) {
				primitives_count++;
				vertex_accumulated = 0;
				render_triangle_clipping(limits_rendertarget, vertex_xy[vertex_count], vertex_xy[vertex_count + 1], vertex_xy[vertex_count + 2]);
				render_triangle_clipping(limits_rendertarget, vertex_xy[vertex_count], vertex_xy[vertex_count + 2], vertex_xy[vertex_count + 3]);
				vertex_count = (vertex_count + 4) & 1023;
				rasterizer.wait();
			}
		}
		else if (primitive_type == NV2A_BEGIN_END::TRIANGLES) {
			convert_vertices(v, vertex_xy + ((vertex_count + vertex_accumulated) & 1023));
			vertex_accumulated++;
			if (vertex_accumulated == 3) {
				primitives_count++;
				vertex_accumulated = 0;
				render_triangle_clipping(limits_rendertarget, vertex_xy[vertex_count], vertex_xy[(vertex_count + 1) & 1023], vertex_xy[(vertex_count + 2) & 1023]); // 4 rgba, 4 texture units 2 uv
				vertex_count = (vertex_count + 3) & 1023;
				rasterizer.wait();
			}
		}
		else if (primitive_type == NV2A_BEGIN_END::TRIANGLE_FAN) {
			if (vertex_accumulated == 0)
			{
				convert_vertices(v, vertex_xy + 1024);
				vertex_accumulated = 1;
			}
			else if (vertex_accumulated == 1)
			{
				convert_vertices(v, vertex_xy);
				vertex_accumulated = 2;
				vertex_count = 1;
			}
			else
			{
				primitives_count++;
				// if software sends the vertices 0 1 2 3 4 5 6
				// hardware will draw triangles made by (0,1,2) (0,2,3) (0,3,4) (0,4,5) (0,5,6)
				convert_vertices(v, vertex_xy + vertex_count);
				render_triangle_clipping(limits_rendertarget, vertex_xy[1024], vertex_xy[(vertex_count - 1) & 1023], vertex_xy[vertex_count]);
				vertex_count = (vertex_count + 1) & 1023;
				rasterizer.wait();
			}
		}
		else if (primitive_type == NV2A_BEGIN_END::TRIANGLE_STRIP) {
			if (vertex_accumulated == 0)
			{
				convert_vertices(v, vertex_xy);
				vertex_accumulated = 1;
			}
			else if (vertex_accumulated == 1)
			{
				convert_vertices(v, vertex_xy + 1);
				vertex_accumulated = 2;
				vertex_count = 2;
			}
			else
			{
				primitives_count++;
				// if software sends the vertices 0 1 2 3 4 5 6
				// hardware will draw triangles made by (0,1,2) (1,3,2) (2,3,4) (3,5,4) (4,5,6)
				convert_vertices(v, vertex_xy + vertex_count);
				if ((vertex_count & 1) == 0)
					render_triangle_clipping(limits_rendertarget, vertex_xy[(vertex_count - 2) & 1023], vertex_xy[(vertex_count - 1) & 1023], vertex_xy[vertex_count]);
				else
					render_triangle_clipping(limits_rendertarget, vertex_xy[(vertex_count - 2) & 1023], vertex_xy[vertex_count], vertex_xy[(vertex_count - 1) & 1023]);
				vertex_count = (vertex_count + 1) & 1023;
				rasterizer.wait();
			}
		}
		else if (primitive_type == NV2A_BEGIN_END::QUAD_STRIP) {
			if (vertex_accumulated == 0)
			{
				convert_vertices(v, vertex_xy);
				vertex_accumulated = 1;
			}
			else if (vertex_accumulated == 1)
			{
				convert_vertices(v, vertex_xy + 1);
				vertex_accumulated = 2;
				vertex_count = 0;
			}
			else
			{
				convert_vertices(v, vertex_xy + ((vertex_count + vertex_accumulated) & 1023));
				vertex_accumulated++;
				if (vertex_accumulated == 4)
				{
					primitives_count++;
					// if software sends the vertices 0 1 2 3 4 5 6 7
					// hardware will draw triangles made by (0,1,2) (2,1,3) (2,3,4) (4,3,5) (4,5,6) (6,5,7)
					render_triangle_clipping(limits_rendertarget, vertex_xy[vertex_count], vertex_xy[(vertex_count + 1) & 1023], vertex_xy[(vertex_count + 2) & 1023]);
					render_triangle_clipping(limits_rendertarget, vertex_xy[(vertex_count + 2) & 1023], vertex_xy[(vertex_count + 1) & 1023], vertex_xy[(vertex_count + 3) & 1023]);
					vertex_accumulated = 2;
					vertex_count = (vertex_count + 2) & 1023;
					rasterizer.wait();
				}
			}
		}
		else {
			if (vertex_count == 0)
				machine().logerror("Unsupported primitive %d\n", int(primitive_type));
			vertex_count++;
		}
		source = (source + 1) & 1023;
	}
	primitives_total_count += primitives_count - pc;
}

void nv2a_renderer::process_persistent_vertex()
{
	memcpy(&vertex_software[1025], &persistvertexattr, sizeof(persistvertexattr));
	assemble_primitive(1025, 1);
}

void nv2a_renderer::compute_limits_rendertarget(uint32_t chanel, uint32_t subchannel)
{
	uint32_t data;
	int x, w;
	int y, h;

	data = channel[chanel][subchannel].object.method[0x0200 / 4];
	x = data & 0xffff;
	w = (data >> 16) & 0xffff;
	x = x*supersample_factor_x;
	w = w*supersample_factor_x;
	limits_rendertarget.setx(x, x + w - 1);
	data = channel[chanel][subchannel].object.method[0x0204 / 4];
	y = data & 0xffff;
	h = (data >> 16) & 0xffff;
	y = y*supersample_factor_y;
	h = h*supersample_factor_y;
	limits_rendertarget.sety(y, y + h - 1);
}

void nv2a_renderer::compute_size_rendertarget(uint32_t chanel, uint32_t subchannel)
{
	size_rendertarget = pitch_rendertarget*(limits_rendertarget.bottom() + 1);
	size_depthbuffer = pitch_depthbuffer*(limits_rendertarget.bottom() + 1);
}

int nv2a_renderer::execute_method(address_space &space, uint32_t chanel, uint32_t subchannel, uint32_t method, uint32_t address, int &countlen)
{
	uint32_t data;

	data = space.read_dword(address);
	channel[chanel][subchannel].object.method[method / 4] = data;
	LOGMASKED(LOG_NV2A_VERBOSE, "A:%08X CH=%02d SCH=%02d MTHD:%08X D:%08X\n\r", address, chanel, subchannel, method, data);
	if (channel[chanel][subchannel].object.objclass == 0x97)
		return execute_method_3d(space, chanel, subchannel, method, address, data, countlen);
	if (channel[chanel][subchannel].object.objclass == 0x39) // 0180
		return execute_method_m2mf(space, chanel, subchannel, method, address, data, countlen);
	if (channel[chanel][subchannel].object.objclass == 0x62) // 0184 0188
		return execute_method_surf2d(space, chanel, subchannel, method, address, data, countlen);
	if (channel[chanel][subchannel].object.objclass == 0x9f) // 019c 02fc
		return execute_method_blit(space, chanel, subchannel, method, address, data, countlen);
	return 0;
}

int nv2a_renderer::execute_method_3d(address_space& space, uint32_t chanel, uint32_t subchannel, uint32_t maddress, uint32_t address, uint32_t data, int &countlen)
{
	if ((chanel != 0) || (subchannel != 0))
		return 0;
	if (maddress == 0x17fc) {
#if 0 // useful while debugging to see what coordinates have been used
		static int debugvc = 0;
		if (debugvc)
			if (data == 0)
			{
				printf("%d %d\n\r", (int)primitive_type, vertex_first);
				for (int n = 0; n < vertex_first; n++)
				{
					if (indexesleft_count > 0)
						printf("%d i:%d ", n, vertex_indexes[n]);
					else
						printf("%d ", n);
					printf("X:%f Y:%f Z:%f W:%f x:%f y:%f\n\r", vertex_software[n].attribute[0].fv[0], vertex_software[n].attribute[0].fv[1], vertex_software[n].attribute[0].fv[2], vertex_software[n].attribute[0].fv[3], vertex_xy[n].x, vertex_xy[n].y);
				}
			}
#endif
		vertex_count = 0;
		vertex_first = 0;
		vertex_accumulated = 0;
		indexesleft_count = 0;
		indexesleft_first = 0;
		primitives_count = 0;
		primitive_type = (NV2A_BEGIN_END)data;
		if (data == 0)
			primitives_batches_count++;
		else
		{
			if (((channel[chanel][subchannel].object.method[0x1e60 / 4] & 7) > 0) && (combiner.used != 0))
				render_spans_callback = nv2a_rasterizer::render_delegate(&nv2a_renderer::render_register_combiners, this);
			else if (texture[0].enabled)
				render_spans_callback = nv2a_rasterizer::render_delegate(&nv2a_renderer::render_texture_simple, this);
			else
				render_spans_callback = nv2a_rasterizer::render_delegate(&nv2a_renderer::render_color, this);
		}
		countlen--;
	}
	if (maddress == 0x1810) {
		// draw vertices
		int offset, count;
		uint32_t n;

		offset = data & 0xffffff;
		count = (data >> 24) & 0xff;
		LOG("vertex %d %d\n\r", offset, count);
		for (n = 0; n <= count; n++) {
			read_vertices_0x1810(space, vertex_first, n + offset, 1);
			assemble_primitive(vertex_first, 1);
			vertex_first = (vertex_first + 1) & 1023;
		}
		countlen--;
	}
	if ((maddress == 0x1800) || (maddress == 0x1808)) {
		int mult;

		if (maddress == 0x1800)
			mult = 2;
		else
			mult = 1;
		// vertices are selected from the vertex buffer using an array of indexes
		// each dword after 1800 contains two 16 bit index values to select the vartices
		// each dword after 1808 contains a 32 bit index value to select the vartices
		while (countlen > 0) {
			int n;

			data = space.read_dword(address);
			n = indexesleft_first + indexesleft_count;
			if (mult == 2) {
				vertex_indexes[n & 1023] = data & 0xffff;
				vertex_indexes[(n + 1) & 1023] = (data >> 16) & 0xffff;
				indexesleft_count = indexesleft_count + 2;
			}
			else {
				vertex_indexes[n & 1023] = data;
				indexesleft_count = indexesleft_count + 1;
			}
			address += 4;
			countlen--;
			read_vertices_0x180x(space, vertex_first, address, mult);
			assemble_primitive(vertex_first, mult);
			vertex_first = (vertex_first + mult) & 1023;
		}
	}
	if (maddress == 0x1818) {
		if (countlen == 0)
			machine().logerror("Method 0x1818 with 0 vertices\n");
		// vertices are taken from the next words, not from a vertex buffer
		// first send primitive type with 17fc
		// then countlen number of dwords with 1818
		// end with 17fc primitive type 0
		// at 1760 16 words specify the vertex format:for each possible vertex attribute the number of components (0=not present) and type of each
		while (countlen > 0) {
			int c;

			c = read_vertices_0x1818(space, vertex_first, address, 1);
			countlen = countlen - c;
			if (countlen < 0) {
				machine().logerror("Method 0x1818 missing %d words\n", -countlen);
				countlen = 0;
				break;
			}
			address = address + c * 4;
			assemble_primitive(vertex_first, 1);
			vertex_first = (vertex_first + 1) & 1023;
		}
	}
	if ((maddress >= 0x1880) && (maddress < 0x1900))
	{
		int v = maddress - 0x1880; // 16 couples,2 float per couple,16*2*4=128
		int attr = v >> 3;
		int comp = (v >> 2) & 1;

		persistvertexattr.attribute[attr].iv[comp] = data;
		if (comp == 1)
		{
			persistvertexattr.attribute[attr].fv[2] = 0;
			persistvertexattr.attribute[attr].fv[3] = 1;
			if (attr == 0)
				process_persistent_vertex();
		}
	}
	if ((maddress >= 0x1900) && (maddress < 0x1940))
	{
		int v = maddress - 0x1900; // 16 dwords,2 values per dword
		int attr = v >> 2;
		uint16_t d1 = data & 0xffff;
		uint16_t d2 = data >> 16;

		persistvertexattr.attribute[attr].fv[0] = (float)((int16_t)d1);
		persistvertexattr.attribute[attr].fv[1] = (float)((int16_t)d2);
		persistvertexattr.attribute[attr].fv[2] = 0;
		persistvertexattr.attribute[attr].fv[3] = 1;
		if (attr == 0)
			process_persistent_vertex();
	}
	if ((maddress >= 0x1940) && (maddress < 0x1980))
	{
		int v = maddress - 0x1940; // 16 dwords,4 values per dword
		int attr = v >> 2;
		uint8_t d1 = data & 255;
		uint8_t d2 = (data >> 8) & 255;
		uint8_t d3 = (data >> 16) & 255;
		uint8_t d4 = data >> 24;

		// if sending color dword is aabbggrr
		persistvertexattr.attribute[attr].fv[0] = (float)d1 / 255.0;
		persistvertexattr.attribute[attr].fv[1] = (float)d2 / 255.0;
		persistvertexattr.attribute[attr].fv[2] = (float)d3 / 255.0;
		persistvertexattr.attribute[attr].fv[3] = (float)d4 / 255.0;
		if (attr == 0)
			process_persistent_vertex();
	}
	if ((maddress >= 0x1980) && (maddress < 0x1a00))
	{
		int v = maddress - 0x1980; // 16 couples,4 values per couple,16*2*4=128
		int attr = v >> 3;
		int comp = (v >> 1) & 2;
		uint16_t d1 = data & 0xffff;
		uint16_t d2 = data >> 16;

		persistvertexattr.attribute[attr].fv[comp] = (float)((int16_t)d1);
		persistvertexattr.attribute[attr].fv[comp+1] = (float)((int16_t)d2);
		if (comp == 2)
			if (attr == 0)
				process_persistent_vertex();
	}
	if ((maddress >= 0x1a00) && (maddress < 0x1b00))
	{
		int v = maddress - 0x1a00; // 16 groups,4 float per group
		int attr = v >> 4;
		int comp = (v >> 2) & 3;

		persistvertexattr.attribute[attr].iv[comp] = data;
		if (comp == 3)
			if (attr == 0)
				process_persistent_vertex();
	}
	if ((maddress >= 0x1518) && (maddress < 0x1528))
	{
		int v = maddress - 0x1518;
		int comp = v >> 2;

		persistvertexattr.attribute[(int)NV2A_VERTEX_ATTR::POS].iv[comp] = data;
		if (comp == 3)
			process_persistent_vertex();
	}
	else if ((maddress >= 0x1500) && (maddress < 0x1590))
	{
		machine().logerror("Yet unsupported method %x\n\r", maddress);
	}
	if ((maddress >= 0x1720) && (maddress < 0x1760)) {
		int bit = maddress / 4 - 0x1720 / 4;

		if (data & 0x80000000)
			vertexbuffer.address[bit] = (data & 0x0fffffff) + dma_offset[7];
		else
			vertexbuffer.address[bit] = (data & 0x0fffffff) + dma_offset[6];
	}
	if ((maddress >= 0x1760) && (maddress < 0x17A0)) {
		int bit = maddress / 4 - 0x1760 / 4;

		vertexbuffer.type[bit] = data & 255;
		vertexbuffer.stride[bit] = (data >> 8) & 255;
		switch (vertexbuffer.type[bit])
		{
		case 0x02: // none
			vertexbuffer.words[bit] = 0;
			break;
		case 0x12: // float1
			vertexbuffer.words[bit] = 1;
			break;
		case 0x16: // normpacked3
			vertexbuffer.words[bit] = 1;
			break;
		case 0x22: // float2
			vertexbuffer.words[bit] = 2;
			break;
		case 0x32: // float3
			vertexbuffer.words[bit] = 3;
			break;
		case 0x40: // d3dcolor
			vertexbuffer.words[bit] = 1;
			break;
		case 0x42: // float4
			vertexbuffer.words[bit] = 4;
			break;
		default:
			machine().logerror("Yet unsupported vertex data type %x\n\r", vertexbuffer.type[bit]);
			vertexbuffer.words[bit] = 0;
		}
		if (vertexbuffer.words[bit] > 0)
			vertexbuffer.enabled |= (1 << bit);
		else
			vertexbuffer.enabled &= ~(1 << bit);
		vertexbuffer.offset[0] = 0;
		for (int n = bit + 1; n <= 16; n++) {
			if ((vertexbuffer.enabled & (1 << (n - 1))) != 0)
				vertexbuffer.offset[n] = vertexbuffer.offset[n - 1] + vertexbuffer.words[n - 1];
			else
				vertexbuffer.offset[n] = vertexbuffer.offset[n - 1];
		}
		countlen--;
	}
	if ((maddress == 0x1d6c) || (maddress == 0x1a4))
		countlen--;
	if (maddress == 0x0308) {
		backface_culling_enabled = data != 0 ? true : false;
	}
	if (maddress == 0x03a0) {
		backface_culling_winding = (NV2A_GL_FRONT_FACE)data;
	}
	if (maddress == 0x039c) {
		backface_culling_culled = (NV2A_GL_CULL_FACE)data;
	}
	if (maddress == 0x0180) {
		geforce_read_dma_object(data, dma_offset[0], dma_size[0]);
	}
	if (maddress == 0x0184) {
		geforce_read_dma_object(data, dma_offset[1], dma_size[1]);
	}
	if (maddress == 0x0188) {
		geforce_read_dma_object(data, dma_offset[2], dma_size[2]);
	}
	if (maddress == 0x0190) {
		geforce_read_dma_object(data, dma_offset[3], dma_size[3]);
	}
	if (maddress == 0x0194) {
		geforce_read_dma_object(data, dma_offset[4], dma_size[4]);
	}
	if (maddress == 0x0198) {
		geforce_read_dma_object(data, dma_offset[5], dma_size[5]);
	}
	if (maddress == 0x019c) {
		geforce_read_dma_object(data, dma_offset[6], dma_size[6]);
	}
	if (maddress == 0x01a0) {
		geforce_read_dma_object(data, dma_offset[7], dma_size[7]);
	}
	if (maddress == 0x01a4) {
		geforce_read_dma_object(data, dma_offset[8], dma_size[8]);
	}
	if (maddress == 0x01a8) {
		geforce_read_dma_object(data, dma_offset[9], dma_size[9]);
	}
	if (maddress == 0x1d70) {
		// with 1d70 write the value at offest [1d6c] inside dma object [1a4]
		uint32_t offset, base;
		uint32_t dmahand, dmaoff, smasiz;

		offset = channel[chanel][subchannel].object.method[0x1d6c / 4];
		dmahand = channel[chanel][subchannel].object.method[0x1a4 / 4];
		geforce_read_dma_object(dmahand, dmaoff, smasiz);
		base = dmaoff;
		space.write_dword(base + offset, data);
		// software expects to find the parameter of this method at pgraph offset b10
		pgraph[0xb10 / 4] = data << 2;
		countlen--;
	}
	if (maddress == 0x1d7c) {
		antialias_control = data;
		compute_supersample_factors(supersample_factor_x, supersample_factor_y);
		compute_limits_rendertarget(chanel, subchannel);
		countlen--;
	}
	if (maddress == 0x1d98) {
		int x, w;

		x = data & 0xffff;
		w = (data >> 16) & 0xffff;
		clear_rendertarget.setx(x, w);
		countlen--;
	}
	if (maddress == 0x1d9c) {
		int y, h;

		y = data & 0xffff;
		h = (data >> 16) & 0xffff;
		clear_rendertarget.sety(y, h);
		countlen--;
	}
	if (maddress == 0x1d94) {
		// possible buffers: color, depth, stencil
		// clear framebuffer
		clear_render_target((data >> 4) & 15, channel[chanel][subchannel].object.method[0x1d90 / 4]);
		clear_depth_buffer(data & 3, channel[chanel][subchannel].object.method[0x1d8c / 4]);
		countlen--;
	}
	if ((maddress >= 0x02c0) && (maddress < 0x2e0)) {
		int x, w, i;

		i = (maddress - 0x2c0) / 4;
		x = data & 0xffff;
		w = (data >> 16) & 0xffff;
		clippingwindows[i].setx(x, x + w - 1);
	}
	if ((maddress >= 0x02e0) && (maddress < 0x300)) {
		int y, h, i;

		i = (maddress - 0x2e0) / 4;
		y = data & 0xffff;
		h = (data >> 16) & 0xffff;
		clippingwindows[i].sety(y, y + h - 1);
	}
	if (maddress == 0x0200) {
		compute_limits_rendertarget(chanel, subchannel);
		compute_size_rendertarget(chanel, subchannel);
	}
	if (maddress == 0x0204) {
		compute_limits_rendertarget(chanel, subchannel);
		compute_size_rendertarget(chanel, subchannel);
	}
	if (maddress == 0x0208) {
		log2height_rendertarget = (data >> 24) & 255;
		log2width_rendertarget = (data >> 16) & 255;
		antialiasing_rendertarget = (data >> 12) & 15;
		type_rendertarget = (NV2A_RT_TYPE)((data >> 8) & 15);
		depthformat_rendertarget = (NV2A_RT_DEPTH_FORMAT)((data >> 4) & 15);
		colorformat_rendertarget = (NV2A_COLOR_FORMAT)((data >> 0) & 15);
		compute_supersample_factors(supersample_factor_x, supersample_factor_y);
		compute_limits_rendertarget(chanel, subchannel);
		compute_size_rendertarget(chanel, subchannel);
/* for debugging
        if (limits_rendertarget.max_x == 1023)
            type_rendertarget = NV2A_RT_TYPE::LINEAR;
*/
		switch (colorformat_rendertarget) {
		case NV2A_COLOR_FORMAT::R5G6B5:
			bytespixel_rendertarget = 2;
			break;
		case NV2A_COLOR_FORMAT::X8R8G8B8_Z8R8G8B8:
		case NV2A_COLOR_FORMAT::X8R8G8B8_X8R8G8B8:
		case NV2A_COLOR_FORMAT::A8R8G8B8:
			bytespixel_rendertarget = 4;
			break;
		case NV2A_COLOR_FORMAT::B8:
			bytespixel_rendertarget = 1;
			break;
		default:
			machine().logerror("Unknown render target color format %d\n\r", int(colorformat_rendertarget));
			bytespixel_rendertarget = 4;
			break;
		}
		dilate_rendertarget = dilatechose[(log2width_rendertarget << 4) + log2height_rendertarget];
	}
	if (maddress == 0x020c) {
		pitch_rendertarget=data & 0xffff;
		pitch_depthbuffer=(data >> 16) & 0xffff;
		compute_size_rendertarget(chanel, subchannel);
		LOG("Pitch color %04X zbuffer %04X\n\r", pitch_rendertarget, pitch_depthbuffer);
		countlen--;
	}
	if (maddress == 0x0100) {
		countlen--;
		if (data != 0) {
			LOG("Software method %04x\n", data);
			pgraph[0x704 / 4] = 0x100 | (chanel << 20) | (subchannel << 16);
			pgraph[0x708 / 4] = data;
			pgraph[0x100 / 4] |= 1;
			pgraph[0x108 / 4] |= 1;
			if (update_interrupts() == true)
				irq_callback(1); // IRQ 3
			else
				irq_callback(0); // IRQ 3
			return 2;
		}
		else
			return 0;
	}
	if (maddress == 0x0130) {
		countlen--;
		if (enable_waitvblank == true)
			return 1; // block until next vblank
		else
			return 0;
	}
	if (maddress == 0x1d8c) {
		countlen--;
		// it is used to specify the clear value for the depth buffer (zbuffer)
		// but also as a parameter for interrupt routines
		pgraph[0x1a88 / 4] = data;
	}
	if (maddress == 0x1d90) {
		countlen--;
		// it is used to specify the clear value for the color buffer
		// but also as a parameter for interrupt routines
		pgraph[0x186c / 4] = data;
	}
	if (maddress == 0x0210) {
		// framebuffer offset
		old_rendertarget = rendertarget;
		// To see it with the image watch extension: @mem(0x000002d2263af060, UINT8, 4, 640, 480, 2560)
		rendertarget = (uint32_t *)direct_access_ptr(data);
		LOG("Render target at %08X\n\r", data);
		countlen--;
	}
	if (maddress == 0x0214) {
		// zbuffer offset ?
		depthbuffer = (uint32_t *)direct_access_ptr(data);
		LOG("Depth buffer at %08X\n\r",data);
		if ((data == 0) || (data > 0x7ffffffc))
			depth_write_enabled = false;
		else if (channel[chanel][subchannel].object.method[0x035c / 4] != 0)
			depth_write_enabled = true;
		else
			depth_write_enabled = false;
		countlen--;
	}
	if (maddress == 0x0300) {
		alpha_test_enabled = data != 0;
	}
	if (maddress == 0x033c) {
		alpha_func = (NV2A_COMPARISON_OP)data;
	}
	if (maddress == 0x0340) {
		alpha_reference = data;
	}
	if (maddress == 0x0304) {
		if (logical_operation_enabled)
			blending_enabled = false;
		else
			blending_enabled = data != 0;
	}
	if (maddress == 0x030c) {
		depth_test_enabled = data != 0;
	}
	if (maddress == 0x0354) {
		depth_function = (NV2A_COMPARISON_OP)data;
	}
	if (maddress == 0x0358) {
		//color_mask = data;
		if (data & 0x000000ff)
			data |= 0x000000ff;
		if (data & 0x0000ff00)
			data |= 0x0000ff00;
		if (data & 0x00ff0000)
			data |= 0x00ff0000;
		if (data & 0xff000000)
			data |= 0xff000000;
		color_mask = data;
	}
	if (maddress == 0x035c) {
		uint32_t g = channel[chanel][subchannel].object.method[0x0214 / 4];
		depth_write_enabled = data != 0;
		if ((g == 0) || (g > 0x7ffffffc))
			depth_write_enabled = false;
	}
	if (maddress == 0x032c) {
		stencil_test_enabled = data != 0;
	}
	if (maddress == 0x0364) {
		stencil_func = (NV2A_COMPARISON_OP)data;
	}
	if (maddress == 0x0368) {
		if (data > 255)
			data = 255;
		stencil_ref = data;
	}
	if (maddress == 0x036c) {
		stencil_mask = data;
	}
	if (maddress == 0x0370) {
		stencil_op_fail = (NV2A_STENCIL_OP)data;
	}
	if (maddress == 0x0374) {
		stencil_op_zfail = (NV2A_STENCIL_OP)data;
	}
	if (maddress == 0x0378) {
		stencil_op_zpass = (NV2A_STENCIL_OP)data;
	}
	if (maddress == 0x0344) {
		blend_function_source = (NV2A_BLEND_FACTOR)data;
	}
	if (maddress == 0x0348) {
		blend_function_destination = (NV2A_BLEND_FACTOR)data;
	}
	if (maddress == 0x034c) {
		blend_color = data;
	}
	if (maddress == 0x0350) {
		blend_equation = (NV2A_BLEND_EQUATION)data;
	}
	if (maddress == 0x0d40) {
		if (data != 0)
			blending_enabled = false;
		else
			blending_enabled = channel[chanel][subchannel].object.method[0x0304 / 4] != 0;
		logical_operation_enabled = data != 0;
	}
	if (maddress == 0x0d44) {
		logical_operation = (NV2A_LOGIC_OP)data;
	}
	// Texture Units
	if ((maddress >= 0x1b00) && (maddress < 0x1c00)) {
		int unit;//,off;

		unit = (maddress >> 6) & 3;
		//off=maddress & 0xc0;
		maddress = maddress & ~0xc0;
		if (maddress == 0x1b00) {
			uint32_t offset;//,base;
			//uint32_t dmahand,dmaoff,dmasiz;

			offset = data;
			texture[unit].buffer = direct_access_ptr(offset);
			/*if (dma0 != 0) {
			    dmahand=channel[channel][subchannel].object.method[0x184/4];
			    geforce_read_dma_object(dmahand,dmaoff,dmasiz);
			} else if (dma1 != 0) {
			    dmahand=channel[channel][subchannel].object.method[0x188/4];
			    geforce_read_dma_object(dmahand,dmaoff,dmasiz);
			}*/
		}
		if (maddress == 0x1b04) {
			int basesizeu, basesizev, basesizew, format;
			bool rectangle;

			texture[unit].dma0 = (data >> 0) & 1;
			texture[unit].dma1 = (data >> 1) & 1;
			texture[unit].cubic = (data >> 2) & 1;
			texture[unit].noborder = (data >> 3) & 1;
			texture[unit].dims = (data >> 4) & 15;
			texture[unit].mipmap = (data >> 19) & 1;
			format = (data >> 8) & 255;
			basesizeu = (data >> 20) & 15;
			basesizev = (data >> 24) & 15;
			basesizew = (data >> 28) & 15;
			texture[unit].sizes = 1 << basesizeu;
			texture[unit].sizet = 1 << basesizev;
			texture[unit].sizer = 1 << basesizew;
			texture[unit].dilate = dilatechose[(basesizeu << 4) + basesizev];
			texture[unit].format = (NV2A_TEX_FORMAT)format;
			switch (texture[unit].format)
			{
			case NV2A_TEX_FORMAT::A1R5G5B5_RECT:
			case NV2A_TEX_FORMAT::R5G6B5_RECT:
			case NV2A_TEX_FORMAT::A8R8G8B8_RECT:
			case NV2A_TEX_FORMAT::DSDT8_RECT:
			case NV2A_TEX_FORMAT::A4R4G4B4_RECT:
			case NV2A_TEX_FORMAT::R8G8B8_RECT:
			case NV2A_TEX_FORMAT::A8L8_RECT:
			case NV2A_TEX_FORMAT::Z24_RECT:
			case NV2A_TEX_FORMAT::Z16_RECT:
			case NV2A_TEX_FORMAT::HILO16_RECT:
			case NV2A_TEX_FORMAT::SIGNED_HILO8_RECT:
				rectangle = true;
				break;
			default:
				rectangle = false;
			}
			texture[unit].rectangle = rectangle;
			if (debug_grab_texttype == format) {
				FILE *f;
				int written;

				debug_grab_texttype = -1;
				f = fopen(debug_grab_textfile, "wb");
				if (f) {
					written = (int)fwrite(texture[unit].buffer, texture[unit].sizes * texture[unit].sizet * 4, 1, f);
					fclose(f);
					machine().logerror("Written %d bytes of texture to specified file\n", written);
				}
				else
					machine().logerror("Unable to save texture to specified file\n");
			}
		}
		if (maddress == 0x1b08) {
			texture[unit].addrmodes = (data >> 0) & 15;
			texture[unit].addrmodet = (data >> 8) & 15;
			texture[unit].addrmoder = (data >> 16) & 15;
		}
		if (maddress == 0x1b0c) {
			texture[unit].colorkey = (data >> 0) & 3;
			texture[unit].imagefield = (data >> 3) & 1;
			texture[unit].aniso = (data >> 4) & 3;
			texture[unit].mipmapmaxlod = (data >> 6) & 0xfff;
			texture[unit].mipmapminlod = (data >> 18) & 0xfff;
			// enable texture ?
			texture[unit].enabled = (data >> 30) & 3;
		}
		if (maddress == 0x1b10) {
			texture[unit].rectangle_pitch = data >> 16;
		}
		if (maddress == 0x1b14) {
			LOG("Texture filtering set to %08X\n\r", data);
			bilinear_filter = true;
		}
		if (maddress == 0x1b1c) {
			texture[unit].rectheight = data & 0xffff;
			texture[unit].rectwidth = data >> 16;
		}
		countlen--;
	}
	if (maddress == 0x1e70) {
		texture[0].mode = data & 31;
		texture[1].mode = (data >> 5) & 31;
		texture[2].mode = (data >> 10) & 31;
		texture[3].mode = (data >> 15) & 31;
	}
	// projection matrix
	if ((maddress >= 0x0440) && (maddress < 0x0480)) {
		maddress = (maddress - 0x0440) / 4;
		*(uint32_t *)(&matrix.projection[maddress >> 2][maddress & 3]) = data;
		countlen--;
	}
	// modelview matrix
	if ((maddress >= 0x0480) && (maddress < 0x04c0)) {
		maddress = (maddress - 0x0480) / 4;
		/* the modelview matrix is obtained by direct3d by multiplying the world matrix and the view matrix
		    modelview = world * view
		   given a point in 3d space with coordinates x y and z, to find te transformed coordinates
		   first create a row vector with components (x,y,z,1) then multiply the vector by the matrix
		    transformed = rowvector * matrix
		   in direct3d the matrix is stored as the sequence (first digit row, second digit column)
		    11 12 13 14
		    21 22 23 24
		    31 32 33 34
		    41 42 43 44
		   but it is sent transposed as the sequence
		    11 21 31 41 12 22 32 42 13 23 33 43 14 24 34 44
		   so in matrix.modelview[x][y] x is the column and y is the row of the direct3d matrix
		*/
		*(uint32_t *)(&matrix.modelview[maddress >> 2][maddress & 3]) = data;
		countlen--;
	}
	// inverse modelview matrix
	if ((maddress >= 0x0580) && (maddress < 0x05c0)) {
		maddress = (maddress - 0x0580) / 4;
		*(uint32_t *)(&matrix.modelview_inverse[maddress >> 2][maddress & 3]) = data;
		countlen--;
	}
	// composite matrix
	if ((maddress >= 0x0680) && (maddress < 0x06c0)) {
		maddress = (maddress - 0x0680) / 4;
		/* the composite matrix is computed by direct3d by multiplying the
		   world, view, projection and viewport matrices
		    composite = world * view * projection * viewport
		   the viewport matrix applies the viewport scale and offset
		 */
		*(uint32_t *)(&matrix.composite[maddress >> 2][maddress & 3]) = data;
		countlen--;
	}
	// viewport translate
	if ((maddress >= 0x0a20) && (maddress < 0x0a30)) {
		maddress = (maddress - 0x0a20) / 4;
		*(uint32_t *)(&matrix.translate[maddress]) = data;
		// set corresponding vertex shader constant too
		vertexprogram.exec.c_constant[59].iv(maddress, data); // constant -37
		if (maddress == 3)
			LOG("viewport translate = {%f %f %f %f}\n", matrix.translate[0], matrix.translate[1], matrix.translate[2], matrix.translate[3]);
		countlen--;
	}
	// viewport scale
	if ((maddress >= 0x0af0) && (maddress < 0x0b00)) {
		maddress = (maddress - 0x0af0) / 4;
		*(uint32_t *)(&matrix.scale[maddress]) = data;
		// set corresponding vertex shader constant too
		vertexprogram.exec.c_constant[58].iv(maddress, data); // constant -38
		if (maddress == 3)
			LOG("viewport scale = {%f %f %f %f}\n", matrix.scale[0], matrix.scale[1], matrix.scale[2], matrix.scale[3]);
		countlen--;
	}
	// Vertex program (shader)
	if (maddress == 0x1e94) {
		/*if (data == 2)
		machine().logerror("Enabled vertex program\n");
		else if (data == 4)
		machine().logerror("Enabled fixed function pipeline\n");
		else if (data == 6)
		machine().logerror("Enabled both fixed function pipeline and vertex program ?\n");
		else
		machine().logerror("Unknown value %d to method 0x1e94\n",data);*/
		vertex_pipeline = data & 6;
		countlen--;
	}
	if (maddress == 0x1e9c) {
		//machine().logerror("VP_UPLOAD_FROM_ID %d\n",data);
		vertexprogram.upload_instruction_index = data;
		vertexprogram.upload_instruction_component = 0;
		countlen--;
	}
	if (maddress == 0x1ea0) {
		//machine().logerror("VP_START_FROM_ID %d\n",data);
		vertexprogram.instructions = vertexprogram.upload_instruction_index;
		vertexprogram.start_instruction = data;
		countlen--;
	}
	if (maddress == 0x1ea4) {
		//machine().logerror("VP_UPLOAD_CONST_ID %d\n",data);
		vertexprogram.upload_parameter_index = data;
		vertexprogram.upload_parameter_component = 0;
		countlen--;
	}
	if ((maddress >= 0x0b00) && (maddress < 0x0b80)) {
		//machine().logerror("VP_UPLOAD_INST\n");
		if (vertexprogram.upload_instruction_index < 256) {
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].i[vertexprogram.upload_instruction_component] = data;
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified |= (1 << vertexprogram.upload_instruction_component);
		}
		else
			machine().logerror("Need to increase size of vertexprogram.instruction to %d\n\r", vertexprogram.upload_instruction_index);
		if (vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified == 15) {
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified = 0;
			vertexprogram.exec.decode_instruction(vertexprogram.upload_instruction_index);
		}
		vertexprogram.upload_instruction_component++;
		if (vertexprogram.upload_instruction_component >= 4) {
			vertexprogram.upload_instruction_component = 0;
			vertexprogram.upload_instruction_index++;
		}
	}
	if ((maddress >= 0x0b80) && (maddress < 0x0c00)) {
		//machine().logerror("VP_UPLOAD_CONST\n");
		if (vertexprogram.upload_parameter_index < 192) {
			vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].iv(vertexprogram.upload_parameter_component, data);
		}
		else
			machine().logerror("Need to increase size of vertexprogram.parameter to %d\n\r", vertexprogram.upload_parameter_index);
		vertexprogram.upload_parameter_component++;
		if (vertexprogram.upload_parameter_component >= 4) {
			if ((vertexprogram.upload_parameter_index == 58) || (vertexprogram.upload_parameter_index == 59))
				LOG("vp constant %d (%s) = {%f %f %f %f}\n", vertexprogram.upload_parameter_index,
					vertexprogram.upload_parameter_index == 58 ? "viewport scale" : "viewport translate",
					vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].fv[0],
					vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].fv[1],
					vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].fv[2],
					vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].fv[3]);
			vertexprogram.upload_parameter_component = 0;
			vertexprogram.upload_parameter_index++;
		}
	}
	if ((maddress >= 0x1e80) && (maddress < 0x1e90)) {
		machine().logerror("Setting v0 vertex program input component %d to %f\n", (maddress - 0x1e80) / 4, *((float *)&data));
	}
	if (maddress == 0x1e90) {
		machine().logerror("Received explicit method to run vertex program\n");
	}
	if (maddress == 0x02a8) {
		fog_color = data;
	}
	// Register combiners
	if (maddress == 0x0288) {
		combiner.setup.final.mapin_rgb.D_input = (Combiner::InputRegister)(data & 15);
		combiner.setup.final.mapin_rgb.D_component = (data >> 4) & 1;
		combiner.setup.final.mapin_rgb.D_mapping = (Combiner::MapFunction)((data >> 5) & 7);
		combiner.setup.final.mapin_rgb.C_input = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.final.mapin_rgb.C_component = (data >> 12) & 1;
		combiner.setup.final.mapin_rgb.C_mapping = (Combiner::MapFunction)((data >> 13) & 7);
		combiner.setup.final.mapin_rgb.B_input = (Combiner::InputRegister)((data >> 16) & 15);
		combiner.setup.final.mapin_rgb.B_component = (data >> 20) & 1;
		combiner.setup.final.mapin_rgb.B_mapping = (Combiner::MapFunction)((data >> 21) & 7);
		combiner.setup.final.mapin_rgb.A_input = (Combiner::InputRegister)((data >> 24) & 15);
		combiner.setup.final.mapin_rgb.A_component = (data >> 28) & 1;
		combiner.setup.final.mapin_rgb.A_mapping = (Combiner::MapFunction)((data >> 29) & 7);
		countlen--;
	}
	if (maddress == 0x028c) {
		combiner.setup.final.color_sum_clamp = (data >> 7) & 1;
		combiner.setup.final.mapin_alpha.G_input = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.final.mapin_alpha.G_component = (data >> 12) & 1;
		combiner.setup.final.mapin_alpha.G_mapping = (Combiner::MapFunction)((data >> 13) & 7);
		combiner.setup.final.mapin_rgb.F_input = (Combiner::InputRegister)((data >> 16) & 15);
		combiner.setup.final.mapin_rgb.F_component = (data >> 20) & 1;
		combiner.setup.final.mapin_rgb.F_mapping = (Combiner::MapFunction)((data >> 21) & 7);
		combiner.setup.final.mapin_rgb.E_input = (Combiner::InputRegister)((data >> 24) & 15);
		combiner.setup.final.mapin_rgb.E_component = (data >> 28) & 1;
		combiner.setup.final.mapin_rgb.E_mapping = (Combiner::MapFunction)((data >> 29) & 7);
		countlen--;
	}
	if ((maddress >= 0x0260) && (maddress < 0x0280)) {
		int n;

		n = (maddress - 0x0260) >> 2;
		combiner.setup.stage[n].mapin_alpha.D_input = (Combiner::InputRegister)(data & 15);
		combiner.setup.stage[n].mapin_alpha.D_component = (data >> 4) & 1;
		combiner.setup.stage[n].mapin_alpha.D_mapping = (Combiner::MapFunction)((data >> 5) & 7);
		combiner.setup.stage[n].mapin_alpha.C_input = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.stage[n].mapin_alpha.C_component = (data >> 12) & 1;
		combiner.setup.stage[n].mapin_alpha.C_mapping = (Combiner::MapFunction)((data >> 13) & 7);
		combiner.setup.stage[n].mapin_alpha.B_input = (Combiner::InputRegister)((data >> 16) & 15);
		combiner.setup.stage[n].mapin_alpha.B_component = (data >> 20) & 1;
		combiner.setup.stage[n].mapin_alpha.B_mapping = (Combiner::MapFunction)((data >> 21) & 7);
		combiner.setup.stage[n].mapin_alpha.A_input = (Combiner::InputRegister)((data >> 24) & 15);
		combiner.setup.stage[n].mapin_alpha.A_component = (data >> 28) & 1;
		combiner.setup.stage[n].mapin_alpha.A_mapping = (Combiner::MapFunction)((data >> 29) & 7);
		countlen--;
	}
	if ((maddress >= 0x0ac0) && (maddress < 0x0ae0)) {
		int n;

		n = (maddress - 0x0ac0) >> 2;
		combiner.setup.stage[n].mapin_rgb.D_input = (Combiner::InputRegister)(data & 15);
		combiner.setup.stage[n].mapin_rgb.D_component = (data >> 4) & 1;
		combiner.setup.stage[n].mapin_rgb.D_mapping = (Combiner::MapFunction)((data >> 5) & 7);
		combiner.setup.stage[n].mapin_rgb.C_input = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.stage[n].mapin_rgb.C_component = (data >> 12) & 1;
		combiner.setup.stage[n].mapin_rgb.C_mapping = (Combiner::MapFunction)((data >> 13) & 7);
		combiner.setup.stage[n].mapin_rgb.B_input = (Combiner::InputRegister)((data >> 16) & 15);
		combiner.setup.stage[n].mapin_rgb.B_component = (data >> 20) & 1;
		combiner.setup.stage[n].mapin_rgb.B_mapping = (Combiner::MapFunction)((data >> 21) & 7);
		combiner.setup.stage[n].mapin_rgb.A_input = (Combiner::InputRegister)((data >> 24) & 15);
		combiner.setup.stage[n].mapin_rgb.A_component = (data >> 28) & 1;
		combiner.setup.stage[n].mapin_rgb.A_mapping = (Combiner::MapFunction)((data >> 29) & 7);
		countlen--;
	}
	if ((maddress >= 0x0a60) && (maddress < 0x0a80)) {
		int n;

		n = (maddress - 0x0a60) >> 2;
		combiner_argb8_float(data, combiner.setup.stage[n].constantcolor0);
		countlen--;
	}
	if ((maddress >= 0x0a80) && (maddress < 0x0aa0)) {
		int n;

		n = (maddress - 0x0a80) >> 2;
		combiner_argb8_float(data, combiner.setup.stage[n].constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x0aa0) && (maddress < 0x0ac0)) {
		int n;

		n = (maddress - 0x0aa0) >> 2;
		combiner.setup.stage[n].mapout_alpha.CD_output = (Combiner::InputRegister)(data & 15);
		combiner.setup.stage[n].mapout_alpha.AB_output = (Combiner::InputRegister)((data >> 4) & 15);
		combiner.setup.stage[n].mapout_alpha.SUM_output = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.stage[n].mapout_alpha.CD_dotproduct = (data >> 12) & 1;
		combiner.setup.stage[n].mapout_alpha.AB_dotproduct = (data >> 13) & 1;
		combiner.setup.stage[n].mapout_alpha.muxsum = (data >> 14) & 1;
		combiner.setup.stage[n].mapout_alpha.bias = (data >> 15) & 1;
		combiner.setup.stage[n].mapout_alpha.scale = (data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
	if (maddress == 0x1e20) {
		combiner_argb8_float(data, combiner.setup.final.constantcolor0);
		countlen--;
	}
	if (maddress == 0x1e24) {
		combiner_argb8_float(data, combiner.setup.final.constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x1e40) && (maddress < 0x1e60)) {
		int n;

		n = (maddress - 0x1e40) >> 2;
		combiner.setup.stage[n].mapout_rgb.CD_output = (Combiner::InputRegister)(data & 15);
		combiner.setup.stage[n].mapout_rgb.AB_output = (Combiner::InputRegister)((data >> 4) & 15);
		combiner.setup.stage[n].mapout_rgb.SUM_output = (Combiner::InputRegister)((data >> 8) & 15);
		combiner.setup.stage[n].mapout_rgb.CD_dotproduct = (data >> 12) & 1;
		combiner.setup.stage[n].mapout_rgb.AB_dotproduct = (data >> 13) & 1;
		combiner.setup.stage[n].mapout_rgb.muxsum = (data >> 14) & 1;
		combiner.setup.stage[n].mapout_rgb.bias = (data >> 15) & 1;
		combiner.setup.stage[n].mapout_rgb.scale = (data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
	if (maddress == 0x1e60) {
		combiner.setup.stages = data & 15;
		countlen--;
	}
	return 0;
}

int nv2a_renderer::execute_method_m2mf(address_space &space, uint32_t chanel, uint32_t subchannel, uint32_t method, uint32_t address, uint32_t data, int &countlen)
{
	if (method == 0x0180) {
		LOG("m2mf method 0180 notify\n");
		geforce_read_dma_object(data, dma_offset[10], dma_size[10]);
	}
	return 0;
}

int nv2a_renderer::execute_method_surf2d(address_space &space, uint32_t chanel, uint32_t subchannel, uint32_t method, uint32_t address, uint32_t data, int &countlen)
{
	if (method == 0x0184) {
		LOG("surf2d method 0184 source\n");
		geforce_read_dma_object(data, dma_offset[11], dma_size[11]);
	}
	if (method == 0x0188) {
		LOG("surf2d method 0188 destination\n");
		geforce_read_dma_object(data, dma_offset[12], dma_size[12]);
	}
	if (method == 0x0300) {
		bitblit.format = data; // 0xa is a8r8g8b8
	}
	if (method == 0x0304) {
		bitblit.pitch_source = data & 0xffff;
		bitblit.pitch_destination = data >> 16;
	}
	if (method == 0x0308) {
		bitblit.source_address = dma_offset[11] + data;
	}
	if (method == 0x030c) {
		bitblit.destination_address = dma_offset[12] + data;
	}
	return 0;
}

int nv2a_renderer::execute_method_blit(address_space &space, uint32_t chanel, uint32_t subchannel, uint32_t method, uint32_t address, uint32_t data, int &countlen)
{
	if (method == 0x019c) {
		LOG("blit method 019c surface objecct handle %d\n", data); // set to 0x11
	}
	if (method == 0x02fc) {
		LOG("blit method 02fc operation %d\n", data); // 3 is copy from source to destination
		bitblit.op = data;
	}
	if (method == 0x0300) {
		bitblit.sourcex = data & 0xffff;
		bitblit.sourcey = data >> 16;
	}
	if (method == 0x0304) {
		bitblit.destinationx = data & 0xffff;
		bitblit.destinationy = data >> 16;
	}
	if (method == 0x0308) {
		bitblit.width = data & 0xffff;
		bitblit.height = data >> 16;
		surface_2d_blit();
	}
	return 0;
}

void nv2a_renderer::surface_2d_blit()
{
	int x, y;
	uint32_t *src, *dest;
	uint32_t *srcrow, *destrow;

	if (bitblit.format != 0xa) {
		machine().logerror("Unsupported format %d in surface_2d_blit\n", bitblit.format);
		return;
	}
	srcrow = (uint32_t *)direct_access_ptr(bitblit.source_address + bitblit.pitch_source * bitblit.sourcey + bitblit.sourcex * 4);
	destrow = (uint32_t *)direct_access_ptr(bitblit.destination_address + bitblit.pitch_destination * bitblit.destinationy + bitblit.destinationx * 4);
	for (y = 0; y < bitblit.height; y++) {
		src = srcrow;
		dest = destrow;
		for (x = 0; x < bitblit.width; x++) {
			*dest = *src;
			dest++;
			src++;
		}
		srcrow += bitblit.pitch_source >> 2;
		destrow += bitblit.pitch_destination >> 2;
	}
}

bool nv2a_renderer::toggle_register_combiners_usage()
{
	combiner.used = 1 - combiner.used;
	return combiner.used != 0;
}

bool nv2a_renderer::toggle_wait_vblank_support()
{
	enable_waitvblank = !enable_waitvblank;
	return enable_waitvblank;
}

bool nv2a_renderer::toggle_clipping_w_support()
{
	enable_clipping_w = !enable_clipping_w;
	return enable_clipping_w;
}

void nv2a_renderer::debug_grab_texture(int type, const char *filename)
{
	debug_grab_texttype = type;
	if (debug_grab_textfile == nullptr)
		debug_grab_textfile = (char *)malloc(128);
	strncpy(debug_grab_textfile, filename, 127);
}

void nv2a_renderer::debug_grab_vertex_program_slot(int slot, uint32_t *instruction)
{
	if (slot >= 1024 / 4)
		return;
	instruction[0] = vertexprogram.exec.op[slot].i[0];
	instruction[1] = vertexprogram.exec.op[slot].i[1];
	instruction[2] = vertexprogram.exec.op[slot].i[2];
	instruction[3] = vertexprogram.exec.op[slot].i[3];
}

void nv2a_renderer::combiner_argb8_float(uint32_t color, float reg[4])
{
	reg[2] = (float)(color & 0xff) / 255.0f;
	reg[1] = (float)((color >> 8) & 0xff) / 255.0f;
	reg[0] = (float)((color >> 16) & 0xff) / 255.0f;
	reg[3] = (float)((color >> 24) & 0xff) / 255.0f;
}

uint32_t nv2a_renderer::combiner_float_argb8(float reg[4])
{
	uint32_t r, g, b, a;

	a = reg[3] * 255.0f;
	b = reg[2] * 255.0f;
	g = reg[1] * 255.0f;
	r = reg[0] * 255.0f;
	return (a << 24) | (r << 16) | (g << 8) | b;
}

float nv2a_renderer::combiner_map_input_select(int id, Combiner::InputRegister code, int index)
{
	switch ((int)code) {
	case 0:
	default:
		return combiner.work[id].registers.zero[index];
	case 1:
		return combiner.work[id].registers.color0[index];
	case 2:
		return combiner.work[id].registers.color1[index];
	case 3:
		return combiner.work[id].registers.fogcolor[index];
	case 4:
		return combiner.work[id].registers.primarycolor[index];
	case 5:
		return combiner.work[id].registers.secondarycolor[index];
	case 8:
		return combiner.work[id].registers.texture0color[index];
	case 9:
		return combiner.work[id].registers.texture1color[index];
	case 10:
		return combiner.work[id].registers.texture2color[index];
	case 11:
		return combiner.work[id].registers.texture3color[index];
	case 12:
		return combiner.work[id].registers.spare0[index];
	case 13:
		return combiner.work[id].registers.spare1[index];
	case 14:
		return combiner.work[id].variables.sumclamp[index];
	case 15:
		return combiner.work[id].variables.EF[index];
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_input_select_array(int id, Combiner::InputRegister code)
{
	switch ((int)code) {
	case 0:
	default:
		return combiner.work[id].registers.zero;
	case 1:
		return combiner.work[id].registers.color0;
	case 2:
		return combiner.work[id].registers.color1;
	case 3:
		return combiner.work[id].registers.fogcolor;
	case 4:
		return combiner.work[id].registers.primarycolor;
	case 5:
		return combiner.work[id].registers.secondarycolor;
	case 8:
		return combiner.work[id].registers.texture0color;
	case 9:
		return combiner.work[id].registers.texture1color;
	case 10:
		return combiner.work[id].registers.texture2color;
	case 11:
		return combiner.work[id].registers.texture3color;
	case 12:
		return combiner.work[id].registers.spare0;
	case 13:
		return combiner.work[id].registers.spare1;
	case 14:
		return combiner.work[id].variables.sumclamp;
	case 15:
		return combiner.work[id].variables.EF;
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_output_select_array(int id, Combiner::InputRegister code)
{
	switch ((int)code) {
	case 0:
		return nullptr;
	case 1:
		return nullptr;
	case 2:
		return nullptr;
	case 3:
		return nullptr;
	case 4:
		return combiner.work[id].registers.primarycolor;
	case 5:
		return combiner.work[id].registers.secondarycolor;
	case 8:
		return combiner.work[id].registers.texture0color;
	case 9:
		return combiner.work[id].registers.texture1color;
	case 10:
		return combiner.work[id].registers.texture2color;
	case 11:
		return combiner.work[id].registers.texture3color;
	case 12:
		return combiner.work[id].registers.spare0;
	case 13:
		return combiner.work[id].registers.spare1;
	case 14:
		return nullptr;
	case 15:
	default:
		return nullptr;
	}
}

float nv2a_renderer::combiner_map_input_function(Combiner::MapFunction code, float value)
{
	switch ((int)code) {
	case 0: // unsigned identity
		return std::max(0.0f, value);
	case 1: // unsigned invert
		return 1.0f - std::clamp(value, 0.0f, 1.0f);
	case 2: // expand normal
		return 2.0f * std::max(0.0f, value) - 1.0f;
	case 3: // expand negate
		return -2.0f * std::max(0.0f, value) + 1.0f;
	case 4: // half bias normal
		return std::max(0.0f, value) - 0.5f;
	case 5: // half bias negate
		return -std::max(0.0f, value) + 0.5f;
	case 6: // signed identyty
		return value;
	case 7: // signed negate
	default:
		return -value;
	}

	// never executed
	//return 0;
}

void nv2a_renderer::combiner_map_input_function_array(Combiner::MapFunction code, float *data)
{
	switch ((int)code) {
	case 0:
		data[0] = std::max(0.0f, data[0]);
		data[1] = std::max(0.0f, data[1]);
		data[2] = std::max(0.0f, data[2]);
		break;
	case 1:
		data[0] = 1.0f - std::clamp(data[0], 0.0f, 1.0f);
		data[1] = 1.0f - std::clamp(data[1], 0.0f, 1.0f);
		data[2] = 1.0f - std::clamp(data[2], 0.0f, 1.0f);
		break;
	case 2:
		data[0] = 2.0f * std::max(0.0f, data[0]) - 1.0f;
		data[1] = 2.0f * std::max(0.0f, data[1]) - 1.0f;
		data[2] = 2.0f * std::max(0.0f, data[2]) - 1.0f;
		break;
	case 3:
		data[0] = -2.0f * std::max(0.0f, data[0]) + 1.0f;
		data[1] = -2.0f * std::max(0.0f, data[1]) + 1.0f;
		data[2] = -2.0f * std::max(0.0f, data[2]) + 1.0f;
		break;
	case 4:
		data[0] = std::max(0.0f, data[0]) - 0.5f;
		data[1] = std::max(0.0f, data[1]) - 0.5f;
		data[2] = std::max(0.0f, data[2]) - 0.5f;
		break;
	case 5:
		data[0] = -std::max(0.0f, data[0]) + 0.5f;
		data[1] = -std::max(0.0f, data[1]) + 0.5f;
		data[2] = -std::max(0.0f, data[2]) + 0.5f;
		break;
	case 6:
		return;
	case 7:
	default:
		data[0] = -data[0];
		data[1] = -data[1];
		data[2] = -data[2];
		break;
	}
}

void nv2a_renderer::combiner_initialize_registers(int id, float rgba[6][4])
{
	for (int n = 0; n < 4; n++) {
		combiner.work[id].registers.primarycolor[n] = rgba[0][n];
		combiner.work[id].registers.secondarycolor[n] = rgba[1][n];
		combiner.work[id].registers.texture0color[n] = rgba[2][n];
		combiner.work[id].registers.texture1color[n] = rgba[3][n];
		combiner.work[id].registers.texture2color[n] = rgba[4][n];
		combiner.work[id].registers.texture3color[n] = rgba[5][n];
		combiner.work[id].registers.fogcolor[n] = rgba[6][n];
	}
	combiner.work[id].registers.spare0[3] = combiner.work[id].registers.texture0color[3]; // alpha of spare 0 must be the alpha of the pixel from texture 0
	combiner.work[id].registers.zero[0] = combiner.work[id].registers.zero[1] = combiner.work[id].registers.zero[2] = combiner.work[id].registers.zero[3] = 0;
}

void nv2a_renderer::combiner_initialize_stage(int id, int stage_number)
{
	int n = stage_number;

	// put register_constantcolor0 in register_color0
	combiner.work[id].registers.color0[0] = combiner.setup.stage[n].constantcolor0[0];
	combiner.work[id].registers.color0[1] = combiner.setup.stage[n].constantcolor0[1];
	combiner.work[id].registers.color0[2] = combiner.setup.stage[n].constantcolor0[2];
	combiner.work[id].registers.color0[3] = combiner.setup.stage[n].constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.work[id].registers.color1[0] = combiner.setup.stage[n].constantcolor1[0];
	combiner.work[id].registers.color1[1] = combiner.setup.stage[n].constantcolor1[1];
	combiner.work[id].registers.color1[2] = combiner.setup.stage[n].constantcolor1[2];
	combiner.work[id].registers.color1[3] = combiner.setup.stage[n].constantcolor1[3];
}

void nv2a_renderer::combiner_initialize_final(int id)
{
	// put register_constantcolor0 in register_color0
	combiner.work[id].registers.color0[0] = combiner.setup.final.constantcolor0[0];
	combiner.work[id].registers.color0[1] = combiner.setup.final.constantcolor0[1];
	combiner.work[id].registers.color0[2] = combiner.setup.final.constantcolor0[2];
	combiner.work[id].registers.color0[3] = combiner.setup.final.constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.work[id].registers.color1[0] = combiner.setup.final.constantcolor1[0];
	combiner.work[id].registers.color1[1] = combiner.setup.final.constantcolor1[1];
	combiner.work[id].registers.color1[2] = combiner.setup.final.constantcolor1[2];
	combiner.work[id].registers.color1[3] = combiner.setup.final.constantcolor1[3];
}

void nv2a_renderer::combiner_map_stage_input(int id, int stage_number)
{
	int n = stage_number;
	int c, d, i;
	float v, *pv;

	// rgb portion
	// A
	// get pointer to rgb components of selected input register
	pv = combiner_map_input_select_array(id, combiner.setup.stage[n].mapin_rgb.A_input);
	c = combiner.setup.stage[n].mapin_rgb.A_component * 3;
	i = combiner.setup.stage[n].mapin_rgb.A_component ^ 1;
	// copy components to A
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.A[d] = pv[c];
		c += i;
	}
	// apply mapping function
	combiner_map_input_function_array(combiner.setup.stage[n].mapin_rgb.A_mapping, combiner.work[id].variables.A);
	// B
	pv = combiner_map_input_select_array(id, combiner.setup.stage[n].mapin_rgb.B_input);
	c = combiner.setup.stage[n].mapin_rgb.B_component * 3;
	i = combiner.setup.stage[n].mapin_rgb.B_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.B[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.stage[n].mapin_rgb.B_mapping, combiner.work[id].variables.B);
	// C
	pv = combiner_map_input_select_array(id, combiner.setup.stage[n].mapin_rgb.C_input);
	c = combiner.setup.stage[n].mapin_rgb.C_component * 3;
	i = combiner.setup.stage[n].mapin_rgb.C_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.C[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.stage[n].mapin_rgb.C_mapping, combiner.work[id].variables.C);
	// D
	pv = combiner_map_input_select_array(id, combiner.setup.stage[n].mapin_rgb.D_input);
	c = combiner.setup.stage[n].mapin_rgb.D_component * 3;
	i = combiner.setup.stage[n].mapin_rgb.D_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.D[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.stage[n].mapin_rgb.D_mapping, combiner.work[id].variables.D);

	// alpha portion
	// A
	// get component (blue or alpha) from selected input
	v = combiner_map_input_select(id, combiner.setup.stage[n].mapin_alpha.A_input, 2 + combiner.setup.stage[n].mapin_alpha.A_component);
	// copy component to A
	combiner.work[id].variables.A[3] = combiner_map_input_function(combiner.setup.stage[n].mapin_alpha.A_mapping, v);
	// B
	v = combiner_map_input_select(id, combiner.setup.stage[n].mapin_alpha.B_input, 2 + combiner.setup.stage[n].mapin_alpha.B_component);
	combiner.work[id].variables.B[3] = combiner_map_input_function(combiner.setup.stage[n].mapin_alpha.B_mapping, v);
	// C
	v = combiner_map_input_select(id, combiner.setup.stage[n].mapin_alpha.C_input, 2 + combiner.setup.stage[n].mapin_alpha.C_component);
	combiner.work[id].variables.C[3] = combiner_map_input_function(combiner.setup.stage[n].mapin_alpha.C_mapping, v);
	// D
	v = combiner_map_input_select(id, combiner.setup.stage[n].mapin_alpha.D_input, 2 + combiner.setup.stage[n].mapin_alpha.D_component);
	combiner.work[id].variables.D[3] = combiner_map_input_function(combiner.setup.stage[n].mapin_alpha.D_mapping, v);
}

void nv2a_renderer::combiner_map_stage_output(int id, int stage_number)
{
	int n = stage_number;
	float *f;

	// rgb
	f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_rgb.AB_output);
	if (f) {
		f[0] = combiner.work[id].functions.RGBop1[0];
		f[1] = combiner.work[id].functions.RGBop1[1];
		f[2] = combiner.work[id].functions.RGBop1[2];
	}
	f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_rgb.CD_output);
	if (f) {
		f[0] = combiner.work[id].functions.RGBop2[0];
		f[1] = combiner.work[id].functions.RGBop2[1];
		f[2] = combiner.work[id].functions.RGBop2[2];
	}
	if ((combiner.setup.stage[n].mapout_rgb.AB_dotproduct | combiner.setup.stage[n].mapout_rgb.CD_dotproduct) == 0) {
		f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_rgb.SUM_output);
		if (f) {
			f[0] = combiner.work[id].functions.RGBop3[0];
			f[1] = combiner.work[id].functions.RGBop3[1];
			f[2] = combiner.work[id].functions.RGBop3[2];
		}
	}
	// alpha
	f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_alpha.AB_output);
	if (f)
		f[3] = combiner.work[id].functions.Aop1;
	f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_alpha.CD_output);
	if (f)
		f[3] = combiner.work[id].functions.Aop2;
	f = combiner_map_output_select_array(id, combiner.setup.stage[n].mapout_alpha.SUM_output);
	if (f)
		f[3] = combiner.work[id].functions.Aop3;
}

void nv2a_renderer::combiner_map_final_input(int id)
{
	int c, d, i;
	float *pv;

	// E
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.E_input);
	c = combiner.setup.final.mapin_rgb.E_component * 3;
	i = combiner.setup.final.mapin_rgb.E_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.E[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.E_mapping, combiner.work[id].variables.E);
	// F
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.F_input);
	c = combiner.setup.final.mapin_rgb.F_component * 3;
	i = combiner.setup.final.mapin_rgb.F_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.F[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.F_mapping, combiner.work[id].variables.F);
	// EF
	combiner.work[id].variables.EF[0] = combiner.work[id].variables.E[0] * combiner.work[id].variables.F[0];
	combiner.work[id].variables.EF[1] = combiner.work[id].variables.E[1] * combiner.work[id].variables.F[1];
	combiner.work[id].variables.EF[2] = combiner.work[id].variables.E[2] * combiner.work[id].variables.F[2];
	// sumclamp
	combiner.work[id].variables.sumclamp[0] = std::max(0.0f, combiner.work[id].registers.spare0[0]) + std::max(0.0f, combiner.work[id].registers.secondarycolor[0]);
	combiner.work[id].variables.sumclamp[1] = std::max(0.0f, combiner.work[id].registers.spare0[1]) + std::max(0.0f, combiner.work[id].registers.secondarycolor[1]);
	combiner.work[id].variables.sumclamp[2] = std::max(0.0f, combiner.work[id].registers.spare0[2]) + std::max(0.0f, combiner.work[id].registers.secondarycolor[2]);
	if (combiner.setup.final.color_sum_clamp != 0) {
		combiner.work[id].variables.sumclamp[0] = std::min(combiner.work[id].variables.sumclamp[0], 1.0f);
		combiner.work[id].variables.sumclamp[1] = std::min(combiner.work[id].variables.sumclamp[1], 1.0f);
		combiner.work[id].variables.sumclamp[2] = std::min(combiner.work[id].variables.sumclamp[2], 1.0f);
	}
	// A
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.A_input);
	c = combiner.setup.final.mapin_rgb.A_component * 3;
	i = combiner.setup.final.mapin_rgb.A_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.A[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.A_mapping, combiner.work[id].variables.A);
	// B
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.B_input);
	c = combiner.setup.final.mapin_rgb.B_component * 3;
	i = combiner.setup.final.mapin_rgb.B_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.B[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.B_mapping, combiner.work[id].variables.B);
	// C
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.C_input);
	c = combiner.setup.final.mapin_rgb.C_component * 3;
	i = combiner.setup.final.mapin_rgb.C_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.C[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.C_mapping, combiner.work[id].variables.C);
	// D
	pv = combiner_map_input_select_array(id, combiner.setup.final.mapin_rgb.D_input);
	c = combiner.setup.final.mapin_rgb.D_component * 3;
	i = combiner.setup.final.mapin_rgb.D_component ^ 1;
	for (d = 0; d < 3; d++) {
		combiner.work[id].variables.D[d] = pv[c];
		c += i;
	}
	combiner_map_input_function_array(combiner.setup.final.mapin_rgb.D_mapping, combiner.work[id].variables.D);
	// G
	combiner.work[id].variables.G = combiner_map_input_select(id, combiner.setup.final.mapin_alpha.G_input, 2 + combiner.setup.final.mapin_alpha.G_component);
}

void nv2a_renderer::combiner_final_output(int id)
{
	// rgb
	combiner.work[id].output[0] = combiner.work[id].variables.A[0] * combiner.work[id].variables.B[0] + (1.0f - combiner.work[id].variables.A[0])*combiner.work[id].variables.C[0] + combiner.work[id].variables.D[0];
	combiner.work[id].output[1] = combiner.work[id].variables.A[1] * combiner.work[id].variables.B[1] + (1.0f - combiner.work[id].variables.A[1])*combiner.work[id].variables.C[1] + combiner.work[id].variables.D[1];
	combiner.work[id].output[2] = combiner.work[id].variables.A[2] * combiner.work[id].variables.B[2] + (1.0f - combiner.work[id].variables.A[2])*combiner.work[id].variables.C[2] + combiner.work[id].variables.D[2];
	combiner.work[id].output[0] = std::min(combiner.work[id].output[0], 2.0f);
	combiner.work[id].output[1] = std::min(combiner.work[id].output[1], 2.0f);
	combiner.work[id].output[2] = std::min(combiner.work[id].output[2], 2.0f);
	// a
	combiner.work[id].output[3] = combiner_map_input_function(combiner.setup.final.mapin_alpha.G_mapping, combiner.work[id].variables.G);
}

void nv2a_renderer::combiner_function_AB(int id, float result[4])
{
	result[0] = combiner.work[id].variables.A[0] * combiner.work[id].variables.B[0];
	result[1] = combiner.work[id].variables.A[1] * combiner.work[id].variables.B[1];
	result[2] = combiner.work[id].variables.A[2] * combiner.work[id].variables.B[2];
}

void nv2a_renderer::combiner_function_AdotB(int id, float result[4])
{
	result[0] = combiner.work[id].variables.A[0] * combiner.work[id].variables.B[0] + combiner.work[id].variables.A[1] * combiner.work[id].variables.B[1] + combiner.work[id].variables.A[2] * combiner.work[id].variables.B[2];
	result[1] = result[0];
	result[2] = result[0];
}

void nv2a_renderer::combiner_function_CD(int id, float result[4])
{
	result[0] = combiner.work[id].variables.C[0] * combiner.work[id].variables.D[0];
	result[1] = combiner.work[id].variables.C[1] * combiner.work[id].variables.D[1];
	result[2] = combiner.work[id].variables.C[2] * combiner.work[id].variables.D[2];
}

void nv2a_renderer::combiner_function_CdotD(int id, float result[4])
{
	result[0] = combiner.work[id].variables.C[0] * combiner.work[id].variables.D[0] + combiner.work[id].variables.C[1] * combiner.work[id].variables.D[1] + combiner.work[id].variables.C[2] * combiner.work[id].variables.D[2];
	result[1] = result[0];
	result[2] = result[0];
}

void nv2a_renderer::combiner_function_ABmuxCD(int id, float result[4])
{
	if (combiner.work[id].registers.spare0[3] >= 0.5f)
		combiner_function_AB(id, result);
	else
		combiner_function_CD(id, result);
}

void nv2a_renderer::combiner_function_ABsumCD(int id, float result[4])
{
	result[0] = combiner.work[id].variables.A[0] * combiner.work[id].variables.B[0] + combiner.work[id].variables.C[0] * combiner.work[id].variables.D[0];
	result[1] = combiner.work[id].variables.A[1] * combiner.work[id].variables.B[1] + combiner.work[id].variables.C[1] * combiner.work[id].variables.D[1];
	result[2] = combiner.work[id].variables.A[2] * combiner.work[id].variables.B[2] + combiner.work[id].variables.C[2] * combiner.work[id].variables.D[2];
}

void nv2a_renderer::combiner_compute_rgb_outputs(int id, int stage_number)
{
	int n = stage_number;
	int m;
	float bias, scale;

	// select bias and scale
	if (combiner.setup.stage[n].mapout_rgb.bias)
		bias = -0.5;
	else
		bias = 0;
	switch (combiner.setup.stage[n].mapout_rgb.scale) {
	case 0:
	default:
		scale = 1.0;
		break;
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 4.0;
		break;
	case 3:
		scale = 0.5;
		break;
	}
	// first
	if (combiner.setup.stage[n].mapout_rgb.AB_dotproduct) {
		m = 1;
		combiner_function_AdotB(id, combiner.work[id].functions.RGBop1);
	}
	else {
		m = 0;
		combiner_function_AB(id, combiner.work[id].functions.RGBop1);
	}
	combiner.work[id].functions.RGBop1[0] = std::clamp((combiner.work[id].functions.RGBop1[0] + bias) * scale, -1.0f, 1.0f);
	combiner.work[id].functions.RGBop1[1] = std::clamp((combiner.work[id].functions.RGBop1[1] + bias) * scale, -1.0f, 1.0f);
	combiner.work[id].functions.RGBop1[2] = std::clamp((combiner.work[id].functions.RGBop1[2] + bias) * scale, -1.0f, 1.0f);
	// second
	if (combiner.setup.stage[n].mapout_rgb.CD_dotproduct) {
		m = m | 1;
		combiner_function_CdotD(id, combiner.work[id].functions.RGBop2);
	}
	else
		combiner_function_CD(id, combiner.work[id].functions.RGBop2);
	combiner.work[id].functions.RGBop2[0] = std::clamp((combiner.work[id].functions.RGBop2[0] + bias) * scale, -1.0f, 1.0f);
	combiner.work[id].functions.RGBop2[1] = std::clamp((combiner.work[id].functions.RGBop2[1] + bias) * scale, -1.0f, 1.0f);
	combiner.work[id].functions.RGBop2[2] = std::clamp((combiner.work[id].functions.RGBop2[2] + bias) * scale, -1.0f, 1.0f);
	// third
	if (m == 0) {
		if (combiner.setup.stage[n].mapout_rgb.muxsum)
			combiner_function_ABmuxCD(id, combiner.work[id].functions.RGBop3);
		else
			combiner_function_ABsumCD(id, combiner.work[id].functions.RGBop3);
		combiner.work[id].functions.RGBop3[0] = std::clamp((combiner.work[id].functions.RGBop3[0] + bias) * scale, -1.0f, 1.0f);
		combiner.work[id].functions.RGBop3[1] = std::clamp((combiner.work[id].functions.RGBop3[1] + bias) * scale, -1.0f, 1.0f);
		combiner.work[id].functions.RGBop3[2] = std::clamp((combiner.work[id].functions.RGBop3[2] + bias) * scale, -1.0f, 1.0f);
	}
}

void nv2a_renderer::combiner_compute_alpha_outputs(int id, int stage_number)
{
	int n = stage_number;
	float bias, scale;

	// select bias and scale
	if (combiner.setup.stage[n].mapout_alpha.bias)
		bias = -0.5;
	else
		bias = 0;
	switch (combiner.setup.stage[n].mapout_alpha.scale) {
	case 0:
	default:
		scale = 1.0;
		break;
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 4.0;
		break;
	case 3:
		scale = 0.5;
		break;
	}
	// first
	combiner.work[id].functions.Aop1 = combiner.work[id].variables.A[3] * combiner.work[id].variables.B[3];
	combiner.work[id].functions.Aop1 = std::clamp((combiner.work[id].functions.Aop1 + bias) * scale, -1.0f, 1.0f);
	// second
	combiner.work[id].functions.Aop2 = combiner.work[id].variables.C[3] * combiner.work[id].variables.D[3];
	combiner.work[id].functions.Aop2 = std::clamp((combiner.work[id].functions.Aop2 + bias) * scale, -1.0f, 1.0f);
	// third
	if (combiner.setup.stage[n].mapout_alpha.muxsum) {
		if (combiner.work[id].registers.spare0[3] >= 0.5f)
			combiner.work[id].functions.Aop3 = combiner.work[id].variables.A[3] * combiner.work[id].variables.B[3];
		else
			combiner.work[id].functions.Aop3 = combiner.work[id].variables.C[3] * combiner.work[id].variables.D[3];
	}
	else
		combiner.work[id].functions.Aop3 = combiner.work[id].variables.A[3] * combiner.work[id].variables.B[3] + combiner.work[id].variables.C[3] * combiner.work[id].variables.D[3];
	combiner.work[id].functions.Aop3 = std::clamp((combiner.work[id].functions.Aop3 + bias) * scale, -1.0f, 1.0f);
}

void nv2a_renderer::vblank_callback(int state)
{
	LOGMASKED(LOG_NV2A_VERBOSE, "vblank_callback\n\r");
	if ((state != 0) && (puller_waiting == 1)) {
		puller_waiting = 0;
		puller_timer_work(0);
	}
	if (state != 0) {
		pcrtc[0x100 / 4] |= 1;
		pcrtc[0x808 / 4] |= 0x10000;
	}
	else {
		pcrtc[0x100 / 4] &= ~1;
		pcrtc[0x808 / 4] &= ~0x10000;
	}
	if (update_interrupts() == true)
		irq_callback(1); // IRQ 3
	else
		irq_callback(0); // IRQ 3
}

bool nv2a_renderer::update_interrupts()
{
	if (pcrtc[0x100 / 4] & pcrtc[0x140 / 4])
		pmc[0x100 / 4] |= 0x1000000;
	else
		pmc[0x100 / 4] &= ~0x1000000;
	if (pgraph[0x100 / 4] & pgraph[0x140 / 4])
		pmc[0x100 / 4] |= 0x1000;
	else
		pmc[0x100 / 4] &= ~0x1000;
	if (((pmc[0x100 / 4] & 0x7fffffff) && (pmc[0x140 / 4] & 1)) || ((pmc[0x100 / 4] & 0x80000000) && (pmc[0x140 / 4] & 2))) {
		// send interrupt
		return true;
	}
	else
		return false;
}

uint32_t nv2a_renderer::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (displayedtarget != nullptr) {
		bitmap_rgb32 bm(displayedtarget, 640, 480, 640);
		uint32_t *dst = (uint32_t *)bitmap.raw_pixptr(0, 0);

		//printf("updatescreen %08X\n\r",pcrtc[0x800/4]);
		memcpy(dst, displayedtarget, bitmap.rowbytes()*bitmap.height());
	}
	return 0;
}

void nv2a_renderer::geforce_assign_object(address_space &space, uint32_t chanel, uint32_t subchannel, uint32_t address)
{
	uint32_t handle, offset, objclass, data;

	handle = space.read_dword(address);
	offset = geforce_object_offset(handle);
	LOG("  assign to subchannel %d object at %d in ramin", subchannel, offset);
	channel[chanel][subchannel].object.offset = offset;
	data = ramin[offset / 4];
	objclass = data & 0xff;
	LOG(" class %03X\n", objclass);
	channel[chanel][subchannel].object.objclass = objclass;
}

TIMER_CALLBACK_MEMBER(nv2a_renderer::puller_timer_work)
{
	int chanel;
	int method, count;
	uint32_t *dmaput, *dmaget;
	uint32_t cmd;
	COMMAND cmdtype;
	int countlen;
	int ret;
	address_space *space = puller_space;
	uint32_t subch;

	for (chanel = 0; chanel < 32; chanel++) {
		dmaput = &channel[chanel][0].regs[0x40 / 4];
		dmaget = &channel[chanel][0].regs[0x44 / 4];
		while (*dmaget != *dmaput) {
			cmd = space->read_dword(*dmaget);
			*dmaget += 4;
			cmdtype = geforce_commandkind(cmd);
			switch (cmdtype)
			{
			case COMMAND::JUMP:
				LOG("jump dmaget %08X", *dmaget);
				*dmaget = cmd & 0xfffffffc;
				LOG(" -> %08X\n\r", *dmaget);
				break;
			case COMMAND::INCREASING:
				method = cmd & (2047 << 2); // if method >= 0x100 send it to assigned object
				subch = (cmd >> 13) & 7;
				count = (cmd >> 18) & 2047;
				if ((method == 0) && (count == 1)) { // OBJECT method, bind an engine object to a subchannel
					geforce_assign_object(*space, chanel, subch, *dmaget);
					*dmaget += 4;
				}
				else {
					LOG("  subch. %d method %04x count %d\n", subch, method, count);
					ret = 0;
					while (count > 0) {
						countlen = 1;
						ret = execute_method(*space, chanel, subch, method, *dmaget, countlen);
						count--;
						method += 4;
						*dmaget += 4;
						if (ret != 0)
							break;
					}
					if (ret != 0) {
						puller_timer->enable(false);
						puller_waiting = ret;
						return;
					}
				}
				break;
			case COMMAND::NON_INCREASING:
				method = cmd & (2047 << 2);
				subch = (cmd >> 13) & 7;
				count = (cmd >> 18) & 2047;
				if ((method == 0) && (count == 1)) {
					geforce_assign_object(*space, chanel, subch, *dmaget);
					*dmaget += 4;
				}
				else {
					LOG("  subch. %d method %04x count %d\n", subch, method, count);
					while (count > 0) {
						countlen = count;
						ret = execute_method(*space, chanel, subch, method, *dmaget, countlen);
						*dmaget += 4 * (count - countlen);
						count = countlen;
					}
				}
				break;
			case COMMAND::LONG_NON_INCREASING:
				method = cmd & (2047 << 2);
				subch = (cmd >> 13) & 7;
				count = space->read_dword(*dmaget);
				*dmaget += 4;
				if ((method == 0) && (count == 1)) {
					geforce_assign_object(*space, chanel, subch, *dmaget);
					*dmaget += 4;
				}
				else {
					LOG("  subch. %d method %04x count %d\n", subch, method, count);
					while (count > 0) {
						countlen = count;
						ret = execute_method(*space, chanel, subch, method, *dmaget, countlen);
						*dmaget += 4 * (count - countlen);
						count = countlen;
					}
				}
				break;
			default:
				machine().logerror("  unimplemented command %08X\n", cmd);
			}
		}
	}
}

uint32_t nv2a_renderer::geforce_r(offs_t offset, uint32_t mem_mask)
{
	static int x, ret;

	ret = 0;
	if (offset == 0x1804f6) {
		x = x ^ 0x08080808;
		ret = x;
	}
	if ((offset >= 0x00100000 / 4) && (offset < 0x00101000 / 4)) {
		//machine().logerror("NV_2A: read PFB[%06X] mask %08X value %08X\n",offset*4-0x00100000,mem_mask,ret);
		if (offset == 0x100200 / 4)
			return 3;
	}
	else if ((offset >= 0x00101000 / 4) && (offset < 0x00102000 / 4)) {
		//machine().logerror("NV_2A: read STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,ret);
	}
	else if ((offset >= 0x00002000 / 4) && (offset < 0x00004000 / 4)) {
		ret = pfifo[offset - 0x00002000 / 4];
		// PFIFO.CACHE1.STATUS or PFIFO.RUNOUT_STATUS
		if ((offset == 0x3214 / 4) || (offset == 0x2400 / 4))
			ret = 0x10;
		//machine().logerror("NV_2A: read PFIFO[%06X] value %08X\n",offset*4-0x00002000,ret);
	}
	else if ((offset >= 0x00700000 / 4) && (offset < 0x00800000 / 4)) {
		ret = ramin[offset - 0x00700000 / 4];
		//machine().logerror("NV_2A: read PRAMIN[%06X] value %08X\n",offset*4-0x00700000,ret);
	}
	else if ((offset >= 0x00400000 / 4) && (offset < 0x00402000 / 4)) {
		ret = pgraph[offset - 0x00400000 / 4];
		//machine().logerror("NV_2A: read PGRAPH[%06X] value %08X\n",offset*4-0x00400000,ret);
	}
	else if ((offset >= 0x00600000 / 4) && (offset < 0x00601000 / 4)) {
		ret = pcrtc[offset - 0x00600000 / 4];
		//machine().logerror("NV_2A: read PCRTC[%06X] value %08X\n",offset*4-0x00600000,ret);
	}
	else if ((offset >= 0x00000000 / 4) && (offset < 0x00001000 / 4)) {
		ret = pmc[offset - 0x00000000 / 4];
		//machine().logerror("NV_2A: read PMC[%06X] value %08X\n",offset*4-0x00000000,ret);
	}
	else if ((offset >= 0x00800000 / 4) && (offset < 0x00900000 / 4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel, subchannel, suboffset;

		suboffset = offset - 0x00800000 / 4;
		chanel = (suboffset >> (16 - 2)) & 31;
		subchannel = (suboffset >> (13 - 2)) & 7;
		suboffset = suboffset & 0x7ff;
		if (suboffset < 0x80 / 4)
			ret = channel[chanel][subchannel].regs[suboffset];
		//machine().logerror("NV_2A: read channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,ret);
		return ret;
	}
	//machine().logerror("NV_2A: read at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,ret);
	return ret;
}

void nv2a_renderer::geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old;
	bool update_int;

	update_int = false;
	if ((offset >= 0x00101000 / 4) && (offset < 0x00102000 / 4)) {
		//machine().logerror("NV_2A: write STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,data);
	}
	else if ((offset >= 0x00002000 / 4) && (offset < 0x00004000 / 4)) {
		int e = offset - 0x00002000 / 4;
		if (e >= (sizeof(pfifo) / sizeof(uint32_t)))
			return;
		COMBINE_DATA(pfifo + e);
		//machine().logerror("NV_2A: read PFIFO[%06X]=%08X\n",offset*4-0x00002000,data & mem_mask); // 2210 pfifo ramht & 1f0 << 12
	}
	else if ((offset >= 0x00700000 / 4) && (offset < 0x00800000 / 4)) {
		int e = offset - 0x00700000 / 4;
		if (e >= (sizeof(ramin) / sizeof(uint32_t)))
			return;
		COMBINE_DATA(ramin + e);
		//machine().logerror("NV_2A: write PRAMIN[%06X]=%08X\n",offset*4-0x00700000,data & mem_mask);
	}
	else if ((offset >= 0x00400000 / 4) && (offset < 0x00402000 / 4)) {
		int e = offset - 0x00400000 / 4;
		if (e >= (sizeof(pgraph) / sizeof(uint32_t)))
			return;
		old = pgraph[e];
		COMBINE_DATA(pgraph + e);
		if (e == 0x100 / 4) {
			pgraph[e] = old & ~data;
			if (data & 1)
				pgraph[0x108 / 4] = 0;
			update_int = true;
		}
		if (e == 0x140 / 4)
			update_int = true;
		if (e == 0x720 / 4) {
			if ((data & 1) && (puller_waiting == 2)) {
				puller_waiting = 0;
				puller_timer->enable();
				puller_timer->adjust(attotime::zero);
			}
		}
		if ((e >= 0x900 / 4) && (e < 0xa00 / 4))
			pgraph[e] = 0;
		//machine().logerror("NV_2A: write PGRAPH[%06X]=%08X\n",offset*4-0x00400000,data & mem_mask);
	}
	else if ((offset >= 0x00600000 / 4) && (offset < 0x00601000 / 4)) {
		int e = offset - 0x00600000 / 4;
		if (e >= (sizeof(pcrtc) / sizeof(uint32_t)))
			return;
		old = pcrtc[e];
		COMBINE_DATA(pcrtc + e);
		if (e == 0x100 / 4) {
			pcrtc[e] = old & ~data;
			update_int = true;
		}
		if (e == 0x140 / 4)
			update_int = true;
		if (e == 0x800 / 4) {
			displayedtarget = (uint32_t *)direct_access_ptr(pcrtc[e]);
			LOG("crtc buffer %08X\n\r", data);
		}
		//machine().logerror("NV_2A: write PCRTC[%06X]=%08X\n",offset*4-0x00600000,data & mem_mask);
	}
	else if ((offset >= 0x00000000 / 4) && (offset < 0x00001000 / 4)) {
		int e = offset - 0x00000000 / 4;
		if (e >= (sizeof(pmc) / sizeof(uint32_t)))
			return;
		COMBINE_DATA(pmc + e);
		if (e == 0x200 / 4) // PMC.ENABLE register
			if (data & 0x1100) // either PFIFO or PGRAPH enabled
				for (int ch = 0; ch < 32; ch++) // zero dma_get in all the channels
					channel[ch][0].regs[0x44 / 4] = 0;
		//machine().logerror("NV_2A: write PMC[%06X]=%08X\n",offset*4-0x00000000,data & mem_mask);
	}
	else if ((offset >= 0x00800000 / 4) && (offset < 0x00900000 / 4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel, subchannel, suboffset;
		//int method, count, handle, objclass;

		suboffset = offset - 0x00800000 / 4;
		chanel = (suboffset >> (16 - 2)) & 31;
		subchannel = (suboffset >> (13 - 2)) & 7;
		suboffset = suboffset & 0x7ff;
		//machine().logerror("NV_2A: write channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,data & mem_mask);
		COMBINE_DATA(&channel[chanel][subchannel].regs[suboffset]);
		if (suboffset >= 0x80 / 4)
			return;
		if ((suboffset == 0x40 / 4) || (suboffset == 0x44 / 4)) { // DMA_PUT or DMA_GET
			uint32_t *dmaput, *dmaget;

			dmaput = &channel[chanel][0].regs[0x40 / 4];
			dmaget = &channel[chanel][0].regs[0x44 / 4];
			//printf("dmaget %08X dmaput %08X\n\r",*dmaget,*dmaput);
			if (*dmaget != *dmaput) {
				if (puller_waiting == 0) {
					puller_space = &space;
					puller_timer->enable();
					puller_timer->adjust(attotime::zero);
				}
			}
		}
	}
	//else
	//      machine().logerror("NV_2A: write at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,data);
	if (update_int == true) {
		if (update_interrupts() == true)
			irq_callback(1); // IRQ 3
		else
			irq_callback(0); // IRQ 3
	}
}

void nv2a_renderer::savestate_items()
{
}

void nv2a_renderer::set_ram_base(void *base)
{
	basemempointer = (uint8_t*)base;
	topmempointer = basemempointer + 512 * 1024 * 1024 - 1;
}

void nv2a_renderer::start(address_space *cpu_space)
{
	puller_timer = machine().scheduler().timer_alloc(timer_expired_delegate(&nv2a_renderer::puller_timer_work, "NV2A Puller Timer", this));
	puller_timer->enable(false);
}
