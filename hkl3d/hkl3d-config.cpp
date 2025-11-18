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

#include <yaml.h>

#include "hkl3d-private.h"

/* Set environment variable DEBUG=1 to enable debug output. */
int debug = 0;

/* yaml_* functions return 1 on success and 0 on failure. */
enum status {
	SUCCESS = 1,
	FAILURE = 0
};

/***************/
/* Hkl3DConfig */
/***************/

Hkl3DConfig* hkl3d_config_new(void)
{
	Hkl3DConfig* self = nullptr;

	self = g_new(Hkl3DConfig, 1);
	if(!self)
		return nullptr;

	darray_init(self->models);

	return self;
}

void hkl3d_config_free(Hkl3DConfig *self)
{
	g_return_if_fail(NULL != self);

	Hkl3DModel **model;

	darray_foreach(model, self->models){
		hkl3d_model_free(*model);
	}
	darray_free(self->models);
	g_free(self);
}

void hkl3d_config_fprintf(FILE *f, const Hkl3DConfig *self)
{
	Hkl3DModel **model;
	fprintf(f, "Hkl3DConfig (models=[");
	darray_foreach(model, self->models){
		fprintf (f, " ");
		hkl3d_model_fprintf(f, *model);
	}
	fprintf (f, "])");
}


#define INDENT "  "
#define STRVAL(x) ((x) ? (char*)(x) : "")

static void indent(int level)
{
	int i;
	for (i = 0; i < level; i++) {
		printf("%s", INDENT);
	}
}

static void print_event(yaml_event_t *event)
{
	static int level = 0;

	switch (event->type) {
	case YAML_NO_EVENT:
		indent(level);
		printf("no-event (%d)\n", event->type);
		break;
	case YAML_STREAM_START_EVENT:
		indent(level++);
		printf("stream-start-event (%d)\n", event->type);
		break;
	case YAML_STREAM_END_EVENT:
		indent(--level);
		printf("stream-end-event (%d)\n", event->type);
		break;
	case YAML_DOCUMENT_START_EVENT:
		indent(level++);
		printf("document-start-event (%d)\n", event->type);
		break;
	case YAML_DOCUMENT_END_EVENT:
		indent(--level);
		printf("document-end-event (%d)\n", event->type);
		break;
	case YAML_ALIAS_EVENT:
		indent(level);
		printf("alias-event (%d)\n", event->type);
		break;
	case YAML_SCALAR_EVENT:
		indent(level);
		printf("scalar-event (%d) = {value=\"%s\", length=%d}\n",
		       event->type,
		       STRVAL(event->data.scalar.value),
		       (int)event->data.scalar.length);
		break;
	case YAML_SEQUENCE_START_EVENT:
		indent(level++);
		printf("sequence-start-event (%d)\n", event->type);
		break;
	case YAML_SEQUENCE_END_EVENT:
		indent(--level);
		printf("sequence-end-event (%d)\n", event->type);
		break;
	case YAML_MAPPING_START_EVENT:
		indent(level++);
		printf("mapping-start-event (%d)\n", event->type);
		break;
	case YAML_MAPPING_END_EVENT:
		indent(--level);
		printf("mapping-end-event (%d)\n", event->type);
		break;
	}
	if (level < 0) {
		fprintf(stderr, "indentation underflow!\n");
		level = 0;
	}
}

// --- !<Mon%20Aug%2023%2014:52:20%202010%0A>
// - FileName: diffabs.glb
//   Objects:
//   - Id: 0
//     Name: base
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 1
//     Name: delta
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 2
//     Name: gamma
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 3
//     Name: kappa
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 4
//     Name: komega
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 5
//     Name: kphi
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
//   - Id: 6
//     Name: mu
//     Transformation: [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
//       0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
//       1.000000]
//     Hide: no
// ...

// * The sequence of yaml events supported is:
// *
// *    <stream>               ::= STREAM-START <document> STREAM-END
// *    <document>             ::= DOCUMENT-START <file-list> DOCUMENT-END
// *    <file-list>            ::= SEQUENCE-START <file-obj> SEQUENCE-END
// *    <file-obj>             ::= MAPPING-START <file-data>* MAPPING-END
// *    <file-data>            ::= "FileName" <string> |
// *                               "Objects" <object-list>
// *    <object-list>          ::= SEQUENCE-START <object-obj> SEQUENCE-END
// *    <object-obj>           ::= MAPPING-START <object-data>* MAPPING-END
// *    <object-data>          ::= "Id" <integer> |
// *                               "Name" <string> |
// *                               "Transformation" <transformation-list> |
// *                               "Hide" <bool> |
// *    <transformation-list>  ::= SEQUENCE-START <float> SEQUENCE-END

