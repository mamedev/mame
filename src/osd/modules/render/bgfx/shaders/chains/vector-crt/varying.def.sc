vec2 v_texcoord0   : TEXCOORD0 = vec2(0.0, 0.0);
vec4 v_beam        : TEXCOORD1 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_beam_color  : COLOR0    = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_beam_timing : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);

vec3 a_position  : POSITION;
vec4 a_color0    : COLOR0;
vec2 a_texcoord0 : TEXCOORD0;

vec4 i_data0 : TEXCOORD7;
vec4 i_data1 : TEXCOORD6;
vec4 i_data2 : TEXCOORD5;
