Kashyyyk
========
A Work-In-Progress, Full-Featured, Open Source IRC Client
*****
About Kashyyyk
--------------

Kashyyyk was originally written by FlyingJester (Martin McDonough) for two
reasons; just for the fun of it, and to replace HexChat on OS X. At the time
of this writing, HexChat (a fork of XChat) has only had a single release on
OS X, which is still in alpha.
Kashyyk uses the libfjirc library originally written for the Whittler IRC bot
and the libfjnet wrapper written for TurboSphere. It has an FLTK-based GUI.
Kashyyyk is intended to be cross platform and easily compiled.

Compiling Kashyyyk
------------------

Kashyyyk requires a C++11 compiler and an ansi C compiler. The build scripts
support GCC, Clang, and MSVC. You will need FLTK 1.3, either in shared or 
static version. You need Python 2.x and SCons to build Kashyyyk.

#### Checklist for Compiling
 - FLTK 1.3
 - MSVC 2013, GCC, or Clang
 - Python 2.x
 - SCons

Remember to initialize the TSPR submodule if you are using the git checkout.

To build Kashyyyk, run `scons` in the root directory.

Kashyyyk Features
-----------------

#### Implemented
 - Multiple Window Support
 - Command Parsing (/nick, /join, etc.)
 - Persistent Preferences
 - Font and Theme Selection
 - Partial Numeric Response Support
 - Notification Center Support

#### Planned
 - Chat Logging
 - Multi-Window UI
 - SSL Support
 - Full Numeric Response Support

#### Broken
 - User Lists on Some Channels

#### Won't Add
 - Scripting

License
-------

Kashyyyk is released under the GPL 2.0.
A copy of the GPL 2.0 is included in the file `license.txt`.
