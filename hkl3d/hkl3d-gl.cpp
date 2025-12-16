/* This file is part of the hkl3d library.
 *
 * The hkl library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The hkl library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the hkl library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2010, 2025      Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 *          Oussama Sboui <oussama.sboui@synchrotron-soleil.fr>
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <assimp/scene.h>
#include <libgen.h>
#include <yaml.h>

#include "hkl3d-private.h"
#include "hkl-geometry-private.h"


/**********/
/* Shader */
/**********/

static void set_uniform_mat4s (Shader *s, const char *name, mat4s m)
{
	GLuint loc = glGetUniformLocation (s->program, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, m.raw[0]);
}

static void set_uniform_vec3sv (Shader *s, const char *name, vec3s v)
{
	GLuint loc = glGetUniformLocation (s->program, name);
	glUniform3fv(loc, 1, v.raw);
}

static void set_uniform_make_vec3s (Shader *s, const char *name, float x, float y, float z)
{
	CGLM_ALIGN_MAT vec3s v = {{x, y, z}};
	set_uniform_vec3sv(s, name, v);
}

static void set_uniform_float(Shader *s, const char *name, float x)
{
	GLuint loc = glGetUniformLocation (s->program, name);
	glUniform1f(loc, x);
}

static Shader init_shader()
{
	GLuint p;

	const char* vertex_shader =
		"#version 300 es\n"
		"\n"
		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aNormal;\n"
		"\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"
		"\n"
		"void main() {\n"
		"  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"  FragPos = vec3(model * vec4(aPos, 1.0));\n"
		"  Normal = mat3(transpose(inverse(model))) * aNormal;\n"
		"}\n";

	const char* fragment_shader =
		"#version 300 es\n"
		"precision mediump float;\n"
		"\n"
		"out vec4 FragColor;\n"
		"\n"
		"struct Material {\n"
		"  vec3 ambient;\n"
		"  vec3 diffuse;\n"
		"  vec3 specular;\n"
		"  float shininess;\n"
		"};\n"
		"\n"
		"struct DirLight {\n"
		"    vec3 direction;\n"
		"\n"
		"    vec3 ambient;\n"
		"    vec3 diffuse;\n"
		"    vec3 specular;\n"
		"};\n"
		"\n"
		"struct PointLight {\n"
		"    vec3 position;\n"
		"\n"
		"    float constant;\n"
		"    float linear;\n"
		"    float quadratic;\n"
		"\n"
		"    vec3 ambient;\n"
		"    vec3 diffuse;\n"
		"    vec3 specular;\n"
		"};\n"
		"#define NR_POINT_LIGHTS 4\n"
		"\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"\n"
		"uniform float alpha;\n"
		"uniform vec3 viewPos;\n"
		"uniform DirLight dirLight;\n"
		"uniform PointLight pointLights[NR_POINT_LIGHTS];\n"
		"uniform Material material;\n"
		"\n"
		"// function prototypes\n"
		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);\n"
		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"
		"\n"
		"void main()\n"
		"{\n"
		"    // properties\n"
		"    vec3 norm = normalize(Normal);\n"
		"    vec3 viewDir = normalize(viewPos - FragPos);\n"
		"\n"
		"    // phase 1: Directional lighting\n"
		"    vec3 result = CalcDirLight(dirLight, norm, viewDir);\n"
		"    // phase 2: Point lights\n"
		"    for(int i = 0; i < NR_POINT_LIGHTS; i++)\n"
		"        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
		"    // phase 3: Spot light\n"
		"    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);\n"
		"\n"
		"    FragColor = vec4(result, alpha);\n"
		"}\n"
		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
		"{\n"
		"    vec3 lightDir = normalize(light.position - fragPos);\n"
		"    // diffuse shading\n"
		"    float diff = max(dot(normal, lightDir), 0.0);\n"
		"    // specular shading\n"
		"    vec3 reflectDir = reflect(-lightDir, normal);\n"
		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		"    // attenuation\n"
		"    float distance    = length(light.position - fragPos);\n"
		"    float attenuation = 1.0 / (light.constant + light.linear * distance +\n"
		"  			     light.quadratic * (distance * distance));\n"
		"    // combine results\n"
		"    vec3 ambient  = light.ambient  * material.ambient;\n"
		"    vec3 diffuse  = light.diffuse  * diff * material.diffuse;\n"
		"    vec3 specular = light.specular * spec * material.specular;\n"
		"    ambient  *= attenuation;\n"
		"    diffuse  *= attenuation;\n"
		"    specular *= attenuation;\n"
		"    return (ambient + diffuse + specular);\n"
		"}\n"
		"\n"
		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)\n"
		"{\n"
		"    vec3 lightDir = normalize(-light.direction);\n"
		"    // diffuse shading\n"
		"    float diff = max(dot(normal, lightDir), 0.0);\n"
		"    // specular shading\n"
		"    vec3 reflectDir = reflect(-lightDir, normal);\n"
		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		"    // combine results\n"
		"    vec3 ambient  = light.ambient  * material.ambient;\n"
		"    vec3 diffuse  = light.diffuse  * diff * material.diffuse;\n"
		"    vec3 specular = light.specular * spec * material.specular;\n"
		"    return (ambient + diffuse + specular);\n"
		"}\n";

	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL );
	glCompileShader (vs);

	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL );
	glCompileShader (fs);

	p = glCreateProgram();
	glAttachShader (p, vs);
	glAttachShader (p, fs);
	glLinkProgram (p);

	Shader shader = {
		.program = p
	};

	glDeleteShader (fs);
	glDeleteShader (vs);

	glUseProgram (p);

	/* projection */
	CGLM_ALIGN_MAT mat4s projection = GLMS_MAT4_IDENTITY_INIT;
	/* projection = glms_perspective (glm_rad(45), 1, 0.1, 100); */
	set_uniform_mat4s(&shader, "projection", projection);

	/* view position */
	CGLM_ALIGN_MAT vec3s viewPos = {{0, 5, 0}};
	set_uniform_vec3sv(&shader, "viewPos", viewPos);

	/* view */
	CGLM_ALIGN_MAT mat4s view = GLMS_MAT4_IDENTITY_INIT;
	CGLM_ALIGN_MAT vec3s center = GLMS_VEC3_ZERO_INIT;
	CGLM_ALIGN_MAT vec3s up = {{0, 0, 1}};
	view = glms_lookat(viewPos, center, up);
	set_uniform_mat4s(&shader, "view", view);

	/* default model */
	CGLM_ALIGN_MAT mat4s model = GLMS_MAT4_IDENTITY_INIT;
	set_uniform_mat4s(&shader, "model", model);

	/* dir light */
	set_uniform_make_vec3s(&shader, "dirLight.direction", -0.2, -1.0, -0.3);
        set_uniform_make_vec3s(&shader, "dirLight.ambient", 0.05, 0.05, 0.05);
        set_uniform_make_vec3s(&shader, "dirLight.diffuse", 0.4, 0.4, 0.4);
        set_uniform_make_vec3s(&shader, "dirLight.specular", 0.5, 0.5, 0.5);

        /* point light 0 */
        set_uniform_make_vec3s(&shader, "pointLights[0].position", 5, 5, 0);
        set_uniform_make_vec3s(&shader, "pointLights[0].ambient", 0.05, 0.05, 0.05);
        set_uniform_make_vec3s(&shader, "pointLights[0].diffuse", 0.8, 0.8, 0.8);
        set_uniform_make_vec3s(&shader, "pointLights[0].specular", 1.0, 1.0, 1.0);
        set_uniform_float(&shader, "pointLights[0].constant", 1.0);
        set_uniform_float(&shader, "pointLights[0].linear", 0.09);
        set_uniform_float(&shader, "pointLights[0].quadratic", 0.032);

        /* point light 1 */
        set_uniform_make_vec3s(&shader, "pointLights[1].position", -5, 5, 0);
        set_uniform_make_vec3s(&shader, "pointLights[1].ambient", 0.05, 0.05, 0.05);
        set_uniform_make_vec3s(&shader, "pointLights[1].diffuse", 0.8, 0.8, 0.8);
        set_uniform_make_vec3s(&shader, "pointLights[1].specular", 1.0, 1.0, 1.0);
        set_uniform_float(&shader, "pointLights[1].constant", 1.0);
        set_uniform_float(&shader, "pointLights[1].linear", 0.09);
        set_uniform_float(&shader, "pointLights[1].quadratic", 0.032);

        /* point light 2 */
        set_uniform_make_vec3s(&shader, "pointLights[2].position", 5, -5, 0);
        set_uniform_make_vec3s(&shader, "pointLights[2].ambient", 0.05, 0.05, 0.05);
        set_uniform_make_vec3s(&shader, "pointLights[2].diffuse", 0.8, 0.8, 0.8);
        set_uniform_make_vec3s(&shader, "pointLights[2].specular", 1.0, 1.0, 1.0);
        set_uniform_float(&shader, "pointLights[2].constant", 1.0);
        set_uniform_float(&shader, "pointLights[2].linear", 0.09);
        set_uniform_float(&shader, "pointLights[2].quadratic", 0.032);

        /* point light 3 */
        set_uniform_make_vec3s(&shader, "pointLights[3].position", -5, -5, 0);
        set_uniform_make_vec3s(&shader, "pointLights[3].ambient", 0.05, 0.05, 0.05);
        set_uniform_make_vec3s(&shader, "pointLights[3].diffuse", 0.8, 0.8, 0.8);
        set_uniform_make_vec3s(&shader, "pointLights[3].specular", 1.0, 1.0, 1.0);
        set_uniform_float(&shader, "pointLights[3].constant", 1.0);
        set_uniform_float(&shader, "pointLights[3].linear", 0.09);
        set_uniform_float(&shader, "pointLights[3].quadratic", 0.032);

	glUseProgram (0);

	return shader;
}


