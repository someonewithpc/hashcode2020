# Seconds to run optimization pass by default
RUNTIME?=20

all: compile run

conpile:
	@g++ -ggdb -std=c++2a -O3 solve.cpp -ltbb -o solve

run:
	@echo "Running solve for ${RUNTIME}s"
	@IFS=' ' parallel -j 8 'cat inputs/{} | ./solve ${RUNTIME} > results/r{}; echo "r{} "' ::: {a..f}
