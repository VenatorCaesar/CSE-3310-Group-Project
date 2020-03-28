SRC1 = src/client.cpp
SRC2 = src/InfoWindow.cpp
SRC3 = src/GameWindow.cpp
SRC4 = src/Player.cpp
SRC5 = src/chat_client.cpp
OBJ1 = $(SRC1:.cpp=.o)
OBJ2 = $(SRC2:.cpp=.o)
OBJ3 = $(SRC3:.cpp=.o)
OBJ4 = $(SRC4:.cpp=.o)
OBJ5 = $(SRC5:.cpp=.o)
EXE = $(SRC1:.cpp=.exe)

GTKFLAGS = `/usr/bin/pkg-config gtkmm-3.0 --cflags --libs`
CFLAGS = -g -std=c++11 -lpthread -Wall -O0 -DASIO_STANDALONE
CPPFLAGS = -I./include -I./asio-1.12.2/include

all : $(EXE) asio-1.12.2
	-rm -r $(OBJ5)
	-rm -r $(OBJ4)
	-rm -f $(OBJ3)
	-rm -f $(OBJ2)
	-rm -f $(OBJ1)

$(EXE): $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)
	g++ $(CFLAGS) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) -o $(EXE) $(GTKFLAGS)

$(OBJ5) : $(SRC5)
	g++ -c $(CFLAGS) $(SRC5) -o $(OBJ5) $(GTKFLAGS) $(CPPFLAGS)

$(OBJ4) : $(SRC4)
	g++ -c $(CFLAGS) $(SRC4) -o $(OBJ4) $(GTKFLAGS) $(CPPFLAGS)

$(OBJ3) : $(SRC3)
	g++ -c $(CFLAGS) $(SRC3) -o $(OBJ3) $(GTKFLAGS) $(CPPFLAGS)
	
$(OBJ2) : $(SRC2)
	g++ -c $(CFLAGS) $(SRC2) -o $(OBJ2) $(GTKFLAGS) $(CPPFLAGS)
	
$(OBJ1) : $(SRC1) asio-1.12.2
	g++ -c $(CFLAGS) $(SRC1) -o $(OBJ1) $(GTKFLAGS) $(CPPFLAGS)
	
asio-1.12.2:
	tar xzf asio-1.12.2.tar.gz
	
clean:
	-rm -r src/client.exe
	-rm -rf asio-1.12.2