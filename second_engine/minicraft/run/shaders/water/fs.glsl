#version 400

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;
flat in int type;

out vec4 color_out;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_PIERRE 2.0f
#define CUBE_EAU 3.0
#define CUBE_BRANCHES 4.0f
#define CUBE_TRONC 5.0f
#define CUBE_SABLE 6.0f
#define CUBE_NUAGE 7.0f

uniform sampler2D TexCustom_0;
uniform sampler2D TexCustom_1;
uniform sampler2D TexCustom_2;

void main()
{
	vec2 textCoord = uv;

	textCoord.x = (textCoord.x / 32) + (1.f/32.f) * type;
	textCoord.y = (textCoord.y / 2) + (0.5f * (1 - normal.z));
	//color_out = texture(TexCustom_0, textCoord);

/*
	if (type == CUBE_EAU){
		color_out = vec4(0.1f, 0.3f, 0.8f, 1);
	}
	else{
		textCoord.x = (textCoord.x / 32) + (1.f/32.f) * type;
		textCoord.y = (textCoord.y / 2) + (0.5f * (1 - normal.z));
		color_out = texture(TexCustom_0, textCoord);	
	}
*/
}