//
// Created by Fx Kaze on 25-3-26.
//

#ifndef PRINT_SIZE_H
#define PRINT_SIZE_H

#include <cstdio>
#include <cstring>

inline void print_size(const char *type_name, size_t size, size_t align) {
  char tmp[1024];
  static int ct = 0;
  ct++;
  if (ct == 1)
    printf("#pragma once\n#include \"types.h\"\n");
  size_t start = strlen(type_name);
  while (true) {
    if (type_name[start - 1] == ':')
      break;
    start--;
    if (start == 0)
      break;
  }
  size_t end = start;
  while (true) {
    if (type_name[end] == '<' || type_name[end] == '\0')
      break;
    end++;
  }
  memcpy(tmp, type_name + start, end - start);
  tmp[end - start] = '\0';
  printf("\ntypedef struct %s_private_s {\n", tmp);
  switch (align) {
    case 1:
      printf("\tu8 ");
      break;
    case 2:
      printf("\tu16 ");
      break;
    case 4:
      printf("\tu32 ");
      break;
    case 8:
      printf("\tu64 ");
      break;
  }
  printf("_data[%lu];\n", size / align);
  printf("} %s_private;\n", tmp);
}

#define PRT_SIZE(type) print_size(#type, sizeof(type), alignof(type))

#endif //PRINT_SIZE_H
