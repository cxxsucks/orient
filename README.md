# orient

A cross-platform filesystem indexer and searcher combining the merits of
`find`, `locate` and `Everything`, optimized for SSDs and personal
computers (\<5M files).

Inverted index, the tech behind `plocate` which make searches done in
constant time, is not implemented due to its sheer amount of workload.
It will be worked on if this repo reached 512 stars, so smash that button
if the app proved useful for you!  
(More about this [here](docs/TODO.md).)

## Unique Features

### Extremely Lightweight

The only runtime dependency is PCRE2, and a fully static executable is
only 2.5MiB in size (stripped).

### Linux, macOS and **Windows**

The First Libre File Indexer Ever on macOS and Windows!😜  

- Ignores macOS firmlink `/System/Volumes/Data` which tricks `find`
- Using native UTF-16 on Windows with minimal modification to code
    (via templates), achieving both performance and simplicity
- Handles Windows drives as if they were Unix directories
    (with '\\' being separator of course)

### Non-root, multithreaded `updatedb` (SSD only)

Unlike `locate`, rebuild filesystem index requires **NO** root permission
(or Administrator on Windows). No setugid either.  
For SSDs, a thread pool is set up for concurrent directory reads,
drastically speeding up read speed.  
For HDDs, this feature shall be disabled, as multithreaded IO gives no
visible performance gains due to their spinning nature.  
Whether multithread updatedb shall be enabled can be toggled per path.

The figure shows that `orient` can scan 870000 files within 1.5 secs with
*cache dropped*, but the disk used is a rather high-end one. Take it with
a grain of salt though.  
[!updatedb](docs/md_pics/updatedb.png)  

### Rapid Fuzzy & Content matching

Like updatedb, the same thread pool is also used for content match.
Fuzzy matching `hello world` from 75000-file Linux kernel took 5.5secs
when cache dropped and 1.5secs with cache.  
[!contentMatch](docs/md_pics/content_match.png)  
(take with a grain of salt; 16x Intel i7 11???H and NVMe SSD)
> On Windows content matching is **significantly slower**, a combined
> effect of UTF8 to UTF16 conversion, lack of efficient kernel memory
> mapping (`mmap(2)`) and the bloated, inefficient nature of Windows.

~~Goodbye `find ... | xargs grep ...` and `find ... -a -exec grep ...`~~

### `find`-like syntax

As shown below, `orient` also implements **large portions of** `find`'s
matches, making users easy to familierze themselves with existing
experience in using `find` while also increasing the app's versatility.  
[!find](docs/md_pics/find.png)

### Match parent dir or child file

Unlike `Everything` which hard code parent match to string matching only,
in `orient`, `-updir -downdir` can be applied to any predicate.

Also `-downdir` is almost 0 overhead and `-updir` make searches **even**
**faster** by caching recent matches. Take a look:
[!updir](docs/md_pics/updir.png)  
[!downdir](docs/md_pics/downdir.png)

## Comparison

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
|   `orient`   |   👍YES    |     👎NO     |     👍YES    | 👍YES | 👍YES |

Notes:

- *Partial*ly matching parent and children means while they do provide
    options to match a file's parent or dir's children, such searches are
    confined to string matches instead of all the app's features.
- `Everything` CLI seems to have all results *prettified*, making it very
    hard to use in combination with other tools, hence the 😕 face.
- `eVeRyThInG` iS pRoPrIeTaRy, OnLy SdK pRoViDeD!!! OuR dEaR lEaDeR rIcHaRd
    StAlLmAn WiLl NuKe it!!!
    > What's worse, `Everything SDK` is filled with global states.🤮

## Quick Start

### Use `find`-like Syntax

Users who are familiar with `find` could jumpstart with `orient`'s `find`
compatible predicates, like `-regex`, `-lname`, `-okdir` and others.  
Note that `orient` predicates are sometimes superset of their `find`
counterparts, like `-quit` optionally accepts a integer argument meaning
how many results can be produced before quitting. Its default value is 1
so that when using `-quit` with no arguments it has no difference from
that in `find`.  

For predicates specific to `orient`, only `orient`-style syntax is
provided, see below.

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

### Use `orient`'s Alternative Syntax

`orient` does not have as much (unique) predicates as `find`. Instead,
`orient` use `-PRED --ARG` syntax, giving multiple matching schemes to
a single predicate, boosting code reuseability. Here are some examples:

- Path match predicates: `-name` `-bregex` `-strstr` `-fuzz`  
    Arguments: `--ignore-case`(except `-fuzz`) `--full` `--readlink`
