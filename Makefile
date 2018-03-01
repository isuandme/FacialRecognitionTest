default : 
	g++ main.cpp -o main.o -lopencv_core -lopencv_imgcodecs -lopencv_face -lopencv_videoio -lopencv_objdetect -lopencv_imgproc -lopencv_highgui

clean :
	 -rm main.o