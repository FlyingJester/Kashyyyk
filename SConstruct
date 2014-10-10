import os
import sys

environment = Environment()

def PrepareCompilerGCC(env):
  env.Append(CCFLAGS = "-pedantic -Werror -Wall -fstrict-enums -fno-threadsafe-statics -g ", CXXFLAGS = "  -Wsign-promo -fno-rtti -fno-exceptions ", CFLAGS = " -ansi ", LINKFLAGS = " -g ")

def PrepareEnvironmentUNIX(env):
  env.Append(CPPDEFINES = ["USE_BSDSOCK"])

if os.getenv('CC', 'none') != 'none':
  print "using CC ", os.environ.get('CC')
  environment.Replace(CC = os.environ.get('CC'))
if os.getenv('CXX', 'none') != 'none':
  print "using CXX ", os.environ.get('CXX')
  environment.Replace(CXX = os.environ.get('CXX'))

if sys.platform.startswith('linux') or sys.platform == 'darwin' or ARGUMENTS.get('compiler', '0') == 'gcc' or ARGUMENTS.get('compiler', '0') == 'clang':
  PrepareCompilerGCC(environment)

if os.name=='posix' or ARGUMENTS.get('posix', '0') == '1':
  PrepareEnvironmentUNIX(environment)

libfjnet = SConscript(dirs = ['libfjnet'], exports = ['environment'])
libfjirc = SConscript(dirs = ['libfjirc'], exports = ['environment'])

environment.Append(CPPPATH = [os.path.join(os.getcwd(), 'libfjirc'), os.path.join(os.getcwd(), 'libfjnet'), os.getcwd()])

SConscript(dirs = ['kashyyyk'], exports = ['environment', 'libfjirc', 'libfjnet'])
