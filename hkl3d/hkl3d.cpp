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
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <assimp/cexport.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/SceneCombiner.h>
#include <libgen.h>
#include <yaml.h>

#include "hkl3d-private.h"
#include "hkl-geometry-private.h"

#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

/***************/
/* Hkl3DObject */
/***************/

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


static Hkl3DObject *hkl3d_object_new(Hkl3DModel *model, unsigned int mesh)
{
	int i;
	GSList *faces;
	Hkl3DObject *self = nullptr;

	self = g_new0 (Hkl3DObject, 1);

	/* fill the hkl3d object structure. */
	self->added = false;
	self->axis = nullptr;
	self->draw_aabb = false;
	self->hide = false;
	self->is_colliding = false;
	self->mesh = mesh;
	self->model = model;
	self->movable = false;
	self->selected = false;
	self->transformation = GLMS_MAT4_IDENTITY; /* todo importe from the node */

	/* bullet */
	self->meshes = trimesh_from_aiMesh(model->scene->mMeshes[self->mesh]);
	self->btShape = shape_from_trimesh(self->meshes, false);
	self->btObject = btObject_from_shape(self->btShape);

	/* OpenGL */
	self->vao = 0;

	return self;
}

static void hkl3d_object_free(Hkl3DObject *self)
{
	if(!self)
		return;

	if(self->btObject){
		delete self->btObject;
		self->btObject = nullptr;
	}
	if(self->btShape){
		delete self->btShape;
		self->btShape = nullptr;
	}
	if(self->meshes){
		delete self->meshes;
		self->meshes = nullptr;
	}
	if(self->axis_name){
		free(self->axis_name);
		self->axis_name = nullptr;
	}

	free(self);
}

static void hkl3d_object_set_movable(Hkl3DObject *self, int movable)
{
	if(!self)
		return;

	if(self->movable != movable){
		self->movable = movable;
		delete self->btObject;
		delete self->btShape;
		self->btShape = shape_from_trimesh(self->meshes, movable);
		self->btObject = btObject_from_shape(self->btShape);
	}
}

void hkl3d_object_axis_name_set(Hkl3DObject *self, const char *name)
{
	g_return_if_fail (NULL != self);
	g_return_if_fail (NULL != name);

	/* TODO ugly... */
	if (name != self->axis_name){
		g_clear_pointer(&self->axis_name, g_free);
		self->axis_name = g_strdup(name);
	}
}

static void matrix_fprintf(FILE *f, const float matrix[])
{
	for(uint i=0; i<4; ++i){
		fprintf(f, "\n ");
		for(uint j=0; j<4; ++j){
			fprintf(f, " %6.3f", matrix[4 * i + j]);
		}
	}
}

void hkl3d_object_aabb_get(const Hkl3DObject *self, float from[3], float to[3])
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

void hkl3d_object_draw_aabb_set(Hkl3DObject *self, bool draw_aabb)
{
	self->draw_aabb = draw_aabb;
}

void hkl3d_object_transformation_set(Hkl3DObject *self, mat4s transformation)
{
	g_return_if_fail (NULL != self);

	self->transformation = transformation;
}

bool hkl3d_object_hide_get(const Hkl3DObject *self)
{
	return self->hide;
}

void hkl3d_object_hide_set(Hkl3DObject *self, bool hide)
{
	g_return_if_fail (nullptr != self);

	self->hide = hide;
}

void hkl3d_object_fprintf(FILE *f, const Hkl3DObject *self)
{
	GSList *faces;

	fprintf(f, "Hkl3dObject (", self);
	fprintf(f, "axis_name=%s, ", self->axis_name);
	fprintf(f, "axis=%p, ", self->axis);
	fprintf(f, "model=%p, ", self->model);
	fprintf(f, "mesh=%d, ", self->mesh);
	fprintf(f, "transformation=[TODO], ");
	/* glms_mat4_print (self->transformation, f); */
	fprintf(f, "btObject=%p, ", self->btObject);
	fprintf(f, "btShape=%p, ", self->btShape);
	fprintf(f, "meshes=%p, ", self->meshes);
	fprintf(f, "is_colliding=%d, ", self->is_colliding);
	fprintf(f, "hide=%d, ", self->hide);
	fprintf(f, "added=%d, ", self->added);
	fprintf(f, "selected=%d)", self->selected);
}

/**************/
/* Hkl3DModel */
/**************/

