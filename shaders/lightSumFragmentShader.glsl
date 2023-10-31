#version 420 core

//out vec4 FragColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

//NOTE(darius) for some weird reason this shit sometimes needs location specifyed, but sometimes brokes beacuse of it. I commented layouts for now, cause it only works this way.
//layout (location = 0) in vec2 TexCoords;
//layout (location = 1) in vec3 Normal;
//layout (location = 2) in vec3 FragPos;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;
in mat3 TBN;

struct Material {
    float shininess;
}; 

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform DirectionalLight dirLight;

#define NR_POINT_LIGHTS 21 
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int lightsCount;
uniform float gammaFactor;

uniform Material material;//NOTE(darius) diable foe now
uniform vec3 viewPos;
uniform bool shadowCaster;


layout(binding=0) uniform sampler2D texture_diffuse1;
layout(binding=1) uniform sampler2D texture_specular1;
layout(binding=2) uniform sampler2D texture_normal1;
layout(binding=3) uniform sampler2D texture_height1;

layout(binding=4) uniform sampler2D depthMap;

uniform float gammaBrightness;

vec3 calcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    //projCoords = projCoords * 0.1;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
    //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    float bias = 0.025;
    if(closestDepth + bias < currentDepth)
        return 0.95;
    return 0.8;
}  

void main()
{
    //vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    //FragColor.rgb = color;
    //return;

    //works
    //float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
    //return;

    //FragColor = vec4(1.0f,0.0f,1.0f,1.0f);//texture(texture_diffuse1, TexCoords);
    //return;

    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if(texColor.a < 0.1)
        discard;

    //vec3 norm = normalize(Normal);
    vec3 norm = texture(texture_specular1, TexCoords).rgb;
    norm = normalize(TBN * (norm * 2.0 - 1.0));

    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = calcDirectionalLight(dirLight, norm, viewDir);

    for(int i = 0; i < lightsCount; i++)
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);    


    float shadow = ShadowCalculation(FragPosLightSpace); 

    if(!shadowCaster)
        result = (1.0 - shadow) * result;
        
    if(lightsCount == 0)
        FragColor = texColor;
    else
        FragColor = gammaBrightness * texColor * vec4(result, 1.0f);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);


    //u can transfer depth map in r channel but cant in a channel
    //FragColor.r = depth;

    //gamma corecion
    //FragColor.rgb = pow((texture(texture_diffuse1, TexCoords) * vec4(result, 1.0f)).rgb, vec3(1.0/gammaFactor));
    //FragColor.rgb = gammaBrightness * (texture(texture_diffuse1, TexCoords)).rgb;
    //FragColor.a = 1.0;
}   

vec3 calcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));

    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    //lightDir = TBN * normalize(light.position - fragPos);
    //viewDir  = TBN * normalize(viewPos - fragPos);    

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
 
    //float intensity = 0.4 * spec;

 	//if (intensity > 0.9) 
 	//	intensity = 1.1;
 	//else if (intensity > 0.5) 
 	//	intensity = 0.7;
 	//else 
 	//	intensity = 0.5;


    return (ambient + diffuse + specular);// * intensity;
}
