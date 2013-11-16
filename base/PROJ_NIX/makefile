LFLAGS := -lglut -lGL -lGLEW 
CFLAGS := -pg -Wall
IFLAGS := -I../../shared32/glm/ -I../../shared32/freeglut -I../../shared32/glew -I../src/SOIL -I../src/tiny_obj_loader

565DefferedShader: Utility.o tiny_obj_loader.o main.o image_helper.o stb_image_aug.o image_DXT.o SOIL.o
	g++ -pg *.o -o 565DefferedShader $(LFLAGS)

tiny_obj_loader.o: ../src/tiny_obj_loader/tiny_obj_loader.cc ../src/tiny_obj_loader/tiny_obj_loader.h
	g++ -pg ../src/tiny_obj_loader/tiny_obj_loader.cc -c

SOIL.o: ../src/SOIL/SOIL.c ../src/SOIL/*.h
	gcc -pg ../src/SOIL/SOIL.c -c

image_DXT.o: ../src/SOIL/image_DXT.c ../src/SOIL/*.h
	gcc -pg ../src/SOIL/image_DXT.c -c

stb_image_aug.o: ../src/SOIL/stb_image_aug.c ../src/SOIL/*.h
	gcc -pg ../src/SOIL/stb_image_aug.c -c

image_helper.o: ../src/SOIL/image_helper.c  ../src/SOIL/*.h
	gcc -pg ../src/SOIL/image_helper.c -c

main.o: ../src/main.cpp ../src/main.h ../src/Utility.h
	g++ -pg ../src/main.cpp $(IFLAGS) -c 

Utility.o: ../src/Utility.cpp ../src/Utility.h
	g++ -pg ../src/Utility.cpp $(IFLAGS) -c

clean:
	rm *.o 565DefferedShader