// stream-start-event (1)
//   document-start-event (3)
//     sequence-start-event (7)
//       mapping-start-event (9)
//         scalar-event (6) = {value="FileName", length=8}
//         scalar-event (6) = {value="diffabs.glb", length=11}
//         scalar-event (6) = {value="Objects", length=7}
//         sequence-start-event (7)
//           mapping-start-event (9)
//             scalar-event (6) = {value="Id", length=2}
//             scalar-event (6) = {value="0", length=1}
//             scalar-event (6) = {value="Name", length=4}
//             scalar-event (6) = {value="base", length=4}
//             scalar-event (6) = {value="Transformation", length=14}
//             sequence-start-event (7)
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//             sequence-end-event (8)
//             scalar-event (6) = {value="Hide", length=4}
//             scalar-event (6) = {value="no", length=2}
//           mapping-end-event (10)
//           mapping-start-event (9)
//             scalar-event (6) = {value="Id", length=2}
//             scalar-event (6) = {value="1", length=1}
//             scalar-event (6) = {value="Name", length=4}
//             scalar-event (6) = {value="delta", length=5}
//             scalar-event (6) = {value="Transformation", length=14}
//             sequence-start-event (7)
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="0.000000", length=8}
//               scalar-event (6) = {value="1.000000", length=8}
//             sequence-end-event (8)
//             scalar-event (6) = {value="Hide", length=4}
//             scalar-event (6) = {value="no", length=2}
//           mapping-end-event (10)
//         [...]
// 	   sequence-end-event (...)
//       mapping-end-event (10)
//     sequence-end-event (...)
//   document-end-sequence (...)
// stream-end-sequence (...)

// https://github.com/meffie/libyaml-examples/blob/master/parse.c

// * The sequence of yaml events supported is:
// *
// *    <stream>               ::= STREAM-START <document> STREAM-END
// *    <document>             ::= DOCUMENT-START <file-list> DOCUMENT-END
// *    <file-list>            ::= SEQUENCE-START <file-obj> SEQUENCE-END
// *    <file-obj>             ::= MAPPING-START <file-data>* MAPPING-END
// *    <file-data>            ::= "FileName" <string> |
// *                               "Objects" <object-list>
// *    <object-list>          ::= SEQUENCE-START <object-obj> SEQUENCE-END
// *    <object-obj>           ::= MAPPING-START <object-data>* MAPPING-END
// *    <object-data>          ::= "Id" <integer> |
// *                               "Name" <string> |
// *                               "Transformation" <transformation-list> |
// *                               "Hide" <bool> |
// *    <transformation-list>  ::= SEQUENCE-START <float> SEQUENCE-END

enum state {
	STATE_START,     /* start state */
	STATE_STREAM,    /* start/end stream */
	STATE_DOCUMENT,  /* start/end document */

	STATE_FLIST,     /* file list */
	STATE_FVALUES,   /* file key-value pairs */
	STATE_FKEY,      /* file key */
	STATE_FNAME,     /* file name value */
	STATE_FOBJECTS,   /* file color value */

	STATE_OLIST,    /* object list */
	STATE_OVALUES,  /* object key-value pairs */
	STATE_OKEY,     /* object key */
	STATE_OID,    /* object Id */
	STATE_ONAME,   /* object Name */
	STATE_OTRANSFORMATION, /* object Transformation */
	STATE_OHIDE, /* object Hide */

	STATE_TLIST, /* transformation list */
	STATE_TVALUES, /* transformation value */

	STATE_STOP      /* end state */
};

static const char *state_name[] = {
	"start",
	"stream",
	"document",
	"flist",
	"fvalues",
	"fkey",
	"fname",
	"fobjects",
	"olist",
	"ovalues",
	"okey",
	"oid",
	"oname",
	"otransformation",
	"ohide",
	"tlist",
	"tvalues",
	"stop",
};

/* * Our application parser state data. */
struct parser_state {
	enum state state;      /* The current parse state */
	Hkl3D *hkl3d;
	char *dir;
	char *fname;
	int oid;
	char *oname;
	mat4s otransformation;
	bool ohide;
	int tindex;
};


void state_fprintf(FILE *f, const parser_state *state)
{
	fprintf(f, "parser_state->state (%d)\n", state->state);
	fprintf(f, "parser_state->hkl3d (%p)\n", state->hkl3d);
	fprintf(f, "parser_state->dir   \"(%s)\"\n", state->dir);
}

/*
 * Convert a yaml boolean string to a boolean value (true|false).
 */
