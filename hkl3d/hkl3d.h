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

#include "hkl.h"

#define SOLEIL_DIFFABS_MODEL "soleil-diffabs.yaml"
#define SOLEIL_CRISTAL_4C_MODEL "soleil-cristal-4c.yaml"

// forward declaration due to bullet static linking
struct btVector3;

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _Hkl3DStats Hkl3DStats;
	typedef struct _Hkl3DObject Hkl3DObject;
	typedef struct _Hkl3DModel Hkl3DModel;
	typedef struct _Hkl3DConfig Hkl3DConfig;
	typedef struct _Hkl3DAxis Hkl3DAxis;
	typedef struct _Hkl3DGeometry Hkl3DGeometry;
	typedef struct _Hkl3D Hkl3D;

	/**************/
	/* Hkl3DStats */
	/**************/

	HKLAPI extern double hkl3d_stats_get_collision_ms(const Hkl3DStats *self);
	HKLAPI extern void hkl3d_stats_fprintf(FILE *f, Hkl3DStats *self);

	/***************/
	/* Hkl3DObject */
	/***************/

	HKLAPI extern void hkl3d_object_aabb_get(const Hkl3DObject *self, float from[3], float to[3]);
	HKLAPI extern void hkl3d_object_axis_name_set(Hkl3DObject *self, const char *axis_name); /* TODO remove not a good idea */
	HKLAPI extern void hkl3d_object_draw_aabb_set(Hkl3DObject *self, bool aabb);
	HKLAPI extern bool hkl3d_object_hide_get(const Hkl3DObject *self);
	HKLAPI extern void hkl3d_object_hide_set(Hkl3DObject *self, bool hide);
	HKLAPI extern void hkl3d_object_transformation_set(Hkl3DObject *self, mat4s transformation);
	HKLAPI extern void hkl3d_object_fprintf(FILE *f, const Hkl3DObject *self);

	/**************/
	/* Hkl3DModel */
	/**************/

	HKLAPI extern void hkl3d_model_free(Hkl3DModel *self);
	HKLAPI extern void hkl3d_model_fprintf(FILE *f, const Hkl3DModel *self);

	/***************/
	/* Hkl3DConfig */
	/***************/

	HKLAPI extern void hkl3d_config_free(Hkl3DConfig *self); // TODO private
	HKLAPI extern Hkl3DConfig *hkl3d_config_new(const char *filename); // TODO private
	HKLAPI extern void hkl3d_config_fprintf(FILE *f, const Hkl3DConfig *self);

	/*************/
	/* Hkl3DAxis */
	/*************/

	HKLAPI extern void hkl3d_axis_fprintf(FILE *f, const Hkl3DAxis *self);

	/*****************/
	/* HKL3DGeometry */
	/*****************/

	HKLAPI extern void hkl3d_geometry_fprintf(FILE *f, const Hkl3DGeometry *self);

	/*********/
	/* HKL3D */
	/*********/

	HKLAPI extern Hkl3D* hkl3d_new(const char *filename, HklGeometry *geometry) HKL_ARG_NONNULL(2);
	HKLAPI extern void hkl3d_free(Hkl3D *self) HKL_ARG_NONNULL(1);

	HKLAPI extern int hkl3d_is_colliding(Hkl3D *self) HKL_ARG_NONNULL(1);
	HKLAPI extern int hkl3d_load_config(Hkl3D *self, const char *filename) HKL_ARG_NONNULL(1, 2);
	HKLAPI extern void hkl3d_save_config(Hkl3D *self, const char *filename) HKL_ARG_NONNULL(1, 2);
	HKLAPI extern Hkl3DModel *hkl3d_add_model_from_file(Hkl3D *self,
							    const char *filename,
							    const char *directory) HKL_ARG_NONNULL(1, 2, 3);

	HKLAPI extern void hkl3d_connect_all_axes(Hkl3D *self) HKL_ARG_NONNULL(1);
	HKLAPI extern void hkl3d_hide_object(Hkl3D *self, Hkl3DObject *object, int hide) HKL_ARG_NONNULL(1, 2);
	HKLAPI extern void hkl3d_remove_object(Hkl3D *self, Hkl3DObject *object) HKL_ARG_NONNULL(1, 2);

	HKLAPI extern void hkl3d_get_bounding_boxes(Hkl3D *self,
						    struct btVector3 *min,
						    struct btVector3 *max) HKL_ARG_NONNULL(1, 2, 3);
	HKLAPI extern int hkl3d_get_nb_manifolds(Hkl3D *self) HKL_ARG_NONNULL(1);
	HKLAPI extern int hkl3d_get_nb_contacts(Hkl3D *self, int manifold) HKL_ARG_NONNULL(1);
	HKLAPI extern void hkl3d_get_collision_coordinates(Hkl3D *self, int manifold, int contact,
							   double *xa, double *ya, double *za,
							   double *xb, double *yb, double *zb) HKL_ARG_NONNULL(1);
	HKLAPI extern void hkl3d_connect_object_to_axis(Hkl3D *self,
							Hkl3DObject *object,
							const char *name) HKL_ARG_NONNULL(1, 2);

	HKLAPI extern void hkl3d_fprintf(FILE *f, const Hkl3D *self) HKL_ARG_NONNULL(1, 2);

	/* Opengl */
	HKLAPI void hkl3d_gl_draw_aabb_set(Hkl3D *self, bool aabb);
	HKLAPI void hkl3d_gl_draw_models(Hkl3D *self);
	HKLAPI void hkl3d_gl_init(Hkl3D *self);
	HKLAPI void hkl3d_gl_resize(Hkl3D *self, gint width, gint height);


#ifdef __cplusplus
}
#endif
