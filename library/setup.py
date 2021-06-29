from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

pymax7219lib_extension = Extension(
    name="pymax7219lib",
    sources=["pymax7219lib.pyx"],
    libraries=["max7219mat"],
    library_dirs=["lib"],
    include_dirs=["lib"]
)

setup(
    name="pymax7219lib",
    ext_modules=cythonize([pymax7219lib_extension])
)
