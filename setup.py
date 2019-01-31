from setuptools import setup, find_packages
from setuptools.command.test import test as TestCommand
from setuptools.extension import Extension
from Cython.Build import cythonize
import numpy
from sklearn_oblique_tree import __version__, __authors__
import sys

packages = find_packages()

extensions = [
     Extension(
         "indictrans._decode.beamsearch",
         [
             "indictrans/_decode/beamsearch.pyx"
         ],
         include_dirs=[numpy.get_include()]
     ),
     Extension(
         "indictrans._decode.viterbi",
         [
             "indictrans/_decode/viterbi.pyx"
         ],
         include_dirs=[numpy.get_include()]
     )

    ]


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
        'numpy',
        'Cython'
    ],
    ext_modules=cythonize(extensions),

    #testing
    tests_require=["pytest"],
    cmdclass={"pytest": PyTest},

    #include external data (murphy's oc1 implementation)
    include_package_data=True,
    zip_safe=False

)
