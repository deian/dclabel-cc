CC=clang++
TARGET=dclabel
$(TARGET).o: $(TARGET).cc $(TARGET).h
	$(CC) -c -o $(TARGET).o -Wall $(TARGET).cc
clean:
	-rm $(TARGET).o
