SHADER_VERSION_STRING

#define NR_POINT_LIGHTS 4

#if defined(VERT_SHADER)
S(
	out vec3 FragPos;
	out vec3 Normal;

	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aNormal;

	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;

	void main()
	{
		gl_Position = projection * view * model * vec4(aPos, 1.0);
		FragPos = vec3(model * vec4(aPos, 1.0));
		Normal = mat3(transpose(inverse(model))) * aNormal;
	}
	);
#elif defined(FRAG_SHADER)
S(

	precision mediump float;

	out vec4 FragColor;

	struct Material {
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
	};

	struct DirLight {
		vec3 direction;

		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	};

	struct PointLight {
		vec3 position;

		float constant;
		float linear;
		float quadratic;

		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	};

	in vec3 FragPos;
	in vec3 Normal;

	uniform float alpha;
	uniform vec3 viewPos;
	uniform DirLight dirLight;
	uniform PointLight pointLights[NR_POINT_LIGHTS];
	uniform Material material;

	// function prototypes
	vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
	vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

	void main()
	{
		// properties
		vec3 norm = normalize(Normal);
		vec3 viewDir = normalize(viewPos - FragPos);

		// phase 1: Directional lighting
		vec3 result = CalcDirLight(dirLight, norm, viewDir);
		// phase 2: Point lights
		for(int i = 0; i < NR_POINT_LIGHTS; i++)
			result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
		// phase 3: Spot light
		//result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

		FragColor = vec4(result, alpha);
	}
	vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
	{
		vec3 lightDir = normalize(light.position - fragPos);
		// diffuse shading
		float diff = max(dot(normal, lightDir), 0.0);
		// specular shading
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		// attenuation
		float distance    = length(light.position - fragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance +
					   light.quadratic * (distance * distance));
		// combine results
		vec3 ambient  = light.ambient  * material.ambient;
		vec3 diffuse  = light.diffuse  * diff * material.diffuse;
		vec3 specular = light.specular * spec * material.specular;
		ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
		return (ambient + diffuse + specular);
	}

	vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
	{
		vec3 lightDir = normalize(-light.direction);
		// diffuse shading
		float diff = max(dot(normal, lightDir), 0.0);
		// specular shading
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		// combine results
		vec3 ambient  = light.ambient  * material.ambient;
		vec3 diffuse  = light.diffuse  * diff * material.diffuse;
		vec3 specular = light.specular * spec * material.specular;
		return (ambient + diffuse + specular);
	};
	);
#endif
#undef VERT_SHADER
#undef FRAG_SHADER
