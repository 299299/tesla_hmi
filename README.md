# tesla like hmi android project

[![](https://img.youtube.com/vi/g6ae_AuJvyg/0.jpg)](https://www.youtube.com/watch?v=g6ae_AuJvyg)
[![](https://img.youtube.com/vi/rxTK5McUPA4/0.jpg)](https://www.youtube.com/watch?v=rxTK5McUPA4)
[![](https://img.youtube.com/vi/ZYY_OpV-Kjs/0.jpg)](https://www.youtube.com/watch?v=ZYY_OpV-Kjs)


the code is using Urho3D as engine, https://github.com/urho3d/Urho3D

hmi related native code is under ./Source/Tools/Game/

* [./Source/Tools/Game/Game.h](./Source/Tools/Game/Game.h)
* [./Source/Tools/Game/Game.cpp](./Source/Tools/Game/Game.cpp)

amap navigation integration code is under ./Android/src/com/github/urho3d/

* [./Android/src/com/github/urho3d/Urho3D.java](./Android/src/com/github/urho3d/Urho3D.java)

notice about testing:

make your C2/EON connect to your phone/pad hotspot, and set to a fixed ip, e.g. "192.168.43.138"

make sure your pad is set to a fixed ip address, either "192.168.43.138" or "192.168.43.1"

the ip of C2 try to connnect to  is list in :

https://github.com/299299/openpilot/blob/stock_additions_new/selfdrive/golden/phone_control.py#L99

latest apk release:

http://d.zqapps.com/qdy8


