CC = gcc
BASE_DIR = ..

OBJECTS = $(BASE_DIR)/objects
HEADERS = $(BASE_DIR)/headers
SOURCES = $(BASE_DIR)/sources
BIN = $(BASE_DIR)/bin

CFLAGS = -Wall -g -c
FLAGS = -o
GCOV_FLAGS = -ftest-coverage -fprofile-arcs
VAL_FLAGS = --leak-check=full

GLIB_HEADERS = `pkg-config --cflags glib-2.0`
GLIB_LIBS = `pkg-config --libs glib-2.0`

all: $(BIN)/http_server

$(BIN)/http_server: $(OBJECTS)/helper_functions.o $(OBJECTS)/log.o $(OBJECTS)/init.o \
	$(OBJECTS)/http_status_codes.o $(OBJECTS)/http_helper_functions.o $(OBJECTS)/http_parser.o \
	$(OBJECTS)/http_POST_request_handler.o $(OBJECTS)/http_OPTIONS_request_handler.o \
	$(OBJECTS)/http_PUT_request_handler.o $(OBJECTS)/http_DELETE_request_handler.o \
	$(OBJECTS)/http_GET_HEAD_request_handler.o $(OBJECTS)/http_request_handler.o \
	$(OBJECTS)/http_server.o $(OBJECTS)/http_server_helper.o
	$(CC) $(OBJECTS)/*.o $(FLAGS) $(BIN)/http_server $(GLIB_LIBS)


$(OBJECTS)/helper_functions.o: $(SOURCES)/helper_functions.c $(HEADERS)/helper_functions.h
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/helper_functions.c $(FLAGS) $(OBJECTS)/helper_functions.o

$(OBJECTS)/log.o: $(SOURCES)/log.c $(SOURCES)/helper_functions.c $(HEADERS)/log.h
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/log.c $(FLAGS) $(OBJECTS)/log.o

$(OBJECTS)/init.o: $(SOURCES)/init.c $(SOURCES)/log.c $(SOURCES)/helper_functions.c $(HEADERS)/init.h
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/init.c $(FLAGS) $(OBJECTS)/init.o

$(OBJECTS)/http_status_codes.o: $(SOURCES)/http_status_codes.c $(HEADERS)/http_status_codes.h $(SOURCES)/helper_functions.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_status_codes.c $(FLAGS) $(OBJECTS)/http_status_codes.o

$(OBJECTS)/http_helper_functions.o: $(SOURCES)/http_helper_functions.c \
	$(HEADERS)/http_helper_functions.h $(SOURCES)/http_status_codes.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_helper_functions.c $(FLAGS) $(OBJECTS)/http_helper_functions.o

$(OBJECTS)/http_parser.o: $(SOURCES)/http_parser.c $(HEADERS)/http_parser.h \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_parser.c $(FLAGS) $(OBJECTS)/http_parser.o

$(OBJECTS)/http_POST_request_handler.o: $(SOURCES)/http_POST_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_POST_request_handler.c $(FLAGS) $(OBJECTS)/http_POST_request_handler.o

$(OBJECTS)/http_OPTIONS_request_handler.o: $(SOURCES)/http_OPTIONS_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_OPTIONS_request_handler.c $(FLAGS) $(OBJECTS)/http_OPTIONS_request_handler.o

$(OBJECTS)/http_PUT_request_handler.o: $(SOURCES)/http_PUT_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_PUT_request_handler.c $(FLAGS) $(OBJECTS)/http_PUT_request_handler.o

$(OBJECTS)/http_DELETE_request_handler.o: $(SOURCES)/http_DELETE_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_DELETE_request_handler.c $(FLAGS) $(OBJECTS)/http_DELETE_request_handler.o

$(OBJECTS)/http_GET_HEAD_request_handler.o: $(SOURCES)/http_GET_HEAD_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_GET_HEAD_request_handler.c $(FLAGS) $(OBJECTS)/http_GET_HEAD_request_handler.o

$(OBJECTS)/http_request_handler.o: $(SOURCES)/http_request_handler.c \
	$(SOURCES)/http_status_codes.c $(SOURCES)/http_helper_functions.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_request_handler.c $(FLAGS) $(OBJECTS)/http_request_handler.o

$(OBJECTS)/http_server_helper.o: $(SOURCES)/http_server_helper.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_server_helper.c $(FLAGS) $(OBJECTS)/http_server_helper.o

$(OBJECTS)/http_server.o: $(SOURCES)/http_server.c $(SOURCES)/log.c
	$(CC) $(CFLAGS) $(GLIB_HEADERS) -I$(HEADERS) $(SOURCES)/http_server.c $(FLAGS) $(OBJECTS)/http_server.o

gcov_compile:
	cd ../gcov && $(CC) $(GCOV_FLAGS) -DGCOV_RUNNING -g -I$(HEADERS) $(SOURCES)/*.c `pkg-config --cflags --libs glib-2.0` -o $(BIN)/http_server

gcov_run:
	$(BIN)/http_server 4000

gcov_report:
	cd ../gcov && gcov $(SOURCES)/*.c

valgrind:
	valgrind $(VAL_FLAGS) $(BIN)/http_server 4000
