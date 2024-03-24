---
title: "HPC - Laboratoire 2"
subtitle: "Profiling"
author: "Rayane Annen"
lang: fr
date: \today
geometry: margin=2cm
---

# Benchmarks 

Machine utilisée pour les tests : 

- Architecture : Intel x86_64
- CPU : i9-9900K 8 Cores / 16 Threads @ 3.60GHz / Turbo 5.00GHz
- OS: Debian 12
- Compilateur : 
    - gcc 12.2.0
    - target: x86_64-linux-gnu
    - Flags de compilation: `-O3 -g -Wall -fno-inline -DLIKWID_PERFMON`
    - Librairies: `stb`, `math.h` et `Likwid`

