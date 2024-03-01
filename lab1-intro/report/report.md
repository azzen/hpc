---
title: "HPC - Laboratoire 1"
author: "Rayane Annen"
date: \today
geometry: margin=2cm
---

# Benchmarks 

Machine utilisée pour les tests : 

- Architecture : AArch64 (ARM)
- CPU : Apple M1 Pro 10 Cores
- RAM : 16 GB LPDDR4X SDRAM

Compilateur :

- clang 15.0.0
- target : arm64-apple-darwin23.3.0
- Flags de compilation : -03 -g -Wall 
- Librairies : math.h et stb

## Tests effectués

Images d'entrées : 

| Nom du fichier  | Dimensions [pixels]          | Nombre de composantes par pixel |
|-----------------|------------------------------|---------------------------------|
| `half-life.png` | $2000 \times 2090 = 4038000$ | 3 (8-bit RGB)                   |
| `medalion.png`  | $1267 \times 919 = 1164373$  | 3 (8-bit RGB)                   |
| `half-life.png` | $1150 \times 710 = 816500$   | 3 (8-bit RGB)                   |

Nombre de type : 

- Tableau 1D : `1`
- Liste chaînée : `2`

Sur la base de ces deux variables (l'image et le type de structures de données) une matrice de tests a été effectuée. Chaque test est effectué 50 fois et le résultat gardé est la moyenne de toutes les runs.

\clearpage

## Résultat obtenu

![Résultats du benchmark obtenu avec la machine de test.](results.eps){width=80%}




