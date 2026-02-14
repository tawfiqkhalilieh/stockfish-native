from setuptools import setup, Extension

# module = Extension(
#     "cppsum",
#     sources=["cppsum.cpp"],
#     language="c++"
# )

# setup(
#     name="cppsum",
#     version="0.1",
#     ext_modules=[module]
# )

module = Extension(
    "run_stockfish",  # MUST match PyModuleDef.name
    sources=["run_stockfish.cpp"],
    language="c++"
)

setup(
    name="run_stockfish",
    version="0.1",
    ext_modules=[module]
)

