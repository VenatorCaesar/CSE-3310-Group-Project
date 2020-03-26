SRC1 = test.cpp
SRC2 = InfoWindow.cpp
SRC3 = GameWindow.cpp
SRC4 = Player.cpp
OBJ1 = $(SRC1:.cpp=.o)
OBJ2 = $(SRC2:.cpp=.o)
OBJ3 = $(SRC3:.cpp=.o)
OBJ4 = $(SRC4:.cpp=.o)
EXE = $(SRC1:.cpp=.exe)

GTKFLAGS = `/usr/bin/pkg-config gtkmm-3.0 --cflags --libs`
CFLAGS = -g -std=c++11 -pthread 

all : $(EXE)
 
$(EXE): $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
	g++ $(CFLAGS) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) -o $(EXE) $(GTKFLAGS)

$(OBJ4) : $(SRC4)
	g++ -c $(CFLAGS) $(SRC4) -o $(OBJ4) $(GTKFLAGS)

$(OBJ3) : $(SRC3)
	g++ -c $(CFLAGS) $(SRC3) -o $(OBJ3) $(GTKFLAGS)
	
$(OBJ2) : $(SRC2)
	g++ -c $(CFLAGS) $(SRC2) -o $(OBJ2) $(GTKFLAGS)
	
$(OBJ1) : $(SRC1)
	g++ -c $(CFLAGS) $(SRC1) -o $(OBJ1) $(GTKFLAGS)