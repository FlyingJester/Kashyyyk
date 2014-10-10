import os
import sys

environment = Environment()

gcc_ccflags = "-pedantic -Werror -Wall -fstrict-enums -fno-threadsafe-statics -g "

def PrepareCompilerGPP(env):
  env.Append(CXXFLAGS = "  -Wsign-promo -fno-rtti -fno-exceptions " + gcc_ccflags, LINKFLAGS = " -g ")

def PrepareCompilerGCC(env):
  env.Append(CFLAGS = " -ansi "+gcc_ccflags, LINKFLAGS = " -g ")

def PrepareCompilerMSVC(env):
  env.Append(CFLAGS = "/O2 /EHsc /Zi /MDd")
def PrepareCompilerMSVCpp(env):
  env.Append(CXXFLAGS = "/O2 /EHsc /Zi /MDd")

def PrepareEnvironmentUNIX(env):
  env.Append(CPPDEFINES = ["USE_BSDSOCK"])
def PrepareEnvironmentWin(env):
  env.Append(LIBPATH = [os.path.join(os.getcwd(), "lib")], LIBS = ["Gdi32.lib", "User32.lib", "Ole32.lib", "Advapi32.lib", "Shell32.lib", "Ws2_32.lib"], CPPDEFINES = ["WIN32"])
  env.Append(CPPDEFINES = ["USE_WINSOCK"])

if os.getenv('CC', 'none') != 'none':
  print "using CC ", os.environ.get('CC')
  environment.Replace(CC = os.environ.get('CC'))
elif ARGUMENTS.get('CC', 'none') != 'none':
  print "using CC ", ARGUMENTS.get('CC', 'none')
  environment.Replace(CC = ARGUMENTS.get('CC', 'none'))
else:
  if sys.platform.startswith('linux') or sys.platform == 'darwin':
    PrepareCompilerGCC(environment)
    environment.Replace(CC = 'cc')
  elif sys.platform.startswith('win'):
    PrepareCompilerMSVC(environment)


if os.getenv('CXX', 'none') != 'none':
  print "using CXX ", os.environ.get('CXX')
  environment.Replace(CXX = os.environ.get('CXX'))
elif ARGUMENTS.get('CXX', 'none') != 'none':
  print "using CXX ", ARGUMENTS.get('CXX', 'none')
  environment.Replace(CXX = ARGUMENTS.get('CXX', 'none'))
else:
  if sys.platform.startswith('linux') or sys.platform == 'darwin':
    PrepareCompilerGPP(environment)
    environment.Replace(CXX = 'c++')
  elif sys.platform.startswith('win'):
    PrepareCompilerMSVCpp(environment)

if os.name=='posix' or ARGUMENTS.get('posix', '0') == '1':
  PrepareEnvironmentUNIX(environment)
elif sys.platform.startswith('win'):
  PrepareEnvironmentWin(environment)

libfjnet = SConscript(dirs = ['libfjnet'], exports = ['environment'])
libfjirc = SConscript(dirs = ['libfjirc'], exports = ['environment'])

environment.Append(CPPPATH = [os.path.join(os.getcwd(), 'libfjirc'), os.path.join(os.getcwd(), 'libfjnet'), os.getcwd()])

SConscript(dirs = ['kashyyyk'], exports = ['environment', 'libfjirc', 'libfjnet'])
