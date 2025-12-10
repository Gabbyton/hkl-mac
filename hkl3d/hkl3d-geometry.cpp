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
#include <sys/time.h>

#include "hkl3d-private.h"
#include "hkl-geometry-private.h"
#include "hkl-parameter-private.h"

#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

/**************/
/* Hkl3DStats */
/**************/

double hkl3d_stats_get_collision_ms(const Hkl3DStats *self)
{
	return self->collision.tv_sec*1000. + self->collision.tv_usec/1000.;
}

void hkl3d_stats_fprintf(FILE *f, const Hkl3DStats *self)
{
	fprintf(f, "Hkl3DStats (transformation=%f ms, collision=%f ms)",
		self->transformation.tv_sec*1000. + self->transformation.tv_usec/1000.,
		hkl3d_stats_get_collision_ms(self));
}

/*************/
/* Hkl3DAxis */
/*************/

static Hkl3DAxis *hkl3d_axis_new(struct aiNode *node, const HklParameter *mparameter,
				 const Hkl3DModel *model)
{
	g_return_val_if_fail (nullptr != node, nullptr);

	size_t i;
	Hkl3DObject **object;
	Hkl3DAxis *self = g_new0 (Hkl3DAxis, 1);
	self->node = node;
	self->mparameter = mparameter;
	darray_init(self->objects); /* dos not own the objects */

	for(i=0; i<node->mNumMeshes; ++i){
		size_t idx = node->mMeshes[i];

		darray_foreach(object, model->objects){
			if ((*object)->mesh == idx){
				darray_append(self->objects, *object);
				break;
			}
		}
	}

	return self;
}

static void hkl3d_axis_free(Hkl3DAxis *self)
{
	if(!self)
		return;

	darray_free(self->objects);
	free(self);
}

void hkl3d_axis_fprintf(FILE *f, const Hkl3DAxis *self)
{
	g_return_if_fail (nullptr != f);
	g_return_if_fail (nullptr != self);

	fprintf(f, "Hkl3DAxis (node=%p, mparameter=%p, objects=[", self->node, self->mparameter);
	for(size_t i=0; i<darray_size(self->objects); ++i){
		if(i) fprintf (f, ", ");
		fprintf (f, "%p", darray_item(self->objects, i));
	}
	fprintf(f, "])");
}

/*****************/
/* Hkl3DGeometry */
/*****************/

Hkl3DAxis *hkl3d_geometry_axis_get(Hkl3DGeometry *self, const char *name)
{
	for(size_t i=0; i<darray_size(self->geometry->axes); ++i){
		if (!strcmp(hkl_parameter_name_get(darray_item(self->geometry->axes, i)),
			    name))
			return darray_item(self->axes, i);
	}
	return nullptr;
}

static void add_axis_from_node(Hkl3DGeometry *self, const Hkl3DModel *model,
			       const HklGeometry *geometry, struct aiNode *node)
{
	size_t i;
	const HklParameter *parameter = hkl_geometry_axis_get(geometry, node->mName.data, NULL);

	darray_append(self->axes, hkl3d_axis_new(node, parameter, model));

	for(i=0; i<node->mNumChildren; ++i){
		add_axis_from_node(self, model, geometry, node->mChildren[i]);
	}
}

Hkl3DGeometry *hkl3d_geometry_new(const Hkl3DConfig *config, HklGeometry *geometry)
{
	HklParameter **axis;
	Hkl3DGeometry *self = nullptr;
	Hkl3DModel **model;

	self = g_new0 (Hkl3DGeometry, 1);
	self->geometry = geometry;
	darray_init(self->axes);
	darray_foreach(model, config->models){
		add_axis_from_node(self, *model, geometry, (*model)->scene->mRootNode);
	}

	return self;
}

void hkl3d_geometry_free(Hkl3DGeometry *self)
{
	g_return_if_fail (NULL != self);

	Hkl3DAxis **axis;
	darray_foreach(axis, self->axes){
		hkl3d_axis_free(*axis);
	}
	darray_free(self->axes);
	free(self);
}

