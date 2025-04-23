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
void naive_avg_Conv4(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]);
void naive_avg_Conv8(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]);
// void avg_Conv(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], unsigned offset);


void filter_Controller(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], ap_int<1> sw0, ap_int<1> sw1){

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE bram port=in
    #pragma HLS INTERFACE bram port=out

    if (sw1 == 0){
        if (sw0 == 0) {
            checkered(in, out);
        } else /*if (sw0 == 1)*/ {
            frame(in, out);
        }
    } else /* if (sw1 == 1)*/{
        if (sw0 == 0) {
            naive_avg_Conv4(in, out);
        } else /*if (sw0 == 1)*/ {
            naive_avg_Conv8(in, out);
        }
    }
}


void checkered(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
	int i,j;
    unsigned pixel;
    for(i = PADSIZE; i < BHEIGHT - PADSIZE; i++){
        for(j = PADSIZE; j < BWIDTH - PADSIZE; j++){
            pixel=in[i][j];
            if (!((i ^ j) & 1)) /*use xor to add lsb and mask it, avoids using divisions*/
                out[i-PADSIZE][j-PADSIZE] = 0x0;
            else
                out[i-PADSIZE][j-PADSIZE] = pixel;
        }
    }
}


void frame(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
	int i,j;

    for(i = PADSIZE + 1; i < BHEIGHT - PADSIZE - 1; i++) { /*non-framed*/
        for(j = PADSIZE + 1; j < BWIDTH - PADSIZE - 1; j++) {
                out[i-PADSIZE][j-PADSIZE]=in[i][j];
        }
    }

    for(i=0; i < UHEIGHT; i++){ //vertical frame
        out[i][0]=0x0;
    }

    for(i=0; i < UHEIGHT; i++){ //vertical frame
        out[i][UWIDTH-1]=0x0;
    }

    for(j=1; j < UWIDTH-1; j++){ //horizontal frame
        out[0][j]=0x0;
    }

    for(j=1; j < UWIDTH-1; j++){ //horizontal frame
        out[UHEIGHT-1][j]=0x0;
    }

}


void naive_avg_Conv4(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
    signed i, j;
    signed ki, kj;
    ap_fixed<16,16> sumRed = 0;
    ap_fixed<16,16> sumGreen = 0;
    ap_fixed<16,16> sumBlue = 0;
    ap_fixed<32,32> pixel, res = 0;
    ap_fixed<17,0> div;

    div=DIV_r4;

    loop_i: for(i = PADSIZE; i < BHEIGHT-PADSIZE ; i++) { /*Pad always to offset 8*/
        loop_j: for(j = PADSIZE; j < BWIDTH-PADSIZE; j++) {
            loop_ki: for (ki = -4; ki < 4+1; ki++) {
                loop_kj: for(kj= -4; kj < 4+1; kj++) {
                    if(ki !=0 || kj !=0) {
                        pixel= (ap_fixed<32,32>) in[i+ki][j+kj]; /*horrible, no reuse of memory, multiple accesses*/
                        sumBlue  += pixel.range(23,16);
                        sumGreen += pixel.range(15,8);
                        sumRed   += pixel.range(7,0);
                    }
                }
            }
            res.range(23,16) = (sumBlue * div).range(24,17); /*Q17.0 * Q0.16 = Q17.16 put back into Q8.0*/
            res.range(15,8)  = (sumGreen * div).range(24,17);
            res.range(7,0)   = (sumRed * div).range(24,17) ;
            out[i-PADSIZE][j-PADSIZE]= (unsigned)res; /*replace by piecewise value from memory*/
            sumBlue  = 0;
            sumGreen = 0;
            sumRed   = 0;
        }
    }

}


void naive_avg_Conv8(unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH]){
    signed i, j;
    signed ki, kj;
    ap_fixed<18,18> sumRed = 0;
    ap_fixed<18,18> sumGreen = 0;
    ap_fixed<18,18> sumBlue = 0;
    ap_fixed<32,32> pixel, res = 0;
    ap_fixed<17,0> div;

    div=DIV_r8;

    loop_i: for(i = PADSIZE; i < BHEIGHT-PADSIZE ; i++) { /*Pad always to offset 8*/
        loop_j: for(j = PADSIZE; j < BWIDTH-PADSIZE; j++) {
            loop_ki: for (ki = -8; ki < 8+1; ki++) {
                loop_kj: for(kj= -8; kj < 8+1; kj++) {
                    if(ki !=0 || kj !=0) {
                        pixel= (ap_fixed<32,32>) in[i+ki][j+kj]; /*horrible, no reuse of memory, multiple accesses*/
                        sumBlue  += pixel.range(23,16);
                        sumGreen += pixel.range(15,8);
                        sumRed   += pixel.range(7,0);
                    }
                }
            }
            res.range(23,16) = (sumBlue * div).range(24,17); /*Q17.0 * Q0.17 = Q17.17 put back into Q8.0*/
            res.range(15,8)  = (sumGreen * div).range(24,17);
            res.range(7,0)   = (sumRed * div).range(24,17);
            out[i-PADSIZE][j-PADSIZE]= (unsigned)res; /*replace by piecewise value from memory*/
            sumBlue  = 0;
            sumGreen = 0;
            sumRed   = 0;
        }
    }
}


