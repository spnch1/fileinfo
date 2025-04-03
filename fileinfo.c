#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdint.h>

void print_file_permissions(DWORD attributes) {
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        printf("d");
    } else {
        printf("-");
    }
    printf(attributes & FILE_ATTRIBUTE_READONLY ? "r" : "-");
    printf("w");
    printf("x");
    printf("rwx");
}

void print_human_readable_size(uint64_t size) {
    if (size < 1024) {
        printf("%llu bytes", size);
    } else if (size < 1024 * 1024) {
        printf("%.2f KB", (double)size / 1024);
    } else if (size < 1024 * 1024 * 1024) {
        printf("%.2f MB", (double)size / (1024 * 1024));
    } else {
        printf("%.2f GB", (double)size / (1024 * 1024 * 1024));
    }
}

void print_file_info(const char *filepath, const int recursive) {
    WIN32_FIND_DATA find_data;
    FILETIME ftLastWriteTime;
    SYSTEMTIME stLastWriteTime;
    FILETIME ftLocalLastWriteTime;

    HANDLE hFind = FindFirstFile(filepath, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "File not found: %s\n", filepath);
        return;
    }

    print_human_readable_size((((uint64_t)find_data.nFileSizeHigh) << 32) | find_data.nFileSizeLow);
    printf("\n");
    print_file_permissions(find_data.dwFileAttributes);
    printf(" ");

    ftLastWriteTime = find_data.ftLastWriteTime;
    FileTimeToLocalFileTime(&ftLastWriteTime, &ftLocalLastWriteTime);
    FileTimeToSystemTime(&ftLocalLastWriteTime, &stLastWriteTime);

    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
           stLastWriteTime.wYear, stLastWriteTime.wMonth, stLastWriteTime.wDay,
           stLastWriteTime.wHour, stLastWriteTime.wMinute, stLastWriteTime.wSecond);

    FindClose(hFind);

    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && recursive) {
        char search_path[MAX_PATH];
        snprintf(search_path, MAX_PATH, "%s\\*", filepath);
        hFind = FindFirstFile(search_path, &find_data);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
                    snprintf(search_path, MAX_PATH, "%s\\%s", filepath, find_data.cFileName);
                    print_file_info(search_path, recursive);
                }
            } while (FindNextFile(hFind, &find_data) != 0);
            FindClose(hFind);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-r] <filepath>\n", argv[0]);
        return 1;
    }

    int recursive = 0;
    const char *filepath;

    if (strcmp(argv[1], "-r") == 0) {
        recursive = 1;
        if (argc < 3) {
            fprintf(stderr, "Usage: %s [-r] <filepath>\n", argv[0]);
            return 1;
        }
        filepath = argv[2];
    } else {
        filepath = argv[1];
    }
    print_file_info(filepath, recursive);

    return 0;
}