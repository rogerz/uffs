/*
  This file is part of UFFS, the Ultra-low-cost Flash File System.
  
  Copyright (C) 2005-2009 Ricky Zheng <ricky_gz_zheng@yahoo.co.nz>

  UFFS is free software; you can redistribute it and/or modify it under
  the GNU Library General Public License as published by the Free Software 
  Foundation; either version 2 of the License, or (at your option) any
  later version.

  UFFS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  or GNU Library General Public License, as applicable, for more details.
 
  You should have received a copy of the GNU General Public License
  and GNU Library General Public License along with UFFS; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA  02110-1301, USA.

  As a special exception, if other files instantiate templates or use
  macros or inline functions from this file, or you compile this file
  and link it with other works to produce a work based on this file,
  this file does not by itself cause the resulting work to be covered
  by the GNU General Public License. However the source code for this
  file must still be made available in accordance with section (3) of
  the GNU General Public License v2.
 
  This exception does not invalidate any other reasons why a work based
  on this file might be covered by the GNU General Public License.
*/

/**
 * \file uffs_fd.c
 * \brief POSIX like, hight level file operations
 * \author Ricky Zheng, created 8th Jun, 2005
 */

#include <string.h>
#include "uffs/uffs_config.h"
#include "uffs/uffs_fs.h"
#include "uffs/uffs_fd.h"
#define PFX "fd: "


#define FD_OFFSET		3	//!< just make file handler more like POSIX (0, 1, 2 for stdin/stdout/stderr)

#define FD2OBJ(fd)	(((fd) >= FD_OFFSET && (fd) < MAX_DIR_HANDLE + FD_OFFSET) ? \
						(uffs_Object *)uffs_PoolGetBufByIndex(uffs_GetObjectPool(), (fd) - FD_OFFSET) : NULL )

#define OBJ2FD(obj)		(uffs_PoolGetIndex(uffs_GetObjectPool(), obj) + FD_OFFSET)

#define CHK_OBJ(obj, ret)	do { \
								if (uffs_PoolVerify(uffs_GetObjectPool(), (obj)) == U_FALSE || \
										uffs_PoolCheckFreeList(uffs_GetObjectPool(), (obj)) == U_TRUE) { \
									uffs_set_error(-UEBADF); \
									return (ret); \
								} \
							} while(0)

#define CHK_DIR(dirp, ret)	do { \
								if (uffs_PoolVerify(&_dir_pool, (dirp)) == U_FALSE || \
										uffs_PoolCheckFreeList(&_dir_pool, (dirp)) == U_TRUE) { \
									uffs_set_error(-UEBADF); \
									return (ret); \
								} \
							} while(0)

#define CHK_DIR_VOID(dirp)	do { \
								if (uffs_PoolVerify(&_dir_pool, (dirp)) == U_FALSE || \
										uffs_PoolCheckFreeList(&_dir_pool, (dirp)) == U_TRUE) { \
									uffs_set_error(-UEBADF); \
									return; \
								} \
							} while(0)



static int _dir_pool_data[sizeof(uffs_DIR) * MAX_DIR_HANDLE / sizeof(int)];
static uffs_Pool _dir_pool;
static int _uffs_errno = 0;

/**
 * initialise uffs_DIR buffers, called by UFFS internal
 */
URET uffs_InitDirEntryBuf(void)
{
	return uffs_PoolInit(&_dir_pool, _dir_pool_data, sizeof(_dir_pool_data),
			sizeof(uffs_DIR), MAX_DIR_HANDLE);
}

/**
 * Release uffs_DIR buffers, called by UFFS internal
 */
URET uffs_ReleaseDirEntryBuf(void)
{
	return uffs_PoolRelease(&_dir_pool);
}

uffs_Pool * uffs_GetDirEntryBufPool(void)
{
	return &_dir_pool;
}

static uffs_DIR * GetDirEntry(void)
{
	uffs_DIR *dirp = (uffs_DIR *) uffs_PoolGet(&_dir_pool);

	if (dirp)
		memset(dirp, 0, sizeof(uffs_DIR));

	return dirp;
}

static void PutDirEntry(uffs_DIR *p)
{
	uffs_PoolPut(&_dir_pool, p);
}


/** get global errno
 */
int uffs_get_error(void)
{
	return _uffs_errno;
}

/** set global errno
 */
int uffs_set_error(int err)
{
	return (_uffs_errno = err);
}

