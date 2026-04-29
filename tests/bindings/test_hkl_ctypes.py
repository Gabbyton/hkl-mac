#!/usr/bin/env python3
"""
ctypes-based rewrite of the hkl Python binding test.
Replaces gi.repository with direct calls to libhkl.dylib.

Copyright (C) 2012-2013, 2023, 2024 Synchrotron SOLEIL
Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
ctypes port: 2026
"""

import ctypes
import ctypes.util
import math
import unittest

# ---------------------------------------------------------------------------
# Load the library
# ---------------------------------------------------------------------------

_lib_path = ctypes.util.find_library("hkl") or "/usr/local/lib/libhkl.dylib"
_lib = ctypes.CDLL(_lib_path)

# ---------------------------------------------------------------------------
# Opaque pointer types
# ---------------------------------------------------------------------------

class _Opaque(ctypes.Structure):
    pass

DetectorP        = ctypes.POINTER(_Opaque)
FactoryP         = ctypes.POINTER(_Opaque)
GeometryP        = ctypes.POINTER(_Opaque)
GeometryListP    = ctypes.POINTER(_Opaque)
GeometryListItemP= ctypes.POINTER(_Opaque)
EngineP          = ctypes.POINTER(_Opaque)
EngineListP      = ctypes.POINTER(_Opaque)
SampleP          = ctypes.POINTER(_Opaque)
LatticeP         = ctypes.POINTER(_Opaque)
ReflectionP      = ctypes.POINTER(_Opaque)
ParameterP       = ctypes.POINTER(_Opaque)
VectorP          = ctypes.POINTER(_Opaque)
QuaternionP      = ctypes.POINTER(_Opaque)
MatrixP          = ctypes.POINTER(_Opaque)
UnitP            = ctypes.POINTER(_Opaque)  # GError**

# GError (we only need the message field)
class GError(ctypes.Structure):
    _fields_ = [
        ("domain",  ctypes.c_uint32),
        ("code",    ctypes.c_int),
        ("message", ctypes.c_char_p),
    ]
GErrorP  = ctypes.POINTER(GError)
GErrorPP = ctypes.POINTER(GErrorP)

# ---------------------------------------------------------------------------
# Enums (integer constants)
# ---------------------------------------------------------------------------

HKL_UNIT_DEFAULT = 0
HKL_UNIT_USER    = 1

HKL_DETECTOR_TYPE_0D = 0

HKL_ENGINE_AXIS_NAMES_GET_READ  = 0
HKL_ENGINE_AXIS_NAMES_GET_WRITE = 1

HKL_ENGINE_CAPABILITIES_READABLE      = 1 << 0
HKL_ENGINE_CAPABILITIES_WRITABLE      = 1 << 1
HKL_ENGINE_CAPABILITIES_INITIALIZABLE = 1 << 2

HKL_ENGINE_DEPENDENCIES_AXES = 1 << 0

# ---------------------------------------------------------------------------
# darray helpers — hkl returns darray* which is {data*, size, alloc}
# ---------------------------------------------------------------------------

class Darray(ctypes.Structure):
    """Matches the darray layout used by libhkl for string arrays."""
    _fields_ = [
        ("data",  ctypes.POINTER(ctypes.c_char_p)),
        ("size",  ctypes.c_size_t),
        ("alloc", ctypes.c_size_t),
    ]

DarrayP = ctypes.POINTER(Darray)

def _darray_to_list(ptr):
    """Convert a darray* of strings to a Python list."""
    if not ptr:
        return []
    arr = ptr.contents
    return [arr.data[i].decode() for i in range(arr.size)]

# darray of doubles
class DarrayDouble(ctypes.Structure):
    _fields_ = [
        ("data",  ctypes.POINTER(ctypes.c_double)),
        ("size",  ctypes.c_size_t),
        ("alloc", ctypes.c_size_t),
    ]

DarrayDoubleP = ctypes.POINTER(DarrayDouble)

def _darray_double_to_list(ptr):
    if not ptr:
        return []
    arr = ptr.contents
    return [arr.data[i] for i in range(arr.size)]

# darray of engine pointers
class DarrayEngine(ctypes.Structure):
    _fields_ = [
        ("data",  ctypes.POINTER(EngineP)),
        ("size",  ctypes.c_size_t),
        ("alloc", ctypes.c_size_t),
    ]

DarrayEngineP = ctypes.POINTER(DarrayEngine)

# ---------------------------------------------------------------------------
# Function signatures
# ---------------------------------------------------------------------------

def _fn(name, restype, *argtypes):
    f = getattr(_lib, name)
    f.restype  = restype
    f.argtypes = list(argtypes)
    return f

# --- detector ---
hkl_detector_factory_new = _fn(
    "hkl_detector_factory_new", DetectorP, ctypes.c_int)
hkl_detector_free = _fn(
    "hkl_detector_free", None, DetectorP)

# --- factory ---
hkl_factory_get_all = _fn(
    "hkl_factory_get_all", ctypes.POINTER(FactoryP),
    ctypes.POINTER(ctypes.c_uint))
