/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSRT_AVL_H
#define DDSRT_AVL_H

/* The tree library never performs memory allocations or deallocations internally.

   - Treedef_t: defines the properties of the tree, offsets,
     comparison functions, augmented structures, flags -- these are
     related to the code/data structure in which the tree is embedded,
     and in nearly all cases known at compile time.
   - avlTree_t: represents the tree, i.e., pointer to the root.
   - avlNode_t: contains the administrative data for a single node in
     the tree.

   For a tree node:
     struct T {
       avlNode_t avlnode;
       int key;
     };
   by definition, avlnodeoffset == offsetof(struct T, avlnode) and
   keyoffset = offsetof(struct T, key). The user of the library only
   ever deals in pointers to (in this case) struct T, never with
   pointers to the avlNode_t, and the compare function operations on
   pointers to keys, in this case pointers to "int"s. If you wish, you
   can also do: keyoffset = 0, in which case the compare function
   would be operating on struct T's.

   The compare function is assumed to behave just like all compare
   functions in the C library: < 0, =0, >0 for left argument less
   than, equal to or greater than the right argument.

   The "augment" function is automatically called whenever some of the
   children of a node change, as well as when the "augment" function
   has been called on some of the children. It allows you to maintain
   a "summary" of the subtree -- currently only used in DDSI, in one
   spot.

   Trees come in various "variants", configured through "treedef"
   flags:
   - direct/indirect key: direct meaning the key value is embedded in
     the structure containing the avlNode_t, indirect meaning a
     pointer to the key value is. The compare function doesn't deal
     with tree nodes, but with key values.
   - re-entrant: in the style of the C library, meaning, the
     comparison function gets a user-supplied 3rd argument (in
     particular used by mmstat).
   - unique keys/duplicate keys: when keys must be unique, some
     optimizations apply; it is up to the caller to ensure one doesn't
     violate the uniqueness of the keys (it'll happily crash in insert
     if you don't); when duplicate keys are allowed, a forward scan of
     the tree will visit them in the order of insertion.

   For a tree node:
     struct T {
       avlnode_t avlnode;
      char *key;
     };
   you could set the "indirect" flag, and then you simply use
   strcmp(), avoiding the need for passing templates in looking up key
   values. Much nicer.

   There is also an orthogonal variant that is enforced through the
   type system -- note that would be possible for all of the above as
   well, but the number of cases simply explodes and none of the above
   flags affects the dynamically changing data structures (just the
   tree definition), unlike this one.

   - the "C" variant keeps track of the number of nodes in the tree to
     support a "count" operation in O(1) time, but is otherwise
     identical.

   The various initializer macros and TreedefInit functions should
   make sense with this.

   All functions for looking up nodes return NULL if there is no node
   satisfying the requirements.

   - Init: initializes a tree (really just: root = NULL, perhaps count = 0)
   - Free: calls "freefun" on each node, which may free the node
   - FreeArg: as "Free", but with an extra, user-supplied, argument
   - Root: returns the root node
   - Lookup: returns a node with key value "key" (ref allowdups flag)
   - LookupIPath: like Lookup, but also filling an IPath_t structure
     for efficient insertion in case of a failed lookup (or inserting
     duplicates)
   - LookupDPath: like Lookup, but also filling a DPath_t structure
     that helps with deleting a node
   - LookupPredEq: locates the node with the greatest key value <= "key"
   - LookupSuccEq: similar, but smallest key value >= "key"
   - LookupPred: similar, < "key"
   - LookupSucc: similar, > "key"
   - Insert: convenience function: LookupIPath ; InsertIPath
   - Delete: convenience function: LookupDPath ; DeleteDPath
   - InsertIPath: insert node based on the "path" obtained from LookupIPath
   - DeleteDPath: delete node, using information in "path" to do so efficiently
   - SwapNode: replace "oldn" by "newn" without modifying the tree
     structure (the key need not be equal, but must be
     FindPred(oldn).key < newn.key < FindSucc(oldn).key, where a
     non-existing predecessor has key -inf and a non-existing
     successor has key +inf, and where it is understood that the <
     operator becomes <= if allowdups is set
   - AugmentUpdate: to be called when something in "node" changes that
     affects the subtree "summary" computed by the configured
     "augment" function
   - IsEmpty: returns 1 if tree is empty, 0 if not
   - IsSingleton: returns 1 if tree contains exactly one node, 0 if not
   - FindMin: returns the node with the smallest key value in the tree
   - FindMax: similar, largest key value
   - FindPred: preceding node in in-order treewalk
   - FindSucc: similar, following node

   - Walk: calls "f" with user-supplied argument "a" once for each
     node, starting at FindMin and ending at FindMax
   - ConstWalk: same, but with a const tree
   - WalkRange: like Walk, but only visiting nodes with key values in
     range [min,max] (that's inclusive)
   - ConstWalkRange: same, but with a const tree
   - WalkRangeReverse: like WalkRange, but in the reverse direction
   - ConstWalkRangeReverse: same, but with a const tree
   - IterFirst: starts forward iteration, starting at (and returning) FindMin
   - IterSuccEq: similar, starting at LookupSuccEq
   - IterSucc: similar, starting at LookupSucc
   - IterNext: returns FindSucc(last returned node); may not be called
     if preceding IterXXX call on same "iter" returned NULL

   That's all there is to it.

   Note that all calls to Walk(f,a) can be rewritten as:
     for(n=IterFirst(&it); n; n=IterNext(&it)) { f(n,a) }
   or as
     for(n=FindMin(); n; n=FindSucc(n)) { f(n,a) }

   The walk functions and iterators may not alter the tree
   structure. If that is desired, the latter can easily be rewritten
   as:
     n=FindMin() ; while(n) { nn=FindSucc(n); f(n,a); n=nn }
   because FindMin/FindSucc doesn't store any information to allow
   fast processing. That'll allow every operation, with the obvious
   exception of f(n) calling Delete(FindSucc(n)).

   Currently, all trees maintain parent pointers, but it may be worth
   doing a separate set without it, as it reduces the size of
   avlNode_t. But in that case, the FindMin/FindSucc option would no
   longer be a reasonable option because it would be prohibitively
   expensive, whereas the IterFirst/IterNext option are alway
   efficiently. If one were to do a threaded tree variant, the
   implemetantion of IterFirst/IterNext would become absolute trivial
   and faster still, but at the cost of significantly more overhead in
   memory and updates. */

