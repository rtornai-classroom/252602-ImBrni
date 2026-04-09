#version 330 core
out vec4 outColor;
uniform vec3 baseColor;

void main() {
    outColor = vec4(baseColor, 1.0);
}
