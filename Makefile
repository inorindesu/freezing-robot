LUADIR=${HOME}/applications/lua-5.2.1

out: main.c builddir
	gcc -std=c99 -L${LUADIR}/lib -I${LUADIR}/include main.c -llua -lm -o ./build/out

builddir: ./build/t

./build/t:
	touch ./build/t
