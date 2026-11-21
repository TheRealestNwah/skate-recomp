#include <bit>
#include <cstdint>

#include <Windows.h>
#include <rex/ppc.h>
#include <rex/runtime.h>
#include <rex/system/xmemory.h>

namespace {

// The Guest ring-space check reads this pointer from its graphics device state.
constexpr uint32_t kGraphicsStateWritebackOffset = 10896;
constexpr uint32_t kPollsBeforeWait = 256;
thread_local uint32_t guest_ring_polls = 0;

struct PhysicalAlias {
    uint32_t Resolve(const rex::memory::Memory& memory, uint32_t address) {
        if (address != virtual_address) {
            virtual_address = address;
            physical_address = memory.GetPhysicalAddress(address);
        }
        return physical_address;
    }

    uint32_t virtual_address = UINT32_MAX;
    uint32_t physical_address = UINT32_MAX;
};

const rex::memory::Memory* GuestMemory() {
    const auto* runtime = rex::Runtime::instance();
    return runtime ? runtime->memory() : nullptr;
}

} // namespace

void SkateNotifyVBlankGateClear(PPCRegister& writeback, PPCRegister& value) {
    if (value.u32 != 0) {
        return;
    }
    using Notify = void (*)(uint32_t);
    static const auto notify = reinterpret_cast<Notify>(
        GetProcAddress(GetModuleHandleW(L"rexgpu-xenos.dll"), "RexNotifySkateVBlankGateClear"));
    const auto* memory = GuestMemory();
    if (notify && memory) {
        static thread_local PhysicalAlias alias;
        notify(alias.Resolve(*memory, writeback.u32 + sizeof(uint32_t)));
    }
}

void SkateBeginGuestRingPoll() {
    guest_ring_polls = 0;
}

void SkateWaitForGuestRingProgress(PPCRegister& device, PPCRegister& held_value) {
    if (++guest_ring_polls < kPollsBeforeWait) {
        return;
    }
    guest_ring_polls = 0;
    using Wait = void (*)(uint32_t, uint32_t);
    static const auto wait = reinterpret_cast<Wait>(
        GetProcAddress(GetModuleHandleW(L"rexgpu-xenos.dll"), "RexWaitForSkateGuestRingProgress"));
    const auto* memory = GuestMemory();
    if (wait && memory) {
        const auto* writeback = memory->TranslateVirtual<const uint32_t*>(
            device.u32 + kGraphicsStateWritebackOffset);
        static thread_local PhysicalAlias alias;
        wait(alias.Resolve(*memory, std::byteswap(*writeback)), held_value.u32);
    }
}
