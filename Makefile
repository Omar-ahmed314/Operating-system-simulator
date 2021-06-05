build:
	# -Wall -Wextra -Wpedantic
	gcc process_generator.c -o process_generator.out 
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out 
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out
	#rm -f processes.txt

all: clean build

run:
	./process_generator.out processes.txt -sch 4
