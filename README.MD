## 前言
作为一名 java 程序猿，java.util 和 java.lang 包下的源码是必须要看，而且是必须要看懂的。前者包含了许多在工程中经常使用的类，如 ArrayList，LinkedList，HashMap，LinkedHashMap等。 List 即线性表，这部分比较简单，网上也有很多这部分的源码解析。Map 用来保存键值对的映射关系，相对比较复杂，但也绝对不是天书那种难度的。

看大佬的代码是提高姿势水平的好办法。如果没办法静下心来，那就试试自己造轮子，这个过程会无形中驱使你仔细阅读源码。


## HashMap 简介
* HashMap 是 Map 接口的实现类，它利用 key 的 hash 作为键，可以非常快地完成增改删查操作。在 hash 较为均匀，或者负载因子较小，容量较大时，可以实现 O(1) 的时间复杂度；如果 hash 碰撞十分严重，或负载因子较大，容量较小，时间复杂度会转为 O(logN)
* 需要注意的是，java 的 HashMap 之前是使用 HashTable 实现的，这意味着在 hash 高度碰撞时，时间复杂度会恶化为 O(n)。自 Java8 开始，加入了另一种数据结构——红黑树（RBTree），它保证了 O(logN) 的时间复杂度
* 简短洁说，HashMap 的工作原理是，当插入新元素时，会先根据 Object.hashCode() 计算出 hashCode，利用 hashCode 定位到数组下标，然后会根据链表和红黑树进行不同的处理。插入完成后需要根据负载因子判断是否需要扩容，并进行再散列。这个过程可能会发生链表转红黑树，或是红黑树转链表的处理。移除和查找的话比较简单，和上面类似。


## HashTable
### 核心思想
在线性表中，数组（Array）和链表（List）是两个代表。前者支持随机访问，但插入和删除开销较大；后者只支持顺序访问，但插入和删除非常快。HashTable 可以看作是这两个的组合，它的核心是一个数组，叫做”桶(bucket)“。当要把一个对象放到数组时，只要根据某种规则计算出它的数组下标，就可以在 O(1) 的时间内完成操作。换句话说，我们需要在 Object 和 数组下标 之间搭建一个桥梁。事实上，这个”桥梁“已经有了，那就是 Object.hashCode() 方法。

在实际开发中，如果一个类需要重写 Object.equals() 方法，原则上也需要重写 Object.hashCode() ，因为相同的对象，必须返回相同的 hashCode。为了保证数组下标小于数组长度，一个非常普通的思路就是对数组长度求余。比如现在的数组容量为 8，Object 的 hashCode 分别为 1，9，4，那么其下标就是 1 % 8 = 1，9 % 8 = 1，4 % 8 = 4。然而，hashCode 为 1 和 9 时，对应的是同一个数组下标，这种”不同的对象对应同一个数组下标“的现象叫做”hash的碰撞“，此时，我们就需要在 1 这个下标构建一个链表，以来保存这两个对象。(图随便画的，还请见谅)

