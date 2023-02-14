//
// Created by Ariel Dang on 2022/11.
//

#ifndef TEST7_DEBUG_FILE_H
#define TEST7_DEBUG_FILE_H


#define DEBUG 1

#define DEBUG_PRINTF(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "node%d:%s:%d:%s(): " fmt "\n", globalMyID, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define DEBUG_PRINT(text) \
        do { if (DEBUG) fprintf(stderr, "node%d:%s:%d:%s(): " text "\n", globalMyID, __FILE__, \
                                __LINE__, __func__); } while (0)

#endif //TEST7_DEBUG_FILE_H
