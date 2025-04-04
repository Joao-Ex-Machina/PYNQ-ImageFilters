void avg_Conv3x3_naive(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH]){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE bram port=in
#pragma HLS INTERFACE bram port=out
    unsigned i, j;
    unsigned ki, kj;
    unsigned sum = 0;
    for(i = 1; i < BHEIGHT-1; i++){
        for(j = 1; j < BWIDTH-1; j++){
            sum = 0;
            for (ki = -1; ki < 2; ki++) /*not parallelizable - AACum*/
                for(kj= -1; kj < 2; kj++)
                    sum+=in[i+ki][i+kj];
            out[i][j]= sum / 9;
        }
    }
            
}
