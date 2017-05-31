# ofxKuBox2d
Kuflex addon for Box2D in openFrameworks intended for simulating physics with Kinect masks.

* It supports world of static edges and falling circles.
* The main feature - using binary mask (received from Kinect of web camera) to affect circles.
It is implemented using just forces from mask to circles, no triangulation!

Example of project used this method:

[![Video](http://img.youtube.com/vi/vHOui4aIokM/0.jpg)](http://www.youtube.com/watch?v=vHOui4aIokM)

## Example
Addon includes example which shows falling circles 
and a "human" on a binary mask which moves left and right 
and affects to the circles.

## Installation
Addon should link smoothly to your openFrameworks project using Project Generator.
For manual installation, add addon's libs and src folders to the includes settings of your IDE project,
as well as all .h and .cpp files from lib and src addon's folders.

It was tested for openFrameworks 0.8.4.

## Details
This project uses Box2D 2.1.