/***********************/
/* Hkl3D OpenGL model  */
/***********************/

void hkl3d_gl_draw_aabb_set(Hkl3D *self, bool aabb)
{
	Hkl3DBulletObject **bobject;

	darray_foreach(bobject, self->bullet->bobjects){
		hkl3d_bullet_object_draw_aabb_set(*bobject, aabb);
	}
}

static void hkl3d_gl_draw_object(const Hkl3DObject *object, Shader *shader)
{
	static unsigned int n_vec3 = 3;
	const struct aiScene *scene = object->model->scene;
	const struct aiMesh *mesh = scene->mMeshes[object->mesh];
	CGLM_ALIGN_MAT vec3s ambient = GLM_VEC3_ZERO_INIT;
	CGLM_ALIGN_MAT vec3s diffuse = GLM_VEC3_ZERO_INIT;
	CGLM_ALIGN_MAT vec3s specular = GLM_VEC3_ZERO_INIT;
	GLfloat shininess = 12;
	GLfloat alpha = object->is_colliding == 0 ? 1.0 : 0.5;;

	const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
	aiGetMaterialFloatArray(material, AI_MATKEY_COLOR_AMBIENT, ambient.raw, &n_vec3);
	aiGetMaterialFloatArray(material, AI_MATKEY_COLOR_DIFFUSE, diffuse.raw, &n_vec3);
	aiGetMaterialFloatArray(material, AI_MATKEY_COLOR_SPECULAR, specular.raw, &n_vec3);
	aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &shininess);

	/* set the uniforms */
	set_uniform_mat4s(shader, "model", object->transformation);
	set_uniform_vec3sv(shader, "material.ambient", ambient);
	set_uniform_vec3sv(shader, "material.diffuse", diffuse);
	set_uniform_vec3sv(shader, "material.specular", specular);
	set_uniform_float(shader, "material.shininess", shininess);
	set_uniform_float(shader, "alpha", alpha);

	glBindVertexArray (object->vao);
	glDrawElements (GL_TRIANGLES, mesh->mNumFaces * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray (0);
}

