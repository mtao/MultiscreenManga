from conan import ConanFile
from conan.tools.meson import MesonToolchain
from conan.tools.gnu import PkgConfigDeps
import os

__BASE_DEPS__ = [
        "spdlog/1.14.1", 
        "range-v3/cci.20240905",  
        "catch2/3.7.1",
        "cxxopts/3.2.0",
        "shaderc/2021.1", "glfw/3.3.8", "glm/0.9.9.8", "imgui/1.88"
        ,"protobuf/5.27.0"
        ,"nlohmann_json/3.11.2"
        ,"pngpp/0.2.10"
        ,"qt/6.7.3"
        ]





class MultiscreenManga(ConanFile):
    requires = __BASE_DEPS__ 
    settings = "os", "compiler", "build_type"  
    generators = "PkgConfigDeps"


    def requirements(self):

        #  make sure fmt is the version we want
        self.requires("fmt/11.0.2", override=True)
        self.requires("vulkan-headers/1.3.268.0",override=True)
        self.requires("vulkan-loader/1.3.268.0",override=True)
        self.requires("glslang/11.7.0",override=True)
        self.requires("spirv-tools/1.3.268.0",override=True)
        self.requires("spirv-headers/1.3.268.0",override=True)
                                               
    def configure(self):
        self.options["glfw"].vulkan_static = True
        self.options["qt"].with_vulkan = True
             
    def generate(self):
        meson = MesonToolchain(self)                   
        meson.generate()

                                               
