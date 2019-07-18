CC = g++-8
CFLAGS = -O2

default: main

main: main.o adaptive_maximization.o blits.o evaluation_oracle.o fantom.o random_greedy.o maximization_result.o utilities.o
	$(CC) $(CFLAGS) -o main main.o adaptive_maximization.o blits.o evaluation_oracle.o fantom.o random_greedy.o maximization_result.o utilities.o

adaptive_maximization.o: adaptive_maximization.h adaptive_maximization.cc evaluation_oracle.h maximization_result.h utilities.h
	$(CC) $(CFLAGS) -c adaptive_maximization.cc

blits.o: blits.h blits.cc evaluation_oracle.h maximization_result.h utilities.h
	$(CC) $(CFLAGS) -c blits.cc

evaluation_oracle.o: evaluation_oracle.h evaluation_oracle.cc
	$(CC) $(CFLAGS) -c evaluation_oracle.cc

fantom.o: fantom.h fantom.cc evaluation_oracle.h adaptive_maximization.h maximization_result.h utilities.h
	$(CC) $(CFLAGS) -c fantom.cc

random_greedy.o: random_greedy.h random_greedy.cc evaluation_oracle.h maximization_result.h utilities.h
	$(CC) $(CFLAGS) -c random_greedy.cc

main.o: main.cc evaluation_oracle.h random_greedy.h maximization_result.h utilities.h
	$(CC) $(CFLAGS) -c main.cc

maximization_result.o: maximization_result.h maximization_result.cc
	$(CC) $(CFLAGS) -c maximization_result.cc

utilities.o: utilities.h utilities.cc
	$(CC) $(CFLAGS) -c utilities.cc

clean:
	$(RM) main *.o
