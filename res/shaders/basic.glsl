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

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

#ShaderType Vertex

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConsants {
    mat4 modelMatrix;
    mat4 viewProjection;
    uint64_t vertexBufferAddress;
} pc;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() {
    VertexBuffer vb = VertexBuffer(pc.vertexBufferAddress);
    Vertex v = vb.vertices[gl_VertexIndex];

    gl_Position = pc.viewProjection * pc.modelMatrix * vec4(v.position, 1.0f);
    outNormal = v.normal;
    outUV = vec2(v.uv_x, v.uv_y);
}

#ShaderType Fragment

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

void main() {
    DirLight light;
    light = DirLight(vec3(0.0f, -1.0f, -1.0f), vec3(1.0f), 1.0f);

    outFragColor = light.intensity * vec4(inNormal, 1.0) * max(dot(inNormal, -normalize(light.direction)), 0.0f) + 0.1 * vec4(inNormal, 1.0);
}
