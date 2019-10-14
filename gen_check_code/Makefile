objects =  gen_check_code.o
gen_check_code:$(objects)
	gcc -o gen_check_code $(objects) -static
	rm -rf $(objects)


clean:
	rm -rf gen_check_code $(objects)
