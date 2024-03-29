import os
import re
from compiler_warnings import compiler_warnings

AddOption('--prefix', dest='prefix',
                      type='string',
                      default='/usr/local/',
                      nargs=1,
                      action='store',
                      metavar='DIR',
                      help='Installation prefix [/usr/local/]')

AddOption('--build-debug', action="store_true", dest="build-debug",
                      default = False,
                      help='Build debug symbols')

def compileProtoFiles(client_env):
    #Compile .proto file to generate protobuf files (.h and .cc).

    protoQuery = client_env.Command (
             ['src/aperturedb/queryMessage.pb.h',
              'src/aperturedb/queryMessage.pb.cc',
             ], # TARGET
              'src/aperturedb/queryMessage.proto', # $SOURCE
              'protoc -I=src/aperturedb/ --cpp_out=src/aperturedb/ $SOURCE'
              )

# Optimization and language options.
FFLAGS = "-fopenmp -fomit-frame-pointer  \
          -fstrict-aliasing -fno-threadsafe-statics \
          -fnon-call-exceptions -fPIC -fstack-protector "

# Note: 'compiler_warnings.py' are in addition to these:
WFLAGS = "-Wall -Wextra -Werror -Weffc++ -Wpointer-arith -Wcast-align \
          -Wwrite-strings -Wno-dangling-else -Wno-sign-compare "

WFLAGS += ' '.join([str(elem) for elem in compiler_warnings])

if GetOption('build-debug'):
    OPTFLAGS = "-O0 -g3 -fvar-tracking-assignments -gdwarf-4 "
else:
    OPTFLAGS = "-O3 "

# Enviroment use by all the builds
env = Environment(
    # The PATH should contain version-specific tools, such as 'protoc',
    # otherwise e.g. '/bin/protoc' would be called and the latter would produce
    # *.pb.* of a version different from the one of $PROTOBUF_INCLUDE dir.
    ENV = {'PATH' : os.environ['PATH']},
    CXXFLAGS="-std=c++2a " + FFLAGS + OPTFLAGS + WFLAGS,
    LINKFLAGS=['-Wl,--no-undefined']
)
# env.MergeFlags(GetOption('cflags'))

comm_env = env.Clone()
comm_env.Replace(
        CPPPATH = ['include', 'src',
                   os.getenv('NLOHMANN_JSON_INCLUDE', default='/usr/include'),
                   os.getenv('GLOG_INCLUDE', default=''),
                   os.getenv('PROTOBUF_INCLUDE', default='')
                  ],
        LIBS    = ['glog'],
        LIBPATH = []
             )

# This is to compile protobuf-based .cc whose code-generation we do not control.
# The rest of the code is supposed to compiled using higher standards.
lenient_env = comm_env.Clone()
lenient_env.Replace(CXXFLAGS = lenient_env['CXXFLAGS'].replace("-Weffc++", ""))
lenient_env.Replace(CXXFLAGS = re.sub("-Warray-bounds[^\s]+",      "-Warray-bounds=1",             lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-attribute=const", "-Wno-suggest-attribute=const", lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-final-types",     "-Wno-suggest-final-types",     lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wuseless-cast",            "-Wno-useless-cast",            lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-override",        "-Wno-suggest-override",        lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wshadow",                  "-Wno-shadow",                  lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wredundant-decls",         "-Wno-redundant-decls",         lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wdeprecated-declarations", "-Wno-deprecated-declarations", lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wundef",                   "-Wno-undef",                   lenient_env['CXXFLAGS']))

comm_cc = [
           'src/comm/ConnClient.cc',
           'src/comm/Connection.cc',
           'src/comm/ConnServer.cc',
           'src/comm/Exception.cc',
           'src/comm/OpenSSLBio.cc',
           'src/comm/TCPConnection.cc',
           'src/comm/TCPSocket.cc',
           'src/comm/TLS.cc',
           'src/comm/TLSConnection.cc',
           'src/comm/TLSSocket.cc',
          ]

comm_env.ParseConfig('pkg-config --cflags --libs openssl')
ulib = comm_env.SharedLibrary('lib/comm', comm_cc)

