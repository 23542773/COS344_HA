#add all the files that you need to the files makefile variable below as a space seperated list
files = Skybox.cpp stb_image.cpp Camera.cpp Terrain.cpp ShapeFactory.cpp utils/ObjectLoader.cpp Mesh.cpp LightingSystem.cpp

main: main.cpp
	g++ -g -I . shader.cpp $(files) main.cpp -lglfw -pthread -lGLEW -ldl -lGL -o main

clean:
	rm -f *.o main

run:
	./main

all:
	make clean
	make
	make run