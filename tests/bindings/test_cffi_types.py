#!/usr/bin/env python3
"""
cffi-based binding for libhkl, derived directly from hkl.h.
No GNOME/gi dependencies required.
"""

import cffi

ffi = cffi.FFI()

ffi.cdef("""
    /* opaque types */
    typedef struct _HklFactory          HklFactory;
    typedef struct _HklGeometry         HklGeometry;
    typedef struct _HklGeometryList     HklGeometryList;
    typedef struct _HklGeometryListItem HklGeometryListItem;
    typedef struct _HklDetector         HklDetector;
    typedef struct _HklEngineList       HklEngineList;
    typedef struct _HklEngine           HklEngine;
    typedef struct _HklSample           HklSample;
    typedef struct _HklLattice          HklLattice;
    typedef struct _HklParameter        HklParameter;
    typedef struct _HklMatrix           HklMatrix;
    typedef struct _HklSampleReflection HklSampleReflection;

    /* plain structs returned by value */
    typedef struct { double data[3]; } HklVector;
    typedef struct { double data[4]; } HklQuaternion;

    /* darray types — field is 'item', not 'data' */
    typedef struct { const char **item; size_t size; size_t alloc; } darray_string;
    typedef struct { HklEngine **item;  size_t size; size_t alloc; } darray_engine;

    /* GError */
    typedef struct {
        uint32_t domain;
        int      code;
        char    *message;
    } GError;

    typedef enum { HKL_UNIT_DEFAULT, HKL_UNIT_USER } HklUnitEnum;
    typedef enum { HKL_DETECTOR_TYPE_0D }             HklDetectorType;
    typedef enum {
        HKL_ENGINE_AXIS_NAMES_GET_READ,
        HKL_ENGINE_AXIS_NAMES_GET_WRITE,
    } HklEngineAxisNamesGet;
    typedef enum {
        HKL_ENGINE_CAPABILITIES_READABLE      = 1,
        HKL_ENGINE_CAPABILITIES_WRITABLE      = 2,
        HKL_ENGINE_CAPABILITIES_INITIALIZABLE = 4,
    } HklEngineCapabilities;
    typedef enum {
        HKL_ENGINE_DEPENDENCIES_AXES   = 1,
        HKL_ENGINE_DEPENDENCIES_ENERGY = 2,
        HKL_ENGINE_DEPENDENCIES_SAMPLE = 4,
    } HklEngineDependencies;

    /* --- factory --- */
    HklFactory   **hkl_factory_get_all(size_t *n);
    HklFactory    *hkl_factory_get_by_name(const char *name, GError **error);
    const char    *hkl_factory_name_get(const HklFactory *self);
    HklGeometry   *hkl_factory_create_new_geometry(const HklFactory *self);
    HklEngineList *hkl_factory_create_new_engine_list(const HklFactory *self);

    /* --- geometry --- */
    HklGeometry        *hkl_geometry_new_copy(const HklGeometry *self);
    void                hkl_geometry_free(HklGeometry *self);
    int                 hkl_geometry_set(HklGeometry *self, const HklGeometry *src);
    const char         *hkl_geometry_name_get(const HklGeometry *self);
    double              hkl_geometry_wavelength_get(const HklGeometry *self, HklUnitEnum unit_type);
    int                 hkl_geometry_wavelength_set(HklGeometry *self, double wavelength,
                                                    HklUnitEnum unit_type, GError **error);
    const darray_string *hkl_geometry_axis_names_get(const HklGeometry *self);
    const HklParameter  *hkl_geometry_axis_get(const HklGeometry *self, const char *name,
                                                GError **error);
    int                  hkl_geometry_axis_set(HklGeometry *self, const char *name,
                                               const HklParameter *axis, GError **error);
    void hkl_geometry_axis_values_get(const HklGeometry *self,
                                      double values[], size_t n_values,
                                      HklUnitEnum unit_type);
    int  hkl_geometry_axis_values_set(HklGeometry *self,
                                      double values[], size_t n_values,
                                      HklUnitEnum unit_type, GError **error);
    void          hkl_geometry_randomize(HklGeometry *self);
    HklQuaternion hkl_geometry_sample_rotation_get(const HklGeometry *self,
                                                    const HklSample *sample);
    HklQuaternion hkl_geometry_detector_rotation_get(const HklGeometry *self,
                                                      const HklDetector *detector);
    HklVector     hkl_geometry_ki_get(const HklGeometry *self);
    HklVector     hkl_geometry_kf_get(const HklGeometry *self, const HklDetector *detector);

    /* --- geometry list --- */
    void                       hkl_geometry_list_free(HklGeometryList *self);
    size_t                     hkl_geometry_list_n_items_get(const HklGeometryList *self);
    const HklGeometryListItem *hkl_geometry_list_items_first_get(const HklGeometryList *self);
    const HklGeometryListItem *hkl_geometry_list_items_next_get(const HklGeometryList *self,
                                                                 const HklGeometryListItem *item);
    const HklGeometry         *hkl_geometry_list_item_geometry_get(const HklGeometryListItem *self);

    /* --- detector --- */
    HklDetector *hkl_detector_factory_new(HklDetectorType type);
    HklDetector *hkl_detector_new_copy(const HklDetector *src);
    void         hkl_detector_free(HklDetector *self);

    /* --- parameter --- */
    HklParameter *hkl_parameter_new_copy(const HklParameter *self);
    void          hkl_parameter_free(HklParameter *self);
    const char   *hkl_parameter_name_get(const HklParameter *self);
    const char   *hkl_parameter_description_get(const HklParameter *self);
    const char   *hkl_parameter_default_unit_get(const HklParameter *self);
    const char   *hkl_parameter_user_unit_get(const HklParameter *self);
    double        hkl_parameter_value_get(const HklParameter *self, HklUnitEnum unit_type);
    int           hkl_parameter_value_set(HklParameter *self, double value,
                                          HklUnitEnum unit_type, GError **error);
    void          hkl_parameter_min_max_get(const HklParameter *self,
                                            double *min, double *max, HklUnitEnum unit_type);
    int           hkl_parameter_min_max_set(HklParameter *self, double min, double max,
                                            HklUnitEnum unit_type, GError **error);
    int           hkl_parameter_fit_get(const HklParameter *self);
    void          hkl_parameter_fit_set(HklParameter *self, int fit);
    void          hkl_parameter_randomize(HklParameter *self);

    /* --- matrix --- */
    HklMatrix *hkl_matrix_new(void);
    void       hkl_matrix_free(HklMatrix *self);
    double     hkl_matrix_get(const HklMatrix *self, unsigned int i, unsigned int j);
    void       hkl_quaternion_to_matrix(const HklQuaternion *self, HklMatrix *m);

    /* --- lattice --- */
    HklLattice         *hkl_lattice_new(double a, double b, double c,
                                        double alpha, double beta, double gamma,
                                        GError **error);
    HklLattice         *hkl_lattice_new_copy(const HklLattice *self);
    HklLattice         *hkl_lattice_new_default(void);
    void                hkl_lattice_free(HklLattice *self);
    int                 hkl_lattice_set(HklLattice *self,
                                        double a, double b, double c,
                                        double alpha, double beta, double gamma,
                                        HklUnitEnum unit_type, GError **error);
    void                hkl_lattice_get(const HklLattice *self,
                                        double *a, double *b, double *c,
                                        double *alpha, double *beta, double *gamma,
                                        HklUnitEnum unit_type);
    const HklParameter *hkl_lattice_a_get(const HklLattice *self);
    int                 hkl_lattice_a_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_b_get(const HklLattice *self);
    int                 hkl_lattice_b_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_c_get(const HklLattice *self);
    int                 hkl_lattice_c_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_alpha_get(const HklLattice *self);
    int                 hkl_lattice_alpha_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_beta_get(const HklLattice *self);
    int                 hkl_lattice_beta_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_gamma_get(const HklLattice *self);
    int                 hkl_lattice_gamma_set(HklLattice *self, const HklParameter *p, GError **error);
    const HklParameter *hkl_lattice_volume_get(const HklLattice *self);
    int                 hkl_lattice_reciprocal(const HklLattice *self, HklLattice *reciprocal);

    /* --- sample --- */
    HklSample          *hkl_sample_new(const char *name);
    HklSample          *hkl_sample_new_copy(const HklSample *self);
    void                hkl_sample_free(HklSample *self);
    const char         *hkl_sample_name_get(const HklSample *self);
    void                hkl_sample_name_set(HklSample *self, const char *name);
    const HklLattice   *hkl_sample_lattice_get(HklSample *self);
    void                hkl_sample_lattice_set(HklSample *self, const HklLattice *lattice);
    const HklParameter *hkl_sample_ux_get(const HklSample *self);
    int                 hkl_sample_ux_set(HklSample *self, const HklParameter *ux, GError **error);
    const HklParameter *hkl_sample_uy_get(const HklSample *self);
    int                 hkl_sample_uy_set(HklSample *self, const HklParameter *uy, GError **error);
    const HklParameter *hkl_sample_uz_get(const HklSample *self);
    int                 hkl_sample_uz_set(HklSample *self, const HklParameter *uz, GError **error);
    const HklMatrix    *hkl_sample_U_get(const HklSample *self);
    void                hkl_sample_U_set(HklSample *self, const HklMatrix *U, GError **error);
    const HklMatrix    *hkl_sample_UB_get(const HklSample *self);
    int                 hkl_sample_UB_set(HklSample *self, const HklMatrix *UB, GError **error);
    size_t              hkl_sample_n_reflections_get(const HklSample *self);
    HklSampleReflection *hkl_sample_reflections_first_get(HklSample *self);
    HklSampleReflection *hkl_sample_reflections_next_get(HklSample *self,
                                                          HklSampleReflection *reflection);
    void   hkl_sample_del_reflection(HklSample *self, HklSampleReflection *reflection);
    void   hkl_sample_add_reflection(HklSample *self, HklSampleReflection *reflection);
    double hkl_sample_get_reflection_measured_angle(const HklSample *self,
                                                     const HklSampleReflection *r1,
                                                     const HklSampleReflection *r2);
    double hkl_sample_get_reflection_theoretical_angle(const HklSample *self,
                                                        const HklSampleReflection *r1,
                                                        const HklSampleReflection *r2);
    int    hkl_sample_affine(HklSample *self, GError **error);
    int    hkl_sample_compute_UB_busing_levy(HklSample *self,
                                              const HklSampleReflection *r1,
                                              const HklSampleReflection *r2,
                                              GError **error);

    /* --- reflection --- */
    HklSampleReflection *hkl_sample_reflection_new(const HklGeometry *geometry,
                                                    const HklDetector *detector,
                                                    double h, double k, double l,
                                                    GError **error);
    void hkl_sample_reflection_hkl_get(const HklSampleReflection *self,
                                        double *h, double *k, double *l);
    int  hkl_sample_reflection_hkl_set(HklSampleReflection *self,
                                        double h, double k, double l, GError **error);
    int  hkl_sample_reflection_flag_get(const HklSampleReflection *self);
    void hkl_sample_reflection_flag_set(HklSampleReflection *self, int flag);
    const HklGeometry *hkl_sample_reflection_geometry_get(HklSampleReflection *self);
    void               hkl_sample_reflection_geometry_set(HklSampleReflection *self,
                                                           const HklGeometry *geometry);

    /* --- engine --- */
    const char          *hkl_engine_name_get(const HklEngine *self);
    const darray_string *hkl_engine_pseudo_axis_names_get(HklEngine *self);
    int                  hkl_engine_pseudo_axis_values_get(HklEngine *self,
                                                            double values[], size_t n_values,
                                                            HklUnitEnum unit_type, GError **error);
    HklGeometryList     *hkl_engine_pseudo_axis_values_set(HklEngine *self,
                                                            double values[], size_t n_values,
                                                            HklUnitEnum unit_type, GError **error);
    unsigned int         hkl_engine_capabilities_get(const HklEngine *self);
    unsigned int         hkl_engine_dependencies_get(const HklEngine *self);
    int                  hkl_engine_initialized_get(const HklEngine *self);
    int                  hkl_engine_initialized_set(HklEngine *self, int initialized,
                                                     GError **error);
    const darray_string *hkl_engine_modes_names_get(const HklEngine *self);
    const char          *hkl_engine_current_mode_get(const HklEngine *self);
    int                  hkl_engine_current_mode_set(HklEngine *self, const char *name,
                                                      GError **error);
    const darray_string *hkl_engine_axis_names_get(const HklEngine *self,
                                                    HklEngineAxisNamesGet mode);
    const darray_string *hkl_engine_parameters_names_get(const HklEngine *self);
    const HklParameter  *hkl_engine_parameter_get(const HklEngine *self, const char *name,
                                                   GError **error);
    int                  hkl_engine_parameter_set(HklEngine *self, const char *name,
                                                   const HklParameter *parameter, GError **error);
    void hkl_engine_parameters_values_get(const HklEngine *self,
                                           double values[], size_t n_values,
                                           HklUnitEnum unit_type);
    int  hkl_engine_parameters_values_set(HklEngine *self,
                                           double values[], size_t n_values,
                                           HklUnitEnum unit_type, GError **error);

    /* --- engine list --- */
    void            hkl_engine_list_free(HklEngineList *self);
    darray_engine  *hkl_engine_list_engines_get(HklEngineList *self);
    HklGeometry    *hkl_engine_list_geometry_get(HklEngineList *self);
    int             hkl_engine_list_geometry_set(HklEngineList *self, const HklGeometry *geometry);
    HklEngine      *hkl_engine_list_engine_get_by_name(HklEngineList *self, const char *name,
                                                        GError **error);
    void            hkl_engine_list_init(HklEngineList *self, HklGeometry *geometry,
                                         HklDetector *detector, HklSample *sample);
    int             hkl_engine_list_get(HklEngineList *self);
    const darray_string *hkl_engine_list_parameters_names_get(const HklEngineList *self);
    const HklParameter  *hkl_engine_list_parameter_get(const HklEngineList *self, const char *name,
                                                        GError **error);
    int                  hkl_engine_list_parameter_set(HklEngineList *self, const char *name,
                                                        const HklParameter *parameter,
                                                        GError **error);
    void hkl_engine_list_parameters_values_get(const HklEngineList *self,
                                                double values[], size_t n_values,
                                                HklUnitEnum unit_type);
    int  hkl_engine_list_parameters_values_set(HklEngineList *self,
                                                double values[], size_t n_values,
                                                HklUnitEnum unit_type, GError **error);

    /* --- vector --- */
    void hkl_vector_init(HklVector *self, double x, double y, double z);
""")

