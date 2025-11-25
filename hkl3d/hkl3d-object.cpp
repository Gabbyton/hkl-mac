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

#include "hkl3d-private.h"

#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"


void ai_mesh_fprintf(FILE *f, const struct aiMesh *mesh, int i)
{
	fprintf(f, "\n meshe %d \"%s\" (%p) (type %d) (%d vertices) (%d faces) (normales %p)",
		i,
		mesh->mName.data,
		mesh,
		mesh->mPrimitiveTypes,
		mesh->mNumVertices,
		mesh->mNumFaces,
		mesh->mNormals);
}

void ai_node_fprintf(FILE *f, const struct aiNode *node, int level)
{
	unsigned int i;

	fprintf(f, "\n%*s - (node) %s (%d meshes) (%d child)",
		level, "",
		node->mName.data,
		node->mNumMeshes,
		node->mNumChildren);
	fprintf(f, "\n%*s   transformation: %f %f %f %f",
		level, "",
		node->mTransformation.a1,
		node->mTransformation.a2,
		node->mTransformation.a3,
		node->mTransformation.a4);
	fprintf(f, "\n%*s                   %f %f %f %f",
		level, "",
		node->mTransformation.b1,
		node->mTransformation.b2,
		node->mTransformation.b3,
		node->mTransformation.b4);
	fprintf(f, "\n%*s                   %f %f %f %f",
		level, "",
		node->mTransformation.c1,
		node->mTransformation.c2,
		node->mTransformation.c3,
		node->mTransformation.c4);
	fprintf(f, "\n%*s                   %f %f %f %f",
		level, "",
		node->mTransformation.d1,
		node->mTransformation.d2,
		node->mTransformation.d3,
		node->mTransformation.d4);

	for(i=0; i<node->mNumMeshes; ++i){
		fprintf(f, "\n%*s   meshes %d",
			level, "",
			node->mMeshes[i]);
	}

	for(i=0; i<node->mNumChildren; ++i){
		ai_node_fprintf(f, node->mChildren[i], level+2);
	}
}

void ai_scene_fprintf(FILE *f, const struct aiScene *scene)
{
	unsigned int i;

	fprintf(f, "\nscene: (%p)", scene);
	fprintf(f, "\n%d animations", scene->mNumAnimations);
	fprintf(f, "\n%d cameras", scene->mNumCameras);
	fprintf(f, "\n%d lights", scene->mNumLights);
	fprintf(f, "\n%d materials", scene->mNumMaterials);
	fprintf(f, "\n%d meshes", scene->mNumMeshes);
	fprintf(f, "\n%d skeletons", scene->mNumSkeletons);
	fprintf(f, "\n%d textures", scene->mNumTextures);

	/* nodes */
	ai_node_fprintf(f, scene->mRootNode, 0);

	/* models */
	for(i=0; i<scene->mNumMeshes; ++i){
		const struct aiMesh *mesh = scene->mMeshes[i];
		ai_mesh_fprintf(f, mesh, i);
	}
}

/***************/
/* Hkl3DObject */
/***************/

Hkl3DObject *hkl3d_object_new(Hkl3DModel *model, unsigned int mesh)
{
	int i;
	GSList *faces;
	Hkl3DObject *self = nullptr;

	self = g_new0 (Hkl3DObject, 1);

	/* fill the hkl3d object structure. */
	self->added = false;
	self->draw_aabb = false;
	self->hide = false;
	self->is_colliding = false;
	self->mesh = mesh;
	self->model = model;
	self->movable = false;
	self->selected = false;
	self->transformation = GLMS_MAT4_IDENTITY; /* todo importe from the node */

	/* bullet */
	self->bullet = hkl3d_bullet_object_new(model->scene->mMeshes[self->mesh]);

	/* OpenGL */
	self->vao = 0;

	return self;
}

void hkl3d_object_free(Hkl3DObject *self)
{
	if(!self)
		return;

	hkl3d_bullet_object_free (self->bullet);

	free(self);
}

static void matrix_fprintf(FILE *f, const float matrix[])
{
	for(uint i=0; i<4; ++i){
		fprintf(f, "\n ");
		for(uint j=0; j<4; ++j){
			fprintf(f, " %6.3f", matrix[4 * i + j]);
		}
	}
}

/* TODO move this elsewhere */
void hkl3d_object_aabb_get(const Hkl3DObject *self, float from[3], float to[3])
{
	btVector3 min, max;

	self->bullet->btShape->getAabb(self->bullet->btObject->getWorldTransform(), min, max);

	from[0] = min.getX();
	from[1] = min.getY();
	from[2] = min.getZ();
	to[0] = max.getX();
	to[1] = max.getY();
	to[2] = max.getZ();
}

void hkl3d_object_draw_aabb_set(Hkl3DObject *self, bool draw_aabb)
{
	self->draw_aabb = draw_aabb;
}

void hkl3d_object_transformation_set(Hkl3DObject *self, mat4s transformation)
{
	g_return_if_fail (NULL != self);

	self->transformation = transformation;
}

bool hkl3d_object_hide_get(const Hkl3DObject *self)
{
	return self->hide;
}

void hkl3d_object_hide_set(Hkl3DObject *self, bool hide)
{
	g_return_if_fail (nullptr != self);

	self->hide = hide;
}

void hkl3d_object_fprintf(FILE *f, const Hkl3DObject *self)
{
	GSList *faces;

	fprintf(f, "Hkl3dObject (", self);
	fprintf(f, "model=%p, ", self->model);
	fprintf(f, "mesh=%d, ", self->mesh);
	fprintf(f, "transformation=[TODO], ");
	/* glms_mat4_print (self->transformation, f); */
	fprintf(f, "is_colliding=%d, ", self->is_colliding);
	fprintf(f, "hide=%d, ", self->hide);
	fprintf(f, "added=%d, ", self->added);
	fprintf(f, "selected=%d)", self->selected);
}