![hashtable示意图](https://upload-images.jianshu.io/upload_images/12199876-448f55b60b87e33a.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

上面的思路十分简单，也很自然，不管你信不信，这就是 HashTable 的核心思想。然而，如果你仔细思考，会发现 hash 的均匀性是一个非常重要的问题。如果 Object.hashCode() 很差（比如总返回同一个值），或数组的长度非常短，会引起非常严重的 hash 碰撞。此时，整个数据结构退化为链表，时间复杂度恶化为 O(N)。

### 提高 hash 均匀性
Object.hashCode() 返回的是一个 int 类型，它占用 4 个字节，范围可达 4G 大小。但我们实际使用的数组大小往往远小于这个值。也就是说，在 hashCode 对 数组长度 求余时，高位信息被浪费掉了。
比如 786432，4390913，1，这三个数看起来相差很多，但对 8 求余时，
```
786433 % 8 = 1
4390913 % 8 = 1
1 % 8 = 1
```
原因就是 int 的 高 24 位信息，都被忽略掉了。因此，我们需要先对 hashCode 做一个处理，使其高位信息仍能参与运算。java中是把高 16位和 hashCode 做异或运算。
```
static final int hash(Object key) {
        int h;
        return (key == null) ? 0 : (h = key.hashCode()) ^ (h >>> 16);
    }
```
同样的三个数，经过这样的处理，其数组下标分别为 
```
(786433 ^ (786433 >>> 16)) % 8 = 5
(4390913 ^ (4390913 >>> 16)) % 8 = 2
(1 ^ (1 >>> 16)) % 8 = 1
```
可以看到，现在的碰撞的情况已经好多了。但上面的运算仍然不是很高效，因为求余运算是一个比较慢的过程。在 java 中，数组的长度必须是 2 的整次幂，这样就可以把求余优化为相与运算。比如，x % 16 和 x & (16 - 1) 的值是相同的，但后者的效率要高很多。

### 负载因子、扩容与再散列
从 HashTable 的原理上看，实际上就是用空间换时间。当数组长度过大，hash 分布更均匀，时间更短但浪费空间；当数组长度过小，hash 碰撞增加，时间更长但节省了空间。为了平衡二者，引入了一个概念叫做 ”负载因子(load_factor)“。当已存储元素的个数与数组容量的比值达到这个阈值时，我们就可以认为已经有相当数量的链表存在，就会考虑对数组扩容。在大多数情况下，这个值为 0.75，一般不建议修改。较小的值会加快速度，但占用内存会增大；反之亦然。

在扩容的过程中，由于数组大小变为原来的两倍，就需要重新调整每个元素的下标，这个过程叫做”再散列“。上面我们提到，数组的长度必须为 2 的整次幂，这样在再散列时，就能利用位操作得到更高的性能
```
Node<K,V>[] newTab = (Node<K,V>[])new Node[newCap];
table = newTab;
if (oldTab != null) {
    for (int j = 0; j < oldCap; ++j) {
        Node<K,V> e;
        if ((e = oldTab[j]) != null) {
            oldTab[j] = null;
            if (e.next == null)
                newTab[e.hash & (newCap - 1)] = e;
            else if (e instanceof TreeNode)
                ((TreeNode<K,V>)e).split(this, newTab, j, oldCap);
            else { // preserve order
                Node<K,V> loHead = null, loTail = null;
                Node<K,V> hiHead = null, hiTail = null;
                Node<K,V> next;
                do {
                    next = e.next;
                    if ((e.hash & oldCap) == 0) {
                        if (loTail == null)
                            loHead = e;
                        else
                            loTail.next = e;
                        loTail = e;
                    }
                    else {
                        if (hiTail == null)
                            hiHead = e;
                        else
                            hiTail.next = e;
                        hiTail = e;
                    }
                } while ((e = next) != null);
                if (loTail != null) {
                    loTail.next = null;
                    newTab[j] = loHead;
                }
                if (hiTail != null) {
                    hiTail.next = null;
                    newTab[j + oldCap] = hiHead;
                }
            }
        }
    }
}
```
上面时扩容时的主要逻辑，非常巧妙的是，通过 hash 与 oldCap 相与的结果，将旧的链表拆分为两条新的链表，再保存到新的位置。假设现在的数组长度为 4，5 个对象的 hash 分别为 1，5，9，13，17，他们和 4 相与的结果为
```
1 & 4 = 0
5 & 4 = 4
9 & 4 = 0
13 & 4 = 4
17 & 4 = 0
```
其中，值为 0 的 1，9，17 在新数组的下标和旧下标相同，为 1；而5，13在新数组的下标为 旧数组容量 + 旧下标，即 4 + 1 = 5

### 链表转红黑树
正如上面所分析的，我们对 hash 的高 16 位进行异或，加入负载因子与扩容机制，以提高效率。即使如此，在 Object.hashCode() 很差时，仍会发生频繁的碰撞。举个例子，容量为 16 的数组，负载因子为 0.75，Object.hashCode() 总返回 1，在触发扩容之前，链表长度已经达到 12 了。这时我们就要转链表为红黑树，以进一步提高效率。

## 红黑树
红黑树是一种树形数据结构，通过左旋、右旋、变色等操作保持平衡，其最长路径最多为最短路径的 2 倍。它不像传统的二叉查找树会退化为链表，也不像完全二叉树那样难以保持平衡。这部分较为复杂，这里我推荐几篇较为详细的文章
* [https://www.cnblogs.com/bbvi/p/5576201.html](https://www.cnblogs.com/bbvi/p/5576201.html)
* [https://www.jianshu.com/p/e136ec79235c](https://www.jianshu.com/p/e136ec79235c)

## 代码实现
通过上面的分析，我们能够实现出一个比较简单的 HashMap 工具类。这个部分我们使用 C 语言实现。待稍稍整理再上传到 github
[https://github.com/nlifew/hashmap](https://github.com/nlifew/hashmap)


