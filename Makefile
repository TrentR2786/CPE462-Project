INSTALLS = -I"C:\opencv\install\include" -L"C:\opencv\install\x64\mingw\lib" -L"C:\opencv\install\x64\mingw\bin"
LIBS = -lopencv_core455 -lopencv_highgui455 -lopencv_imgproc455 -lopencv_imgcodecs455 -lopencv_video455 -lopencv_videoio455

% : %.cpp
	g++ $(INSTALLS) $< $(LIBS) -o app