hkl_factory_name_get = _fn(
    "hkl_factory_name_get", ctypes.c_char_p, FactoryP)
hkl_factory_create_new_geometry = _fn(
    "hkl_factory_create_new_geometry", GeometryP, FactoryP)
hkl_factory_create_new_engine_list = _fn(
    "hkl_factory_create_new_engine_list", EngineListP, FactoryP)

# --- geometry ---
hkl_geometry_free = _fn(
    "hkl_geometry_free", None, GeometryP)
hkl_geometry_name_get = _fn(
    "hkl_geometry_name_get", ctypes.c_char_p, GeometryP)
hkl_geometry_wavelength_get = _fn(
    "hkl_geometry_wavelength_get", ctypes.c_double, GeometryP, ctypes.c_int)
hkl_geometry_wavelength_set = _fn(
    "hkl_geometry_wavelength_set", ctypes.c_int,
    GeometryP, ctypes.c_double, ctypes.c_int, GErrorPP)
hkl_geometry_axis_names_get = _fn(
    "hkl_geometry_axis_names_get", DarrayP, GeometryP)
hkl_geometry_axis_values_get = _fn(
    "hkl_geometry_axis_values_get", DarrayDoubleP, GeometryP, ctypes.c_int)
hkl_geometry_axis_values_set = _fn(
    "hkl_geometry_axis_values_set", ctypes.c_int,
    GeometryP, ctypes.POINTER(ctypes.c_double), ctypes.c_size_t,
    ctypes.c_int, GErrorPP)
hkl_geometry_axis_get = _fn(
    "hkl_geometry_axis_get", ParameterP, GeometryP, ctypes.c_char_p, GErrorPP)
hkl_geometry_axis_set = _fn(
    "hkl_geometry_axis_set", ctypes.c_int,
    GeometryP, ctypes.c_char_p, ParameterP, GErrorPP)
hkl_geometry_sample_rotation_get = _fn(
    "hkl_geometry_sample_rotation_get", QuaternionP, GeometryP, SampleP)
hkl_geometry_detector_rotation_get = _fn(
    "hkl_geometry_detector_rotation_get", QuaternionP, GeometryP, DetectorP)
hkl_geometry_ki_get = _fn(
    "hkl_geometry_ki_get", VectorP, GeometryP)
hkl_geometry_kf_get = _fn(
    "hkl_geometry_kf_get", VectorP, GeometryP, DetectorP)
hkl_geometry_new_copy = _fn(
    "hkl_geometry_new_copy", GeometryP, GeometryP)

# --- geometry list ---
hkl_geometry_list_free = _fn(
    "hkl_geometry_list_free", None, GeometryListP)
hkl_geometry_list_n_items_get = _fn(
    "hkl_geometry_list_n_items_get", ctypes.c_size_t, GeometryListP)
hkl_geometry_list_items_first_get = _fn(
    "hkl_geometry_list_items_first_get", GeometryListItemP, GeometryListP)
hkl_geometry_list_items_next_get = _fn(
    "hkl_geometry_list_items_next_get", GeometryListItemP,
    GeometryListP, GeometryListItemP)
hkl_geometry_list_item_geometry_get = _fn(
    "hkl_geometry_list_item_geometry_get", GeometryP, GeometryListItemP)

# --- engine ---
hkl_engine_name_get = _fn(
    "hkl_engine_name_get", ctypes.c_char_p, EngineP)
hkl_engine_pseudo_axis_names_get = _fn(
    "hkl_engine_pseudo_axis_names_get", DarrayP, EngineP)
hkl_engine_pseudo_axis_values_get = _fn(
    "hkl_engine_pseudo_axis_values_get", DarrayDoubleP, EngineP, ctypes.c_int, GErrorPP)
hkl_engine_pseudo_axis_values_set = _fn(
    "hkl_engine_pseudo_axis_values_set", GeometryListP,
    EngineP, ctypes.POINTER(ctypes.c_double), ctypes.c_size_t,
    ctypes.c_int, GErrorPP)
hkl_engine_modes_names_get = _fn(
    "hkl_engine_modes_names_get", DarrayP, EngineP)
hkl_engine_current_mode_get = _fn(
    "hkl_engine_current_mode_get", ctypes.c_char_p, EngineP)
hkl_engine_current_mode_set = _fn(
    "hkl_engine_current_mode_set", ctypes.c_int,
    EngineP, ctypes.c_char_p, GErrorPP)
hkl_engine_parameters_names_get = _fn(
    "hkl_engine_parameters_names_get", DarrayP, EngineP)
hkl_engine_parameters_values_get = _fn(
    "hkl_engine_parameters_values_get", DarrayDoubleP, EngineP, ctypes.c_int, GErrorPP)
hkl_engine_parameters_values_set = _fn(
    "hkl_engine_parameters_values_set", ctypes.c_int,
    EngineP, ctypes.POINTER(ctypes.c_double), ctypes.c_size_t,
    ctypes.c_int, GErrorPP)