static void hkl3d_gl_draw_models(Hkl3D *self)
{
	Hkl3DModel **model;
	Hkl3DObject **object;

	/* remove all objects from the collision world */
	darray_foreach(model, self->config->models){
		darray_foreach(object, (*model)->objects){
			hkl3d_gl_draw_object(*object, &self->shader);
		}
	}
}

static void hkl3d_gl_draw_debug (Hkl3D *self)
{
	CGLM_ALIGN_MAT mat4s identity = GLMS_MAT4_IDENTITY_INIT;

	set_uniform_mat4s(&self->shader, "model", identity);
	hkl3d_bullet_gl_draw_debug (self->bullet);
}

void hkl3d_gl_draw (Hkl3D *self)
{
	glUseProgram (self->shader.program);

	hkl3d_gl_draw_models (self);

	hkl3d_gl_draw_debug (self);

	glUseProgram (0);
}

void hkl3d_gl_resize(Hkl3D *self, gint width, gint height)
{
	glUseProgram (self->shader.program);

	glViewport(0, 0, width, height);

	/* TODO store the lovation in the class */
	CGLM_ALIGN_MAT mat4s projection = GLMS_MAT4_IDENTITY_INIT;
	projection = glms_perspective (glm_rad(45), (GLfloat)width / (GLfloat)height, 0.1, 100);
	glUniformMatrix4fv(glGetUniformLocation (self->shader.program, "projection"),
			   1, GL_FALSE, projection.raw[0]);

	glUseProgram (0);
}


