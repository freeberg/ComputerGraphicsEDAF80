#version 410

uniform vec3 ambient;        // Material ambient
uniform vec3 diffuse;        // Material diffuse
uniform vec3 specular;        // Material specular
uniform float shininess;
uniform vec3 light_position;
out vec4 fColor;

in VS_OUT { 
    vec3  normal;                // Normal vector
    vec3  vertex;                // View vector
    vec3  light;                // Light vector
} fs_in;

void main() 
{ 
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(fs_in.light);
    vec3 V = normalize(fs_in.vertex);
    vec3 R = normalize(reflect(-L,N));
    
    vec3 diff = diffuse*max(dot(N,L), 0.0);
    vec3 spec = specular*pow(max(dot(R,V),0.0), shininess); 
    fColor.xyz = ambient + diff + spec; 
    fColor.w = 1.0;
}