- Content match predicates: `-content-{strstr,fuzz,regex}`  
    Arguments: `--ignore-case`(except fuzz) `--blocked` `--allow-binary`
- File stat predicates: `-size` `-{a,m,c}{time,min}` `-inum`
    Arguments: File name or integer prefixed by `+` or `-`
- and more...

Many `find` compatible predicates are actually aliases, like  
`-lname` is identical to `-name --readlink`  
`-regex` - `-bregex --ignore-case` (the `b` stands for basename)  
`-samefile` is basically `-inode` since `-inode` also accepts filename  
> It is also possible to mix two syntaxes together, though unrecommended  
> like `-iname --full` or `-anewer +5`

Here are some examples:  
See more on how to use them [here](docs/predicates.md).

```sh
# Find C source files containing "hello"; orient style only
orient / -content-strstr hello -a -name "*.c"

# Many `find` style predicates are actually aliases, ex:
orient / -iname "*.cpp" # find style
orient / -name --ignore-case "*.cpp" # orient style

# Assume /usr/tmp links to /var/tmp
# `-lname` is identical to `-name --readlink`  
orient / -lname "*tmp"
orient / -name --readlink "*tmp"
# `orient` style is more versatile:
orient / -bregex --readlink 'tmp$' # No `find` style alternative
```

### Modifiers

With the introduction of modifier predicates, it is possible to "do
something" before propagating to other preds, which is exactly what
`-updir` and `-downdir` does: they match the parent of files and
children of directories.  
With modifiers, `-updir -downdir` can be applied to any predicate
in `orient`, unlike `Everything` which hard code parent match to string
matching only.

Also `-downdir` is almost 0 overhead and `-updir` make searches **even**
**faster** by caching recent matches.

Some more modifiers include `-prunemod`, `-quitmod` and `-not`.

## Installation

Since the application is a CLI, simply grab the executable of your system
and it should work.
> On Linux, `-user -nouser -group -nogroup` require glibc to work.  

Unfortunately the macOS ARM version is missing since I don't have one such
machine🫥. Feel free to report whether it works on issue or discussion.
> Currently this app is too little-tested to release to a distribution.
> May release to Arch AUR first btw.

### Build From Source

Building from source is recommended in the early stage of release.
Give it a shot!  
Build dependencies:

- CMake
- PCRE2
- rapidfuzz
- GoogleTest (Test Only)

Aside from `CMake`, all dependency can be auto-downloaded by CMake.
Using an installed one is also possible, should you have already
installed some of them.

Configure Options:

- `ORIE_TEST`: Build GoogleTest test suites
- `ORIE_SYSTEM_PCRE2`: Use System PCRE2 Library instead of compiling
    a new one.
- `ORIE_LINK_STATIC`: Statically link orient executable
- `ORIE_SYSTEM_RAPIDFUZZ`: Use System rapidfuzz Library (header only)

If `ORIE_LINK_STATIC` is on, then `ORIE_SYSTEM_PCRE2` shall better be
off, or the app would still dynamically link to PCRE2.  
Replace the `OPTION` below with your enabled options, and run the
following commands:

```sh
git clone https://github.com/ccPlus/orient.git
cd orient; mkdir build; cd build
cmake -DOPTION1=ON -DOPTION1=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

## Caveats

### **Unimplemented** `find` features

Mostly global options:

- `-context` (SELinux context)
- `-printf -fprintf -ls -fls` (Format print)
- `-newerXY`
- `-mindepth` `-maxdepth`
- `-H -L -P` (symlink following global options)
- `-D` (debugopts)
- `-O` (optimize level)
    > `orient` has its own optimizer similar to `find -O3`
- `-regextype` (hardcoded PCRE2)
- `-warn -nowarn`
- `-d -depth` (depth first search)
    > `orient` can only search according to index.
    > `-delete` is not affected though, unlike `find`.
- `-files0-from`
- `-mount -xdev -xautofs` (do not descend into mounts)
- `-help -version`

## Future Work

Documentations would be the center of works recently.  
Bug reports and feature request are still accepted anyway, in GitHub Issues
Tracker of this repository.  
See [TODO List](docs/TODO.md) for details.

## Credits

- [dirent](https://github.com/tronkko/dirent): Unix `dirent` port to Windows
    > Heavily modified here for symlink and (fake) device, socket support,
    > therefore it is directly placed into source instead of module.
- [PCRE2](https://github.com/PCRE2Project/pcre2) Regular Expression
- [rapidfuzz-cpp](https://github.com/maxbachmann/rapidfuzz-cpp):
    Fuzzy string matcher
