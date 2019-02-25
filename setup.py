from setuptools import setup, find_packages
from setuptools.command.test import test as TestCommand
from setuptools.extension import Extension
#from Cython.Build import cythonize

from sklearn_oblique_tree import __authors__, __version__
import numpy,sys


packages = find_packages()

extensions = [
     Extension("sklearn_oblique_tree.oblique._oblique",["sklearn_oblique_tree/oblique/_oblique.pyx",
                                                        "oc1_source/load_data.c",
                                                        #"oc1_source/train_util.c",
                                                        "oc1_source/perturb.c",
                                                        "oc1_source/classify.c",

                                                        "oc1_source/compute_impurity.c",
                                                        "oc1_source/impurity_measures.c",
                                                        "oc1_source/prune.c",
                                                        "oc1_source/util.c",
                                                       # "oc1_source/classify_util.c",
                                                        "oc1_source/tree_util.c"
                                                        ],
               include_dirs=[numpy.get_include(), '.'],
               extra_compile_args=["-w"]

     )
    ]

#util.c tree_util.c load_data.c perturb.c compute_impurity.c impurity_measures.c classify.c prune.c

def readme():
    with open('README.md') as f:
        return f.read()

class PyTest(TestCommand):
    """
    Custom Test Configuration Class
    Read here for details: https://docs.pytest.org/en/latest/goodpractices.html
    """
    user_options = [("pytest-args=", "a", "Arguments to pass to pytest")]

    def initialize_options(self):
        TestCommand.initialize_options(self)
        self.pytest_args = ""

    def run_tests(self):
        import shlex
        # import here, cause outside the eggs aren't loaded
        import pytest

        errno = pytest.main(shlex.split(self.pytest_args))
        sys.exit(errno)

setup(
    name='sklearn-oblique-tree',
    version=__version__,
    license='GNU GENERAL PUBLIC LICENSE',
    description='a python interface to oblique decision tree implementations',
    long_description=readme(),
    packages=packages,
    url='https://github.com/AndriyMulyar/sklearn-oblique-tree',
    author=__authors__,
    author_email='contact@andriymulyar.com',
    keywords='scikit-learn oblique-classifier-1 oc1 oblique-decision-tree decision-tree',
    classifiers=[
        '( Status :: 4 - Beta',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Programming Language :: Python :: 3.5',
        'Natural Language :: English',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
        'Intended Audience :: Science/Research'
    ],
    install_requires=[
        'scikit-learn>=0.20.0',
        'numpy'
    ],


    #ext_modules=cythonize(extensions, gdb_debug=False),
    ext_modules=extensions,

    #testing
    tests_require=["pytest"],
    cmdclass={"pytest": PyTest},

    #include external data (murphy's oc1 implementation)
    include_package_data=True,
    zip_safe=False

)
