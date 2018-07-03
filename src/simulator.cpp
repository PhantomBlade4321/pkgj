#include "pkgi.hpp"
extern "C" {
#include "style.h"
}

#include <fmt/format.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PKGI_FOLDER "pkgi"
#define PKGI_APP_FOLDER "app"

void pkgi_log(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    va_end(args);
}

int pkgi_snprintf(char* buffer, uint32_t size, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    int len = vsnprintf(buffer, size - 1, msg, args);
    va_end(args);
    buffer[len] = 0;
    return len;
}

void pkgi_vsnprintf(char* buffer, uint32_t size, const char* msg, va_list args)
{
    int len = vsnprintf(buffer, size - 1, msg, args);
    buffer[len] = 0;
}

char* pkgi_strstr(const char* str, const char* sub)
{
    return strstr((char*)str, sub);
}

int pkgi_stricontains(const char* str, const char* sub)
{
    return strcasestr(str, sub) != NULL;
}

int pkgi_stricmp(const char* a, const char* b)
{
    return strcasecmp(a, b);
}

void pkgi_strncpy(char* dst, uint32_t size, const char* src)
{
    strncpy(dst, src, size);
}

char* pkgi_strrchr(const char* str, char ch)
{
    return strrchr((char*)str, ch);
}

void pkgi_memcpy(void* dst, const void* src, uint32_t size)
{
    memcpy(dst, src, size);
}

void pkgi_memmove(void* dst, const void* src, uint32_t size)
{
    memmove(dst, src, size);
}

int pkgi_memequ(const void* a, const void* b, uint32_t size)
{
    return memcmp(a, b, size) == 0;
}

int pkgi_is_unsafe_mode(void)
{
    return 1;
}

int pkgi_file_exists(const char* path)
{
    struct stat s;
    return stat(path, &s) == 0;
}

void pkgi_mkdirs(const char* ppath)
{
    std::string path = ppath;
    path.push_back('/');
    auto ptr = path.begin();
    while (true)
    {
        ptr = std::find(ptr, path.end(), '/');
        if (ptr == path.end())
            break;

        char last = *ptr;
        *ptr = 0;
        LOG("mkdir %s", path.c_str());
        int err = mkdir(path.c_str(), 0777);
        if (err < 0 && errno != EEXIST)
            throw std::runtime_error(fmt::format(
                    "sceIoMkdir({}) failed: {:#08x}",
                    path.c_str(),
                    static_cast<uint32_t>(err)));
        *ptr = last;
        ++ptr;
    }
}

void pkgi_rm(const char* file)
{
    unlink(file);
}

int pkgi_load(const char* name, void* data, uint32_t max)
{
    int fd = open(name, O_RDONLY, 0777);
    if (fd < 0)
        return -1;

    char* data8 = static_cast<char*>(data);

    int total = 0;
    while (max != 0)
    {
        int readd = read(fd, data8 + total, max);
        if (readd < 0)
        {
            total = -1;
            break;
        }
        else if (readd == 0)
        {
            break;
        }
        total += readd;
        max -= readd;
    }

    close(fd);
    return total;
}

int pkgi_save(const char* name, const void* data, uint32_t size)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        return 0;

    int ret = 1;
    const char* data8 = static_cast<const char*>(data);
    while (size != 0)
    {
        int written = write(fd, data8, size);
        if (written <= 0)
        {
            ret = 0;
            break;
        }
        data8 += written;
        size -= written;
    }

    close(fd);
    return ret;
}

void* pkgi_create(const char* path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        return NULL;
    }

    return (void*)(intptr_t)fd;
}

void* pkgi_openrw(const char* path)
{
    int fd = open(path, O_RDWR, 0777);
    if (fd < 0)
        return NULL;

    return (void*)(intptr_t)fd;
}

int pkgi_seek(void* f, uint64_t offset)
{
    return lseek((intptr_t)f, offset, SEEK_SET);
}

int pkgi_read(void* f, void* buffer, uint32_t size)
{
    return read((intptr_t)f, buffer, size);
}

int pkgi_write(void* f, const void* buffer, uint32_t size)
{
    return write((intptr_t)f, buffer, size);
}

void pkgi_close(void* f)
{
    close((intptr_t)f);
}

uint32_t pkgi_time_msec()
{
    return time(NULL) * 1000;
}
