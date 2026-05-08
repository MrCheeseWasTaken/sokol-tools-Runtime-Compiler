# Changes made

- added sokol library as gitmodule, [ext/sokol](ext/sokol)
- added [ext/sokol_impl](ext/sokol_impl)
- added library sokol_gfx to build
- added library sokol_app to build

- added library Shader_Compiler
- added exe example that uses Shader_Compiler

- shdc now compiles as a library

- moved ```main.cc``` to [shdc_exe/main.cc](src/shdc_exe/main.cc)

- sokol-shdc.exe links shdc
    - removed ```main.cc``` from sources of shdc (now) library

- [shdc/input.h](src/shdc/input.h) | [shdc/input.cc](src/shdc/input.cc)
    - added ```cpp static Input just_parse(const std::string& shader_code);```
        - substitutes ```cpp load_and_parse```

- [shdc/bytecode.h](src/shdc/bytecode.h) | [shdc/bytecode.cc](src/shdc/bytecode.cc)
    - added override ```cpp static Bytecode compile(std::string& tmpdir, const Input& inp, const Spirvcross& spirvcross, Slang::Enum slang);```
        - substitutes ```cpp static Bytecode compile(const Args& args, const Input& inp, const Spirvcross& spirvcross, Slang::Enum slang);```
        - The original ``compile`` now calls the new override to reduce code duplication
        - TODO: Get ```std::string tmpdir``` for the function in [ShaderCompiler.cpp](src/ShaderCompiler/ShaderCompiler.cpp)

- [shdc/reflection.h](src/shdc/reflection.h) | [shdc/bytecode.cc](src/shdc/reflection.cc)
    - added override ```static Reflection build(const Input& inp, const Spirvcross& spirvcross);```
        - substitutes ```cpp static Reflection build(const Args& args, const Input& inp, const std::array<Spirvcross,Slang::Num>& spirvcross);```
        - the original ```build``` now calls the new override to reduce code duplication

