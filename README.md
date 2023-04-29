# orient

[English](./README_en.md)

可以在Linux, macOS与Windows上运行的文件检索工具，含有`find`, `locate`
以及`Everything`的各种功能，外加内容查找。

## 写在前面

倒排索引是 `plocate` 背后的技术，可以在近乎恒定的时间内完成搜索，
**在 `orient v0.4.0` 及更高版本中实现**。  
不幸的是，`v0.4` 没有经过很好的测试，
目前发布的版本是 `v0.3.x`。 此外，自述文件中的所有演示都是在 v0.3.0 上完成的。  
对于通常只有小于三百万个文件的个人计算机，有没有倒排索引并不会造成太大的区别。

*文件内容未编入索引。*但当此仓库达到 512 颗星后，将着手开发文件内容索引。
如果您觉得该项目有用或是有趣，请点个star吧！  
（有关此内容的更多信息详见[此处](docs/TODO.md)。）

本自述文件是关于 CLI 应用程序的。 对于 GUI 前端，请参见
[SearchEverywhere](https://github.com/cxxsucks/SearchEverywhere).
(也挺不稳定的)

## 独特的功能

### 轻量级

唯一的运行时依赖项是 PCRE2，静态的可执行文件大小仅为 2.5MiB（stripped）。

### Linux、macOS 和 **Windows**

macOS 和 Windows 上的第一个自由软件文件索引器！😜

- 忽略欺骗 `find` 的 macOS firmlink, 例如 `/System/Volumes/Data`
- 在 Windows 上使用原生 UTF-16，通过模板对代码进行最少的修改，实现性能和简洁性
- 像处理 Unix 目录一样处理 Windows 驱动器（当然，“\\”是分隔符）

分别在 macOS 和 Windows 上的截图：
![orie_mac](docs/md_pics/orie_mac.png)  
![orie_win](docs/md_pics/orie_win.png)  

### 无需管理员权限的多线程(仅非机械硬盘)索引更新

与 `locate` 不同，重建文件系统索引**不**需要root 权限（或 Windows 上的管理员）。
也没有setugid。  
对于SSD，设置线程池用于并发目录读取，大大加快读取速度。  
对于HDD，应禁用此功能，因为多线程 IO 由于其旋转特性而不会提供明显的性能提升。  
可以对路径切换是否启用多线程。

图中显示 `orient` 可以在 1 秒内扫描 810000 个文件，*虽然已经清理了页面缓存*，
但使用的磁盘是相当高端的，因此不要太当真。
![updatedb](docs/md_pics/updatedb.png)  

### 快速模糊和内容匹配

与`updatedb`一样，同样的线程池也用于内容匹配。
从 75000 个文件的 Linux 内核源代码树中模糊匹配“hello world”在缓存删除时花费了
5.5 秒，在使用缓存时花费了 1.5 秒。
![contentMatch](docs/md_pics/content_match.png)  
（期望别那么高；使用设备是16x Intel i7 11800H 和 NVMe SSD）
> 在 Windows 上，内容匹配**明显较慢**，这是由 UTF8 到 UTF16 转换
> 缺乏有效的内核内存映射 (`mmap(2)`) 以及 Windows 臃肿、低效的综合影响。

~~Goodbye `find ... | xargs grep ...` and `find ... -a -exec grep ...`~~

### 类似 `find` 的语法

如下所示，`orient` 还实现了 **大部分** `find` 的
匹配，让用户更容易熟悉使用`find`的现有经验，同时也增加了应用程序的多功能性。
![查找](docs/md_pics/find.png)

### 匹配父目录或子文件

不像 `Everything` 硬编码父匹配只匹配字符串，
在 `orient` 中，`-updir -downdir` 可以应用于任何谓词。

此外，`-downdir` 的开销几乎为 0，而 `-updir` 通过缓存最近的匹配项使搜索**更快**。
匹配父目录：
![updir](docs/md_pics/updir.png)  
匹配子文件：
![downdir](docs/md_pics/downdir.png)

## 比较

|              | Linux | Windows | macOS | Android |   License   |
|:------------:|:-----:|:-------:|:-----:|:-------:|:-----------:|
| `Everything` | 👎NO  |  👍YES  | 👎NO  |  👎NO   | Proprietary |
|    `find`    | 👍YES |  👎NO   | 👎NO  |  👎NO   |    GPLv3    |
|  `fsearch`   | 👍YES |  👎NO   | 👎NO  |  👎NO   |    GPLv2    |
|   `locate`   | 👍YES |  👎NO   | 👎NO  |  👎NO   |    GPLv3    |
|   `orient`   | 👍YES |  👍YES  | 👍YES |  👎NO   |    GPLv3    |

Continued Table  
|              | `-and -or` | Invert Index | Match Parent |  GUI  |  CLI  |
|:------------:|:----------:|:------------:|:------------:|:-----:|:-----:|
| `Everything` |   👍YES    |     👎NO     |   Partial    | 👍YES |  😕   |
|    `find`    |   👍YES    |     👎NO     |     👎NO     | 👎NO  | 👍YES |
|  `fsearch`   |   👍YES    |     👎NO     |   Partial    | 👍YES | 👎NO  |
|   `locate`   |   👎NO     |     👍YES    |     👎NO     | 👎NO  | 👍YES |
|   `orient`   |   👍YES    |     👍YES    |     👍YES    | 👍YES | 👍YES |

注意：

- *部分*匹配父项和子项意味着虽然它们确实提供了匹配文件的父项或目录的子项的选项，
    但此类搜索仅限于字符串匹配而不是应用程序的所有功能。
- `Everything` CLI 似乎对所有结果都进行了*美化*，使其很难与其他工具结合使用，
    因此那里放了个疑惑的表情。
- `Everything`是专有的，只有SDK！我们亲爱的领导人理查德·斯托曼会把它砸得稀巴烂！(手动狗头)
    > 更糟糕的是，`Everything SDK` 充满了全局状态。🤮

## 快速入门

### 使用类似 `find` 的语法

熟悉 `find` 的用户可以使用 `orient` 的 `find` 兼容语法快速上手，
如`-regex`、`-lname`、`-okdir`等。  
请注意，`orient` 谓词有时是其对应的 `find` 的超集，例如 `-quit`
可选地接受一个整数参数，这意味着在退出之前可以产生多少结果。 它的默认值为 1，
因此当使用不带参数的 `-quit` 时，它与使用 `find` 时没有区别。

对于特定于 `orient` 的谓词，仅提供 `orient` 风格的语法，见下文。

```sh
# mp3 or mp4 file excluding under hidden dirs
find ~ \( -name ".*" -a -prune -a -false \) -o -name "*.mp[34]"
orient ~ \( -name ".*" -a -prune -a -false \) -o -name "*.mp[34]"

# Ask user to whether to show its realpath when a symlink found in /usr
# until user inputs "yes" (realpath executes)
find /usr -type l -a -okdir realpath \{\} \; -a -quit
orient /usr -type l -a -okdir realpath \{\} \; -a -quit
# Until 2 user inputs "yes"
orient /usr -type l -a -okdir realpath \{\} \; -a -quit 2
# Even better, -quitmod
orient /usr -quitmod \( -type l -a -okdir realpath \{\} \; \)
# -quit -quitmod has some quirks; see docs/predicates.md
```

## 使用 `orient` 的替代语法

`orient` 没有 `find` 那样多的（独特的）谓词。 相反，`orient` 使用
`-PRED --ARG` 语法，为单个谓词提供多种匹配方案，从而提高代码的可重用性。

- 路径匹配谓词：`-name` `-bregex` `-strstr` `-fuzz`
     参数：`--ignore-case`（`-fuzz` 除外）`--full` `--readlink`
- 内容匹配谓词：`-content-{strstr,fuzz,regex}`
     参数：`--ignore-case`（`-fuzz` 除外）`--blocked` `--allow-binary`
- 文件统计谓词：`-size` `-{a,m,c}{time,min}` `-inum`
     参数：文件名或以“+”或“-”为前缀的整数
- 以及更多...

许多 `find` 兼容谓词实际上是别名，例如 `-lname` 与 `-name --readlink` 相同，  
`-regex` 与 `-bregex --ignore-case` 相同（`b` 代表基本名称）
`-samefile` 就是 `-inode` 的代称，因为 `-inode` 也接受文件名
> 也可以将两种语法混合在一起，但不推荐，例如 `-iname --full` 或 `-anewer +5`

```sh
# Find C source files containing "hello"; orient style only
orient / -content-strstr hello -name "*.c"

# Many `find` style predicates are actually aliases, ex:
orient / -iname "*.cpp" # find style
orient / -name --ignore-case "*.cpp" # orient style

# Assuming /home/a/b links to /var/tmp, then
# all the following 3 lines matches /home/a/b
# `-lname` is identical to `-name --readlink`  
orient / -lname "*tmp"
orient / -name --readlink "*tmp"
# `orient` style is more versatile:
orient / -bregex --readlink 'tmp$' # No `find` style alternative
```

### 修饰符

通过引入修饰谓词，可以在传播到其他谓词之前“做某事”，这正是 `-updir` 和 `-downdir`
所做的：它们匹配文件的父目录和目录的子文件。  
使用修饰符，`-updir -downdir` 可以应用于 `orient` 中的任何谓词，
这与 `Everything` 不同，后者将父匹配硬编码为仅字符串匹配。  
*任何* 谓词包括递归使用 `-updir -downdir` 本身。

此外，`-downdir` 的开销几乎为 0，而 `-updir` 通过缓存最近的匹配项使搜索**更快**。

其他的修饰符包括`-prunemod`、`-quitmod`和`-not`。 例如：

```sh
# Find bin/gcc*
orient / -updir -name "gcc*" -a -executable
# Find bin/gcc* or bin/clang*
orient / -updir \( -name "gcc*" -o -name "clang*" \) -a -executable

# Find git repositories, first level only
orient / -downdir \( -name .git -a -type d \) -a -prune
# Must use -exec test on find and is extremely slow
find .. -type d -a -exec test -d '{}/.git' \; -a -print -a -prune

# .cc files under src directory of a git repository
orient / -updir \( -name src -a -updir -downdir -name .git \) -name "*.cc"
```

## 安装

由于该应用程序是命令行界面，只需获取系统的可执行文件，它就应该可以运行。
> 在 Linux 上，`-user -nouser -group -nogroup` 需要 glibc 才能工作。

不幸的是，macOS ARM 版本缺失，因为苹果机太贵了🫥。请在Issue或Discussion
中报告在该机器上是否可用。
> 目前测试太少，不是很适合发布到发行版。  
> 可能首先发布到 Arch AUR btw, 顺便说一下。

### 从源代码构建

现在仍在发布的早期阶段，建议从源代码构建。(`v0.4.0`暂时必须源码编译)  
也可以直接编译（更不稳定的）GUI
[SearchEverywhere](https://github.com/cxxsucks/SearchEverywhere)，
编译GUI的时候`orient` CLI也被顺带着编译了。  
构建所需依赖：

- CMake
- PCRE2
- rapidfuzz
- GoogleTest (Test Only)

除了 `CMake` 本身之外，所有依赖项都可以由 CMake 自动下载。
如果您已经在系统上安装了其中一部分，通过切换下面的这些配置选项，也可以使用已安装的。

配置选项：

- `ORIE_TEST`：构建 GoogleTest 测试套件
- `ORIE_SYSTEM_PCRE2`：使用系统 PCRE2 库而不是编译新库。
- `ORIE_LINK_STATIC`：静态链接 orient 可执行文件
- `ORIE_SYSTEM_RAPIDFUZZ`：使用系统 rapidfuzz 库（仅标头）

将下面的 `OPTION` 替换为您启用的选项，然后运行以下命令：

```sh
git clone https://github.com/cxxsucks/orient.git
cd orient; mkdir build; cd build
cmake -DOPTION1=ON -DOPTION2=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

## 注意事项

### 默认多线程读取

默认配置硬编码了几个扫描，并在所有起点上启用多线程读取，这对于机械硬盘来说不是最佳选择。
如果您碰巧使用 HDD，请在第一次运行 `orient` 时执行以下操作：

1. 运行 `orient -updatedb`
2. 立即用 `Ctrl-C` 打断
3. 打开 `~/.config/orie/default.txt` 或 `%APPDATA%\.orie\default.txt`
4. 查看 `ROOT` 之后的路径，如果这些路径中的任何一条实际上不在 SSD 上，则删除 `SSD` 字段。
5. 如果有任何未列出的硬盘根路径，请写入 `ROOT "/path/to/mountpoint"` 或直接用
    `IGNORED "/path/to/mountpoint"` 不对其进行索引。

在 Linux 上，`/sys/block/sda/queue/rotational` 能够判断某个硬盘是否是机械式的，
不幸（但意料之中）的是macOS 和 Windows 没有。  
在未来的版本中，将从 `/etc/mtab` 和 `/sys/.../rotational` 获取挂载点，
它们将被用来在 Linux 和 macOS 上自动配置根路径。  
> 苹果真酷！ 他们一定会为他们的 MacBook 配备世界上最好的硬盘，
> 这些硬盘绝对不可能是机械的！

### 未测试的功能

`exec` 系列谓词已在 Windows 上实现，但未经测试。
稍有软件开发常识的人都可以看出，这个未经测试的功能即使不会把程序搞崩，
也肯定会错得离谱。  
除了`exec`之外，其他未测试的功能及其原因都能在
[TODO 列表](docs/TODO.md) 中找到。

### **未实现**的`find`里的功能

主要是全局选项：

- `-context`（SELinux 上下文）
- `-printf -fprintf -ls -fls`（格式打印）
-`-newerXY`
-`-mindepth``-maxdepth`
- `-H -L -P`（是否跟随符号链接）
-`-D`（调试选项）
- `-O`（优化级别）
     > `orient` 有自己的优化器，类似于 `find -O3`
- `-regextype`（硬编码 PCRE2）
-`-警告-nowarn`
- `-d -depth`（深度优先搜索）
     > `orient`只能根据索引搜索。
     > 与 `find` 不同，`-delete` 不受影响。
-`-files0-from`
- `-mount -xdev -xautofs`（不要进入挂载点）
- `-help -version`

## 未来工作

文档将是最近工作的中心。  
错误报告和功能请求仍可直接发到这个存储库的 Issue 里面。  
更多工作详见[TODO列表](docs/TODO.md)。

## 致谢

- [dirent](https://github.com/tronkko/dirent)：对 Windows 的 Unix `dirent` 适配
    > 在这里对符号链接和（伪造的）设备、套接字支持进行了大量修改，
    > 因此它直接放在源代码中而不是子模块中。
- [PCRE2](https://github.com/PCRE2Project/pcre2) 正则表达式
- [rapidfuzz-cpp](https://github.com/maxbachmann/rapidfuzz-cpp):
     **仅标头**的模糊字符串匹配库
