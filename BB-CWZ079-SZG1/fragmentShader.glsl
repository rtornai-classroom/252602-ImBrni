#version 330 core

in vec2 fragPosition;

uniform int objectType; // 0 = segment, 1 = circle
uniform vec3 segmentColor;
uniform vec2 circleCenter;
uniform float circleRadius;
uniform vec3 centerColor;
uniform vec3 edgeColor;

out vec4 fragmentColor;

void main() {
    if (objectType == 0) {
        fragmentColor = vec4(segmentColor, 1.0);
        return;
    }

    float dist = distance(fragPosition, circleCenter);
    if (dist > circleRadius) {
        discard;
    }

    float t = clamp(dist / circleRadius, 0.0, 1.0);
    vec3 color = mix(centerColor, edgeColor, t);
    fragmentColor = vec4(color, 1.0);
}
