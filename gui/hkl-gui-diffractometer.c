/* This file is part of the hkl library.
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
 * Copyright (C) 2003-2019, 2022, 2024, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#include <glib.h>
#include <glib-object.h>

#include "hkl.h"
#include "hkl-gui-diffractometer-private.h"

struct diffractometer_t *create_diffractometer(HklFactory *factory)
{
	struct diffractometer_t *self = g_new(struct diffractometer_t, 1);

	self->factory = factory;
	self->geometry = hkl_factory_create_new_geometry (factory);
	self->engines = hkl_factory_create_new_engine_list (factory);
	self->detector = hkl_detector_factory_new (HKL_DETECTOR_TYPE_0D);
	self->solutions = NULL;

	return self;
}

void diffractometer_fprintf(FILE *f, struct diffractometer_t *self)
{
	hkl_geometry_fprintf(f, self->geometry);
	hkl_detector_fprintf(f, self->detector);
	hkl_engine_list_fprintf(f, self->engines);
	/* hkl_geometry_list_fprintf(f, self->solutions); */
}

void diffractometer_free(struct diffractometer_t *self)
{
	if (NULL != self->geometry) {
		hkl_geometry_free(self->geometry);
		self->geometry = NULL;
	}

	if (NULL != self->engines){
		hkl_engine_list_free(self->engines);
		self->engines = NULL;
	}
	if(NULL != self->detector){
		hkl_detector_free(self->detector);
		self->detector = NULL;
	}

	if(NULL != self->solutions){
		hkl_geometry_list_free(self->solutions);
		self->solutions = NULL;
	}
	g_free(self);
}

void diffractometer_update(struct diffractometer_t *self)
{
	g_return_if_fail(NULL != self);
	g_return_if_fail(NULL != self->engines);

	hkl_engine_list_get(self->engines);
	diffractometer_fprintf(stdout, self);
}

void diffractometer_set_sample(struct diffractometer_t *self,
			       HklSample *sample)
{
	g_return_if_fail(NULL != self);
	g_return_if_fail(NULL != sample);

	hkl_engine_list_init(self->engines,
			     self->geometry,
			     self->detector,
			     sample);

	/* TOTO remove ...*/
	HklEngine *engine = darray_item(*hkl_engine_list_engines_get(self->engines), 0);
	const darray_string *pseudo_axes = hkl_engine_pseudo_axis_names_get(engine);
	const size_t n_pseudo_axes = darray_size(*pseudo_axes);
	double targets[] = {0, 1, 0};
	self->solutions = hkl_engine_pseudo_axis_values_set(engine,
							    targets, n_pseudo_axes,
							    HKL_UNIT_DEFAULT, NULL);
	/* TODO remove until here */

	diffractometer_update(self);
}

gdouble diffractometer_get_wavelength(const struct diffractometer_t *self)
{
	g_return_val_if_fail(NULL != self, 0);

	return hkl_geometry_wavelength_get(self->geometry, HKL_UNIT_USER);
}

void diffractometer_set_wavelength(struct diffractometer_t *self,
				   double wavelength)
{
	g_return_if_fail(NULL != self);

	if(hkl_geometry_wavelength_set(self->geometry,
				       wavelength, HKL_UNIT_USER, NULL)){
		diffractometer_update(self);
	}
}

gboolean diffractometer_set_solutions(struct diffractometer_t *self, HklGeometryList *solutions)
{
	g_return_val_if_fail(NULL != self, FALSE);
	g_return_val_if_fail(NULL != solutions, FALSE);

	if(self->solutions)
		hkl_geometry_list_free(self->solutions);
	self->solutions = solutions;

	return TRUE;
}

gboolean diffractometer_pseudo_axis_values_set(struct diffractometer_t *self,
					       HklEngine *engine, gdouble values[], guint n_values,
					       GError **error)
{
	g_return_val_if_fail(NULL != self, FALSE);
	g_return_val_if_fail(NULL != engine, FALSE);

	HklGeometryList *solutions;

	solutions = hkl_engine_pseudo_axis_values_set(engine, values, n_values, HKL_UNIT_USER, error);

	return diffractometer_set_solutions(self, solutions);
}

void diffractometer_set_geometry(struct diffractometer_t *self,
				 const HklGeometry *geometry)
{
	g_return_if_fail(NULL != self);
	g_return_if_fail(NULL != geometry);

	hkl_geometry_set(self->geometry, geometry);
	hkl_engine_list_geometry_set(self->engines, geometry);
}
