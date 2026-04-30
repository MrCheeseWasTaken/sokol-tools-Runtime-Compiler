#include "ShaderCompiler.hpp"

#include <spirv.h>
#include <bytecode.h>
#include <input.h>
#include <spirvcross.h>
#include <reflection.h>
#include <util.h>

using namespace shdc;
using namespace shdc::refl;

void ShaderCompiler::Initialize()
{
    Spirv::initialize_spirv_tools();
}

void shdc::ShaderCompiler::Finalize()
{
    Spirv::finalize_spirv_tools();
}

Slang::Enum backend_to_slang(sg_backend backend)
{
    switch (backend)
    {
    case SG_BACKEND_GLCORE:
        return Slang::Enum::GLSL430;

    case SG_BACKEND_GLES3:
        return Slang::Enum::GLSL300ES;

    case SG_BACKEND_D3D11:
        return Slang::Enum::HLSL5;

    case SG_BACKEND_METAL_MACOS:
        return Slang::Enum::METAL_MACOS;

    case SG_BACKEND_METAL_IOS:
        return Slang::Enum::METAL_IOS;

    case SG_BACKEND_METAL_SIMULATOR:
        return Slang::Enum::METAL_SIM;

    case SG_BACKEND_WGPU:
        return Slang::Enum::WGSL;

    case SG_BACKEND_VULKAN:
        return Slang::Enum::SPIRV_VK;

    case SG_BACKEND_DUMMY:
    default:
        assert(false && "No valid slang");
        // return Slang::Enum::Num;  // no valid slang
    }
}

sg_shader_attr_base_type attr_basetype(Type::Enum e)
{
    switch (e)
    {
    case Type::Float:
        return SG_SHADERATTRBASETYPE_FLOAT;
    case Type::Int:
        return SG_SHADERATTRBASETYPE_SINT;
    case Type::UInt:
        return SG_SHADERATTRBASETYPE_UINT;
    default:
        assert("Cant find base type");
        return SG_SHADERATTRBASETYPE_UNDEFINED;
    }
}

sg_shader_stage shader_stage(ShaderStage::Enum stage)
{
    if (stage == ShaderStage::Vertex)
        return SG_SHADERSTAGE_VERTEX;
    else if (stage == ShaderStage::Fragment)
        return SG_SHADERSTAGE_FRAGMENT;
    else if (stage == ShaderStage::Compute)
        return SG_SHADERSTAGE_COMPUTE;
    else
    {
        assert(false && "could not find shader stage\n");
        // return SG_SHADERSTAGE_NONE;
    }
}

sg_uniform_type flattened_uniform_type(Type::Enum e)
{
    switch (e)
    {
    case Type::Float:
    case Type::Float2:
    case Type::Float3:
    case Type::Float4:
    case Type::Mat4x4:
        return SG_UNIFORMTYPE_FLOAT4;
    case Type::Int:
    case Type::Int2:
    case Type::Int3:
    case Type::Int4:
        return SG_UNIFORMTYPE_INT4;
    default:
        assert(false && "Cant flatten uniform");
        // return SG_UNIFORMTYPE_INVALID;
    }
}

sg_uniform_type uniform_type(Type::Enum e)
{
    switch (e)
    {
    case Type::Float:
        return SG_UNIFORMTYPE_FLOAT;
    case Type::Float2:
        return SG_UNIFORMTYPE_FLOAT2;
    case Type::Float3:
        return SG_UNIFORMTYPE_FLOAT3;
    case Type::Float4:
        return SG_UNIFORMTYPE_FLOAT4;
    case Type::Int:
        return SG_UNIFORMTYPE_INT;
    case Type::Int2:
        return SG_UNIFORMTYPE_INT2;
    case Type::Int3:
        return SG_UNIFORMTYPE_INT3;
    case Type::Int4:
        return SG_UNIFORMTYPE_INT4;
    case Type::Mat4x4:
        return SG_UNIFORMTYPE_MAT4;
    default:
        assert(false && "cant convert uniform type");
        // return SG_UNIFORMTYPE_INVALID;
    }
}

