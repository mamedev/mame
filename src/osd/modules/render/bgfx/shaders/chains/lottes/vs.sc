varying float saturation;
varying float tint;
varying float U; //U and W are for the tint/saturation calculations
varying float W;
varying vec3 YUVr;
varying vec3 YUVg;
varying vec3 YUVb;
#define PI 3.141592653589

void main()
{  
	//gl_TexCoord[0]  = gl_MultiTexCoord0;
	gl_TexCoord[0]  = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position     = ftransform();
	
	//Had to move the YUV calculations to the vertex shader for space
	saturation = 1.1;  // 1.0 is normal saturation. Increase as needed.
	tint = 0.0;  //0.0 is 0.0 degrees of Tint. Adjust as needed.
	U = cos(tint*PI/180.0);
	W = sin(tint*PI/180.0);
	YUVr=vec3(0.701*saturation*U+0.16774*saturation*W+0.299,0.587-0.32931*saturation*W-0.587*saturation*U,-0.497*saturation*W-0.114*saturation*U+0.114);
    YUVg=vec3(-0.3281*saturation*W-0.299*saturation*U+0.299,0.413*saturation*U+0.03547*saturation*W+0.587,0.114+0.29265*saturation*W-0.114*saturation*U);
    YUVb=vec3(0.299+1.24955*saturation*W-0.299*saturation*U,-1.04634*saturation*W-0.587*saturation*U+0.587,0.886*saturation*U-0.20321*saturation*W+0.114);
}


