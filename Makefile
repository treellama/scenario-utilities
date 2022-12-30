all: fuxdiff resdiff

fuxdiff: fuxdiff.cpp
	g++ -o fuxdiff -std=c++11 fuxdiff.cpp

resdiff: resdiff.cpp macroman.cpp
	g++ -o resdiff -std=c++11 resdiff.cpp macroman.cpp

