# Interactive Wall Version 2

### Requirements
- Windows 8/8.1
- A USB 3.0 port
- A Kinect V2
- Kinect V2 SDK: http://www.microsoft.com/en-us/download/details.aspx?id=44561
- Visual Studio 2013 (full or express verison)

### To set up and compile project

1. Downlaod this OpenFrameworks Fork: https://github.com/liquidzym/openFrameworks/tree/VS2013
2. Download this Kinect V2 addon: https://github.com/elliotwoods/ofxKinectForWindows2
3. Extract OpenFrameworks to a known place.
4. Extract ofxKinectForWindows2 to **(OpenFrameworks root)/addons** & rename to just "ofxKinectForWindows2"
5. Copy this project into **(OpenFramworks root)/apps/myApps/**  
(make sure you create the "myApps" folder, if it's not there)
6. go to the **(OpenFrameworks root)\addons\ofxKinectForWindows2\ofxKinectForWindows2Lib** folder, edit "ofxKinectForWindows2Lib.vcxproj" and modify lines 21, 27 to "<PlatformToolset>v120</PlatformToolset>" (it is v110, which will only compile with Visual Studio 2012)

7. Compile, you will get an error in the "Proceses.h" file, comment out the three lines causing the error.

8. Compile & enjoy!
