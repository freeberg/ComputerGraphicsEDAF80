#version 410

layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoo; 
// layout (location = 3) in vec3 tangent;
// layout (location = 4) in vec3 binormal; 

uniform mat4 vertex_world_to_clip;  // Model -> Clip space
uniform mat4 vertex_model_to_world;          // Model -> World space
uniform mat4 normal_model_to_world;        // Inverse transpose
uniform vec3 light_position;       // Defined in world space 
uniform vec3 camera_position;         // Defined in world space
uniform float u_time;

out VS_OUT { 
    vec3 vertex;                // View vector
    vec3 normal;                // Normal vector
    vec3 tangent;               //Tangent vector
    vec2 bumpcoo0;
    vec2 bumpcoo1;
    vec2 bumpcoo2;                // Texture Coordinates
    vec3 binormal;             // binormal vector
    vec3 light;                // Light vector
} vs_out; 

void main() 
{   
    float amp = 1.8;
    float freq = 0.2;
    float pha = 0.5;
    float k = 2.0;
    float dirx = -1.0;
    float dirz = 0.0;

    float wave1 = amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k);
    float dyx1 = 0.5 * k * freq * amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k - 1) * cos((dirx * 
                         vertex.x + dirz * vertex.z) * freq + u_time * pha)*dirx;
    float dyz1 = 0.5 * k * freq * amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k - 1) * cos((dirx * 
                         vertex.x + dirz * vertex.z) * freq + u_time * pha)*dirz;

    amp = 1.5;
    freq = 0.4;
    pha = 1.3;
    k = 2.0;
    dirx = -0.7;
    dirz = 0.7;
    float wave2 = amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k);
    float dyx2 = 0.5 * k * freq * amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k - 1) * cos((dirx * 
                         vertex.x + dirz * vertex.z) * freq + u_time * pha)*dirx;
    float dyz2 = 0.5 * k * freq * amp * pow((sin((dirx * vertex.x + dirz * vertex.z) *
                         freq + u_time * pha) * 0.5 + 0.5), k - 1) * cos((dirx * 
                         vertex.x + dirz * vertex.z) * freq + u_time * pha)*dirz;
    
    float y = vertex.y + wave1 + wave2;
    float dyx = dyx1 + dyx2;
    float dyz = dyz1 + dyz2;
    vs_out.binormal = normalize(vec3(1.0, dyx, 0.0));
    vs_out.tangent = normalize(vec3(0.0, dyz, 1.0));
    vec3 new_normal = normalize(vec3(-dyx,1.0,-dyz));

    vec2 texScale = vec2(8.0, 4.0); 
    float bumpTime = mod(u_time, 100.0);
    vec2 bumpSpeed = vec2(-0.05, 0.0);

    vs_out.bumpcoo0 = texcoo.xy*texScale + bumpTime*bumpSpeed;
    vs_out.bumpcoo1 = texcoo.xy*texScale*2 + bumpTime*bumpSpeed*4;
    vs_out.bumpcoo2 = texcoo.xy*texScale*4 + bumpTime*bumpSpeed*8;

    vec3 worldPos = (vertex_model_to_world*vec4(vertex.x, y, vertex.z,1)).xyz; 
    vs_out.normal = (normal_model_to_world*vec4(new_normal,0)).xyz; 
    vs_out.vertex = camera_position  - worldPos; 
    vs_out.light = light_position - worldPos; 
    vec3 vertex_new = vec3(vertex.x, y, vertex.z);
    gl_Position = vertex_world_to_clip*vec4(vertex_new,1); 
}