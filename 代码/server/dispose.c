//
// Created by 33698 on 2023/12/16.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dispose.h"
#include "data.h"

void extract_username_password(const char *buffer, char *username, char *password);

void extract_token_type(const char *buffer, char *token, char *type);

void extract_token_type_other(const char *buffer, char *token, char *type, char *other);

size_t generate_random_string(char *str, size_t length);

// 字符串拼接函数
size_t concat_strings(char *destination, const char *source, size_t current_length, size_t destination_size);

char *remove_newline(char *str);

// 处理登录请求
Response dispose_login_request(const char *buffer, User *users, Token *tokens, Other *others) {
    printf("login request\n");
    printf("buffer:\n %s\n", buffer);

    // 读取buffer中的用户名和密码
    char username[20];
    char password[20];
    extract_username_password(buffer, username, password);
    printf("username: %s\n", username);
    printf("password: %s\n", password);

    // 检查用户名和密码是否正确
    int i;
    int correct = 0;
    int new_user = 1;
    for (i = 0; i < MAX_USERS; i++) {
        if (users[i].username[0] == '\0') {
            break;
        }
        if (strncmp(users[i].username, username, strlen(username)) == 0) {
            new_user = 0;
        }
        if (strncmp(users[i].username, username, strlen(username)) == 0 &&
            strncmp(users[i].password, password, strlen(password)) == 0) {
            correct = 1;
            break;
        }
    }
    printf("correct: %d\n", correct);

    Response response;
    if (correct) {
        // 生成token
        char token[20];
        generate_random_string(token, 20);
        printf("token: %s\n", token);

        // 将token和用户名保存到tokens中
        int add_t = add_token(tokens, token, i);
        if (add_t == 0) {
            response.status = "500 Internal Server Error";
            response.message = "Internal Server Error";
            return response;
        }
        printf("index: %d\n", i);

        // 将token写入response
        response.status = "200 OK";
        char message[100];
        strcpy(message, "login success: ");
        // 调用字符串拼接函数
        concat_strings(message, token, strlen(message), sizeof(message));
        response.message = message;
    } else {
        if (new_user) {
            int add_u = add_user(users, username, password);
            if (add_u == 1) {
                // 生成token
                char token[20];
                generate_random_string(token, 20);
                printf("token: %s\n", token);

                // 将token和用户名保存到tokens中
                int add_t2 = add_token(tokens, token, i);
                if (add_t2 == 0) {
                    response.status = "500 Internal Server Error";
                    response.message = "Internal Server Error";
                    return response;
                }
                printf("index: %d\n", i);

                // 将token写入response
                response.status = "200 OK";
                char message[100];
                strcpy(message, "register success: ");
                // 调用字符串拼接函数
                concat_strings(message, token, strlen(message), sizeof(message));
                response.message = message;
            } else {
                response.status = "500 Internal Server Error";
                response.message = "Internal Server Error";
            }
        } else {
            response.status = "401 Unauthorized";
            response.message = "Password incorrect";
        }
    }
    printf("\n\n");
    return response;
}

