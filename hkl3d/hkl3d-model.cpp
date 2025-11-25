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

// #include <assimp/cexport.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "hkl3d-private.h"

/**************/
/* Hkl3DModel */
/**************/

/*
 * Initialize the bullet collision environment.
 * create the Hkl3DObjects
 * create the Hkl3DConfig
 */
Hkl3DModel *hkl3d_model_new_from_file(const char *filename)
{
	g_return_val_if_fail(NULL != filename, nullptr);

	unsigned int i;
	Hkl3DModel *self;
	const struct aiScene *scene;

	scene = aiImportFile(filename,
			     aiProcess_CalcTangentSpace
			     | aiProcess_Triangulate
			     | aiProcess_JoinIdenticalVertices
			     | aiProcess_SortByPType);

	if (nullptr == scene) {
		fprintf(stdout, "\n%s", aiGetErrorString());
		goto out;
	}

	/* aiExportScene(scene, "glb2", g_strdup_printf("%s.glb", filename), 0); */

	self = g_new0 (Hkl3DModel, 1);
	self->filename = g_strdup(filename);
	self->scene = scene;
	darray_init(self->objects);

	/* keep only the mesh with triangles */
	for(i=0; i<scene->mNumMeshes; ++i){
		const struct aiMesh *mesh = scene->mMeshes[i];
		if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE){
			Hkl3DObject *object = hkl3d_object_new(self, i);

			if (nullptr != object)
				darray_append(self->objects, object);
		}
	}

	return self;

out:
	return nullptr;
}

const char *
hkl3d_model_filename_get(const Hkl3DModel *self)
{
	g_return_val_if_fail(nullptr != self, nullptr);

	return self->filename;
}

void hkl3d_model_free(Hkl3DModel *self)
{
	g_return_if_fail (nullptr != self);

	Hkl3DObject **object;

	darray_foreach (object, self->objects){
		hkl3d_object_free (*object);
	}
	darray_free (self->objects);
	aiReleaseImport (self->scene);
	free (self->filename);
	free (self);
}

static void hkl3d_model_delete_object(Hkl3DModel *self, Hkl3DObject *object)
{
	size_t i;

	if(!self || !object)
		return;

	if(self != object->model)
		return;

	/* find the index of the object */
	for(i=0; darray_item(self->objects, i) != object; ++i);

	hkl3d_object_free(object);
	darray_remove(self->objects, i);
}

void hkl3d_model_fprintf(FILE *f, const Hkl3DModel *self)
{
	fprintf(f,"Hkl3DModel (filename=[%s], scene=%p, objects[",
		self->filename, self->scene);
	for(size_t i=0; i<darray_size(self->objects); ++i){
		if(i) fprintf(f, ", ");
		hkl3d_object_fprintf(f, darray_item(self->objects, i));
	}
	fprintf(f, "])");
}
