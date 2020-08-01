#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform mat_ubo
{
    vec2 actor_position;
    float actor_rotation;
} mat_buff;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main ()
{
    vec3 pos = in_position;
    
    pos.x = (pos.x * cos (mat_buff.actor_rotation)) - (pos.y * sin (mat_buff.actor_rotation));
    pos.y = (pos.y * cos (mat_buff.actor_rotation)) + (pos.x * sin (mat_buff.actor_rotation));
    
    //pos += vec3 (mat_buff.actor_position, 0);

    gl_Position = vec4 (pos, 1);

    out_color = in_color;
}
