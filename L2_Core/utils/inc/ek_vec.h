/**
 * @file ek_vec.h
 * @brief 动态数组（Vector）实现
 * @author N1netyNine99
 *
 * 提供类型安全的动态数组实现，支持动态扩容、元素添加/删除等操作
 * 使用宏实现类型生成，避免 void* 的类型不安全问题
 *
 * @note 使用前必须通过 EK_VEC_IMPLEMENT(type) 宏定义特定类型的向量
 * @note 所有内存操作使用 ek_malloc/ek_realloc/ek_free，需确保内存管理模块已初始化
 */

#ifndef EK_VEC_H
#define EK_VEC_H

#include "../../../ek_conf.h"

#if EK_VEC_ENABLE == 1

#    include "ek_def.h"
#    include "ek_mem.h"

/**
 * @brief 大数组扩容阈值
 *
 * 当容量小于此阈值时，扩容策略为翻倍；大于等于此阈值时，扩容策略为增加当前容量的 1/2
 */
#    define VEC_LARGE_THRESHOLD (32)

/**
 * @brief 定义指定类型的动态数组结构体
 * @param type 元素数据类型
 *
 * @example
 * // 定义 int 类型的动态数组
 * EK_VEC_IMPLEMENT(int);
 *
 * // 使用动态数组
 * ek_vec_t(int) my_vec;
 * ek_vec_init(my_vec);
 * ek_vec_append(my_vec, 42);
 * ek_vec_destroy(my_vec);
 */
#    define EK_VEC_IMPLEMENT(type) \
        typedef struct             \
        {                          \
            type *items;           \
            uint32_t amount;       \
            uint32_t cap;          \
        } ek_vec_##type##_t

/**
 * @brief 获取指定类型的动态数组类型名
 * @param type 元素数据类型
 */
#    define ek_vec_t(type) ek_vec_##type##_t

/**
 * @brief 正向遍历动态数组
 * @param pos 当前索引（迭代变量，通常为 uint32_t 类型）
 * @param v 动态数组变量
 *
 * @warning 遍历过程中不能删除元素，否则会导致索引混乱
 */
#    define ek_vec_iterate(pos, v) for (pos = 0; pos < v.amount; pos++)

/**
 * @brief 从指定索引开始遍历动态数组
 * @param pos 当前索引（迭代变量，通常为 uint32_t 类型）
 * @param index 起始索引
 * @param v 动态数组变量
 *
 * @note 如果 index 超出范围，则从数组末尾开始（不会执行循环体）
 */
#    define ek_vec_iterate_index(pos, index, v) for (pos = (index < v.amount) ? index : v.amount; pos < v.amount; pos++)

/**
 * @brief 初始化动态数组
 * @param v 动态数组变量
 *
 * @note 使用前必须调用此宏进行初始化
 */
#    define ek_vec_init(v)  \
        do                  \
        {                   \
            v.items = NULL; \
            v.amount = 0;   \
            v.cap = 0;      \
        } while (0)

/**
 * @brief 销毁动态数组并释放内存
 * @param v 动态数组变量
 *
 * @note 销毁后动态数组变量仍存在，但内容已清空
 * @note 如需重新使用，应重新调用 ek_vec_init
 */
#    define ek_vec_destroy(v) \
        do                    \
        {                     \
            ek_free(v.items); \
            v.amount = 0;     \
            v.cap = 0;        \
        } while (0)

/**
 * @brief 向动态数组末尾追加元素
 * @param v 动态数组变量
 * @param val 要追加的元素值
 *
 * @note 当容量不足时自动扩容
 * @note 扩容策略：小数组翻倍，大数组增加 1/2
 */
#    define ek_vec_append(v, val)                                                                       \
        do                                                                                              \
        {                                                                                               \
            if (v.cap <= v.amount)                                                                      \
            {                                                                                           \
                uint32_t _temp_for_cap_ex_ =                                                            \
                    (v.cap < VEC_LARGE_THRESHOLD) ? (v.cap ? 2 * v.cap : 8) : (v.cap + v.cap / 2);      \
                void *_temp_for_new_items_ = ek_realloc(v.items, _temp_for_cap_ex_ * sizeof(*v.items)); \
                if (_temp_for_new_items_ != NULL)                                                       \
                {                                                                                       \
                    v.cap = _temp_for_cap_ex_;                                                          \
                    v.items = _temp_for_new_items_;                                                     \
                }                                                                                       \
                else break;                                                                             \
            }                                                                                           \
            v.items[v.amount++] = val;                                                                  \
        } while (0)

/**
 * @brief 从动态数组中移除指定位置的元素
 * @param v 动态数组变量
 * @param index 要移除的元素索引
 *
 * @note 移除后，后续元素会向前移动填补空位
 * @note 如果索引超出范围，则不执行任何操作
 */
#    define ek_vec_remove(v, index)                                                                             \
        do                                                                                                      \
        {                                                                                                       \
            if (index < v.amount)                                                                               \
            {                                                                                                   \
                for (uint32_t _temp_for_iter_v_ = index; _temp_for_iter_v_ < v.amount - 1; _temp_for_iter_v_++) \
                {                                                                                               \
                    v.items[_temp_for_iter_v_] = v.items[_temp_for_iter_v_ + 1];                                \
                }                                                                                               \
                v.amount--;                                                                                     \
            }                                                                                                   \
        } while (0)

/**
 * @brief 无序移除动态数组中的元素（O(1) 时间复杂度）
 * @param v 动态数组变量
 * @param index 要移除的元素索引
 *
 * @note 通过将最后一个元素移动到待删除位置实现快速删除
 * @note 时间复杂度：O(1)
 * @warning 会破坏数组中元素的原始顺序，仅适用于对顺序不敏感的场景
 */
#    define ek_vec_remove_unorder(v, index)             \
        do                                              \
        {                                               \
            if (index < v.amount)                       \
            {                                           \
                v.items[index] = v.items[v.amount - 1]; \
                v.amount--;                             \
            }                                           \
        } while (0)

/**
 * @brief 清空动态数组（不释放内存）
 * @param v 动态数组变量
 *
 * @note 仅将元素数量置零，保留已分配的内存空间
 * @note 如需释放内存，应使用 ek_vec_destroy 或 ek_vec_shrink
 */
#    define ek_vec_clear(v) \
        do                  \
        {                   \
            v.amount = 0;   \
        } while (0)

/**
 * @brief 收缩动态数组内存至实际大小
 * @param v 动态数组变量
 *
 * @note 将容量缩减为当前元素数量，释放多余内存,如果元素数目为0，则会释放items的所有内存
 * @note 如果 realloc 失败，则保持原状态不变
 */
#    define ek_vec_shrink(v)                                                                   \
        do                                                                                     \
        {                                                                                      \
            if (v.amount)                                                                      \
            {                                                                                  \
                void *_temp_for_new_items_ = ek_realloc(v.items, v.amount * sizeof(*v.items)); \
                if (_temp_for_new_items_)                                                      \
                {                                                                              \
                    v.items = _temp_for_new_items_;                                            \
                    v.cap = v.amount;                                                          \
                }                                                                              \
            }                                                                                  \
            else if (v.items)                                                                  \
            {                                                                                  \
                ek_free(v.items);                                                              \
                v.items = NULL;                                                                \
                v.cap = 0;                                                                     \
            }                                                                                  \
        } while (0)

#endif /* EK_VEC_ENABLE */

#endif /* EK_VEC_H */
