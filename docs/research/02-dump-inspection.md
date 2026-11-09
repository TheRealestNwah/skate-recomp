# skate. disc inspection

Inspected the user-supplied USA retail disc on November 6, 2026. Tracking issue [#3](https://github.com/TheRealestNwah/skate-recomp/issues/3).

## Method

XboxDev extract-xiso build `202609111233` extracted the disc next to its ISO. Every extracted path and size matched `extract-xiso -l`, for 141 files and 5,338,829,814 bytes. The XEX header and BIG directories were read with `tools/inspect_disc.py`. PE sections and import ordinals came from `tools/inspect_xex.cpp` using the ReXGlue 0.10.1 loader in tool mode. See [the extraction and inspection commands](../../CLAUDE.md#local-disc-inspection), [extract-xiso](https://github.com/XboxDev/extract-xiso/releases/tag/build-202609111233) and [ReXGlue’s XEX structures](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/include/rex/system/util/xex2_info.h).

## Disc executable

| Field | Value |
| --- | --- |
| Executable | `default.xex` (`XEX2`, title module) |
| Title ID | `45410813` |
| Media ID | `7D3C6C51` |
| Execution version and base version | `0.0.0.1` (`00000001` each) |
| Region flags | `000000FF` (NTSC-U) |
| Allowed media flags | `00000004` |
| XEX size | 6,291,456 bytes |
| XEX SHA-256 | `fb1b41031ff5cf2385a16f37ef3af2f303f35fcc5fdda8dd2095ebdbefe4b095` |
| Payload offset | `0x4000` |
| Encryption | Normal (`1`, AES) |
| Compression | Normal (`2`, LZX), 32 KiB window |
| Loaded image | `0x82000000`, 19,202,048 bytes (`0x1250000`) |
| Entry point | `0x82DF9450` |
| XEX page descriptors | 293 |

### PE sections

These are virtual addresses and sizes after the SDK loads the image.

| Section | Guest address | Virtual size (bytes) | Flags |
| --- | --- | ---: | --- |
| `.rdata` | `0x82000400` | 1,646,048 | R |
| `.pdata` | `0x82192200` | 378,824 | R |
| `.text` | `0x821F0000` | 15,112,524 | RX |
| `.data` | `0x83060000` | 1,466,112 | RW |
| `.XBMOVIE` | `0x831C6000` | 12 | RW |
| `.tls` | `0x831C6200` | 13 | RW |
| `.edata` | `0x831D0000` | 27,596 | R |
| `.idata` | `0x831E0000` | 1,518 | RW |
| `.XBLD` | `0x831F0000` | 176 | R |
| `.reloc` | `0x831F0200` | 909,968 | R |

### Import libraries

| Library | Unique imported ordinals | Linked version | Minimum version |
| --- | ---: | --- | --- |
| `xam` | 159 | `2.0.5766.0` | `2.0.5766.0` |
| `xboxkrnl` | 191 | `2.0.5766.0` | `2.0.5766.0` |

All 350 ordinals resolve to names in the SDK export tables. The imports cover memory, files, synchronization, XAM profiles and input, graphics, XMA and network services. Name resolution establishes which API each ordinal refers to. Runtime implementation coverage will be checked in M3.

<details>
<summary>Import ordinals and names</summary>

#### xam

```text
0001 NetDll_WSAStartup
0002 NetDll_WSACleanup
0003 NetDll_socket
0004 NetDll_closesocket
0005 NetDll_shutdown
0006 NetDll_ioctlsocket
0007 NetDll_setsockopt
0008 NetDll_getsockopt
0009 NetDll_getsockname
000A NetDll_getpeername
000B NetDll_bind
000C NetDll_connect
000D NetDll_listen
000E NetDll_accept
000F NetDll_select
0010 NetDll_WSAGetOverlappedResult
0011 NetDll_WSACancelOverlappedIO
0012 NetDll_recv
0013 NetDll_WSARecv
0014 NetDll_recvfrom
0015 NetDll_WSARecvFrom
0016 NetDll_send
0017 NetDll_WSASend
0018 NetDll_sendto
0019 NetDll_WSASendTo
001A NetDll_inet_addr
001B NetDll_WSAGetLastError
001C NetDll_WSASetLastError
001D NetDll_WSACreateEvent
001E NetDll_WSACloseEvent
001F NetDll_WSASetEvent
0020 NetDll_WSAResetEvent
0021 NetDll_WSAWaitForMultipleEvents
0022 NetDll___WSAFDIsSet
0023 NetDll_WSAEventSelect
0033 NetDll_XNetStartup
0034 NetDll_XNetCleanup
0035 NetDll_XNetRandom
0036 NetDll_XNetCreateKey
0037 NetDll_XNetRegisterKey
0038 NetDll_XNetUnregisterKey
0039 NetDll_XNetXnAddrToInAddr
003A NetDll_XNetServerToInAddr
003B NetDll_XNetTsAddrToInAddr
003C NetDll_XNetInAddrToXnAddr
003D NetDll_XNetInAddrToServer
003E NetDll_XNetInAddrToString
003F NetDll_XNetUnregisterInAddr
0040 NetDll_XNetXnAddrToMachineId
0041 NetDll_XNetConnect
0042 NetDll_XNetGetConnectStatus
0043 NetDll_XNetDnsLookup
0044 NetDll_XNetDnsRelease
0045 NetDll_XNetQosListen
0046 NetDll_XNetQosLookup
0047 NetDll_XNetQosServiceLookup
0048 NetDll_XNetQosRelease
0049 NetDll_XNetGetTitleXnAddr
004A NetDll_XNetGetDebugXnAddr
004B NetDll_XNetGetEthernetLinkStatus
004C NetDll_XNetGetBroadcastVersionStatus
004D NetDll_XNetQosGetListenStats
004E NetDll_XNetGetOpt
004F NetDll_XNetSetOpt
0135 XNetLogonGetMachineID
0136 XNetLogonGetTitleID
0190 XamInputGetCapabilities
0191 XamInputGetState
0192 XamInputSetState
0198 XamInputGetKeystrokeEx
01A4 XamLoaderLaunchTitle
01A6 XamLoaderSetLaunchData
01A7 XamLoaderGetLaunchDataSize
01A8 XamLoaderGetLaunchData
01A9 XamLoaderTerminateTitle
01AF XamTaskSchedule
01B1 XamTaskCloseHandle
01B3 XamTaskShouldExit
01D7 Refresh
01DA XCustomSetDynamicActions
01DC XCustomGetLastActionPressEx
01DD XCustomRegisterDynamicActions
01DF XCustomGetCurrentGamercard
01EA XamAlloc
01EC XamFree
01F4 XMsgInProcessCall
01F6 XMsgSystemProcessCall
01F7 XMsgStartIORequest
01F8 XMsgCancelIORequest
01FC XMsgStartIORequestEx
020A XamUserGetXUID
020E XamUserGetName
0210 XamUserGetSigninState
0212 XamUserCheckPrivilege
0213 XamUserAreUsersFriends
0217 XamUserGetMembershipTierFromXUID
0218 XamUserGetOnlineCountryFromXUID
0219 XamUserReadProfileSettings
021A XamUserWriteProfileSettings
0227 XamUserGetSigninInfo
024E XamCreateEnumeratorHandle
024F XamGetPrivateEnumStructureFromHandle
0250 XamEnumerate
0259 XamContentCreateEx
025A XamContentClose
025B XamContentDelete
025C XamContentCreateEnumerator
025D XamContentCreateDeviceEnumerator
025E XamContentGetDeviceData
025F XamContentGetDeviceName
0260 XamContentSetThumbnail
0261 XamContentGetThumbnail
0262 XamContentGetCreator
0263 XamContentLaunchImage
0265 XamContentGetDeviceState
0266 XamContentGetLicenseMask
0267 XamContentFlush
0268 XamContentResolve
026A XamContentInstall
0280 XamGetExecutionId
0282 XamGetSystemVersion
028A XamNotifyCreateListener
028B XNotifyGetNext
028C XNotifyPositionUI
028D XNotifyDelayUI
02BC XamShowSigninUI
02BE XamShowSigninUIp
02BF XamShowFriendsUI
02C0 XamShowMessagesUI
02C1 XamShowKeyboardUI
02C2 XamShowQuickChatUI
02C3 XamShowVoiceMailUI
02C5 XamShowAchievementsUI
02C6 XamShowPlayerReviewUI
02C7 XamShowMarketplaceUI
02C8 XamShowPlayersUI
02CA XamShowMessageBoxUI
02CB XamShowDeviceSelectorUI
02CC XamShowMessageComposeUI
02CD XamShowGameInviteUI
02CE XamShowFriendRequestUI
02D5 XamShowGamerCardUIForXUID
02D9 XamShowDirtyDiscErrorUI
02DC XamShowMessageBoxUIEx
02EE XamUserCreateAchievementEnumerator
02F0 XamWriteGamerTile
02F5 XamReadTileToTexture
02F7 XamUserCreateStatsEnumerator
030A XamParseGamerTileKey
030C XamVoiceCreate
030D XamVoiceHeadsetPresent
030E XamVoiceSubmitPacket
030F XamVoiceClose
0316 XamSessionCreateHandle
0317 XamSessionRefObjByHandle
03CB XGetAVPack
03CC XGetGameRegion
03CD XGetLanguage
03D1 XGetVideoMode
```

#### xboxkrnl

```text
0001 DbgBreakPoint
0003 DbgPrint
0009 ExAllocatePool
000A ExAllocatePoolWithTag
000D ExCreateThread
000E ExEventObjectType
000F ExFreePool
0010 ExGetXConfigSetting
0015 ExRegisterTitleTerminateNotification
0017 ExSemaphoreObjectType
0019 ExTerminateThread
001B ExThreadObjectType
001C ExTimerObjectType
0020 FscGetCacheElementCount
0021 FscSetCacheElementCount
0028 HalReturnToFirmware
003B IoDismountVolume
003C IoDismountVolumeByFileHandle
004D KeAcquireSpinLockAtRaisedIrql
0052 KeBugCheck
0053 KeBugCheckEx
0059 KeDebugMonitorData
005A KeDelayExecutionThread
005D KeEnableFpuExceptions
005F KeEnterCriticalRegion
0066 KeGetCurrentProcessType
006B KeLockL2
006C KeUnlockL2
0074 KeInitializeSemaphore
007D KeLeaveCriticalRegion
007F KePulseEvent
0081 KeQueryBasePriorityThread
0083 KeQueryPerformanceFrequency
0084 KeQuerySystemTime
0085 KeRaiseIrqlToDpcLevel
0088 KeReleaseSemaphore
0089 KeReleaseSpinLockFromRaisedIrql
008F KeResetEvent
0092 KeResumeThread
0097 KeSetAffinityThread
0099 KeSetBasePriorityThread
009C KeSetDisableBoostThread
009D KeSetEvent
00AD KeTimeStampBundle
00AE KeTryToAcquireSpinLockAtRaisedIrql
00AF KeWaitForMultipleObjects
00B0 KeWaitForSingleObject
00B1 KfAcquireSpinLock
00B3 KfLowerIrql
00B4 KfReleaseSpinLock
00B9 MmAllocatePhysicalMemory
00BA MmAllocatePhysicalMemoryEx
00BD MmFreePhysicalMemory
00BE MmGetPhysicalAddress
00C4 MmQueryAddressProtect
00C5 MmQueryAllocationSize
00C6 MmQueryStatistics
00C7 MmSetAddressProtect
00CC NtAllocateVirtualMemory
00CD NtCancelTimer
00CE NtClearEvent
00CF NtClose
00D1 NtCreateEvent
00D2 NtCreateFile
00D4 NtCreateMutant
00D5 NtCreateSemaphore
00D7 NtCreateTimer
00D9 NtDeviceIoControlFile
00DA NtDuplicateObject
00DB NtFlushBuffersFile
00DC NtFreeVirtualMemory
00DF NtOpenFile
00E1 NtProtectVirtualMemory
00E2 NtPulseEvent
00E3 NtQueueApcThread
00E4 NtQueryDirectoryFile
00E7 NtQueryFullAttributesFile
00E8 NtQueryInformationFile
00EE NtQueryVirtualMemory
00EF NtQueryVolumeInformationFile
00F0 NtReadFile
00F1 NtReadFileScatter
00F2 NtReleaseMutant
00F3 NtReleaseSemaphore
00F5 NtResumeThread
00F6 NtSetEvent
00F7 NtSetInformationFile
00FA NtSetTimerEx
00FB NtSignalAndWaitForSingleObjectEx
00FC NtSuspendThread
00FD NtWaitForSingleObjectEx
00FE NtWaitForMultipleObjectsEx
00FF NtWriteFile
0101 NtYieldExecution
0103 ObCreateSymbolicLink
0104 ObDeleteSymbolicLink
0105 ObDereferenceObject
010B ObLookupThreadByThreadId
010D ObOpenObjectByName
010E ObOpenObjectByPointer
0110 ObReferenceObjectByHandle
0119 RtlCaptureContext
011A RtlCompareMemory
011B RtlCompareMemoryUlong
0125 RtlEnterCriticalSection
0126 RtlFillMemoryUlong
012B RtlImageXexHeaderField
012C RtlInitAnsiString
012E RtlInitializeCriticalSection
012F RtlInitializeCriticalSectionAndSpinCount
0130 RtlLeaveCriticalSection
0133 RtlMultiByteToUnicodeN
0135 RtlNtStatusToDosError
0136 RtlRaiseException
013B sprintf
013F RtlTimeFieldsToTime
0140 RtlTimeToTimeFields
0141 RtlTryEnterCriticalSection
0143 RtlUnicodeToMultiByteN
0147 RtlUnwind
014D _vsnprintf
0152 KeTlsAlloc
0153 KeTlsFree
0154 KeTlsGetValue
0155 KeTlsSetValue
0158 XboxKrnlVersion
0192 XeCryptSha
0193 XexExecutableModuleHandle
0194 XexCheckExecutablePrivilege
0195 XexGetModuleHandle
0196 XexGetModuleSection
0197 XexGetProcedureAddress
0199 XexLoadImage
01A1 XexUnloadImage
01A2 XexUnloadImageAndExitThread
01A5 __C_specific_handler
01AE ExLoadedCommandLine
01B1 VdCallGraphicsNotificationRoutines
01B4 VdEnableDisableClockGating
01B6 VdEnableRingBufferRPtrWriteBack
01B9 VdGetCurrentDisplayGamma
01BA VdGetCurrentDisplayInformation
01BD VdGetSystemCommandBuffer
01BE VdGlobalDevice
01C0 VdGpuClockInMHz
01C1 VdHSIOCalibrationLock
01C2 VdInitializeEngines
01C3 VdInitializeRingBuffer
01C5 VdInitializeScalerCommandBuffer
01C6 VdIsHSIOTrainingSucceeded
01C7 VdPersistDisplay
01C8 VdQuerySystemCommandBuffer
01C9 VdQueryVideoFlags
01CA VdQueryVideoMode
01D3 VdSetDisplayMode
01D5 VdSetGraphicsInterruptCallback
01D8 VdSetSystemCommandBuffer
01D9 VdSetSystemCommandBufferGpuIdentifierAddress
01DC VdShutdownEngines
01DF KiApcNormalRoutineNop
01F3 XAudioRegisterRenderDriverClient
01F4 XAudioUnregisterRenderDriverClient
01F5 XAudioSubmitRenderDriverFrame
01F7 XAudioGetVoiceCategoryVolumeChangeMask
01F8 XAudioGetVoiceCategoryVolume
01FD XAudioQueryDriverPerformance
01FF XAudioGetSpeakerConfig
0224 XMACreateContext
0225 XMAInitializeContext
0226 XMAReleaseContext
0227 XMAEnableContext
0228 XMADisableContext
0229 XMAGetOutputBufferWriteOffset
022A XMASetOutputBufferReadOffset
022B XMAGetOutputBufferReadOffset
022C XMASetOutputBufferValid
022D XMAIsOutputBufferValid
022E XMASetInputBuffer0Valid
022F XMAIsInputBuffer0Valid
0230 XMASetInputBuffer1Valid
0231 XMAIsInputBuffer1Valid
0232 XMASetInputBuffer0
0233 XMASetInputBuffer1
0256 XeKeysConsolePrivateKeySign
0257 XeKeysConsoleSignatureVerification
0259 StfsCreateDevice
025A StfsControlDevice
025B VdSwap
0266 KeCertMonitorData
0269 VdRetrainEDRAM
026A VdRetrainEDRAMWorker
```

</details>

## Archive inventory

The disc contains 29 `BIG4` and four `BIGF` archives with 16,531 indexed members in total. Both variants in this dump use a little-endian total length, big-endian entry count and directory length, and big-endian member offset and length followed by a NUL-terminated name. Every listed member range fits its archive. Nested `.big` members are counted in the outer directory only.

The largest member group is `.rx2` (10,686 entries). Audio archives contain `.sns`, `.snr`, `.abk` and other bank or stream files. The standalone tree has 31 `.vp6` movies, 13 `.bh` files and 12 `.rx2` files. There are no standalone `.arena` files in the dump.

| Archive | Magic | Bytes | Members |
| --- | --- | ---: | ---: |
| `data/audio/ambience.big` | `BIG4` | 44,442,944 | 14 |
| `data/audio/ambienceresident.big` | `BIG4` | 1,168 | 13 |
| `data/audio/crowds.big` | `BIG4` | 8,107,024 | 10 |
| `data/audio/emitters.big` | `BIG4` | 30,763,168 | 138 |
| `data/audio/emitters2.big` | `BIG4` | 580,808 | 5 |
| `data/audio/english/announcerspeech.big` | `BIG4` | 37,887,296 | 80 |
| `data/audio/english/cameramanspeech.big` | `BIG4` | 130,617,280 | 276 |
| `data/audio/english/livingworldspeech.big` | `BIG4` | 107,900,032 | 596 |
| `data/audio/english/maincastspeech.big` | `BIG4` | 64,841,600 | 532 |
| `data/audio/english/nis.big` | `BIG4` | 157,812,288 | 582 |
| `data/audio/fe.big` | `BIG4` | 2,979,332 | 7 |
| `data/audio/grains.big` | `BIG4` | 3,583,978 | 14 |
| `data/audio/pedestrian.big` | `BIG4` | 565,244 | 5 |
| `data/audio/rolling.big` | `BIG4` | 1,014,024 | 5 |
| `data/audio/skateboard.big` | `BIG4` | 6,584,016 | 11 |
| `data/audio/splicercollision.big` | `BIG4` | 3,991,235 | 10 |
| `data/audio/splicerskateboard.big` | `BIG4` | 2,499,946 | 17 |
| `data/audio/vehicle.big` | `BIG4` | 2,181,576 | 12 |
| `data/big/aipaths.big` | `BIG4` | 4,065,349 | 64 |
| `data/big/anim.big` | `BIG4` | 11,848,784 | 21 |
| `data/big/english.big` | `BIG4` | 339,083 | 4 |
| `data/big/miscboot.big` | `BIG4` | 1,210,216 | 25 |
| `data/big/miscgame.big` | `BIG4` | 38,829 | 97 |
| `data/big/miscload.big` | `BIG4` | 209,835 | 240 |
| `data/big/missions.big` | `BIG4` | 964,989 | 259 |
| `data/big/render.big` | `BIG4` | 2,106,338 | 632 |
| `data/big/ropa2.big` | `BIG4` | 3,865,157 | 47 |
| `data/big/scene.big` | `BIG4` | 55,674,613 | 229 |
| `data/content/assettoc.big` | `BIGF` | 215,640 | 8 |
| `data/content/createacharacter.big` | `BIGF` | 531,256,496 | 8,242 |
| `data/content/livingworld.big` | `BIGF` | 16,911,599 | 359 |
| `data/content/marquee.big` | `BIGF` | 84,555,019 | 1,285 |
| `data/fe/screens.big` | `BIG4` | 58,604,930 | 2,692 |

## Title update assessment

**[Inference] Start M3 with the disc XEX.** The execution and base versions match, and the upstream loader can decode the image and enumerate its imports. A Canary tester reported running the USA `0.0.0.1` build through completion. A Title update does not currently look necessary for title-screen bring-up. Static codegen and runtime execution against the disc build still need to verify that. The disc’s `$SystemUpdate` directory contains a console update and is separate from a title patch. [Xenia Canary compatibility report](https://github.com/xenia-canary/game-compatibility/issues/319)

## Disc file tree

Paths are relative to the disc root. The listing includes the console update present on the disc.

```text
$SystemUpdate/
  su20076000_00000000
data/
  audio/
    ambience.big
    ambienceresident.big
    crowds.big
    emitters.big
    emitters2.big
    english/
      announcerspeech.big
      cameramanspeech.big
      englishSk8AnnHals.hla
      englishSk8CMHals.hla
      englishSk8LWHals.hla
      englishSk8MCHals.hla
      livingworldspeech.big
      maincastspeech.big
      nis.big
    fe.big
    grains.big
    MixMapCOMPETITION.mxb
    MixMapSTREET.mxb
    music/
      game.mpf
      Game_Stream.mus
      ipod.mpf
      Ipod_Stream.mus
      world.mpf
      World_Stream.mus
    pedestrian.big
    rolling.big
    skate.ems
    skateboard.big
    splicercollision.big
    splicerskateboard.big
    vehicle.big
  big/
    aipaths.bh
    aipaths.big
    anim.bh
    anim.big
    english.bh
    english.big
    miscboot.bh
    miscboot.big
    miscgame.bh
    miscgame.big
    miscload.bh
    miscload.big
    missions.bh
    missions.big
    render.bh
    render.big
    ropa2.bh
    ropa2.big
    scene.bh
    scene.big
  content/
    assettoc.big
    createacharacter.bh
    createacharacter.big
    livingworld.bh
    livingworld.big
    marquee.bh
    marquee.big
    missing_model.rx2
    textures/
      cubemap.rx2
      globaltextures.rx2
      missingtexture.rx2
      particletextures.rx2
      splashtextures.rx2
    world/
      models/
        DIST_ProxyWorld.rx2
        DIST_ProxyWorld_Textures.rx2
        DIST_skybox.rx2
        DIST_skybox_Textures.rx2
        visual_treatments.rx2
        visual_treatments_Textures.rx2
      stream/
        BAM.loc
        BAM.xmm
        BAM.xsf
        BAM.xsm
        BAM.xss
        BAM.xst
        DIST_ProxyHulls.loc
        DIST_ProxyHulls.xmm
        DIST_ProxyHulls.xsf
        DIST_ProxyHulls.xsm
        DIST_ProxyHulls.xss
        DIST_ProxyHulls.xst
  db/
    skater.bin
    skater.vlt
  fe/
    memcard/
      CORRUPT1.xbx
    screens.big
    source/
      images/
        mediasave/
          MEDIASAVEEMPTY.JPG
  movies/
    Attract_english_ntsc.vp6
    Board_Promo_Almost_english_ntsc.vp6
    Board_Promo_AWS_english_ntsc.vp6
    Board_Promo_Baker_english_ntsc.vp6
    Board_Promo_Flip_english_ntsc.vp6
    Board_Promo_Girl_english_ntsc.vp6
    Board_Promo_Mystery_english_ntsc.vp6
    Board_Promo_PlanB_english_ntsc.vp6
    Board_Promo_Real_english_ntsc.vp6
    Board_Promo_Zero_english_ntsc.vp6
    Credits_english_ntsc.vp6
    EA_ID_english_ntsc.vp6
    EAHD_Bumper_english_ntsc.vp6
    Game_Intro_1_english_ntsc.vp6
    Game_Intro_1Thumb.jpg
    Game_Intro_3_english_ntsc.vp6
    Game_Intro_3Thumb.jpg
    ROP_2003_english_ntsc.vp6
    ROP_2004_english_ntsc.vp6
    ROP_2006_english_ntsc.vp6
    ROP_2007_english_ntsc.vp6
    ROP_2011_english_ntsc.vp6
    ROP_2012_english_ntsc.vp6
    ROP_2013_english_ntsc.vp6
    ROP_2014_english_ntsc.vp6
    Skate_EA_Intro_english_ntsc.vp6
    SPONSOR_ALIENThumb.jpg
    SPONSOR_ALMOSTThumb.jpg
    SPONSOR_BAKERThumb.jpg
    SPONSOR_ENJOIThumb.jpg
    SPONSOR_FLIPThumb.jpg
    SPONSOR_GIRLThumb.jpg
    SPONSOR_MYSTERYThumb.jpg
    SPONSOR_PLANBThumb.jpg
    SPONSOR_REALThumb.jpg
    SPONSOR_ZEROThumb.jpg
    Viral_Busenitz_english_ntsc.vp6
    VIRAL_BUSENITZThumb.jpg
    Viral_Carroll_english_ntsc.vp6
    VIRAL_CARROLLThumb.jpg
    Viral_Cole_english_ntsc.vp6
    VIRAL_COLEThumb.jpg
    Viral_Dill_english_ntsc.vp6
    VIRAL_DILLThumb.jpg
    VIRAL_EIGHTThumb.jpg
    Viral_Gonz_english_ntsc.vp6
    VIRAL_GONZThumb.jpg
    Viral_Hsu_english_ntsc.vp6
    VIRAL_HSUThumb.jpg
    Viral_PJ_english_ntsc.vp6
    VIRAL_PJLADThumb.jpg
    VIRAL_PJThumb.jpg
default.xex
```
