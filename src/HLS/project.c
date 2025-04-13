#include <ap_int.h>

#define BHEIGHT 64 /*full block*/
#define BWIDTH 64

# define PADSIZE 8

#define UHEIGHT (BHEIGHT-2*PADSIZE) /*Non-padded parts*/
#define UWIDTH (BWIDTH-2*PADSIZE)


ap_int<16>  div_off4= 17;   /*Reciprocal of 1/15*/
ap_int<16>  div_off8= 1;  /*Reciprocal of 1/289*/

void filter_Controller(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], ap_int<1> sw0, ap_int<1> sw1){
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE bram port=in
    #pragma HLS INTERFACE bram port=out
    
    if(sw0==0 && sw1==0)
        checkered(in, out);
    else if(sw0==0 && sw1==0)
        frame (in, out);
    else if(sw0==0 && sw1==0)
        avg_Conv(in, out, 4);
    else(sw0==0 && sw1==0)
        avg_Conv(in, out, 8);
}

void checkered(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
    for(i=PADSIZE; i< BHEIGHT-PADSIZE; i++)
        for(j=PADSIZE; j < BWIDTH-PADSIZE; j ++)
            if (!(i ^ j & 1)) /*use xor to add lsb and mask it, avoids using divisions*/
                out[i-PADSIZE][j-PADSIZE]=0x0;
            else
                out[i-PADSIZE][j-PADSIZE]=in[i][j];
}

void frame(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
    for(i=1+PADSIZE; i < BHEIGHT-1-PADSIZE; i++) /*non-framed*/
        for(j=1+PADSIZE; j < BWIDTH-1-PADSIZE; j ++)
                out[i-PADSIZE][j-PADSIZE]=in[i][j];
    
    for(i=0; i < UHEIGHT; i++){ /*vertical frame*/
        out[i][0]=0x0;
        out[i][UWIDTH-1]=0x0;
    }

    for(j=1; j < UWIDTH-1; j++){ /*horizontal frame*/
        out[0][j]=0x0;
        out[UHEIGHT-1][j]=0x0;
    }

}

void avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH], unsigned offset){
    ap_int<8> i, j;
    ap_int<8> ki, kj;
    ap_int<8> sum = 0;
    ap_int<16> accum[][];
    ap_int<16> div;
    if(offset == 4)
        div=div_off4;
    else
        div=div_off8;
    count=
    for(i = offset; i < BHEIGHT-offset; i++){
        for(j = offset; j < BWIDTH-offset; j++){
            sum = 0;
            for (ki = -offset; ki < offset+1; ki++) /*not parallelizable - AACum*/
                for(kj= -offset; kj < offset+1; kj++)
                    if(ki !=0 || kj !=0)    
                        accum[][]+=in[i][j]; /*have to finish accum indexing*/
            out[i-offset][j-offset]=accum[i-offset][j-offset] * div;
            accum[][]=0;
        }
        count++;
        if(count< offset*2+1)
            count=0;
    }
            
}

void naive_avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH], unsigned offset){
    unsigned i, j;
    unsigned ki, kj;
    unsigned sum = 0;
    ap_int<16> div;
    if(offset == 4)
        div=div_off4;
    else
        div=div_off8;
    for(i = offset; i < BHEIGHT-offset; i++){
        for(j = offset; j < BWIDTH-offset; j++){
            sum = 0;
            for (ki = -offset; ki < offset+1; ki++) /*not parallelizable - AACum*/
                for(kj= -offset; kj < offset+1; kj++)
                    if(ki !=0 || kj !=0)    
                        sum+=in[i+ki][i+kj];
            out[i][j]= sum * div; /*replace by piecewise value from memory*/
        }
    }
            
}
