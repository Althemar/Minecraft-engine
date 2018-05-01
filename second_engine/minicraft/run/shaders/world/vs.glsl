#version 400

uniform mat4 mvp;
uniform mat4 nmat;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform float time;
uniform mat4 V_light;
uniform mat4 P_light;
uniform float elapsted;

layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec3 vs_normal_in;
layout(location=2) in vec2 vs_uv_in;
layout(location=3) in float vs_type_in;
layout(location=4) in float vs_type_left_in;
layout(location=5) in float vs_type_opposite_in;
layout(location=6) in float vs_type_right_in;

//Variables en sortie
out vec3 pos;
out vec3 normal;
out vec4 color;
out vec2 uv;
flat out int type;
out vec4 ShadowCoord;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0f
#define CUBE_PIERRE 2.0f
#define CUBE_EAU 3.0f
#define CUBE_BRANCHES 4.0f
#define CUBE_TRONC 5.0f
#define CUBE_SABLE 6.0f
#define CUBE_NUAGE 7.0f

const vec4 CubeColors[8]=vec4[8](
	vec4(0.1,0.8,0.2,1.0), //CUBE_HERBE
	vec4(0.2,0.1,0.0,1.0), //CUBE_TERRE
	vec4(0.7,0.7,0.7,1.0), //CUBE_PIERRE
	vec4(0.0,0.0,1.0,0.8), //CUBE_EAU
	vec4(0.3,0.6,0.3,1.0), //CUBE_BRANCHES
	vec4(0.2,0.1,0.0,1.0), //CUBE_TRONC
	vec4(0.7,0.7,0.0,1.0), //CUBE_SABLE
	vec4(1.0,1.0,1.0,1.0)  //CUBE_NUAGE
);

mat4 biasMatrix = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0
);

// Param for waves
const float waveLength = 2.f / 0.98f; 	// The length of a wave : 2 / L
const float waveAmplitude = 0.35;	// The amplitude of a wave : 
const float waveSpeed = 2.99f * waveLength; 	// The speed of a wave : Speed * waveLength
vec2 direction = vec2(0.f,1.0f);

// Param for gerstner waves

// Min is 0 (like cos waves), max in (1 / (waveLength * waveAmplitude)). Should be set between 0 and 1;
const float steepness = 0.7 ; 


void SetCircular(){
	direction = normalize(vec2(vs_position_in.xy) - vec2(0, 0));
}

vec4 SinusWave(vec4 position){
	float z = waveAmplitude * sin(dot(direction, position.xy) * waveLength + time * waveSpeed ) - waveAmplitude + position.z;
	position.z = z;
	return position;
}

vec4 GerstnerWave(vec4 position){
	if (vs_type_left_in == CUBE_EAU && vs_type_opposite_in == CUBE_EAU && vs_type_right_in == CUBE_EAU){
		position.x = steepness * waveAmplitude * direction.x * cos(waveLength * dot(direction, position.xy) +  time * waveSpeed ) + position.x;
		position.y = steepness * waveAmplitude * direction.y * cos(waveLength * dot(direction, position.xy) +  time * waveSpeed ) + position.y;
	}
	position.z = waveAmplitude * sin(dot(direction, position.xy) * waveLength + time * waveSpeed ) - waveAmplitude + position.z;
	return position;
}

void main()
{
	uv = vs_uv_in;
	normal = (nmat * vec4(vs_normal_in,1.0)).xyz; 
	
	type = int(vs_type_in);

	vec4 vecIn = vec4(vs_position_in,1.0);
	vecIn = m * vecIn;
	pos = vecIn.xyz;

	if (type == CUBE_EAU){
		//SetCircular();
		vecIn = GerstnerWave(vecIn);
		//vecIn = SinusWave(vecIn);
	}

	gl_Position = p * v * vecIn;

	mat4 depthMVP = P_light * V_light * m;
	mat4 biasDepthMVP = biasMatrix * depthMVP;
	ShadowCoord = biasDepthMVP * vec4(vs_position_in,1.0);

	color = CubeColors[int(vs_type_in)];	
}