hashtable_dir = ../hashtable/
holdall_dir = ../holdall/
option_dir = ../option/
CC = gcc
CFLAGS = -std=c2x \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
  -O2 \
  -I$(hashtable_dir) -I$(holdall_dir) -I$(option_dir) \
  -DHASHTABLE_STATS=0
vpath %.c $(hashtable_dir) $(holdall_dir) $(option_dir)
vpath %.h $(hashtable_dir) $(holdall_dir) $(option_dir)
objects = main.o hashtable.o holdall.o option.o
executable = xwc
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

main.o: main.c hashtable.h holdall.h
hashtable.o: hashtable.c hashtable.h
holdall.o: holdall.c holdall.h
option.o: $(option_dir)option.c $(option_dir)option.h

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)
