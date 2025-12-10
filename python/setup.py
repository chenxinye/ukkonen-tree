from setuptools import setup, Extension
import sys

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11 until it is actually
    installed, so that the ``get_include()`` method can be invoked. """

    def __str__(self):
        import pybind11
        return pybind11.get_include()

ext_modules = [
    Extension(
        'pyukkonen',                  
        ['bindings.cpp'],            
        include_dirs=[
            get_pybind_include(),
            get_pybind_include()
        ],
        language='c++',
        extra_compile_args=['-std=c++17', '-O3'],
    ),
]

setup(
    name='pyukkonen',
    version='0.0.1',                  
    author='Xinye Chen',              
    author_email='xinyechenai@gmail.com', 
    url='https://github.com/chenxinye/ukkonen-tree', 
    description='A high-performance C++ Suffix Tree implementation with Python bindings',
    long_description=open('README.md').read(), 
    long_description_content_type='text/markdown',
    ext_modules=ext_modules,
    zip_safe=False,
    python_requires=">=3.6",
)