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
 * \file uffs_config.h
 * \brief basic configuration of uffs
 * \author Ricky Zheng
 */

#ifndef _UFFS_CONFIG_H_
#define _UFFS_CONFIG_H_

/**
 * \def UFFS_MAX_PAGE_SIZE
 * \note maximum page size UFFS support
 */
#define UFFS_MAX_PAGE_SIZE		2048

/**
 * \def UFFS_MAX_SPARE_SIZE
 */
#define UFFS_MAX_SPARE_SIZE ((UFFS_MAX_PAGE_SIZE / 256) * 8)

/**
 * \def MAX_CACHED_BLOCK_INFO
 * \note uffs cache the block info for opened directories and files,
 *       a practical value is 5 ~ MAX_OBJECT_HANDLE
 */
#define MAX_CACHED_BLOCK_INFO	10

/** 
 * \def MAX_PAGE_BUFFERS
 * \note the bigger value will bring better read/write performance.
 *       but few writing performance will be improved when this 
 *       value is become larger than 'max pages per block'
 */
#define MAX_PAGE_BUFFERS		10


/** 
 * \def CLONE_BUFFER_THRESHOLD
 * \note reserve buffers for clone. 1 or 2 should be enough.
 */
#define CLONE_BUFFERS_THRESHOLD	2

/**
 * \def MAX_SPARE_BUFFERS
 * \note spare buffers are used for lower level flash operations, 5 should be enough.
 */
#define MAX_SPARE_BUFFERS		5


/**
 * \def MAX_DIRTY_PAGES_IN_A_BLOCK 
 * \note this value should be between '2' and the lesser of 'max pages per block' and (MAX_PAGE_BUFFERS - CLONE_BUFFERS_THRESHOLD - 1).
 *       the smaller the value the frequently the buffer will be flushed.
 */
#define MAX_DIRTY_PAGES_IN_A_BLOCK	7

/**
 * \def MAX_DIRTY_BUF_GROUPS
 */
#define MAX_DIRTY_BUF_GROUPS	3


/**
 * \def CONFIG_USE_STATIC_MEMORY_ALLOCATOR
 * \note uffs will use static memory allocator if this is defined.
 *       to use static memory allocator, you need to provide memory
 *       buffer when creating uffs_Device.
 *
 *       use UFFS_STATIC_BUFF_SIZE() to calculate memory buffer size.
 */
#define CONFIG_USE_STATIC_MEMORY_ALLOCATOR 0

/**
 * \def CONFIG_USE_NATIVE_MEMORY_ALLOCATOR
 * \note  the native memory allocator should only be used for
 *        tracking memory leak bugs or tracking memory consuming.
 *        In your final product, you either disable the native memory
 *        allocator or use the system heap as the memory pool for the
 *        native memory allocator.
 */
#define CONFIG_USE_NATIVE_MEMORY_ALLOCATOR 0


/**
 * \def CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR
 * \note  using system platform's 'malloc' and 'free'.
 */
#define CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR 1



/** 
 * \def CONFIG_FLUSH_BUF_AFTER_WRITE
 * \note UFFS will write all data directly into flash in 
 *       each 'write' call if you enable this option.
 *       (which means lesser data lost when power failue but
 *		 pooer writing performance).
 *		 It's not recommented to open this define for normal applications.
 */
//#define CONFIG_FLUSH_BUF_AFTER_WRITE

/**
 * \def CONFIG_TREE_NODE_USE_DOUBLE_LINK
 * \note: enable double link tree node will speed up insert/delete operation,
 */
#define CONFIG_TREE_NODE_USE_DOUBLE_LINK

/** 
 * \def MAX_OBJECT_HANDLE
 * maximum number of object handle 
 */
#define MAX_OBJECT_HANDLE	10

/**
 * \def MINIMUN_ERASED_BLOCK
 *  UFFS will not allow appending or creating new files when the free/erased block
 *  is lower then MINIMUN_ERASED_BLOCK.
 */
#define MINIMUN_ERASED_BLOCK 2

