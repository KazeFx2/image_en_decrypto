## Encrypto
1. Resize Image
2. Gen Confusion Seq
      1. unsigned 16-bits seq
      2. limit it into `CONFUSION_SEED_UPPER_BOUND` ~ `CONFUSION_SEED_LOWER_BOUND`
3. Init condition $?$ with $x_0$, $p$, for $n$ `PLCM` iterations
      1. `PLCM` (Piecewise Linear Chaotic Maps)
         - $$x_{i+1} = F(x_i, p)=\left \{ \begin{align*}&x_i / p,&&x_i\in [0, p)\\&(x_i - p)/(\frac{1}{2} - p),&&x_i\in[p, 0.5]\\&F(1-x_i, p),&&x_i\in(0.5, 1]\end{align*}\right.$$
      2. Rearrange pixel positions
         - $$\left \{ \begin{align*}\alpha &= (\alpha + o)\ mod\ N\\\beta &= (o + s_csin(2\pi\alpha/N))\ mod\ N\end{align*}\right.$$
         