static mat4s hkl_parameter_transformation_get(const HklParameter *self)
{
        CGLM_ALIGN_MAT mat4s r = GLMS_MAT4_IDENTITY_INIT;
        HklParameterType type = hkl_parameter_type_get(self);

        match(type){
                of(Parameter) { };
                of(Rotation, axis_v) {
                        CGLM_ALIGN_MAT vec3s axis = {{
					(float)axis_v->data[0],
					(float)axis_v->data[1],
					(float)axis_v->data[2]
				}};

                        r = glms_rotate_make(self->_value, axis);
                };
                of(RotationWithOrigin, axis_v, pivot_v) {
                        CGLM_ALIGN_MAT vec3s pivot = {{
					(float)pivot_v->data[0],
					(float)pivot_v->data[1],
					(float)pivot_v->data[2]
				}};
                        float angle = self->_value;
                        CGLM_ALIGN_MAT vec3s axis = {{
					(float)axis_v->data[0],
					(float)axis_v->data[1],
					(float)axis_v->data[2]
				}};

                        r = glms_rotate_atm(pivot, angle, axis);
                };
                of(Translation, v_v) {
                        CGLM_ALIGN_MAT vec3s v = {{
					(float)v_v->data[0],
					(float)v_v->data[1],
					(float)v_v->data[2]
				}};
                        v = glms_vec3_scale(v, self->_value);

                        r = glms_translate_make(v);
                };
        }

        return r;
}

static mat4s hkl3d_axis_get_local_transformation(const Hkl3DAxis *self)
{
	mat4s transformation;

	memcpy (transformation.raw, &self->node->mTransformation.a1, 16 * sizeof(float));

	if (self->mparameter){
		transformation = glms_mat4_mul (transformation, hkl_parameter_transformation_get(self->mparameter));
	}

	return transformation;
}

static Hkl3DAxis *hkl3d_geometry_axis_get_by_node(const Hkl3DGeometry *self, const aiNode *node)
{
	Hkl3DAxis **axis = nullptr;

	darray_foreach(axis, self->axes){
		if ((*axis)->node == node)
			break;
	}

	return (*axis);
}

static mat4s hkl3d_axis_get_world_transformation(const Hkl3DAxis *self, const Hkl3DGeometry *geometry)
{
	aiNode *node;
	mat4s transformation = hkl3d_axis_get_local_transformation(self);

	node = self->node->mParent;
	while (node){
		Hkl3DAxis *pAxis = hkl3d_geometry_axis_get_by_node(geometry, node);
		if (nullptr != pAxis)
			transformation = glms_mat4_mul (hkl3d_axis_get_local_transformation(pAxis),
							transformation);
		node = node->mParent;
	}

	return transformation;
}

static void hkl3d_axis_apply_transformation_to_objects(Hkl3DAxis *self, const Hkl3DGeometry *geometry)
{
	Hkl3DObject **object;

	mat4s transformation = hkl3d_axis_get_world_transformation (self, geometry);

	darray_foreach(object, self->objects){
		(*object)->transformation = transformation;
	}
}

void hkl3d_geometry_apply_transformations(Hkl3DGeometry *self)
{
	Hkl3DAxis **axis;

	darray_foreach(axis, self->axes){
		hkl3d_axis_apply_transformation_to_objects(*axis, self);
	}
}

void hkl3d_geometry_fprintf(FILE *f, const Hkl3DGeometry *self)
{
	g_return_if_fail (NULL != f);
	g_return_if_fail (NULL != self);

	fprintf(f, "Hkl3DGeometry (geometry=%p, axes=[", self->geometry);
	// TODO hkl_geometry_fprintf(f, self->geometry);
	for(size_t i=0; i<darray_size(self->axes); ++i){
		if(i) fprintf(f, ", ");
		hkl3d_axis_fprintf(f, darray_item(self->axes, i));
	}
	fprintf(f, "])");
}
