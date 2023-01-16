# uetopia-online-subsystem
Online Subsystem plugin for Unreal Engine 5

5.1.0 Branch requires some OSS changes:  support for tournaments, and some extra delegates.

You can use a pre-patched 5.1.0 engine here:

https://github.com/uetopia/UnrealEngine/tree/5.1.0_UETOPIA

Known Issues:
There is a bug inside SocketIO, which is causing a crash on connect:  https://github.com/getnamo/SocketIOClient-Unreal/issues/358

The workaround indicated is a fix.

You can also clone from ExampleGame, which has the workaround patch applied:  https://github.com/uetopia/ExampleGame/tree/master/Plugins/SocketIOClient