CC = gcc
BASE_DIR = ..

HEADERS = $(BASE_DIR)/headers
SOURCES = $(BASE_DIR)/sources
OBJECTS = $(BASE_DIR)/objects

BIN = $(BASE_DIR)/bin

CFLAGS = -Wall -g -c
FLAGS = -o

GLIB_HEADERS = `pkg-config --cflags glib-2.0`
GLIB_LIBS = `pkg-config --libs glib-2.0`

$(BIN)/test_http_server: $(SOURCES)/test_http_server.c
	$(CC) -Wall -I$$HOME/local/include -I../../headers $(OBJECTS)/*.o $(GLIB_HEADERS) $(SOURCES)/test_http_server.c -L$$HOME/local/lib -lcunit $(FLAGS) $(BIN)/test_http_server $(GLIB_LIBS)

