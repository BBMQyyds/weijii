//
// Created by 33698 on 2023/12/17.
//
#include "user.h"
#include <string.h>
#include <stdio.h>

// 解析用户数据
void parse_user_data(const char *input, User *user) {
    // 读取用户账号密码
    sscanf(input, "%s %s", user->username, user->password);

    // 读取用户拥有的数据文件
    user->num_data_files = 0;
    user->is_waiting = 0;

    // 分割用户数据文件
    char *token = strtok(input, " ");
    while (token != NULL) {
        if (user->num_data_files < MAX_DATA_FILES) {
            // 跳过账号和密码部分
            if (strlen(token) > 0 && strncmp(token, user->username, strlen(user->username)) != 0 &&
                strncmp(token, user->password, strlen(user->password)) != 0) {
                strcpy(user->data_files[user->num_data_files], token);
                user->num_data_files++;
            }
        }
        token = strtok(NULL, " ");
    }
}

// 从字符串中读取用户数据
int read_users_from_string(const char *input, User *users, int max_users) {
    int num_users = 0;

    // 保存 strtok_r 的上下文
    char *saveptr;

    // 分割用户数据
    char *token = strtok_r((char *) input, "\n", &saveptr);
    while (token != NULL && num_users < max_users) {
        // 跳过空行
        if (strlen(token) > 0) {
            parse_user_data(token, &users[num_users]);
            num_users++;
        }

        // 继续分割下一个用户数据
        token = strtok_r(NULL, "\n", &saveptr);
    }

    return num_users;
}

// 写入用户数据
void stringfy_user_data(char *output, User *users) {
    for (int i = 0; i < MAX_USERS; ++i) {
        if (users[i].username[0] == '\0') {
            break;
        }
        sprintf(output + strlen(output), "%s %s", users[i].username, users[i].password);
        for (int j = 0; j < users[i].num_data_files; ++j) {
            sprintf(output + strlen(output), " %s", users[i].data_files[j]);
        }
        sprintf(output + strlen(output), "\n");
    }
}

// 添加用户
int add_user(User *users, const char *username, const char *password) {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (users[i].username[0] == '\0') {
            strcpy(users[i].username, username);
            strcpy(users[i].password, password);
            users[i].num_data_files = 1;
            strcpy(users[i].data_files[0], "data.json");
            users[i].is_waiting = 0;
            return 1;
        }
    }
    return 0;
}

// 打印用户数据
void print_users(const User *users) {
    printf("\n\n");
    printf("users:\n");
    for (int i = 0; i < MAX_USERS; ++i) {
        if (users[i].username[0] == '\0') {
            break;
        }
        printf("username: %s\n", users[i].username);
        printf("password: %s\n", users[i].password);
        printf(" is_waiting: %d\n", users[i].is_waiting);
        printf(" num_data_files: %d\n", users[i].num_data_files);
        printf(" data_files: ");
        for (int j = 0; j < users[i].num_data_files; ++j) {
            printf("%s ", users[i].data_files[j]);
        }
        printf("\n\n");
    }
}

// 查找用户
int find_user(const User *users, const char *username) {
    for (int i = 0; i < MAX_USERS; ++i) {
        if (users[i].username[0] == '\0') {
            break;
        }
        if (strncmp(users[i].username, username, strlen(username)) == 0) {
            return i;
        }
    }
    return -1;
}

// 删除用户
int remove_user(User *users, const char *username) {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        if (users[i].username[0] == '\0') {
            break;
        }
        if (strncmp(users[i].username, username, strlen(username)) == 0) {
            users[i].username[0] = '\0';
            users[i].password[0] = '\0';
            users[i].num_data_files = 0;
            users[i].is_waiting = 0;
            return 1;
        }
    }
    return 0;
}

// 添加令牌
int add_token(Token *tokens, const char *key, int value) {
    int i;
    for (i = 0; i < MAX_TOKENS; i++) {
        //覆盖已有的令牌
        if (strncmp(tokens[i].key, key, strlen(key)) == 0) {
            tokens[i].value = value;
            return 1;
        }
        if (tokens[i].key[0] == '\0') {
            strcpy(tokens[i].key, key);
            tokens[i].value = value;
            return 1;
        }
    }
    return 0;
}

// 打印令牌
void print_tokens(const Token *tokens) {
    printf("\n\n");
    printf("tokens:\n");
    for (int i = 0; i < MAX_TOKENS; ++i) {
        if (tokens[i].key[0] == '\0') {
            break;
        }
        printf("key: %s\n", tokens[i].key);
        printf("value: %d\n", tokens[i].value);
        printf("\n");
    }
}

// 查找令牌
int find_token(const Token *tokens, const char *key) {
    for (int i = 0; i < MAX_TOKENS; ++i) {
        if (tokens[i].key[0] == '\0') {
            break;
        }
        // 将tokens[i].key长度减1后再比较
        if (strncmp(tokens[i].key, key, strlen(key)) == 0) {
            return i;
        }
    }
    return -1;
}

// 删除令牌
int remove_token(Token *tokens, const char *key) {
    int i;
    for (i = 0; i < MAX_TOKENS; i++) {
        if (tokens[i].key[0] == '\0') {
            break;
        }
        if (strncmp(tokens[i].key, key, strlen(key)) == 0) {
            tokens[i].key[0] = '\0';
            tokens[i].value = 0;
            return 1;
        }
    }
    return 0;
}

// 添加其他
int add_other(Other *others, const char *key, const char *value) {
    int i;
    for (i = 0; i < MAX_OTHERS; i++) {
        //覆盖已有的令牌
        if (strncmp(others[i].key, key, strlen(key)) == 0) {
            strcpy(others[i].value, value);
            return 1;
        }
        if (others[i].key[0] == '\0') {
            strcpy(others[i].key, key);
            strcpy(others[i].value, value);
            return 1;
        }
    }
    return 0;
}

// 打印其他
void print_others(const Other *others) {
    printf("\n\n");
    printf("others:\n");
    for (int i = 0; i < MAX_OTHERS; ++i) {
        if (others[i].key[0] == '\0') {
            break;
        }
        printf("key: %s\n", others[i].key);
        printf("value: %s\n", others[i].value);
        printf("\n");
    }
}

// 查找其他
int find_other(const Other *others, const char *key) {
    for (int i = 0; i < MAX_OTHERS; ++i) {
        if (others[i].key[0] == '\0') {
            break;
        }
        // 将tokens[i].key长度减1后再比较
        if (strncmp(others[i].key, key, strlen(key)) == 0) {
            return i;
        }
    }
    return -1;
}

// 删除其他
int remove_other(Other *others, const char *key) {
    int i;
    for (i = 0; i < MAX_OTHERS; i++) {
        if (others[i].key[0] == '\0') {
            break;
        }
        if (strncmp(others[i].key, key, strlen(key)) == 0) {
            others[i].key[0] = '\0';
            others[i].value[0] = '\0';
            return 1;
        }
    }
    return 0;
}

// 字符串化用户文件列表
void stringfy_user_files(char *output, const User *user) {
    for (int i = 0; i < user->num_data_files; ++i) {
        if (i == 0) {
            strcpy(output, user->data_files[i]);
            continue;
        }
        strcat(output, " ");
        strcat(output, user->data_files[i]);
    }
}

// 查找用户文件
int find_data_file(const User *user, const char *filename){
    for (int i = 0; i < user->num_data_files; ++i) {
        if (strncmp(user->data_files[i], filename, strlen(filename)) == 0) {
            return i;
        }
    }
    return -1;
}