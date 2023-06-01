# fools2023

[TheZZAZZGlitch's April Fools Event 2023](https://zzazzdzz.github.io/fools2023/).

A small CTF challenge, to fill the gap between 2022 and 2024 *(and 2024 will likely be the final large-scale Fools Event, due to adult life being unreasonably tiring and boring)*.

Since there was a small demand for preserving the challenge, and there was little to no work required from me to do it, I decided on following through and archiving it.

# Disclaimer

Since Fools2023 was rushed beyond belief, take extreme caution while browsing the source code. Severe cringing and pain might occur, especially for experienced developers. If you or a loved one get diagnosed with notlikethisisis, receiving financial compensation might not be possible.

In fact, if you want to try the challenge yourself, using [fan-made disassemblies and VM implementations](https://github.com/OdnetninI/zzazz-2023-server) might actually be better than using the originals.

# Quick instructions

## GLVM machines

```
gcc monitor.c -o port13337
gcc authmonitor.c -o port13338
gcc infsystem.c -o port13339
```

During the original event, these services were exposed using [xinetd](https://linux.die.net/man/8/xinetd). Make sure to run the binaries while having their corresponding filesystem trees (`*_fs` folders) in their working directory.

## Challenges

GLVM code for actual challenges was compiled using [customasm](https://github.com/hlorenzi/customasm). Both source code and pre-compiled binaries are included in `challenges/`.