sg_image_type image_type(ImageType::Enum e)
{
    switch (e)
    {
    case ImageType::_2D:
        return SG_IMAGETYPE_2D;
    case ImageType::CUBE:
        return SG_IMAGETYPE_CUBE;
    case ImageType::_3D:
        return SG_IMAGETYPE_3D;
    case ImageType::ARRAY:
        return SG_IMAGETYPE_ARRAY;
    default:
        assert(false && "Failed to get image type");
    }
}

sg_image_sample_type image_sample_type(ImageSampleType::Enum e)
{
    switch (e)
    {
    case ImageSampleType::FLOAT:
        return SG_IMAGESAMPLETYPE_FLOAT;
    case ImageSampleType::DEPTH:
        return SG_IMAGESAMPLETYPE_DEPTH;
    case ImageSampleType::SINT:
        return SG_IMAGESAMPLETYPE_SINT;
    case ImageSampleType::UINT:
        return SG_IMAGESAMPLETYPE_UINT;
    case ImageSampleType::UNFILTERABLE_FLOAT:
        return SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT;
    default:
        assert(false && "Failed to get image type");
    }
}

sg_pixel_format storage_pixel_format(refl::StoragePixelFormat::Enum e)
{
    switch (e)
    {
    case StoragePixelFormat::RGBA8:
        return SG_PIXELFORMAT_RGBA8;
    case StoragePixelFormat::RGBA8SN:
        return SG_PIXELFORMAT_RGBA8SN;
    case StoragePixelFormat::RGBA8UI:
        return SG_PIXELFORMAT_RGBA8UI;
    case StoragePixelFormat::RGBA8SI:
        return SG_PIXELFORMAT_RGBA8SI;
    case StoragePixelFormat::RGBA16UI:
        return SG_PIXELFORMAT_RGBA16UI;
    case StoragePixelFormat::RGBA16SI:
        return SG_PIXELFORMAT_RGBA16SI;
    case StoragePixelFormat::RGBA16F:
        return SG_PIXELFORMAT_RGBA16F;
    case StoragePixelFormat::R32UI:
        return SG_PIXELFORMAT_R32UI;
    case StoragePixelFormat::R32SI:
        return SG_PIXELFORMAT_R32SI;
    case StoragePixelFormat::R32F:
        return SG_PIXELFORMAT_R32F;
    case StoragePixelFormat::RG32UI:
        return SG_PIXELFORMAT_RG32UI;
    case StoragePixelFormat::RG32SI:
        return SG_PIXELFORMAT_RG32SI;
    case StoragePixelFormat::RG32F:
        return SG_PIXELFORMAT_RG32F;
    case StoragePixelFormat::RGBA32UI:
        return SG_PIXELFORMAT_RGBA32UI;
    case StoragePixelFormat::RGBA32SI:
        return SG_PIXELFORMAT_RGBA32SI;
    case StoragePixelFormat::RGBA32F:
        return SG_PIXELFORMAT_RGBA32F;
    default:
        assert(false && "cant convert Pixel format type");
        // return SG_PIXELFORMAT_NONE;
    }
}

sg_sampler_type sampler_type(SamplerType::Enum e)
{
    switch (e)
    {
    case SamplerType::FILTERING:
        return SG_SAMPLERTYPE_FILTERING;
    case SamplerType::COMPARISON:
        return SG_SAMPLERTYPE_COMPARISON;
    case SamplerType::NONFILTERING:
        return SG_SAMPLERTYPE_NONFILTERING;
    default:
        assert(false && "Cant find sampler type");
    }
}

