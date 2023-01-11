# TODO list for `orient` library and CLI

## An Upcoming Rewrite

TL;DR: Rapid, scalable search on par with `plocate` will be worked on
when this repo reaches 512 stars, or this repo and `SearchEverywhere`
reached 1024 stars in aggregate.  

`plocate` is undeniably the state-of-the-art file searcher, being the
first ever to ship an inverted index designed for filename searching.
However, porting their inverted index is no easy task, as `plocate` is,
after all, only a file name searcher.

Fortunately, there is still hope that such an advanced inverted index
can be added to `orient`. The designing phase of this upcoming `orient`
version has already finished, and can be viewed [here](trigram_design.md).
Even a crude content indexer is [drafted](content_idx.md).  
*Ideally*, this omniscent version would provide a constant-time file
search (cost the same amount of time regardless of your fs size), while
preserving all the rich functionalies already existed. Database file
would also be much smaller, and memory usage spikes during updatedb
is completely gone, giving unprecedented, enterprise level of scalability.

The reality is far from ideal though. Turns out, the works needing to be
done is an **absolute monstrosity** to say the least: zstd random access,
TurboPFor, integration with `-and -or -not`, with multithreaded content
match, with sequential predicates like `-size`, with`-updir -downdir`,
the list goes on. Saying that it would fill my free time up is an
underestimate, and there is a moderate chance that this project would
turn into a full-time job.  
Considering that whether the `find/locate` is useful to the general public
or not is still unknown, plus that I have other jobs to work with, I
decided to play it safe, releasing the *unscalable prototype version*
first. If there are sufficient users or developers found `find/locate`
useful, sacrificing my own free time for a new state-of-the-art file
searcher may be a wise and enticing choice.

To this day, the best metric of *usefulness* for softwares is GitHub stars
imo. This repo (`orient`) being the place most of logic resides is more
empthasized than the GUI repo (`SearchEverywhere`).  
For a repo with moderate dedication put into it like this one, I think
512 stars is an attainable goal, though I have no clue what the number
should be since I have never published anything useful to GitHub.

## QOL changes that are currently being worked on

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
- [] Use `std::shared_ptr<std::bytes>` in saving dumped data
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
    > Reasons stated in `app::main`
- [] Remove `app::main`, but add an overload of `app::get_jobs`
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
    > as stated in `app::main`.
- [] `-newerXY`
- [] `-printf` `-ls` `-fprintf` `-fls`
    > These preds/actions are so complex that time spent on
    > implementing them could be better spent on more useful
    > preds or code refactoring.  
    > Can be implemented as a member function of `fs_data_iter`
- [] Build a .deb
- [] Build a .rpm
- [] Arch `pkgbuild`
- [] Gentoo `ebuild`
    > Currently this app is too little-tested to release to a distribution.
    > May release to Arch AUR first btw.

## Some ideas & unplanned works

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
