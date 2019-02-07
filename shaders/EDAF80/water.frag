#version 410

uniform vec3 depth_color;        // Material ambient
uniform vec3 shallow_color;
uniform vec3 diffuse;        // Material diffuse
uniform vec3 specular;        // Material specular
uniform float shininess;
uniform vec3 light_position;
uniform samplerCube my_cube_map;
uniform sampler2D bump;
out vec4 fColor;


in VS_OUT { 
    vec3 vertex;                // View vector
    vec3 normal;                // Normal vector
    vec3 tangent;               //Tangent vector
    vec2 bumpcoo0;
    vec2 bumpcoo1;
    vec2 bumpcoo2;              // Texture Coordinates
    vec3 binormal;             // binormal vector
    vec3 light;                // Light vector
} fs_in;

void main() 
{ 
    vec4 bumpy0 = (texture(bump,fs_in.bumpcoo0)*2-1);
    vec4 bumpy1 = (texture(bump,fs_in.bumpcoo1)*2-1);
    vec4 bumpy2 = (texture(bump,fs_in.bumpcoo2)*2-1);

    vec3 tex = mat3(fs_in.binormal, fs_in.tangent, fs_in.normal) * normalize(bumpy0 + bumpy1 + bumpy2).xyz;
    vec3 N = normalize(tex); 
    vec3 L = normalize(fs_in.light); 
    vec3 V = normalize(fs_in.vertex); 
    vec3 R = normalize(reflect(-V,N)); 
    float facing = 1 - max(dot(V,N), 0);
    vec3 base_color = mix(depth_color, shallow_color, facing);
    vec4 reflection = texture(my_cube_map, R);
    float I = (base_color.x + base_color.y + base_color.z) / 3;
    vec4 refraction = texture(my_cube_map,refract(-V, N, 1.33));

    float R0 = 0.02037;
    float fresnel = R0 + (1 - R0) * pow((1 - dot(V, N)),5);

    fColor.xyz = base_color + fresnel*reflection.xyz + refraction.xyz * (1 - fresnel); 
    fColor.w = 1.0;
    //vec3 diff = diffuse*max(dot(N,L), 0.0);
    //vec3 spec = specular*pow(max(dot(R,V),0.0), shininess); 
}