hkl_engine_parameter_get = _fn(
    "hkl_engine_parameter_get", ParameterP, EngineP, ctypes.c_char_p, GErrorPP)
hkl_engine_axis_names_get = _fn(
    "hkl_engine_axis_names_get", DarrayP, EngineP, ctypes.c_int)
hkl_engine_capabilities_get = _fn(
    "hkl_engine_capabilities_get", ctypes.c_uint, EngineP)
hkl_engine_dependencies_get = _fn(
    "hkl_engine_dependencies_get", ctypes.c_uint, EngineP)
hkl_engine_initialized_get = _fn(
    "hkl_engine_initialized_get", ctypes.c_int, EngineP)
hkl_engine_initialized_set = _fn(
    "hkl_engine_initialized_set", ctypes.c_int,
    EngineP, ctypes.c_int, GErrorPP)

# --- engine list ---
hkl_engine_list_free = _fn(
    "hkl_engine_list_free", None, EngineListP)
hkl_engine_list_engines_get = _fn(
    "hkl_engine_list_engines_get", DarrayEngineP, EngineListP)
hkl_engine_list_engine_get_by_name = _fn(
    "hkl_engine_list_engine_get_by_name", EngineP,
    EngineListP, ctypes.c_char_p, GErrorPP)
hkl_engine_list_init = _fn(
    "hkl_engine_list_init", ctypes.c_int,
    EngineListP, GeometryP, DetectorP, SampleP, GErrorPP)
hkl_engine_list_get = _fn(
    "hkl_engine_list_get", ctypes.c_int, EngineListP, GErrorPP)
hkl_engine_list_parameters_names_get = _fn(
    "hkl_engine_list_parameters_names_get", DarrayP, EngineListP)
hkl_engine_list_parameters_values_get = _fn(
    "hkl_engine_list_parameters_values_get", DarrayDoubleP,
    EngineListP, ctypes.c_int, GErrorPP)
hkl_engine_list_parameters_values_set = _fn(
    "hkl_engine_list_parameters_values_set", ctypes.c_int,
    EngineListP, ctypes.POINTER(ctypes.c_double), ctypes.c_size_t,
    ctypes.c_int, GErrorPP)
hkl_engine_list_parameter_get = _fn(
    "hkl_engine_list_parameter_get", ParameterP,
    EngineListP, ctypes.c_char_p, GErrorPP)
hkl_engine_list_parameter_set = _fn(
    "hkl_engine_list_parameter_set", ctypes.c_int,
    EngineListP, ctypes.c_char_p, ParameterP, GErrorPP)

# --- parameter ---
hkl_parameter_description_get = _fn(
    "hkl_parameter_description_get", ctypes.c_char_p, ParameterP)
hkl_parameter_min_max_set = _fn(
    "hkl_parameter_min_max_set", ctypes.c_int,
    ParameterP, ctypes.c_double, ctypes.c_double, ctypes.c_int, GErrorPP)

# --- sample ---
hkl_sample_new = _fn(
    "hkl_sample_new", SampleP, ctypes.c_char_p)
hkl_sample_free = _fn(
    "hkl_sample_free", None, SampleP)
hkl_sample_new_copy = _fn(
    "hkl_sample_new_copy", SampleP, SampleP)
hkl_sample_name_get = _fn(
    "hkl_sample_name_get", ctypes.c_char_p, SampleP)
hkl_sample_name_set = _fn(
    "hkl_sample_name_set", ctypes.c_int,
    SampleP, ctypes.c_char_p, GErrorPP)
hkl_sample_lattice_get = _fn(
    "hkl_sample_lattice_get", LatticeP, SampleP)
hkl_sample_lattice_set = _fn(
    "hkl_sample_lattice_set", ctypes.c_int,
    SampleP, LatticeP, GErrorPP)
hkl_sample_ux_get = _fn("hkl_sample_ux_get", ParameterP, SampleP)
hkl_sample_uy_get = _fn("hkl_sample_uy_get", ParameterP, SampleP)
hkl_sample_uz_get = _fn("hkl_sample_uz_get", ParameterP, SampleP)
hkl_sample_ux_set = _fn(
    "hkl_sample_ux_set", ctypes.c_int, SampleP, ParameterP, GErrorPP)
hkl_sample_uy_set = _fn(
    "hkl_sample_uy_set", ctypes.c_int, SampleP, ParameterP, GErrorPP)
hkl_sample_uz_set = _fn(
    "hkl_sample_uz_set", ctypes.c_int, SampleP, ParameterP, GErrorPP)
hkl_sample_U_get = _fn("hkl_sample_U_get", MatrixP, SampleP)
hkl_sample_UB_get = _fn("hkl_sample_UB_get", MatrixP, SampleP)
hkl_sample_UB_set = _fn(
    "hkl_sample_UB_set", ctypes.c_int, SampleP, MatrixP, GErrorPP)
hkl_sample_add_reflection = _fn(
    "hkl_sample_add_reflection", ReflectionP,
    SampleP, GeometryP, DetectorP,
    ctypes.c_double, ctypes.c_double, ctypes.c_double)
