CC = gcc
CFLAGS = -g -Wall
TARGET = atp-emmc-getHealthRpt

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET)