/* 树库永远不会在内部执行内存分配或释放。

   - Treedef_t：定义树的属性，偏移量，
     比较函数，增强结构，标志 -- 这些是
     与嵌入树的代码/数据结构相关，
     并且在几乎所有情况下都在编译时知道。
   - avlTree_t：表示树，即指向根的指针。
   - avlNode_t：包含树中单个节点的管理数据。

   对于一个树节点：
     struct T {
       avlNode_t avlnode;
       int key;
     };
   根据定义，avlnodeoffset == offsetof(struct T, avlnode) 和
   keyoffset = offsetof(struct T, key)。库的用户只
   与（在本例中）struct T 的指针打交道，而不是
   与 avlNode_t 的指针，比较函数操作的是
   指向键的指针，在这种情况下是指向 "int" 的指针。如果你愿意，
   你也可以这样做：keyoffset = 0，此时比较函数
   将在 struct T 上操作。

   比较函数被认为与 C 库中的所有比较
   函数一样：<0，=0，>0 表示左参数小于
   等于或大于右参数。

   “增强”函数在节点的某些子节点发生更改时会自动调用，
   以及在已经调用了某些子节点的 "增强" 函数时。它允许你维护
   子树的 "摘要" -- 目前仅在 DDSI 中使用，在一个
   地方。

   树有各种 "变体"，通过 "treedef"
   标志进行配置：
   - 直接/间接键：直接表示键值嵌入到
     包含 avlNode_t 的结构中，间接表示指向
     键值的指针。比较函数不处理
     树节点，而是处理键值。
   - 可重入：以 C 库的风格，意味着，
     比较函数获取用户提供的第三个参数（特别是
     由 mmstat 使用）。
   - 唯一键/重复键：当键必须唯一时，应用某些
     优化；确保不违反键的唯一性取决于调用者（如果你不这样做
     它会在插入时愉快地崩溃）；当允许重复键时，树的前向扫描将
     按插入顺序访问它们。

   对于一个树节点：
     struct T {
       avlnode_t avlnode;
       char *key;
     };
   你可以设置 "indirect" 标志，然后你只需使用
   strcmp()，避免在查找键时传递模板
   值。更好。

   还有一个通过类型系统强制执行的正交变体 -- 请注意，所有上述情况也可以
   以及，但是案例数量简直爆炸，而且上面没有一个
   标志会影响动态更改的数据结构（只是
   树定义），与这个不同。

   - "C" 变体跟踪树中节点的数量
     支持 O(1) 时间内的 "count" 操作，但其他方面
     相同。

   各种初始化宏和 TreedefInit 函数应该
   随着这个有意义。

   查找节点的所有函数在没有节点时返回 NULL
   满足要求。

   - Init：初始化一棵树（真的只是：root = NULL，可能 count = 0）
   - Free：对每个节点调用 "freefun"，可以释放节点
   - FreeArg：与 "Free" 相同，但带有额外的用户提供的参数
   - Root：返回根节点
   - Lookup：返回具有键值 "key" 的节点（参考 allowdups 标志）
   - LookupIPath：类似于 Lookup，但还填充了 IPath_t 结构
     以便在查找失败（或插入重复项）时进行高效插入
   - LookupDPath：类似于 Lookup，但还填充了 DPath_t 结构
     有助于删除节点
   - LookupPredEq：定位键值最大且 <= "key" 的节点
   - LookupSuccEq：类似，但键值最小且 >= "key"
   - LookupPred：类似，< "key"
   - LookupSucc：类似，> "key"
   - Insert：方便函数：LookupIPath；InsertIPath
   - Delete：方便函数：LookupDPath；DeleteDPath
   - InsertIPath：根据从 LookupIPath 获取的 "path" 插入节点
   - DeleteDPath：删除节点，使用 "path" 中的信息高效地执行此操作
   - SwapNode：在不修改树结构的情况下用 "newn" 替换 "oldn"
     （键不需要相等，但必须是
     FindPred(oldn).key < newn.key < FindSucc(oldn).key，其中一个
     不存在的前驱具有键 -inf，不存在的
     后继具有键 +inf，如果设置了 allowdups，则 <
     运算符变为 <=
   - AugmentUpdate：在 "node" 中发生影响由配置的
     "增强" 函数计算的子树 "摘要" 的更改时调用
   - IsEmpty：如果树为空，则返回 1，否则返回 0
   - IsSingleton：如果树只包含一个节点，则返回 1，否则返回 0
   - FindMin：返回树中具有最小键值的节点
   - FindMax：类似，最大键值
   - FindPred：按顺序遍历树的前一个节点
   - FindSucc：类似，后一个节点

   - Walk：对每个节点调用 "f" 一次，带有用户提供的参数 "a"
     从 FindMin 开始，到 FindMax 结束
   - ConstWalk：相同，但是对于一个 const 树
   - WalkRange：像 Walk 一样，但只访问键值在
     范围 [min,max]（包含）的节点
   - ConstWalkRange：相同，但是对于一个 const 树
   - WalkRangeReverse：像 WalkRange 一样，但反向
   - ConstWalkRangeReverse：相同，但是对于一个 const 树
   - IterFirst：开始正向迭代，从（并返回）FindMin 开始
   - IterSuccEq：类似，从 LookupSuccEq 开始
   - IterSucc：类似，从 LookupSucc 开始
   - IterNext：返回 FindSucc（上次返回的节点）；如果之前的 IterXXX 调用
     在相同的 "iter" 上返回 NULL，则可能不会被调用

   就这些了。

   请注意，所有对 Walk(f,a) 的调用都可以重写为：
     for(n=IterFirst(&it); n; n=IterNext(&it)) { f(n,a) }
   或者
     for(n=FindMin(); n; n=FindSucc(n)) { f(n,a) }

  以下代码是用C编程语言编写的：遍历函数和迭代器不能改变树结构。如果需要这样做，后者可以轻松地重写为：
    n=FindMin()；while(n) { nn=FindSucc(n); f(n,a); n=nn }
  因为FindMin/FindSucc不存储任何信息以允许快速处理。这将允许每个操作，显然除了f(n)调用Delete(FindSucc(n))。

  目前，所有树都维护父指针，但是可能值得单独设置一个没有它的集合，因为它减小了avlNode_t的大小。但在这种情况下，FindMin/FindSucc选项将不再是一个合理的选择，因为它将变得非常昂贵，而IterFirst/IterNext选项总是有效的。如果有人要做一个线程树变体，IterFirst/IterNext的实现将变得绝对简单且更快，但以内存和更新中的显著更多开销为代价。*/

