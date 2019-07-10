//
// PUBLIC DOMAIN CRT STYLED SCAN-LINE SHADER
//
//   by Timothy Lottes
//
// This is more along the style of a really good CGA arcade monitor.
// With RGB inputs instead of NTSC.
// The shadow mask example has the mask rotated 90 degrees for less chromatic aberration.
//
// Left it unoptimized to show the theory behind the algorithm.
//
// It is an example what I personally would want as a display option for pixel art games.
// Please take and use, change, or whatever.

//Comment these out to disable the corresponding effect.
#define VERTICAL //rotates shadow mask effect to fix vertical games on landscape monitors
#define CURVATURE //Screen curvature effect.
#define YUV //Tint and Saturation adjustments.  You adjust the settings in Lottes_CRT.vsh now...
#define GAMMA_CONTRAST_BOOST //Expands contrast and makes image brighter but causes clipping.
#define BLOOM //enables a bloom effect
//#define MASK_APERTURE_GRILL //Only uncomment one of the MASK patterns at a time...
#define MASK_TV
//#define MASK_VGA
//#define ORIGINAL_SCANLINES //Enable to use the original scanlines.
//#define ORIGINAL_HARDPIX //Enable to use the original hardPix calculation.

//Normal MAME GLSL Uniforms
uniform sampler2D color_texture;
uniform vec2      color_texture_sz;			// size of color_texture
uniform vec2      color_texture_pow2_sz;	// size of color texture rounded up to power of 2
uniform vec2      screen_texture_sz;		// size of output resolution
uniform vec2      screen_texture_pow2_sz;   // size of output resolution rounded up to power of 2

//CRT Filter Variables
const float hardScan=-20.0; //-8,-12,-16, etc to make scalines more prominent.
const vec2 warp=vec2(1.0/64.0,1.0/48.0);  //adjusts the warp filter (curvature).
const float maskDark=0.4; //Sets how dark a "dark subpixel" is in the aperture pattern.
const float maskLight=1.5; //Sets how dark a "bright subpixel" is in the aperture pattern.
const float hardPix=-5.0; //-1,-2,-4, etc to make the upscaling sharper.
const float hardBloomScan=-2.5;
const float hardBloomPix=-1.75;
const float bloomAmount=1.0/12.0; //Lower this if there is too much bloom!
const float blackClip = 0.02;
const float brightMult = 1.2;
const float maskStrength = 0.6; //This sets the strength of the shadow mask effect
const vec3 gammaBoost = vec3(1.0/1.15, 1.0/1.15, 1.0/1.15);
varying vec3 YUVr;
varying vec3 YUVg;
varying vec3 YUVb;

//CRT Filter Functions

// sRGB to Linear.
// Assuing using sRGB typed textures this should not be needed.
float ToLinear1(float c){return(c<=0.04045)?c/12.92:pow((c+0.055)/1.055,2.4);}
vec3 ToLinear(vec3 c){return vec3(ToLinear1(c.r),ToLinear1(c.g),ToLinear1(c.b));}

// Linear to sRGB.
// Assuing using sRGB typed textures this should not be needed.
float ToSrgb1(float c){return(c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055);}
vec3 ToSrgb(vec3 c){return vec3(ToSrgb1(c.r),ToSrgb1(c.g),ToSrgb1(c.b));}

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(vec2 pos,vec2 off){
  pos=(floor(pos*color_texture_pow2_sz+off)+0.5)/color_texture_pow2_sz;
  //if(max(abs(pos.x-0.5),abs(pos.y-0.5))>0.5)return vec3(0.0,0.0,0.0);
  return ToLinear(texture2D(color_texture,pos.xy).rgb);}

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos){pos=pos*color_texture_pow2_sz;return -((pos-floor(pos))-vec2(0.5));}
    
// 1D Gaussian.
float Gaus(float pos,float scale){return exp2(scale*pos*pos);}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos,float off){
  vec3 b=Fetch(pos,vec2(-1.0,off));
  vec3 c=Fetch(pos,vec2( 0.0,off));
  vec3 d=Fetch(pos,vec2( 1.0,off));
  float dst=Dist(pos).x;
  // Convert distance to weight.
#ifdef ORIGINAL_HARDPIX
  float scale=hardPix;
#else
  float scale=hardPix * max(0.2, 1.5-color_texture_sz.x/512.0);
#endif
  float wb=Gaus(dst-1.0,scale);
  float wc=Gaus(dst+0.0,scale);
  float wd=Gaus(dst+1.0,scale);
  // Return filtered sample.
  return (b*wb+c*wc+d*wd)/(wb+wc+wd);}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos,float off){
  vec3 a=Fetch(pos,vec2(-2.0,off));
  vec3 b=Fetch(pos,vec2(-1.0,off));
  vec3 c=Fetch(pos,vec2( 0.0,off));
  vec3 d=Fetch(pos,vec2( 1.0,off));
  vec3 e=Fetch(pos,vec2( 2.0,off));
  float dst=Dist(pos).x;
  // Convert distance to weight.
#ifdef ORIGINAL_HARDPIX
  float scale=hardPix;
#else
  float scale=hardPix * max(0.2, 1.5-color_texture_sz.x/512.0);
#endif
  float wa=Gaus(dst-2.0,scale);
  float wb=Gaus(dst-1.0,scale);
  float wc=Gaus(dst+0.0,scale);
  float wd=Gaus(dst+1.0,scale);
  float we=Gaus(dst+2.0,scale);
  // Return filtered sample.
  return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);}
  
