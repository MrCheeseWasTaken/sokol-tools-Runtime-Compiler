#pragma once

#include <string>
#include <vector>
#include <array>

#include <bytecode.h>
#include <spirvcross.h>

#include <sokol_gfx.h>

namespace shdc
{
    struct ShaderUniform
    {
        std::string name;
        size_t size;
        int slot;
    };

    struct ShaderProgram
    {
        std::string name;
        
        sg_shader_desc shader_desc;
        std::vector<std::pair<uint16_t, sg_vertex_format>> vertex_attributes = {};
        std::vector<ShaderUniform> uniforms = {};
    };

    struct CompiledShaderPrograms
    {
        // we need to do this because sg_shader_desc takes the source code as const char*
        // so we need to keep the std::string alive
        // user should never need these but keeping them in PUBLIC for transparency
        Spirvcross spirvcross;
        Bytecode bytecode;

        std::vector<ShaderProgram> programs;
    };

    struct ShaderCompiler
    {
        static void Initialize();
        static void Finalize();

        // Takes in a string that is the same format as normal shader file and compiles it
        // CompiledShaderPrograms::programs is empty if failed 
        static CompiledShaderPrograms CompileShader(const char *shader_code, sg_backend backend, bool byte_code = false);
    };
}
