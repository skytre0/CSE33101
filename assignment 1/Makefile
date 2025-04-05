SANFLAGS = -fsanitize=address
CXXFLAGS = -Wall -O2 -std=c++17 -g $(SANFLAGS)
LDFLAGS = $(SANFLAGS)

all: eval merge_sort heap_sort bubble_sort insertion_sort selection_sort quick_sort tournament_sort comb_sort cocktail_shaker_sort introsort library_sort tim_sort

eval:	eval.o
	g++ -o $@ $^ $(LDFLAGS)
merge_sort:	merge_sort.o
	g++ -o $@ $^ $(LDFLAGS)
heap_sort:	heap_sort.o
	g++ -o $@ $^ $(LDFLAGS)
bubble_sort:	bubble_sort.o
	g++ -o $@ $^ $(LDFLAGS)
insertion_sort:	insertion_sort.o
	g++ -o $@ $^ $(LDFLAGS)
selection_sort:	selection_sort.o
	g++ -o $@ $^ $(LDFLAGS)
quick_sort:	quick_sort.o
	g++ -o $@ $^ $(LDFLAGS)

tournament_sort:	tournament_sort.o
	g++ -o $@ $^ $(LDFLAGS)
comb_sort:	comb_sort.o
	g++ -o $@ $^ $(LDFLAGS)
cocktail_shaker_sort:	cocktail_shaker_sort.o
	g++ -o $@ $^ $(LDFLAGS)
introsort:	introsort.o
	g++ -o $@ $^ $(LDFLAGS)
library_sort:	library_sort.o
	g++ -o $@ $^ $(LDFLAGS)
tim_sort:	tim_sort.o
	g++ -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f merge_sort heap_sort bubble_sort insertion_sort selection_sort quick_sort *.o