.blend.glb:
	$(BLENDER) -b -P blender_gltf_converter.py -- -mp ./$<

MODELS=diffabs.blend

EXTRA_DIST=$(MODELS) diffabs.yaml

CLEANFILES=diffabs.glb

hkl3ddir = $(pkgdatadir)/hkl3d
hkl3d_DATA = diffabs.glb diffabs.yaml

export-model:
	scp $(MODELS) people.debian.org:public_html/hkl3d/

import-model:
	scp people.debian.org:public_html/hkl3d/*.blend .
