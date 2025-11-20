converter=blender_gltf_converter.py

models=soleil-diffabs.yaml soleil-cristal-4c.yaml
glbs=$(models:.yaml=.glb)
blends=$(models:.yaml=.blend)
pngs=$(models:.yaml=.png)

.blend.glb:
	$(BLENDER) -b -P $(converter) -- -mp ./$<

.blend.png:
	$(BLENDER_THUMBNAILER) $< $@

EXTRA_DIST=$(converter) $(models) $(glbs) $(blends) $(pngs)

CLEANFILES=$(glbs) $(pngs)

hkl3ddir = $(pkgdatadir)/hkl3d
hkl3d_DATA = $(EXTRA_DIST)

soleil-diffabs.blend:
	wget https://people.debian.org/~picca/hkl3d/$@ -O $@

soleil-cristal-4c.blend:
	wget https://people.debian.org/~picca/hkl3d/$@ -O $@

export-all-model:
	scp $(blends) people.debian.org:public_html/hkl3d/

import-all-model:
	scp people.debian.org:public_html/hkl3d/*.blend .
	scp -r people.debian.org:Cristal .
