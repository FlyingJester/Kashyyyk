import os
import sys
import commands
import readmegen

if ARGUMENTS.get('pandoc_readme', '0') == '1':
    readmegen.generate()

environment = Environment(ENV=os.environ)

poll_api = 'select'

AddOption('--enable-debug', dest = 'enabledebug', default=True, help=\
"Enables building with debug symbols.\n")

enabledebug = GetOption('enabledebug')

gcc_ccflags = " -pedantic -Werror -Wall "

if enabledebug:
  gcc_ccflags += " -g "

def PrepareCompilerGPP(env):
  print "Preparing g++"
  env.Append(CXXFLAGS = " -std=c++11 -Wsign-promo -fno-rtti -fno-exceptions -fstrict-enums -fno-threadsafe-statics " + gcc_ccflags, CPPPATH =
["/usr/local/include"])
  if enabledebug:
    env.Append(LINKFLAGS = " -g ")

def PrepareCompilerGCC(env):
  print "Preparing gcc"
  env.Append(CFLAGS = " -ansi -Wno-overlength-strings " + gcc_ccflags)
  if enabledebug:
    env.Append(LINKFLAGS = " -g ")

def PrepareCompilerMSVC(env):
  env.Append(CFLAGS = " /EHsc /Zi /MDd /DEBUG ", LINKFLAGS = " /DEBUG /NODEFAULTLIB:MSVCRT ")
def PrepareCompilerMSVCpp(env):
  env.Append(CXXFLAGS = " /EHsc /Zi /MDd /DEBUG ", LINKFLAGS = " /DEBUG /NODEFAULTLIB:MSVCRT ")

def PrepareEnvironmentUNIX(env):
  if sys.platform == 'cygwin':
    env.Append(CPPDEFINES = ["USE_CYGSOCK", "WIN32"], CCFLAGS = " -mwindows ")
  else:
    env.Append(CPPDEFINES = ["USE_BSDSOCK"], LIBPATH=["/usr/local/lib"])
def PrepareEnvironmentWin(env):
  env.Append(LIBPATH = [os.path.join(os.getcwd(), "lib")], LIBS = ["Gdi32.lib", "User32.lib", "Ole32.lib", "Advapi32.lib", "Shell32.lib", "Ws2_32.lib"], CPPDEFINES = ["WIN32"])
  env.Append(CPPDEFINES = ["USE_WINSOCK"])

if os.getenv('CC', 'none') != 'none':
  print "using CC ", os.environ.get('CC')
  environment.Replace(CC = os.environ.get('CC'))
  environment.Append(CFLAGS = os.getenv('CFLAGS', ''))
  if ARGUMENTS.get('gccflags', '0') == '1':
    PrepareCompilerGCC(environment)
  elif ARGUMENTS.get('msvcflags', '0') == '1':
    PrepareCompilerMSVC(environment)
  else:
    print "No preset flags used for CC."

elif ARGUMENTS.get('CC', 'none') != 'none':
  print "using CC ", ARGUMENTS.get('CC', 'none')
  environment.Replace(CC = ARGUMENTS.get('CC', 'none'), CFLAGS = ARGUMENTS.get('CFLAGS', ''))
  if ARGUMENTS.get('gccflags', '0') == '1':
    PrepareCompilerGCC(environment)
  elif ARGUMENTS.get('msvcflags', '0') == '1':
    PrepareCompilerMSVC(environment)
  else:
    print "No preset flags used for CC."

else:
  if sys.platform.startswith('linux') or 'bsd' in sys.platform or sys.platform == 'darwin' or sys.platform == 'cygwin':
    PrepareCompilerGCC(environment)
    environment.Replace(CC = 'cc')
  elif sys.platform.startswith('win'):
    PrepareCompilerMSVC(environment)


if os.getenv('CXX', 'none') != 'none':
  print "using CXX ", os.environ.get('CXX')
  environment.Replace(CXX = os.environ.get('CXX'))
  environment.Append(CXXFLAGS = os.getenv('CXXFLAGS', ''))
  if ARGUMENTS.get('g++flags', '0') == '1':
    PrepareCompilerGPP(environment)
  elif ARGUMENTS.get('msvcflags', '0') == '1':
    PrepareCompilerMSVC(environment)
  else:
    print "No preset flags used for CXX."