sg_vertex_format vertex_format(Type::Enum e)
{
    switch (e)
    {
    case Type::Invalid:
        assert(false && "Invalid vertex format");
        // return SG_VERTEXFORMAT_INVALID;

    case Type::Float:
        return SG_VERTEXFORMAT_FLOAT;
    case Type::Float2:
        return SG_VERTEXFORMAT_FLOAT2;
    case Type::Float3:
        return SG_VERTEXFORMAT_FLOAT3;
    case Type::Float4:
        return SG_VERTEXFORMAT_FLOAT4;

    case Type::Int:
        return SG_VERTEXFORMAT_INT;
    case Type::Int2:
        return SG_VERTEXFORMAT_INT2;
    case Type::Int3:
        return SG_VERTEXFORMAT_INT3;
    case Type::Int4:
        return SG_VERTEXFORMAT_INT4;

    case Type::UInt:
        return SG_VERTEXFORMAT_UINT;
    case Type::UInt2:
        return SG_VERTEXFORMAT_UINT2;
    case Type::UInt3:
        return SG_VERTEXFORMAT_UINT3;
    case Type::UInt4:
        return SG_VERTEXFORMAT_UINT4;

    case Type::Bool:
    case Type::Bool2:
    case Type::Bool3:
    case Type::Bool4:
        assert(false && "cant convert vertex format type");
        // return SG_VERTEXFORMAT_INVALID;

    case Type::Mat2x1:
    case Type::Mat2x2:
    case Type::Mat2x3:
    case Type::Mat2x4:
    case Type::Mat3x1:
    case Type::Mat3x2:
    case Type::Mat3x3:
    case Type::Mat3x4:
    case Type::Mat4x1:
    case Type::Mat4x2:
    case Type::Mat4x3:
    case Type::Mat4x4:
        assert(false && "cant convert vertex format type");
        // return SG_VERTEXFORMAT_INVALID;

    case Type::Struct:
    default:
        assert(false && "cant convert vertex format type");
        // return SG_VERTEXFORMAT_INVALID;
    }
}

const char *hlsl_target(shdc::Slang::Enum slang, shdc::ShaderStage::Enum stage)
{
    using namespace shdc;
    switch (slang)
    {
    case Slang::HLSL4:
        switch (stage)
        {
        case ShaderStage::Vertex:
            return "vs_4_0";
        case ShaderStage::Fragment:
            return "ps_4_0";
        case ShaderStage::Compute:
            return "cs_4_0";
        default:
            return nullptr;
        }
    case Slang::HLSL5:
        switch (stage)
        {
        case ShaderStage::Vertex:
            return "vs_5_0";
        case ShaderStage::Fragment:
            return "ps_5_0";
        case ShaderStage::Compute:
            return "cs_5_0";
        default:
            return nullptr;
        }
    default:
        return 0;
    }
}

int roundup(int val, int round_to)
{
    return (val + (round_to - 1)) & ~(round_to - 1);
}

