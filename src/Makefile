CC = gcc
CFLAGS = -I. -Wall -O3
LDFLAGS =
LIBS = -lm

all: muTests muXFigDriver.o muXForms.o mappings.o clipping.o

clean:
	rm -f *~ *.o

muTests: muTests.o muPlot.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o "$@" $(LIBS)

muTests.o: muTests.c muPlot.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"

muPlot.o: muPlot.c muPlot.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"

clipping.o: clipping.c muPlot.h clipping.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"

mappings.o: mappings.c muPlot.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"

muXForms.o: muXForms.c muPlot.h muPlotXForms.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"

muXFigDriver.o: muXFigDriver.c muPlot.h
	$(CC) $(CFLAGS) -c "$<" -o "$@"