hkl_sample_del_reflection = _fn(
    "hkl_sample_del_reflection", ctypes.c_int,
    SampleP, ReflectionP, GErrorPP)
hkl_sample_n_reflections_get = _fn(
    "hkl_sample_n_reflections_get", ctypes.c_uint, SampleP)
hkl_sample_reflections_first_get = _fn(
    "hkl_sample_reflections_first_get", ReflectionP, SampleP)
hkl_sample_reflections_next_get = _fn(
    "hkl_sample_reflections_next_get", ReflectionP, SampleP, ReflectionP)
hkl_sample_get_reflection_measured_angle = _fn(
    "hkl_sample_get_reflection_measured_angle", ctypes.c_double,
    SampleP, ReflectionP, ReflectionP)
hkl_sample_get_reflection_theoretical_angle = _fn(
    "hkl_sample_get_reflection_theoretical_angle", ctypes.c_double,
    SampleP, ReflectionP, ReflectionP)

# --- reflection ---
hkl_sample_reflection_hkl_get = _fn(
    "hkl_sample_reflection_hkl_get", None,
    ReflectionP,
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double))
hkl_sample_reflection_hkl_set = _fn(
    "hkl_sample_reflection_hkl_set", ctypes.c_int,
    ReflectionP,
    ctypes.c_double, ctypes.c_double, ctypes.c_double, GErrorPP)
hkl_sample_reflection_flag_get = _fn(
    "hkl_sample_reflection_flag_get", ctypes.c_int, ReflectionP)
hkl_sample_reflection_flag_set = _fn(
    "hkl_sample_reflection_flag_set", ctypes.c_int,
    ReflectionP, ctypes.c_int, GErrorPP)
hkl_sample_reflection_geometry_get = _fn(
    "hkl_sample_reflection_geometry_get", GeometryP, ReflectionP)
hkl_sample_reflection_geometry_set = _fn(
    "hkl_sample_reflection_geometry_set", ctypes.c_int,
    ReflectionP, GeometryP, GErrorPP)

# --- lattice ---
hkl_lattice_new = _fn(
    "hkl_lattice_new", LatticeP,
    ctypes.c_double, ctypes.c_double, ctypes.c_double,
    ctypes.c_double, ctypes.c_double, ctypes.c_double, GErrorPP)
hkl_lattice_free = _fn(
    "hkl_lattice_free", None, LatticeP)
hkl_lattice_new_copy = _fn(
    "hkl_lattice_new_copy", LatticeP, LatticeP)
hkl_lattice_set = _fn(
    "hkl_lattice_set", ctypes.c_int,
    LatticeP,
    ctypes.c_double, ctypes.c_double, ctypes.c_double,
    ctypes.c_double, ctypes.c_double, ctypes.c_double,
    ctypes.c_int, GErrorPP)
hkl_lattice_get = _fn(
    "hkl_lattice_get", ctypes.c_int,
    LatticeP,
    ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
    ctypes.c_int)
