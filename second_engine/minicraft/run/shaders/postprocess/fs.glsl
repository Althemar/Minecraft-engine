#version 400

in vec2 uv;

uniform sampler2D TexColor;
uniform sampler2D TexDepth;
uniform float screen_width;
uniform float screen_height;
uniform vec2 near_far;

out vec4 color_out;

float LinearizeDepth(float z)
{
	float n = near_far.x; // camera z near
  	float f = near_far.y; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	vec4 color = texture2D( TexColor , uv );
	float depth = texture2D( TexDepth , uv ).r;	
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);

    //Gamma correction
    color.r = pow(color.r,1.0/2.2);
    color.g = pow(color.g,1.0/2.2);
    color.b = pow(color.b,1.0/2.2);

	color_out = vec4(color.rgb,1.0);
}