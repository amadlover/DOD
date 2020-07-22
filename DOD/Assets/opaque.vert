#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform mat_ubo
{
    vec2 actor_position;
    vec2 actor_rotation;
} mat_buff;

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main ()
{
    gl_Position = vec4 (mat_buff.actor_position, 1, 1) * vec4 (in_position, 1, 1);

    out_color = in_color;
}
