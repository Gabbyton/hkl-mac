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
 * Copyright (C) 2003-2019, 2022, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 *	    Oussama Sboui <oussama.sboui@synchrotron-soleil.fr>
 */
#include <assimp/cexport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cglm/struct.h>
#include <epoxy/gl.h>

#include "hkl3d.h"
#include "hkl/ccan/compiler/compiler.h"
#include "hkl-gui.h"
#include "hkl-gui-macros.h"
#include "hkl-gui-3d.h"


/************/
/* HklGui3d */
/************/

enum {
	PROP_0,

	PROP_FILENAME,
	PROP_GEOMETRY,
	PROP_AABB,

	N_PROPERTIES
};

/* Keep a pointer to the properties definition */
static GParamSpec *props[N_PROPERTIES] = { NULL, };

enum {
	CHANGED,

	N_SIGNALS
};

static NEEDED guint signals[N_SIGNALS] = { 0 };

typedef struct _HklGui3DPrivate HklGui3DPrivate;

struct _HklGui3DPrivate {
	/* Properties */
	char *filename;
	HklGeometry *geometry;
	/* Properties */

	GtkBuilder *builder;

	GtkFrame *frame1;
	GtkBox *vbox1;
	GtkTreeView *treeview1;
	/* GtkToolButton *toolbutton1; */
	/* GtkToolButton *toolbutton2; */
	/* GtkToolButton *toolbutton3; */
	GtkFileChooserDialog *filechooserdialog1;
	GtkButton *button1;
	GtkButton *button2;
	GtkTreeStore *treestore1;

	/* opengl connected to the drawingarea1 */
	struct {
		gint32 beginx;
		gint32 beginy;
	} mouse;
	gboolean aabb;
};

struct _HklGui3D {
	GObject parent_instance;
	HklGui3DPrivate * priv;
	Hkl3D *hkl3d;

	GtkGLArea *gl_area;
	GtkWidget *toggle_button_aabb;
};

G_DEFINE_TYPE_WITH_PRIVATE (HklGui3D, hkl_gui_3d, G_TYPE_OBJECT);

/* static void hkl_gui_3d_update_hkl3d_objects_TreeStore(HklGui3D *self) */
/* { */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self); */
/* 	size_t i; */
/* 	size_t j; */

/* 	gtk_tree_store_clear(priv->treestore1); */

/* 	for(i=0; i<self->hkl3d->config->len; ++i){ */
/* 		GtkTreeIter iter = {0}; */

/* 		gtk_tree_store_append(priv->treestore1, &iter, NULL); */
/* 		gtk_tree_store_set(priv->treestore1, &iter, */
/* 				   HKL_GUI_3D_COL_NAME, self->hkl3d->config->models[i]->filename, */
/* 				   HKL_GUI_3D_COL_MODEL, self->hkl3d->config->models[i], */
/* 				   HKL_GUI_3D_COL_OBJECT, NULL, */
/* 				   -1); */

/* 		for(j=0; j<self->hkl3d->config->models[i]->len; ++j){ */
/* 			GtkTreeIter citer = {0}; */

/* 			gtk_tree_store_append(priv->treestore1, &citer, &iter); */
/* 			gtk_tree_store_set(priv->treestore1, &citer, */
/* 					   HKL_GUI_3D_COL_NAME, self->hkl3d->config->models[i]->objects[j]->axis_name, */
/* 					   HKL_GUI_3D_COL_HIDE, self->hkl3d->config->models[i]->objects[j]->hide, */
/* 					   HKL_GUI_3D_COL_MODEL, self->hkl3d->config->models[i], */
/* 					   HKL_GUI_3D_COL_OBJECT, self->hkl3d->config->models[i]->objects[j], */
/* 					   -1); */
/* 		} */
/* 	} */
/* } */

/* properties */

static const char *
hkl_gui_3d_get_filename(HklGui3D *self)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	return priv->filename;
}

static HklGeometry *
hkl_gui_3d_get_geometry(HklGui3D *self)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	return priv->geometry;
}

static gboolean
hkl_gui_3d_get_aabb(HklGui3D *self)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	return priv->aabb;
}

static void _filename_and_geometry(HklGui3D *self)
{

	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);
	if(priv->filename && priv->geometry){
		if (self->hkl3d)
			hkl3d_free(self->hkl3d);
		self->hkl3d = hkl3d_new(priv->filename, priv->geometry);
	}
}