static Hkl3DModel *hkl3d_model_new(const char *filename)
{
	g_return_val_if_fail (NULL != filename, nullptr);

	Hkl3DModel *self = nullptr;

	self = g_new0 (Hkl3DModel, 1);

	self->filename = strdup(filename); /* TODO load the scene from here */
	self->scene = nullptr;
	darray_init(self->objects);

	return self;
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

/*
 * Initialize the bullet collision environment.
 * create the Hkl3DObjects
 * create the Hkl3DConfig
 */
static Hkl3DModel *hkl3d_model_new_from_file(const char *filename)
{
	unsigned int i;
	Hkl3DModel *self = nullptr;
	const struct aiScene *scene;

	g_return_val_if_fail(NULL != filename, nullptr);

	scene = aiImportFile(filename,
			     aiProcess_CalcTangentSpace
			     | aiProcess_Triangulate
			     | aiProcess_JoinIdenticalVertices
			     | aiProcess_SortByPType
			     | aiProcess_PreTransformVertices);

	if (nullptr == scene) {
		fprintf(stdout, "\n%s", aiGetErrorString());
		goto out;
	}

	//aiExportScene(scene, "gltf2", "test.gltf", 0);

	self = hkl3d_model_new(filename);
	if (nullptr == self)
		goto out;

	self->scene = scene;

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

static Hkl3DAxis *hkl3d_axis_new(void)
{
	Hkl3DAxis *self = nullptr;

	self = g_new0 (Hkl3DAxis, 1);

	self->objects = nullptr; /* do not own the objects */
	self->len = 0;

	return self;
}

static void hkl3d_axis_free(Hkl3DAxis *self)
{
	if(!self)
		return;

	free(self->objects);
	free(self);
}

/* should be optimized (useless if the Hkl3DObject had a connection with the Hkl3DAxis */
static void hkl3d_axis_attach_object(Hkl3DAxis *self, Hkl3DObject *object)
{
	object->axis = self;
	self->objects = (Hkl3DObject **)realloc(self->objects, sizeof(*self->objects) * (self->len + 1));
	self->objects[self->len++] = object;
}

/* should be optimized (useless if the Hkl3DObject had a connection with the Hkl3DAxis */
static void hkl3d_axis_detach_object(Hkl3DAxis *self, Hkl3DObject *object)
{
	size_t i;

	if(!self || !object)
		return;

	if(self != object->axis)
		return;

	/* find the index of the object in the object list */
	for(i=0; self->objects[i] != object; ++i);

	object->axis = nullptr;
	/* move all above objects of 1 position */
	self->len--;
	if(i < self->len)
		memmove(object, object+1, sizeof(object) * (self->len - i));
}

void hkl3d_axis_fprintf(FILE *f, const Hkl3DAxis *self)
{
	g_return_if_fail (nullptr != f);
	g_return_if_fail (nullptr != self);

	fprintf(f, "Hkl3DAxis (objects=[");
	for(size_t i=0; i<self->len; ++i){
		if(i) fprintf (f, ", ");
		fprintf (f, "%p", self->objects[i]);
	}
	fprintf(f, "])");
}

/*****************/
/* Hkl3DGeometry */
/*****************/

static Hkl3DGeometry *hkl3d_geometry_new(HklGeometry *geometry)
{
	uint i;
	Hkl3DGeometry *self = nullptr;

	self = g_new0 (Hkl3DGeometry, 1);

	self->geometry = geometry;
	self->axes = g_new0(Hkl3DAxis*, darray_size(geometry->axes));

	for(i=0; i<darray_size(geometry->axes); ++i)
		self->axes[i] = hkl3d_axis_new();

	return self;
}

static void hkl3d_geometry_free(Hkl3DGeometry *self)
{
	uint i;

	if(!self)
		return;

	for(i=0; i<darray_size(self->geometry->axes); ++i)
		hkl3d_axis_free(self->axes[i]);
	free(self->axes);
	free(self);
}

static void hkl3d_geometry_apply_transformations(Hkl3DGeometry *self)
{
	HklHolder **holder;

	darray_foreach(holder, self->geometry->holders){
		size_t j;
		btQuaternion btQ(0, 0, 0, 1);

		size_t len = (*holder)->config->len;
		for(j=0; j<len; j++){
			size_t k;
			size_t idx = (*holder)->config->idx[j];
			const HklQuaternion *q = hkl_parameter_quaternion_get(darray_item(self->geometry->axes, idx));
			float G3DM[16];

			/* conversion beetween hkl -> bullet coordinates */
			btQ *= btQuaternion(q->data[1],
					    q->data[2],
					    q->data[3],
					    q->data[0]);

			/* move each object connected to that hkl Axis. */
			/* apply the quaternion transformation to the bullet object */
			/* use the bullet library to compute the OpenGL matrix */
			/* apply this matrix to the G3DObject for the visualisation */
			for(k=0; k<self->axes[idx]->len; ++k){
				self->axes[idx]->objects[k]->btObject->getWorldTransform().setRotation(btQ);
				self->axes[idx]->objects[k]->btObject->getWorldTransform().getOpenGLMatrix( G3DM );
				memcpy(self->axes[idx]->objects[k]->transformation.raw, &G3DM[0], sizeof(G3DM));
			}
		}
	}
}

void hkl3d_geometry_fprintf(FILE *f, const Hkl3DGeometry *self)
{
	g_return_if_fail (NULL != f);
	g_return_if_fail (NULL != self);

	fprintf(f, "Hkl3DGeometry (geometry=%p, axes=[", self->geometry);
	// TODO hkl_geometry_fprintf(f, self->geometry);
	for(size_t i=0; i<darray_size(self->geometry->axes); ++i){
		if(i) fprintf(f, ", ");
		hkl3d_axis_fprintf(f, self->axes[i]);
	}
	fprintf(f, "])");
}

static Hkl3DAxis *hkl3d_geometry_axis_get(Hkl3DGeometry *self, const char *name)
{
	for(size_t i=0; i<darray_size(self->geometry->axes); ++i){
		if (!strcmp(hkl_parameter_name_get(darray_item(self->geometry->axes, i)),
			    name))
			return self->axes[i];
	}
	return nullptr;
}

/*********/
/* HKL3D */
/*********/

static void hkl3d_apply_transformations(Hkl3D *self)
{
	struct timeval debut, fin;

	/* set the right transformation of each objects and get numbers */
	gettimeofday(&debut, nullptr);
	hkl3d_geometry_apply_transformations(self->geometry);
	gettimeofday(&fin, nullptr);
	timersub(&fin, &debut, &self->stats.transformation);
}

void hkl3d_connect_all_axes(Hkl3D *self)
{
	Hkl3DModel **model;
	Hkl3DObject **object;

	/* connect use the axes names */
	darray_foreach(model, self->config->models){
		darray_foreach(object, (*model)->objects){
			hkl3d_connect_object_to_axis(self, *object, (*object)->axis_name);
		}
	}
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

	self->geometry = hkl3d_geometry_new(geometry);
	self->config = hkl3d_config_new(filename);
	hkl3d_load_config(self, filename);

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

	hkl3d_connect_all_axes(self);

	return self;
}

void hkl3d_free(Hkl3D *self)
{
	Hkl3DModel **model;
	Hkl3DObject **object;

	/* remove all objects from the collision world */
	darray_foreach(model, self->config->models){
		darray_foreach(object, (*model)->objects){
			if((*object)->added)
				self->_btWorld->removeCollisionObject((*object)->btObject);
		}
	}

	hkl3d_geometry_free(self->geometry);
	hkl3d_config_free(self->config);

	if (self->_btWorld)
		delete self->_btWorld;
	if (self->_btBroadphase)
		delete self->_btBroadphase;
	if (self->_btDispatcher)
		delete self->_btDispatcher;
	if (self->_btCollisionConfiguration)
		delete self->_btCollisionConfiguration;

	free(self);
}

Hkl3DModel *hkl3d_add_model_from_file(Hkl3D *self,
				      const char *filename, const char *directory)
{
	int current;
	int res;
	const aiScene *scene = nullptr;
	Hkl3DModel *model = nullptr;

	/* first set the current directory using the directory
	 * parameter. Maybe using openat should be a better solution
	 * in the hkl3d_model_new_from_file */
	current = open(".", O_RDONLY);
	if (current < 0)
		return nullptr;
	res = chdir(directory);
	if(res < 0)
		goto close_current;

	model = hkl3d_model_new_from_file(filename);
	if(model)
		darray_append(self->config->models, model);

	/* restore the current directory */
	res = fchdir(current);

	return model;

close_current:
	close(current);
	return nullptr;
}

/* check that the axis name is really available in the Geometry */
/* if axis name not valid make the object static object->name = nullptr */
/* ok so check if the axis was already connected  or not */
/* if already connected check if it was a different axis do the job */
/* if not yet connected do the job */
/* fill movingCollisionObject and movingG3DObjects vectors for transformations */
void hkl3d_connect_object_to_axis(Hkl3D *self, Hkl3DObject *object, const char *name)
{
	Hkl3DAxis *axis3d = hkl3d_geometry_axis_get(self->geometry, name);
	if (!object->movable){
		if(axis3d){ /* static -> movable */
			self->_btWorld->removeCollisionObject(object->btObject);
			hkl3d_object_set_movable(object, true);
			self->_btWorld->addCollisionObject(object->btObject);
			object->added = true;
			hkl3d_axis_attach_object(axis3d, object);
		}
	}else{
		if(!axis3d){ /* movable -> static */
			self->_btWorld->removeCollisionObject(object->btObject);
			hkl3d_object_set_movable(object, false);
			self->_btWorld->addCollisionObject(object->btObject);
			object->added = true;
		}else{ /* movable -> movable */
			if(strcmp(object->axis_name, name)){ /* not the same axis */
				hkl3d_axis_detach_object(object->axis, object);
				hkl3d_axis_attach_object(axis3d, object);
			}
		}
	}
	hkl3d_object_axis_name_set(object, name);
}

/**
 * Hkl3D::hide_object:
 *
 * update the visibility of an Hkl3DObject in the bullet world
 * add or remove the object from the _btWorld depending on the hide
 * member of the object.
 **/
void hkl3d_hide_object(Hkl3D *self, Hkl3DObject *object, int hide)
{
	object->hide = hide;

	if (true == object->hide){
		if (true == object->added){
			self->_btWorld->removeCollisionObject(object->btObject);
			object->added = false;
		}
	}else{
		if (false == object->added){
			self->_btWorld->addCollisionObject(object->btObject);
			object->added = true;
		}
	}
}

/* remove an object from the model */
void hkl3d_remove_object(Hkl3D *self, Hkl3DObject *object)
{
	hkl3d_hide_object(self, object, TRUE);
	hkl3d_axis_detach_object(object->axis, object);
}


int hkl3d_is_colliding(Hkl3D *self)
{
	int numManifolds;
	struct timeval debut, fin;
	Hkl3DModel **model;
	Hkl3DObject **object;

	/* apply geometry transformation */
	hkl3d_apply_transformations(self);

	/* perform the collision detection and get numbers */
	gettimeofday(&debut, nullptr);
	if(self->_btWorld){
		self->_btWorld->performDiscreteCollisionDetection();
		self->_btWorld->updateAabbs();
	}
	gettimeofday(&fin, nullptr);
	timersub(&fin, &debut, &self->stats.collision);

	numManifolds = self->_btDispatcher->getNumManifolds();

	/* update Hkl3DObject collision from manifolds */
	darray_foreach(model, self->config->models){
		darray_foreach(object, (*model)->objects){
			(*object)->is_colliding = FALSE;
			for(int k=0; k<numManifolds; ++k){
				btPersistentManifold *manifold = self->_btDispatcher->getManifoldByIndexInternal(k);
				(*object)->is_colliding |= (*object)->btObject == manifold->getBody0();
				(*object)->is_colliding |= (*object)->btObject == manifold->getBody1();
			}
		}
	}

	return numManifolds != 0;
}

/**
 * Hkl3D::get_bounding_boxes:
 * @min:
 * @max:
 *
 * get the bounding boxes of the current world from the bullet internals.
 **/
void hkl3d_get_bounding_boxes(Hkl3D *self,
			      struct btVector3 *min, struct btVector3 *max)
{
	self->_btWorld->getBroadphase()->getBroadphaseAabb(*min, *max);
}

int hkl3d_get_nb_manifolds(Hkl3D *self)
{
	return self->_btDispatcher->getNumManifolds();
}

int hkl3d_get_nb_contacts(Hkl3D *self, int manifold)
{
	return self->_btDispatcher->getManifoldByIndexInternal(manifold)->getNumContacts();
}

void hkl3d_get_collision_coordinates(Hkl3D *self, int manifold, int contact,
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

bool hkl3d_contains_model(const Hkl3D *self, const char *filename)
{
	g_return_val_if_fail(nullptr != self, false);
	g_return_val_if_fail(nullptr != filename, false);

	Hkl3DModel **model;

	darray_foreach(model, self->config->models){
		if (0 == strcmp(filename, hkl3d_model_filename_get(*model)))
			return true;
	}

	return false;
}

Hkl3DObject *
hkl3d_get_object_by_id(const Hkl3D *self, const char *filename, int id)
{
	g_return_val_if_fail(nullptr != self, nullptr);
	g_return_val_if_fail(nullptr != filename, nullptr);

	Hkl3DModel **model;

	darray_foreach (model, self->config->models){
		if (id < darray_size((*model)->objects)
		    && 0 == strcmp((*model)->filename, filename)){
			return darray_item((*model)->objects, id);
		}
	}

	return nullptr;
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
	fprintf(f, ", _btCollisionConfiguration=%p", self->_btCollisionConfiguration);
	fprintf(f, ", _btBroadphase=%p", self->_btBroadphase);
	fprintf(f, ", _btWorld=%p", self->_btWorld);
	fprintf(f, ", _btDispatcher=%p)", self->_btDispatcher);
}
