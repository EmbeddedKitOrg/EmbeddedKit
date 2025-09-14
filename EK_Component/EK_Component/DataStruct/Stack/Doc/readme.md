# EK_Stack - 栈数据结构

## 概述

EK_Stack 是一个基于连续内存实现的LIFO（后进先出）栈数据结构，专为嵌入式系统设计。支持字节流方式存储任意类型数据，提供高效的入栈、出栈操作，并具备完善的状态检查功能。

## 核心特性

- **LIFO结构**：后进先出的数据访问模式，符合栈的基本特性
- **字节流存储**：支持任意数据类型，灵活性强
- **双内存模式**：支持静态和动态内存分配
- **状态监控**：提供栈满、空、剩余空间等状态查询
- **边界检查**：自动检查栈溢出和下溢，确保内存安全
- **连续内存**：基于连续内存块实现，内存访问效率高

## 数据结构

### EK_Stack_t - 栈结构体
```c
typedef struct
{
    void *Stack_Mem;        // 栈内存起始地址
    void *Stack_TopPtr;     // 栈顶指针
    size_t Stack_Capacity;  // 栈容量（字节数）
    bool Stack_isDynamic;   // 动态创建标志
} EK_Stack_t;
```

### 关键字段说明
- **Stack_Mem**：指向栈内存的起始地址（栈底）
- **Stack_TopPtr**：指向当前栈顶位置，初始时等于Stack_Mem
- **Stack_Capacity**：栈的总容量，以字节为单位
- **Stack_isDynamic**：标识内存分配方式，影响销毁时的内存释放

## API 接口

### 栈创建

#### 动态栈创建
```c
EK_Stack_t *EK_pStackCreate_Dynamic(size_t capacity);
```
- **功能**：动态分配内存创建栈
- **参数**：`capacity` - 栈容量（字节数）
- **返回值**：栈指针，失败返回NULL
- **内存管理**：栈结构体和栈空间都由malloc分配

#### 静态栈创建
```c
EK_Result_t EK_rStackCreate_Static(EK_Stack_t *stack, void *mem_ptr, size_t capacity);
```
- **功能**：使用用户提供的内存初始化栈
- **参数**：
  - `stack`：用户分配的栈结构体
  - `mem_ptr`：用户分配的栈空间
  - `capacity`：栈空间大小
- **返回值**：操作结果状态码

### 栈销毁
```c
EK_Result_t EK_rStackDelete(EK_Stack_t *stack);
```
- **功能**：销毁栈并释放资源
- **动态栈**：释放malloc分配的内存
- **静态栈**：清空栈内容，重置栈顶指针

### 状态查询

#### 空栈检查
```c
bool EK_bStackIsEmpty(EK_Stack_t *stack);
```
- **功能**：检查栈是否为空
- **返回值**：空栈返回true

#### 满栈检查
```c
bool EK_bStackIsFull(EK_Stack_t *stack);
```
- **功能**：检查栈是否已满
- **返回值**：满栈返回true

#### 获取剩余空间
```c
size_t EK_sStackGetRemain(EK_Stack_t *stack);
```
- **功能**：获取栈剩余可用空间
- **返回值**：剩余字节数

### 数据操作

#### 入栈操作
```c
EK_Result_t EK_rStackPush(EK_Stack_t *stack, void *data, size_t data_size);
```
- **功能**：向栈顶压入数据
- **参数**：
  - `stack`：栈指针
  - `data`：要压入的数据指针
  - `data_size`：数据大小（字节）
- **特性**：
  - 自动检查剩余空间
  - 数据完整性保证
  - 栈顶指针自动更新

#### 出栈操作
```c
EK_Result_t EK_rStackPop(EK_Stack_t *stack, void *data_buffer, size_t data_size);
```
- **功能**：从栈顶弹出数据
- **参数**：
  - `stack`：栈指针
  - `data_buffer`：接收数据的缓冲区
  - `data_size`：要弹出的数据大小
- **特性**：
  - 数据从栈中完全移除
  - 自动处理栈顶指针更新
  - 边界检查防止下溢

## 使用场景与示例

### 1. 函数调用栈模拟
```c
// 创建函数调用栈
EK_Stack_t *call_stack = EK_pStackCreate_Dynamic(1024);

// 函数调用记录
typedef struct {
    uint32_t return_addr;
    uint32_t local_vars[4];
} CallFrame_t;

// 函数调用时压栈
void function_call(uint32_t addr) {
    CallFrame_t frame = {.return_addr = addr, .local_vars = {0}};
    EK_rStackPush(call_stack, &frame, sizeof(frame));
}

// 函数返回时出栈
uint32_t function_return(void) {
    CallFrame_t frame;
    if (EK_rStackPop(call_stack, &frame, sizeof(frame)) == EK_OK) {
        return frame.return_addr;
    }
    return 0; // 栈为空
}
```