/* POSIX compliant file system APIs */

int uffs_open(const char *name, int oflag, ...)
{
	uffs_Object *obj;
	int ret = 0;

	obj = uffs_GetObject();
	if (obj == NULL) {
		uffs_set_error(-UEMFILE);
		ret = -1;
	}
	else {
		if (uffs_OpenObject(obj, name, oflag) == U_FAIL) {
			uffs_set_error(-uffs_GetObjectErr(obj));
			uffs_PutObject(obj);
			ret = -1;
		}
		else {
			ret = OBJ2FD(obj);
		}
	}

	return ret;
}

int uffs_close(int fd)
{
	int ret = 0;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);

	uffs_ClearObjectErr(obj);
	if (uffs_CloseObject(obj) == U_FAIL) {
		uffs_set_error(-uffs_GetObjectErr(obj));
		ret = -1;
	}
	else {
		uffs_PutObject(obj);
		ret = 0;
	}

	return ret;
}

int uffs_read(int fd, void *data, int len)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = uffs_ReadObject(obj, data, len);
	uffs_set_error(-uffs_GetObjectErr(obj));

	return ret;
}

int uffs_write(int fd, void *data, int len)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = uffs_WriteObject(obj, data, len);
	uffs_set_error(-uffs_GetObjectErr(obj));

	return ret;
}

long uffs_seek(int fd, long offset, int origin)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = uffs_SeekObject(obj, offset, origin);
	uffs_set_error(-uffs_GetObjectErr(obj));
	
	return ret;
}

long uffs_tell(int fd)
{
	long ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = (long) uffs_GetCurOffset(obj);
	uffs_set_error(-uffs_GetObjectErr(obj));
	
	return ret;
}

int uffs_eof(int fd)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = uffs_EndOfFile(obj);
	uffs_set_error(-uffs_GetObjectErr(obj));
	
	return ret;
}

int uffs_flush(int fd)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = (uffs_FlushObject(obj) == U_SUCC) ? 0 : -1;
	uffs_set_error(-uffs_GetObjectErr(obj));
	
	return ret;
}

int uffs_rename(const char *old_name, const char *new_name)
{
	int err = 0;
	int ret = 0;

	ret = (uffs_RenameObject(old_name, new_name, &err) == U_SUCC) ? 0 : -1;
	uffs_set_error(-err);

	return ret;
}

int uffs_remove(const char *name)
{
	int err = 0;
	int ret = 0;

	if (name[strlen(name) - 1] == '/') {
		err = UENOENT;
		ret = -1;
	}
	else if (uffs_DeleteObject(name, &err) == U_SUCC) {
		ret = 0;
	}
	else {
		ret = -1;
	}

	uffs_set_error(-err);
	return ret;
}

int uffs_truncate(int fd, long remain)
{
	int ret;
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);
	uffs_ClearObjectErr(obj);
	ret = (uffs_TruncateObject(obj, remain) == U_SUCC) ? 0 : -1;
	uffs_set_error(-uffs_GetObjectErr(obj));
	
	return ret;
}

static int do_stat(uffs_Object *obj, struct uffs_stat *buf)
{
	uffs_ObjectInfo info;
	int ret = 0;
	int err = 0;

	if (uffs_GetObjectInfo(obj, &info, &err) == U_FAIL) {
		ret = -1;
	}
	else {
		buf->st_dev = obj->dev->dev_num;
		buf->st_ino = info.serial;
		buf->st_nlink = 0;
		buf->st_uid = 0;
		buf->st_gid = 0;
		buf->st_rdev = 0;
		buf->st_size = info.len;
		buf->st_blksize = obj->dev->com.pg_data_size;
		buf->st_blocks = 0;
		buf->st_atime = info.info.last_modify;
		buf->st_mtime = info.info.last_modify;
		buf->st_ctime = info.info.create_time;
		buf->st_mode = (info.info.attr & FILE_ATTR_DIR ? US_IFDIR : US_IFREG);
		if (info.info.attr & FILE_ATTR_WRITE)
			buf->st_mode |= US_IRWXU;
	}

	uffs_set_error(-err);
	return ret;
}

