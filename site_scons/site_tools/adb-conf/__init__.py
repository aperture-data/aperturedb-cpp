# __init__.py for adb-conf tool
# modeled on https://github.com/manuelnaranjo/scons-pkg-config
# Author: Drew Ogle <drew@aperturedata.io>

import os
import os.path
#from SCons.Script.Main import GetOption


def exists(env):
    # check to see if tool is available
    return True


def adbHasPackage(env, name):
    env.adbData.hasPackage(name, include_path, lib_path)


def adbAddPackageFromOption(
        env,
        package_name,
        option_name,
        env_prefix,
        with_lib=True,
        override={}):
    env.adbData.addPackageWithOption(
        env,
        package_name,
        option_name,
        env_prefix,
        with_lib,
        override)


def adbAddLocalPackage(
        env,
        package_name,
        env_prefix,
        inc_path,
        lib_path,
        override={}):
    env.adbData.addLocalPackage(
        env,
        package_name,
        env_prefix,
        inc_path,
        lib_path,
        override)


def adbSetOptionMap(env, optionMap):
    env.adbData.setOptionMap(optionMap)


def adbPackageIncludePaths(env, *packages):
    return env.adbData.getPackageIncludePaths(*packages)


def adbPackageIncludePath(env, package):
    return env.adbData.getPackageIncludePath(package)


def adbPackageLibPaths(env, *packages):
    return env.adbData.getPackageLibPaths(*packages)


def generate(env):

    class ApertureDataConf():
        def __init__(self):
            self.packageList = {}
            self.optionMap = {}

        def addPackage(self, name, inc, lib):
            self.packageList[name] = {"inc": inc, "lib": lib}

        def hasPackage(self, name):
            return name in self.packageList

        def setOptionMap(self, optionMap):
            self.optionMap = optionMap

        def getPackageIncludePath(self, pkg):
            if not self.hasPackage(pkg):
                err = f"Cannot return include path for {pkg}: Not Found"
                raise BaseException(err)
            package = self.packageList[pkg]
            if "include" not in package:
                err = f"Pacakge {pkg} has no defined includes"
                raise BaseException(err)
            if "root" in package:
                current_path = env.Dir('.').srcnode().get_abspath()
                relative_include = os.path.relpath(
                    os.path.join(
                        package["root"],
                        package["include"]),
                    current_path)
                return relative_include
            else:
                return package["include"]

        def getPackageIncludePaths(self, *packages):
            inc_paths = []
            for pkg in packages:
                inc_paths.append(self.getPackageIncludePath(pkg))
            return inc_paths

        def getPackageLibPaths(self, *packages):
            lib_paths = []
            for pkg in packages:
                if not self.hasPackage(pkg):
                    err = f"Cannot return library path for {pkg}: Not Found"
                    raise BaseException(err)
                package = self.packageList[pkg]
                if "lib" not in package:
                    err = f"Pacakge {pkg} has no defined library"
                    raise BaseException(err)
                lib_path = None
                if "root" in package:
                    current_path = env.Dir('.').srcnode().get_abspath()
                    relative_include = os.path.relpath(os.path.join(
                        package["root"], package["lib"]), current_path)
                    lib_path = relative_include
                else:
                    lib_path = package["lib"]
                lib_paths.append(lib_path)
            return lib_paths

        def addLocalPackage(
                self,
                env,
                package_name,
                env_prefix,
                inc_path,
                lib_path,
                override):
            if self.hasPackage(package_name):
                raise BaseException(
                    "Cannot add Package " +
                    package_name +
                    ": Already exists")
            root_path = env.Dir('.').srcnode().get_abspath()
            package = {}
            package["root"] = root_path
            package["include"] = inc_path
            package["lib"] = lib_path
            self.packageList[package_name] = package

        def addPackageWithOption(
                self,
                env,
                package_name,
                cmd_option,
                env_prefix,
                with_lib,
                override):
            if self.hasPackage(package_name):
                raise BaseException(
                    "Cannot add Package " +
                    package_name +
                    ": Already exists")

            if "ENV_INC_POST" not in override:
                env_include = env_prefix + "_INCLUDE"
            else:
                env_include = env_prefix + override["ENV_INC_POST"]

            package = {}
            option_inc_path = ""
            if cmd_option in self.optionMap:
                option_inc_path = os.path.join(
                    env.GetOption(cmd_option), 'include')
            else:
                option_inc_path = os.getenv(
                    env_include, default=os.path.join(
                        env.GetOption(cmd_option), 'include'))
            package["include"] = option_inc_path
            if with_lib:
                env_include = env_prefix + "_LIB"
                option_lib_path = ""
                if cmd_option in self.optionMap:
                    option_lib_path = os.path.join(
                        env.GetOption(cmd_option), 'lib')
                else:
                    option_lib_path = os.getenv(
                        env_include, default=os.path.join(
                            env.GetOption(cmd_option), 'lib'))
                package["lib"] = option_lib_path
            self.packageList[package_name] = package

    adbConfObject = ApertureDataConf()
    setattr(env, 'adbData', adbConfObject)
    env.AddMethod(adbSetOptionMap)
    env.AddMethod(adbAddPackageFromOption)
    env.AddMethod(adbHasPackage)
    env.AddMethod(adbPackageIncludePath)
    env.AddMethod(adbPackageIncludePaths)
    env.AddMethod(adbPackageLibPaths)
    env.AddMethod(adbAddLocalPackage)
