import os
import sys

environment = Environment()

def PrepareCompilerGCC(env):
  env.Append(CCFLAGS = "-pedantic -Werror -Wall -fstrict-enums -fno-threadsafe-statics -g ", CXXFLAGS = "  -Wsign-promo -fno-rtti -fno-exceptions ", CFLAGS = " -ansi ", LINKFLAGS = " -g ")

def PrepareEnvironmentUNIX(env):
  env.Append(CPPDEFINES = ["USE_BSDSOCK"])

PrepareCompilerGCC(environment)
PrepareEnvironmentUNIX(environment)

libfjnet = SConscript(dirs = ['libfjnet'], exports = ['environment'])
libfjirc = SConscript(dirs = ['libfjirc'], exports = ['environment'])

environment.Append(CPPPATH = [os.path.join(os.getcwd(), 'libfjirc'), os.path.join(os.getcwd(), 'libfjnet'), os.getcwd()])

SConscript(dirs = ['kashyyyk'], exports = ['environment', 'libfjirc', 'libfjnet'])
