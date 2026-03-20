#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 matProjection;
uniform mat4 matModelView;
uniform int objectType; // 0 = segment, 1 = circle
uniform float circleRadius;
uniform float segmentLength;
uniform float segmentHeight;

in vec2 vsPosition[];
out vec2 fragPosition;

void emitVertex(vec2 p) {
    fragPosition = p;
    gl_Position = matProjection * matModelView * vec4(p, 0.0, 1.0);
    EmitVertex();
}

void main() {
    vec2 center = vsPosition[0];

    if (objectType == 0) {
        float halfLen = segmentLength * 0.5;
        float halfThick = segmentHeight * 0.5;

        emitVertex(vec2(center.x - halfLen, center.y - halfThick));
        emitVertex(vec2(center.x + halfLen, center.y - halfThick));
        emitVertex(vec2(center.x - halfLen, center.y + halfThick));
        emitVertex(vec2(center.x + halfLen, center.y + halfThick));
        EndPrimitive();
        return;
    }

    emitVertex(vec2(center.x - circleRadius, center.y - circleRadius));
    emitVertex(vec2(center.x + circleRadius, center.y - circleRadius));
    emitVertex(vec2(center.x - circleRadius, center.y + circleRadius));
    emitVertex(vec2(center.x + circleRadius, center.y + circleRadius));
    EndPrimitive();
}