static void
hkl_gui_3d_set_filename(HklGui3D *self, const char *filename)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	if(priv->filename)
		g_free(priv->filename);
	priv->filename = g_strdup(filename);

	_filename_and_geometry(self);
	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FILENAME]);
}

static void
hkl_gui_3d_set_geometry(HklGui3D *self, HklGeometry *geometry)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	priv->geometry = geometry;
	_filename_and_geometry(self);
	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_GEOMETRY]);
}

static void
hkl_gui_3d_set_aabb(HklGui3D *self, gboolean aabb)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	g_return_if_fail(aabb != priv->aabb);

	priv->aabb = aabb;
	hkl3d_gl_draw_aabb_set(self->hkl3d, aabb);

	hkl_gui_3d_invalidate(self);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_AABB]);
}

static void
set_property (GObject *object, guint prop_id,
	      const GValue *value, GParamSpec *pspec)
{
	HklGui3D *self = HKL_GUI_3D (object);

	switch (prop_id) {
	case PROP_FILENAME:
		hkl_gui_3d_set_filename(self, g_value_get_string (value));
		break;
	case PROP_GEOMETRY:
		hkl_gui_3d_set_geometry(self, g_value_get_pointer (value));
		break;
	case PROP_AABB:
		hkl_gui_3d_set_aabb(self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
get_property (GObject *object, guint prop_id,
	      GValue *value, GParamSpec *pspec)
{
	HklGui3D *self = HKL_GUI_3D (object);

	switch (prop_id)
	{
	case PROP_FILENAME:
		g_value_set_string (value, hkl_gui_3d_get_filename (self));
		break;
	case PROP_GEOMETRY:
		g_value_set_pointer (value, hkl_gui_3d_get_geometry (self));
		break;
	case PROP_AABB:
		g_value_set_boolean (value, hkl_gui_3d_get_aabb (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
finalize (GObject* object)
{
	HklGui3D *self = HKL_GUI_3D (object);
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(HKL_GUI_3D(self));

	g_free(priv->filename);

	g_object_unref(priv->builder);

	hkl3d_free(self->hkl3d);

	G_OBJECT_CLASS (hkl_gui_3d_parent_class)->finalize (object);
}

HklGui3D*
hkl_gui_3d_new (const char *filename, HklGeometry *geometry)
{
	return g_object_new (HKL_GUI_TYPE_3D,
			     "filename", filename,
			     "geometry", geometry,
			     NULL);
}

GtkFrame *hkl_gui_3d_get_frame(HklGui3D *self)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);

	return priv->frame1;
}

void hkl_gui_3d_update(HklGui3D *self)
{
	if(self->hkl3d){
		hkl3d_is_colliding(self->hkl3d);
		hkl_gui_3d_invalidate(self);
	}
}

/************/
/* Callback */
/************/

/* void hkl_gui_3d_cellrenderertext2_toggled_cb(GtkCellRendererToggle* renderer, */
/* 					     const gchar* path, gpointer user_data) */
/* { */
/* 	HklGui3D *self = HKL_GUI_3D(user_data); */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */
/* 	guint hide; */
/* 	Hkl3DObject *object; */
/* 	GtkTreeIter iter = {0}; */


/* 	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL(priv->treestore1), */
/* 					     &iter, path); */
/* 	gtk_tree_model_get (GTK_TREE_MODEL(priv->treestore1), */
/* 			    &iter, */
/* 			    HKL_GUI_3D_COL_OBJECT, &object, */
/* 			    -1); */

/* 	hide = !gtk_cell_renderer_toggle_get_active(renderer); */

/* 	if(object){ */
/* 		hkl3d_hide_object(self->hkl3d, object, hide); */
/* 		gtk_tree_store_set (priv->treestore1, */
/* 				    &iter, */
/* 				    HKL_GUI_3D_COL_HIDE, hide, */
/* 				    -1); */
/* 		hkl_gui_3d_is_colliding(self); */
/* 		hkl_gui_3d_invalidate(self); */
/* 	}else{ */
/* 		Hkl3DModel *model; */

/* 		gtk_tree_model_get (GTK_TREE_MODEL(priv->treestore1), */
/* 				    &iter, */
/* 				    HKL_GUI_3D_COL_MODEL, &model, */
/* 				    -1); */
/* 		if(model){ */
/* 			GtkTreeIter children = {0}; */
/* 			gboolean valid; */
/* 			size_t i = 0; */

/* 			gtk_tree_store_set (priv->treestore1, */
/* 					    &iter, */
/* 					    HKL_GUI_3D_COL_HIDE, hide, */
/* 					    -1); */

/* 			/\* set all the children rows *\/ */
/* 			valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(priv->treestore1), */
/* 							     &children, &iter); */
/* 			while(valid){ */
/* 				hkl3d_hide_object(self->hkl3d, model->objects[i++], hide); */
/* 				gtk_tree_store_set (priv->treestore1, */
/* 						    &children, */
/* 						    HKL_GUI_3D_COL_HIDE, hide, */
/* 						    -1); */

/* 				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->treestore1), &children); */
/* 			} */
/* 			hkl_gui_3d_is_colliding(self); */
/* 			hkl_gui_3d_invalidate(self); */
/* 		} */
/* 	} */
/* } */

/* void hkl_gui_3d_treeview1_cursor_changed_cb(GtkTreeView *tree_view, */
/* 					    gpointer user_data) */
/* { */
/* 	int i; */
/* 	int j; */
/* 	HklGui3D *self = user_data; */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */
/* 	GtkTreeIter iter = {0}; */
/* 	Hkl3DObject *object; */
/* 	GtkTreePath *path; */
/* 	GtkTreeViewColumn * column; */

/* 	gtk_tree_view_get_cursor(priv->treeview1, &path, &column); */
/* 	gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->treestore1), &iter, path); */
/* 	gtk_tree_path_free(path); */

/* 	/\* need to unselect of objects of all 3d models *\/ */
/* 	for(i=0; i<self->hkl3d->config->len; ++i) */
/* 		for(j=0; j<self->hkl3d->config->models[i]->len; ++j) */
/* 			self->hkl3d->config->models[i]->objects[j]->selected = FALSE; */

/* 	/\* now select the right object *\/ */
/* 	gtk_tree_model_get (GTK_TREE_MODEL(priv->treestore1), */
/* 			    &iter, */
/* 			    HKL_GUI_3D_COL_OBJECT, &object, */
/* 			    -1); */
/* 	if(object) */
/* 		object->selected = TRUE; */

/* 	hkl_gui_3d_invalidate(self); */
/* } */

/* void hkl_gui_3d_toolbutton1_clicked_cb(GtkToolButton *toolbutton, */
/* 				       gpointer user_data) */
/* { */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */

/* 	gtk_widget_show(GTK_WIDGET(priv->filechooserdialog1)); */
/* } */

/* /\* remove an object from the model *\/ */
/* void hkl_gui_3d_toolbutton2_clicked_cb(GtkToolButton *toolbutton, */
/* 				       gpointer user_data) */
/* { */
/* 	HklGui3D *self = user_data; */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */
/* 	GtkTreeIter iter = {0}; */
/* 	GtkTreePath *path; */
/* 	GtkTreeViewColumn * column; */
/* 	Hkl3DObject *object; */

/* 	gtk_tree_view_get_cursor(priv->treeview1, &path, &column); */
/* 	gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->treestore1), &iter, path); */
/* 	gtk_tree_path_free(path); */

/* 	gtk_tree_model_get (GTK_TREE_MODEL(priv->treestore1), */
/* 			    &iter, */
/* 			    HKL_GUI_3D_COL_OBJECT, &object, */
/* 			    -1); */
/* 	if(object){ */
/* 		hkl3d_remove_object(self->hkl3d, object); */
/* 		hkl_gui_3d_update_hkl3d_objects_TreeStore(self); */
/* 		hkl_gui_3d_invalidate(self); */
/* 	} */
/* } */

/* static void */
/* reset_3d(G3DGLRenderOptions *renderoptions) */
/* { */
/* 	/\* renderoptions *\/ */
/* 	renderoptions->updated = TRUE; */
/* 	renderoptions->initialized = FALSE; */
/*         renderoptions->zoom = 7; */
/*         renderoptions->bgcolor[0] = 0.9; */
/*         renderoptions->bgcolor[1] = 0.8; */
/*         renderoptions->bgcolor[2] = 0.6; */
/*         renderoptions->bgcolor[3] = 1.0; */
/* 	renderoptions->glflags = */
/* 		/\* G3D_FLAG_GL_ISOMETRIC | *\/ */
/* 		G3D_FLAG_GL_SPECULAR | */
/* 		G3D_FLAG_GL_SHININESS | */
/* 		G3D_FLAG_GL_TEXTURES | */
/* 		G3D_FLAG_GL_COLORS| */
/* 		G3D_FLAG_GL_COORD_AXES; */

/*         g3d_quat_trackball(renderoptions->quat, 0.0, 0.0, 0.0, 0.0, 0.8); */

/* 	/\* rotate a little bit *\/ */
/* 	gfloat q1[4], q2[4]; */
/* 	gfloat a1[3] = { 0.0, 1.0, 0.0 }; */
/* 	gfloat a2[3] = { 1.0, 0.0, 1.0 }; */

/* 	g3d_quat_rotate(q1, a1, - 45.0 * G_PI / 180.0); */
/* 	g3d_quat_rotate(q2, a2, - 45.0 * G_PI / 180.0); */
/* 	g3d_quat_add(renderoptions->quat, q1, q2); */
/* } */


/***************/
/* OpenGL part */
/***************/

/* void hkl_gui_3d_draw_selected(HklGui3D *self) */
/* { */
/* 	int i; */
/* 	int j; */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self); */

/* 	/\* glDisable(GL_LIGHTING); *\/ */

/* 	for(i=0; i<self->hkl3d->config->len; i++) */
/* 		for(j=0; j<self->hkl3d->config->models[i]->len; j++){ */
/* 			if(self->hkl3d->config->models[i]->objects[j]->selected */
/* 			   && !self->hkl3d->config->models[i]->objects[j]->hide){ */
/* 				// Push the GL attribute bits so that we don't wreck any settings */
/* 				glPushAttrib( GL_ALL_ATTRIB_BITS ); */

/* 				// Enable polygon offsets, and offset filled polygons forward by 2.5 */
/* 				glEnable( GL_POLYGON_OFFSET_FILL ); */
/* 				glPolygonOffset( -2.5, -2.5); */

/* 				// Set the render mode to be line rendering with a thick line width */
/* 				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); */
/* 				glLineWidth( 3.f ); */
/* 				// Set the colour to be pink */
/* 				glColor3f( 1.f, .0f, 1.f ); */
/* 				// Render the object */
/* 				draw_g3dObject(self->hkl3d->config->models[i]->objects[j]->g3d); */
/* 				// Set the polygon mode to be filled triangles */
/* 				glLineWidth( 1.f ); */
/* 				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); */
/* 				// Set the colour to the background */
/* 				glCullFace(GL_FRONT); */
/* 				glColor3f( 0.0f, 0.0f, 0.0f ); */
/* 				// Render the object */
/* 				draw_g3dObject(self->hkl3d->config->models[i]->objects[j]->g3d); */

/* 				// Pop the state changes off the attribute */
/* 				// to set things back how they were */
/* 				glPopAttrib(); */
/* 			} */
/* 		} */
/* 	/\* glEnable(GL_LIGHTING); *\/ */
/* } */

/* static void draw_sphere(float radius, int lats, int longs) */
/* { */
/* 	int i, j; */
/* 	for(i=0;i<=lats;i++){ */
/* 		float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats); */
/* 		float z0  = radius * sin(lat0); */
/* 		float zr0 =  radius * cos(lat0); */

/* 		float lat1 = M_PI * (-0.5 + (float) i / lats); */
/* 		float z1 = radius * sin(lat1); */
/* 		float zr1 = radius * cos(lat1); */

/* 		glBegin(GL_QUAD_STRIP); */
/* 		for(j=0;j<=longs;j++) { */
/* 			float lng = 2 * M_PI * (float) (j - 1) / longs; */
/* 			float x = cos(lng); */
/* 			float y = sin(lng); */

/* 			glNormal3f(x * zr1, y * zr1, z1); */
/* 			glVertex3f(x * zr1, y * zr1, z1); */
/* 			glNormal3f(x * zr0, y * zr0, z0); */
/* 			glVertex3f(x * zr0, y * zr0, z0); */
/* 		} */
/* 		glEnd(); */
/* 	} */
/* } */

/* void hkl_gui_3d_draw_collisions(HklGui3D *self) */
/* { */
/* 	int i; */
/* 	int numManifolds; */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self); */

/* 	/\* glDisable(GL_LIGHTING); *\/ */
/* 	///one way to draw all the contact points is iterating over contact manifolds / points: */
/* 	numManifolds = hkl3d_get_nb_manifolds(self->hkl3d); */
/* 	for (i=0; i<numManifolds; i++){ */
/* 		int numContacts; */
/* 		int j; */

/* 		// now draw the manifolds / points */
/* 		numContacts = hkl3d_get_nb_contacts(self->hkl3d, i); */
/* 		for (j=0; j<numContacts; j++){ */
/* 			double xa, ya, za; */
/* 			double xb, yb, zb; */

/* 			hkl3d_get_collision_coordinates(self->hkl3d, i, j, */
/* 							&xa, &ya, &za, &xb, &yb, &zb); */

/* 			glDisable(GL_DEPTH_TEST); */
/* 			glBegin(GL_LINES); */
/* 			glColor4f(0, 0, 0, 1); */
/* 			glVertex3d(xa, ya, za); */
/* 			glVertex3d(xb, yb, zb); */
/* 			glEnd(); */
/* 			glColor4f(1, 0, 0, 1); */
/* 			glPushMatrix(); */
/* 			glTranslatef (xb, yb, zb); */
/* 			glScaled(0.05,0.05,0.05); */

/* 			draw_sphere(1, 10, 10); */

/* 			glPopMatrix(); */
/* 			glColor4f(1, 1, 0, 1); */
/* 			glPushMatrix(); */
/* 			glTranslatef (xa, ya, za); */
/* 			glScaled(0.05,0.05,0.05); */

/* 			draw_sphere(1, 10, 10); */

/* 			glPopMatrix(); */
/* 			glEnable(GL_DEPTH_TEST); */
/* 		} */
/* 	} */
/* 	glFlush(); */
/* } */


static gboolean
hkl_gui_3d_gl_area_render_cb(GtkGLArea *area,
			     GdkGLContext *context,
			     gpointer user_data)
{
	HklGui3D *self = HKL_GUI_3D (user_data);

	gtk_gl_area_make_current (area);

	if (gtk_gl_area_get_error (area) != NULL)
		return FALSE;


	/* Clear the viewport */
	glClearColor (0.9, 0.8, 0.6, 1.0); /* black */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw scenes */
	hkl3d_gl_draw_models(self->hkl3d);

	/* Flush the contents of the pipeline */
	glFlush ();

	return TRUE;
}

static gboolean
hkl_gui_3d_gl_area_resize_cb(GtkGLArea *area,
			     gint width,
			     gint height,
			     gpointer user_data)
{
	HklGui3D *self = HKL_GUI_3D (user_data);

	gtk_gl_area_make_current (area);

	if (gtk_gl_area_get_error (area) != NULL)
		return FALSE;

	hkl3d_gl_resize(self->hkl3d, width, height);

	return true;
}

void hkl_gui_3d_invalidate(HklGui3D *self)
{
	/* queue a redraw on the GtkGLArea */

	/* hkl_gui_3d_gl_area_render_cb(self->gl_area, */
	/* 			     gtk_gl_area_get_context (self->gl_area), */
	/* 			     self); */
	/* apply the transformation */

	gtk_gl_area_queue_render (self->gl_area);
}


/* static gboolean */
/* hkl_gui_3d_gl_area_button_press_event_cb(GtkWidget *gl_area, */
/* 					 GdkEventButton* event, */
/* 					 gpointer user_data) */
/* { */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */

/* 	/\* left mouse buttom: rotate object *\/ */
/* 	if(event->button == 1) */
/* 	{ */
/* 		priv->mouse.beginx = event->x; */
/* 		priv->mouse.beginy = event->y; */
/* 		return TRUE; */
/* 	} */

/* 	// don't block */
/* 	return FALSE; */
/* } */

/* gboolean */
/* hkl_gui_3d_gl_area_scroll_event_cb(GtkWidget *gl_area, */
/* 				   GdkEventScroll *event, */
/* 				   gpointer user_data) */
/* { */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */

/* #define ZOOM_BY 10 */
/* 	if(event->direction == GDK_SCROLL_DOWN) */
/* 		priv->renderoptions.zoom += ZOOM_BY; */
/* 	else */
/* 		priv->renderoptions.zoom -= ZOOM_BY; */
/* #undef ZOOM_BY */

/* 	if(priv->renderoptions.zoom < 1) */
/* 		priv->renderoptions.zoom = 1; */
/* 	if(priv->renderoptions.zoom > 120) */
/* 		priv->renderoptions.zoom = 120; */

/* 	/\* queue a redraw on the GtkGLArea *\/ */
/* 	gtk_widget_queue_draw (gl_area); */

/* 	return FALSE; */
/* } */

/* gboolean */
/* hkl_gui_3d_gl_area_motion_notify_event_cb(GtkWidget *gl_area, */
/* 					  GdkEventMotion* event, */
/* 					  gpointer user_data) */
/* { */
/* 	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(user_data); */
/* 	GtkAllocation alloc; */
/* 	gint x, y; */
/* 	GdkModifierType state; */

/* 	gtk_widget_get_allocation(gl_area, &alloc); */

/* 	if(event->is_hint){ */
/* 		gdk_window_get_device_position(event->window, */
/* 					       event->device, */
/* 					       &x, &y, &state); */
/* 	}else{ */
/* 		x = event->x; */
/* 		y = event->y; */
/* 		state = event->state; */
/* 	} */

/* 	/\* left button pressed *\/ */
/* 	if(state & GDK_BUTTON1_MASK) */
/* 	{ */
/* 		if(state & GDK_SHIFT_MASK) */
/* 		{ */
/* 			/\* shift pressed, translate view *\/ */
/* 			priv->renderoptions.offx += (float)(x - priv->mouse.beginx) / alloc.width / priv->renderoptions.zoom; */
/* 			priv->renderoptions.offy -= (float)(y - priv->mouse.beginy) / alloc.height / priv->renderoptions.zoom; */
/* 		} */
/* 		else */
/* 		{ */
/* 			/\* rotate view *\/ */
/* 			gfloat spin_quat[4]; */
/* 			g3d_quat_trackball(spin_quat, */
/* 					   (2.0 * priv->mouse.beginx - alloc.width) / alloc.width, */
/* 					   (alloc.height - 2.0 * priv->mouse.beginy) / alloc.height, */
/* 					   (2.0 * x - alloc.width) / alloc.width, */
/* 					   (alloc.height - 2.0 * y) / alloc.height, */
/* 					   0.8 /\* trackball radius *\/); */
/* 			g3d_quat_add(priv->renderoptions.quat, */
/* 				     spin_quat, priv->renderoptions.quat); */
/* 			/\* normalize quat some times *\/ */
/* 			priv->renderoptions.norm_count ++; */
/* 			if(priv->renderoptions.norm_count > 97) { */
/* 				priv->renderoptions.norm_count = 0; */
/* 				g3d_quat_normalize(priv->renderoptions.quat); */
/* 			} */

/* 			/\* g3d_quat_to_rotation_xyz(priv->renderoptions.quat, *\/ */
/* 			/\*	&rx, &ry, &rz); *\/ */
/* 			/\* text = g_strdup_printf("%-.2f°, %-.2f°, %-.2f°", *\/ */
/* 			/\*	rx * 180.0 / G_PI, ry * 180.0 / G_PI, rz * 180.0 / G_PI); *\/ */
/* 			/\* gui_glade_status(priv, text); *\/ */
/* 			/\* g_free(text); *\/ */
/* 		} */
/* 	} */

/* 	/\* middle mouse button *\/ */
/* 	if(state & GDK_BUTTON2_MASK) */
/* 	{ */
/* 		priv->renderoptions.zoom += */
/* 			((y - priv->mouse.beginy) / (gfloat)alloc.height) * 40; */
/* 		if(priv->renderoptions.zoom < 1) */
/* 			priv->renderoptions.zoom = 1; */
/* 		if(priv->renderoptions.zoom > 120) */
/* 			priv->renderoptions.zoom = 120; */
/* 	} */
/* 	priv->mouse.beginx = x; */
/* 	priv->mouse.beginy = y; */

/* 	/\* queue a redraw on the GtkGLArea *\/ */
/* 	gtk_widget_queue_draw (gl_area); */

/* 	return FALSE; */
/* } */

/*********************************************/
/* HklGui3D and HklGui3DClass initialization */
/*********************************************/

static void
hkl_gui_3d_class_init (HklGui3DClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);

	/* virtual method */
	gobject_class->finalize = finalize;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	/* properties */
	props[PROP_FILENAME] =
		g_param_spec_string ("filename",
				     "Filename",
				     "The confuration filename",
				     NULL,
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	props[PROP_GEOMETRY] =
		g_param_spec_pointer ("geometry",
				      "Geometry",
				      "The Hkl Geometry used underneath",
				      G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	props[PROP_AABB] =
		g_param_spec_boolean ("aabb",
				      "Aabb",
				      "Draw the aabb boxes",
				      false,
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class,
					   N_PROPERTIES,
					   props);

}


static void
gl_area_on_realize_cb (GtkGLArea *area,
		       gpointer user_data)
{
	HklGui3D *self = HKL_GUI_3D (user_data);

	gtk_gl_area_make_current (area);

	if (gtk_gl_area_get_error (area) != NULL)
		return;

	hkl3d_gl_init(self->hkl3d);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

static void
aabb_activated (GtkToggleButton *button,
		gpointer user_data)
{
	HklGui3D *self = HKL_GUI_3D(user_data);
	gboolean aabb;

	aabb = gtk_toggle_button_get_active (button);

	hkl_gui_3d_set_aabb(self, aabb);
}

static void hkl_gui_3d_init (HklGui3D * self)
{
	HklGui3DPrivate *priv = hkl_gui_3d_get_instance_private(self);
	GtkWidget *vbox;
	GtkWidget *hbox;

	/* properties */
	priv->filename = NULL;
	priv->geometry = NULL;
	priv->aabb = FALSE;

	/* widgets instances */
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	priv->frame1 = GTK_FRAME (gtk_frame_new("3d"));
	self->gl_area = GTK_GL_AREA (gtk_gl_area_new ());
	self->toggle_button_aabb = gtk_toggle_button_new_with_label("aabb");

	/* frame1 */
	gtk_frame_set_child(priv->frame1, vbox);

	/* button_aabb */
	g_signal_connect (self->toggle_button_aabb, "toggled",
			  G_CALLBACK (aabb_activated), self);

	/* gl_area */
	gtk_gl_area_set_has_depth_buffer (self->gl_area, true);
	gtk_gl_area_set_auto_render (self->gl_area, true);
	gtk_widget_set_hexpand( GTK_WIDGET (self->gl_area), true);
	gtk_widget_set_vexpand( GTK_WIDGET (self->gl_area), true);
	gtk_widget_set_can_focus (GTK_WIDGET (self->gl_area), true);
	g_signal_connect(self->gl_area, "realize",
			 G_CALLBACK(gl_area_on_realize_cb), self);
	g_signal_connect(self->gl_area, "resize",
			 G_CALLBACK(hkl_gui_3d_gl_area_resize_cb), self);
	g_signal_connect(self->gl_area, "render",
			 G_CALLBACK(hkl_gui_3d_gl_area_render_cb), self);

	/* hbox */
	gtk_box_append (GTK_BOX (hbox), self->toggle_button_aabb);

	/* vbox */
	gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (self->gl_area));
	gtk_box_append (GTK_BOX (vbox), hbox);

	/* gtk_box_pack_end(priv->vbox1, GTK_WIDGET(self->gl_area), TRUE, TRUE, 0); */

	/* add events to the GtkGLArea */
	/* gtk_widget_add_events(GTK_WIDGET(self->gl_area), */
	/* 		      GDK_BUTTON1_MOTION_MASK | */
	/* 		      GDK_BUTTON2_MOTION_MASK | */
	/* 		      GDK_BUTTON_PRESS_MASK | */
	/* 		      GDK_VISIBILITY_NOTIFY_MASK); */

	/* connect the GL callbacks */
	/* g_signal_connect(self->gl_area, "button-press-event", */
	/* 		 G_CALLBACK(hkl_gui_3d_gl_area_button_press_event_cb), self); */
	/* g_signal_connect(self->gl_area, "motion-notify-event", */
	/* 		 G_CALLBACK(hkl_gui_3d_gl_area_motion_notify_event_cb), self); */
	/* gtk_widget_set_visible (GTK_WIDGET (priv->frame1), true); */
}
