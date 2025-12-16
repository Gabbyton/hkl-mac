SHADER_VERSION_STRING

#if defined(VERT_SHADER)
S(
	out vec3 oColors;

	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aColors;

	uniform mat4 projection;
	uniform mat4 view;

	void main()
	{
		gl_Position = projection * view * vec4(aPos, 1.0);
                oColors = aColors;
	}
	);
#elif defined(FRAG_SHADER)
S(
	precision mediump float;

	out vec4 FragColor;

        in vec3 oColors;

	void main()
	{
		FragColor = vec4(oColors, 1.0);
	}
	);
#endif
#undef VERT_SHADER
#undef FRAG_SHADER
