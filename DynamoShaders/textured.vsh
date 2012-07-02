// Simple texture mapping shader

uniform mat4 u_worldMatrix;
uniform mat4 u_projectionMatrix;

attribute vec4 a_position;
attribute vec2 a_texCoord0;

varying vec2 v_texCoord0;

void main()
{
    mat4 mvp = u_projectionMatrix * u_worldMatrix;
    gl_Position = mvp * a_position;
    v_texCoord0 = a_texCoord0;
}
