#include "kernel_audit.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string_view>

#include <rex/logging.h>
#include <rex/runtime.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/user_module.h>
#include <rex/system/xex_module.h>

namespace {

// ReXGlue v0.10.1 exports the stubs but does not mark them in its ordinal tables.
// This inventory covers the imports in the supported disc image.
constexpr std::string_view kSdkStubs[] = {
    "IoDismountVolume",
    "IoDismountVolumeByFileHandle",
    "NetDll_WSACancelOverlappedIO",
    "NetDll_WSAEventSelect",
    "NetDll_WSAGetOverlappedResult",
    "NetDll_WSARecv",
    "NetDll_WSASend",
    "NetDll_XNetConnect",
    "NetDll_XNetCreateKey",
    "NetDll_XNetDnsLookup",
    "NetDll_XNetGetBroadcastVersionStatus",
    "NetDll_XNetGetConnectStatus",
    "NetDll_XNetInAddrToServer",
    "NetDll_XNetQosGetListenStats",
    "NetDll_XNetQosLookup",
    "NetDll_XNetRegisterKey",
    "NetDll_XNetServerToInAddr",
    "NetDll_XNetSetOpt",
    "NetDll_XNetTsAddrToInAddr",
    "NetDll_XNetUnregisterInAddr",
    "NetDll_XNetUnregisterKey",
    "NetDll_getpeername",
    "NetDll_getsockname",
    "NetDll_getsockopt",
    "Refresh",
    "RtlCaptureContext",
    "RtlUnwind",
    "StfsControlDevice",
    "StfsCreateDevice",
    "VdQuerySystemCommandBuffer",
    "VdSetSystemCommandBuffer",
    "XAudioQueryDriverPerformance",
    "XCustomGetCurrentGamercard",
    "XCustomGetLastActionPressEx",
    "XCustomSetDynamicActions",
    "XNetLogonGetMachineID",
    "XNetLogonGetTitleID",
    "XamContentInstall",
    "XamContentLaunchImage",
    "XamContentResolve",
    "XamShowAchievementsUI",
    "XamShowFriendRequestUI",
    "XamShowFriendsUI",
    "XamShowGameInviteUI",
    "XamShowGamerCardUIForXUID",
    "XamShowMarketplaceUI",
    "XamShowMessageBoxUIEx",
    "XamShowMessageComposeUI",
    "XamShowMessagesUI",
    "XamShowPlayerReviewUI",
    "XamShowPlayersUI",
    "XamShowQuickChatUI",
    "XamShowSigninUIp",
    "XamShowVoiceMailUI",
    "XamTaskCloseHandle",
    "XamUserCreateStatsEnumerator",
    "XamUserGetMembershipTierFromXUID",
    "XamUserGetOnlineCountryFromXUID",
    "XamVoiceSubmitPacket",
    "XeKeysConsolePrivateKeySign",
    "XeKeysConsoleSignatureVerification",
    "XexUnloadImageAndExitThread",
    "__C_specific_handler",
};

// These exports use a fixed answer or skip the operation in this SDK version.
constexpr std::string_view kSdkFallbacks[] = {
    "FscGetCacheElementCount",
    "FscSetCacheElementCount",
    "KeEnableFpuExceptions",
    "KeLockL2",
    "KeUnlockL2",
    "NetDll_WSACleanup",
    "NetDll_XNetGetEthernetLinkStatus",
    "NetDll_XNetInAddrToXnAddr",
    "NetDll_XNetQosListen",
    "NetDll_XNetXnAddrToInAddr",
    "NetDll_XNetXnAddrToMachineId",
    "VdCallGraphicsNotificationRoutines",
    "VdEnableDisableClockGating",
    "VdGetSystemCommandBuffer",
    "VdInitializeEngines",
    "VdInitializeScalerCommandBuffer",
    "VdIsHSIOTrainingSucceeded",
    "VdPersistDisplay",
    "VdRetrainEDRAM",
    "VdRetrainEDRAMWorker",
    "VdSetDisplayMode",
    "VdSetSystemCommandBufferGpuIdentifierAddress",
    "VdShutdownEngines",
    "XCustomRegisterDynamicActions",
    "XGetAVPack",
    "XNotifyDelayUI",
    "XNotifyPositionUI",
    "XamCreateEnumeratorHandle",
    "XamFree",
    "XamGetSystemVersion",
    "XamLoaderTerminateTitle",
    "XamShowSigninUI",
    "XamTaskShouldExit",
    "XamVoiceClose",
    "XamVoiceHeadsetPresent",
};

} // namespace

void LogKernelImportCoverage(rex::Runtime& runtime) {
    const auto userModule = runtime.kernel_state()->GetExecutableModule();
    const auto* module = userModule ? userModule->xex_module() : nullptr;
    if (!module) {
        REXLOG_WARN("Boot import audit could not access the loaded XEX.");
        return;
    }

    unsigned imports = 0;
    unsigned gaps = 0;
    for (const auto& library : *module->import_libraries()) {
        for (const auto& imported : library.imports) {
            ++imports;
            const auto* symbol = runtime.export_resolver()->GetExportByOrdinal(
                library.name, static_cast<uint16_t>(imported.ordinal));
            const char* reason = nullptr;
            if (!symbol) {
                reason = "unresolved ordinal";
            } else if ((symbol->tags & rex::runtime::ExportTag::kStub) ||
                       std::ranges::find(kSdkStubs, symbol->name) != std::end(kSdkStubs)) {
                reason = "SDK stub";
            } else if (std::ranges::find(kSdkFallbacks, symbol->name) != std::end(kSdkFallbacks)) {
                reason = "SDK fixed fallback";
            } else if (symbol->type == rex::runtime::Export::Type::kVariable &&
                       (!symbol->is_implemented() || !symbol->variable_ptr)) {
                reason = "unimplemented variable";
            } else if (imported.thunk_address &&
                       !runtime.function_dispatcher()->GetFunction(imported.thunk_address)) {
                reason = "unmapped thunk";
            }
            if (reason) {
                ++gaps;
                REXLOG_WARN("Boot import {}:{:#06x} {} ({})", library.name, imported.ordinal,
                            symbol ? symbol->name : "<unknown>", reason);
            }
        }
    }
    REXLOG_INFO("Boot import audit: {} imports, {} stubbed, fixed fallback or unresolved", imports, gaps);
}
