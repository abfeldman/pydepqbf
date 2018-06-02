from distutils.core import setup, Extension

VERSION = '0.0.1'
PYDEPQBF_MODULE = Extension("pydepqbf",
                            sources=["pydepqbf.c"],
                            libraries=["qdpll", "picosat", "nenofex"])

setup(name="pydepqbf",
      version=VERSION,
      author="Alexander Feldman",
      author_email="alex@llama.gs",
      url="https://github.com/abfeldman/pydepqbf",
      license="MIT",
      classifiers=["Development Status :: 4 - Beta",
                   "Intended Audience :: Developers",
                   "Operating System :: OS Independent",
                   "Programming Language :: C",
                   "Programming Language :: Python :: 2",
                   "Programming Language :: Python :: 3",
                   "Topic :: Scientific/Engineering :: Artificial Intelligence",
                   "Topic :: Utilities"],
      ext_modules=[PYDEPQBF_MODULE],
      py_modules=['test_pydepqbf'],
      description="bindings to depqbf (a QBF solver)",
      long_description=open('README.rst').read())
