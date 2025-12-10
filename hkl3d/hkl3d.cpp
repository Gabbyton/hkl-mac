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

/*********/
/* HKL3D */
/*********/

static void hkl3d_apply_transformations(Hkl3D *self)
{
	struct timeval debut, fin;

	/* set the right transformation of each objects and get numbers */
	gettimeofday(&debut, nullptr);
	hkl3d_geometry_apply_transformations (self->geometry);
	hkl3d_bullet_apply_transformations (self->bullet);
	gettimeofday(&fin, nullptr);
	timersub(&fin, &debut, &self->stats.transformation);
}

/**
 * Hkl3D::Hkl3D:
 * @filename:
 * @geometry:
 *
 *
 *
 * Returns:
 **/
Hkl3D *hkl3d_new(const char *filename, HklGeometry *geometry)
{
	g_return_val_if_fail (nullptr != filename, nullptr);
	g_return_val_if_fail (nullptr != geometry, nullptr);

	Hkl3D *self = nullptr;

	self = g_new0 (Hkl3D, 1);

	g_return_val_if_fail (nullptr != self, nullptr);

	self->config = hkl3d_config_new(filename);
	self->geometry = hkl3d_geometry_new(self->config, geometry);
	self->bullet = hkl3d_bullet_new(self->geometry);

	return self;
}

void hkl3d_free(Hkl3D *self)
{
	hkl3d_bullet_free (self->bullet);
	hkl3d_geometry_free (self->geometry);
	hkl3d_config_free (self->config);
	free (self);
}

int hkl3d_is_colliding(Hkl3D *self)
{
	bool res;
	int numManifolds;
	struct timeval debut, fin;
	Hkl3DModel **model;
	Hkl3DObject **object;

	/* apply geometry transformation */
	hkl3d_apply_transformations(self);

	/* perform the collision detection and get numbers */
	gettimeofday(&debut, nullptr);
	res = hkl3d_bullet_perform_collision(self->bullet, self->config);
	gettimeofday(&fin, nullptr);
	timersub(&fin, &debut, &self->stats.collision);

	return res;
}

void hkl3d_get_collision_coordinates(Hkl3D *self, int manifold, int contact,
				     double *xa, double *ya, double *za,
				     double *xb, double *yb, double *zb)
{
	return hkl3d_bullet_get_collision_coordinates(self->bullet, manifold, contact,
						      xa, ya, za, xb, yb, zb);
}

void hkl3d_fprintf(FILE *f, const Hkl3D *self)
{
	fprintf(f, "Hkl3d (");
	fprintf(f, "geometry=");
	hkl3d_geometry_fprintf(f, self->geometry);
	fprintf(f, ", stats=");
	hkl3d_stats_fprintf(f, &self->stats);
	fprintf(f, ", config=");
	hkl3d_config_fprintf(f, self->config);
	fprintf(f, ", bullet=");
	hkl3d_bullet_fprintf(f, self->bullet);
	fprintf(f, ")");
}
