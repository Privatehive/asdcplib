from conan import ConanFile
from conan.tools.apple import fix_apple_shared_install_name
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import apply_conandata_patches, copy, export_conandata_patches, get, replace_in_file, rm, rmdir
from conan.tools.gnu import Autotools, AutotoolsToolchain
from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.layout import basic_layout
from conan.tools.microsoft import is_msvc, MSBuild, MSBuildToolchain
from conan.tools.scm import Version
from conan.errors import ConanInvalidConfiguration
import re
import os

required_conan_version = ">=2.0"

def read_version():
    with open('configure.ac', 'r') as f:
        version = re.search("AC_INIT\(\[asdcplib\],\s?\[([0-9.]*)\]", f.read())
        return version.group(1)

class AsdcpLibConan(ConanFile):

    # ---Package reference---
    name = "asdcplib"
    version = read_version()
    user = "imftool"
    channel = "stable"
    # ---Metadata---
    description = "The asdcplib library is an API and command-line tool set that offers access to files conforming to the sound and picture track file formats developed by the SMPTE Working Group DC28.20 (now TC 21DC)."
    license = "proprietary"
    # ---Requirements---
    requires = []
    tool_requires = ["cmake/[>=3.21.1]", "ninja/[>=1.11.1]"]
    # ---Sources---
    exports = ["configure.ac", "COPYING"]
    exports_sources = ["CMakeLists.txt", "COPYING", "configure.ac", "src/*"]
    # ---Binary model---
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "encryption_support": [True, False], "xml_support": [True, False], "jxs_support": [True, False]}
    default_options = {
        "shared": True,
        "fPIC": True,
        "encryption_support": True,
        "xml_support": True,
        "jxs_support": False,
    }
    # ---Build---
    generators = []
    # ---Folders---
    no_copy_source = True

    def validate(self):
        valid_os = ["Windows", "Linux", "Macos"]
        if str(self.settings.os) not in valid_os:
            raise ConanInvalidConfiguration(f"{self.name} {self.version} is only supported for the following operating systems: {valid_os}")
        valid_arch = ["x86_64", "armv8"]
        if str(self.settings.arch) not in valid_arch:
            raise ConanInvalidConfiguration(f"{self.name} {self.version} is only supported for the following architectures on {self.settings.os}: {valid_arch}")
        if str(self.settings.os) == 'Windows' and self.options.shared:
            raise ConanInvalidConfiguration(f"{self.name} {self.version} does not support building shared library on Windows")

    def requirements(self):
        if self.options.encryption_support:
            self.requires("openssl/[~3.0]", options={"no_zlib": True, "no_asm": True, "shared": False})
        if self.options.xml_support:
            self.requires("xerces-c/[>=3.2.5]", options={"network": False, "shared": False})

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if not self.options.shared:
            self.options.rm_safe("fPIC")

    def generate(self):
        VirtualBuildEnv(self).generate()
        tc = CMakeToolchain(self, generator="Ninja")
        tc.variables["WITHOUT_SSL"] = self.options.encryption_support == False
        tc.variables["WITHOUT_XML"] = self.options.xml_support == False
        tc.variables["USE_ASDCP_JXS"] = self.options.jxs_support == True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        if self.settings.build_type == 'Release':
            cmake.install(cli_args=["--strip"])
        else:
            cmake.install()

    def package_info(self):
        self.cpp_info.builddirs = ["lib/cmake"]
        self.cpp_info.set_property("cmake_find_mode", "none")
