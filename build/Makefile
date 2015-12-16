# MAKE FILE

RM:= rm
CC:= gcc
CFLAGS := -Wall -g

OBJETOS = main.o

all : pi_Search

pi_Search : $(OBJETOS)
	$(CC) $(CFLAGS) $(OBJETOS) -o $@
	@echo 'Creación exitosa'


# Reglas Generales

%.o : ../src/%.c
	$(CC) -c $(DEFS) $(CFLAGS) $< -o $@

.PHONY: all clean

clean : 
	$(RM) Server_Con1 $(OBJETOS)

