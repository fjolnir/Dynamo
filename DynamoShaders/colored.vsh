uniform mat4 u_worldMatrix;
uniform mat4 u_projectionMatrix;

attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    mat4 mvp = u_projectionMatrix * u_worldMatrix;
    gl_Position = mvp * a_position;

    v_color = a_color;
}
