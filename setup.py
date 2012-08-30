from distutils.core import setup, Extension

module1 = Extension('s3p',
                    sources = ['s3pmodule.c', 's3p.c'],
                    include_dirs = ['./'],
                    extra_compile_args = ['--std=c99'])

setup (name = 's3p',
       version = '0.1',
       description = 'The Super Simple Streaming Protocol library.',
       author = "Nicholas Pascucci",
       author_email = "npascut1@gmail.com",
       url = "http://nickpascucci.github.com/S3P/",
       ext_modules = [module1])
