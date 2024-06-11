---
title: "HPC - Projet"
subtitle: "`bpftrace`"
author: "Rayane Annen et Felix Breval"
lang: fr
date: \today
geometry: margin=2cm
colorlinks: true
toc: true
header-includes:
    - \usepackage{bm}
    - \usepackage{xcolor}
    - \usepackage{multicol}
    - \definecolor{cgreen}{RGB}{0, 120, 0}
    - \newcommand{\hideFromPandoc}[1]{#1}
    - \hideFromPandoc{
        \let\Begin\begin
        \let\End\end}
---

# Procédure d'installation et de mise en place

## Prérequis

Afin d'utiliser l'outil, il faut un noyau linux compatible, il est recommandé d'avoir noyau en version 4.9 ou plus.

De plus, cmake version 3.20 minimum est nécessaire afin de build le code d'exemple.

## Installation via un package manager (Ubuntu)

Sur un système Ubuntu (au minimum 19.04), on peut simplement utiliser la commande suivante pour installer les outils `bpftrace`:

```sh
sudo apt install -y bpftrace
```

Pour build le projet directement ou utiliser un autre gestionnaire de package, une [page d'installation](https://github.com/bpftrace/bpftrace/blob/master/INSTALL.md) est disponible.

# Procédure de test

Afin de pouvoir s'assurer du bon fonctionnement du programme, une procédure de test a été mise en place.

Elle peut être executée avec la commande `sudo ./run_tests.sh build/segmentation img/nyc.png 3 output.png`

Le programme effectue du tracing sur un programme de test qui segmente les images en utilisant les algorithmes kmeans et kmeans++. 

Les sondes mesurées sont les suivantes:

* un nombre d'appel à une fonction

* cycles de cpu (divisé par 1e6)

* pages faults

* syscalls

* un histogramme sur la distribution du temps d'exécution de l'appel système openat

Les sorties attendues pour le test sont les suivantes :

```bash
lib/stb already exists, skipping setup.sh
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /path/to/your/code/build
[ 50%] Built target segmentation_simd
[100%] Built target segmentation
Attaching 351 probes...
Tracing the program segmentation... Hit CTRL+C to end
Opening file: /etc/ld.so.cache
Opening file: /lib/x86_64-linux-gnu/libm.so.6
Opening file: /lib/x86_64-linux-gnu/libc.so.6
Opening file: img/nyc.png
Image loaded!
Opening file: output.png
Programm ended successfully



@calls[uretprobe:build/segmentation:distance]: 4899000
@cpu_cycles: 34659
@page_faults: 28779
@syscall_count[tracepoint:syscalls:sys_enter_rseq]: 1
@syscall_count[tracepoint:syscalls:sys_enter_getrandom]: 1
@syscall_count[tracepoint:syscalls:sys_enter_exit_group]: 1
@syscall_count[tracepoint:syscalls:sys_enter_access]: 1
@syscall_count[tracepoint:syscalls:sys_enter_set_robust_list]: 1
@syscall_count[tracepoint:syscalls:sys_enter_prlimit64]: 1
@syscall_count[tracepoint:syscalls:sys_enter_set_tid_address]: 1
@syscall_count[tracepoint:syscalls:sys_enter_arch_prctl]: 2
@syscall_count[tracepoint:syscalls:sys_enter_pread64]: 2
@syscall_count[tracepoint:syscalls:sys_enter_lseek]: 2
@syscall_count[tracepoint:syscalls:sys_enter_mprotect]: 4
@syscall_count[tracepoint:syscalls:sys_enter_munmap]: 4
@syscall_count[tracepoint:syscalls:sys_enter_close]: 5
@syscall_count[tracepoint:syscalls:sys_enter_write]: 5
@syscall_count[tracepoint:syscalls:sys_enter_openat]: 5
@syscall_count[tracepoint:syscalls:sys_enter_newfstatat]: 6
@syscall_count[tracepoint:syscalls:sys_enter_read]: 7
@syscall_count[tracepoint:syscalls:sys_enter_mmap]: 15
@syscall_count[tracepoint:syscalls:sys_enter_brk]: 778
Attaching 3 probes...
Making an hist for the program segmentation... Hit CTRL+C to end
Image loaded!
Programm ended successfully



@duration: 
[4K, 8K)               3 |@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|
[8K, 16K)              1 |@@@@@@@@@@@@@@@@@                                   |
[16K, 32K)             0 |                                                    |
[32K, 64K)             0 |                                                    |
[64K, 128K)            1 |@@@@@@@@@@@@@@@@@                                   |

@start[1255067]: 0
```