/***************************************************************************

    rgbvmx.h

    VMX/Altivec optimised RGB utilities.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RGBVMX__
#define __RGBVMX__

#ifndef __APPLE_ALTIVEC__
#include <altivec.h>
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* intermediate RGB values are stored in a vector */
typedef vector signed short rgbint;

/* intermediate RGB values are stored in a vector */
typedef vector signed short rgbaint;



/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_comp_to_rgbint - converts a trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_comp_to_rgbint(rgbint *rgb, INT16 r, INT16 g, INT16 b)
{
	rgbint result = { 0, r, g, b, 0, 0, 0, 0 };
	*rgb = result;
}


/*-------------------------------------------------
    rgba_comp_to_rgbint - converts a quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_comp_to_rgbaint(rgbaint *rgb, INT16 a, INT16 r, INT16 g, INT16 b)
{
	rgbaint result = { a, r, g, b, 0, 0, 0, 0 };
	*rgb = result;
}


/*-------------------------------------------------
    rgb_to_rgbint - converts a packed trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_to_rgbint(rgbint *rgb, rgb_t color)
{
	vector signed char temp = (vector signed char)vec_perm((vector signed int)vec_lde(0, &color), vec_splat_s32(0), vec_lvsl(0, &color));
	*rgb = vec_mergeh((vector signed char)vec_splat_s32(0), temp);
}


/*-------------------------------------------------
    rgba_to_rgbaint - converts a packed quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_to_rgbaint(rgbaint *rgb, rgb_t color)
{
	vector signed char temp = (vector signed char)vec_perm((vector signed int)vec_lde(0, &color), vec_splat_s32(0), vec_lvsl(0, &color));
	*rgb = vec_mergeh((vector signed char)vec_splat_s32(0), temp);
}


/*-------------------------------------------------
    rgbint_to_rgb - converts an rgbint back to
    a packed trio of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb(const rgbint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	rgb_t result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbaint_to_rgba - converts an rgbint back to
    a packed quad of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba(const rgbaint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	rgb_t result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbint_to_rgb_clamp - converts an rgbint back
    to a packed trio of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb_clamp(const rgbint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	rgb_t result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbaint_to_rgba_clamp - converts an rgbint back
    to a packed quad of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba_clamp(const rgbaint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	rgb_t result;
	vec_ste(temp, 0, &result);
	return result;
}



/***************************************************************************
    CORE MATH
***************************************************************************/

/*-------------------------------------------------
    rgbint_add - add two rgbint values
-------------------------------------------------*/

