gcc -c ./src/* -I ./include -I ./headers
mkdir -p ./target  
g++ *.o -o ./target/app -L ./lib -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lfftw3 -lassimp -lzlibstatic -lm
rm *.o ;
