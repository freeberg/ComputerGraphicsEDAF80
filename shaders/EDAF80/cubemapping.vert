#version 410

layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec3 normal;

uniform mat4 vertex_world_to_clip;          // Model -> Clip Space

out VS_OUT {
    vec3  normal;               // texture coordinates
} vs_out; 

void main() 
{ 
    vs_out.normal = normal; 
    gl_Position = vertex_world_to_clip * vec4(vertex, 1);
}