void hkl3d_gl_init(Hkl3D *self)
{
	Hkl3DModel **model;
	Hkl3DObject **object;

	self->shader = init_shader();

	/* initialize all OpenGL buffers */
	darray_foreach(model, self->config->models){
		darray_foreach(object, (*model)->objects){

			/* skip already initialized buffer */
			if ((*object)->vao) continue;

			int k;
			const struct aiScene *scene = (*model)->scene;
			const struct aiMesh *mesh = scene->mMeshes[(*object)->mesh];

			GLuint vao = 0;
			glGenVertexArrays (1, &vao);
			glBindVertexArray (vao);

			/* positions */
			GLuint vbo = 0;
			size_t size_vertex = mesh->mNumVertices * 3 * sizeof (float);
			GLfloat *vertices = g_new(GLfloat,  mesh->mNumVertices * 3);
			for(k=0; k<mesh->mNumVertices; ++k){
				vertices[k * 3 + 0] = mesh->mVertices[k].x;
				vertices[k * 3 + 1] = mesh->mVertices[k].y;
				vertices[k * 3 + 2] = mesh->mVertices[k].z;
			}
			glGenBuffers (1, &vbo );
			glBindBuffer (GL_ARRAY_BUFFER, vbo );
			glBufferData (GL_ARRAY_BUFFER, size_vertex, vertices, GL_STATIC_DRAW);

			glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), NULL);
			glEnableVertexAttribArray (0);

			/* normals */
			GLuint vbo_normals = 0;
			size_t size_normals = mesh->mNumVertices * 3 * sizeof (float);
			glGenBuffers (1, &vbo_normals );
			glBindBuffer (GL_ARRAY_BUFFER, vbo_normals );
			glBufferData (GL_ARRAY_BUFFER, size_normals, mesh->mNormals, GL_STATIC_DRAW);

			glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), NULL);
			glEnableVertexAttribArray (1);

			/* indices */
			GLuint ebo = 0;
			size_t size_indices = mesh->mNumFaces * 3 * sizeof(unsigned int);
			unsigned int *indices = g_new(unsigned int, mesh->mNumFaces * 3);
			unsigned int idx=0;
			for(k=0; k<mesh->mNumFaces; ++k){
				const struct aiFace face = mesh->mFaces[k];

				indices[idx++] = face.mIndices[0];
				indices[idx++] = face.mIndices[1];
				indices[idx++] = face.mIndices[2];
			}
			glGenBuffers (1, &ebo);
			glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData (GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_STATIC_DRAW);

			(*object)->vao = vao;
		}
	}
}
