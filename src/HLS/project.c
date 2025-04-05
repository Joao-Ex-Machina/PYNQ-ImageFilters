#include <ap_int.h>

void filter_Controller(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH], ap_int<1> sw0, ap_int<1> sw1){
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE bram port=in
    #pragma HLS INTERFACE bram port=out
    
    if(sw0==0 && sw1==0)
        checkered(in, out);
    else if(sw0==0 && sw1==0)
        frame (in, out);
    else if(sw0==0 && sw1==0)
        avg_Conv(in, out, 2);
    else(sw0==0 && sw1==0)
        avg_Conv(in, out, 4);
}

void checkered(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH]){
    for(i=0; i< BHEIGHT; i++)
        for(j=0; j < BWIDTH; j ++)
            if (!(i ^ j & 1)) /*use xor to add lsb and mask it, avoids using divisions*/
                out[i][j]=0x00ffffff;
            else
                out[i][j]=in[i][j];
}

void frame(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH]){
    for(i=0; i < BHEIGHT; i++)
        for(j=0; j < BWIDTH; j ++)
            if (i==0 || j==0 || i==BHEIGHT-1 || j==BWIDTH-1)
                out[i][j]=0x00ffffff;
            else
                out[i][j]=in[i][j];


}

void avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH], unsigned offset){
    unsigned i, j;
    unsigned ki, kj;
    unsigned sum = 0;
    for(i = offset; i < BHEIGHT-offset; i++){
        for(j = offset; j < BWIDTH-offset; j++){
            sum = 0;
            for (ki = -offset; ki < offset+1; ki++) /*not parallelizable - AACum*/
                for(kj= -offset; kj < offset+1; kj++)
                    if(ki !=0 || kj !=0)    
                        sum+=in[i+ki][i+kj];
            out[i][j]= sum / 8;
        }
    }
            
}


