CC=g++
COMPILING_FLAGS=-c
BUILD_FLAGS=-lws2_32

EXECUTABLES = ./tests/exceptions_test.exe ./tests/server_socket_test.exe
OBJECTS = ./networking/exceptions.o ./networking/server_socket.o \
	./tests/exceptions_test.o ./tests/server_socket_test.o



tests: ./tests/exceptions_test.exe ./tests/server_socket_test.exe

./tests/exceptions_test.exe: ./networking/exceptions.o ./tests/exceptions_test.o
	$(CC) $^ $(BUILD_FLAGS) -o $@

./tests/server_socket_test.exe: ./networking/exceptions.o ./networking/server_socket.o ./tests/server_socket_test.o
	$(CC) $^ $(BUILD_FLAGS) -o $@

%.o: %.cpp
	$(CC) $(COMPILING_FLAGS) $< -o $@



clean:
ifeq ($(OS),Windows_NT)
	del /Q $(subst /,\,$(EXECUTABLES)) $(subst /,\,$(OBJECTS)) 2>nul
else
	rm -f $(EXECUTABLES) $(OBJECTS)
endif