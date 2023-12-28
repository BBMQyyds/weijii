//
// Created by 33698 on 2023/12/17.
//

#ifndef WEIJI_DATA_H
#define WEIJI_DATA_H

#include <stddef.h>

// 数据结构定义
typedef struct {
    char *data;
    size_t size;
} Data;

// 函数声明
Data read_data(const char *filename);

int write_data(const char *filename, const Data *data);

#endif //WEIJI_DATA_H
