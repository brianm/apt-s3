
default: s3

clean:
	rm -f *.o s3

s3: s3_main.o s3.o connect.o jsonz.o object.o
	 g++ -g -o s3 s3.o s3_main.o connect.o jsonz.o object.o -lapt-pkg -lapt-inst -lssl -lcrypto -lcurl

%.o: %.cc
	 gcc  -I /usr/include -I./ -g -c $<
%.o: %.c
	 gcc  -I /usr/include -I./ -g -c $<

install: s3
	cp s3 /usr/lib/apt/methods/
