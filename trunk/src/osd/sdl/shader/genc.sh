#! /bin/sh

##
## VSH
##

echo "const char glsl_general_vsh_src[] =" > glsl_general.vsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_general.vsh >> glsl_general.vsh.c
echo ";" >> glsl_general.vsh.c

##
## IDX16 FSH 
##

echo "const char glsl_plain_idx16_lut_fsh_src[] =" > glsl_plain_idx16_lut.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_plain_idx16_lut.fsh >> glsl_plain_idx16_lut.fsh.c
echo ";" >> glsl_plain_idx16_lut.fsh.c

echo "const char glsl_bilinear_idx16_lut_fsh_src[] =" > glsl_bilinear_idx16_lut.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_bilinear_idx16_lut.fsh >>  glsl_bilinear_idx16_lut.fsh.c
echo ";" >> glsl_bilinear_idx16_lut.fsh.c

##
## RGB 32 FSH LUT
##

echo "const char glsl_plain_rgb32_lut_fsh_src[] =" > glsl_plain_rgb32_lut.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_plain_rgb32_lut.fsh >>  glsl_plain_rgb32_lut.fsh.c
echo ";" >> glsl_plain_rgb32_lut.fsh.c

echo "const char glsl_bilinear_rgb32_lut_fsh_src[] =" > glsl_bilinear_rgb32_lut.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_bilinear_rgb32_lut.fsh >>  glsl_bilinear_rgb32_lut.fsh.c
echo ";" >> glsl_bilinear_rgb32_lut.fsh.c

##
## RGB 32 FSH DIRECT
##

echo "const char glsl_plain_rgb32_dir_fsh_src[] =" > glsl_plain_rgb32_dir.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_plain_rgb32_dir.fsh >>  glsl_plain_rgb32_dir.fsh.c
echo ";" >> glsl_plain_rgb32_dir.fsh.c

echo "const char glsl_bilinear_rgb32_dir_fsh_src[] =" > glsl_bilinear_rgb32_dir.fsh.c
sed -e 's/^/"/g' -e 's/$/\\n"/g' glsl_bilinear_rgb32_dir.fsh >>  glsl_bilinear_rgb32_dir.fsh.c
echo ";" >> glsl_bilinear_rgb32_dir.fsh.c

