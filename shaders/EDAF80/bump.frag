#version 410

uniform vec3 diffuse;        // Material diffuse
uniform vec3 specular;        // Material specular
uniform float shininess;
uniform vec3 light_position;

uniform sampler2D my_diffuse_map;
uniform sampler2D my_bump_map;
uniform mat4 normal_model_to_world;        // Inverse transpose



out vec4 fColor;

in VS_OUT { 
    vec3 vertex;                // View vector
    vec3 normal;                // Normal vector
    vec3 tangent;               //Tangent vector
    vec3 texcoo;                // Texture Coordinates
    vec3 binormal;             // binormal vector
    vec3 light;                // Light vector
} fs_in;

void main() 
{ 
    vec4 texrange = vec4(texture(my_bump_map, fs_in.texcoo.xy)) * 2 - vec4(1);
    mat4 TBN = mat4(vec4(fs_in.tangent,0), vec4(fs_in.binormal,0), vec4(fs_in.normal,0), vec4(0,0,0,1));
    vec3 normal2 = (normal_model_to_world*TBN*texrange).xyz;
    vec3 N = normalize(normal2); 
    vec3 L = normalize(fs_in.light); 
    vec3 V = normalize(fs_in.vertex); 
    vec3 R = normalize(reflect(-L,N)); 

    vec3 diff = diffuse*max(dot(N,L), 0.0);
    vec3 spec = specular*pow(max(dot(R,V),0.0), shininess); 
    fColor.xyz = texture(my_diffuse_map, fs_in.texcoo.xy).xyz + diff + spec; 
    fColor.w = 1.0;
}