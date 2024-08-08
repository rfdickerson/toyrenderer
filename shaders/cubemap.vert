#version 450

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) out vec3 outTexCoords;

// The vertices of a unit cube
const vec3 positions[8] = vec3[8](
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0)
);

// Indices for drawing a cube
const int indices[36] = int[36](
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    5, 4, 7, 7, 6, 5,
    4, 0, 3, 3, 7, 4,
    3, 2, 6, 6, 7, 3,
    4, 5, 1, 1, 0, 4
);

void main() {
    vec3 pos = positions[indices[gl_VertexIndex]];

    // Remove translation from the view matrix
    mat4 rotView = mat4(mat3(camera.view));

    vec4 clipPos = camera.proj * rotView * vec4(pos, 1.0);

    // Ensure the cubemap is always rendered at maximum depth
    gl_Position = clipPos.xyww;

    outTexCoords = pos;
    outTexCoords = vec3(pos.x, pos.y, pos.z);
}