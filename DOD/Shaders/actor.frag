#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D color_texture[4];
layout (push_constant) uniform image_index
{
    uint index;
} constants;

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_color;

void main ()
{
    out_color = texture (color_texture[constants.index], in_color.xy);
}    
