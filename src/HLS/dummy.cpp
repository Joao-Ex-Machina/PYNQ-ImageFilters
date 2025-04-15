void tester( unsigned in[10][10], unsigned out[10][10]){
    /* iterable coordinates*/
    ap_int<8> ki, kj;
    ap_int<8> iaccum, jaccum;
    /* Bi-dimensional array of Q17.0 accumulators for the worst case*/
    /*Why Q17.0? Do 255 * 288....*/
    ap_int<17> accum[3][2][2];
    ap_int<17> accumRed, accumGreen, accumBlue;
    ap_fixed<8,8, AP_TRN, AP_SAT> res=0, resRed, resGreen, resBlue;
    ap_fixed<17,0, AP_TRN, AP_SAT> div;
    ap_int<8> count = offset + 1;
    ap_int<8> countMax;
    ap_uint<32> pixel;
    int bingo, bongo;

    loop_init_i: for (int i=0; i<5; i++){
        loop_init_j: for(int j=0; j < 5; j++){
		accum[0][i][j]=0;
		accum[1][i][j]=0;
		accum[2][i][j]=0;
        }
    }

    loop_accum_i: for(int i = 0; i < 10; i++){ /*Pad always to offset 8*/
        loop_accum_j: for(int j = 0; j <10; j++){
            pixel=in[i][j];

            if(j%2 ==0)
		bingo = 0;
            else
		bingo = 1;

            if(j%2 ==0)
		bongo = 0;
            else
		bongo = 1;

            ap_int<17> aux = 0;
            aux(7,0) = pixel.range(23,16);
            accum[0][bingo][bongo] += aux;
            aux(7,0) = pixel.range(15,8);
            accum[1][bingo][bongo] += aux;
            aux(7,0) = pixel.range(7,0);
            accum[2][bingo][bongo] += aux;

          }
    }
}
