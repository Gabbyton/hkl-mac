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


class Hkl3DDebug : public btIDebugDraw
{
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
		{
			GLfloat vertexes[] = {
				from.x(), from.y(), from.z(), color.x(), color.y(), color.z(),
				to.x(), to.y(), to.z(), color.x(), color.y(), color.z(),
			};

			GLuint vbo = 0;
			glGenBuffers( 1, &vbo );
			glBindBuffer( GL_ARRAY_BUFFER, vbo );
			glBufferData( GL_ARRAY_BUFFER, (6 * 2) * sizeof( float ), vertexes, GL_STATIC_DRAW );

			GLuint vao = 0;
			glGenVertexArrays( 1, &vao );
			glBindVertexArray( vao );

			glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof (GLfloat), NULL );
			glEnableVertexAttribArray( 0 );

			glDrawArrays (GL_LINES, 0, 2 * 1);

			glDeleteBuffers(1, &vbo);
			glDeleteVertexArrays(1, &vao);
		}

	void reportErrorWarning(const char* warningString)
		{
			fprintf (stdout, "%s\n", warningString);
		};

	void draw3dText(const btVector3& location, const char* textString)
		{
			fprintf (stdout, "%s\n", textString);
		};

	void setDebugMode(int debugMode) {};

	int getDebugMode() const { return DBG_DrawWireframe | DBG_DrawAabb | DBG_DrawContactPoints; }

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
		{
			drawSphere (PointOnB, distance, color);
			drawLine (PointOnB, PointOnB + normalOnB * distance, color);
		};

};

void hkl3d_bullet_gl_draw_debug (Hkl3DBullet *self)
{
	self->_btWorld->debugDrawWorld();
}

/*********************/
/* Hkl3DBulletObject */
/*********************/

static btTriangleMesh *trimesh_from_axis(Hkl3DAxis *axis)
{
	unsigned int i;
	Hkl3DObject **object;
	btTriangleMesh *trimesh;

	trimesh = new btTriangleMesh();

	int mNumVertices = 0;
	darray_foreach (object, axis->objects){
		const struct aiMesh *mesh = (*object)->model->scene->mMeshes[(*object)->mesh];

		mNumVertices += mesh->mNumVertices;
	}
	trimesh->preallocateVertices(mNumVertices);

	darray_foreach (object, axis->objects){
		const struct aiMesh *mesh = (*object)->model->scene->mMeshes[(*object)->mesh];

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
			trimesh->addTriangle(vertex0, vertex1, vertex2, false);
		}
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


static Hkl3DBulletObject *
hkl3d_bullet_object_new(Hkl3DAxis *axis)
{
	Hkl3DBulletObject *self = nullptr;

	if (darray_size(axis->objects) > 0){
		self = g_new0 (Hkl3DBulletObject, 1);
		self->axis = axis;
		self->object = darray_item(axis->objects, 0);
		self->draw_aabb = false;
		self->meshes = trimesh_from_axis(axis);
		self->btShape = shape_from_trimesh(self->meshes, false);
		self->btObject = btObject_from_shape(self->btShape);
	}

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

void hkl3d_bullet_object_aabb_get(const Hkl3DBulletObject *self,
				  float from[3], float to[3])
{
	btVector3 min, max;

	self->btShape->getAabb(self->btObject->getWorldTransform(), min, max);

	from[0] = min.getX();
	from[1] = min.getY();
	from[2] = min.getZ();
	to[0] = max.getX();
	to[1] = max.getY();
	to[2] = max.getZ();
}

void hkl3d_bullet_object_draw_aabb_set(Hkl3DBulletObject *self, bool draw_aabb)
{
	self->draw_aabb = draw_aabb;
}

void hkl3d_bullet_object_fprintf(FILE *f, const Hkl3DBulletObject *self)
{
	GSList *faces;

	fprintf (f, "Hkl3dBulletObject (", self);
	fprintf (f, "axis=%p, ", self->axis);
	fprintf (f, "object=%p, ", self->object);
	fprintf (f, "object->is_colliding=%d, ", self->object->is_colliding);
	fprintf (f, "btObject=%p, ", self->btObject);
	fprintf (f, "btShape=%p, ", self->btShape);
	fprintf (f, "meshes=%p)", self->meshes);
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

	auto debug = new Hkl3DDebug();
	self->_btWorld->setDebugDrawer (debug);

	/* populate the collision world with all objects of the geometry */
	darray_init(self->bobjects);
	darray_foreach(axis, geometry->axes){
		Hkl3DBulletObject *bobject = hkl3d_bullet_object_new(*axis);

		if (nullptr != bobject){
			darray_append(self->bobjects, bobject);
			self->_btWorld->addCollisionObject(bobject->btObject);
		}
	}

	return self;
}

void
hkl3d_bullet_free(Hkl3DBullet *self)
{
	Hkl3DBulletObject **bobject;

	darray_foreach(bobject, self->bobjects){
		self->_btWorld->removeCollisionObject((*bobject)->btObject);
		hkl3d_bullet_object_free (*bobject);
	}
	darray_free(self->bobjects);

	if (self->_btWorld)
		delete self->_btWorld;
	if (self->_btBroadphase)
		delete self->_btBroadphase;
	if (self->_btDispatcher)
		delete self->_btDispatcher;
	if (self->_btCollisionConfiguration)
		delete self->_btCollisionConfiguration;

}

void hkl3d_bullet_apply_transformations(Hkl3DBullet *self)
{
	Hkl3DBulletObject **bobject;

	darray_foreach(bobject, self->bobjects){
		(*bobject)->btObject->getWorldTransform().setFromOpenGLMatrix( &(*bobject)->object->transformation.raw[0][0] );
	}
}


bool
hkl3d_bullet_perform_collision (Hkl3DBullet *self, Hkl3DConfig *config)
{
	int numManifolds;
	Hkl3DObject **object;
	Hkl3DBulletObject **bobject;

	self->_btWorld->performDiscreteCollisionDetection();
	self->_btWorld->updateAabbs();

	numManifolds = self->_btDispatcher->getNumManifolds();

	fprintf (stdout, "numManifolds=%d\n", numManifolds);

	/* update Hkl3DObject collision from manifolds */
	darray_foreach(bobject, self->bobjects){
			int is_colliding = FALSE;

			for(int k=0; k<numManifolds; ++k){
				btPersistentManifold *manifold = self->_btDispatcher->getManifoldByIndexInternal(k);
				is_colliding |= (*bobject)->btObject == manifold->getBody0();
				is_colliding |= (*bobject)->btObject == manifold->getBody1();
			}

			darray_foreach (object, (*bobject)->axis->objects) {
				(*object)->is_colliding = is_colliding;
			}
	}

	// hkl3d_bullet_fprintf(stdout, self);

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
	int i = 0;
	Hkl3DBulletObject **bobject;

	fprintf(f, "Hkl3DBullet (");
	fprintf(f, "btCollisionConfiguration=%p", self->_btCollisionConfiguration);
	fprintf(f, ", btBroadphase=%p", self->_btBroadphase);
	fprintf(f, ", btWorld=%p", self->_btWorld);
	fprintf(f, ", btDispatcher=%p)", self->_btDispatcher);
	fprintf(f, ", bobjects=[");
	darray_foreach(bobject, self->bobjects){
		if (i++)
			fprintf(f, ", ");
		hkl3d_bullet_object_fprintf(f, *bobject);
	}
	fprintf(f , "])\n");

}
