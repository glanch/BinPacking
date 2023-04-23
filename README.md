# OR_II_BPP_2023

This is a program, which reads an instance of an bpp and solves the bpp with the following BIP model with the SCIP-Framework and soplex as solver.

## Modeldefinition BPP
# Notation

| Indizes und Mengen |                        |
| ------------------ | ---------------------- |
| $i \in \mathcal{I}$ | Items |
| $j \in \mathcal{J}$ | Bins |

| Parameter |                        |
| --------- | ---------------------- |
| $w_{i}$ | Weight of item $i$ |
| $b$ | Capacity of a single bin |

| Entscheidungsvariablen |                        |
| ---------------------- | ---------------------- |
| $X_{ij} \in\{0, 1\}$ | = 1 if we pack item $i$ into bin $j$, 0 otherwise |
| $Y_{j} \in\{0, 1\}$ | =1 if we use bin $j$, 0 otherwise |

## Compact model BPP

```math
\min \sum_{j \in \mathcal{J}} Y_{j} \\

s.t. \\

\sum_{j \in \mathcal{J}} X_{ij} = 1; \forall   i \in \mathcal{I} \\

\sum_{i \in \mathcal{I}} w_{i} \cdot X_{ij} \leq b\cdot Y_{j}; \forall j \in \mathcal{J} \\

```
