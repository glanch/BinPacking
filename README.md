# OR_II_VRP_2023

This is a program, which reads an instance of an vrp and solves the vrp with the following notation with the SCIP-Framework and soplex as solver.

## Modeldefinition VRP
# Notation

| Indizes und Mengen |                        |
| ------------------ | ---------------------- |
| $i, j \in \mathcal{I}$ | Destinations (0 is the depot) |
| $m \in \mathcal{M}$ | Vehicles |

| Parameter |                        |
| --------- | ---------------------- |
| $c_{ij}$ | distance from $i$ to $j$ |
| $w_{i}$ | transportation units at destination $i$ |
| $b_{m}$ | capacity for vehicle $m$ |

| Entscheidungsvariablen |                        |
| ---------------------- | ---------------------- |
| $X_{ijm} \in\{0, 1\}$ | = 1 if we drive from $i$ to $j$ with vehicle $m$, 0 otherwise |
| $Y_{im} \in \{0, 1\}$ | = 1 if vehicle $m$ drives to destination $i$ |
| $Z_i \in \mathbf{R}$ | real-valued auxiliary variable to avoid short cycles |

## Compact model VRP

```math
Min \sum_{i \in \mathcal{I}} \sum_{j \in \mathcal{I}} \sum_{m \in \mathcal{M}} c_{ij} \cdot X_{ijm} \\

s.t. \\

\sum_{i \in \mathcal{I}} w_{i} \cdot Y_{im} \leq b_m; \forall   m \in \mathcal{M} \\

\sum_{j \in \mathcal{I}} X_{ijm} = Y_{im}; \forall i \in \mathcal{I}, m \in \mathcal{M} \\


\sum_{i \in \mathcal{I}} X_{ijm} = Y_{jm}; \forall j \in \mathcal{I}, m \in \mathcal{M} \\

\sum_{m \in \mathcal{M}} Y_{im} = 1; \forall  i \in \mathcal{I} \backslash \{0\} \\

Z_i - Z_j + |\mathcal{I}| \cdot \sum_{m \in \mathcal{M}} X_{ijm} \leq |\mathcal{I}| - 1; \forall i,j \in \mathcal{I}\backslash \{0\}, (i \neq j) 



