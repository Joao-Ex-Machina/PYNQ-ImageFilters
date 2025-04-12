#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x, y;
    int divAcc, radious, dimBlock;
    
    if (argc != 3) {
        fprintf(stderr, "Error\nUsage: %s KernelRadious BlockDim\n", argv[0]);
        return -1;
    }
    
    radious = atoi(argv[1]);
    dimBlock = atoi(argv[2]);
    
    int div[dimBlock][dimBlock];
    
    for (int i = 0; i < dimBlock; i++) {
        for (int j = 0; j < dimBlock; j++){
            divAcc = 0;
            for(int ioff = -radious; ioff < radious+1; ioff++){
                for(int joff = -radious; joff < radious+1; joff++) {
                    x = i + ioff;
                    y = j + joff;
                    if (x < 0 || x > dimBlock - 1)
                        continue;
                    if (y < 0 || y > dimBlock - 1)
                        continue;;
                    divAcc++;
                }
            }
            div[i][j] = divAcc;
        }
    }
    
    for (int i = -1; i < dimBlock; i++) {
        fprintf(stdout, "%3d  ", i);
    }

    fprintf(stdout, "\n");

    for (int i = 0; i < dimBlock; i++) {
        for (int j = -1; j < dimBlock; j++){
            if (j == -1){
                fprintf(stdout, "%3d  ", i);
                continue;
            }

            fprintf(stdout, "%3d  ", div[i][j]);
        }
        fprintf(stdout, "\n");
    }



}