elif ARGUMENTS.get('CXX', 'none') != 'none':
  print "using CXX ", ARGUMENTS.get('CXX', 'none')
  environment.Replace(CXX = ARGUMENTS.get('CXX', 'none'), CXXFLAGS = ARGUMENTS.get('CXXFLAGS', ''))
  if ARGUMENTS.get('g++flags', '0') == '1':
    PrepareCompilerGPP(environment)
  elif ARGUMENTS.get('msvcflags', '0') == '1':
    PrepareCompilerMSVC(environment)
  else:
    print "No preset flags used for CXX."

if True:
  if sys.platform.startswith('linux') or 'bsd' in sys.platform or sys.platform == 'darwin' or sys.platform == 'cygwin':
    PrepareCompilerGPP(environment)
    environment.Replace(CXX = 'c++')
  elif sys.platform.startswith('win'):
    PrepareCompilerMSVCpp(environment)
  else:
    print sys.platform
if os.name=='posix' or ARGUMENTS.get('posix', '0') == '1':
  PrepareEnvironmentUNIX(environment)
  conf = Configure(environment)
  if sys.platform == 'darwin':
    environment.Append(CPPDEFINES = ["USE_PIPE_CONCURRENT_QUEUE"])
  elif conf.CheckLib("tbb"):
    environment.Append(LIBS = ["tbb"], CPPDEFINES = ["USE_INTEL_TBB"])
  if ((conf.CheckFunc('kqueue') and conf.CheckFunc('kevent') ) or conf.CheckCHeader('sys/kqueue.h') ) and ARGUMENTS.get('poll', 'kqueue') == 'kqueue':
    environment.Append(CPPDEFINES = ["USE_KQUEUE"])
    environment.Append(CPPDEFINES = ["NEEDS_FJNET_POLL_TIMEOUT=0"])
    poll_api = 'kqueue'
  elif (conf.CheckFunc('poll') or conf.CheckCHeader('poll.h') ) and ARGUMENTS.get('poll', 'poll') == 'poll':
    poll_api = 'poll'
  else:
    environment.Append(CPPDEFINES = ["NEEDS_FJNET_POLL_TIMEOUT=1"])

  if sys.platform != 'darwin':
    if not conf.CheckLib("tbb"):
      if conf.CheckCXXHeader("mutex"):
        environment.Append(CPPDEFINES = ["USE_MUTEX"])
      elif conf.CheckCHeader("pthread.h"):
        environment.Append(CPPDEFINES = ["USE_PTHREAD"])

  if (sys.platform == 'darwin' or 'bsd' in sys.platform) and conf.CheckCHeader("kqueue.h"):
    if conf.CheckCHeader("strings.h"):
      environment.Append(CPPDEFINES = ["HAS_STRINGS"])

  conf.Finish()
elif sys.platform.startswith('win'):
  PrepareEnvironmentWin(environment)

AddOption('--enable-iconlauncher', dest = 'enableicon', default=False, help=\
"Disable compiling the Icon Launcher.\n"
"This is useful for when using older or less capable compilers that can't handle string literals longer than 65k characters long.")

AddOption('--with-include', dest = 'withinclude', default="", help=\
"One additional include directory. Make it count.")

disableicon = not GetOption('enableicon')
withinclude = GetOption('withinclude')

if disableicon:
  environment.Append(CPPDEFINES=["NO_ICONLAUNCHER"])

if withinclude != "":
  environment.Append(CPPPATH=[withinclude])


libfjnet = SConscript(dirs = ['libfjnet'], exports = ['environment', 'poll_api'])
libfjirc = SConscript(dirs = ['libfjirc'], exports = ['environment'])
libfjcsv = SConscript(dirs = ['libfjcsv'], exports = ['environment'])
libyyymonitor = SConscript(dirs = ['libyyymonitor'], exports = ['environment'])

kashyyyk_libs = [libfjirc, libfjnet, libfjcsv, libyyymonitor]

if not disableicon:
	yyyicons = SConscript(dirs = [os.path.join('extra', 'icons')],    exports = ['environment'])
	kashyyyk_libs += [yyyicons]

environment.Append(CPPPATH = [os.path.join(os.getcwd(), 'libfjirc'),
                              os.path.join(os.getcwd(), 'libfjnet'),
                              os.path.join(os.getcwd(), 'libfjcsv'),
                              os.path.join(os.getcwd(), 'extra'),
                              os.getcwd(), os.path.join(os.getcwd(), 'libyyymonitor'), os.getcwd()])

SConscript(dirs = ['kashyyyk'], exports = ['kashyyyk_libs', 'environment'])