/**
 * \def CONFIG_CHANGE_MODIFY_TIME
 * \note If defined, closing a file which is opened for writing/appending will
 *       update the file's modify time as well. Disable this feature will save a
 *       lot of writing activities if you frequently open files for write and close it.
 */
//#define CONFIG_CHANGE_MODIFY_TIME


/**
 * \def CONFIG_ENABLE_BAD_BLOCK_VERIFY
 * \note allow erase and verify block marked as 'bad' when format UFFS partition.
 *		it's not recommented for most NAND flash.
 */
#define CONFIG_ENABLE_BAD_BLOCK_VERIFY


/**
 * \def CONFIG_PAGE_WRITE_VERIFY
 * \note verify page data after write, for extra safe data storage.
 */
#define CONFIG_PAGE_WRITE_VERIFY


/** micros for calculating buffer sizes */

/**
 *	\def UFFS_BLOCK_INFO_BUFFER_SIZE
 *	\brief calculate memory bytes for block info caches
 */
#define UFFS_BLOCK_INFO_BUFFER_SIZE(n_pages_per_block)	\
			(											\
				(										\
					sizeof(uffs_BlockInfo) +			\
					sizeof(uffs_PageSpare) * n_pages_per_block \
				 ) * MAX_CACHED_BLOCK_INFO				\
			)

/**
 *	\def UFFS_PAGE_BUFFER_SIZE
 *	\brief calculate memory bytes for page buffers
 */
#define UFFS_PAGE_BUFFER_SIZE(n_page_size)	\
			(								\
				(							\
					sizeof(uffs_Buf) + n_page_size	\
				) * MAX_PAGE_BUFFERS		\
			)

/**
 *	\def UFFS_TREE_BUFFER_SIZE
 *	\brief calculate memory bytes for tree nodes
 */
#define UFFS_TREE_BUFFER_SIZE(n_blocks) (sizeof(TreeNode) * n_blocks)


#define UFFS_SPARE_BUFFER_SIZE (MAX_SPARE_BUFFERS * UFFS_MAX_SPARE_SIZE)


/**
 *	\def UFFS_STATIC_BUFF_SIZE
 *	\brief calculate total memory usage of uffs system
 */
#define UFFS_STATIC_BUFF_SIZE(n_pages_per_block, n_page_size, n_blocks) \
			(		\
				UFFS_BLOCK_INFO_BUFFER_SIZE(n_pages_per_block) + \
				UFFS_PAGE_BUFFER_SIZE(n_page_size) + \
				UFFS_TREE_BUFFER_SIZE(n_blocks) + \
				UFFS_SPARE_BUFFER_SIZE \
			 )



/* config check */
#if (MAX_PAGE_BUFFERS - CLONE_BUFFERS_THRESHOLD) < 3
#error "MAX_PAGE_BUFFERS is too small"
#endif

#if (MAX_DIRTY_PAGES_IN_A_BLOCK < 2)
#error "MAX_DIRTY_PAGES_IN_A_BLOCK should >= 2"
#endif

#if (MAX_PAGE_BUFFERS - CLONE_BUFFERS_THRESHOLD - 1 < MAX_DIRTY_PAGES_IN_A_BLOCK)
#error "MAX_DIRTY_PAGES_IN_A_BLOCK should < (MAX_PAGE_BUFFERS - CLONE_BUFFERS_THRESHOLD)"
#endif

#if defined(CONFIG_PAGE_WRITE_VERIFY) && (CLONE_BUFFERS_THRESHOLD < 2)
#error "CLONE_BUFFERS_THRESHOLD should >= 2 when CONFIG_PAGE_WRITE_VERIFY is enabled."
#endif

#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR + CONFIG_USE_NATIVE_MEMORY_ALLOCATOR + CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR > 1
#error "Please enable ONLY one memory allocator"
#endif

#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR + CONFIG_USE_NATIVE_MEMORY_ALLOCATOR + CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR == 0
#error "Please enable ONE of memory allocators"
#endif


#ifdef WIN32
# pragma warning(disable : 4996)
#endif

#endif
