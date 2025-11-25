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
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"


/*********************/
/* Hkl3DBulletObject */
/*********************/

static btTriangleMesh *trimesh_from_aiMesh(const struct aiMesh *mesh)
{
	unsigned int i;
	btTriangleMesh *trimesh;

	trimesh = new btTriangleMesh();
	trimesh->preallocateVertices(mesh->mNumVertices);
	for(i=0; i<mesh->mNumFaces; ++i){
		btVector3 vertex0 (mesh->mVertices[mesh->mFaces[i].mIndices[0]].x,
				   mesh->mVertices[mesh->mFaces[i].mIndices[0]].y,
				   mesh->mVertices[mesh->mFaces[i].mIndices[0]].z);
		btVector3 vertex1 (mesh->mVertices[mesh->mFaces[i].mIndices[1]].x,
				   mesh->mVertices[mesh->mFaces[i].mIndices[1]].y,
				   mesh->mVertices[mesh->mFaces[i].mIndices[1]].z);
		btVector3 vertex2 (mesh->mVertices[mesh->mFaces[i].mIndices[2]].x,
				   mesh->mVertices[mesh->mFaces[i].mIndices[2]].y,
				   mesh->mVertices[mesh->mFaces[i].mIndices[2]].z);
		trimesh->addTriangle(vertex0, vertex1, vertex2, true);
	}

	return trimesh;
}

static btCollisionShape* shape_from_trimesh(btTriangleMesh *trimesh, int movable)
{
	btCollisionShape* shape;

	/*
	 * create the bullet shape depending on the static status or not of the piece
	 * static : do not move
	 * movable : connected to a HklGeometry axis.
	 */
	if (movable >= 0){
		shape = dynamic_cast<btGImpactMeshShape*>(new btGImpactMeshShape(trimesh));
		shape->setMargin(btScalar(0));
		shape->setLocalScaling(btVector3(1,1,1));
		/* maybe usefull for softbodies (useless for now) */
		(dynamic_cast<btGImpactMeshShape*>(shape))->postUpdate();
		/* needed for the collision and must be call after the postUpdate (doc) */
		(dynamic_cast<btGImpactMeshShape*>(shape))->updateBound();
	}else{
		shape = dynamic_cast<btBvhTriangleMeshShape*>(new btBvhTriangleMeshShape (trimesh, true));
		shape->setMargin(btScalar(0));
		shape->setLocalScaling(btVector3(1,1,1));
	}

	return shape;
}

static btCollisionObject * btObject_from_shape(btCollisionShape* shape)
{
	btCollisionObject *btObject;

	/* create the Object and add the shape */
	btObject = new btCollisionObject();
	btObject->setCollisionShape(shape);
	btObject->activate(true);

	return btObject;
}


Hkl3DBulletObject *hkl3d_bullet_object_new(const struct aiMesh *mesh)
{
	Hkl3DBulletObject *self = g_new0 (Hkl3DBulletObject, 1);

	self->meshes = trimesh_from_aiMesh(mesh);
	self->btShape = shape_from_trimesh(self->meshes, false);
	self->btObject = btObject_from_shape(self->btShape);

	return self;
}

void hkl3d_bullet_object_free(Hkl3DBulletObject *self)
{
	if(self->btObject){
		delete self->btObject;
	}
	if(self->btShape){
		delete self->btShape;
	}
	if(self->meshes){
		delete self->meshes;
	}
}

void hkl3d_bullet_object_fprintf(FILE *f, const Hkl3DBulletObject *self)
{
	GSList *faces;

	fprintf(f, "Hkl3dBulletObject (", self);
	fprintf(f, "btObject=%p, ", self->btObject);
	fprintf(f, "btShape=%p, ", self->btShape);
	fprintf(f, "meshes=%p), ", self->meshes);
}

/***************/
/* Hkl3DBullet */
/***************/

