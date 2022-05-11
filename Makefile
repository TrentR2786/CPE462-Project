% : %.cpp
	g++ $< `pkg-config --cflags --libs tesseract opencv4` -o app