#include <stdint.h>
#include <stdlib.h>

#include "dds/ddsrt/attributes.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DDSRT_AVL_MAX_TREEHEIGHT (12 * sizeof(void *))

typedef int (*ddsrt_avl_compare_t)(const void *a, const void *b);
typedef int (*ddsrt_avl_compare_r_t)(const void *a, const void *b, void *arg);
typedef void (*ddsrt_avl_augment_t)(void *node, const void *left, const void *right);
typedef void (*ddsrt_avl_walk_t)(void *node, void *arg);
typedef void (*ddsrt_avl_const_walk_t)(const void *node, void *arg);

typedef struct ddsrt_avl_node {
  struct ddsrt_avl_node *cs[2]; /* 0 = left, 1 = right */
  struct ddsrt_avl_node *parent;
  int height;
} ddsrt_avl_node_t;

#define DDSRT_AVL_TREEDEF_FLAG_INDKEY 1
#define DDSRT_AVL_TREEDEF_FLAG_R 2
#define DDSRT_AVL_TREEDEF_FLAG_ALLOWDUPS 4

typedef struct ddsrt_avl_treedef {
#if defined(__cplusplus)
  ddsrt_avl_treedef() {}
#endif
  size_t avlnodeoffset;
  size_t keyoffset;
  union {
    ddsrt_avl_compare_t comparekk;
    ddsrt_avl_compare_r_t comparekk_r;
  } u;
  ddsrt_avl_augment_t augment;
  uint32_t flags;
  void *cmp_arg; /* for _r variant */
} ddsrt_avl_treedef_t;

