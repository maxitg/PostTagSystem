[Tag Systems](#tag-systems)

# Tag Systems

*PostTagSystem* implements a [tag system](https://www.wolframscience.com/nks/p93--tag-systems/)
[introduced by *Emil Post* in 1921](https://www.wolframscience.com/nks/notes-3-7--history-of-tag-systems/):

```wl
{{0, _, _} -> {0, 0}, {1, _, _} -> {1, 1, 0, 1}}
```

The interesting feature of this system is that it appears to always reach a fixed point, however it does that in a
number of generations that varies widely with initial state of queue.

This repository implements the evolution of that system in an attempt to prove that it always reaches a fixed point.
