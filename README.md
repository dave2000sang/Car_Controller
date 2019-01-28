# Car_Controller
Remotely control a robot car using TCP/IP from my smartphone controller sending instructions to the raspberry pi 3 on the car. Used Java for Client code (Android app).

Android Studio GUI:
    Swipe on the touch-sensitive screen will draw a straight line, indicating the direction (angle to turn) and speed, depending on how fast the swipe was. For example, the speed will be set based on a scale of 1 - 10, and this speed is transmitted along with the turning angle to the Raspberry Pi computer through TCP/IP, and the car will move accordingly.


![](https://github.com/dave2000sang/Car_Controller/blob/master/gif/tcp_control.gif)
