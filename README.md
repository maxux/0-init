# 0-init
This is a small init process intended to start and restart 0-core system.

This init system won't do anything more than restarting 0-core if it crash or gracefully restart
in case of update or specialy requested.

# Notes about 0-core
Here are some points which needs to change or be discussed about 0-core to handle correctly a process restart:

## System Services
Like any RC system, we should keep track/checking whenever a process is already running.
Currently, if core0 start again, it restarts sshd, haveged, ntpd, syslogd, ... this should not happens.

All of theses process already write their pid into /var/run/[process].pid, as usual. Since /var/run is part
of the root tmpfs, it's wiped across reboot (stateless system). This directory should be watched in order
to know what to restart or not.

## Firewall
If core0 restarts, it will assumes default ruleset are applied. This is not true anymore.

Moreover, it's a mistake to keeping state in memory about firewall rules. When doing some action
to the firewall, we should **always** query nft current state, and not rely on memory state. If
the firewall is (for some reason) modified externally, this will lead to an unexpected result.

## Splash
In order to debug easily, adding a kernel option like `nosplash` is interresting to see what
happens without being annoyed with the splash screen. Running as « Agent Mode » is not enough since
we want the full power of core0 running, but still want to disable splash
