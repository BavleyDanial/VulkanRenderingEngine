#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConsants {
    uint64_t vertexBufferAddress;
} pc;

#ShaderType Vertex

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() {
    VertexBuffer vb = VertexBuffer(pc.vertexBufferAddress);
    Vertex v = vb.vertices[gl_VertexIndex];

    gl_Position = vec4(v.position, 1.0f);
    outNormal = v.normal;
    outUV = vec2(v.uv_x, v.uv_y);
}

#ShaderType Fragment

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

void main() {
    outFragColor = vec4(inNormal * 0.5 + 0.5, 1.0f);
}
