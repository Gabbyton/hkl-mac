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

#include <stdio.h>

#include <glib.h>
#include <glib-object.h>

#include "hkl.h"

struct diffractometer_t {
	HklFactory *factory;
	HklGeometry *geometry;
	HklDetector *detector;
	HklEngineList *engines;
	HklGeometryList *solutions; /* not owned */
};

struct diffractometer_t * create_diffractometer(HklFactory *factory);

void diffractometer_fprintf(FILE *f, struct diffractometer_t *self);

void diffractometer_free(struct diffractometer_t *self);

void diffractometer_update(struct diffractometer_t *self);

void diffractometer_set_sample(struct diffractometer_t *self,
			       HklSample *sample);

gdouble diffractometer_get_wavelength(const struct diffractometer_t *self);

void diffractometer_set_wavelength(struct diffractometer_t *self,
				   double wavelength);

gboolean diffractometer_set_solutions(struct diffractometer_t *self, HklGeometryList *solutions);

gboolean diffractometer_pseudo_axis_values_set(struct diffractometer_t *self,
					       HklEngine *engine, gdouble values[], guint n_values,
					       GError **error);

void diffractometer_set_geometry(struct diffractometer_t *self,
				 const HklGeometry *geometry);