Hkl3DBullet *hkl3d_bullet_new(const Hkl3DGeometry *geometry)
{
	Hkl3DBullet *self =  g_new0 (Hkl3DBullet, 1);
	Hkl3DAxis **axis;
	Hkl3DObject **object;

	/* initialize the bullet part */
	self->_btCollisionConfiguration = new btDefaultCollisionConfiguration();
	self->_btDispatcher = new btCollisionDispatcher(self->_btCollisionConfiguration);
	btGImpactCollisionAlgorithm::registerAlgorithm(self->_btDispatcher);

	btVector3 worldAabbMin(-1000,-1000,-1000);
	btVector3 worldAabbMax( 1000, 1000, 1000);

	self->_btBroadphase = new btAxisSweep3(worldAabbMin, worldAabbMax);

	self->_btWorld = new btCollisionWorld(self->_btDispatcher,
					      self->_btBroadphase,
					      self->_btCollisionConfiguration);

	/* populate the collision world with all objects of the geometry */
	darray_foreach(axis, geometry->axes){
		darray_foreach(object, (*axis)->objects){
			hkl3d_bullet_collision_object_add (self, *object);
		}
	}


	return self;
}

void
hkl3d_bullet_free(Hkl3DBullet *self)
{
	if (self->_btWorld)
		delete self->_btWorld;
	if (self->_btBroadphase)
		delete self->_btBroadphase;
	if (self->_btDispatcher)
		delete self->_btDispatcher;
	if (self->_btCollisionConfiguration)
		delete self->_btCollisionConfiguration;

}

void
hkl3d_bullet_collision_object_remove(Hkl3DBullet *self, Hkl3DObject *object)
{
	self->_btWorld->removeCollisionObject(object->bullet->btObject);
	object->added = false;
}

void
hkl3d_bullet_collision_object_add(Hkl3DBullet *self, Hkl3DObject *object)
{
	self->_btWorld->addCollisionObject(object->bullet->btObject);
	object->added = true;
}

bool
hkl3d_bullet_perform_collision (Hkl3DBullet *self, Hkl3DConfig *config)
{
	int numManifolds;
	Hkl3DModel **model;
	Hkl3DObject **object;

	self->_btWorld->performDiscreteCollisionDetection();
	self->_btWorld->updateAabbs();

	numManifolds = self->_btDispatcher->getNumManifolds();

	/* update Hkl3DObject collision from manifolds */
	darray_foreach(model, config->models){
		darray_foreach(object, (*model)->objects){
			(*object)->is_colliding = FALSE;
			for(int k=0; k<numManifolds; ++k){
				btPersistentManifold *manifold = self->_btDispatcher->getManifoldByIndexInternal(k);
				(*object)->is_colliding |= (*object)->bullet->btObject == manifold->getBody0();
				(*object)->is_colliding |= (*object)->bullet->btObject == manifold->getBody1();
			}
		}
	}

	return numManifolds != 0;
}

void
hkl3d_bullet_get_collision_coordinates(const Hkl3DBullet *self, int manifold, int contact,
				       double *xa, double *ya, double *za,
				       double *xb, double *yb, double *zb)
{
	btPersistentManifold *contactManifold;

	contactManifold = self->_btDispatcher->getManifoldByIndexInternal(manifold);
	btManifoldPoint & pt = contactManifold->getContactPoint(contact);
	btVector3 ptA = pt.getPositionWorldOnA();
	btVector3 ptB = pt.getPositionWorldOnB();

	*xa = ptA.x();
	*ya = ptA.y();
	*za = ptA.z();
	*xb = ptB.x();
	*yb = ptB.y();
	*zb = ptB.z();
}

void
hkl3d_bullet_fprintf(FILE *f, const Hkl3DBullet *self)
{
	fprintf(f, "Hkl3DBullet (");
	fprintf(f, "_btCollisionConfiguration=%p", self->_btCollisionConfiguration);
	fprintf(f, ", _btBroadphase=%p", self->_btBroadphase);
	fprintf(f, ", _btWorld=%p", self->_btWorld);
	fprintf(f, ", _btDispatcher=%p)", self->_btDispatcher);
}
