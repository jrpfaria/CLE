#define CAPS_ASC(a, b)  if (a > b) {int t = a; a = b; b = t;}

void bitonic_sort_desc(int* val, int num){
    int v = num >> 1;
    int nL = 1;
    int k = log2(num);

    for (int m = 0; m < k; m++)
    {
        int n = 0;
        int u = 0;

        while(n < nL)
        {  
            for (int t = 0; t < v; t++){
                int t_u = t + u;
                int t_u_v = t_u + v;
                if ((t_u & v) == 0)
                    CAPS_DESC(val[t_u], val[t_u_v]);
            }
            u += (v << 1);
            n++;
        }
        v >>= 1;
        nL <<= 1;
    }
}