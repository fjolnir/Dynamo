// Just composites together the background layers

uniform sampler2D u_colormap0;
uniform sampler2D u_colormap1;
uniform sampler2D u_colormap2;
uniform sampler2D u_colormap3;

uniform lowp float u_layer0Opacity;
uniform lowp float u_layer1Opacity;
uniform lowp float u_layer2Opacity;
uniform lowp float u_layer3Opacity;

varying highp vec2 v_texCoord0;
varying highp vec2 v_texCoord1;
varying highp vec2 v_texCoord2;
varying highp vec2 v_texCoord3;

void main()
{
	mediump vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	mediump vec4 temp;

	temp = texture2D(u_colormap0, v_texCoord0);
	result = mix(result, temp, temp.a * u_layer0Opacity);
	
	temp = texture2D(u_colormap1, v_texCoord1);
	result = mix(result, temp, temp.a * u_layer1Opacity);

	temp = texture2D(u_colormap2, v_texCoord2);
	result = mix(result, temp, temp.a * u_layer2Opacity);

	temp = texture2D(u_colormap3, v_texCoord3);
	result = mix(result, temp, temp.a * u_layer3Opacity);

	gl_FragColor = result;
}