typedef struct ddsrt_avl_ctreedef {
  ddsrt_avl_treedef_t t;
} ddsrt_avl_ctreedef_t;

typedef struct ddsrt_avl_tree {
  ddsrt_avl_node_t *root;
} ddsrt_avl_tree_t;

typedef struct ddsrt_avl_ctree {
  ddsrt_avl_tree_t t;
  size_t count;
} ddsrt_avl_ctree_t;

typedef struct ddsrt_avl_path {
  int depth; /* total depth of path */
  int pnodeidx;
  ddsrt_avl_node_t *parent; /* (nodeidx == 0 ? NULL : *(path[nodeidx-1])) */
  ddsrt_avl_node_t **pnode[DDSRT_AVL_MAX_TREEHEIGHT];
} ddsrt_avl_path_t;

typedef struct ddsrt_avl_ipath {
  ddsrt_avl_path_t p;
} ddsrt_avl_ipath_t;

typedef struct ddsrt_avl_dpath {
  ddsrt_avl_path_t p;
} ddsrt_avl_dpath_t;

typedef struct ddsrt_avl_iter {
  const ddsrt_avl_treedef_t *td;
  ddsrt_avl_node_t *right;
  ddsrt_avl_node_t **todop;
  ddsrt_avl_node_t *todo[1 + DDSRT_AVL_MAX_TREEHEIGHT];
} ddsrt_avl_iter_t;

typedef struct ddsrt_avl_citer {
  ddsrt_avl_iter_t t;
} ddsrt_avl_citer_t;

/* avlnodeoffset and keyoffset must both be in [0,2**31-1] */
#define DDSRT_AVL_TREEDEF_INITIALIZER(avlnodeoffset, keyoffset, comparekk_, augment) \
  { (avlnodeoffset), (keyoffset), {.comparekk = (comparekk_)}, (augment), 0, 0 }
