/* 
 * Copyright (C) 2013  Inori Sakura <inorindesu@gmail.com>
 * 
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/*from lua manual 5.2*/
static void *allocator (void *ud, void *ptr, size_t osize,
                      size_t nsize)
{
  (void)ud;  (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}

#define BUFFER_SIZE (1024)

typedef struct load_t
{
  FILE* fp;
  char buffer[BUFFER_SIZE];
}load_t;

static const char* loader(lua_State* L, void* data, size_t* size)
{
  load_t* loadInfo = data;
  int read = fread(loadInfo->buffer, 1, BUFFER_SIZE, loadInfo->fp);
  if (read == 0)
    {
      *size = 0;
      return NULL;
    }
  else
    {
      *size = read;
      return loadInfo->buffer;
    }
}

void printStackTop(lua_State* s)
{
  fprintf(stdout, "current stack top is: %d\n", lua_gettop(s));
}

int main()
{
  lua_State* state = lua_newstate(allocator, NULL);
  luaL_requiref(state, "base", luaopen_base, 1);
  char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
  fprintf(stdout, "lua calculator started.\n");
  while(true)
    {
      printStackTop(state);
      fprintf(stdout, "Please select action:\n");
      fgets(buffer, BUFFER_SIZE - 1, stdin);
      int idx;
      for (idx = 0; buffer[idx] != '\n'; idx++);
      buffer[idx] = '\0';

      FILE* f = fopen(buffer, "r");
      if (f == NULL)
        {
          fprintf(stdout, "ERROR: lua script (%s) not found\n", buffer);
          continue;
        }
      load_t* loadVector = malloc(sizeof(load_t));
      loadVector->fp = f;

      int result;
      result = lua_load(state, loader, loadVector, buffer, "t");
      free(loadVector);
      fclose(f);
      if (result == LUA_ERRSYNTAX)
        {
          fprintf(stdout, "ERROR: syntax error in lua script\n");
          lua_pop(state, 1);
          continue;
        }
      
      /*
       * the script is assumed to have 2 objects:
       * 1. paramCount (int)
       * 2. a function named calc (func)
       */
      result = lua_pcall(state, 0, LUA_MULTRET, 0);
      if(result == LUA_OK)
        {
          lua_getglobal(state, "paramCount");
          if (lua_isnumber(state, -1) != 1)
            {
              fprintf(stdout, "ERROR: paramCount should be a positive integer\n");
              continue;
            }
          unsigned int paramCount = lua_tounsigned(state, -1);
          fprintf(stdout, "I need %u params.\n", paramCount);

          lua_pop(state, 1);
          lua_getglobal(state, "calc");
          if (lua_isfunction(state, -1) != 1)
            {
              fprintf(stdout, "ERROR: calc should be a function");
              continue;
            }

          int i;
          for( i = 0; i < paramCount; i++)
            {
              int param;
              fprintf(stdout, "Please give me param %d\n", i + 1);
              int scanned = fscanf(stdin, "%d", &param);
              for(;fgetc(stdin) != '\n';);
              if (scanned == 0)
                break;
              lua_pushinteger(state, param);
            }
          if (i != paramCount)
            {
              fprintf(stdout, "ERROR: parameter input not done\n");
              continue;
            }
          
          result = lua_pcall(state, paramCount, 1, 0);
          if (result != LUA_OK)
            {
              fprintf(stdout, "ERROR: when doing calculation (%d)\n", result);
              fprintf(stdout, "%s\n", lua_tostring(state, -1));
              lua_pop(state, 1);
              continue;
            }
          if (lua_isnumber(state, -1) != 1)
            {
              fprintf(stdout, "ERROR: calculation did not return a integer\n");
              continue;
            }
          fprintf(stdout, "Result: %d\n", lua_tointeger(state, -1));
          lua_pop(state, 1);
        }
      else if (result == LUA_ERRRUN)
        {
          fprintf(stdout, "ERROR: runtime error\n");
          fprintf(stdout, "%s\n", lua_tostring(state, -1));
          lua_pop(state, 1);
          continue;
        }
      else
        {
          fprintf(stdout, "ERROR: other error\n");
          fprintf(stdout, "%s\n", lua_tostring(state, -1));
          lua_pop(state, 1);
          continue;
        }
    }
  free(buffer);
  lua_close(state);
}