static int get_boolean(const char *string, bool *value)
{
	const char *t[] = {"y", "Y", "yes", "Yes", "YES", "true", "True", "TRUE", "on", "On", "ON", NULL};
	const char *f[] = {"n", "N", "no", "No", "NO", "false", "False", "FALSE", "off", "Off", "OFF", NULL};
	const char **p;

	for (p = t; *p; p++) {
		if (strcmp(string, *p) == 0) {
			*value = true;
			return 0;
		}
	}
	for (p = f; *p; p++) {
		if (strcmp(string, *p) == 0) {
			*value = false;
			return 0;
		}
	}
	return EINVAL;
}

/*
 * Consume yaml events generated by the libyaml parser to
 * import our data into raw c data structures. Error processing
 * is keep to a mimimum since this is just an example.
 */
int consume_event(struct parser_state *s, yaml_event_t *event)
{
	char *value;
	Hkl3DObject *object;

	if (debug) {
		printf("state=%d (%s) event=%d\n", s->state, state_name[s->state], event->type);
	}
	switch (s->state) {
	case STATE_START:
		switch (event->type) {
		case YAML_STREAM_START_EVENT:
			s->state = STATE_STREAM;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_STREAM:
		switch (event->type) {
		case YAML_DOCUMENT_START_EVENT:
			s->state = STATE_DOCUMENT;
			break;
		case YAML_STREAM_END_EVENT:
			s->state = STATE_STOP;  /* All done. */
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_DOCUMENT:
		switch (event->type) {
		case YAML_SEQUENCE_START_EVENT:
			s->state = STATE_FLIST;
			break;
		case YAML_DOCUMENT_END_EVENT:
			s->state = STATE_STREAM;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_FLIST:
		switch (event->type) {
		case YAML_MAPPING_START_EVENT:
			s->state = STATE_FKEY;
			break;
		case YAML_SEQUENCE_END_EVENT:
			s->state = STATE_DOCUMENT;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_FKEY:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			if (debug){
				fprintf(stderr, "Fkey: [%s]\n", value);
			}
			if (strcmp(value, "FileName") == 0) {
				s->state = STATE_FNAME;
			} else if (strcmp(value, "Objects") == 0) {
				s->state = STATE_FOBJECTS;
			} else {
				fprintf(stderr, "Unexpected key: %s\n", value);
				return FAILURE;
			}
			break;
		case YAML_MAPPING_END_EVENT:
			s->state = STATE_FLIST;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_FNAME:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			if (hkl3d_contains_model(s->hkl3d, value)) {
				fprintf(stderr, "Warning: [%s] already loaded previously.\n", value);
			}else{
				if (s->fname) {
					fprintf(stderr, "Warning: duplicate 'FileName' key.\n");
					free(s->fname);
				}
				s->fname = strdup(value);
				if (debug){
					fprintf (stdout, "Adding [%s] model.\n", value);
				}
				hkl3d_add_model_from_file(s->hkl3d, value, s->dir);
			}
			s->state = STATE_FKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_FOBJECTS:
		switch (event->type) {
		case YAML_SEQUENCE_START_EVENT:
			s->state = STATE_OVALUES;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_OVALUES:
		switch (event->type) {
		case YAML_MAPPING_START_EVENT:
			s->state = STATE_OKEY;
			break;
		case YAML_SEQUENCE_END_EVENT:
			s->state = STATE_FKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_OKEY:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			if (debug){
				fprintf(stderr, "Okey: [%s]\n", value);
			}
			if (strcmp(value, "Id") == 0) {
				s->state = STATE_OID;
			} else if (strcmp(value, "Name") == 0) {
				s->state = STATE_ONAME;
			} else if (strcmp(value, "Transformation") == 0) {
				s->state = STATE_OTRANSFORMATION;
			} else if (strcmp(value, "Hide") == 0) {
				s->state = STATE_OHIDE;
			} else {
				fprintf(stderr, "Unexpected key: %s\n", value);
				return FAILURE;
			}
			break;
		case YAML_MAPPING_END_EVENT:
			object = hkl3d_get_object_by_id(s->hkl3d, s->fname, s->oid);
			if (nullptr != object){
				hkl3d_object_axis_name_set(object, s->oname);
				hkl3d_object_transformation_set(object, s->otransformation);
				hkl3d_object_hide_set(object, s->ohide);
				g_clear_pointer(&s->oname, g_free);
				s->state = STATE_OVALUES;
			} else {
				fprintf(stderr, "Can not find the Hkl3DObject in [%s] with Id: %d\n", s->fname, s->oid);
				return FAILURE;
			}
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_OID:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			s->oid = atoi (value);
			if (debug){
				fprintf (stderr, "OID: [%d]\n", s->oid);
			}
			s->state = STATE_OKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_ONAME:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			if (debug){
				fprintf (stderr, "OName: [%s]\n", value);
			}
			if (s->oname) {
				fprintf(stderr, "Warning: duplicate 'Name' key.\n");
				free(s->oname);
			}
			s->oname = strdup(value);
			s->state = STATE_OKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_OTRANSFORMATION:
		switch (event->type) {
		case YAML_SEQUENCE_START_EVENT:
			s->tindex = 0; /* start the sequence */
			s->state = STATE_TVALUES;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_TVALUES:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			if (s->tindex >= 16){
				fprintf(stderr, "Warning: Skip extra transfromation value (16 values expected).\n");
			}else{
				value = (char *)event->data.scalar.value;
				((float *)(s->otransformation.raw))[s->tindex++] = atof(value);
				if (debug){
					fprintf (stderr, "TValue: [%f][%d]\n", atof(value), s->tindex-1);
				}
			}
			s->state = STATE_TVALUES;
			break;
		case YAML_SEQUENCE_END_EVENT:
			/* TODO check 16 values */
			s->state = STATE_OKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_OHIDE:
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			value = (char *)event->data.scalar.value;
			if (get_boolean(value, &s->ohide)) {
				fprintf(stderr, "Invalid boolean string value: %s\n", value);
				return FAILURE;
			}
			if (debug){
				fprintf (stderr, "Ohide: [%d]\n", s->ohide);
			}
			s->state = STATE_OKEY;
			break;
		default:
			fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
			return FAILURE;
		}
		break;

	case STATE_STOP:
		break;
	}
	return SUCCESS;
}

int hkl3d_load_config(Hkl3D *self, const char *filename)
{
	int code = EXIT_FAILURE;
	int status;
	FILE *file = nullptr;
	struct parser_state state;
	yaml_parser_t parser;

	memset(&state, 0, sizeof(state));
	state.hkl3d = self;
	state.dir = g_path_get_dirname(filename);
	state.state = STATE_START;

	if (debug){
		state_fprintf(stderr, &state);
	}

	file = fopen(filename, "rb");
	if (!file){
		fprintf(stderr, "Could not open the %s config file\n", filename);
		goto out_release_dir;
	}

	if (!yaml_parser_initialize(&parser)){
		fprintf(stderr, "Could not initialize the parser object\n");
		goto out_close_file;
	}

	yaml_parser_set_input_file(&parser, file);
	do {
		yaml_event_t event;

		status = yaml_parser_parse(&parser, &event);
		if (status == FAILURE) {
			fprintf(stderr, "yaml_parser_parse error\n");
			goto out_release_parser;
		}
		status = consume_event(&state, &event);
		yaml_event_delete(&event);
		if (status == FAILURE) {
			fprintf(stderr, "consume_event error\n");
			goto out_release_parser;
		}
	} while (state.state != STATE_STOP);

	hkl3d_connect_all_axes(self);

	if (debug){
		hkl3d_fprintf(stdout, self);
	}

	code = EXIT_SUCCESS;

out_release_parser:
	g_clear_pointer(&state.oname, g_free);
	g_clear_pointer(&state.fname, g_free);
	yaml_parser_delete(&parser);
out_close_file:
	fclose (file);
out_release_dir:
	g_clear_pointer (&state.dir, g_free);

	return code;
}

void hkl3d_save_config(Hkl3D *self, const char *filename)
{
	Hkl3DModel **model;

	darray_foreach(model, self->config->models){
		char number[64];
		int properties1, key1, value1,seq0;
		int root;
		time_t now;
		yaml_emitter_t emitter;
		yaml_document_t output_document;
		yaml_event_t output_event;
		FILE * file;
		Hkl3DObject **object;

		memset(&emitter, 0, sizeof(emitter));
		memset(&output_document, 0, sizeof(output_document));
		memset(&output_event, 0, sizeof(output_event));

		if (!yaml_emitter_initialize(&emitter))
			fprintf(stderr, "Could not inialize the emitter object\n");

		/* Set the emitter parameters */
		file = fopen(filename, "a+");
		if(!file){
			fprintf(stderr, "Could not open the config file %s to save\n", filename);
			return;
		}
		yaml_emitter_set_output_file(&emitter, file);
		yaml_emitter_open(&emitter);

		/* Create an output_document object */
		if (!yaml_document_initialize(&output_document, nullptr, nullptr, nullptr, 0, 0))
			fprintf(stderr, "Could not create a output_document object\n");

		/* Create the root of the config file */
		time(&now);
		root = yaml_document_add_sequence(&output_document,
						  (yaml_char_t *)ctime(&now),
						  YAML_BLOCK_SEQUENCE_STYLE);

		/* create the property of the root sequence */
		properties1 = yaml_document_add_mapping(&output_document,
							(yaml_char_t *)YAML_MAP_TAG,
							YAML_BLOCK_MAPPING_STYLE);

		yaml_document_append_sequence_item(&output_document, root, properties1);

		/* add the map key1 : value1 to the property */
		key1 = yaml_document_add_scalar(&output_document,
						nullptr,
						(yaml_char_t *)"FileName",
						-1,
						YAML_PLAIN_SCALAR_STYLE);
		value1 = yaml_document_add_scalar(&output_document,
						  nullptr,
						  (yaml_char_t *)(*model)->filename,
						  -1,
						  YAML_PLAIN_SCALAR_STYLE);
		yaml_document_append_mapping_pair(&output_document, properties1, key1, value1);

		/* add the map key1 : seq0 to the first property */
		key1 = yaml_document_add_scalar(&output_document,
						nullptr,
						(yaml_char_t *)"Objects",
						-1,
						YAML_PLAIN_SCALAR_STYLE);
		/* create the sequence of objects */
		seq0 = yaml_document_add_sequence(&output_document,
						  (yaml_char_t *)YAML_SEQ_TAG,
						  YAML_BLOCK_SEQUENCE_STYLE);
		darray_foreach(object, (*model)->objects){
			int k;
			int l;
			int properties;
			int key;
			int value;
			int seq1;

			properties = yaml_document_add_mapping(&output_document,
							       (yaml_char_t *)YAML_MAP_TAG,
							       YAML_BLOCK_MAPPING_STYLE);
			yaml_document_append_sequence_item(&output_document,seq0, properties);

			key = yaml_document_add_scalar(&output_document,
						       nullptr,
						       (yaml_char_t *)"Id", -1,
						       YAML_PLAIN_SCALAR_STYLE);

			sprintf(number, "%d", (*object)->mesh);
			value = yaml_document_add_scalar(&output_document,
							 nullptr,
							 (yaml_char_t *)number,
							 -1,
							 YAML_PLAIN_SCALAR_STYLE);
			yaml_document_append_mapping_pair(&output_document,properties,key,value);

			key = yaml_document_add_scalar(&output_document,
						       nullptr,
						       (yaml_char_t *)"Name",
						       -1,
						       YAML_PLAIN_SCALAR_STYLE);
			value = yaml_document_add_scalar(&output_document,
							 nullptr,
							 (yaml_char_t *)(*object)->axis_name,
							 -1,
							 YAML_PLAIN_SCALAR_STYLE);
			yaml_document_append_mapping_pair(&output_document,properties,key,value);

			key = yaml_document_add_scalar(&output_document,
						       nullptr,
						       (yaml_char_t *)"Transformation",
						       -1,
						       YAML_PLAIN_SCALAR_STYLE);
			seq1 = yaml_document_add_sequence(&output_document,
							  (yaml_char_t *)YAML_SEQ_TAG,
							  YAML_FLOW_SEQUENCE_STYLE);
			yaml_document_append_mapping_pair(&output_document,properties, key, seq1);
			for(k=0; k<4; k++){
				for(l=0; l<4; l++){
					sprintf(number, "%f", (*object)->transformation.raw[k][l]);
					value = yaml_document_add_scalar(&output_document,
									 nullptr,
									 (yaml_char_t *)number,
									 -1,
									 YAML_PLAIN_SCALAR_STYLE);
					yaml_document_append_sequence_item(&output_document,seq1,value);
				}
			}

			key = yaml_document_add_scalar(&output_document,
						       nullptr,
						       (yaml_char_t *)"Hide",
						       -1,
						       YAML_PLAIN_SCALAR_STYLE);
			if((*object)->hide)
				value = yaml_document_add_scalar(&output_document,
								 nullptr,
								 (yaml_char_t *)"yes",
								 -1,
								 YAML_PLAIN_SCALAR_STYLE);
			else
				value = yaml_document_add_scalar(&output_document,
								 nullptr,
								 (yaml_char_t *)"no",
								 -1,
								 YAML_PLAIN_SCALAR_STYLE);
			yaml_document_append_mapping_pair(&output_document,properties,key,value);
		}
		yaml_document_append_mapping_pair(&output_document, properties1, key1, seq0);

		/* flush the document */
		yaml_emitter_dump(&emitter, &output_document);
		fclose(file);

		yaml_document_delete(&output_document);
		yaml_emitter_delete(&emitter);
	}
}
