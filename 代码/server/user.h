//
// Created by 33698 on 2023/12/17.
//

#ifndef WEIJI_USER_H
#define WEIJI_USER_H

#include <stdio.h>

#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 50
#define MAX_DATA_FILES 10
#define MAX_DATA_FILENAME_LENGTH 50
#define MAX_USERS 1000
#define MAX_TOKENS 5000
#define MAX_OTHERS 5000
#define TOKEN_LENGTH 20
#define OTHER_LENGTH 50

// 用户结构体定义
typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char data_files[MAX_DATA_FILES][MAX_DATA_FILENAME_LENGTH];
    int is_waiting;
    int num_data_files;
} User;

// 定义键值对结构体
typedef struct {
    char key[TOKEN_LENGTH];
    int value;
} Token;

// 定义键值对结构体
typedef struct {
    char key[TOKEN_LENGTH];
    char value[OTHER_LENGTH];
} Other;

// 函数声明
void parse_user_data(const char *input, User *user);

int read_users_from_string(const char *input, User *users, int max_users);

void stringfy_user_data(char *output, User *users);

int add_user(User *users, const char *username, const char *password);

void print_users(const User *users);

int find_user(const User *users, const char *username);

int remove_user(User *users, const char *username);

int add_token(Token *tokens, const char *key, int value);

void print_tokens(const Token *tokens);

int find_token(const Token *tokens, const char *key);

int remove_token(Token *tokens, const char *key);

int add_other(Other *others, const char *key, const char *value);

void print_others(const Other *others);

int find_other(const Other *others, const char *key);

int remove_other(Other *others, const char *key);

void stringfy_user_files(char *output, const User *user);

int find_data_file(const User *user, const char *filename);

#endif //WEIJI_USER_H