### 2. 表达式求值
```c
// 操作数栈
EK_Stack_t *operand_stack = EK_pStackCreate_Dynamic(256);

// 压入操作数
void push_operand(int value) {
    EK_rStackPush(operand_stack, &value, sizeof(int));
}

// 弹出操作数进行运算
int calculate_add(void) {
    int b, a;
    EK_rStackPop(operand_stack, &b, sizeof(int));
    EK_rStackPop(operand_stack, &a, sizeof(int));
    int result = a + b;
    EK_rStackPush(operand_stack, &result, sizeof(int));
    return result;
}
```

### 3. 括号匹配检查
```c
#define MAX_BRACKETS 64
static uint8_t bracket_buffer[MAX_BRACKETS];
EK_Stack_t bracket_stack;

// 初始化括号检查栈
void bracket_checker_init(void) {
    EK_rStackCreate_Static(&bracket_stack, bracket_buffer, MAX_BRACKETS);
}

// 检查括号匹配
bool check_brackets(const char *expression) {
    for (int i = 0; expression[i] != '\0'; i++) {
        char ch = expression[i];
        
        if (ch == '(' || ch == '[' || ch == '{') {
            // 左括号入栈
            EK_rStackPush(&bracket_stack, &ch, sizeof(char));
        }
        else if (ch == ')' || ch == ']' || ch == '}') {
            // 右括号检查匹配
            if (EK_bStackIsEmpty(&bracket_stack)) return false;
            
            char left_bracket;
            EK_rStackPop(&bracket_stack, &left_bracket, sizeof(char));
            
            if ((ch == ')' && left_bracket != '(') ||
                (ch == ']' && left_bracket != '[') ||
                (ch == '}' && left_bracket != '{')) {
                return false;
            }
        }
    }
    
    return EK_bStackIsEmpty(&bracket_stack); // 栈为空表示匹配
}
```

### 4. 撤销/重做功能
```c
// 操作记录
typedef struct {
    uint8_t operation_type;
    uint32_t data;
    uint32_t timestamp;
} Operation_t;

EK_Stack_t *undo_stack = EK_pStackCreate_Dynamic(sizeof(Operation_t) * 50);
EK_Stack_t *redo_stack = EK_pStackCreate_Dynamic(sizeof(Operation_t) * 50);

// 执行操作并记录
void execute_operation(uint8_t type, uint32_t data) {
    Operation_t op = {
        .operation_type = type,
        .data = data,
        .timestamp = get_tick()
    };
    
    // 清空重做栈
    EK_rStackDelete(redo_stack);
    redo_stack = EK_pStackCreate_Dynamic(sizeof(Operation_t) * 50);
    
    // 记录到撤销栈
    EK_rStackPush(undo_stack, &op, sizeof(op));
    
    // 执行实际操作
    perform_operation(type, data);
}

// 撤销操作
void undo_operation(void) {
    if (!EK_bStackIsEmpty(undo_stack)) {
        Operation_t op;
        EK_rStackPop(undo_stack, &op, sizeof(op));
        
        // 移到重做栈
        EK_rStackPush(redo_stack, &op, sizeof(op));
        
        // 执行撤销
        reverse_operation(op.operation_type, op.data);
    }
}

// 重做操作
void redo_operation(void) {
    if (!EK_bStackIsEmpty(redo_stack)) {
        Operation_t op;
        EK_rStackPop(redo_stack, &op, sizeof(op));
        
        // 移回撤销栈
        EK_rStackPush(undo_stack, &op, sizeof(op));
        
        // 重新执行
        perform_operation(op.operation_type, op.data);
    }
}
```

### 5. 深度优先搜索（DFS）
```c
// 节点结构
typedef struct {
    uint16_t node_id;
    uint8_t depth;
} DFS_Node_t;

EK_Stack_t *dfs_stack = EK_pStackCreate_Dynamic(sizeof(DFS_Node_t) * 100);

// DFS遍历
void dfs_traverse(uint16_t start_node) {
    DFS_Node_t start = {.node_id = start_node, .depth = 0};
    EK_rStackPush(dfs_stack, &start, sizeof(start));
    
    while (!EK_bStackIsEmpty(dfs_stack)) {
        DFS_Node_t current;
        EK_rStackPop(dfs_stack, &current, sizeof(current));
        
        // 访问当前节点
        visit_node(current.node_id, current.depth);
        
        // 获取邻接节点并压栈
        uint16_t neighbors[10];
        int count = get_neighbors(current.node_id, neighbors, 10);
        
        for (int i = count - 1; i >= 0; i--) { // 逆序压栈保持顺序
            if (!is_visited(neighbors[i])) {
                DFS_Node_t next = {
                    .node_id = neighbors[i],
                    .depth = current.depth + 1
                };
                EK_rStackPush(dfs_stack, &next, sizeof(next));
            }
        }
    }
}
```

