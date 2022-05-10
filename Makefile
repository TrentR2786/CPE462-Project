% : %.cpp
	g++ $< `pkg-config --cflags --libs tesseract opencv` -o app