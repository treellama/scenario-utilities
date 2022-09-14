all: fuxdiff strdiff

fuxdiff: fuxdiff.cpp
	g++ -o fuxdiff -std=c++11 fuxdiff.cpp

strdiff: strdiff.cpp macroman.cpp
	g++ -o strdiff -std=c++11 strdiff.cpp macroman.cpp

