#pragma once

// SDL unloads the GL driver when its last GL window is destroyed. The driver's
// one-time allocations then lose their owner and LeakSanitizer reports them,
// with every driver frame shown as <unknown module>. Calling this while a GL
// window and context exist takes one more reference to every loaded library,
// which keeps the driver in memory until exit.
void PinLoadedLibraries();