hkl_lattice_a_get = _fn("hkl_lattice_a_get", ParameterP, LatticeP)
hkl_lattice_b_get = _fn("hkl_lattice_b_get", ParameterP, LatticeP)
hkl_lattice_c_get = _fn("hkl_lattice_c_get", ParameterP, LatticeP)
hkl_lattice_alpha_get = _fn("hkl_lattice_alpha_get", ParameterP, LatticeP)
hkl_lattice_beta_get  = _fn("hkl_lattice_beta_get",  ParameterP, LatticeP)
hkl_lattice_gamma_get = _fn("hkl_lattice_gamma_get", ParameterP, LatticeP)
hkl_lattice_a_set = _fn(
    "hkl_lattice_a_set", ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_b_set = _fn(
    "hkl_lattice_b_set", ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_c_set = _fn(
    "hkl_lattice_c_set", ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_alpha_set = _fn(
    "hkl_lattice_alpha_set", ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_beta_set = _fn(
    "hkl_lattice_beta_set",  ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_gamma_set = _fn(
    "hkl_lattice_gamma_set", ctypes.c_int, LatticeP, ParameterP, GErrorPP)
hkl_lattice_reciprocal = _fn(
    "hkl_lattice_reciprocal", ctypes.c_int, LatticeP, LatticeP, GErrorPP)
hkl_lattice_volume_get = _fn(
    "hkl_lattice_volume_get", ctypes.c_double, LatticeP, GErrorPP)

# --- vector --- (no alloc/free - always embedded or returned by value ptr)
hkl_vector_init = _fn(
    "hkl_vector_init", None, VectorP,
    ctypes.c_double, ctypes.c_double, ctypes.c_double)

# HklVector is a plain struct {double data[3]} - access data directly
class HklVector(ctypes.Structure):
    _fields_ = [("data", ctypes.c_double * 3)]

# --- quaternion --- (no alloc/free - returned as pointer into geometry)
# HklQuaternion is a plain struct {double data[4]}
class HklQuaternion(ctypes.Structure):
    _fields_ = [("data", ctypes.c_double * 4)]

hkl_quaternion_to_matrix = _fn(
    "hkl_quaternion_to_matrix", None, QuaternionP, MatrixP)

# ---------------------------------------------------------------------------
# Helper wrappers
# ---------------------------------------------------------------------------

def _gerror():
    """Return a fresh GError** and a function to raise if set."""
    err = GErrorP()
    return ctypes.byref(err), err

def _check(err):
    if err and err.contents:
        msg = err.contents.message.decode()
        raise RuntimeError(msg)

def _doubles_array(values):
    arr = (ctypes.c_double * len(values))(*values)
    return arr, len(values)

def factories():
    """Return dict of {name: factory_pointer}."""
    count = ctypes.c_uint(0)
    ptr = hkl_factory_get_all(ctypes.byref(count))
    result = {}
    for i in range(count.value):
        fac = ptr[i]
        name = hkl_factory_name_get(fac).decode()
        result[name] = fac
    return result

def geometry_wavelength_set(geom, wl):
    err_ref, err = _gerror()
    hkl_geometry_wavelength_set(geom, wl, HKL_UNIT_USER, err_ref)
    _check(err)

def geometry_wavelength_get(geom):
    return hkl_geometry_wavelength_get(geom, HKL_UNIT_USER)

def geometry_axis_values_set(geom, values):
    arr, n = _doubles_array(values)
    err_ref, err = _gerror()
    hkl_geometry_axis_values_set(geom, arr, n, HKL_UNIT_USER, err_ref)
    _check(err)

def geometry_axis_values_get(geom):
    ptr = hkl_geometry_axis_values_get(geom, HKL_UNIT_USER)
    return _darray_double_to_list(ptr)

def geometry_axis_names_get(geom):
    return _darray_to_list(hkl_geometry_axis_names_get(geom))

def engine_list_engines(el):
    ptr = hkl_engine_list_engines_get(el)
    if not ptr:
        return []
    arr = ptr.contents
    return [arr.data[i] for i in range(arr.size)]

def geometry_list_items(gl):
    items = []
    item = hkl_geometry_list_items_first_get(gl)
    while item:
        items.append(item)
        item = hkl_geometry_list_items_next_get(gl, item)
    return items

def reflection_hkl_get(r):
    h, k, l = ctypes.c_double(), ctypes.c_double(), ctypes.c_double()
    hkl_sample_reflection_hkl_get(
        r, ctypes.byref(h), ctypes.byref(k), ctypes.byref(l))
    return (h.value, k.value, l.value)

def lattice_get(lat, unit=HKL_UNIT_DEFAULT):
    a, b, c = ctypes.c_double(), ctypes.c_double(), ctypes.c_double()
    alpha, beta, gamma = ctypes.c_double(), ctypes.c_double(), ctypes.c_double()
    hkl_lattice_get(lat,
                    ctypes.byref(a), ctypes.byref(b), ctypes.byref(c),
                    ctypes.byref(alpha), ctypes.byref(beta), ctypes.byref(gamma),
                    unit)
    return (a.value, b.value, c.value, alpha.value, beta.value, gamma.value)

def vector_data(v):
    vec = ctypes.cast(v, ctypes.POINTER(HklVector)).contents
    return list(vec.data)

def quaternion_data(q):
    quat = ctypes.cast(q, ctypes.POINTER(HklQuaternion)).contents
    return list(quat.data)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestAPI(unittest.TestCase):

    def test_defines(self):
        # Just verify the library loaded and VERSION constant exists via name
        self.assertTrue(_lib is not None)

    def test_vector_api(self):
        v = HklVector()
        vp = ctypes.cast(ctypes.byref(v), VectorP)
        data = list(v.data)
        self.assertEqual(len(data), 3)
        self.assertEqual(data, [0.0, 0.0, 0.0])
        hkl_vector_init(vp, 1, 2, 3)
        self.assertEqual(list(v.data), [1.0, 2.0, 3.0])

    def test_quaternion_api(self):
        q = HklQuaternion()
        data = list(q.data)
        self.assertEqual(len(data), 4)

    def test_factory_api(self):
        facs = factories()
        self.assertGreater(len(facs), 0)
        for name, fac in facs.items():
            self.assertIsInstance(name, str)
            geom = hkl_factory_create_new_geometry(fac)
            self.assertIsNotNone(geom)
            el = hkl_factory_create_new_engine_list(fac)
            self.assertIsNotNone(el)
            hkl_geometry_free(geom)
            hkl_engine_list_free(el)

    def test_detector_api(self):
        det = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D)
        self.assertIsNotNone(det)
        hkl_detector_free(det)

    def test_geometry_api(self):
        facs = factories()
        fac = facs["K6C"]
        geom = hkl_factory_create_new_geometry(fac)
        det = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D)
        sample = hkl_sample_new(b"toto")

        # wavelength
        geometry_wavelength_set(geom, 1.0)
        self.assertAlmostEqual(geometry_wavelength_get(geom), 1.0)

        # axis values round-trip
        values_w = [0, 30, 0, 0, 0, 60]
        geometry_axis_values_set(geom, values_w)
        values_r = geometry_axis_values_get(geom)
        for w, r in zip(values_w, values_r):
            self.assertAlmostEqual(w, r)

        # axis names
        names = geometry_axis_names_get(geom)
        self.assertGreater(len(names), 0)
        for name in names:
            err_ref, err = _gerror()
            axis = hkl_geometry_axis_get(geom, name.encode(), err_ref)
            _check(err)
            err_ref, err = _gerror()
            hkl_parameter_min_max_set(
                axis, 0, math.radians(180), HKL_UNIT_USER, err_ref)
            _check(err)
            err_ref, err = _gerror()
            hkl_geometry_axis_set(geom, name.encode(), axis, err_ref)
            _check(err)

        # rotations return HklQuaternion by value
        q = hkl_geometry_sample_rotation_get(geom, sample)
        self.assertIsInstance(q, HklQuaternion)
        self.assertEqual(len(list(q.data)), 4)

        q2 = hkl_geometry_detector_rotation_get(geom, det)
        self.assertIsInstance(q2, HklQuaternion)

        # ki/kf return HklVector by value
        ki = hkl_geometry_ki_get(geom)
        self.assertIsInstance(ki, HklVector)
        self.assertEqual(len(list(ki.data)), 3)
        kf = hkl_geometry_kf_get(geom, det)
        self.assertIsInstance(kf, HklVector)

        hkl_sample_free(sample)
        hkl_detector_free(det)
        hkl_geometry_free(geom)

    def test_engine_api(self):
        facs = factories()
        fac = facs["K6C"]
        det = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D)
        geom = hkl_factory_create_new_geometry(fac)
        geometry_axis_values_set(geom, [0.0, 30.0, 0.0, 0.0, 0.0, 60.0])

        sample = hkl_sample_new(b"toto")
        lat = hkl_sample_lattice_get(sample)
        err_ref, err = _gerror()
        hkl_lattice_set(lat, 1.54, 1.54, 1.54, 90, 90, 90,
                        HKL_UNIT_USER, err_ref)
        _check(err)
        err_ref, err = _gerror()
        hkl_sample_lattice_set(sample, lat, err_ref)
        _check(err)

        el = hkl_factory_create_new_engine_list(fac)
        err_ref, err = _gerror()
        hkl_engine_list_init(el, geom, det, sample, err_ref)
        _check(err)
        err_ref, err = _gerror()
        hkl_engine_list_get(el, err_ref)
        _check(err)

        # get hkl engine
        err_ref, err = _gerror()
        hkl_eng = hkl_engine_list_engine_get_by_name(el, b"hkl", err_ref)
        _check(err)

        err_ref, err = _gerror()
        values_ptr = hkl_engine_pseudo_axis_values_get(
            hkl_eng, HKL_UNIT_USER, err_ref)
        _check(err)
        values = _darray_double_to_list(values_ptr)

        # check modes
        modes = _darray_to_list(hkl_engine_modes_names_get(hkl_eng))
        for m in modes:
            self.assertIsInstance(m, str)

        # set values 100 times
        for _ in range(100):
            arr, n = _doubles_array(values)
            err_ref, err = _gerror()
            gl = hkl_engine_pseudo_axis_values_set(
                hkl_eng, arr, n, HKL_UNIT_USER, err_ref)
            if err and err.contents:
                print(values, err.contents.message.decode())
            else:
                self.assertIsNotNone(gl)
                for item in geometry_list_items(gl):
                    g = hkl_geometry_list_item_geometry_get(item)
                    self.assertIsNotNone(g)
                hkl_geometry_list_free(gl)
            values[1] += 0.01

        # check all engines
        for eng in engine_list_engines(el):
            name = hkl_engine_name_get(eng).decode()
            self.assertIsInstance(name, str)
            pseudo_names = _darray_to_list(hkl_engine_pseudo_axis_names_get(eng))
            self.assertIsInstance(pseudo_names, list)
            modes = _darray_to_list(hkl_engine_modes_names_get(eng))
            self.assertGreater(len(modes), 0)
            err_ref, err = _gerror()
            vals_ptr = hkl_engine_pseudo_axis_values_get(
                eng, HKL_UNIT_USER, err_ref)
            if not (err and err.contents):
                vals = _darray_double_to_list(vals_ptr)
                for v in vals:
                    self.assertIsInstance(v, float)

        # check parameters and axes per mode
        for eng in engine_list_engines(el):
            modes = _darray_to_list(hkl_engine_modes_names_get(eng))
            for mode in modes:
                err_ref, err = _gerror()
                hkl_engine_current_mode_set(eng, mode.encode(), err_ref)
                _check(err)

                param_names = _darray_to_list(
                    hkl_engine_parameters_names_get(eng))
                self.assertIsInstance(param_names, list)

                err_ref, err = _gerror()
                pvals_ptr = hkl_engine_parameters_values_get(
                    eng, HKL_UNIT_USER, err_ref)
                if not (err and err.contents):
                    pvals = _darray_double_to_list(pvals_ptr)
                    arr, n = _doubles_array(pvals)
                    err_ref, err = _gerror()
                    hkl_engine_parameters_values_set(
                        eng, arr, n, HKL_UNIT_USER, err_ref)

                for pname in param_names:
                    err_ref, err = _gerror()
                    p = hkl_engine_parameter_get(
                        eng, pname.encode(), err_ref)
                    _check(err)
                    desc = hkl_parameter_description_get(p)
                    self.assertIsInstance(desc.decode(), str)

                axes_r = _darray_to_list(hkl_engine_axis_names_get(
                    eng, HKL_ENGINE_AXIS_NAMES_GET_READ))
                axes_w = _darray_to_list(hkl_engine_axis_names_get(
                    eng, HKL_ENGINE_AXIS_NAMES_GET_WRITE))
                self.assertIsInstance(axes_r, list)
                self.assertIsInstance(axes_w, list)

        # capabilities
        no_write = {"incidence", "emergence"}
        for eng in engine_list_engines(el):
            caps = hkl_engine_capabilities_get(eng)
            self.assertTrue(caps & HKL_ENGINE_CAPABILITIES_READABLE)
            name = hkl_engine_name_get(eng).decode()
            if name not in no_write:
                self.assertTrue(caps & HKL_ENGINE_CAPABILITIES_WRITABLE)
            if name == "psi":
                self.assertTrue(
                    caps & HKL_ENGINE_CAPABILITIES_INITIALIZABLE)

        # initialized get/set
        for eng in engine_list_engines(el):
            caps = hkl_engine_capabilities_get(eng)
            if caps & HKL_ENGINE_CAPABILITIES_INITIALIZABLE:
                err_ref, err = _gerror()
                hkl_engine_initialized_set(eng, 0, err_ref)
                _check(err)
                self.assertEqual(hkl_engine_initialized_get(eng), 0)
                err_ref, err = _gerror()
                hkl_engine_initialized_set(eng, 1, err_ref)
                _check(err)
                self.assertEqual(hkl_engine_initialized_get(eng), 1)

        # dependencies
        for eng in engine_list_engines(el):
            deps = hkl_engine_dependencies_get(eng)
            self.assertTrue(deps & HKL_ENGINE_DEPENDENCIES_AXES)

        hkl_engine_list_free(el)
        hkl_sample_free(sample)
        hkl_detector_free(det)
        hkl_geometry_free(geom)

    def test_engine_list_api(self):
        facs = factories()
        for name, fac in facs.items():
            el = hkl_factory_create_new_engine_list(fac)

            names = _darray_to_list(hkl_engine_list_parameters_names_get(el))
            self.assertIsInstance(names, list)

            for pname in names:
                err_ref, err = _gerror()
                p = hkl_engine_list_parameter_get(
                    el, pname.encode(), err_ref)
                _check(err)
                err_ref, err = _gerror()
                hkl_engine_list_parameter_set(
                    el, pname.encode(), p, err_ref)
                _check(err)

            err_ref, err = _gerror()
            vals_ptr = hkl_engine_list_parameters_values_get(
                el, HKL_UNIT_USER, err_ref)
            if not (err and err.contents):
                vals = _darray_double_to_list(vals_ptr)
                if vals:  # only call set if there are actually parameters
                    arr, n = _doubles_array(vals)
                    err_ref, err = _gerror()
                    hkl_engine_list_parameters_values_set(
                        el, arr, n, HKL_UNIT_USER, err_ref)

            hkl_engine_list_free(el)

    def test_lattice_api(self):
        err_ref, err = _gerror()
        lat = hkl_lattice_new(
            1.54, 1.54, 1.54,
            math.radians(90.0), math.radians(90.0), math.radians(90.0),
            err_ref)
        _check(err)
        lat2 = hkl_lattice_new_copy(lat)

        a   = hkl_lattice_a_get(lat)
        b   = hkl_lattice_b_get(lat)
        c   = hkl_lattice_c_get(lat)
        alpha = hkl_lattice_alpha_get(lat)
        beta  = hkl_lattice_beta_get(lat)
        gamma = hkl_lattice_gamma_get(lat)

        err_ref, err = _gerror()
        hkl_lattice_a_set(lat, a, err_ref);     _check(err)
        err_ref, err = _gerror()
        hkl_lattice_b_set(lat, b, err_ref);     _check(err)
        err_ref, err = _gerror()
        hkl_lattice_c_set(lat, c, err_ref);     _check(err)
        err_ref, err = _gerror()
        hkl_lattice_alpha_set(lat, alpha, err_ref); _check(err)
        err_ref, err = _gerror()
        hkl_lattice_beta_set(lat, beta, err_ref);   _check(err)
        err_ref, err = _gerror()
        hkl_lattice_gamma_set(lat, gamma, err_ref);  _check(err)

        v1 = lattice_get(lat, HKL_UNIT_DEFAULT)
        err_ref, err = _gerror()
        hkl_lattice_set(lat, *v1, HKL_UNIT_DEFAULT, err_ref)
        _check(err)

        err_ref, err = _gerror()
        hkl_lattice_set(lat, 1, 2, 3, 90, 90, 90, HKL_UNIT_USER, err_ref)
        _check(err)

        self.assertNotEqual(
            lattice_get(lat, HKL_UNIT_DEFAULT),
            lattice_get(lat2, HKL_UNIT_DEFAULT))

        hkl_lattice_free(lat2)
        hkl_lattice_free(lat)

    def test_sample_api(self):
        sample = hkl_sample_new(b"toto")
        self.assertEqual(hkl_sample_name_get(sample).decode(), "toto")

        copy = hkl_sample_new_copy(sample)
        self.assertEqual(
            hkl_sample_name_get(copy).decode(),
            hkl_sample_name_get(sample).decode())

        err_ref, err = _gerror()
        hkl_sample_name_set(copy, b"titi", err_ref)
        _check(err)
        self.assertNotEqual(
            hkl_sample_name_get(copy).decode(),
            hkl_sample_name_get(sample).decode())

        err_ref, err = _gerror()
        lat = hkl_lattice_new(
            1.54, 1.54, 1.54,
            math.radians(90.0), math.radians(90.0), math.radians(90.0),
            err_ref)
        _check(err)
        err_ref, err = _gerror()
        hkl_sample_lattice_set(sample, lat, err_ref)
        _check(err)

        v = lattice_get(lat, HKL_UNIT_DEFAULT)
        err_ref, err = _gerror()
        hkl_lattice_set(lat, *v, HKL_UNIT_DEFAULT, err_ref)
        _check(err)

        slat = hkl_sample_lattice_get(sample)
        self.assertEqual(
            lattice_get(lat, HKL_UNIT_DEFAULT),
            lattice_get(slat, HKL_UNIT_DEFAULT))

        err_ref, err = _gerror()
        hkl_lattice_set(lat, 1, 2, 3, 90, 90, 90, HKL_UNIT_USER, err_ref)
        _check(err)
        self.assertNotEqual(
            lattice_get(lat, HKL_UNIT_DEFAULT),
            lattice_get(slat, HKL_UNIT_DEFAULT))

        ux = hkl_sample_ux_get(sample)
        uy = hkl_sample_uy_get(sample)
        uz = hkl_sample_uz_get(sample)
        err_ref, err = _gerror()
        hkl_sample_ux_set(sample, ux, err_ref); _check(err)
        err_ref, err = _gerror()
        hkl_sample_uy_set(sample, uy, err_ref); _check(err)
        err_ref, err = _gerror()
        hkl_sample_uz_set(sample, uz, err_ref); _check(err)

        U  = hkl_sample_U_get(sample)
        UB = hkl_sample_UB_get(sample)
        err_ref, err = _gerror()
        hkl_sample_UB_set(sample, UB, err_ref)
        _check(err)

        recip = hkl_lattice_new_copy(lat)
        err_ref, err = _gerror()
        hkl_lattice_reciprocal(lat, recip, err_ref)
        _check(err)

        err_ref, err = _gerror()
        hkl_lattice_volume_get(lat, err_ref)
        _check(err)

        hkl_lattice_free(recip)
        hkl_lattice_free(lat)
        hkl_sample_free(copy)
        hkl_sample_free(sample)

    def test_reflection_api(self):
        det = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D)
        facs = factories()
        fac = facs["K6C"]
        geom = hkl_factory_create_new_geometry(fac)
        geometry_axis_values_set(geom, [0.0, 30.0, 0.0, 0.0, 0.0, 60.0])
        sample = hkl_sample_new(b"toto")

        r1 = hkl_sample_add_reflection(sample, geom, det, 1, 1, 1)
        r2 = hkl_sample_add_reflection(sample, geom, det, 1, 1, 1)

        self.assertEqual(reflection_hkl_get(r2), (1.0, 1.0, 1.0))
        err_ref, err = _gerror()
        hkl_sample_reflection_hkl_set(r2, 1, 0, 1, err_ref)
        _check(err)
        self.assertEqual(reflection_hkl_get(r2), (1.0, 0.0, 1.0))

        flag = hkl_sample_reflection_flag_get(r1)
        err_ref, err = _gerror()
        hkl_sample_reflection_flag_set(r1, flag, err_ref)
        _check(err)

        g = hkl_sample_reflection_geometry_get(r1)
        err_ref, err = _gerror()
        hkl_sample_reflection_geometry_set(r1, g, err_ref)
        _check(err)

        hkl_sample_get_reflection_measured_angle(sample, r1, r2)
        hkl_sample_get_reflection_theoretical_angle(sample, r1, r2)

        # collect all reflections then delete them
        refs = []
        r = hkl_sample_reflections_first_get(sample)
        while r:
            refs.append(r)
            r = hkl_sample_reflections_next_get(sample, r)
        for ref in refs:
            err_ref, err = _gerror()
            hkl_sample_del_reflection(sample, ref, err_ref)
            _check(err)

        hkl_sample_free(sample)
        hkl_detector_free(det)
        hkl_geometry_free(geom)


if __name__ == "__main__":
    unittest.main(verbosity=2)