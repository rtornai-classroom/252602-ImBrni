#version 330 core

layout(location = 0) in vec2 position;
out vec2 vsPosition;

void main() {
    vsPosition = position;
}
