# TODO list for `orient` library and CLI

## Indexing File Content

Now that an inverted index on file namehas been implemented, the next
major leap would be in content searching by indexing some, but (evidently)
not all, file content. A user would be able to select which files are to be
indexed by specifying both starting directories and file names (or even
complete `orient` expressions). Only text files less than 100KiB are indexed.  
Similar to file name inverted index, trigrams would be the main indexing
method, mapping file contents directly to file names.

This is easier said than done, and if `orient` proved to be useless to
its users, implementing such technique is not worthwhile, to say the least.
Therefore, when this repository (`orient`) reached 512 starts, which is a
decent indication of its usefulness towards normal users, development on
indexing file content would commence. *Smash the star button if the repo*
*interests you or proved useful for you.*

## QOL changes being worked on

The TODOs on the following unordered list is, well, unordered.  
For personal reasons, documentation and fixing bugs is my current focus.  
Feel free to start an issue on either bugs or feature requests.

- [x] `-prunemod` modifier
    > Always true, but only runs its child if the file is a directory.  
    > If the child returns true, skip entering this directory.  
    > Improves readability compared to `find`-style `-prune`
- [x] `-quit` `-quitmod`
    > Takes an integer `n` as argument, 1 by default.  
    > For `-quit`, after running `n` times, search stops prematurely.  
    > For `quitmod`, stop when its child returns true `n` times.
    > When used without an argument, `-quit` is identical to that in `find`.  
    > May not work well with multithreaded `-content-*`
- [] `-depth` as a predicate
    > Takes an `+n`-style argument like `-size`.  
    > More versatile than `find`'s `-maxdepth -mindepth`, which are global options.
- [] SELinux `-context`
- [x] Fuzzy match `-fuzzy` (file name only)
    > `maxbachmann/rapidfuzz-cpp` looks appealing  
    > Can be disabled compile-time.
    > Accepts similarity threshold as a float argument between 0~1.
- [] Windows `-exec` test
    > Exploritory tests suffice, since Linux is the main platform.
- [] Better `app::os_defult` for Linux and macOS
    > Use `/proc/mount` and `/etc/mtabs` for mountpoints rather
    > than hard-code them.  
    > Determine SSD/HDD with `/sys/block/sda/queue/rotational`.
- [x] Use `std::vector<std::bytes>` in saving dumped data
    > Currently `std::shared_ptr<std::byte[]>`, which is not
    > "authentic" C++17 and some outdated compiler complains.  
    > Can be solved with custom deleter.  
    > After this, the code will compile on macOS Catalina and
    > the Android NDK downloaded by `Qt Creator`.
- [] `-xor -nor -xnor -nand`
    > Already implemented in `orie::pred_tree::cond_node`, but
    > remain unused and untested.
- [] `-downdir` match number of children matched
    > Already implemented but untested, therefore not listed as a feature.
- [] Use a non-bloated argument parser
    > Reasons stated in `main` function
- [x] Remove `app::main`, but add an overload of `app::get_jobs`
    taking `argc argv`
- [] Stream operator for `fs_data_iter`
    > Good for handling trailing `/` on Windows.
- [] `--help` `--version`

## Low-priority TODOs

- [] Windows `-exec` unit test
    > Windows provides too little shell utility :(
- [] `-find`-like `-files0-from`
- [] `-find`-like `-H -L -P`
- [] `-find`-like `-mindepth -maxdepth`
    > Not considered until a non-bloated arg parser is found,  
    > as stated in `main` function.
- [] `-newerXY`
- [] `-printf` `-ls` `-fprintf` `-fls`
    > These preds/actions are so complex that time spent on
    > implementing them could be better spent on more useful
    > preds or code refactoring.  
    > Can be implemented via a format method of `fs_data_iter`
- [] Build a .deb
- [] Build a .rpm
- [] Arch `pkgbuild`
- [] Gentoo `ebuild`
    > Currently this app is too little-tested to release to a distribution.
    > May release to Arch AUR first btw.

## Some ideas & unplanned works

### Separate `find` Compat Script

`find`'s many global options are utilities unrelated to file searching
itself, and are verbose to implement in C++, but quite easy in bash, like
`-files0-from` which is just tokenizing a file and appending to start
paths.  
Others, like `-mount`, can be emulated with `-prunemod -path /mntpoint -a ( )`.
`-daystart` remains an obstacle requiring extensive changes.

### Root Path Auto Generation

Default config generation hard-codes some starting points and enables
multithreaded read on all of them, which is suboptimal for rotational
hard disks.  

On Linux, `/sys/block/sda/queue/rotational` provide insights on whether
a disk is rotational, which macOS and Windows unfortunately (but
unexpectedly) do not have.  
In a future release root points will be aquired from `/etc/mtab` and
`/sys/.../rotational`, which auto-configure root paths on Linux and macOS.
> Apple is so cool! They must have equipped their MacBooks with the best
> hard disks in the world and is definitely not rotational!

A bash script may do that job, and `app::os_default` can be simplified to
an `execvp` call, but direct implementation in `app::os_default` is also
feasible and easier for (especially `SearchEverywhere`) end users.

### Using `CLucene`

`Clucene` is a feature-rich performant general-purpose indexer. It can
index anything from filename to full content. At first glance, indexing
cotent with `CLucene` is a rather intriguing idea, until you realize
throwing filename, size, timestamps and other stats all into `CLucene`
is even more intriguing. What's the point of all the indexing done in
`orient` then?  
*This does not necessarily make it a bad idea* though. I decided not to
try it simply because I have always been using others' packages and
determined to make my hobby project **mostly original**.  
If this repo does not perform well, I may just take this "clever" approach.
The "clever" project will be called `cleverywhere` or `clevery` LOL.

### Async IO

Current IO strategy is multiple synchorous IO done in a thread pool, which
greatly boosts efficiency on SSDs only. For HDDs though, mutithreading fails
to work, and only asynchorous IO may come to rescue.  
*However*, when `updatedb`ing a HDD, virtually all time are spenr on spinning
the disk; the works done by CPU is barely noticeable. Even async IO could only
offer little improvement, if any.
