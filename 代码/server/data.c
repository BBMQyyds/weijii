//
// Created by 33698 on 2023/12/17.
//

#include "data.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

Data read_data(const char *filename) {
    Data result = {NULL, 0};

    // 打开文件
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return result;
    }

    // 获取文件大小
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        close(fd);
        return result;
    }

    // 重新设置文件偏移为文件开头
    lseek(fd, 0, SEEK_SET);

    // 分配内存用于存储文件数据
    result.data = (char *) malloc(file_size);
    if (result.data == NULL) {
        perror("Error allocating memory");
        close(fd);
        return result;
    }

    // 读取文件数据
    ssize_t bytes_read = read(fd, result.data, file_size);
    if (bytes_read == -1) {
        perror("Error reading file");
        free(result.data);
        close(fd);
        return result;
    }

    // 设置数据大小
    result.size = (size_t) bytes_read;

    // 关闭文件
    close(fd);

    return result;
}

int write_data(const char *filename, const Data *data) {
    // 打开文件，如果不存在则创建
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    printf("fd = %d\n", fd);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    // 写入数据
    ssize_t bytes_written = write(fd, data->data, data->size);
    printf("bytes_written = %zd\n", bytes_written);
    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        return -1;
    }

    printf("write success!\n");

    // 关闭文件
    close(fd);

    return 0;
}
