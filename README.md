# Building hkl on a Mac

This repo contains an updated fork of the HKL Library developed at the ESRF to allow building on a Mac-based System.

This is a **Minimal installation** that only includes the **CORE** hkl library. All other features are not accounted for.

This installation was completed to allow Mac users to utilize all hkl-based solvers in the hklpy2 package.

## Confirmed Case

This has only been tested on my personal system, a Macbook Air M1 8GB with MacOS Sonoma 14.5. This uses an Apple Silicon chip.

## Installation Instructions

### Latest Release

A complete set of library files are regularly built with an automated repository workflow targetting the latest MacOS system version (typically ARM on the latest macOS version). To install from these files, follow these steps:

1. Download homebrew if you haven't already. This package manager makes all the steps much easier.

```{bash}
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. Install the following prerequisite packages on your system:

```{bash}
brew install pygobject3 gobject-introspection glib gtk3
```

2. Go the `Releases > Latest Build` page and download the files.
3. Export the files to the appropriate folders. Create the folders if necessary. **IMPORTANT:** This will require sudo privileges!

| Files | Destination Folder |
| --- | --- |
| libhkl.a<br>libhkl.5.dylib<br>libhkl.dylib<br>libhkl.la | /usr/local/lib |
| Hkl-5.0.typelib | /usr/local/lib/ | /usr/local/lib/girepository-1.0 |

4. On your Python work environment, install the `pygobject` package.
```{bash}
pip install pygobject
# or with uv
uv add pygobject
```

5. The `hkl` package should then be installed.

### Local Build and Deployment

1. Download this repository and open it if you haven't so already:

```{bash}
git clone https://github.com/Gabbyton/hkl-mac
cd hkl-mac
```

2. Download homebrew if you haven't already. This package manager makes all the steps much easier.

```{bash}
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

3. The following packages are required for **MINIMAL** installation.

```{bash}
brew install gsl gtk-doc hdf5 glib gsl autoconf automake libtool gobject-introspection pkg-config autoconf-archive cglm
```

4. Run the following commands:

```{bash}
./autogen.sh
./configure --disable-gui --disable-binoculars --enable-introspection=yes --disable-hkl-doc
make
make install
```

**NOTE:** To reset the build run:
```{bash}
make clear
```

5. After `make install`, the relevant files will be installed in `/usr/local/lib` and `/usr/local/lib/girepository-1.0`.

# Running Examples

An updated example can be found in `tests/binding/python.py`. To run the example properly, your Mac needs to know where to find the dylib files. To do so, run the following or add the lines to your `~/.zshrc`:

```{bash}
export GI_TYPELIB_PATH=/usr/local/lib/girepository-1.0
export DYLD_LIBRARY_PATH=/usr/local/lib
```

If running on your base environment, just go ahead and run your script as usual.

if running on a python environment, an additional package is required for the `gi` package:

```{bash}
pip install PyGObject
```

# Licenses

This work was directly copied and modified from the original HKL library codebase developed at the SOLEIL Synchrotron at the ESRF. Please defer to all copyright notices on all files.

Limited Modifications Copyright (C) 2026 Gabriel Obsequio Ponon

## Original Copyright Notice:

```{text}
Copyright (C) 2010-2019, 2025
Synchrotron SOLEIL 
L'Orme des Merisiers Saint-Aubin
BP 48 91192 GIF-sur-YVETTE CEDEX
```

## Original Authors:

Authors: Picca Frédéric-Emmanuel, Email: <picca@synchrotron-soleil.fr>
Oussama Sboui, Email: <sboui@synchrotron-soleil.fr>

Original: [[Repo]](git://repo.or.cz/hkl.git) | [[Homepage]](https://people.debian.org/~picca/hkl/hkl.html#orgb4f97f6)