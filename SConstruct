import os
import re
import os.path
from compiler_warnings import compiler_warnings

# maintains which options were triggered on the commandline
setOptions = {}
def optionWasSet(option, opt, value, parser):
	setOptions[option.dest] = True
	setattr(parser.values, option.dest, value)

AddOption('--prefix', dest='prefix',
                      type='string',
                      default='/usr/local/',
                      nargs=1,
                      action='store',
                      metavar='DIR',
                      help='Installation prefix [/usr/local/]')

AddOption( '--json-root', dest='json_root',
			  type='string',
			  default='/usr/',
			  nargs=1,
			  action='callback',
			  callback=optionWasSet,
			  metavar='DIR',
			  help='Location of nlohmann/json [/usr/]')

AddOption( '--glog-root', dest='glog_root',
			  type='string',
			  default='/usr/local',
			  nargs=1,
			  action='callback',
			  callback=optionWasSet,
			  metavar='DIR',
			  help='Location of google logging [/usr/local]')

AddOption( '--prometheus-root', dest='prometheus_root',
			  type='string',
			  default='/usr/local',
			  nargs=1,
			  action='callback',
			  callback=optionWasSet,
			  metavar='DIR',
			  help='Location of prometheus-cpp [/usr/local]')

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
#env = Environment(CXXFLAGS="-std=c++2a " + FFLAGS + OPTFLAGS + WFLAGS)
env = Environment(CXXFLAGS="-std=c++2a " + FFLAGS + OPTFLAGS + WFLAGS, tools=['default','adb-conf'])

env.adbSetOptionMap( setOptions )
env.adbAddPackageFromOption( "nlohmann/json", "json_root", "NLOHMANN_JSON", with_lib=False )
env.adbAddPackageFromOption( "glog", "glog_root", "GLOG" )
env.adbAddPackageFromOption( "prometheus-cpp", "prometheus_root", "PROMETHEUS_CPP", override={"ENV_INC_POST":"CORE_INCLUDE"} )


comm_env = env.Clone()
comm_env.Replace(
        CPPPATH = ['include', 'src' ] +
		   env.adbPackageIncludePaths( "nlohmann/json", "glog","prometheus-cpp")
		   ,
        LIBS    = [],
        LIBPATH = []
             )

#nlohmann/json
comm_conf = Configure(comm_env)
if not comm_conf.CheckCXXHeader('nlohmann/json.hpp'):
	print("Unable to find nlohmann/json in " + env.adbPackageIncludePath( "nlohmann/json"))
	Exit(1)
#glog
if not comm_conf.CheckCXXHeader('glog/logging.h'):
	print("Unable to find glog in " + env.adbPackageIncludePath( "glog" ))
	print(f"Unable to find glog in {glog_inc_path}")
	Exit(1)

#prometheus
if not comm_conf.CheckCXXHeader('prometheus/registry.h'):
	print(f"Unable to find prometheus in "+ env.adbPackageIncludePath( "prometheus-cpp" ))
	Exit(1)

# prometheus-pull
if not comm_conf.CheckCXXHeader('prometheus/exposer.h'):
	print(f"Unable to find prometheus-pull in "+ env.adbPackageIncludePath( "prometheus-cpp" ))
	Exit(1)



# This is to compile protobuf-based .cc whose code-generation we do not control.
# The rest of the code is supposed to compiled using higher standards.
lenient_env = comm_env.Clone()
lenient_env.Replace(CXXFLAGS = lenient_env['CXXFLAGS'].replace("-Weffc++", ""))
lenient_env.Replace(CXXFLAGS = re.sub("-Warray-bounds[^\s]+", "-Warray-bounds=1", lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-attribute=const", "-Wno-suggest-attribute=const", lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-final-types",     "-Wno-suggest-final-types",     lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wuseless-cast",            "-Wno-useless-cast",            lenient_env['CXXFLAGS']))
lenient_env.Replace(CXXFLAGS = re.sub("-Wsuggest-override",        "-Wno-suggest-override",        lenient_env['CXXFLAGS']))

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
        CPPPATH = ['include', 'src'] +
			   env.adbPackageIncludePaths( "nlohmann/json"),
        LIBS    = ['comm'],
        LIBPATH = ['lib/']
             )

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
        CPPPATH  = ['include', 'src',] +
		   env.adbPackageIncludePaths( "nlohmann/json", "glog","prometheus-cpp"),
        LIBPATH  = ['lib', ] +
		   env.adbPackageLibPaths( "glog","prometheus-cpp"),
        CXXFLAGS = CXXFLAGS,
        LIBS     = [
                    'aperturedb-client',
                    'comm',
                    'pthread',
                    'gtest',
                    'glog',
                    'prometheus-cpp-core',
                   ],
        RPATH    = ['../lib']
        )


comm_test_env.ParseConfig('pkg-config --cflags --libs protobuf')

comm_test_source_files = [
                          'test/AuthEnabledVDMSServer.cc',
                          'test/Barrier.cc',
                          'test/TCPConnectionTests.cc',
                          'test/TLSConnectionTests.cc',
                          'test/VDMSServer.cc',
                          'test/VDMSServerTests.cc',
                          'test/TimedQueueTests.cc',
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

env.adbAddLocalPackage("ad_comm","AD_COMM","src","lib")
env.adbAddLocalPackage("ad_client","AD_CLIENT","src","lib")
SConscript(os.path.join('tools/prometheus_ambassador', 'SConscript'), exports=['env'])
SConscript(os.path.join('tools/send_query', 'SConstruct'), exports=['env'])
