uniform sampler2D u_colormap0;
uniform mediump vec4 u_color;

varying highp vec2 v_texCoord0;

void main()
{
	gl_FragColor = u_color * texture2D(u_colormap0, v_texCoord0);
}
