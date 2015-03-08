uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

float xstep = 1.0/screen_width;
float ystep = 1.0/screen_height;

float LinearizeDepth(float z)
{
	float n = 0.5; // camera z near
  	float f = 10000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}
vec4 getPixel(int x, int y, sampler2D sampler){
	return texture2D( sampler , vec2( gl_TexCoord[0].x+xstep*float(x), gl_TexCoord[0].y+ystep*float(y) ) );
}
vec4 getColor(int x, int y){
	return getPixel(x,y,Texture0);
}
float getDepth(int x, int y){
	return LinearizeDepth(getPixel(x,y,Texture1).r);
}

void main (void)
{
	float depth = getDepth(0,0);	

	int bs = int(depth * 32.0); if(bs==32) bs = 0;
	int ao = int((1.0-depth) * 8.0);
	
	float outline = (1.0-abs(depth-getDepth(1,1))*256.0);
	
	vec4 color = vec4(0,0,0,0);
	for(int x=-bs;x<=bs;x++)
		for(int y=-bs;y<=bs;y++)
			color += getColor(x,y);
	color /= (bs*2+1)*(bs*2+1);

	float aveDepth = 0.0;
	int c = 0;
	for(int x=-ao;x<=ao;x++)
		for(int y=-ao;y<=ao;y++){
			float tmp = getDepth(x,y);
			if(abs(depth-tmp)<=.001){
				aveDepth += tmp;
				c++;
			}
		}
	aveDepth /= float(c);

	float diff = (1.0-clamp((depth - aveDepth)*1024.0,0.0,1.0));

	gl_FragColor = getColor(0,0);
	//gl_FragColor *= diff;
	gl_FragColor *= outline;
	gl_FragColor.a = 1.0;
}