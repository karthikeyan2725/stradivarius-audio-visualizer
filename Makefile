build:
	gcc -c ./src/* -I ./include -I ./headers 
	mkdir -p ./target                                            
	g++ *.o -o ./target/app -L ./lib -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lfftw3 -lassimp -lzlibstatic -lm
	rm *.o
debug:
	gcc -g -c ./src/* -I ./include -I ./headers 
	mkdir -p ./target                                            
	g++ -g *.o -o ./target/app -L ./lib -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lfftw3 -lassimp -lzlibstatic -lm
	rm *.o
	gdb --args ./target/app ./audio/escape.wav ./models/well_tunnel/scene.gltf
clean:
	rm ./target
run:
	./target/app ./audio/escape.wav
