#pragma once
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
 * Copyright (C) 2010-2019, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 *          Oussama SBOUI <oussama.sboui@synchrotron-soleil.fr>
 */

#include <assimp/scene.h>
#include <cglm/struct.h>
#include <epoxy/gl.h>

#include "hkl.h"
#include "hkl3d.h"

// forward declaration due to bullet static linking
struct btCollisionObject;
struct btCollisionWorld;
struct btCollisionConfiguration;
struct btBroadphaseInterface;
struct btCollisionDispatcher;
struct btCollisionShape;
struct btVector3;
struct btTriangleMesh;

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _Shader {
		GLuint program;
	} Shader;

	/**************/
	/* Hkl3DStats */
	/**************/

	struct _Hkl3DStats
	{
		struct timeval collision;
		struct timeval transformation;
	};

	/***************/
	/* Hkl3DObject */
	/***************/

	struct _Hkl3DObject
	{
		Hkl3DModel *model; /* weak reference */
		Hkl3DAxis *axis; /* weak reference */
		unsigned int mesh;
		struct btCollisionObject *btObject;
		struct btCollisionShape *btShape;
		struct btTriangleMesh *meshes;
		int is_colliding;
		int hide;
		int added;
		int draw_aabb;
		int selected;
		int movable;
		char *axis_name; /* TODO remove */
		GLuint vao;
		CGLM_ALIGN_MAT mat4s transformation;
	};

	/**************/
	/* Hkl3DModel */
	/**************/

	struct _Hkl3DModel
	{
		char *filename;
		const struct aiScene *scene;
		darray_object objects;
	};

	/***************/
	/* Hkl3DConfig */
	/***************/

	struct _Hkl3DConfig
	{
		char const *filename; /* config filename */
		darray_model models;
	};

	/*************/
	/* Hkl3DAxis */
	/*************/

	struct _Hkl3DAxis
	{
		Hkl3DObject **objects; /* connected object */
		size_t len;
	};

	/*****************/
	/* HKL3DGeometry */
	/*****************/

	struct _Hkl3DGeometry
	{
		HklGeometry *geometry; /* weak reference */
		Hkl3DAxis **axes;
	};

	/*********/
	/* HKL3D */
	/*********/

	struct _Hkl3D
	{
		Hkl3DGeometry *geometry;
		Hkl3DStats stats;
		Hkl3DConfig *config;

		struct btCollisionConfiguration *_btCollisionConfiguration;
		struct btBroadphaseInterface *_btBroadphase;
		struct btCollisionWorld *_btWorld;
		struct btCollisionDispatcher *_btDispatcher;

		Shader shader;
	};

	bool hkl3d_contains_model(const Hkl3D *self, const char * filename);
	Hkl3DObject *hkl3d_get_object_by_id(const Hkl3D *self, const char *filename, int id);

#ifdef __cplusplus
}
#endif
