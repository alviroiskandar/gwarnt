# SPDX-License-Identifier: GPL-2.0-only

CC = gcc
CXX = g++
INCLFLAGS = -I./src
CFLAGS = -Wall -Wextra -std=c99 -Os $(INCLFLAGS)
CXXFLAGS = -Wall -Wextra -std=c++11 -Os $(INCLFLAGS)
LDFLAGS = -Os
LDLIBS = -lm -lpthread -lcurl
DEPFLAGS = -MT "$@" -MMD -MP -MF $(@:%.o=%.d)

C_SOURCES	:=
CXX_SOURCES	:= \
	src/gwarnt/entry.cpp \
	src/gwarnt/helpers.cpp \
	src/gwarnt/net.cpp \
	src/gwarnt/p2p_ad.cpp \
	src/gwarnt/p2p/binance.cpp

C_OBJECTS	:= $(C_SOURCES:.c=.o)
CXX_OBJECTS	:= $(CXX_SOURCES:.cpp=.o)
DEPFILES	:= $(C_OBJECTS:.o=.d) $(CXX_OBJECTS:.o=.d)

TARGET = gwarnt

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CXX_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

clean:
	rm -f $(C_OBJECTS) $(CXX_OBJECTS) $(TARGET) $(DEPFILES)

.PHONY: all clean
