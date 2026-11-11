r102 window (2026-11-12, s90 judgment): fetchchunk 15/15 SHA MATCH,
sha256 2b3473d9... FRESH WINDOW (differs from r94 09dd99c4 / r101 a5684e43).
Header: rounds=39 slots_written=39/509 uptime=75s state=2 write_errors=0
(structurally identical to r101 = boot reached the same depth).

VERDICT: launchd abort RECURRED at 100.93s (r101=100.967s) BUT THE STACK
CHANGED FACE:
  r101: pc 0x23945D1FC = AccessoryInteraction.handleMeDeviceUpdate ->
        MKBGetDeviceLockState failed -> abort (no-SEP, nobody answers)
  r102: pc 0x2390491FC = AONSense.dylib +0x491FC, symbolized to
        SwiftProtobuf ...LogEntry_PrivateData_ReceiverToGpsTimeConversion
        ...SwiftProtobuf.MessageAAMc +0x10 (a METACLASS symbol);
        pc sits in an autda(DA-key 0x2e3f) chain: autda x16 -> ldr x8,
        [x16,#0x40] -> mov x9,x8 -> blraa x16,x17 (auth GOT slot
        0x271beaa00). #0 = SwiftUICore +0xc95944, #24-29 =
        GenerativeFunctionsInstrumentation, #30/#31 = Foundation.
  => NEW WALL in the Swift/ObjC metaclass AUTH family (wall-86 shape),
     NOT the MKB face. Whether the AKS fix worked is UNDETERMINED: the
     MKB abort site is gone, but keybagd p12 shows the identical status
     rows (exit=0, MWL load at 97.1s) and no AKS opens=/calls= rows
     published (the probe publishes only on open/call).

Four-step wiring check (all pass at build level):
  1. kernel_args: payload.json carries offline-keystore=1
     offline-key-events=1 aks=1 ✓
  2. bsd_init.o recipe: -D XNU_NATIVE_KEYSTORE=1 present ✓ (verified in
     bsd_init.o.json)
  3. bsd_init.o: U _native_keystore_probe present (call compiled in;
     direct BL scans of base kernel + fileset come up empty because lld
     routes through veneers - object-level U symbol is the proof) ✓
  4. archive/link: KeyStoreProbe.o + NativeKeyStoreEvents.late.o in
     libShengUfs.a; nm kernel shows native_aks_device_id /
     native_keystore_probe ✓

s91 first blade: symbolize the full 32 frames (tools in ios-m2381-unixnu
temp-s90/, slide from census +1534000-equivalent this round); decide the
AKS-effectiveness question (does the AONSense wall sit BEFORE the MKB
face?); if the wall is the auth family, diff against the r94 T0SZ face
for regression; consider a probe row that publishes unconditionally
(native_keyevents_status) so next window shows the probe's stage even
when nobody opens the client.

Reverted-to anchors: r94 1f16e61d (sanity) / r101 a993df19.
