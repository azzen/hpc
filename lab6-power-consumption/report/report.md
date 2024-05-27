---
title: "HPC - Laboratoire 6"
subtitle: "Consommation energétique"
author: "Rayane Annen"
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

# Machine de test

Machine utilisée pour les tests :

- Architecture : Intel x86_64
- CPU : Intel Core i7-1165G7 @ 2.8 GHz / Turbo @ 4.7 GHz
  - Cache L1d : 192 KiB
  - Cache L1i : 128 KiB
  - Cache L2 : 5 MiB
  - Cache L3 : 12 MiB
- OS: Ubuntu 24.04
- Compilateur :
  - gcc 13.2.0
  - target: x86_64-linux-gnu
  - Flags de compilation: `-O3 -g -Wall -fno-inline -march=native -fno-tree-vectorize`
  - Librairies: `stb`, `math.h`, `Likwid`

Outils :

- Perf
- Powercap

N.B : Pas possible d'utiliser likwid car incompatible avec ma machine.

# Capture des performances

## `perf`

| Consommation d'énergie [J] | Evènement             | Programme           |
|----------------------------|-----------------------|---------------------|
| 6.63                       | `power/energy-cores/` | `segmentation`      |
| 5.03                       | `power/energy-cores/` | `segmentation_simd` |
| 0.07                       | `power/energy-gpu/`   | `segmentation`      |
| 0.05                       | `power/energy-gpu/`   | `segmentation_simd` |
| 8.85                       | `power/energy-pkg/`   | `segmentation`      |
| 4.88                       | `power/energy-pkg/`   | `segmentation_simd` |
| 2.54                       | `power/energy-psys/`  | `segmentation`      |
| 1.38                       | `power/energy-psys/`  | `segmentation_simd` |

Mesures plus précises en utilisant les marqueurs directement, j'ai décidé de mesurer la fonction `kmeans_pp` :

```c
void kmeans(struct img_1D_t *image, int num_clusters){
    uint8_t *centers = calloc(num_clusters * image->components, sizeof(uint8_t));
    perf_manager pmon;
    perf_manager_init(&pmon);


    perf_manager_resume(&pmon);
    // Initialize the cluster centers using the k-means++ algorithm.
    kmeans_pp(image, num_clusters, centers);
    perf_manager_pause(&pmon);
    // ....
}
```

| Consommation d'énergie [J] | Evènement             | Programme           |
|----------------------------|-----------------------|---------------------|
| 1.73                       | `power/energy-cores/` | `segmentation`      |
| 1.88                       | `power/energy-cores/` | `segmentation_simd` |
| 0.01                       | `power/energy-gpu/`   | `segmentation`      |
| 0.01                       | `power/energy-gpu/`   | `segmentation_simd` |
| 2.27                       | `power/energy-pkg/`   | `segmentation`      |
| 2.17                       | `power/energy-pkg/`   | `segmentation_simd` |
| 0.60                       | `power/energy-psys/`  | `segmentation`      |
| 0.80                       | `power/energy-psys/`  | `segmentation_simd` |
