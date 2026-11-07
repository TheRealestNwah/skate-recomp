#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <rex/kernel/init.h>
#include <rex/logging.h>
#include <rex/runtime.h>
#include <rex/system/user_module.h>
#include <rex/system/xex_module.h>

int main(int argc, char** argv) {
    using rex::X_STATUS;
    if (argc != 2) {
        std::cerr << "Usage: inspect_xex <absolute path to extracted default.xex>\n";
        return 2;
    }

    try {
        const auto executable = std::filesystem::canonical(argv[1]);
        const auto output = executable.parent_path() / ".inspection";
        std::filesystem::create_directories(output);
        rex::InitLogging(output / "loader.log");

        rex::Runtime runtime(executable.parent_path(), output / "user", {},
                             output / "cache", output / "metadata");
        rex::RuntimeConfig config;
        config.tool_mode = true;
        config.kernel_init = rex::kernel::InitializeKernel;
        if (runtime.Setup(std::move(config)) != X_STATUS_SUCCESS) {
            throw std::runtime_error("ReXGlue tool-mode setup failed.");
        }
        if (runtime.LoadXexImage("game:\\" + executable.filename().string()) != X_STATUS_SUCCESS) {
            throw std::runtime_error("ReXGlue could not load the disc XEX. See loader.log.");
        }
        const auto user_module = runtime.kernel_state()->GetExecutableModule();
        const auto* module = user_module ? user_module->xex_module() : nullptr;
        if (!module) {
            throw std::runtime_error("ReXGlue did not expose the loaded XEX module.");
        }

        std::ofstream report(output / "xex.tsv");
        report.exceptions(std::ios::badbit | std::ios::failbit);
        report << "image\t" << std::hex << module->base_address() << '\t'
               << module->image_size() << '\t' << module->entry_point() << '\n';
        for (const auto& section : module->binary_sections()) {
            report << "section\t" << section.name << '\t' << section.virtual_address << '\t'
                   << section.virtual_size << '\t' << section.executable << '\t'
                   << section.writable << '\n';
        }
        for (const auto& library : *module->import_libraries()) {
            report << "library\t" << library.name << '\t' << library.version.value << '\t'
                   << library.min_version.value << '\n';
            for (const auto& imported : library.imports) {
                const auto* symbol = runtime.export_resolver()->GetExportByOrdinal(
                    library.name, static_cast<uint16_t>(imported.ordinal));
                report << "import\t" << library.name << '\t' << imported.ordinal << '\t'
                       << (symbol ? symbol->name : "unknown") << '\t'
                       << imported.value_address << '\t' << imported.thunk_address << '\t'
                       << (symbol ? symbol->tags : 0) << '\n';
            }
        }

        // This decoded image is a Game file and stays next to the user's extracted XEX.
        std::ofstream image(output / "image.bin", std::ios::binary);
        image.exceptions(std::ios::badbit | std::ios::failbit);
        image.write(reinterpret_cast<const char*>(runtime.virtual_membase() + module->base_address()),
                    module->image_size());
        image.close();
        report.close();
        std::cout << "Wrote local inspection data to " << output << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