INLINE void rgbint_add(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_add(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_add - add two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_add(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_add(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_sub - subtract two rgbint values
-------------------------------------------------*/

INLINE void rgbint_sub(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_sub(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_sub - subtract two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_sub(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_sub(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_subr - reverse subtract two rgbint
    values
-------------------------------------------------*/

INLINE void rgbint_subr(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_sub(*color2, *color1);
}


/*-------------------------------------------------
    rgbaint_subr - reverse subtract two rgbaint
    values
-------------------------------------------------*/

INLINE void rgbaint_subr(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_sub(*color2, *color1);
}



/***************************************************************************
    TABLES
***************************************************************************/

static const struct
{
	rgbaint	maxbyte;
	rgbaint	scale_table[256];
} rgbvmx_statics =
{
	{ 255, 255, 255, 255, 255, 255, 255, 255 },
	{
		{   1, 256,   1, 256,   1, 256,   1, 256 }, {   2, 255,   2, 255,   2, 255,   2, 255 },
		{   3, 254,   3, 254,   3, 254,   3, 254 }, {   4, 253,   4, 253,   4, 253,   4, 253 },
		{   5, 252,   5, 252,   5, 252,   5, 252 }, {   6, 251,   6, 251,   6, 251,   6, 251 },
		{   7, 250,   7, 250,   7, 250,   7, 250 }, {   8, 249,   8, 249,   8, 249,   8, 249 },
		{   9, 248,   9, 248,   9, 248,   9, 248 }, {  10, 247,  10, 247,  10, 247,  10, 247 },
		{  11, 246,  11, 246,  11, 246,  11, 246 }, {  12, 245,  12, 245,  12, 245,  12, 245 },
		{  13, 244,  13, 244,  13, 244,  13, 244 }, {  14, 243,  14, 243,  14, 243,  14, 243 },
		{  15, 242,  15, 242,  15, 242,  15, 242 }, {  16, 241,  16, 241,  16, 241,  16, 241 },
		{  17, 240,  17, 240,  17, 240,  17, 240 }, {  18, 239,  18, 239,  18, 239,  18, 239 },
		{  19, 238,  19, 238,  19, 238,  19, 238 }, {  20, 237,  20, 237,  20, 237,  20, 237 },
		{  21, 236,  21, 236,  21, 236,  21, 236 }, {  22, 235,  22, 235,  22, 235,  22, 235 },
		{  23, 234,  23, 234,  23, 234,  23, 234 }, {  24, 233,  24, 233,  24, 233,  24, 233 },
		{  25, 232,  25, 232,  25, 232,  25, 232 }, {  26, 231,  26, 231,  26, 231,  26, 231 },
		{  27, 230,  27, 230,  27, 230,  27, 230 }, {  28, 229,  28, 229,  28, 229,  28, 229 },
		{  29, 228,  29, 228,  29, 228,  29, 228 }, {  30, 227,  30, 227,  30, 227,  30, 227 },
		{  31, 226,  31, 226,  31, 226,  31, 226 }, {  32, 225,  32, 225,  32, 225,  32, 225 },
		{  33, 224,  33, 224,  33, 224,  33, 224 }, {  34, 223,  34, 223,  34, 223,  34, 223 },
		{  35, 222,  35, 222,  35, 222,  35, 222 }, {  36, 221,  36, 221,  36, 221,  36, 221 },
		{  37, 220,  37, 220,  37, 220,  37, 220 }, {  38, 219,  38, 219,  38, 219,  38, 219 },
		{  39, 218,  39, 218,  39, 218,  39, 218 }, {  40, 217,  40, 217,  40, 217,  40, 217 },
		{  41, 216,  41, 216,  41, 216,  41, 216 }, {  42, 215,  42, 215,  42, 215,  42, 215 },
		{  43, 214,  43, 214,  43, 214,  43, 214 }, {  44, 213,  44, 213,  44, 213,  44, 213 },
		{  45, 212,  45, 212,  45, 212,  45, 212 }, {  46, 211,  46, 211,  46, 211,  46, 211 },
		{  47, 210,  47, 210,  47, 210,  47, 210 }, {  48, 209,  48, 209,  48, 209,  48, 209 },
		{  49, 208,  49, 208,  49, 208,  49, 208 }, {  50, 207,  50, 207,  50, 207,  50, 207 },
		{  51, 206,  51, 206,  51, 206,  51, 206 }, {  52, 205,  52, 205,  52, 205,  52, 205 },
		{  53, 204,  53, 204,  53, 204,  53, 204 }, {  54, 203,  54, 203,  54, 203,  54, 203 },
		{  55, 202,  55, 202,  55, 202,  55, 202 }, {  56, 201,  56, 201,  56, 201,  56, 201 },
		{  57, 200,  57, 200,  57, 200,  57, 200 }, {  58, 199,  58, 199,  58, 199,  58, 199 },
		{  59, 198,  59, 198,  59, 198,  59, 198 }, {  60, 197,  60, 197,  60, 197,  60, 197 },
		{  61, 196,  61, 196,  61, 196,  61, 196 }, {  62, 195,  62, 195,  62, 195,  62, 195 },
		{  63, 194,  63, 194,  63, 194,  63, 194 }, {  64, 193,  64, 193,  64, 193,  64, 193 },
		{  65, 192,  65, 192,  65, 192,  65, 192 }, {  66, 191,  66, 191,  66, 191,  66, 191 },
		{  67, 190,  67, 190,  67, 190,  67, 190 }, {  68, 189,  68, 189,  68, 189,  68, 189 },
		{  69, 188,  69, 188,  69, 188,  69, 188 }, {  70, 187,  70, 187,  70, 187,  70, 187 },
		{  71, 186,  71, 186,  71, 186,  71, 186 }, {  72, 185,  72, 185,  72, 185,  72, 185 },
		{  73, 184,  73, 184,  73, 184,  73, 184 }, {  74, 183,  74, 183,  74, 183,  74, 183 },
		{  75, 182,  75, 182,  75, 182,  75, 182 }, {  76, 181,  76, 181,  76, 181,  76, 181 },
		{  77, 180,  77, 180,  77, 180,  77, 180 }, {  78, 179,  78, 179,  78, 179,  78, 179 },
		{  79, 178,  79, 178,  79, 178,  79, 178 }, {  80, 177,  80, 177,  80, 177,  80, 177 },
		{  81, 176,  81, 176,  81, 176,  81, 176 }, {  82, 175,  82, 175,  82, 175,  82, 175 },
		{  83, 174,  83, 174,  83, 174,  83, 174 }, {  84, 173,  84, 173,  84, 173,  84, 173 },
		{  85, 172,  85, 172,  85, 172,  85, 172 }, {  86, 171,  86, 171,  86, 171,  86, 171 },
		{  87, 170,  87, 170,  87, 170,  87, 170 }, {  88, 169,  88, 169,  88, 169,  88, 169 },
		{  89, 168,  89, 168,  89, 168,  89, 168 }, {  90, 167,  90, 167,  90, 167,  90, 167 },
		{  91, 166,  91, 166,  91, 166,  91, 166 }, {  92, 165,  92, 165,  92, 165,  92, 165 },
		{  93, 164,  93, 164,  93, 164,  93, 164 }, {  94, 163,  94, 163,  94, 163,  94, 163 },
		{  95, 162,  95, 162,  95, 162,  95, 162 }, {  96, 161,  96, 161,  96, 161,  96, 161 },
		{  97, 160,  97, 160,  97, 160,  97, 160 }, {  98, 159,  98, 159,  98, 159,  98, 159 },
		{  99, 158,  99, 158,  99, 158,  99, 158 }, { 100, 157, 100, 157, 100, 157, 100, 157 },
		{ 101, 156, 101, 156, 101, 156, 101, 156 }, { 102, 155, 102, 155, 102, 155, 102, 155 },
		{ 103, 154, 103, 154, 103, 154, 103, 154 }, { 104, 153, 104, 153, 104, 153, 104, 153 },
		{ 105, 152, 105, 152, 105, 152, 105, 152 }, { 106, 151, 106, 151, 106, 151, 106, 151 },
		{ 107, 150, 107, 150, 107, 150, 107, 150 }, { 108, 149, 108, 149, 108, 149, 108, 149 },
		{ 109, 148, 109, 148, 109, 148, 109, 148 }, { 110, 147, 110, 147, 110, 147, 110, 147 },
		{ 111, 146, 111, 146, 111, 146, 111, 146 }, { 112, 145, 112, 145, 112, 145, 112, 145 },
		{ 113, 144, 113, 144, 113, 144, 113, 144 }, { 114, 143, 114, 143, 114, 143, 114, 143 },
		{ 115, 142, 115, 142, 115, 142, 115, 142 }, { 116, 141, 116, 141, 116, 141, 116, 141 },
		{ 117, 140, 117, 140, 117, 140, 117, 140 }, { 118, 139, 118, 139, 118, 139, 118, 139 },
		{ 119, 138, 119, 138, 119, 138, 119, 138 }, { 120, 137, 120, 137, 120, 137, 120, 137 },
		{ 121, 136, 121, 136, 121, 136, 121, 136 }, { 122, 135, 122, 135, 122, 135, 122, 135 },
		{ 123, 134, 123, 134, 123, 134, 123, 134 }, { 124, 133, 124, 133, 124, 133, 124, 133 },
		{ 125, 132, 125, 132, 125, 132, 125, 132 }, { 126, 131, 126, 131, 126, 131, 126, 131 },
		{ 127, 130, 127, 130, 127, 130, 127, 130 }, { 128, 129, 128, 129, 128, 129, 128, 129 },
		{ 129, 128, 129, 128, 129, 128, 129, 128 }, { 130, 127, 130, 127, 130, 127, 130, 127 },
		{ 131, 126, 131, 126, 131, 126, 131, 126 }, { 132, 125, 132, 125, 132, 125, 132, 125 },
		{ 133, 124, 133, 124, 133, 124, 133, 124 }, { 134, 123, 134, 123, 134, 123, 134, 123 },
		{ 135, 122, 135, 122, 135, 122, 135, 122 }, { 136, 121, 136, 121, 136, 121, 136, 121 },
		{ 137, 120, 137, 120, 137, 120, 137, 120 }, { 138, 119, 138, 119, 138, 119, 138, 119 },
		{ 139, 118, 139, 118, 139, 118, 139, 118 }, { 140, 117, 140, 117, 140, 117, 140, 117 },
		{ 141, 116, 141, 116, 141, 116, 141, 116 }, { 142, 115, 142, 115, 142, 115, 142, 115 },
		{ 143, 114, 143, 114, 143, 114, 143, 114 }, { 144, 113, 144, 113, 144, 113, 144, 113 },
		{ 145, 112, 145, 112, 145, 112, 145, 112 }, { 146, 111, 146, 111, 146, 111, 146, 111 },
		{ 147, 110, 147, 110, 147, 110, 147, 110 }, { 148, 109, 148, 109, 148, 109, 148, 109 },
		{ 149, 108, 149, 108, 149, 108, 149, 108 }, { 150, 107, 150, 107, 150, 107, 150, 107 },
		{ 151, 106, 151, 106, 151, 106, 151, 106 }, { 152, 105, 152, 105, 152, 105, 152, 105 },
		{ 153, 104, 153, 104, 153, 104, 153, 104 }, { 154, 103, 154, 103, 154, 103, 154, 103 },
		{ 155, 102, 155, 102, 155, 102, 155, 102 }, { 156, 101, 156, 101, 156, 101, 156, 101 },
		{ 157, 100, 157, 100, 157, 100, 157, 100 }, { 158,  99, 158,  99, 158,  99, 158,  99 },
		{ 159,  98, 159,  98, 159,  98, 159,  98 }, { 160,  97, 160,  97, 160,  97, 160,  97 },
		{ 161,  96, 161,  96, 161,  96, 161,  96 }, { 162,  95, 162,  95, 162,  95, 162,  95 },
		{ 163,  94, 163,  94, 163,  94, 163,  94 }, { 164,  93, 164,  93, 164,  93, 164,  93 },
		{ 165,  92, 165,  92, 165,  92, 165,  92 }, { 166,  91, 166,  91, 166,  91, 166,  91 },
		{ 167,  90, 167,  90, 167,  90, 167,  90 }, { 168,  89, 168,  89, 168,  89, 168,  89 },
		{ 169,  88, 169,  88, 169,  88, 169,  88 }, { 170,  87, 170,  87, 170,  87, 170,  87 },
		{ 171,  86, 171,  86, 171,  86, 171,  86 }, { 172,  85, 172,  85, 172,  85, 172,  85 },
		{ 173,  84, 173,  84, 173,  84, 173,  84 }, { 174,  83, 174,  83, 174,  83, 174,  83 },
		{ 175,  82, 175,  82, 175,  82, 175,  82 }, { 176,  81, 176,  81, 176,  81, 176,  81 },
		{ 177,  80, 177,  80, 177,  80, 177,  80 }, { 178,  79, 178,  79, 178,  79, 178,  79 },
		{ 179,  78, 179,  78, 179,  78, 179,  78 }, { 180,  77, 180,  77, 180,  77, 180,  77 },
		{ 181,  76, 181,  76, 181,  76, 181,  76 }, { 182,  75, 182,  75, 182,  75, 182,  75 },
		{ 183,  74, 183,  74, 183,  74, 183,  74 }, { 184,  73, 184,  73, 184,  73, 184,  73 },
		{ 185,  72, 185,  72, 185,  72, 185,  72 }, { 186,  71, 186,  71, 186,  71, 186,  71 },
		{ 187,  70, 187,  70, 187,  70, 187,  70 }, { 188,  69, 188,  69, 188,  69, 188,  69 },
		{ 189,  68, 189,  68, 189,  68, 189,  68 }, { 190,  67, 190,  67, 190,  67, 190,  67 },
		{ 191,  66, 191,  66, 191,  66, 191,  66 }, { 192,  65, 192,  65, 192,  65, 192,  65 },
		{ 193,  64, 193,  64, 193,  64, 193,  64 }, { 194,  63, 194,  63, 194,  63, 194,  63 },
		{ 195,  62, 195,  62, 195,  62, 195,  62 }, { 196,  61, 196,  61, 196,  61, 196,  61 },
		{ 197,  60, 197,  60, 197,  60, 197,  60 }, { 198,  59, 198,  59, 198,  59, 198,  59 },
		{ 199,  58, 199,  58, 199,  58, 199,  58 }, { 200,  57, 200,  57, 200,  57, 200,  57 },
		{ 201,  56, 201,  56, 201,  56, 201,  56 }, { 202,  55, 202,  55, 202,  55, 202,  55 },
		{ 203,  54, 203,  54, 203,  54, 203,  54 }, { 204,  53, 204,  53, 204,  53, 204,  53 },
		{ 205,  52, 205,  52, 205,  52, 205,  52 }, { 206,  51, 206,  51, 206,  51, 206,  51 },
		{ 207,  50, 207,  50, 207,  50, 207,  50 }, { 208,  49, 208,  49, 208,  49, 208,  49 },
		{ 209,  48, 209,  48, 209,  48, 209,  48 }, { 210,  47, 210,  47, 210,  47, 210,  47 },
		{ 211,  46, 211,  46, 211,  46, 211,  46 }, { 212,  45, 212,  45, 212,  45, 212,  45 },
		{ 213,  44, 213,  44, 213,  44, 213,  44 }, { 214,  43, 214,  43, 214,  43, 214,  43 },
		{ 215,  42, 215,  42, 215,  42, 215,  42 }, { 216,  41, 216,  41, 216,  41, 216,  41 },
		{ 217,  40, 217,  40, 217,  40, 217,  40 }, { 218,  39, 218,  39, 218,  39, 218,  39 },
		{ 219,  38, 219,  38, 219,  38, 219,  38 }, { 220,  37, 220,  37, 220,  37, 220,  37 },
		{ 221,  36, 221,  36, 221,  36, 221,  36 }, { 222,  35, 222,  35, 222,  35, 222,  35 },
		{ 223,  34, 223,  34, 223,  34, 223,  34 }, { 224,  33, 224,  33, 224,  33, 224,  33 },
		{ 225,  32, 225,  32, 225,  32, 225,  32 }, { 226,  31, 226,  31, 226,  31, 226,  31 },
		{ 227,  30, 227,  30, 227,  30, 227,  30 }, { 228,  29, 228,  29, 228,  29, 228,  29 },
		{ 229,  28, 229,  28, 229,  28, 229,  28 }, { 230,  27, 230,  27, 230,  27, 230,  27 },
		{ 231,  26, 231,  26, 231,  26, 231,  26 }, { 232,  25, 232,  25, 232,  25, 232,  25 },
		{ 233,  24, 233,  24, 233,  24, 233,  24 }, { 234,  23, 234,  23, 234,  23, 234,  23 },
		{ 235,  22, 235,  22, 235,  22, 235,  22 }, { 236,  21, 236,  21, 236,  21, 236,  21 },
		{ 237,  20, 237,  20, 237,  20, 237,  20 }, { 238,  19, 238,  19, 238,  19, 238,  19 },
		{ 239,  18, 239,  18, 239,  18, 239,  18 }, { 240,  17, 240,  17, 240,  17, 240,  17 },
		{ 241,  16, 241,  16, 241,  16, 241,  16 }, { 242,  15, 242,  15, 242,  15, 242,  15 },
		{ 243,  14, 243,  14, 243,  14, 243,  14 }, { 244,  13, 244,  13, 244,  13, 244,  13 },
		{ 245,  12, 245,  12, 245,  12, 245,  12 }, { 246,  11, 246,  11, 246,  11, 246,  11 },
		{ 247,  10, 247,  10, 247,  10, 247,  10 }, { 248,   9, 248,   9, 248,   9, 248,   9 },
		{ 249,   8, 249,   8, 249,   8, 249,   8 }, { 250,   7, 250,   7, 250,   7, 250,   7 },
		{ 251,   6, 251,   6, 251,   6, 251,   6 }, { 252,   5, 252,   5, 252,   5, 252,   5 },
		{ 253,   4, 253,   4, 253,   4, 253,   4 }, { 254,   3, 254,   3, 254,   3, 254,   3 },
		{ 255,   2, 255,   2, 255,   2, 255,   2 }, { 256,   1, 256,   1, 256,   1, 256,   1 }
	}
};



/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbint_blend(rgbint *color1, const rgbint *color2, UINT8 color1scale)
{
	vector signed int temp;
	*color1 = vec_mergeh(*color1, *color2);
	temp = vec_msum(*color1, rgbvmx_statics.scale_table[color1scale], vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color1 = vec_packs(temp, temp);
}


/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbaint_blend(rgbaint *color1, const rgbaint *color2, UINT8 color1scale)
{
	vector signed int temp;
	*color1 = vec_mergeh(*color1, *color2);
	temp = vec_msum(*color1, rgbvmx_statics.scale_table[color1scale], vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color1 = vec_packs(temp, temp);
}


/*-------------------------------------------------
    rgbint_scale_and_clamp - scale the given
    color by an 8.8 scale factor and clamp to
    byte values
-------------------------------------------------*/

INLINE void rgbint_scale_and_clamp(rgbint *color, INT16 colorscale)
{
	rgbint splatmap = vec_splat((rgbint)vec_lvsl(0, &colorscale), 0);
	rgbint vecscale = vec_lde(0, &colorscale);
	vector signed int temp;
	vecscale = (rgbint)vec_perm(vecscale, vecscale, (vector unsigned char)splatmap);
	*color = (rgbint)vec_mergeh(*color, (rgbint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor and clamp to
    byte values
-------------------------------------------------*/

INLINE void rgbaint_scale_and_clamp(rgbaint *color, INT16 colorscale)
{
	rgbaint splatmap = vec_splat((rgbaint)vec_lvsl(0, &colorscale), 0);
	rgbaint vecscale = vec_lde(0, &colorscale);
	vector signed int temp;
	vecscale = (rgbaint)vec_perm(vecscale, vecscale, (vector unsigned char)splatmap);
	*color = (rgbaint)vec_mergeh(*color, (rgbaint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}


/*-------------------------------------------------
    rgb_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE rgb_t rgb_bilinear_filter(rgb_t rgb00, rgb_t rgb01, rgb_t rgb10, rgb_t rgb11, UINT8 u, UINT8 v)
{
	rgb_t	result;
	rgbint	color00 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb00), vec_splat_s32(0), vec_lvsl(0, &rgb00));
	rgbint	color01 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb01), vec_splat_s32(0), vec_lvsl(0, &rgb01));
	rgbint	color10 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb10), vec_splat_s32(0), vec_lvsl(0, &rgb10));
	rgbint	color11 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb11), vec_splat_s32(0), vec_lvsl(0, &rgb11));

	/* interleave color01 and color00 at the byte level */
	color01 = vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	color01 = vec_packs((vector signed int)color01, (vector signed int)color01);
	color01 = vec_packsu(color01, color01);
	vec_ste((vector unsigned int)color01, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgba_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE rgb_t rgba_bilinear_filter(rgb_t rgb00, rgb_t rgb01, rgb_t rgb10, rgb_t rgb11, UINT8 u, UINT8 v)
{
	rgb_t	result;
	rgbaint	color00 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb00), vec_splat_s32(0), vec_lvsl(0, &rgb00));
	rgbaint	color01 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb01), vec_splat_s32(0), vec_lvsl(0, &rgb01));
	rgbaint	color10 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb10), vec_splat_s32(0), vec_lvsl(0, &rgb10));
	rgbaint	color11 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb11), vec_splat_s32(0), vec_lvsl(0, &rgb11));

	/* interleave color01 and color00 at the byte level */
	color01 = vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbaint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	color01 = vec_packs((vector signed int)color01, (vector signed int)color01);
	color01 = vec_packsu(color01, color01);
	vec_ste((vector unsigned int)color01, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbint_bilinear_filter(rgbint *color, rgb_t rgb00, rgb_t rgb01, rgb_t rgb10, rgb_t rgb11, UINT8 u, UINT8 v)
{
	rgbint color00 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb00), vec_splat_s32(0), vec_lvsl(0, &rgb00));
	rgbint color01 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb01), vec_splat_s32(0), vec_lvsl(0, &rgb01));
	rgbint color10 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb10), vec_splat_s32(0), vec_lvsl(0, &rgb10));
	rgbint color11 = (rgbint)vec_perm((vector signed int)vec_lde(0, &rgb11), vec_splat_s32(0), vec_lvsl(0, &rgb11));

	/* interleave color01 and color00 at the byte level */
	color01 = vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	*color = vec_packs((vector signed int)color01, (vector signed int)color01);
}


/*-------------------------------------------------
    rgbaint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbaint_bilinear_filter(rgbaint *color, rgb_t rgb00, rgb_t rgb01, rgb_t rgb10, rgb_t rgb11, UINT8 u, UINT8 v)
{
	rgbaint color00 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb00), vec_splat_s32(0), vec_lvsl(0, &rgb00));
	rgbaint color01 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb01), vec_splat_s32(0), vec_lvsl(0, &rgb01));
	rgbaint color10 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb10), vec_splat_s32(0), vec_lvsl(0, &rgb10));
	rgbaint color11 = (rgbaint)vec_perm((vector signed int)vec_lde(0, &rgb11), vec_splat_s32(0), vec_lvsl(0, &rgb11));

	/* interleave color01 and color00 at the byte level */
	color01 = vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbaint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	*color = vec_packs((vector signed int)color01, (vector signed int)color01);
}


#endif /* __RGBVMX__ */
