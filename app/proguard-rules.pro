# Shizuku starts UserService classes from the ComponentName in a separate
# process.  The class is therefore not reachable from the normal Android
# component graph as far as R8 is concerned.
-keep class com.ghostlock.app.ShellUserService {
    public <init>();
    *;
}

# These AIDL endpoints cross the Shizuku Binder boundary.  Keep their
# descriptor, Stub/Proxy implementations, and generated parcel helpers.
-keep interface com.ghostlock.app.IGhostlockShell { *; }
-keep class com.ghostlock.app.IGhostlockShell$* { *; }
-keep interface com.ghostlock.app.IGhostlockShellCallback { *; }
-keep class com.ghostlock.app.IGhostlockShellCallback$* { *; }
