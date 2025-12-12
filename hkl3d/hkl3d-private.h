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

	/*********************/
	/* Hkl3DBulletObject */
	/*********************/

	typedef darray(Hkl3DBulletObject *) darray_bobject;

	struct _Hkl3DBulletObject
	{
		Hkl3DAxis *axis;
		Hkl3DObject *object; /* for the transformation */
		struct btCollisionObject *btObject;
		struct btCollisionShape *btShape;
		struct btTriangleMesh *meshes;
	};

	void hkl3d_bullet_object_free(Hkl3DBulletObject *self);
	void hkl3d_bullet_object_fprintf(FILE *f, const Hkl3DBulletObject *self);

	/***************/
	/* Hkl3DObject */
	/***************/

	typedef darray(Hkl3DObject *) darray_object;

	struct _Hkl3DObject
	{
		Hkl3DModel *model; /* weak reference */
		unsigned int mesh;
		int is_colliding;
		int hide;
		int added;
		int draw_aabb;
		int selected;
		int movable;
		GLuint vao;
		CGLM_ALIGN_MAT mat4s transformation;
	};

	Hkl3DObject *hkl3d_object_new(Hkl3DModel *self, unsigned int mesh);
	void hkl3d_object_set_movable(Hkl3DObject *self, int movable);
	void hkl3d_object_free(Hkl3DObject *self);

	/**************/
	/* Hkl3DModel */
	/**************/

	typedef darray(Hkl3DModel *) darray_model;

	struct _Hkl3DModel
	{
		char *filename;
		const struct aiScene *scene;
		darray_object objects; /* owner of the objects */
	};

	Hkl3DModel *hkl3d_model_new_from_file(const char *filename);

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

	typedef darray(Hkl3DAxis *) darray_axis;

	struct _Hkl3DAxis
	{
		struct aiNode *node;
		const HklParameter *mparameter;
		darray_object objects; /* connected object */
	};

	void hkl3d_axis_attach_object(Hkl3DAxis *self, Hkl3DObject *object);
	void hkl3d_axis_detach_object(Hkl3DAxis *self, Hkl3DObject *object);

	/*****************/
	/* HKL3DGeometry */
	/*****************/

	struct _Hkl3DGeometry
	{
		HklGeometry *geometry; /* weak reference */
		darray_axis axes;
	};

	Hkl3DGeometry *hkl3d_geometry_new(const Hkl3DConfig *config, HklGeometry *geometry);
	void hkl3d_geometry_free(Hkl3DGeometry *self);
	Hkl3DAxis *hkl3d_geometry_axis_get(Hkl3DGeometry *self, const char *name);
	void hkl3d_geometry_apply_transformations(Hkl3DGeometry *self);

	/***************/
	/* Hkl3DBullet */
	/***************/

	struct _Hkl3DBullet
	{
		struct btCollisionConfiguration *_btCollisionConfiguration;
		struct btBroadphaseInterface *_btBroadphase;
		struct btCollisionWorld *_btWorld;
		struct btCollisionDispatcher *_btDispatcher;
		darray_bobject bobjects;
	};

	Hkl3DBullet *hkl3d_bullet_new (const Hkl3DGeometry *geometry);
	void hkl3d_bullet_free (Hkl3DBullet *self);
	void hkl3d_bullet_apply_transformations(Hkl3DBullet *self);
	void hkl3d_bullet_get_collision_coordinates(const Hkl3DBullet *self,
						    int manifold, int contact,
						    double *xa, double *ya, double *za,
						    double *xb, double *yb, double *zb);
	bool hkl3d_bullet_perform_collision (Hkl3DBullet *self, Hkl3DConfig *config);
	void hkl3d_bullet_remove_all_objects (Hkl3DBullet *self, const Hkl3DConfig *config);

	void hkl3d_bullet_fprintf(FILE *f, const Hkl3DBullet *self);

	/*********/
	/* HKL3D */
	/*********/

	struct _Hkl3D
	{
		Hkl3DGeometry *geometry;
		Hkl3DStats stats;
		Hkl3DConfig *config;
		Hkl3DBullet *bullet;
		Shader shader;
	};

	bool hkl3d_contains_model(const Hkl3D *self, const char * filename);
	Hkl3DObject *hkl3d_get_object_by_id(const Hkl3D *self, const char *filename, int id);

#ifdef __cplusplus
}
#endif
