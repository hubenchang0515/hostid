#include "hostid.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sha2.h"

#define SERIAL_MAX_LENGTH 1024

#if defined(__linux__) || defined(linux) || defined(__linux)

#include <linux/limits.h>
#include <libmount/libmount.h>
#include <libudev.h>

static const char* root_mount_point = "/";

static const char* get_device_path_of_mount_point(const char* mount_point)
{
    static char device_path[PATH_MAX];
    device_path[0] = 0;

    struct libmnt_context* ctx = mnt_new_context();
    if (ctx == NULL)
        return NULL;

    struct libmnt_table* table = NULL;

    if (mnt_context_get_fstab(ctx, &table) < 0)
    {
        mnt_free_context(ctx);
        return NULL;
    }

    struct libmnt_fs* fs = mnt_table_find_target(table, mount_point, MNT_ITER_BACKWARD);
    if (fs == NULL)
    {
        mnt_free_context(ctx);
        return NULL;
    }

    const char* path = mnt_fs_get_srcpath(fs);
    strncpy(device_path, path, PATH_MAX);
    mnt_free_context(ctx);
    return device_path;
}


static const char* get_serial_of_device(const char* device_path)
{
    static char device_serial[SERIAL_MAX_LENGTH];
    device_serial[0] = 0;

    struct udev* udev = udev_new();
    if (udev == NULL)
        return NULL;

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (enumerate == NULL)
    {
        udev_unref(udev);
        return NULL;
    }

    if (udev_enumerate_scan_devices(enumerate) < 0)
    {
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return NULL;
    }

    struct udev_list_entry* device_entry = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(device_entry, device_entry)
    {
        const char* syspath = udev_list_entry_get_name(device_entry);
        struct udev_device* device = udev_device_new_from_syspath(udev, syspath);
        struct udev_list_entry* link_entry = udev_device_get_devlinks_list_entry(device);

        udev_list_entry_foreach(link_entry, link_entry)
        {
            const char* link = udev_list_entry_get_name(link_entry);
            if (strcmp(link, device_path) == 0)
            {
                const char* serial = udev_device_get_property_value(device, "ID_SERIAL");
                strncpy(device_serial, serial, SERIAL_MAX_LENGTH);
                udev_device_unref(device);
                goto SUCCESS;
            }
        }

        udev_device_unref(device);
    }

SUCCESS:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return device_serial;
}

#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include <Windows.h>

static const char* root_mount_point = "C:";

static const char* get_device_path_of_mount_point(const char* mount_point)
{
    static char device_path[MAX_PATH];
    device_path[0] = 0;

    strncpy(device_path, "\\\\.\\", MAX_PATH);
    strncat(device_path,  mount_point, MAX_PATH);
    return device_path;
}

static const char* get_serial_of_device(const char* device_path)
{
    static char device_serial[SERIAL_MAX_LENGTH];
    device_serial[0] = 0;

    HANDLE device = CreateFileA(device_path, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (device == INVALID_HANDLE_VALUE)
        return NULL;

    STORAGE_PROPERTY_QUERY query;
    query.PropertyId = StorageDeviceProperty;
    query.QueryType  = PropertyStandardQuery;

    STORAGE_DESCRIPTOR_HEADER header;
    BOOL success = DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), NULL, NULL);
    if (!success || header.Size == 0)
    {
        CloseHandle(device);
        return NULL;
    }

    void* buffer = malloc(header.Size);
    if (buffer == NULL)
    {
        CloseHandle(device);
        return NULL;
    }
    success = DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer, header.Size, NULL, NULL);
    if (!success)
    {
        free(buffer);
        CloseHandle(device);
        return NULL;
    }

    const STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
    strncpy(device_serial, (char*)buffer + descriptor->SerialNumberOffset, SERIAL_MAX_LENGTH);

    free(buffer);
    CloseHandle(device);
    return device_serial;
}

#else
    #error OS not supported
#endif


const char* host_id()
{
    const char* device_path = get_device_path_of_mount_point(root_mount_point);
    if (device_path == NULL) return NULL;
    const char* device_serial = get_serial_of_device(device_path);
    if (device_serial == NULL) return NULL;

    static Sha512 sha512;
    sha512Reset(&sha512);
    return sha512OfString(&sha512, device_serial);
}