int uffs_stat(const char *name, struct uffs_stat *buf)
{
	uffs_Object *obj;
	int ret = 0;
	int err = 0;
	int oflag = UO_RDONLY;

	obj = uffs_GetObject();
	if (obj) {
		if (*name && name[strlen(name) - 1] == '/')
			oflag |= UO_DIR;

		if (uffs_OpenObject(obj, name, oflag) == U_SUCC) {
			ret = do_stat(obj, buf);
			uffs_CloseObject(obj);
		}
		else {
			err = uffs_GetObjectErr(obj);
			ret = -1;
		}
		uffs_PutObject(obj);
	}
	else {
		err = UENOMEM;
		ret = -1;
	}

	uffs_set_error(-err);
	return ret;
}

int uffs_lstat(const char *name, struct uffs_stat *buf)
{
	return uffs_stat(name, buf);
}

int uffs_fstat(int fd, struct uffs_stat *buf)
{
	uffs_Object *obj = FD2OBJ(fd);

	CHK_OBJ(obj, -1);

	return do_stat(obj, buf);
}

int uffs_closedir(uffs_DIR *dirp)
{
	CHK_DIR(dirp, -1);

	uffs_FindObjectClose(&dirp->f);
	if (dirp->obj) {
		uffs_CloseObject(dirp->obj);
		uffs_PutObject(dirp->obj);
	}
	PutDirEntry(dirp);

	return 0;
}

uffs_DIR * uffs_opendir(const char *path)
{
	int err = 0;
	uffs_DIR *ret = NULL;
	uffs_DIR *dirp = GetDirEntry();

	if (dirp) {
		dirp->obj = uffs_GetObject();
		if (dirp->obj) {
			if (uffs_OpenObject(dirp->obj, path, UO_RDONLY | UO_DIR) == U_SUCC) {
				if (uffs_FindObjectOpen(&dirp->f, dirp->obj) == U_SUCC) {
					ret = dirp;
					goto ext;
				}
				else {
					uffs_CloseObject(dirp->obj);
				}
			}
			else {
				err = uffs_GetObjectErr(dirp->obj);
			}
			uffs_PutObject(dirp->obj);
			dirp->obj = NULL;
		}
		else {
			err = UEMFILE;
		}
		PutDirEntry(dirp);
	}
	else {
		err = UEMFILE;
	}
ext:
	uffs_set_error(-err);
	return ret;
}

struct uffs_dirent * uffs_readdir(uffs_DIR *dirp)
{
	struct uffs_dirent *ent;

	CHK_DIR(dirp, NULL);

	if (uffs_FindObjectNext(&dirp->info, &dirp->f) == U_SUCC) {
		ent = &dirp->dirent;
		ent->d_ino = dirp->info.serial;
		ent->d_namelen = dirp->info.info.name_len;
		memcpy(ent->d_name, dirp->info.info.name, ent->d_namelen);
		ent->d_name[ent->d_namelen] = 0;
		ent->d_off = dirp->f.pos;
		ent->d_reclen = sizeof(struct uffs_dirent);
		ent->d_type = dirp->info.info.attr;

		return ent;
	}
	else
		return NULL;
}

void uffs_rewinddir(uffs_DIR *dirp)
{
	CHK_DIR_VOID(dirp);

	uffs_FindObjectRewind(&dirp->f);
}


int uffs_mkdir(const char *name, ...)
{
	uffs_Object *obj;
	int ret = 0;
	int err = 0;

	if (name[strlen(name) - 1] != '/') {
		err = UEINVAL;
		ret = -1;
	}
	else {
		obj = uffs_GetObject();
		if (obj) {
			if (uffs_CreateObject(obj, name, UO_CREATE|UO_DIR) != U_SUCC) {
				err = obj->err;
				ret = -1;
			}
			else {
				uffs_CloseObject(obj);
				ret = 0;
			}
			uffs_PutObject(obj);
		}
		else {
			err = UEMFILE;
			ret = -1;
		}
	}

	uffs_set_error(-err);
	return ret;
}

int uffs_rmdir(const char *name)
{
	int err = 0;
	int ret = 0;

	if (name[strlen(name) - 1] != '/') {
		err = UENOENT;
		ret = -1;
	}
	else if (uffs_DeleteObject(name, &err) == U_SUCC) {
		ret = 0;
	}
	else {
		ret = -1;
	}

	uffs_set_error(-err);
	return ret;
}


#if 0
void uffs_seekdir(uffs_DIR *dirp, long loc)
{
	return ;
}

long uffs_telldir(uffs_DIR *dirp)
{
	return 0;
}
#endif
