PROG = MechV
OBJS = Tuples.o Math.o Img.o Base.o HexBase.o ConvolHex.o Hex.o DrawHexImg.o main.o

$(PROG) : $(OBJS)
	g++ -pthread -lopencv_core -lopencv_videoio -lopencv_highgui -o $(PROG) $(OBJS)
main.o : Hex.h DrawHexImg.h
	g++ -c main.cpp
DrawHexImg.o : DrawHexImg.h HexBase.h
	g++ -c DrawHexImg.cpp
Hex.o : Hex.h ConvolHex.h
	g++ -c Hex.cpp
ConvolHex.o : ConvolHex.h HexBase.h
	g++ -c ConvolHex.cpp
HexBase.o : HexBase.h Img.h
	g++ -c HexBase.cpp
Img.o : Img.h Base.h Math.h
	g++ -c Img.cpp
Base.o : Base.h Math.h
	g++ -c Base.cpp
Math.o : Math.h Tuples.h
	g++ -c Math.cpp
Tuples.o : Tuples.h
	g++ -c Tuples.cpp
clean :
	rm -f core $(PROG) $(OBJS)
