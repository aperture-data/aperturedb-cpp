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

OPTFLAGS = "-O3 "
# Enviroment use by all the builds
env = Environment(CXXFLAGS="-std=c++11 " + FFLAGS + OPTFLAGS + WFLAGS)
# env.MergeFlags(GetOption('cflags'))

comm_env = env.Clone()
comm_env.Replace(
        CPPPATH = ['include', 'src'],
        LIBS    = [],
        LIBPATH = []
             )

comm_env.Replace(CXXFLAGS = comm_env['CXXFLAGS'].replace("-Weffc++", ""))
comm_env.Replace(CXXFLAGS = re.sub("-Warray-bounds[^\s]+", "-Warray-bounds=1", comm_env['CXXFLAGS']))
comm_env.Replace(CXXFLAGS = re.sub("-Wsuggest-attribute=const", "-Wno-suggest-attribute=const", comm_env['CXXFLAGS']))
comm_env.Replace(CXXFLAGS = re.sub("-Wsuggest-final-types",     "-Wno-suggest-final-types",     comm_env['CXXFLAGS']))
comm_env.Replace(CXXFLAGS = re.sub("-Wuseless-cast",            "-Wno-useless-cast",            comm_env['CXXFLAGS']))
comm_env.Replace(CXXFLAGS = re.sub("-Wsuggest-override",        "-Wno-suggest-override",        comm_env['CXXFLAGS']))

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
        CPPPATH = ['include', 'src'],
        LIBS    = ['comm'],
        LIBPATH = ['lib/']
             )

compileProtoFiles(client_env)

client_cc = [
           'src/aperturedb/queryMessage.pb.cc',
           'src/aperturedb/TokenBasedVDMSClient.cc',
           'src/aperturedb/VDMSClient.cc',
           'src/aperturedb/VDMSClientImpl.cc'
          ]

client_env.ParseConfig('pkg-config --cflags --libs protobuf')
ulib = client_env.SharedLibrary('lib/aperturedb-client', client_cc)

CXXFLAGS = env['CXXFLAGS']

# Comm Testing
comm_test_env = Environment(CPPPATH  = ['include', 'src'],
                            CXXFLAGS = CXXFLAGS,
                            LIBS     = ['aperturedb-client', 'comm', 'pthread', 'gtest', 'glog'],
                            LIBPATH  = ['lib/']
                            )

comm_test_env.ParseConfig('pkg-config --cflags --libs protobuf')

comm_test_source_files = [
                          'test/AuthEnabledVDMSServer.cc',
                          'test/Barrier.cc',
                          'test/TCPConnectionTests.cc',
                          'test/TLSConnectionTests.cc',
                          'test/VDMSServer.cc',
                          'test/VDMSServerTests.cc'
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
        env.Install(os.path.join(prefix, "include/aperturedb/"),
                                 source=Glob("include/aperturedb/" + "*.h")),
        )

env.Alias('install',
        env.Install(os.path.join(prefix, "include/comm/"),
                                 source=Glob("include/comm/" + "*.h")),
        )