vec3 Horz7(vec2 pos,float off){
  vec3 a=Fetch(pos,vec2(-3.0,off));
  vec3 b=Fetch(pos,vec2(-2.0,off));
  vec3 c=Fetch(pos,vec2(-1.0,off));
  vec3 d=Fetch(pos,vec2( 0.0,off));
  vec3 e=Fetch(pos,vec2( 1.0,off));
  vec3 f=Fetch(pos,vec2( 2.0,off));
  vec3 g=Fetch(pos,vec2( 3.0,off));
  float dst=Dist(pos).x;
  // Convert distance to weight.
  float scale=hardBloomPix* max(0.5, 1.5-color_texture_sz.x/512.0);
  float wa=Gaus(dst-3.0,scale);
  float wb=Gaus(dst-2.0,scale);
  float wc=Gaus(dst-1.0,scale);
  float wd=Gaus(dst+0.0,scale);
  float we=Gaus(dst+1.0,scale);
  float wf=Gaus(dst+2.0,scale);
  float wg=Gaus(dst+3.0,scale);
  // Return filtered sample.
  return (a*wa+b*wb+c*wc+d*wd+e*we+f*wf+g*wg)/(wa+wb+wc+wd+we+wf+wg);}

// Return scanline weight.
float Scan(vec2 pos,float off){
  float dst=Dist(pos).y;
#ifdef ORIGINAL_SCANLINES
  return Gaus(dst+off,hardScan);}
#else
  vec3 col=Fetch(pos,vec2(0.0));
  return Gaus(dst+off,hardScan/(dot(col,col)*0.25+1.0));} //Modified to make scanline respond to pixel brightness
#endif

// Return scanline weight for bloom.
float BloomScan(vec2 pos,float off){
  float dst=Dist(pos).y;
  return Gaus(dst+off,hardBloomScan);}

// Allow nearest three lines to effect pixel.
vec3 Tri(vec2 pos){
  vec3 a=Horz3(pos,-1.0);
  vec3 b=Horz5(pos, 0.0);
  vec3 c=Horz3(pos, 1.0);
  float wa=Scan(pos,-1.0);
  float wb=Scan(pos, 0.0);
  float wc=Scan(pos, 1.0);
  return a*wa+b*wb+c*wc;}
  
// Small bloom.
vec3 Bloom(vec2 pos){
  vec3 a=Horz5(pos,-2.0);
  vec3 b=Horz7(pos,-1.0);
  vec3 c=Horz7(pos, 0.0);
  vec3 d=Horz7(pos, 1.0);
  vec3 e=Horz5(pos, 2.0);
  float wa=BloomScan(pos,-2.0);
  float wb=BloomScan(pos,-1.0);
  float wc=BloomScan(pos, 0.0);
  float wd=BloomScan(pos, 1.0);
  float we=BloomScan(pos, 2.0);
  return a*wa+b*wb+c*wc+d*wd+e*we;}

// Distortion of scanlines, and end of screen alpha.
vec2 Warp(vec2 pos){
  pos=pos*2.0-1.0;    
  pos*=vec2(1.0+(pos.y*pos.y)*warp.x,1.0+(pos.x*pos.x)*warp.y);
  return pos*0.5+0.5;}
	
// Shadow mask.

vec3 Mask(vec2 pos){
#ifdef VERTICAL
pos.xy=pos.yx;
#endif
#ifdef MASK_VGA
  pos.x+=pos.y*3.0;
  vec3 mask=vec3(maskDark,maskDark,maskDark);
  pos.x=fract(pos.x/6.0);
  if(pos.x<0.333)mask.r=maskLight;
  else if(pos.x<0.666)mask.g=maskLight;
  else mask.b=maskLight;
#endif
#ifdef MASK_TV
  float line=maskLight;
  float odd=0.0;
  if(fract(pos.x/6.0)<0.5)odd=1.0;
  if(fract((pos.y+odd)/2.0)<0.5)line=maskDark;  
  pos.x=fract(pos.x/3.0);
  vec3 mask=vec3(maskDark,maskDark,maskDark);
  if(pos.x<0.333)mask.r=maskLight;
  else if(pos.x<0.666)mask.g=maskLight;
  else mask.b=maskLight;
  mask*=line;     
#endif
#ifdef MASK_APERTURE_GRILL
  pos.x=fract(pos.x/3.0);
  vec3 mask=vec3(maskDark,maskDark,maskDark);
  if(pos.x<0.333)mask.r=maskLight;
  else if(pos.x<0.666)mask.g=maskLight;
  else mask.b=maskLight;  
#endif
  return mask;} 

void main(void){
#ifdef CURVATURE
  vec2 pos=Warp(gl_TexCoord[0].xy);
#else
  vec2 pos=gl_TexCoord[0].xy;
#endif
  gl_FragColor.a=texture2D(color_texture,pos.xy).a;
  gl_FragColor.rgb=Tri(pos)*mix(vec3(1.0),Mask(gl_FragCoord.xy),maskStrength);
#ifdef BLOOM
 gl_FragColor.rgb+=Bloom(pos)*bloomAmount; 
#endif
#ifdef YUV
  gl_FragColor.rgb = vec3(dot(YUVr,gl_FragColor.rgb),dot(YUVg,gl_FragColor.rgb),dot(YUVb,gl_FragColor.rgb));
  gl_FragColor.rgb=clamp(gl_FragColor.rgb,0.0,1.0);
#endif
#ifdef GAMMA_CONTRAST_BOOST
  gl_FragColor.rgb=brightMult*pow(gl_FragColor.rgb,gammaBoost )-vec3(blackClip);
#endif
	gl_FragColor.rgb=clamp(ToSrgb(gl_FragColor.rgb),0.0,1.0);
}