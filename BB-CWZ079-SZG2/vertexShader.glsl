#version 330 core
layout(location = 0) in vec2 aPosition;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

void main() {
    gl_Position = matProjection * matView * matModel * vec4(aPosition, 0.0, 1.0);
}