CompiledShaderPrograms ShaderCompiler::CompileShader(const char *shader_code, sg_backend backend, bool byte_code)
{
    CompiledShaderPrograms compiled_programs;

    Slang::Enum slang = backend_to_slang(backend);

    Input inp = Input::just_parse(shader_code);
    if (inp.out_error.valid())
    {
        inp.out_error.print(ErrMsg::GCC);
        return {};
    }

    // compile source snippets to SPIRV blobs (multiple compilations is necessary
    // because of conditional compilation by target language)
    std::vector<std::string> defines;
    Spirv spirv;
    spirv = Spirv::compile_glsl_and_extract_bindings(inp, slang, defines);
    if (!spirv.errors.empty())
    {
        bool has_errors = false;
        for (const ErrMsg &err : spirv.errors)
        {
            if (err.type == ErrMsg::ERROR)
            {
                has_errors = true;
            }
            err.print(ErrMsg::GCC);
        }
        if (has_errors)
        {
            return {};
        }
    }

    // cross-translate SPIRV to shader dialect
    compiled_programs.spirvcross = Spirvcross::translate(inp, spirv, slang);
    Spirvcross& spirvcross = compiled_programs.spirvcross;
    if (spirvcross.error.valid())
    {
        spirvcross.error.print(ErrMsg::GCC);
        return {};
    }

    // compile shader-byte code if requested (HLSL / Metal)
    Bytecode& bytecode = compiled_programs.bytecode;
    if (byte_code or Slang::is_spirv(slang))
    {
        compiled_programs.bytecode = Bytecode::compile("TODO: GET TEMP DIR", inp, spirvcross, slang);
        if (!bytecode.errors.empty())
        {
            bool has_errors = false;
            for (const ErrMsg &err : bytecode.errors)
            {
                if (err.type == ErrMsg::ERROR)
                {
                    has_errors = true;
                }
                err.print(ErrMsg::GCC);
            }
            if (has_errors)
            {
                return {};
            }
        }
    }

    // build merged Reflection info
    const Reflection reflection = Reflection::build(inp, spirvcross);
    if (reflection.error.valid())
    {
        reflection.error.print(ErrMsg::GCC);
        return {};
    }

    // Populating compiled_programs
    for (const ProgramReflection &prog : reflection.progs)
    {
        compiled_programs.programs.push_back({});
        ShaderProgram &shaderProgram = compiled_programs.programs.back();
        sg_shader_desc &desc = shaderProgram.shader_desc;

        shaderProgram.name = prog.name;
        desc.label = (prog.name + "_shader").c_str();
        
        for (int stage_index = 0; stage_index < ShaderStage::Num; stage_index++)
        {
            const StageReflection &refl = prog.stages[stage_index];
            ShaderStage::Enum shaderStage = ShaderStage::from_index(stage_index);

            sg_shader_function *shaderFunction;
            if (shaderStage == ShaderStage::Vertex)
            {
                if (!prog.has_vs())
                    continue;
                shaderFunction = &desc.vertex_func;
            }
            else if (shaderStage == ShaderStage::Fragment)
            {
                if (!prog.has_fs())
                    continue;
                shaderFunction = &desc.fragment_func;
            }
            else if (shaderStage == ShaderStage::Compute)
            {
                if (!prog.has_cs())
                    continue;
                shaderFunction = &desc.compute_func;
            }
            else
            {
                continue;
            }

            if (byte_code)
            {
                const BytecodeBlob *blob = bytecode.find_blob_by_snippet_index(prog.stage(shaderStage).snippet_index);
                shaderFunction->bytecode.ptr = blob->data.data();
                shaderFunction->bytecode.size = blob->data.size() * sizeof(uint8_t);
            }
            else
            {
                const shdc::SpirvcrossSource *source = spirvcross.find_source_by_snippet_index(prog.stage(shaderStage).snippet_index);
                shaderFunction->source = (const char *)source->source_code.data();
                const char *d3d11_tgt = hlsl_target(slang, shaderStage);
                if (d3d11_tgt)
                {
                    shaderFunction->d3d11_target = d3d11_tgt;
                }
            }

            shaderFunction->entry = refl.entry_point_by_slang(slang).c_str();
        }
        if (Slang::is_msl(slang) && prog.has_cs())
        {
            desc.mtl_threads_per_threadgroup.x = prog.cs().cs_workgroup_size[0];
            desc.mtl_threads_per_threadgroup.y = prog.cs().cs_workgroup_size[1];
            desc.mtl_threads_per_threadgroup.z = prog.cs().cs_workgroup_size[2];
        }
        if (prog.has_vs())
        {
            for (int attr_index = 0; attr_index < StageAttr::Num; attr_index++)
            {
                const StageAttr &attr = prog.vs().inputs[attr_index];
                if (attr.slot >= 0)
                {
                    desc.attrs[attr_index].base_type = attr_basetype(attr.type_info.basetype());
                    if (Slang::is_glsl(slang))
                    {
                        desc.attrs[attr_index].glsl_name = attr.name.c_str();
                    }
                    else if (Slang::is_hlsl(slang))
                    {
                        desc.attrs[attr_index].hlsl_sem_name = attr.sem_name.c_str();
                        desc.attrs[attr_index].hlsl_sem_index = attr.sem_index;
                    }
                }
            }
        }
        for (int ub_index = 0; ub_index < MaxUniformBlocks; ub_index++)
        {
            const UniformBlock *ub = prog.bindings.find_uniform_block_by_sokol_slot(ub_index);
            if (ub)
            {
                shaderProgram.uniforms.push_back({});
                shaderProgram.uniforms.back().name = ub->name;
                shaderProgram.uniforms.back().size = ub->struct_info.size;
                shaderProgram.uniforms.back().slot = ub->sokol_slot;

                const std::string ubn = fmt::format("desc.uniform_blocks[{}]", ub_index);
                desc.uniform_blocks[ub_index].stage = shader_stage(ub->stage);
                desc.uniform_blocks[ub_index].layout = SG_UNIFORMLAYOUT_STD140;
                desc.uniform_blocks[ub_index].size = roundup(ub->struct_info.size, 16);
                if (Slang::is_hlsl(slang))
                {
                    desc.uniform_blocks[ub_index].hlsl_register_b_n = ub->hlsl_register_b_n;
                }
                else if (Slang::is_msl(slang))
                {
                    desc.uniform_blocks[ub_index].msl_buffer_n = ub->msl_buffer_n;
                }
                else if (Slang::is_wgsl(slang))
                {
                    desc.uniform_blocks[ub_index].wgsl_group0_binding_n = ub->wgsl_group0_binding_n;
                }
                else if (Slang::is_spirv(slang))
                {
                    desc.uniform_blocks[ub_index].spirv_set0_binding_n = ub->spirv_set0_binding_n;
                }
                else if (Slang::is_glsl(slang) && (ub->struct_info.struct_items.size() > 0))
                {
                    if (ub->flattened)
                    {
                        // NOT A BUG (to take the type from the first struct item, but the size from the toplevel ub)
                        desc.uniform_blocks[ub_index].glsl_uniforms[0].type = flattened_uniform_type(ub->struct_info.struct_items[0].type);
                        desc.uniform_blocks[ub_index].glsl_uniforms[0].array_count = roundup(ub->struct_info.size, 16) / 16;
                        desc.uniform_blocks[ub_index].glsl_uniforms[0].glsl_name = ub->name.c_str();
                    }
                    else
                    {
                        for (int u_index = 0; u_index < (int)ub->struct_info.struct_items.size(); u_index++)
                        {
                            const Type &u = ub->struct_info.struct_items[u_index];
                            const std::string un = fmt::format("{}.glsl_uniforms[{}]", ubn, u_index);
                            desc.uniform_blocks[ub_index].glsl_uniforms[u_index].type = uniform_type(u.type);
                            desc.uniform_blocks[ub_index].glsl_uniforms[u_index].array_count = u.array_count;
                            desc.uniform_blocks[ub_index].glsl_uniforms[u_index].glsl_name = (ub->inst_name + "." + u.name).c_str();
                        }
                    }
                }
            }
        }
        for (int view_index = 0; view_index < MaxViews; view_index++)
        {
            const Bindings::View view = prog.bindings.get_view_by_sokol_slot(view_index);
            if (view.type == BindSlot::Type::Texture)
            {
                const Texture *tex = &view.texture;
                const std::string &tn = fmt::format("desc.views[{}].texture", view_index);
                desc.views[view_index].texture.stage = shader_stage(tex->stage);
                desc.views[view_index].texture.image_type = image_type(tex->type);
                desc.views[view_index].texture.sample_type = image_sample_type(tex->sample_type);
                desc.views[view_index].texture.multisampled = tex->multisampled;
                if (Slang::is_hlsl(slang))
                {
                    desc.views[view_index].texture.hlsl_register_t_n = tex->hlsl_register_t_n;
                }
                else if (Slang::is_msl(slang))
                {
                    desc.views[view_index].texture.msl_texture_n = tex->msl_texture_n;
                }
                else if (Slang::is_wgsl(slang))
                {
                    desc.views[view_index].texture.wgsl_group1_binding_n = tex->wgsl_group1_binding_n;
                }
                else if (Slang::is_spirv(slang))
                {
                    desc.views[view_index].texture.spirv_set1_binding_n = tex->spirv_set1_binding_n;
                }
            }
            else if (view.type == BindSlot::Type::StorageBuffer)
            {
                const StorageBuffer *sbuf = &view.storage_buffer;
                const std::string &sbn = fmt::format("desc.views[{}].storage_buffer", view_index);
                desc.views[view_index].storage_buffer.stage = shader_stage(sbuf->stage);
                desc.views[view_index].storage_buffer.readonly = sbuf->readonly;
                if (Slang::is_hlsl(slang))
                {
                    if (sbuf->hlsl_register_t_n >= 0)
                    {
                        desc.views[view_index].storage_buffer.hlsl_register_t_n = sbuf->hlsl_register_t_n;
                    }
                    if (sbuf->hlsl_register_u_n >= 0)
                    {
                        desc.views[view_index].storage_buffer.hlsl_register_u_n = sbuf->hlsl_register_u_n;
                    }
                }
                else if (Slang::is_msl(slang))
                {
                    desc.views[view_index].storage_buffer.msl_buffer_n = sbuf->msl_buffer_n;
                }
                else if (Slang::is_wgsl(slang))
                {
                    desc.views[view_index].storage_buffer.wgsl_group1_binding_n = sbuf->wgsl_group1_binding_n;
                }
                else if (Slang::is_spirv(slang))
                {
                    desc.views[view_index].storage_buffer.spirv_set1_binding_n = sbuf->spirv_set1_binding_n;
                }
                else if (Slang::is_glsl(slang))
                {
                    desc.views[view_index].storage_buffer.glsl_binding_n = sbuf->glsl_binding_n;
                }
            }
            else if (view.type == BindSlot::Type::StorageImage)
            {
                const StorageImage *simg = &view.storage_image;
                const std::string &sin = fmt::format("desc.views[{}].storage_image", view_index);
                desc.views[view_index].storage_image.stage = shader_stage(simg->stage);
                desc.views[view_index].storage_image.image_type = image_type(simg->type);
                desc.views[view_index].storage_image.access_format = storage_pixel_format(simg->access_format);
                desc.views[view_index].storage_image.writeonly = simg->writeonly;
                if (Slang::is_hlsl(slang))
                {
                    desc.views[view_index].storage_image.hlsl_register_u_n = simg->hlsl_register_u_n;
                }
                else if (Slang::is_msl(slang))
                {
                    desc.views[view_index].storage_image.msl_texture_n = simg->msl_texture_n;
                }
                else if (Slang::is_wgsl(slang))
                {
                    desc.views[view_index].storage_image.wgsl_group1_binding_n = simg->wgsl_group1_binding_n;
                }
                else if (Slang::is_spirv(slang))
                {
                    desc.views[view_index].storage_image.spirv_set1_binding_n = simg->spirv_set1_binding_n;
                }
                else if (Slang::is_glsl(slang))
                {
                    desc.views[view_index].storage_image.glsl_binding_n = simg->glsl_binding_n;
                }
            }
        }
        for (int smp_index = 0; smp_index < MaxSamplers; smp_index++)
        {
            const Sampler *smp = prog.bindings.find_sampler_by_sokol_slot(smp_index);
            if (smp)
            {
                const std::string sn = fmt::format("desc.samplers[{}]", smp_index);
                desc.samplers[smp_index].stage = shader_stage(smp->stage);
                desc.samplers[smp_index].sampler_type = sampler_type(smp->type);
                if (Slang::is_hlsl(slang))
                {
                    desc.samplers[smp_index].hlsl_register_s_n = smp->hlsl_register_s_n;
                }
                else if (Slang::is_msl(slang))
                {
                    desc.samplers[smp_index].msl_sampler_n = smp->msl_sampler_n;
                }
                else if (Slang::is_wgsl(slang))
                {
                    desc.samplers[smp_index].wgsl_group1_binding_n = smp->wgsl_group1_binding_n;
                }
                else if (Slang::is_spirv(slang))
                {
                    desc.samplers[smp_index].spirv_set1_binding_n = smp->spirv_set1_binding_n;
                }
            }
        }
        for (int tex_smp_index = 0; tex_smp_index < MaxTextureSamplers; tex_smp_index++)
        {
            const TextureSampler *tex_smp = prog.bindings.find_texture_sampler_by_sokol_slot(tex_smp_index);
            if (tex_smp)
            {
                const std::string tsn = fmt::format("desc.texture_sampler_pairs[{}]", tex_smp_index);
                desc.texture_sampler_pairs[tex_smp_index].stage = shader_stage(tex_smp->stage);
                desc.texture_sampler_pairs[tex_smp_index].view_slot = prog.bindings.find_texture_by_name(tex_smp->texture_name)->sokol_slot;
                desc.texture_sampler_pairs[tex_smp_index].sampler_slot = prog.bindings.find_sampler_by_name(tex_smp->sampler_name)->sokol_slot;
                if (Slang::is_glsl(slang))
                {
                    desc.texture_sampler_pairs[tex_smp_index].glsl_name = tex_smp->name.c_str();
                }
            }
        }

        for (const StageAttr &attr : prog.vs().inputs)
        {
            if (attr.slot >= 0)
            {
                shaderProgram.vertex_attributes.push_back({});
                shaderProgram.vertex_attributes.back().first = attr.slot;
                shaderProgram.vertex_attributes.back().second = vertex_format(attr.type_info.type);
            }
        }
    }

    return compiled_programs;
}
