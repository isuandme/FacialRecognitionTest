default : 
	g++ main.cpp -o main.o -lopencv_core -lopencv_imgcodecs

clean :
	 -rm main.o