// 处理等待请求
Response dispose_wait_request(const char *buffer, User *users, Token *tokens, Other *others) {
    printf("wait request\n");
    printf("buffer:\n %s\n", buffer);

    // 读取buffer中的token和type
    char token[20];
    char type[20];
    extract_token_type(buffer, token, type);
    printf("token: %s\n", token);
    printf("type: %s\n", type);

    // 检查token是否正确
    int index = find_token(tokens, token);
    if (index == -1) {
        Response response;
        response.status = "401 Unauthorized";
        response.message = "Invalid token";
        return response;
    }

    printf("index: %d\n", index);
    const char *username = users[tokens[index].value].username;
    printf("username: %s\n", username);

    printf("\n\n");

    // 检查type是否正确（为common,move,noji,dark）
    if (strncmp(type, "common", 6) != 0 && strncmp(type, "move", 4) != 0 &&
        strncmp(type, "noji", 4) != 0 && strncmp(type, "dark", 4) != 0) {
        Response response;
        response.status = "400 Bad Request";
        response.message = "Invalid type";
        return response;
    }

    // 检查自己是否已进入对局
    int find_t = find_token(tokens, username);
    if (find_t != -1) {
        Response response;
        response.status = "200 OK";
        char message[100];
        response.message = strcat(strcpy(message, "wait success: "), users[tokens[find_t].value].username);
        return response;
    }

    int type_waiting = strncmp(type, "common", 6) == 0 ? 1 :
                       strncmp(type, "move", 4) == 0 ? 2 : strncmp(type, "noji", 4) == 0 ? 3 : 4;

    // 检查是否有其他用户正在等待
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].username[0] == '\0') {
            break;
        }
        //除去自己
        if (strncmp(users[i].username, username, strlen(username)) == 0) {
            continue;
        }
        if (users[i].is_waiting == type_waiting) {
            // 将自己-对手和对手-自己设置为对战状态，即写入到tokens中
            int add_t1 = add_token(tokens, username, i);
            int add_t2 = add_token(tokens, users[i].username, tokens[index].value);
            int add_o1 = add_other(others, username, users[i].username);
            int add_o2 = add_other(others, users[i].username, username);
            if (add_t1 == 0 || add_t2 == 0 || add_o1 == 0 || add_o2 == 0) {
                remove_token(tokens, username);
                remove_token(tokens, users[i].username);
                remove_other(others, username);
                remove_other(others, users[i].username);
                Response response;
                response.status = "500 Internal Server Error";
                response.message = "Internal Server Error";
                return response;
            }
            users[i].is_waiting = 0;
            users[tokens[index].value].is_waiting = 0;
            Response response;
            response.status = "200 OK";
            char message[100];
            response.message = strcat(strcpy(message, "Wait success: "), users[i].username);
            return response;
        }
    }

    // 将用户设置为等待状态
    users[tokens[index].value].is_waiting = type_waiting;
    Response response;
    response.status = "200 OK";
    response.message = "waiting...";
    return response;
}

// 处理落子请求
Response dispose_luozi_request(const char *buffer, User *users, Token *tokens, Other *others) {

    printf("luozi request\n");
    printf("buffer:\n %s\n", buffer);

    // 读取buffer中的token和type
    char token[20];
    char type[20];
    char luozi[20];
    extract_token_type_other(buffer, token, type, luozi);
    printf("token: %s\n", token);
    printf("type: %s\n", type);
    printf("luozi: %s\n", luozi);

    // 检查token是否正确
    int index = find_token(tokens, token);
    if (index == -1) {
        Response response;
        response.status = "401 Unauthorized";
        response.message = "Invalid token";
        return response;
    }

    printf("index: %d\n", index);
    const char *username = users[tokens[index].value].username;
    printf("username: %s\n", username);

    printf("\n\n");

    // 检查type是否正确（为common,move,noji,dark）
    if (strncmp(type, "common", 6) != 0 && strncmp(type, "move", 4) != 0 &&
        strncmp(type, "noji", 4) != 0 && strncmp(type, "dark", 4) != 0) {
        Response response;
        response.status = "400 Bad Request";
        response.message = "Invalid type";
        return response;
    }

    if (strncmp(luozi, "wait", 4) == 0) {

        int find_t = find_token(tokens, username);
        if (find_t == -1) {
            Response response;
            response.status = "200 OK";
            response.message = "win the game";
            return response;
        }

        int find_o = find_other(others, users[tokens[find_t].value].username);

        if (find_o == -1) {
            Response response;
            response.status = "200 OK";
            response.message = "win the game";
            return response;
        }

        char message[100];
        Response response;
        response.status = "200 OK";
        response.message = strcat(strcpy(message, "wait success:"), others[find_o].value);
        return response;
    } else {

        int add_o = add_other(others, username, luozi);

        if (add_o == 0) {
            Response response;
            response.status = "500 Internal Server Error";
            response.message = "Internal Server Error";
            return response;
        }

        Response response;
        response.status = "200 OK";
        response.message = "luozi success";
        return response;
    }
}

