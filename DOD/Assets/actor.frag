#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_color;

void main ()
{
    out_color = vec2 (gl_FragCoords.xy, 1, 0);
}    
