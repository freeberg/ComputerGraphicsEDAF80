#version 410

layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec3 normal;
// layout (location = 2) in vec3 texcoo; 
// layout (location = 3) in vec3 tangent;
// layout (location = 4) in vec3 binormal; 

uniform mat4 vertex_world_to_clip;  // Model -> Clip space
uniform mat4 vertex_model_to_world;          // Model -> World space
uniform mat4 normal_model_to_world;        // Inverse transpose
uniform vec3 light_position;       // Defined in world space 
uniform vec3 camera_position;         // Defined in world space

out VS_OUT { 
    vec3  normal;                // Normal vector
    vec3  vertex;                // View vector
    vec3  light;                // Light vector
} vs_out; 

void main() 
{ 
    vec3 worldPos = (vertex_model_to_world*vec4(vertex,1)).xyz; 
    vs_out.normal = (normal_model_to_world*vec4(normal,0)).xyz; 
    vs_out.vertex = camera_position   - worldPos; 
    vs_out.light = light_position - worldPos; 

    gl_Position = vertex_world_to_clip*vec4(vertex,1); 
}