// 处理认输请求
Response dispose_lose_request(const char *buffer, User *users, Token *tokens, Other *others) {
    printf("lose request\n");
    printf("buffer:\n %s\n", buffer);

    // 读取buffer中的token和type
    char token[20];
    char type[20];
    char lose[6144];
    extract_token_type_other(buffer, token, type, lose);
    printf("token: %s\n", token);
    printf("type: %s\n", type);
    printf("lose: %s\n", lose);

    // 检查token是否正确
    int index = find_token(tokens, token);
    if (index == -1) {
        Response response;
        response.status = "401 Unauthorized";
        response.message = "Invalid token";
        return response;
    }

    printf("index: %d\n", index);
    const char *username = users[tokens[index].value].username;
    printf("username: %s\n", username);

    printf("\n\n");

    // 检查type是否正确（为common,move,noji,dark）
    if (strncmp(type, "common", 6) != 0 && strncmp(type, "move", 4) != 0 &&
        strncmp(type, "noji", 4) != 0 && strncmp(type, "dark", 4) != 0) {
        Response response;
        response.status = "400 Bad Request";
        response.message = "Invalid type";
        return response;
    }

    int find_t = find_token(tokens, username);
    char username2[25];
    strcpy(username2, users[tokens[find_t].value].username);
    remove_token(tokens, username2);
    remove_token(tokens, username);
    remove_other(others, username2);
    remove_other(others, username);

    //将lose写入文件
    Data data = {NULL, 0};
    //malloc分配内存
    data.data = (char *) malloc(sizeof(char) * strlen(lose));
    //将lose写入data
    strcpy(data.data, lose);
    //将size写入data
    data.size = strlen(lose);
    char filename[80];
    char random[32];
    strcpy(filename, "/usr/local/linux/big/build/data/");
    //调用随机数函数
    generate_random_string(random, 24);
    concat_strings(random, ".json", strlen(random), sizeof(random));
    //调用字符串拼接函数
    concat_strings(filename, random, strlen(filename), sizeof(filename));
    int write = write_data(filename, &data);
    if (write != 0) {
        Response response;
        response.status = "500 Internal Server Error";
        response.message = "Internal Server Error";
        return response;
    } else {
        int find_u1 = find_user(users, username);
        int find_u2 = find_user(users, username2);

        if (find_u1 == -1 || find_u2 == -1) {
            Response response;
            response.status = "500 Internal Server Error";
            response.message = "Internal Server Error";
            return response;
        }

        if (users[find_u1].num_data_files == MAX_DATA_FILES || users[find_u2].num_data_files == MAX_DATA_FILES) {
            Response response;
            response.status = "500 Internal Server Error";
            response.message = "Internal Server Error";
            return response;
        }

        char *random_without_line = remove_newline(random);

        strcpy(users[find_u1].data_files[users[find_u1].num_data_files], random_without_line);
        strcpy(users[find_u2].data_files[users[find_u2].num_data_files], random_without_line);

        users[find_u1].num_data_files++;
        users[find_u2].num_data_files++;

        Response response;
        response.status = "200 OK";
        response.message = "lose success";
        return response;
    }
}

// 处理棋谱请求
Response dispose_qipu_request(const char *buffer, User *users, Token *tokens, Other *others) {
    printf("qipu request\n");
    printf("buffer:\n %s\n", buffer);

    // 读取buffer中的token和type
    char token[20];
    char type[20];
    char other[50];
    extract_token_type_other(buffer, token, type, other);
    printf("token: %s\n", token);
    printf("type: %s\n", type);
    printf("other: %s\n", other);

    // 检查token是否正确
    int index = find_token(tokens, token);
    if (index == -1) {
        Response response;
        response.status = "401 Unauthorized";
        response.message = "Invalid token";
        return response;
    }

    printf("index: %d\n", index);
    const char *username = users[tokens[index].value].username;
    printf("username: %s\n", username);

    printf("\n\n");

    // 检查type是否正确（为common,move,noji,dark）
    if (strncmp(type, "common", 6) != 0 && strncmp(type, "move", 4) != 0 &&
        strncmp(type, "noji", 4) != 0 && strncmp(type, "dark", 4) != 0) {
        Response response;
        response.status = "400 Bad Request";
        response.message = "Invalid type";
        return response;
    }

    int find_u = find_user(users, username);

    if (strncmp(other, "qipu", 4) != 0) {
        char output[500];

        if (find_u == -1) {
            Response response;
            response.status = "500 Internal Server Error";
            response.message = "Internal Server Error";
            return response;
        }

        stringfy_user_files(output, &users[find_u]);

        Response response;
        response.status = "200 OK";
        response.message = output;
        return response;
    }
    int find_d = find_data_file(&users[find_u], other);

    if (find_d == -1) {
        Response response;
        response.status = "500 Internal Server Error";
        response.message = "Internal Server Error";
        return response;
    }

    char output2[6144];
    char filename[50];
    strcpy(filename, "/data/");
    concat_strings(filename, users[find_u].data_files[find_d], strlen(filename), sizeof(filename));
    Data data = read_data(filename);
    strcpy(output2, data.data);

    Response response;
    response.status = "200 OK";
    response.message = output2;
    return response;
}

