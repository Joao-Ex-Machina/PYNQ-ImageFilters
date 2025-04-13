#include <ap_int.h>
#include <ap_fixed.h>

#define BHEIGHT 128 /*full square block*/
#define BWIDTH BHEIGHT

#define PADSIZE 8

#define UHEIGHT (BHEIGHT-2*PADSIZE) /*Non-padded parts*/
#define UWIDTH (BWIDTH-2*PADSIZE)

#define DIV_r4 0.0125 /*Reciprocal of 1/80*/
#define DIV_r8 0.003472222  /*Reciprocal of 1/288*/

void checkered(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]);
void frame(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]);
void avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], unsigned offset);

void filter_Controller(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], ap_int<1> sw0, ap_int<1> sw1){
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE bram port=in
    #pragma HLS INTERFACE bram port=out

    if(sw0==0 && sw1==0)
        checkered(in, out);
    else if(sw0==0 && sw1==1)
        frame (in, out);
    else if(sw0==1 && sw1==0)
        avg_Conv(in, out, 4);
    else /*if (sw0==1 && sw1==1)*/
        avg_Conv(in, out, 8);
}

void checkered(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
	int i,j;

    for(i=PADSIZE; i< BHEIGHT-PADSIZE; i++)
        for(j=PADSIZE; j < BWIDTH-PADSIZE; j ++)
            if (!((i ^ j) & 1)) /*use xor to add lsb and mask it, avoids using divisions*/
                out[i-PADSIZE][j-PADSIZE]=0x0;
            else
                out[i-PADSIZE][j-PADSIZE]=in[i][j];
}

void frame(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
	int i,j;

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

void avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], unsigned offset){
    /* iterable coordinates*/
    ap_int<8> ki, kj;
    ap_int<8> iaccum, jaccum;
    /* Bi-dimensional array of Q17.0 accumulators for the worst case*/
    /*Why Q17.0? Do 255 * 288....*/
    ap_int<17> accum[2*8+1][UWIDTH];
    ap_fixed<8,8, AP_TRN, AP_SAT> res;
    ap_fixed<17,0, AP_TRN, AP_SAT> div;
    ap_int<8> count = offset + 1;
    ap_int<8> countMax;

    if(offset == 4){
        div=DIV_r4; /*setup Q0.17 divisors */
        countMax=9; /*setup max line counter */
    }
    else{
        div=DIV_r8;
        countMax=17;
    }

    for (int i=0; i<2*8+1; i++)
        for(int j=0; j > UWIDTH; j++)
            accum[i][j]=0;

    for(int i = PADSIZE-offset; i < BHEIGHT-PADSIZE+offset; i++){ /*Pad always to offset 8*/
        for(int j = PADSIZE-offset; j < BWIDTH-PADSIZE+offset; j++){

            for (ki = -offset; ki < offset+1; ki++){
                for(kj= -offset; kj < offset+1; kj++){

                    iaccum = i + ki - PADSIZE; /*calculate input image indexing*/
                    jaccum = j + kj - PADSIZE;
                    if(iaccum < 0 || iaccum >= UHEIGHT) /*Verify that indexed accum related to non-paded pixel*/
                        continue;
                    if(jaccum < 0 || jaccum >= UWIDTH)
                        continue;

                    iaccum = count + ki; /*convert to reusable accum matrix indexing*/
                    if(iaccum < 0)
                        iaccum += countMax;
                    else if(iaccum >= countMax)
                        iaccum -= countMax;

                    if(ki !=0 || kj !=0)  /*Self does not count to out*/
                        accum[iaccum][jaccum] += in[i][j]; /*have to finish accum indexing*/
                }
            }
            if(i-PADSIZE-offset < PADSIZE || i-PADSIZE-offset >= UHEIGHT)
                continue;
            if(j-PADSIZE-offset < PADSIZE || j-PADSIZE-offset >= UWIDTH)
                continue;

            jaccum=j-PADSIZE-offset;
            iaccum=count-offset;

            if(iaccum < 0)
                iaccum += countMax;
            else if(iaccum >= countMax)
                iaccum -= countMax;

            res = accum[iaccum][jaccum] * div; /*Q17.0 * Q0.16 = Q17.16*/
            out[i-PADSIZE][j-PADSIZE]= res;
            accum[iaccum][jaccum]=0;
        }
        count++;
        if(count >= 8*2+1)
            count=0;
    }

}

void naive_avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[BHEIGHT][BWIDTH], unsigned offset){
    unsigned i, j;
    unsigned ki, kj;
    unsigned sum = 0;
    ap_int<16> div;
    if(offset == 4)
        div=DIV_r4;
    else
        div=DIV_r8;
    for(i = 8-offset; i < BHEIGHT-8+offset; i++){ /*Pad always to offset 8*/
        for(j = 8-offset; j < BWIDTH-8+offset; j++){
            sum = 0;
            for (ki = -offset; ki < offset+1; ki++) /*not parallelizable - AACum*/
                for(kj= -offset; kj < offset+1; kj++)
                    if(ki !=0 || kj !=0)
                        sum+=in[i+ki][i+kj];
            out[i][j]= sum * div; /*replace by piecewise value from memory*/
        }
    }

}
