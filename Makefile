alcohol: alcohol.o
	c++ -o alcohol alcohol.o -ltrapfpe -lpgplot -lcpgplot -lX11 -lm 

euler.o: alcohol.cpp
	c++ -c alcohol.cpp