void extract_token_type(const char *buffer, char *token, char *type) {
    // 找到token参数的起始位置
    const char *token_start = strstr(buffer, "token=");

    // 如果找到了token参数
    if (token_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = token_start + strlen("token=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, '&');

        // 如果没有 '&'，则直接取到字符串末尾
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(token, value_start, value_end - value_start);
        token[value_end - value_start] = '\0';
    } else {
        // 没有找到token参数，返回错误
        strcpy(token, "ERROR");
    }

    // 找到type参数的起始位置
    const char *type_start = strstr(buffer, "type=");
    if (type_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = type_start + strlen("type=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, ' ');

        // 如果没有空格，说明type是最后一个参数
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(type, value_start, value_end - value_start);
        type[value_end - value_start] = '\0';
    } else {
        // 没有找到type参数，返回错误
        strcpy(type, "ERROR");
    }
}

void extract_username_password(const char *buffer, char *username, char *password) {
    // 找到username参数的起始位置
    const char *username_start = strstr(buffer, "username=");

    // 如果找到了username参数
    if (username_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = username_start + strlen("username=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, '&');

        // 如果没有 '&'，则直接取到字符串末尾
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(username, value_start, value_end - value_start);
        username[value_end - value_start] = '\0';
    } else {
        // 没有找到username参数，返回错误
        strcpy(username, "ERROR");
    }

    // 找到password参数的起始位置
    const char *password_start = strstr(buffer, "password=");
    if (password_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = password_start + strlen("password=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, ' ');

        // 如果没有空格，说明password是最后一个参数
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(password, value_start, value_end - value_start);
        password[value_end - value_start] = '\0';
    } else {
        // 没有找到password参数，返回错误
        strcpy(password, "ERROR");
    }
}

void extract_token_type_other(const char *buffer, char *token, char *type, char *other) {
    // 找到token参数的起始位置
    const char *token_start = strstr(buffer, "token=");

    // 如果找到了token参数
    if (token_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = token_start + strlen("token=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, '&');

        // 如果没有 '&'，则直接取到字符串末尾
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(token, value_start, value_end - value_start);
        token[value_end - value_start] = '\0';
    } else {
        // 没有找到token参数，返回错误
        strcpy(token, "ERROR");
    }

    // 找到type参数的起始位置
    const char *type_start = strstr(buffer, "type=");
    if (type_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = type_start + strlen("type=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, '&');

        // 如果没有 '&'，则直接取到字符串末尾
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(type, value_start, value_end - value_start);
        type[value_end - value_start] = '\0';
    } else {
        // 没有找到type参数，返回错误
        strcpy(type, "ERROR");
    }

    // 找到other参数的起始位置
    const char *other_start = strstr(buffer, "other=");
    if (other_start != NULL) {
        // 找到参数值的起始位置
        const char *value_start = other_start + strlen("other=");

        // 找到参数值的结束位置
        const char *value_end = strchr(value_start, ' ');

        // 如果没有空格，说明other是最后一个参数
        if (value_end == NULL) {
            value_end = buffer + strlen(buffer);
        }

        // 提取参数值
        strncpy(other, value_start, value_end - value_start);
        other[value_end - value_start] = '\0';
    } else {
        // 没有找到other参数，返回错误
        strcpy(other, "ERROR");
    }
}

// 生成指定长度的随机字符串
size_t generate_random_string(char *str, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;

    // 使用当前时间作为随机数种子
    srand((unsigned int) time(NULL));

    for (size_t i = 0; i < length; ++i) {
        str[i] = charset[rand() % charset_size];
    }

    // 在字符串末尾添加 null 终止符
    str[length] = '\0';

    return length;
}

// 字符串拼接函数
size_t concat_strings(char *destination, const char *source, size_t current_length, size_t destination_size) {
    // 计算要追加的字符串的长度
    size_t source_length = strlen(source);

    // 计算目标字符串剩余的可用空间
    size_t remaining_space = destination_size - current_length;

    // 检查是否有足够的空间来进行拼接
    if (source_length >= remaining_space) {
        fprintf(stderr, "Error: Not enough space for concatenation.\n");
        return current_length;
    }

    // 使用 strncat 进行拼接
    strncat(destination, source, remaining_space);

    // 返回拼接后的新长度
    return current_length + source_length;
}

// 参数为一个字符串，返回不包含\n的字符串
char *remove_newline(char *str) {
    char *pos = strchr(str, '\n');
    if (pos != NULL) {
        *pos = '\0';
    }
    return str;
}