#define DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY(avlnodeoffset, keyoffset, comparekk_, augment) \
  {                                                                                         \
    (avlnodeoffset), (keyoffset), {.comparekk = (comparekk_)}, (augment),                   \
        DDSRT_AVL_TREEDEF_FLAG_INDKEY, 0                                                    \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk_, augment) \
  {                                                                                            \
    (avlnodeoffset), (keyoffset), {.comparekk = (comparekk_)}, (augment),                      \
        DDSRT_AVL_TREEDEF_FLAG_ALLOWDUPS, 0                                                    \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk_, \
                                                       augment)                              \
  {                                                                                          \
    (avlnodeoffset), (keyoffset), {.comparekk = (comparekk_)}, (augment),                    \
        DDSRT_AVL_TREEDEF_FLAG_INDKEY | DDSRT_AVL_TREEDEF_FLAG_ALLOWDUPS, 0                  \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_R(avlnodeoffset, keyoffset, comparekk_, cmparg, augment) \
  {                                                                                            \
    (avlnodeoffset), (keyoffset), {.comparekk_r = (comparekk_)}, (augment),                    \
        DDSRT_AVL_TREEDEF_FLAG_R, (cmparg)                                                     \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_R(avlnodeoffset, keyoffset, comparekk_, cmparg, \
                                               augment)                                      \
  {                                                                                          \
    (avlnodeoffset), (keyoffset), {.comparekk_r = (comparekk_)}, (augment),                  \
        DDSRT_AVL_TREEDEF_FLAG_INDKEY | DDSRT_AVL_TREEDEF_FLAG_R, (cmparg)                   \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk_, cmparg, \
                                                  augment)                                      \
  {                                                                                             \
    (avlnodeoffset), (keyoffset), {.comparekk_r = (comparekk_)}, (augment),                     \
        DDSRT_AVL_TREEDEF_FLAG_R | DDSRT_AVL_TREEDEF_FLAG_ALLOWDUPS, (cmparg)                   \
  }
#define DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk_, \
                                                         cmparg, augment)                      \
  {                                                                                            \
    (avlnodeoffset), (keyoffset), {.comparekk_r = (comparekk_)}, (augment),                    \
        DDSRT_AVL_TREEDEF_FLAG_INDKEY | DDSRT_AVL_TREEDEF_FLAG_R |                             \
            DDSRT_AVL_TREEDEF_FLAG_ALLOWDUPS,                                                  \
        (cmparg)                                                                               \
  }

/* Not maintaining # nodes */

DDS_EXPORT void ddsrt_avl_treedef_init(ddsrt_avl_treedef_t *td,
                                       size_t avlnodeoffset,
                                       size_t keyoffset,
                                       ddsrt_avl_compare_t comparekk,
                                       ddsrt_avl_augment_t augment,
                                       uint32_t flags) ddsrt_nonnull((1, 4));
DDS_EXPORT void ddsrt_avl_treedef_init_r(ddsrt_avl_treedef_t *td,
                                         size_t avlnodeoffset,
                                         size_t keyoffset,
                                         ddsrt_avl_compare_r_t comparekk_r,
                                         void *cmp_arg,
                                         ddsrt_avl_augment_t augment,
                                         uint32_t flags) ddsrt_nonnull((1, 4));

DDS_EXPORT void ddsrt_avl_init(const ddsrt_avl_treedef_t *td,
                               ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_free(const ddsrt_avl_treedef_t *td,
                               ddsrt_avl_tree_t *tree,
                               void (*freefun)(void *node)) ddsrt_nonnull((1, 2));
DDS_EXPORT void ddsrt_avl_free_arg(const ddsrt_avl_treedef_t *td,
                                   ddsrt_avl_tree_t *tree,
                                   void (*freefun)(void *node, void *arg),
                                   void *arg) ddsrt_nonnull((1, 2));

DDS_EXPORT void *ddsrt_avl_root(const ddsrt_avl_treedef_t *td,
                                const ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_root_non_empty(const ddsrt_avl_treedef_t *td,
                                          const ddsrt_avl_tree_t *tree)
    ddsrt_nonnull_all ddsrt_attribute_returns_nonnull;
DDS_EXPORT void *ddsrt_avl_lookup(const ddsrt_avl_treedef_t *td,
                                  const ddsrt_avl_tree_t *tree,
                                  const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_ipath(const ddsrt_avl_treedef_t *td,
                                        const ddsrt_avl_tree_t *tree,
                                        const void *key,
                                        ddsrt_avl_ipath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_dpath(const ddsrt_avl_treedef_t *td,
                                        const ddsrt_avl_tree_t *tree,
                                        const void *key,
                                        ddsrt_avl_dpath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_pred_eq(const ddsrt_avl_treedef_t *td,
                                          const ddsrt_avl_tree_t *tree,
                                          const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_succ_eq(const ddsrt_avl_treedef_t *td,
                                          const ddsrt_avl_tree_t *tree,
                                          const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_pred(const ddsrt_avl_treedef_t *td,
                                       const ddsrt_avl_tree_t *tree,
                                       const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_lookup_succ(const ddsrt_avl_treedef_t *td,
                                       const ddsrt_avl_tree_t *tree,
                                       const void *key) ddsrt_nonnull_all;

DDS_EXPORT void ddsrt_avl_insert(const ddsrt_avl_treedef_t *td,
                                 ddsrt_avl_tree_t *tree,
                                 void *node) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_delete(const ddsrt_avl_treedef_t *td,
                                 ddsrt_avl_tree_t *tree,
                                 void *node) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_insert_ipath(const ddsrt_avl_treedef_t *td,
                                       ddsrt_avl_tree_t *tree,
                                       void *node,
                                       ddsrt_avl_ipath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_delete_dpath(const ddsrt_avl_treedef_t *td,
                                       ddsrt_avl_tree_t *tree,
                                       void *node,
                                       ddsrt_avl_dpath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_swap_node(const ddsrt_avl_treedef_t *td,
                                    ddsrt_avl_tree_t *tree,
                                    void *oldn,
                                    void *newn) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_augment_update(const ddsrt_avl_treedef_t *td,
                                         void *node) ddsrt_nonnull_all;

DDS_EXPORT int ddsrt_avl_is_empty(const ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT int ddsrt_avl_is_singleton(const ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;

DDS_EXPORT void *ddsrt_avl_find_min(const ddsrt_avl_treedef_t *td,
                                    const ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_find_max(const ddsrt_avl_treedef_t *td,
                                    const ddsrt_avl_tree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_find_pred(const ddsrt_avl_treedef_t *td,
                                     const ddsrt_avl_tree_t *tree,
                                     const void *vnode) ddsrt_nonnull((1, 2));
DDS_EXPORT void *ddsrt_avl_find_succ(const ddsrt_avl_treedef_t *td,
                                     const ddsrt_avl_tree_t *tree,
                                     const void *vnode) ddsrt_nonnull((1, 2));

DDS_EXPORT void ddsrt_avl_walk(const ddsrt_avl_treedef_t *td,
                               ddsrt_avl_tree_t *tree,
                               ddsrt_avl_walk_t f,
                               void *a) ddsrt_nonnull((1, 2, 3));
DDS_EXPORT void ddsrt_avl_const_walk(const ddsrt_avl_treedef_t *td,
                                     const ddsrt_avl_tree_t *tree,
                                     ddsrt_avl_const_walk_t f,
                                     void *a) ddsrt_nonnull((1, 2, 3));
DDS_EXPORT void ddsrt_avl_walk_range(const ddsrt_avl_treedef_t *td,
                                     ddsrt_avl_tree_t *tree,
                                     const void *min,
                                     const void *max,
                                     ddsrt_avl_walk_t f,
                                     void *a) ddsrt_nonnull((1, 2, 3, 4, 5));
DDS_EXPORT void ddsrt_avl_const_walk_range(const ddsrt_avl_treedef_t *td,
                                           const ddsrt_avl_tree_t *tree,
                                           const void *min,
                                           const void *max,
                                           ddsrt_avl_const_walk_t f,
                                           void *a) ddsrt_nonnull((1, 2, 3, 4, 5));
DDS_EXPORT void ddsrt_avl_walk_range_reverse(const ddsrt_avl_treedef_t *td,
                                             ddsrt_avl_tree_t *tree,
                                             const void *min,
                                             const void *max,
                                             ddsrt_avl_walk_t f,
                                             void *a) ddsrt_nonnull((1, 2, 3));
DDS_EXPORT void ddsrt_avl_const_walk_range_reverse(const ddsrt_avl_treedef_t *td,
                                                   const ddsrt_avl_tree_t *tree,
                                                   const void *min,
                                                   const void *max,
                                                   ddsrt_avl_const_walk_t f,
                                                   void *a) ddsrt_nonnull((1, 2, 3));

DDS_EXPORT void *ddsrt_avl_iter_first(const ddsrt_avl_treedef_t *td,
                                      const ddsrt_avl_tree_t *tree,
                                      ddsrt_avl_iter_t *iter) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_iter_succ_eq(const ddsrt_avl_treedef_t *td,
                                        const ddsrt_avl_tree_t *tree,
                                        ddsrt_avl_iter_t *iter,
                                        const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_iter_succ(const ddsrt_avl_treedef_t *td,
                                     const ddsrt_avl_tree_t *tree,
                                     ddsrt_avl_iter_t *iter,
                                     const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_iter_next(ddsrt_avl_iter_t *iter) ddsrt_nonnull_all;

/* Maintaining # nodes */

#define DDSRT_AVL_CTREEDEF_INITIALIZER(avlnodeoffset, keyoffset, comparekk, augment) \
  { DDSRT_AVL_TREEDEF_INITIALIZER(avlnodeoffset, keyoffset, comparekk, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_INDKEY(avlnodeoffset, keyoffset, comparekk, augment) \
  { DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY(avlnodeoffset, keyoffset, comparekk, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, augment) \
  { DDSRT_AVL_TREEDEF_INITIALIZER_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_INDKEY_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, \
                                                        augment)                             \
  { DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_R(avlnodeoffset, keyoffset, comparekk, cmparg, augment) \
  { DDSRT_AVL_TREEDEF_INITIALIZER_R(avlnodeoffset, keyoffset, comparekk, cmparg, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_INDKEY_R(avlnodeoffset, keyoffset, comparekk, cmparg, \
                                                augment)                                     \
  { DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_R(avlnodeoffset, keyoffset, comparekk, cmparg, augment) }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, cmparg, \
                                                   augment)                                     \
  {                                                                                             \
    DDSRT_AVL_TREEDEF_INITIALIZER_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, cmparg,      \
                                              augment)                                          \
  }
#define DDSRT_AVL_CTREEDEF_INITIALIZER_INDKEY_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk,    \
                                                          cmparg, augment)                        \
  {                                                                                               \
    DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY_R_ALLOWDUPS(avlnodeoffset, keyoffset, comparekk, cmparg, \
                                                     augment)                                     \
  }

DDS_EXPORT void ddsrt_avl_ctreedef_init(ddsrt_avl_ctreedef_t *td,
                                        size_t avlnodeoffset,
                                        size_t keyoffset,
                                        ddsrt_avl_compare_t comparekk,
                                        ddsrt_avl_augment_t augment,
                                        uint32_t flags) ddsrt_nonnull((1, 4));
DDS_EXPORT void ddsrt_avl_ctreedef_init_r(ddsrt_avl_ctreedef_t *td,
                                          size_t avlnodeoffset,
                                          size_t keyoffset,
                                          ddsrt_avl_compare_r_t comparekk_r,
                                          void *cmp_arg,
                                          ddsrt_avl_augment_t augment,
                                          uint32_t flags) ddsrt_nonnull((1, 4));

DDS_EXPORT void ddsrt_avl_cinit(const ddsrt_avl_ctreedef_t *td,
                                ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_cfree(const ddsrt_avl_ctreedef_t *td,
                                ddsrt_avl_ctree_t *tree,
                                void (*freefun)(void *node)) ddsrt_nonnull((1, 2));
DDS_EXPORT void ddsrt_avl_cfree_arg(const ddsrt_avl_ctreedef_t *td,
                                    ddsrt_avl_ctree_t *tree,
                                    void (*freefun)(void *node, void *arg),
                                    void *arg) ddsrt_nonnull((1, 2));

DDS_EXPORT void *ddsrt_avl_croot(const ddsrt_avl_ctreedef_t *td,
                                 const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_croot_non_empty(const ddsrt_avl_ctreedef_t *td,
                                           const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup(const ddsrt_avl_ctreedef_t *td,
                                   const ddsrt_avl_ctree_t *tree,
                                   const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_ipath(const ddsrt_avl_ctreedef_t *td,
                                         const ddsrt_avl_ctree_t *tree,
                                         const void *key,
                                         ddsrt_avl_ipath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_dpath(const ddsrt_avl_ctreedef_t *td,
                                         const ddsrt_avl_ctree_t *tree,
                                         const void *key,
                                         ddsrt_avl_dpath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_pred_eq(const ddsrt_avl_ctreedef_t *td,
                                           const ddsrt_avl_ctree_t *tree,
                                           const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_succ_eq(const ddsrt_avl_ctreedef_t *td,
                                           const ddsrt_avl_ctree_t *tree,
                                           const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_pred(const ddsrt_avl_ctreedef_t *td,
                                        const ddsrt_avl_ctree_t *tree,
                                        const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_clookup_succ(const ddsrt_avl_ctreedef_t *td,
                                        const ddsrt_avl_ctree_t *tree,
                                        const void *key) ddsrt_nonnull_all;

DDS_EXPORT void ddsrt_avl_cinsert(const ddsrt_avl_ctreedef_t *td,
                                  ddsrt_avl_ctree_t *tree,
                                  void *node) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_cdelete(const ddsrt_avl_ctreedef_t *td,
                                  ddsrt_avl_ctree_t *tree,
                                  void *node) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_cinsert_ipath(const ddsrt_avl_ctreedef_t *td,
                                        ddsrt_avl_ctree_t *tree,
                                        void *node,
                                        ddsrt_avl_ipath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_cdelete_dpath(const ddsrt_avl_ctreedef_t *td,
                                        ddsrt_avl_ctree_t *tree,
                                        void *node,
                                        ddsrt_avl_dpath_t *path) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_cswap_node(const ddsrt_avl_ctreedef_t *td,
                                     ddsrt_avl_ctree_t *tree,
                                     void *oldn,
                                     void *newn) ddsrt_nonnull_all;
DDS_EXPORT void ddsrt_avl_caugment_update(const ddsrt_avl_ctreedef_t *td,
                                          void *node) ddsrt_nonnull_all;

DDS_EXPORT int ddsrt_avl_cis_empty(const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT int ddsrt_avl_cis_singleton(const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT size_t ddsrt_avl_ccount(const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;

DDS_EXPORT void *ddsrt_avl_cfind_min(const ddsrt_avl_ctreedef_t *td,
                                     const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_cfind_max(const ddsrt_avl_ctreedef_t *td,
                                     const ddsrt_avl_ctree_t *tree) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_cfind_pred(const ddsrt_avl_ctreedef_t *td,
                                      const ddsrt_avl_ctree_t *tree,
                                      const void *vnode) ddsrt_nonnull((1, 2));
DDS_EXPORT void *ddsrt_avl_cfind_succ(const ddsrt_avl_ctreedef_t *td,
                                      const ddsrt_avl_ctree_t *tree,
                                      const void *vnode) ddsrt_nonnull((1, 2));

DDS_EXPORT void ddsrt_avl_cwalk(const ddsrt_avl_ctreedef_t *td,
                                ddsrt_avl_ctree_t *tree,
                                ddsrt_avl_walk_t f,
                                void *a) ddsrt_nonnull((1, 2, 3));
DDS_EXPORT void ddsrt_avl_cconst_walk(const ddsrt_avl_ctreedef_t *td,
                                      const ddsrt_avl_ctree_t *tree,
                                      ddsrt_avl_const_walk_t f,
                                      void *a) ddsrt_nonnull((1, 2, 3));
DDS_EXPORT void ddsrt_avl_cwalk_range(const ddsrt_avl_ctreedef_t *td,
                                      ddsrt_avl_ctree_t *tree,
                                      const void *min,
                                      const void *max,
                                      ddsrt_avl_walk_t f,
                                      void *a) ddsrt_nonnull((1, 2, 3, 4, 5));
DDS_EXPORT void ddsrt_avl_cconst_walk_range(const ddsrt_avl_ctreedef_t *td,
                                            const ddsrt_avl_ctree_t *tree,
                                            const void *min,
                                            const void *max,
                                            ddsrt_avl_const_walk_t f,
                                            void *a) ddsrt_nonnull((1, 2, 3, 4, 5));
DDS_EXPORT void ddsrt_avl_cwalk_range_reverse(const ddsrt_avl_ctreedef_t *td,
                                              ddsrt_avl_ctree_t *tree,
                                              const void *min,
                                              const void *max,
                                              ddsrt_avl_walk_t f,
                                              void *a) ddsrt_nonnull((1, 2, 3, 4, 5));
DDS_EXPORT void ddsrt_avl_cconst_walk_range_reverse(const ddsrt_avl_ctreedef_t *td,
                                                    const ddsrt_avl_ctree_t *tree,
                                                    const void *min,
                                                    const void *max,
                                                    ddsrt_avl_const_walk_t f,
                                                    void *a) ddsrt_nonnull((1, 2, 3, 4, 5));

DDS_EXPORT void *ddsrt_avl_citer_first(const ddsrt_avl_ctreedef_t *td,
                                       const ddsrt_avl_ctree_t *tree,
                                       ddsrt_avl_citer_t *iter) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_citer_succ_eq(const ddsrt_avl_ctreedef_t *td,
                                         const ddsrt_avl_ctree_t *tree,
                                         ddsrt_avl_citer_t *iter,
                                         const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_citer_succ(const ddsrt_avl_ctreedef_t *td,
                                      const ddsrt_avl_ctree_t *tree,
                                      ddsrt_avl_citer_t *iter,
                                      const void *key) ddsrt_nonnull_all;
DDS_EXPORT void *ddsrt_avl_citer_next(ddsrt_avl_citer_t *iter) ddsrt_nonnull_all;

#if defined(__cplusplus)
}
#endif

#endif /* DDSRT_AVL_H */