lib = ffi.dlopen("/usr/local/lib/libhkl.dylib")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def check(err):
    if err[0] != ffi.NULL:
        raise RuntimeError(ffi.string(err[0].message).decode())

def darray_str(d):
    return [ffi.string(d.item[i]).decode() for i in range(d.size)]

def darray_eng(d):
    return [d.item[i] for i in range(d.size)]

def factories():
    n = ffi.new("size_t *")
    ptr = lib.hkl_factory_get_all(n)
    return {ffi.string(lib.hkl_factory_name_get(ptr[i])).decode(): ptr[i]
            for i in range(n[0])}

def geometry_axis_values_get(geom, n, unit=None):
    if unit is None: unit = lib.HKL_UNIT_USER
    buf = ffi.new("double[]", n)
    lib.hkl_geometry_axis_values_get(geom, buf, n, unit)
    return list(buf)

def geometry_axis_values_set(geom, values, unit=None):
    if unit is None: unit = lib.HKL_UNIT_USER
    buf = ffi.new("double[]", values)
    err = ffi.new("GError **")
    lib.hkl_geometry_axis_values_set(geom, buf, len(values), unit, err)
    check(err)

def engine_pseudo_axis_values_get(engine, n, unit=None):
    if unit is None: unit = lib.HKL_UNIT_USER
    buf = ffi.new("double[]", n)
    err = ffi.new("GError **")
    lib.hkl_engine_pseudo_axis_values_get(engine, buf, n, unit, err)
    check(err)
    return list(buf)

