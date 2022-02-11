PROG = MechV
OBJS = Tuples.o Math.o Base.o Img.o PatternNode.o HexBase.o HexEye.o ConvolHex.o Hex.o ColWheel.o ColLayer.o PatternLuna.o PatLunaLayer.o LineFinder.o CameraTrans.o DrivePlane.o DriveLines.o DrawHexImg.o main.o

$(PROG) : $(OBJS)
	g++ -pthread -lopencv_core -lopencv_videoio -lopencv_highgui -o $(PROG) $(OBJS)
main.o : Hex.h PatLunaLayer.h DriveLines.h DrawHexImg.h
	g++ -c main.cpp
DrawHexImg.o : DrawHexImg.h HexBase.h HexEye.h DrivePlane.h
	g++ -c DrawHexImg.cpp
DriveLines.o : DriveLines.h DrivePlane.h
	g++ -c DriveLines.cpp
DrivePlane.o : DrivePlane.h CameraTrans.h LineFinder.h
	g++ -c DrivePlane.cpp
CameraTrans.o : CameraTrans.h PatternNode.h Base.h
	g++ -c CameraTrans.cpp
LineFinder.o : LineFinder.h PatternLuna.h
	g++ -c LineFinder.cpp
PatLunaLayer.o : PatLunaLayer.h PatternLuna.h ColLayer.h
	g++ -c PatLunaLayer.cpp
PatternLuna.o : PatternLuna.h PatternNode.h Base.h
	g++ -c PatternLuna.cpp
ColLayer.o : ColLayer.h HexBase.h ColWheel.h
	g++ -c ColLayer.cpp
ColWheel.o : ColWheel.h Base.h
	g++ -c ColWheel.cpp
Hex.o : Hex.h ConvolHex.h
	g++ -c Hex.cpp
ConvolHex.o : ConvolHex.h HexBase.h
	g++ -c ConvolHex.cpp
HexEye.o : HexEye.h HexBase.h
	g++ -c HexEye.cpp
HexBase.o : HexBase.h Img.h
	g++ -c HexBase.cpp
PatternNode.o : PatternNode.h Math.h
	g++ -c PatternNode.cpp
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
