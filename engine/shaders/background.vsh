// Computes the texture offsets for each layer

uniform mat4 u_worldMatrix; // Ignored
uniform mat4 u_projectionMatrix; // Ignored
uniform vec2 u_backgroundSize;
uniform vec2 u_backgroundOffset;
// Depth is normalized between -1.0&1.0 (Positive direction being towards the viewer)
uniform float u_layer0Depth;
uniform float u_layer1Depth;
uniform float u_layer2Depth;
uniform float u_layer3Depth;

attribute vec4 a_position;
attribute vec2 a_texCoord0;

varying vec2 v_texCoord0;
varying vec2 v_texCoord1;
varying vec2 v_texCoord2;
varying vec2 v_texCoord3;


void main()
{
	gl_Position = a_position; // We don't do any transformation

	v_texCoord0 = a_texCoord0 + (1.0+u_layer0Depth) * u_backgroundOffset;
	v_texCoord1 = a_texCoord0 + (1.0+u_layer1Depth) * u_backgroundOffset;
	v_texCoord2 = a_texCoord0 + (1.0+u_layer2Depth) * u_backgroundOffset;
	v_texCoord3 = a_texCoord0 + (1.0+u_layer3Depth) * u_backgroundOffset;

}
