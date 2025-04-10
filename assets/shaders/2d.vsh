#version 430 core
precision mediump float;

struct Vertex2D {
    vec3 position;
    vec4 color;
    vec2 texcoords;
    uint texture_id;
};

layout(std430, binding = 0)
buffer VertexBuffer
{
    Vertex2D vertices[];
};

out vec4 out_color;
out vec2 out_texcoord;

void main() {
    gl_Position = vec4(vertices[gl_VertexID].position, 1.0);
    gl_Position.xy -= 1.0;
    gl_Position.y *= -1;
    out_color = vertices[gl_VertexID].color;
    out_texcoord = vertices[gl_VertexID].texcoords;
}
