uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

float LinearizeDepth(float z)
{
	float n = 0.5; // camera z near
  	float f = 10000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;	
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);


	int bs = depth * 8;
	int ao = (1-depth) * 8;
	
	
	/*
	vec4 color = vec4(0,0,0,0);
	for(int x=-bs;x<=bs;x++)
		for(int y=-bs;y<=bs;y++)
			color += texture2D( Texture0 , vec2( gl_TexCoord[0].x+x*xstep, gl_TexCoord[0].y+y*ystep) );
	color /= (bs*2+1)*(bs*2+1);
	*/

	gl_FragColor = color;

	float aveDepth = 0;
	int c = 0;
	for(int x=-ao;x<=ao;x++)
		for(int y=-ao;y<=ao;y++){
			float d = LinearizeDepth(texture2D( Texture1 , vec2( gl_TexCoord[0].x+x*xstep, gl_TexCoord[0].y+y*ystep) ));
			if(abs(d-depth)<=.001){
				aveDepth += d;
				c++;
			}
		}
	aveDepth /= c;
	

	float diff = 1-clamp((depth - aveDepth)*1024,0,1);
	gl_FragColor *= diff;
	gl_FragColor.a = 1;
}