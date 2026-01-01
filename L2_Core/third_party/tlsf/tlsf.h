/*
** Two Level Segregated Fit memory allocator, version 3.1.
** Written by Matthew Conte
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://www.gii.upv.es/tlsf/main/docs
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
** 
** Copyright (c) 2006-2016, Matthew Conte
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the copyright holder nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __TLSF_H
#define __TLSF_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* tlsf_t：TLSF 结构体。可以包含 1 到 N 个内存池。 */
/* pool_t：TLSF 可以管理的一块内存。 */
typedef void *tlsf_t;
typedef void *pool_t;
    
/* 创建/销毁内存池。 */
tlsf_t tlsf_create(void *mem);
tlsf_t tlsf_create_with_pool(void *mem, size_t bytes);
void tlsf_destroy(tlsf_t tlsf);
pool_t tlsf_get_pool(tlsf_t tlsf);

/* 添加/移除内存池。 */
pool_t tlsf_add_pool(tlsf_t tlsf, void *mem, size_t bytes);
void tlsf_remove_pool(tlsf_t tlsf, pool_t pool);

/* malloc/memalign/realloc/free 的替代实现。 */
void *tlsf_malloc(tlsf_t tlsf, size_t bytes);
void *tlsf_memalign(tlsf_t tlsf, size_t align, size_t bytes);
void *tlsf_realloc(tlsf_t tlsf, void *ptr, size_t size);
void tlsf_free(tlsf_t tlsf, void *ptr);

/* 返回内部块大小，而非原始请求大小 */
size_t tlsf_block_size(void *ptr);

/* 内部结构的开销/限制。 */
size_t tlsf_size(void);
size_t tlsf_align_size(void);
size_t tlsf_block_size_min(void);
size_t tlsf_block_size_max(void);
size_t tlsf_pool_overhead(void);
size_t tlsf_alloc_overhead(void);

/* 调试功能。 */
typedef void (*tlsf_walker)(void *ptr, size_t size, int used, void *user);
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void *user);
/* 如果任何内部一致性检查失败，则返回非零值。 */
int tlsf_check(tlsf_t tlsf);
int tlsf_check_pool(pool_t pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TLSF_H */