client_env = comm_env.Clone()
client_env.Replace(
        CPPPATH = ['include', 'src',
                    os.getenv('NLOHMANN_JSON_INCLUDE', default='/usr/include'),
                    os.getenv('PROTOBUF_INCLUDE',      default='')
                  ],
        # This might not be required:
        LIBS    = ['comm'],

        # Pointing to $XYZ_LIB in LIBPATH and RPATH is a way for the executables
        # that will be linked with this lib to ibe able to find the correct
        # version of XYZ lib, even if serveral version of XYZ lib exists on the
        # system. (And to avoid potential coredumps when a wrong .so is used at
        # run-time.) If curious, do
        #
        #    % ldd lib/libaperturedb-client.so  | grep protobuf
        #
        # once the lib is build.
        LIBPATH = ['lib/',
                    os.getenv('PROTOBUF_LIB', default=''),
		    env.Literal('\\$$ORIGIN')
                  ],
        )

client_env.Append(RPATH = client_env['LIBPATH'])

compileProtoFiles(client_env)

protobuf_cc = [
           'src/aperturedb/queryMessage.pb.cc',
           ]
client_cc = [
           'src/aperturedb/TokenBasedVDMSClient.cc',
           'src/aperturedb/VDMSClient.cc',
           'src/aperturedb/VDMSClientImpl.cc'
          ]

client_env.ParseConfig('pkg-config --cflags --libs protobuf')
ulib = client_env.SharedLibrary('lib/aperturedb-client',
        [client_cc, lenient_env.SharedObject(protobuf_cc)]
        )

CXXFLAGS = env['CXXFLAGS']

# Comm Testing
comm_test_env = Environment(
        ENV      = {'PATH' : os.environ['PATH']},
        CPPPATH  = ['include', 'src',
                    os.getenv('NLOHMANN_JSON_INCLUDE', default='/usr/include'),
                    os.getenv('PROMETHEUS_CPP_CORE_INCLUDE', default=''),
                    os.getenv('GLOG_INCLUDE',          default=''),
                    os.getenv('PROTOBUF_INCLUDE',      default='')
                   ],
        LIBPATH  = ['lib',
                    os.getenv('PROMETHEUS_CPP_LIB',    default=''),
                    os.getenv('GLOG_LIB',              default=''),
                    os.getenv('PROTOBUF_LIB',          default='')
                   ],
        CXXFLAGS = CXXFLAGS,
        LIBS     = ['aperturedb-client',
                    'prometheus-cpp-core',
                    'comm',
                    'gtest',
                    'glog',
                    'protobuf',
                    'pthread',
                   ],
        )

# Using RPATH causes the linker to record locations of shared libs inside the
# executable, so that we do not need to either (a) copy the libs to standard
# locations or (b) point to them in LD_LIBRARY_PATH.  To check if this worked
# correctly, do
#
#     % ldd test/comm_test | grep 'not found'
#
# To check for which libs relative paths are used, do
#
#     % ldd test/comm_test | grep '=>' | grep -v '=> /'
#
# For an info on RPATH, see the man page for 'ld'.

comm_test_env.Append(RPATH = comm_test_env['LIBPATH'])


comm_test_env.ParseConfig('pkg-config --cflags --libs openssl')

comm_test_source_files = [
                          'test/AuthEnabledVDMSServer.cc',
                          'test/Barrier.cc',
                          'test/TCPConnectionTests.cc',
                          'test/TLSConnectionTests.cc',
                          'test/VDMSServer.cc',
                          'test/VDMSServerTests.cc',
                          'test/TimedQueueTests.cc',
                          'test/Base64Tests.cc',
                         ]

comm_test = comm_test_env.Program('test/comm_test', comm_test_source_files)

prefix = str(GetOption('prefix'))

env.Alias('install',
        env.Install(os.path.join(prefix, "lib"), source="lib/libcomm.so"),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "lib"), source="lib/libaperturedb-client.so"),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "bin"),
                source="tools/prometheus_ambassador/prometheus_ambassador"),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "include/aperturedb/"),
                                 source=Glob("include/aperturedb/" + "*.h")),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "include/comm/"),
                                 source=Glob("include/comm/" + "*.h")),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "include/metrics/"),
                                 source=Glob("include/metrics/" + "*.h")),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "include/util/"),
                                 source=Glob("include/util/" + "*.h")),
        )

env.Alias('install',
	 env.Command(target=os.path.join(prefix, "include/aperturedb/version.h"),
                     source="src/aperturedb/version.h",
                     action="sed 's/VDMS_VER/APERTUREDB_CPP_VER/g' $SOURCE > $TARGET"
                     )
        )

SConscript(os.path.join('tools/prometheus_ambassador', 'SConscript'), exports=['env'])
SConscript(os.path.join('tools/send_query', 'SConstruct'), exports=['env'])
