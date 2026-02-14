from setuptools import setup, Extension

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

module = Extension(
    "native_stockfish",
    sources=["native_stockfish.cpp"],
    language="c++",
    extra_compile_args=["-std=c++17"], 
)

setup(
    name="native_stockfish",
    version="0.1.0",
    author="Tawfiq Khalilieh",
    author_email="taw.coding@gmail.com",
    description="The fastest client for the Stockfish chess engine using C++ extension.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/tawfiqkhalilieh/stockfish-native",
    ext_modules=[module],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires=">=3.8",
)

