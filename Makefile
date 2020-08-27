# Projekt: Osy Projekt
# Autor:   Jan Kusák|Kus0054
# Datum:   14.05.2020

CC = g++
CFLAGS = -lrt
APP = app

$(APP): $(APP).cpp
	$(CC) $(APP).cpp $(CFLAGS) -o $(APP)

clean: 
	rm -f $(APP)