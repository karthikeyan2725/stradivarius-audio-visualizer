build:
	gcc -c ./src/* -I ./include     
	mkdir -p ./target                                            
	g++ *.o -o ./target/app -L ./lib -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl  
	rm *.o
clean:
	rm ./target
run:
	./target/app
