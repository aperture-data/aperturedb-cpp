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
             ['src/queryMessage.pb.h',
              'src/queryMessage.pb.cc',
              'python/vdmsqueryMessage_pb2.py',
             ], # TARGET
              'src/queryMessage.proto', # $SOURCE
              'protoc -I=src/ --cpp_out=src/ $SOURCE'
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

client_env = env.Clone()
client_env.Replace(
        CPPPATH = ['include/aperturedb', 'src'],
        LIBS    = [],
        LIBPATH = []
             )

client_env.Replace(CXXFLAGS = client_env['CXXFLAGS'].replace("-Weffc++", ""))
client_env.Replace(CXXFLAGS = re.sub("-Warray-bounds[^\s]+", "-Warray-bounds=1", client_env['CXXFLAGS']))
client_env.Replace(CXXFLAGS = re.sub("-Wsuggest-attribute=const", "-Wno-suggest-attribute=const", client_env['CXXFLAGS']))
client_env.Replace(CXXFLAGS = re.sub("-Wsuggest-final-types",     "-Wno-suggest-final-types",     client_env['CXXFLAGS']))
client_env.Replace(CXXFLAGS = re.sub("-Wuseless-cast",            "-Wno-useless-cast",            client_env['CXXFLAGS']))
client_env.Replace(CXXFLAGS = re.sub("-Wsuggest-override",        "-Wno-suggest-override",        client_env['CXXFLAGS']))

compileProtoFiles(client_env)

comm_cc = [
           'src/ConnClient.cc',
           'src/Connection.cc',
           'src/ConnServer.cc',
           'src/Exception.cc',
           'src/queryMessage.pb.cc',
           'src/TCPConnection.cc',
           'src/TCPSocket.cc',
           'src/TLS.cc',
           'src/TLSConnection.cc',
           'src/TLSSocket.cc',
           'src/VDMSClient.cc'
          ]

client_env.ParseConfig('pkg-config --cflags --libs protobuf')
client_env.ParseConfig('pkg-config --cflags --libs openssl')
ulib = client_env.SharedLibrary('lib/aperturedb-client', comm_cc)

CXXFLAGS = env['CXXFLAGS']

# Comm Testing
comm_test_env = Environment(CPPPATH  = ['include/aperturedb', 'src'],
                            CXXFLAGS = CXXFLAGS,
                            LIBS     = ['aperturedb-client', 'pthread', 'gtest', 'glog'],
                            LIBPATH  = ['lib/']
                            )

comm_test_env.ParseConfig('pkg-config --cflags --libs protobuf')

comm_test_source_files = [
                          'test/TCPConnectionTests.cc',
                          'test/TLSConnectionTests.cc',
                          'test/VDMSServer.cc',
                          'test/VDMSServerTests.cc'
                         ]
comm_test = comm_test_env.Program('test/comm_test', comm_test_source_files)

prefix = str(GetOption('prefix'))

env.Alias('install',
        env.Install(os.path.join(prefix, "lib"), source="lib/libaperturedb-client.so"))


# Mock-up Server
server_env = Environment(CPPPATH  = ['include/aperturedb', 'src', 'test'],
                            CXXFLAGS = CXXFLAGS,
                            LIBS     = ['aperturedb-client', 'pthread', 'gtest', 'glog'],
                            LIBPATH  = ['lib/']
                            )

server_cc = ['server/server.cc']

server = server_env.Program('server/server', server_cc)
