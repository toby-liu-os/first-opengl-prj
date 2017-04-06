#version 100

attribute vec3 position;
attribute vec3 color;

uniform mat4 MVPMatrix;

varying vec4 fragColor;

void main(void)
{
    fragColor = vec4(color, 1.0);
    gl_Position = MVPMatrix * vec4(position, 1.0);
}