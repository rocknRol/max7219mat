LIB_DIR = lib

default: pymax7219lib

pymax7219lib: setup.py pymax7219lib.pyx $(LIB_DIR)/libmax7219mat.a
	python3 setup.py build_ext --inplace && rm -f pymax7219.c && rm -Rf build

$(LIB_DIR)/libmax7219mat.a:
	make -C $(LIB_DIR) libmax7219mat.a

clean:
	rm *.so
