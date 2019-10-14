objects =  u_boot_env_gen.o crc32.o
exec = u_boot_env_gen
$(exec):$(objects)
	gcc -o $(exec) $(objects) -static
	rm -rf $(objects)

%.o:%.c
	gcc -I../include -c $< -o $@

crc32.c:../common/crc32.c
	cp ../common/crc32.c crc32.c

clean:
	rm -rf $(exec) $(objects)
	rm -f crc32.c
