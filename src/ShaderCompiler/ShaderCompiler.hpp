#pragma once

#include <string>
#include <vector>
#include <array>

#include <bytecode.h>
#include <spirvcross.h>
#include <types/reflection/stage_attr.h>

#include <sokol_gfx.h>

namespace shdc
{
    struct ShaderUniform
    {
        std::string name;
        std::string inst_name;
        size_t size;
        int slot;
    };

    struct Vertex_Attributes
    {
        std::string name;
        std::string sem_name;
        sg_vertex_format format = SG_VERTEXFORMAT_INVALID;
        int slot = 0;
    };

    struct ShaderProgram
    {
        std::string name;
        sg_shader_desc shader_desc;

        std::array<std::string, ShaderStage::Num> shader_entry_names = {};

        std::array<Vertex_Attributes, refl::StageAttr::Num> vertex_attributes = {};

        std::array<std::string, MaxTextureSamplers> tex_smp_names = {};

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
        std::string error = "";
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
