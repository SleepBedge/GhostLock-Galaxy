package com.ghostlock.app;

import android.os.ParcelFileDescriptor;
import com.ghostlock.app.IGhostlockShellCallback;

interface IGhostlockShell {
    void run(in ParcelFileDescriptor binary, in ParcelFileDescriptor helper,
             in ParcelFileDescriptor ksud, IGhostlockShellCallback callback);
    void stop();
}
