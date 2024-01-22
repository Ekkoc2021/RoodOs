/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-03 23:32:03
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-13 21:11:15
 * @Description:
 *      字符串相关函数: 以\0结尾
 *          长度计算:strlen_
 *          字符串复制:strncpy_
 *          字符串拼接:strncat_
 *          字符串比较:strcmp_
 *          字符查找:strchr_  strrchr
 *          字符串匹配:strstr_ kmp? 大概率用不到不写了
 *          字符串格式化返回:sprintf_
 *          内存设置:memset_
 *          内存拷贝:memcpy_
 *          十进制转十六进制
 */

// #define STR_DEBUG
#ifndef STR_H_2413
#define STR_H_2413

typedef unsigned int sizeT;
typedef unsigned long long ui64;
typedef int int32;
#include "stdarg.h"

sizeT itos_(char *buff, int32 i);

void uiToHexStr(sizeT i, char *buff);

void iToHexStr(sizeT i, char *buff);

sizeT sprintf_(char *str, const char *format, ...);

char *strncat_(char *dest, const char *src, sizeT n);

sizeT strcpy_(char *dest, const char *src);

char *strncpy_(char *dest, const char *src, sizeT n);

sizeT strlen_(const char *str);

// integer to string
sizeT uitos_(char *buff, sizeT i);

// double to string
sizeT ftos_(char *buff, double f, sizeT precision);

// string to integer
sizeT stoi_(const char *buff);

// string to double
double stof_(const char *buff);

void memset_(char *str, char a, sizeT n);

// return a-b
char strcmp_(const char *a, const char *b);

// get the index of a in template,not include return -1
sizeT strchr_(const char *template, char a);

void memcpy_(char *destination, char *source, sizeT size);

#endif