// /*This needs to be extensibly reviewed*/
// void avg_Conv( unsigned in[BHEIGHT][BWIDTH], unsigned out[UHEIGHT][UWIDTH], unsigned offset){
//     /* iterable coordinates*/
//     ap_int<8> ki, kj;
//     ap_int<8> iaccum, jaccum;
//     /* Bi-dimensional array of Q17.0 accumulators for the worst case*/
//     /*Why Q17.0? Do 255 * 288....*/
//     ap_int<17> accum[3][2*8+1][UWIDTH];
//     ap_int<17> accumRed, accumGreen, accumBlue;
//     ap_fixed<8,8, AP_TRN, AP_SAT> res=0, resRed, resGreen, resBlue;
//     ap_fixed<17,0, AP_TRN, AP_SAT> div;
//     ap_int<8> count = offset + 1;
//     ap_int<8> countMax;
//     ap_uint<32> pixel;
//     if(offset == 4){
//         div=DIV_r4; /*setup Q0.17 divisors */
//         countMax=9; /*setup max line counter */
//     }
//     else{
//         div=DIV_r8;
//         countMax=17;
//     }

//     loop_init_i: for (int i=0; i<2*8+1; i++){
//         loop_init_j: for(int j=0; j < UWIDTH; j++){
// 		accum[0][i][j]=0;
// 		accum[1][i][j]=0;
// 		accum[2][i][j]=0;
//         }
//     }


//     loop_accum_i: for(int i = PADSIZE-offset; i < BHEIGHT-PADSIZE+offset; i++){ /*Pad always to offset 8*/
//         loop_accum_j: for(int j = PADSIZE-offset; j < BWIDTH-PADSIZE+offset; j++){
//             pixel=in[i][j];

//             loop_ki: for (ki = -offset; ki < offset+1; ki++){
//                 loop_kj: for(kj= -offset; kj < offset+1; kj++){

//                     iaccum = i + ki - PADSIZE; /*calculate input image indexing*/
//                     jaccum = j + kj - PADSIZE;
//                     if(iaccum < 0 || iaccum >= UHEIGHT) /*Verify that indexed accum related to non-paded pixel*/
//                         continue;
//                     if(jaccum < 0 || jaccum >= UWIDTH)
//                         continue;

//                     iaccum = count + ki; /*convert to reusable accum matrix indexing*/
//                     if(iaccum < 0)
//                         iaccum += countMax;
//                     else if(iaccum >= countMax)
//                         iaccum -= countMax;

//                     if(ki !=0 || kj !=0){  /*Self does not count to out*/
// 			ap_int<17> aux = 0;
// 			aux(7,0) = pixel.range(23,16);
// 			accum[0][iaccum][jaccum] += aux;
// 			aux(7,0) = pixel.range(15,8);
// 			accum[1][iaccum][jaccum] += aux;
// 			aux(7,0) = pixel.range(7,0);
// 			accum[2][iaccum][jaccum] += aux;

//                     }
//                }
//             }
//             if(i-PADSIZE-offset < PADSIZE || i-PADSIZE-offset >= UHEIGHT)
//                 continue;
//             if(j-PADSIZE-offset < PADSIZE || j-PADSIZE-offset >= UWIDTH)
//                 continue;

//             jaccum=j-PADSIZE-offset;
//             iaccum=count-offset;

//             if(iaccum < 0)
//                 iaccum += countMax;
//             else if(iaccum >= countMax)
//                 iaccum -= countMax;

//             res.range(23,16) = accum[0][iaccum][jaccum] * div; /*Q17.0 * Q0.16 = Q17.16 put back into Q8.0*/
//             res.range(15,8)  = accum[1][iaccum][jaccum] * div;
//             res.range(7,0)   = accum[2][iaccum][jaccum] * div ;
//             out[i-PADSIZE][j-PADSIZE] = res;
//             accum[0][iaccum][jaccum] = 0;
//             accum[1][iaccum][jaccum] = 0;
//             accum[2][iaccum][jaccum] = 0;
//         }
//         count++; /*to create a perfect loop we have to remove it from here*/
//         if(count >= 8*2+1)
//             count=0;
//     }

// }