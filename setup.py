from setuptools import setup


def readme():
    with open('README.md') as f:
        return f.read()


setup(
    name='pyrefo',
    version='0.4',
    description='a fast regex for object',
    long_description=readme(),
    long_description_content_type='text/markdown',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Topic :: Text Processing :: Linguistic',
    ],
    keywords='regex',
    url='http://github.com/yimian/pyrefo',
    author='zhangjinjie',
    author_email='zhangjinjie@yimian.com.cn',
    license='GPLv3+',
    packages=['pyrefo'],
    setup_requires=['cffi>=1.0.0', 'pytest-runner'],
    cffi_modules=['pyrefo/pyrefo_build.py:ffi'],
    install_requires=['cffi>=1.0.0'],
    include_package_data=True,
    tests_require=['pytest'],
    zip_safe=False
)