def geometry_list_items(gl):
    items = []
    item = lib.hkl_geometry_list_items_first_get(gl)
    while item != ffi.NULL:
        items.append(item)
        item = lib.hkl_geometry_list_items_next_get(gl, item)
    return items

def sample_reflections(sample):
    refs = []
    r = lib.hkl_sample_reflections_first_get(sample)
    while r != ffi.NULL:
        refs.append(r)
        r = lib.hkl_sample_reflections_next_get(sample, r)
    return refs

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

import math
import unittest

class TestAPI(unittest.TestCase):

    def test_defines(self):
        self.assertIsNotNone(lib)

    def test_vector_api(self):
        v = ffi.new("HklVector *")
        self.assertEqual(list(v.data), [0.0, 0.0, 0.0])
        lib.hkl_vector_init(v, 1, 2, 3)
        self.assertEqual(list(v.data), [1.0, 2.0, 3.0])

    def test_quaternion_api(self):
        q = ffi.new("HklQuaternion *")
        self.assertEqual(len(list(q.data)), 4)

    def test_factory_api(self):
        facs = factories()
        self.assertGreater(len(facs), 0)
        for name, fac in facs.items():
            self.assertIsInstance(name, str)
            geom = lib.hkl_factory_create_new_geometry(fac)
            self.assertNotEqual(geom, ffi.NULL)
            el = lib.hkl_factory_create_new_engine_list(fac)
            self.assertNotEqual(el, ffi.NULL)
            lib.hkl_geometry_free(geom)
            lib.hkl_engine_list_free(el)

    def test_detector_api(self):
        det = lib.hkl_detector_factory_new(lib.HKL_DETECTOR_TYPE_0D)
        self.assertNotEqual(det, ffi.NULL)
        lib.hkl_detector_free(det)

    def test_geometry_api(self):
        fac  = factories()["K6C"]
        geom = lib.hkl_factory_create_new_geometry(fac)
        det  = lib.hkl_detector_factory_new(lib.HKL_DETECTOR_TYPE_0D)
        samp = lib.hkl_sample_new(b"toto")

        err = ffi.new("GError **")
        lib.hkl_geometry_wavelength_set(geom, 1.0, lib.HKL_UNIT_USER, err)
        check(err)
        self.assertAlmostEqual(
            lib.hkl_geometry_wavelength_get(geom, lib.HKL_UNIT_USER), 1.0)

        values_w = [0.0, 30.0, 0.0, 0.0, 0.0, 60.0]
        geometry_axis_values_set(geom, values_w)
        values_r = geometry_axis_values_get(geom, len(values_w))
        for w, r in zip(values_w, values_r):
            self.assertAlmostEqual(w, r)

        names = darray_str(lib.hkl_geometry_axis_names_get(geom))
        self.assertGreater(len(names), 0)
        for name in names:
            err = ffi.new("GError **")
            axis = lib.hkl_geometry_axis_get(geom, name.encode(), err)
            check(err)
            err = ffi.new("GError **")
            lib.hkl_parameter_min_max_set(axis, 0, math.radians(180),
                                          lib.HKL_UNIT_USER, err)
            check(err)

        q = lib.hkl_geometry_sample_rotation_get(geom, samp)
        self.assertEqual(len(list(q.data)), 4)

        q2 = lib.hkl_geometry_detector_rotation_get(geom, det)
        self.assertEqual(len(list(q2.data)), 4)

        ki = lib.hkl_geometry_ki_get(geom)
        self.assertEqual(len(list(ki.data)), 3)
        kf = lib.hkl_geometry_kf_get(geom, det)
        self.assertEqual(len(list(kf.data)), 3)

        lib.hkl_sample_free(samp)
        lib.hkl_detector_free(det)
        lib.hkl_geometry_free(geom)

    def test_engine_api(self):
        fac  = factories()["K6C"]
        det  = lib.hkl_detector_factory_new(lib.HKL_DETECTOR_TYPE_0D)
        geom = lib.hkl_factory_create_new_geometry(fac)
        geometry_axis_values_set(geom, [0.0, 30.0, 0.0, 0.0, 0.0, 60.0])

        samp = lib.hkl_sample_new(b"toto")
        lat  = lib.hkl_sample_lattice_get(samp)
        err  = ffi.new("GError **")
        lib.hkl_lattice_set(lat, 1.54, 1.54, 1.54, 90, 90, 90,
                            lib.HKL_UNIT_USER, err)
        check(err)
        lib.hkl_sample_lattice_set(samp, lat)

        el = lib.hkl_factory_create_new_engine_list(fac)
        lib.hkl_engine_list_init(el, geom, det, samp)
        lib.hkl_engine_list_get(el)

        err = ffi.new("GError **")
        hkl_eng = lib.hkl_engine_list_engine_get_by_name(el, b"hkl", err)
        check(err)

        pseudo_names = darray_str(lib.hkl_engine_pseudo_axis_names_get(hkl_eng))
        values = engine_pseudo_axis_values_get(hkl_eng, len(pseudo_names))
        print("\nhkl values:", values)

        modes = darray_str(lib.hkl_engine_modes_names_get(hkl_eng))
        for m in modes:
            self.assertIsInstance(m, str)

        for _ in range(100):
            buf = ffi.new("double[]", values)
            err = ffi.new("GError **")
            gl = lib.hkl_engine_pseudo_axis_values_set(
                hkl_eng, buf, len(values), lib.HKL_UNIT_USER, err)
            if err[0] != ffi.NULL:
                print(values, ffi.string(err[0].message).decode())
            else:
                self.assertNotEqual(gl, ffi.NULL)
                for item in geometry_list_items(gl):
                    g = lib.hkl_geometry_list_item_geometry_get(item)
                    self.assertNotEqual(g, ffi.NULL)
                lib.hkl_geometry_list_free(gl)
            values[1] += 0.01

        for eng in darray_eng(lib.hkl_engine_list_engines_get(el)):
            name = ffi.string(lib.hkl_engine_name_get(eng)).decode()
            self.assertIsInstance(name, str)
            modes = darray_str(lib.hkl_engine_modes_names_get(eng))
            self.assertGreater(len(modes), 0)

            caps = lib.hkl_engine_capabilities_get(eng)
            self.assertTrue(caps & lib.HKL_ENGINE_CAPABILITIES_READABLE)
            if name not in ("incidence", "emergence"):
                self.assertTrue(caps & lib.HKL_ENGINE_CAPABILITIES_WRITABLE)

            deps = lib.hkl_engine_dependencies_get(eng)
            self.assertTrue(deps & lib.HKL_ENGINE_DEPENDENCIES_AXES)

            if caps & lib.HKL_ENGINE_CAPABILITIES_INITIALIZABLE:
                err = ffi.new("GError **")
                lib.hkl_engine_initialized_set(eng, 0, err); check(err)
                self.assertEqual(lib.hkl_engine_initialized_get(eng), 0)
                err = ffi.new("GError **")
                lib.hkl_engine_initialized_set(eng, 1, err); check(err)
                self.assertEqual(lib.hkl_engine_initialized_get(eng), 1)

        lib.hkl_engine_list_free(el)
        lib.hkl_sample_free(samp)
        lib.hkl_detector_free(det)
        lib.hkl_geometry_free(geom)

    def test_engine_list_api(self):
        facs = factories()
        for name, fac in facs.items():
            el = lib.hkl_factory_create_new_engine_list(fac)
            param_names = darray_str(lib.hkl_engine_list_parameters_names_get(el))
            self.assertIsInstance(param_names, list)
            for pname in param_names:
                err = ffi.new("GError **")
                p = lib.hkl_engine_list_parameter_get(el, pname.encode(), err)
                check(err)
                err = ffi.new("GError **")
                lib.hkl_engine_list_parameter_set(el, pname.encode(), p, err)
                check(err)
            n = len(param_names)
            if n > 0:
                buf = ffi.new("double[]", n)
                lib.hkl_engine_list_parameters_values_get(
                    el, buf, n, lib.HKL_UNIT_USER)
                err = ffi.new("GError **")
                lib.hkl_engine_list_parameters_values_set(
                    el, buf, n, lib.HKL_UNIT_USER, err)
                check(err)
            lib.hkl_engine_list_free(el)

    def test_lattice_api(self):
        err = ffi.new("GError **")
        lat = lib.hkl_lattice_new(1.54, 1.54, 1.54,
                                   math.radians(90), math.radians(90), math.radians(90),
                                   err)
        check(err)
        lat2 = lib.hkl_lattice_new_copy(lat)

        a  = ffi.new("double *"); b  = ffi.new("double *"); c  = ffi.new("double *")
        al = ffi.new("double *"); be = ffi.new("double *"); ga = ffi.new("double *")
        lib.hkl_lattice_get(lat, a, b, c, al, be, ga, lib.HKL_UNIT_DEFAULT)
        v1 = (a[0], b[0], c[0], al[0], be[0], ga[0])
        lib.hkl_lattice_get(lat2, a, b, c, al, be, ga, lib.HKL_UNIT_DEFAULT)
        v2 = (a[0], b[0], c[0], al[0], be[0], ga[0])
        self.assertEqual(v1, v2)

        err = ffi.new("GError **")
        lib.hkl_lattice_set(lat, 1, 2, 3, 90, 90, 90, lib.HKL_UNIT_USER, err)
        check(err)
        lib.hkl_lattice_get(lat, a, b, c, al, be, ga, lib.HKL_UNIT_DEFAULT)
        v3 = (a[0], b[0], c[0], al[0], be[0], ga[0])
        self.assertNotEqual(v1, v3)

        lib.hkl_lattice_free(lat2)
        lib.hkl_lattice_free(lat)

    def test_sample_api(self):
        samp = lib.hkl_sample_new(b"toto")
        self.assertEqual(ffi.string(lib.hkl_sample_name_get(samp)).decode(), "toto")

        copy = lib.hkl_sample_new_copy(samp)
        self.assertEqual(ffi.string(lib.hkl_sample_name_get(copy)).decode(), "toto")
        lib.hkl_sample_name_set(copy, b"titi")
        self.assertNotEqual(
            ffi.string(lib.hkl_sample_name_get(copy)).decode(),
            ffi.string(lib.hkl_sample_name_get(samp)).decode())

        err = ffi.new("GError **")
        lat = lib.hkl_lattice_new(1.54, 1.54, 1.54,
                                   math.radians(90), math.radians(90), math.radians(90),
                                   err)
        check(err)
        lib.hkl_sample_lattice_set(samp, lat)

        recip = lib.hkl_lattice_new_copy(lat)
        lib.hkl_lattice_reciprocal(lat, recip)

        lib.hkl_lattice_free(recip)
        lib.hkl_lattice_free(lat)
        lib.hkl_sample_free(copy)
        lib.hkl_sample_free(samp)

    def test_reflection_api(self):
        fac  = factories()["K6C"]
        det  = lib.hkl_detector_factory_new(lib.HKL_DETECTOR_TYPE_0D)
        geom = lib.hkl_factory_create_new_geometry(fac)
        geometry_axis_values_set(geom, [0.0, 30.0, 0.0, 0.0, 0.0, 60.0])
        samp = lib.hkl_sample_new(b"toto")

        err = ffi.new("GError **")
        r1 = lib.hkl_sample_reflection_new(geom, det, 1, 1, 1, err)
        check(err)
        err = ffi.new("GError **")
        r2 = lib.hkl_sample_reflection_new(geom, det, 1, 1, 1, err)
        check(err)

        lib.hkl_sample_add_reflection(samp, r1)
        lib.hkl_sample_add_reflection(samp, r2)

        h = ffi.new("double *"); k = ffi.new("double *"); l = ffi.new("double *")
        lib.hkl_sample_reflection_hkl_get(r2, h, k, l)
        self.assertEqual((h[0], k[0], l[0]), (1.0, 1.0, 1.0))

        err = ffi.new("GError **")
        lib.hkl_sample_reflection_hkl_set(r2, 1, 0, 1, err)
        check(err)
        lib.hkl_sample_reflection_hkl_get(r2, h, k, l)
        self.assertEqual((h[0], k[0], l[0]), (1.0, 0.0, 1.0))

        flag = lib.hkl_sample_reflection_flag_get(r1)
        lib.hkl_sample_reflection_flag_set(r1, flag)

        g = lib.hkl_sample_reflection_geometry_get(r1)
        lib.hkl_sample_reflection_geometry_set(r1, g)

        lib.hkl_sample_get_reflection_measured_angle(samp, r1, r2)
        lib.hkl_sample_get_reflection_theoretical_angle(samp, r1, r2)

        for ref in sample_reflections(samp):
            lib.hkl_sample_del_reflection(samp, ref)

        lib.hkl_sample_free(samp)
        lib.hkl_detector_free(det)
        lib.hkl_geometry_free(geom)


if __name__ == "__main__":
    unittest.main(verbosity=2)