import os.path
import os
import platform
from BuildID import *

############### functions ###############
def filtered_glob(env, pattern, omit=[], ondisk=True, source=False, strings=False):
    return filter(lambda f: os.path.basename(f.path) not in omit, env.Glob(pattern))

def prefix_with(prefix):
    return lambda x: prefix + x

###
# Set up the CORE environment, which captures as much platform-specific
# information as it possibly can.
###
core_env = Environment(tools=['default','textfile'], CPPPATH=['#include'])
core_env.AddMethod(filtered_glob, "FilteredGlob")

# Find the build type and platform.
build_type = ARGUMENTS.get('mode', 'debug')
build_platform = None

# these include little environment fixups.
if core_env['PLATFORM'] == 'posix':
    build_platform = 'linux'
elif core_env['PLATFORM'] == 'win32':
    core_env['ENV'] = os.environ
    build_platform = 'win'
elif core_env['PLATFORM'] == 'darwin':
    core_env.Replace(CXX = "clang")
    build_platform = 'mac'

###
# Platform specific patches to the environment.
# This includes setting up library paths.
###

#linux paths and options
if build_platform == 'linux':
    # library paths
    # core_env.AppendUnique(LIBPATH = ['#lib/linux'])

    # compiler flags
    core_env.AppendUnique(CCFLAGS = Split('-fPIC -m64 -pthread'))
    core_env.AppendUnique(CXXFLAGS = Split('-std=c++11'))

    if build_type == 'debug':
        # debug, no optimization.
        core_env.AppendUnique(CPPFLAGS = ['-g', '-O0'])

#windows paths and options
elif build_platform == 'win':
    core_env.AppendUnique(LIBPATH = ['#lib/windows'])

iblis_sources = core_env.Glob("#src/*.cpp")
iblis_objects = core_env.Object(iblis_sources)

iblis_library = core_env.Library("iblisVM", iblis_objects)

