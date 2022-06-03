
INCLUDES :=  \
 -I /usr/local/include/sofia-sip-1.12 \
 -I /usr/include/sofia-sip

DEFINES  =  -D _ISSI -D SU_DEBUG=9

LIBRARY_FILES := \
 ssc_log.c \
 ssc_oper.c \
 ssc_oper_container.c \
 ssc_sip.c \
 ssc_types.c

CC           = gcc
CFLAGS       = -std=gnu99 -fPIC -MD -Wall  -O2 $(INCLUDES) $(DEFINES)
LDFLAGS      = -shared

OBJECTS = $(LIBRARY_FILES:.c=.o)

TARGET  = libssc.so

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

install: $(TARGET)
	cp $(TARGET) /usr/local/lib
	ldconfig -n /usr/local/lib

