# OR_II_BPP_2023

This is a program, which reads an instance of an bpp and solves the bpp with the following BIP model with the SCIP-Framework and soplex as solver.

## Parameter

| Indizes und Mengen |                        |
| ------------------ | ---------------------- |
| $i \in \mathcal{I}$ | Items |
| $j \in \mathcal{J}$ | Bins |

| Parameter |                        |
| --------- | ---------------------- |
| $w_{i}$ | Weight of item $i$ |
| $b$ | Capacity of a single bin |


## Compact model BPP

### Variables
| Entscheidungsvariablen |                        |
| ---------------------- | ---------------------- |
| $X_{ij} \in\{0, 1\}$ | = 1 if we pack item $i$ into bin $j$, 0 otherwise |
| $Y_{j} \in\{0, 1\}$ | =1 if we use bin $j$, 0 otherwise |

```
\min \sum_{j \in \mathcal{J}} Y_{j} \\

s.t. \\

\sum_{j \in \mathcal{J}} X_{ij} = 1; \forall   i \in \mathcal{I} \\

\sum_{i \in \mathcal{I}} w_{i} \cdot X_{ij} \leq b\cdot Y_{j}; \forall j \in \mathcal{J} \\

```

## CG BPP

### Notation
Let $\mathcal P$ denote the set of feasible packing patterns:
```math
    \mathcal P :=
    \left\{ 
        \begin{pmatrix} a_1 \\ \vdots \\ a_n \end{pmatrix} \in \{0,1\}^n \mid \sum_{i \in \mathcal{I}} w_i \cdot a_i \leq b
    \right\}
```

Furthermore, we denote a subset of patterns by $\mathcal{P'} \subseteq \mathcal{P}$. 



### RMP
#### Variables
| Entscheidungsvariablen | Bedeutung              |
| ---------------------- | ---------------------- |
| $\lambda_{p} \in \mathbb{Q}$ for all $p \in \mathcal{P}$ | relaxed version of binary variable in integer formulation, for binary restriction it holds that = 1 if we pack a bin using pattern $p$, 0 otherwise |

#### Formulation
```math
\begin{aligned}{}
\min&& \sum_{p \in \mathcal{P'}} \lambda_p &\\ 
\text{s.t.}&& \sum_{p \in \mathcal{P}'} a_i^p\cdot\lambda_p = 1 && \forall i \in \mathcal{I} \\

&&\lambda_p \geq 0
\end{aligned}
```

### Pricing Problem

#### Variables
| Entscheidungsvariablen | Bedeutung              |
| ---------------------- | ---------------------- |
| $x_{i} \in\{0, 1\}$ for all items $i \in \mathcal{I}$ | = 1 if item $i$ is contained in packing pattern, 0 otherwise |


#### Formulation
```math
\begin{aligned}{}
\min && 1 - \sum_{i \in \mathcal{I}} \pi_i \cdot x_i  &\\ 
\text{s.t.}&& \sum_{p \in \mathcal{I}} w_i \cdot x_i\cdot\lambda_p \leq b && \\

&&x_i \in \{0, 1\} \forall i \in \mathcal{I}
\end{aligned}
```
