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

## Installation via un package manager (Ubuntu)

Sur un système Ubuntu (au minimum 19.04), on peut simplement utiliser la commande suivante pour installer les outils `bpftrace`:

```sh
sudo apt install -y bpftrace
```

TODO demander généric build

## Mise en place et sanity check

Cette section présente la mise en place de l'outil et un petit test afin de s'assurer que le programme fonctionne correctement.

# Procédure de test

TODO procedure