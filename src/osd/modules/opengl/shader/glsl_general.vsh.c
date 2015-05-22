// license:BSD-3-Clause
// copyright-holders:Sven Gothel
const char glsl_general_vsh_src[] =
"\n"
"void main()\n"
"{\n"
"   gl_TexCoord[0]  = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"   gl_Position     = ftransform();\n"
"}\n"
"\n"
;
