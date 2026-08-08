package com.ghostlock.app;

interface IGhostlockShellCallback {
    void onOutput(String line);
    void onExit(int code);
}
