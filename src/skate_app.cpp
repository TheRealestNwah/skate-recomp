#include <memory>

#include <rex/rex_app.h>

#include "kernel_audit.h"

rex::PPCImageInfo SkateGuestImage();

namespace {

class SkateApp final : public rex::ReXApp {
public:
    explicit SkateApp(rex::ui::WindowedAppContext& context)
        : ReXApp(context, "skateport", SkateGuestImage()) {}

private:
    void OnPreSetup(rex::RuntimeConfig& config) override {
        if (config.gpu_plugin.empty()) {
            config.gpu_plugin = "xenos";
        }
    }

    void OnPostLoadXexImage() override {
        LogKernelImportCoverage(*runtime());
    }
};

std::unique_ptr<rex::ui::WindowedApp> CreateSkateApp(rex::ui::WindowedAppContext& context) {
    return std::make_unique<SkateApp>(context);
}

} // namespace

REX_DEFINE_APP(skateport, CreateSkateApp)
