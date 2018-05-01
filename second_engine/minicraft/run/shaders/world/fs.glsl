#version 400

//Variables en entree
in vec3 pos;
in vec3 normal;
in vec4 color;
in vec2 uv;
flat in int type;
uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 camPos;

in vec4 ShadowCoord;


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
uniform sampler2D TexShadow;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

vec3 Shadow1(vec3 baseColor){
	float cosTheta = dot(normal, lightDir);
	cosTheta = clamp(cosTheta, 0.0, 1.0);

	//float delta = 1;
	float bias = 0.005*tan(acos(cosTheta)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = clamp(bias, 0,0.01);

	float visibility = 1.0;
	if ( texture( TexShadow, ShadowCoord.xy ).z  <  ShadowCoord.z - bias){
		visibility = 0.5;
	};

	for (int i=0;i<4;i++){
		if ( texture( TexShadow, ShadowCoord.xy + poissonDisk[i]/700.0 ).z  <  ShadowCoord.z-bias ){
			visibility-=0.05;
		}
	}

/*
	for (int i=0;i<4;i++){
         int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
        visibility -= 0.05*(1.0-texture( TexShadow, vec3(ShadowCoord.xy + poissonDisk[index]/700.0,  (ShadowCoord.z-bias)/ShadowCoord.w) ));
    }*/
	return baseColor * visibility;
}

float ShadowCalculation(){
	// perform perspective divide
    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(TexShadow, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(TexShadow, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(TexShadow, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

    return shadow;
}

vec3 Shadow2(vec3 baseColor){

    vec3 normal = normalize(normal);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.15 * baseColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - pos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(camPos - pos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

	float shadow = ShadowCalculation();
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor; 
	return lighting; 
}



void main()
{

	//vec3 p_color = color.rgb * (max(0,normal.z+normal.y/2)+0.2f);
	//color_out = vec4(p_color.rgb,color.a);



	vec2 textCoord = uv;
	

	textCoord.x = (textCoord.x / 32) + (1.f/32.f) * type;
	textCoord.y = (textCoord.y / 2) + (0.5f * (1 - normal.z));

	vec4 textureColor = texture(TexCustom_0, textCoord);

	vec3 newColor = textureColor.rgb * (max(0,normal.z+normal.y/2)+0.2f);
	
	newColor = Shadow1(newColor);
	//newColor = Shadow2(newColor);

	color_out = vec4(newColor, textureColor.a);
}