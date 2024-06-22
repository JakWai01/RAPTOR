run:
	g++ -O3 main.cpp -o main
	./main -G ../fv/ -Q ./queries/de_fv.queries

run-full:
	g++ -O3 main.cpp -o main
	./main -G ../Gesamt/ -Q ./queries/de_full.queries
