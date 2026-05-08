#include <string>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>

#include <ShaderCompiler.hpp>

#include "math.hpp"

struct Vertex
{
    float x, y, z;
};

sg_shader shader = {};
sg_pipeline pipeline = {};

sg_buffer vertex_buf = {};
sg_buffer index_buf = {};
sg_bindings bindings = {};

mat4 mvp;
int mvp_uniform_slot;
size_t mvp_uniform_size;

std::string SHADER_CODE = R"(
@vs vs
layout(binding=0) uniform vs_params {
    mat4 mvp;
};

in vec3 v_vertexPosition;

out vec4 f_color;

const vec4 colors[3] = {
    vec4(1, 0, 0, 1),
    vec4(0, 1, 0, 1),
    vec4(0, 0, 1, 1),
};

void main() {
    gl_Position = mvp * vec4(v_vertexPosition, 1.0);
    // gl_Position = vec4(v_vertexPosition, 1.0);
    int index = gl_VertexIndex % 3;
    f_color = colors[index] + vec4(1, 0, 0, 1);
}
@end

@fs fs
in vec4 f_color;

out vec4 FragColor;

void main() {
    FragColor = f_color + vec4(1, 0, 0, 1);
}
@end

@program UnlitShader vs fs

)";

void init()
{
    {
        sg_desc desc = {};
        desc.environment = sglue_environment();
        desc.logger.func = slog_func;
        sg_setup(desc);
    }

    shdc::ShaderCompiler::Initialize();

    shdc::CompiledShaderPrograms programs = shdc::ShaderCompiler::CompileShader(SHADER_CODE.c_str(), sg_query_backend());

    shdc::ShaderCompiler::Finalize();

    if (programs.programs.empty())
    {
        return;
    }

    shdc::ShaderProgram &program = programs.programs.front();

    shader = sg_make_shader(program.shader_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.layout.buffers[0].stride = sizeof(Vertex);
    for (int i = 0; i < program.vertex_attributes.size(); i++)
    {
        if (program.vertex_attributes[i].format != sg_vertex_format::SG_VERTEXFORMAT_INVALID)
            pip_desc.layout.attrs[program.vertex_attributes[i].slot].format = program.vertex_attributes[i].format;
    }
    for (int i = 0; i < program.uniforms.size(); i++)
    {
        shdc::ShaderUniform &uniform = program.uniforms[i];
        if (uniform.name == "vs_params")
        {
            mvp_uniform_slot = uniform.slot;
            mvp_uniform_size = uniform.size;
        }
    }
    pipeline = sg_make_pipeline(pip_desc);

    {
        std::vector<Vertex> vertices = {
            // Front (+Z)
            {-1, -1, +1},
            {+1, -1, +1},
            {+1, +1, +1},
            {-1, +1, +1},

            // Back (-Z)
            {+1, -1, -1},
            {-1, -1, -1},
            {-1, +1, -1},
            {+1, +1, -1},

            // Left (-X)
            {-1, -1, -1},
            {-1, -1, +1},
            {-1, +1, +1},
            {-1, +1, -1},

            // Right (+X)
            {+1, -1, +1},
            {+1, -1, -1},
            {+1, +1, -1},
            {+1, +1, +1},

            // Bottom (-Y)
            {-1, -1, -1},
            {+1, -1, -1},
            {+1, -1, +1},
            {-1, -1, +1},

            // Top (+Y)
            {-1, +1, +1},
            {+1, +1, +1},
            {+1, +1, -1},
            {-1, +1, -1},
        };

        sg_buffer_desc desc = {};
        desc.data.ptr = vertices.data();
        desc.data.size = vertices.size() * sizeof(Vertex);
        vertex_buf = sg_make_buffer(desc);
    }

    {
        std::vector<uint16_t> indices = {
            0, 1, 2, 0, 2, 3,       // front
            4, 5, 6, 4, 6, 7,       // back
            8, 9, 10, 8, 10, 11,    // left
            12, 13, 14, 12, 14, 15, // right
            16, 17, 18, 16, 18, 19, // bottom
            20, 21, 22, 20, 22, 23  // top
        };

        sg_buffer_desc desc = {};
        desc.data.ptr = indices.data();
        desc.data.size = indices.size() * sizeof(uint16_t);
        desc.usage.index_buffer = true;
        index_buf = sg_make_buffer(desc);
    }

    bindings.vertex_buffers[0] = vertex_buf;
    bindings.index_buffer = index_buf;
}

void frame()
{
    {
        static float time = 0;
        time += (float)sapp_frame_duration();

        mat4 model;
        mat4 view;
        mat4 proj;
        mat4 temp;

        // model: rotate
        model = mat4_rotate_y(time);
        model = mat4_mul(model, mat4_rotate_x(time * 1.1));

        // view: move camera back
        view = mat4_translate(0.0f, 0.0f, -5.0f);

        // projection
        proj = mat4_perspective(deg_to_rad(60.0f), 640.f / 480.f, 0.1f, 100.0f);

        mvp = mat4_mul(proj, mat4_mul(view, model));
    }

    sg_pass pass = {};
    pass.action.colors->load_action = SG_LOADACTION_CLEAR;
    pass.swapchain = sglue_swapchain();
    sg_begin_pass(pass);

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(bindings);
    sg_apply_uniforms(mvp_uniform_slot, {mvp.m, mvp_uniform_size});

    sg_draw(0, 36, 1);

    sg_end_pass();
    sg_commit();
}

void cleanup()
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[])
{
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.width = 640,
    desc.height = 480,
    desc.window_title = "Shader Compiler Example";
    return desc;
}
