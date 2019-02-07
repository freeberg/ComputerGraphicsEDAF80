#version 410

uniform samplerCube my_cube_map;

out vec4 fColor;

in VS_OUT { 
    vec3 normal;               // texture coordinates
} fs_in;

void main() 
{ 
    fColor = texture(my_cube_map, fs_in.normal);
}