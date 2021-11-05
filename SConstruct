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
             ['client/queryMessage.pb.h',
              'client/queryMessage.pb.cc',
             ], # TARGET
              'client/queryMessage.proto', # $SOURCE
              'protoc -I=client/ --cpp_out=client/ $SOURCE'
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
        CPPPATH = ['include/comm', 'comm'],
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
           'comm/ConnClient.cc',
           'comm/Connection.cc',
           'comm/ConnServer.cc',
           'comm/Exception.cc',
           'comm/OpenSSLBio.cc',
           'comm/TCPConnection.cc',
           'comm/TCPSocket.cc',
           'comm/TLS.cc',
           'comm/TLSConnection.cc',
           'comm/TLSSocket.cc',
          ]

comm_env.ParseConfig('pkg-config --cflags --libs openssl')
ulib = comm_env.SharedLibrary('lib/comm', comm_cc)

client_env = comm_env.Clone()
client_env.Replace(
        CPPPATH = ['include/aperturedb', 'include/comm', 'client'],
        LIBS    = ['comm'],
        LIBPATH = ['lib/']
             )

compileProtoFiles(client_env)

client_cc = [
           'client/queryMessage.pb.cc',
           'client/TokenBasedVDMSClient.cc',
           'client/VDMSClient.cc',
           'client/VDMSClientImpl.cc'
          ]

client_env.ParseConfig('pkg-config --cflags --libs protobuf')
ulib = client_env.SharedLibrary('lib/aperturedb-client', client_cc)

CXXFLAGS = env['CXXFLAGS']

# Comm Testing
comm_test_env = Environment(CPPPATH  = ['include/aperturedb', 'include/comm','client', 'comm'],
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

# Mock-up Server
server_env = Environment(CPPPATH  = ['include/comm', 'comm', 'test'],
                            CXXFLAGS = CXXFLAGS,
                            LIBS     = ['comm', 'pthread', 'gtest', 'glog'],
                            LIBPATH  = ['lib/']
                            )

server_cc = ['server/server.cc']

server = server_env.Program('server/server', server_cc)
