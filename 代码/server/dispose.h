//
// Created by 33698 on 2023/12/16.
//

#ifndef UNTITLED_DISPOSE_H
#define UNTITLED_DISPOSE_H

#include "user.h"

// 结构体声明
typedef struct {
    const char *status;
    const char *message;
} Response;

// 函数声明
Response dispose_login_request(const char *buffer, User *users, Token *tokens, Other *others);

Response dispose_wait_request(const char *buffer, User *users, Token *tokens, Other *others);

Response dispose_luozi_request(const char *buffer, User *users, Token *tokens, Other *others);

Response dispose_lose_request(const char *buffer, User *users, Token *tokens, Other *others);

Response dispose_qipu_request(const char *buffer, User *users, Token *tokens, Other *others);

#endif //UNTITLED_DISPOSE_H
