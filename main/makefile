hashtable_dir = ../hashtable/
holdall_dir = ../holdall/
line_dir = ../line/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings\
  -O2 \
  -DHOLDALL_PUT_TAIL	\
  -I$(holdall_dir) -I$(hashtable_dir) -I$(line_dir)
vpath %.c $(holdall_dir) $(hashtable_dir) $(line_dir)
vpath %.h $(holdall_dir) $(hashtable_dir) $(line_dir)
objects = hashtable.o holdall.o main.o line.o
executable = lnid
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

holdall.o: holdall.c holdall.h
main.o: main.c hashtable.h holdall.h line.h
hashtable.o: hashtable.c hashtable.h
line.o: line.c line.h

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)