### 6. 中断上下文保存
```c
// CPU上下文结构
typedef struct {
    uint32_t registers[16];  // 寄存器组
    uint32_t pc;            // 程序计数器
    uint32_t psr;           // 程序状态寄存器
} CPU_Context_t;

#define CONTEXT_STACK_SIZE (sizeof(CPU_Context_t) * 8)
static uint8_t context_buffer[CONTEXT_STACK_SIZE];
EK_Stack_t context_stack;

// 初始化上下文栈
void context_stack_init(void) {
    EK_rStackCreate_Static(&context_stack, context_buffer, CONTEXT_STACK_SIZE);
}

// 中断发生时保存上下文
void save_context(CPU_Context_t *context) {
    if (EK_sStackGetRemain(&context_stack) >= sizeof(CPU_Context_t)) {
        EK_rStackPush(&context_stack, context, sizeof(CPU_Context_t));
    } else {
        // 栈溢出处理
        handle_stack_overflow();
    }
}

// 中断返回时恢复上下文
bool restore_context(CPU_Context_t *context) {
    if (!EK_bStackIsEmpty(&context_stack)) {
        EK_rStackPop(&context_stack, context, sizeof(CPU_Context_t));
        return true;
    }
    return false;
}
```

### 7. 内存分配跟踪
```c
// 内存分配记录
typedef struct {
    void *ptr;
    size_t size;
    uint32_t alloc_time;
} MemAlloc_t;

EK_Stack_t *alloc_stack = EK_pStackCreate_Dynamic(sizeof(MemAlloc_t) * 32);

// 跟踪内存分配
void *tracked_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr != NULL) {
        MemAlloc_t record = {
            .ptr = ptr,
            .size = size,
            .alloc_time = get_tick()
        };
        EK_rStackPush(alloc_stack, &record, sizeof(record));
    }
    return ptr;
}

// 释放最近分配的内存
void free_last_allocation(void) {
    if (!EK_bStackIsEmpty(alloc_stack)) {
        MemAlloc_t record;
        EK_rStackPop(alloc_stack, &record, sizeof(record));
        free(record.ptr);
        printf("Freed %zu bytes allocated at time %u\n", 
               record.size, record.alloc_time);
    }
}

// 清理所有跟踪的内存
void cleanup_all_allocations(void) {
    while (!EK_bStackIsEmpty(alloc_stack)) {
        free_last_allocation();
    }
}
```

## 设计优势

### 1. 连续内存优化
- **缓存友好**：连续内存访问，提高CPU缓存命中率
- **无内存碎片**：固定大小的连续内存空间
- **高效访问**：通过指针算术实现O(1)的栈顶访问

### 2. 字节流设计
- **类型无关**：可存储任意数据类型
- **灵活分割**：支持任意大小的数据块
- **内存高效**：无额外的节点开销

### 3. 双模式内存管理
- **静态分配**：编译时确定，无运行时开销
- **动态分配**：运行时灵活创建，便于资源管理

### 4. 完善的边界检查
- **溢出保护**：防止栈溢出导致的内存破坏
- **下溢保护**：防止空栈弹出操作
- **空间查询**：便于预先检查可用空间

## 性能特点

- **入栈操作**：O(1) 时间复杂度
- **出栈操作**：O(1) 时间复杂度
- **状态查询**：O(1) 时间复杂度
- **内存开销**：栈结构体约16字节 + 用户栈空间
- **线程安全**：需要外部同步保护

## 注意事项

1. **容量规划**：根据数据使用模式合理设置栈容量
2. **数据对齐**：注意结构体数据的内存对齐问题
3. **错误处理**：检查API返回值，处理栈满/空等异常情况
4. **内存管理**：动态栈记得在不需要时调用删除函数
5. **并发访问**：多线程环境需要适当的同步机制
6. **栈溢出**：深度递归或大量数据时注意栈容量限制

该栈模块为嵌入式系统提供了高效、可靠的LIFO数据结构解决方案，特别适用于函数调用管理、表达式求值、算法实现、上下文保存等场景。