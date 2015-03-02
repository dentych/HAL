sources = formatter.c text_formatter.c
objects = $(sources:.c=.o)
target = formatter
cxx = g++

.PHONY: all
all: $(target)
$(target): $(objects)
	$(cxx) -o $(target) $(objects)

%.o: %.cpp
	$(cxx) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(objects) $(target)
