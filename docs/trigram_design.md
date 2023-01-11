# Design Doc for Adding Trigrams to `orient`

TODO

## Intro to `plocate`'s trilet indexing

The de-facto `locate` implementation (not `find`, which is still `GNU find`)
is `plocate`, which creates index by what they call *trilets*.  
I was not able to fully understand its source code, but my
understanding is that it maintains a helper index recording
where each 3-letter combination exists on the main index.

Therefore, for `*.mp3`, the helper is first queried for where
the trilets `.mp` and `mp3` exists, which returns 2 arrays,
the intersection of which is where `*.mp3` exists.

A more complicated example is `*usr*kernel*`.  
`kernel` is longer than `usr`, therefore it is used to trim search ranges.  
Intersection of `ker ern rne nel` is first found, then these
specific file names are matched against `*usr*kernel*`,
filtering only those matching the whole expression.

### Problems in adding this feature

First is branching support. `-or -and` are fundamental to
`orient` and `find`, which `plocate` does not provide.

Second is memory issues. `plocate`, which is supposed to
only run a few miliseconds, could read all its index into memory.  
For `SearchEverywhere` (`orient` GUI), they are supposed
to remain in memory. This extra index will cause RAM usage to skyrocket.  
