/***************************************************************************
Copyright (c) 2018, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "common.h"

#define NBMAX 1024
//#define PREFETCH 1
#include <altivec.h> 

#define HAVE_KERNEL4x8_ASM 1


#if defined(HAVE_KERNEL4x8_ASM)
static void dgemv_kernel_4x8(BLASLONG n, BLASLONG lda, double *ap, double *x, double *y, double alpha) {

    FLOAT *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7;
    BLASLONG off2;
    BLASLONG tempR;
    __asm__(
         
            "sldi   %[temp],%[off], 4 \n\t" // lda * sizeof (double) *2
            "sldi   %[off], %[off], 3 \n\t" // lda * sizeof (double)    
            "xxlxor 34,34,34  \n\t"
            "xxlxor 35,34,34  \n\t"
            "add    %[a2], %[a0], %[temp]    \n\t"
            "add    %[a1], %[a0], %[off]     \n\t"
            "xxlxor 4,34,34 \n\t"
            "xxlxor 5,34,34 \n\t"
            "xxlxor 6,34,34 \n\t"
            "xxlxor 7,34,34 \n\t"            
            "add    %[a3], %[a2], %[off]     \n\t"
            "add    %[a4], %[a2], %[temp]    \n\t"            
 
            "xxlxor 8,34,34 \n\t"
            "xxlxor 9,34,34 \n\t"              
            "add    %[a5], %[a3], %[temp]    \n\t"
            "li     %[off],0    \n\t"
            "li     %[off2],16  \n\t" 
  
            "add    %[a6], %[a4], %[temp]    \n\t" 
            "add    %[a7], %[a5], %[temp]    \n\t"
    



            "lxvp 32, 0(%[x])   \n\t"
            "lxvp 36, 0(%[a0])  \n\t"
            "lxvp 38, 0(%[a1])  \n\t"
            "lxvp 40, 0(%[a2])  \n\t"
            "lxvp 42, 0(%[a3])  \n\t"
            "lxvp 44, 0(%[a4])  \n\t"
            "lxvp 46, 0(%[a5])  \n\t"
            "lxvp 48, 0(%[a6])  \n\t"
            "lxvp 50, 0(%[a7])  \n\t"
#if defined(PREFETCH)    
            "li     %[temp],896  \n\t"
#endif    
            "addic. %[n],%[n],-4 \n\t"

            "li       %[off],32 \n\t" 


            "ble-     two%=      \n\t"

            //--------------------------------------------------           
            ".align   5           \n\t"
            "one%=:                     \n\t"
            "xvmaddadp   34,36,32  \n\t"
            "xvmaddadp   35,38,32  \n\t"
            "addi   %[off2],  %[off2],32 \n\t"
            "xvmaddadp   4,40,32  \n\t"
            "xvmaddadp   5,42,32  \n\t"
            "xvmaddadp   6,44,32  \n\t"
            "xvmaddadp   7,46,32  \n\t" 
            "xvmaddadp   8,48,32  \n\t"
            "xvmaddadp   9,50,32  \n\t"

            "xvmaddadp  34,37,33  \n\t"
            "xvmaddadp  35,39,33  \n\t"            
            "lxvp 36, 32(%[a0])  \n\t"
            "lxvp 38, 32(%[a1])  \n\t"
            "xvmaddadp  4,41,33  \n\t"
            "xvmaddadp  5,43,33  \n\t"            
            "addi       %[off],  %[off],32 \n\t"
            "lxvp 40, 32(%[a2])  \n\t"
            "lxvp 42, 32(%[a3])  \n\t"
            "xvmaddadp  6,45,33  \n\t"
            "xvmaddadp  7,47,33  \n\t"            
            "lxvp 44, 32(%[a4])  \n\t"
            "lxvp 46, 32(%[a5])  \n\t"
            "xvmaddadp  8,49,33  \n\t"
            "xvmaddadp  9,51,33  \n\t" 
            
            "addic. %[n],%[n],-4 \n\t"                        
            "lxvp 48, 32(%[a6])  \n\t"
            "lxvp 50, 32(%[a7])  \n\t"
            "lxvp 32, 32(%[x])   \n\t"
            "ble- two%=  \n\t"
            "xvmaddadp   34,36,32  \n\t"
            "xvmaddadp   35,38,32  \n\t"
            "addi   %[off2],  %[off2],32 \n\t" 
            "xvmaddadp   4,40,32  \n\t"
            "xvmaddadp   5,42,32  \n\t"
            "xvmaddadp   6,44,32  \n\t"
            "xvmaddadp   7,46,32  \n\t" 
            "xvmaddadp   8,48,32  \n\t"
            "xvmaddadp   9,50,32  \n\t"

            "xvmaddadp  34,37,33  \n\t"
            "xvmaddadp  35,39,33  \n\t"            
            "lxvp 36, 64(%[a0])  \n\t"
            "lxvp 38, 64(%[a1])  \n\t"
            "xvmaddadp  4,41,33  \n\t"
            "xvmaddadp  5,43,33  \n\t"            
            "addi       %[off],  %[off],32 \n\t"
            "lxvp 40, 64(%[a2])  \n\t"
            "lxvp 42, 64(%[a3])  \n\t"
            "xvmaddadp  6,45,33  \n\t"
            "xvmaddadp  7,47,33  \n\t"            
            "lxvp 44, 64(%[a4])  \n\t"
            "lxvp 46, 64(%[a5])  \n\t"
            "xvmaddadp  8,49,33  \n\t"
            "xvmaddadp  9,51,33  \n\t" 
            
            "addic. %[n],%[n],-4 \n\t"                        
            "lxvp 48, 64(%[a6])  \n\t"
            "lxvp 50, 64(%[a7])  \n\t"
            "lxvp 32, 64(%[x])   \n\t"
            "ble- two%=  \n\t"
            "xvmaddadp   34,36,32  \n\t"
            "xvmaddadp   35,38,32  \n\t"
#if defined(PREFETCH)            
            "addi    %[temp],%[temp],128 \n\t"   
#endif                                             
            "addi   %[off2],  %[off2],32 \n\t" 
            "xvmaddadp   4,40,32  \n\t"
            "xvmaddadp   5,42,32  \n\t"
            "xvmaddadp   6,44,32  \n\t"
            "xvmaddadp   7,46,32  \n\t" 
            "xvmaddadp   8,48,32  \n\t"
            "xvmaddadp   9,50,32  \n\t"
#if defined(PREFETCH)
            "dcbt   %[temp],%[a0]  \n\t"
#endif            

            "xvmaddadp  34,37,33  \n\t"
            "xvmaddadp  35,39,33  \n\t"            
            "lxvp 36, 96(%[a0])  \n\t"
            "lxvp 38, 96(%[a1])  \n\t"
            "xvmaddadp  4,41,33  \n\t"
            "xvmaddadp  5,43,33  \n\t"            
#if defined(PREFETCH)
            "dcbt   %[temp],%[a1]  \n\t"
#endif            
            "lxvp 40, 96(%[a2])  \n\t"
            "lxvp 42, 96(%[a3])  \n\t"
            "addi       %[off],  %[off],32 \n\t"
            "xvmaddadp  6,45,33  \n\t"
            "xvmaddadp  7,47,33  \n\t"            
            "lxvp 44, 96(%[a4])  \n\t"
            "lxvp 46, 96(%[a5])  \n\t"
            "xvmaddadp  8,49,33  \n\t"
            "xvmaddadp  9,51,33  \n\t" 
#if defined(PREFETCH)
            "dcbt   %[temp],%[a3]  \n\t"
#endif            
            "lxvp 48, 96(%[a6])  \n\t"
            "lxvp 50, 96(%[a7])  \n\t"
            "lxvp 32, 96(%[x])   \n\t"
           
            "addic. %[n],%[n],-4 \n\t"                        
            "ble- two%=  \n\t"            
 
            "addi   %[off2],  %[off2],32 \n\t" 
#if defined(PREFETCH)
            "dcbt   %[temp],%[a2]  \n\t"
#endif            
            "xvmaddadp   34,36,32  \n\t"
            "xvmaddadp   35,38,32  \n\t"
            "xvmaddadp   4,40,32  \n\t"
            "xvmaddadp   5,42,32  \n\t"
#if defined(PREFETCH)
            "dcbt   %[temp],%[a4]  \n\t"                         
#endif            
            "xvmaddadp   6,44,32  \n\t"
            "xvmaddadp   7,46,32  \n\t" 
            "xvmaddadp   8,48,32  \n\t"
            "xvmaddadp   9,50,32  \n\t"

#if defined(PREFETCH)
          "dcbt   %[temp],%[a5]  \n\t"
#endif              
            "xvmaddadp  34,37,33  \n\t"
            "xvmaddadp  35,39,33  \n\t"            
            "lxvp 36, 128(%[a0])  \n\t"
            "lxvp 38, 128(%[a1])  \n\t"
            "xvmaddadp  4,41,33  \n\t"
            "xvmaddadp  5,43,33  \n\t"            
            "addi       %[off],  %[off],32 \n\t"
            "lxvp 40, 128(%[a2])  \n\t"
            "lxvp 42, 128(%[a3])  \n\t"
#if defined(PREFETCH)
            "dcbt   %[temp],%[a6]  \n\t"  
#endif            
            "xvmaddadp  6,45,33  \n\t"
            "xvmaddadp  7,47,33  \n\t"            
            "lxvp 44, 128(%[a4])  \n\t"
            "lxvp 46, 128(%[a5])  \n\t"
            "xvmaddadp  8,49,33  \n\t"
            "xvmaddadp  9,51,33  \n\t" 
            
#if defined(PREFETCH)
            "dcbt   %[temp],%[a7]  \n\t"  
#endif            
            "addic. %[n],%[n],-4 \n\t"
            "lxvp 48, 128(%[a6])  \n\t"
            "lxvp 50, 128(%[a7])  \n\t"
            "lxvp 32, 128(%[x])   \n\t"
#if defined(PREFETCH)
            "dcbt   %[temp],%[x]  \n\t" 
#endif            
	    "addi    %[a0], %[a0], 128     \n\t"
	    "addi    %[a1], %[a1], 128     \n\t"
	    "addi    %[a2], %[a2], 128     \n\t"
	    "addi    %[a3], %[a3], 128     \n\t"
	    "addi    %[a4], %[a4], 128     \n\t"
	    "addi    %[a5], %[a5], 128    \n\t"
	    "addi    %[a6], %[a6], 128     \n\t"
	    "addi    %[a7], %[a7], 128     \n\t"
	    "addi    %[x], %[x], 128     \n\t"
            "bgt+ one%=  \n\t"
            ".align   5           \n\t"
            "two%=: \n\t"
            //--------------------------------------------

            "xvmaddadp   34,36,32  \n\t"
            "xvmaddadp   35,38,32  \n\t"
            "xvmaddadp   4,40,32  \n\t"
            "xvmaddadp   5,42,32  \n\t"
            "xvmaddadp   6,44,32  \n\t"
            "xvmaddadp   7,46,32  \n\t" 
            "xvmaddadp   8,48,32  \n\t"
            "xvmaddadp   9,50,32  \n\t" 
            XXSPLTD_S(36,%x[alpha],0)
            "xvmaddadp  34,37,33  \n\t"
            "xvmaddadp  35,39,33  \n\t"            
            "xvmaddadp  4,41,33  \n\t"
            "xvmaddadp  5,43,33  \n\t"            
            "xvmaddadp  6,45,33  \n\t"
            "xvmaddadp  7,47,33  \n\t"            
            "xvmaddadp  8,49,33  \n\t"
            "xvmaddadp  9,51,33  \n\t"  

            "lxvp 38, 0(%[y]) \n\t"
            "lxvp 40, 32(%[y]) \n\t"


#if defined(POWER10) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            XXMRGHD_S(42,34,35)
            XXMRGLD_S(43,34,35)

            XXMRGHD_S(44,4,5)
            XXMRGLD_S(45,4,5)

            "xvadddp 42,42,43 \n\t"

            XXMRGHD_S(46,6,7)
            XXMRGLD_S(47,6,7)

            "xvadddp 44,44,45 \n\t"

            XXMRGHD_S(48,8,9)
            XXMRGLD_S(49,8,9)
#else
            XXMRGLD_S(42,35,34)
            XXMRGHD_S(43,35,34)

            XXMRGLD_S(44,5,4)
            XXMRGHD_S(45,5,4)

            "xvadddp 42,42,43 \n\t"

            XXMRGLD_S(46,7,6)
            XXMRGHD_S(47,7,6)

            "xvadddp 44,44,45 \n\t"

            XXMRGLD_S(48,9,8)
            XXMRGHD_S(49,9,8)
#endif

            "xvadddp 46,46,47 \n\t"
            
#if defined(POWER10) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
			"xvmaddadp  38,42,36  \n\t"
			"xvmaddadp  39,44,36  \n\t"
#else
            "xvmaddadp  39,42,36  \n\t"
            "xvmaddadp  38,44,36  \n\t"
#endif
 
            "xvadddp 48,48,49 \n\t"

#if defined(POWER10) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
			"xvmaddadp  41,48,36  \n\t"
#else
            "xvmaddadp  41,46,36  \n\t"
#endif

            "stxvp 38, 0(%[y]) \n\t"

#if defined(POWER10) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
			"xvmaddadp  40,46,36  \n\t"
#else
            "xvmaddadp  40,48,36  \n\t" 
#endif

            "stxvp 40, 32(%[y])  \n\t"
                 
            : [memy] "+m" (*(double (*)[8])y),
            [n] "+&r" (n),
            [a0] "=b" (a0),
            [a1] "=&b" (a1),
            [a2] "=&b" (a2),
            [a3] "=&b" (a3),
            [a4] "=&b" (a4),
            [a5] "=&b" (a5),
            [a6] "=&b" (a6),
            [a7] "=&b" (a7),            
            [off] "+&b" (lda),
            [off2]"=&b" (off2),
            [temp] "=&b" (tempR)
            : [memx] "m" (*(const double (*)[n])x),
            [mem_ap] "m" (*(const double (*)[n*8]) ap),
            [alpha] "d" (alpha),
            "[a0]" (ap),
            [x] "b" (x),
            [y] "b" (y)
            : "cc","vs4","vs5","vs6","vs7","vs8","vs9" ,"vs32","vs33","vs34","vs35", "vs36", "vs37", "vs38", "vs39",
            "vs40", "vs41", "vs42", "vs43", "vs44", "vs45", "vs46", "vs47", "vs48", "vs49", "vs50", "vs51"
            );
    return;
}
#else
static void dgemv_kernel_4x8(BLASLONG n, BLASLONG lda, FLOAT *ap, FLOAT *x, FLOAT *y, FLOAT alpha) {
    BLASLONG i;
#if defined(PREFETCH)  
    BLASLONG j, c, k;
#endif    
    FLOAT *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7;
    __vector double *va0, *va1, *va2, *va3, *va4, *va5, *va6, *va7, *v_x;
    register __vector double temp0 = {0, 0};
    register __vector double temp1 = {0, 0};
    register __vector double temp2 = {0, 0};
    register __vector double temp3 = {0, 0};
    register __vector double temp4 = {0, 0};
    register __vector double temp5 = {0, 0};
    register __vector double temp6 = {0, 0};
    register __vector double temp7 = {0, 0};

    a0 = ap;
    a1 = ap + lda;
    a2 = a1 + lda;
    a3 = a2 + lda;
    a4 = a3 + lda;
    a5 = a4 + lda;
    a6 = a5 + lda;
    a7 = a6 + lda;
    va0 = (__vector double*) a0;
    va1 = (__vector double*) a1;
    va2 = (__vector double*) a2;
    va3 = (__vector double*) a3;
    va4 = (__vector double*) a4;
    va5 = (__vector double*) a5;
    va6 = (__vector double*) a6;
    va7 = (__vector double*) a7;
    v_x = (__vector double*) x;
 
#if defined(PREFETCH)

    c = n >> 1;

    for (j = 0; j < c; j += 64) {
        k = (c - j) > 64 ? 64 : (c - j);
        __builtin_prefetch(v_x + 64);
        __builtin_prefetch(va0 + 64);
        __builtin_prefetch(va1 + 64);
        __builtin_prefetch(va2 + 64);
        __builtin_prefetch(va3 + 64);
        __builtin_prefetch(va4 + 64);
        __builtin_prefetch(va5 + 64);
        __builtin_prefetch(va6 + 64);
        __builtin_prefetch(va7 + 64); 
         for (i = 0; i < k; i += 2) {
#else
        
        for (i = 0; i < n/2; i += 2) {
#endif
            temp0 += v_x[i] * va0[i];
            temp1 += v_x[i] * va1[i];
            temp2 += v_x[i] * va2[i];
            temp3 += v_x[i] * va3[i];
            temp4 += v_x[i] * va4[i];
            temp5 += v_x[i] * va5[i];
            temp6 += v_x[i] * va6[i];
            temp7 += v_x[i] * va7[i];
            temp0 += v_x[i + 1] * va0[i + 1];
            temp1 += v_x[i + 1] * va1[i + 1];
            temp2 += v_x[i + 1] * va2[i + 1];
            temp3 += v_x[i + 1] * va3[i + 1];

            temp4 += v_x[i + 1] * va4[i + 1];
            temp5 += v_x[i + 1] * va5[i + 1];
            temp6 += v_x[i + 1] * va6[i + 1];
            temp7 += v_x[i + 1] * va7[i + 1];
        }
#if defined(PREFETCH)
        va0 += 64;
        va1 += 64;
        va2 += 64;
        va3 += 64;
        va4 += 64;
        va5 += 64;
        va6 += 64;
        va7 += 64;
        v_x += 64;

    }
#endif
    y[0] += alpha * (temp0[0] + temp0[1]);
    y[1] += alpha * (temp1[0] + temp1[1]);
    y[2] += alpha * (temp2[0] + temp2[1]);
    y[3] += alpha * (temp3[0] + temp3[1]);

    y[4] += alpha * (temp4[0] + temp4[1]);
    y[5] += alpha * (temp5[0] + temp5[1]);
    y[6] += alpha * (temp6[0] + temp6[1]);
    y[7] += alpha * (temp7[0] + temp7[1]);

}

#endif
 

static void dgemv_kernel_4x4(BLASLONG n, BLASLONG lda, FLOAT *ap, FLOAT *x, FLOAT *y, FLOAT alpha) {
    BLASLONG i = 0;
    FLOAT *a0, *a1, *a2, *a3;
    a0 = ap;
    a1 = ap + lda;
    a2 = a1 + lda;
    a3 = a2 + lda;
    __vector double* va0 = (__vector double*) a0;
    __vector double* va1 = (__vector double*) a1;
    __vector double* va2 = (__vector double*) a2;
    __vector double* va3 = (__vector double*) a3;
    __vector double* v_x = (__vector double*) x;
    register __vector double temp0 = {0, 0};
    register __vector double temp1 = {0, 0};
    register __vector double temp2 = {0, 0};
    register __vector double temp3 = {0, 0};
    register __vector double temp4 = {0, 0};
    register __vector double temp5 = {0, 0};
    register __vector double temp6 = {0, 0};
    register __vector double temp7 = {0, 0};

    for (i = 0; i < n / 2; i += 2) {
        temp0 += v_x[i] * va0[i];
        temp1 += v_x[i] * va1[i];
        temp2 += v_x[i] * va2[i];
        temp3 += v_x[i] * va3[i];
        temp4 += v_x[i + 1] * va0[i + 1];
        temp5 += v_x[i + 1] * va1[i + 1];
        temp6 += v_x[i + 1] * va2[i + 1];
        temp7 += v_x[i + 1] * va3[i + 1];
    }

    temp0 += temp4;
    temp1 += temp5;
    temp2 += temp6;
    temp3 += temp7;
    y[0] += alpha * (temp0[0] + temp0[1]);
    y[1] += alpha * (temp1[0] + temp1[1]);
    y[2] += alpha * (temp2[0] + temp2[1]);
    y[3] += alpha * (temp3[0] + temp3[1]);

}
 

static void dgemv_kernel_4x2(BLASLONG n, BLASLONG lda, FLOAT *ap, FLOAT *x, FLOAT *y, FLOAT alpha, BLASLONG inc_y) {

    BLASLONG i;
    FLOAT *a0, *a1;
    a0 = ap;
    a1 = ap + lda;
    __vector double* va0 = (__vector double*) a0;
    __vector double* va1 = (__vector double*) a1;
    __vector double* v_x = (__vector double*) x;
    __vector double temp0 = {0, 0};
    __vector double temp1 = {0, 0};
    for (i = 0; i < n / 2; i += 2) {
        temp0 += v_x[i] * va0[i] + v_x[i + 1] * va0[i + 1];
        temp1 += v_x[i] * va1[i] + v_x[i + 1] * va1[i + 1];
    }



    y[0] += alpha * (temp0[0] + temp0[1]);
    y[inc_y] += alpha * (temp1[0] + temp1[1]);
}

static void dgemv_kernel_4x1(BLASLONG n, FLOAT *ap, FLOAT *x, FLOAT *y, FLOAT alpha) {

    BLASLONG i;
    FLOAT *a0;
    a0 = ap;
    __vector double* va0 = (__vector double*) a0;
    __vector double* v_x = (__vector double*) x;
    __vector double temp0 = {0, 0};
    for (i = 0; i < n / 2; i += 2) {
        temp0 += v_x[i] * va0[i] + v_x[i + 1] * va0[i + 1];
    }

    *y += alpha * (temp0[0] + temp0[1]);

}

static void copy_x(BLASLONG n, FLOAT *src, FLOAT *dest, BLASLONG inc_src) {
    BLASLONG i;
    for (i = 0; i < n; i++) {
        *dest++ = *src;
        src += inc_src;
    }
}

int CNAME(BLASLONG m, BLASLONG n, BLASLONG dummy1, FLOAT alpha, FLOAT *a, BLASLONG lda, FLOAT *x, BLASLONG inc_x, FLOAT *y, BLASLONG inc_y, FLOAT *buffer) {
    BLASLONG i;
    BLASLONG j;
    FLOAT *a_ptr;
    FLOAT *x_ptr;
    FLOAT *y_ptr;

    BLASLONG n1;
    BLASLONG m1;
    BLASLONG m2;
    BLASLONG m3;
    BLASLONG n2; 
    FLOAT ybuffer[8] __attribute__((aligned(16)));
    FLOAT *xbuffer;

    if (m < 1) return (0);
    if (n < 1) return (0);

    xbuffer = buffer;

    n1 = n >> 3;
    n2 = n & 7;

    m3 = m & 3;
    m1 = m - m3;
    m2 = (m & (NBMAX - 1)) - m3;

    BLASLONG NB = NBMAX;

    while (NB == NBMAX) {

        m1 -= NB;
        if (m1 < 0) {
            if (m2 == 0) break;
            NB = m2;
        }

        y_ptr = y;
        a_ptr = a;
        x_ptr = x;

        if (inc_x != 1)
            copy_x(NB, x_ptr, xbuffer, inc_x);
        else
            xbuffer = x_ptr;

        BLASLONG lda8 = lda << 3;


        if (inc_y == 1) {

            for (i = 0; i < n1; i++) {
                 
                dgemv_kernel_4x8(NB, lda, a_ptr, xbuffer, y_ptr, alpha);
 
                y_ptr += 8;
                a_ptr += lda8;
#if defined(PREFETCH)                
               __builtin_prefetch(y_ptr+64);
#endif               
            }

        } else {
                   
            for (i = 0; i < n1; i++) {
                ybuffer[0] = 0;
                ybuffer[1] = 0;
                ybuffer[2] = 0;
                ybuffer[3] = 0;
                ybuffer[4] = 0;
                ybuffer[5] = 0;
                ybuffer[6] = 0;
                ybuffer[7] = 0;
                dgemv_kernel_4x8(NB, lda, a_ptr, xbuffer, ybuffer, alpha);

 

                *y_ptr += ybuffer[0];
                y_ptr += inc_y;
                *y_ptr += ybuffer[1];
                y_ptr += inc_y;
                *y_ptr += ybuffer[2];
                y_ptr += inc_y;
                *y_ptr += ybuffer[3];
                y_ptr += inc_y;

                *y_ptr += ybuffer[4];
                y_ptr += inc_y;
                *y_ptr += ybuffer[5];
                y_ptr += inc_y;
                *y_ptr += ybuffer[6];
                y_ptr += inc_y;
                *y_ptr += ybuffer[7];
                y_ptr += inc_y;

                a_ptr += lda8;
            }

        }


        if (n2 & 4) {
            ybuffer[0] = 0;
            ybuffer[1] = 0;
            ybuffer[2] = 0;
            ybuffer[3] = 0;
            dgemv_kernel_4x4(NB, lda, a_ptr, xbuffer, ybuffer, alpha);

            a_ptr += lda<<2;

            *y_ptr += ybuffer[0];
            y_ptr += inc_y;
            *y_ptr += ybuffer[1];
            y_ptr += inc_y;
            *y_ptr += ybuffer[2];
            y_ptr += inc_y;
            *y_ptr += ybuffer[3];
            y_ptr += inc_y;
        }

        if (n2 & 2) {
            dgemv_kernel_4x2(NB, lda, a_ptr, xbuffer, y_ptr, alpha, inc_y);
            a_ptr += lda << 1;
            y_ptr += 2 * inc_y;

        }

        if (n2 & 1) {
            dgemv_kernel_4x1(NB, a_ptr, xbuffer, y_ptr, alpha);
            a_ptr += lda;
            y_ptr += inc_y;

        }

        a += NB;
        x += NB * inc_x;


    }

    if (m3 == 0) return (0);

    x_ptr = x;
    a_ptr = a;
    if (m3 == 3) {
        FLOAT xtemp0 = *x_ptr * alpha;
        x_ptr += inc_x;
        FLOAT xtemp1 = *x_ptr * alpha;
        x_ptr += inc_x;
        FLOAT xtemp2 = *x_ptr * alpha;

        FLOAT *aj = a_ptr;
        y_ptr = y;

        if (lda == 3 && inc_y == 1) {

            for (j = 0; j < (n & -4); j += 4) {

                y_ptr[j] += aj[0] * xtemp0 + aj[1] * xtemp1 + aj[2] * xtemp2;
                y_ptr[j + 1] += aj[3] * xtemp0 + aj[4] * xtemp1 + aj[5] * xtemp2;
                y_ptr[j + 2] += aj[6] * xtemp0 + aj[7] * xtemp1 + aj[8] * xtemp2;
                y_ptr[j + 3] += aj[9] * xtemp0 + aj[10] * xtemp1 + aj[11] * xtemp2;
                aj += 12;
            }

            for (; j < n; j++) {
                y_ptr[j] += aj[0] * xtemp0 + aj[1] * xtemp1 + aj[2] * xtemp2;
                aj += 3;
            }

        } else {

            if (inc_y == 1) {

                BLASLONG register lda2 = lda << 1;
                BLASLONG register lda4 = lda << 2;
                BLASLONG register lda3 = lda2 + lda;

                for (j = 0; j < (n & -4); j += 4) {

                    y_ptr[j] += *aj * xtemp0 + *(aj + 1) * xtemp1 + *(aj + 2) * xtemp2;
                    y_ptr[j + 1] += *(aj + lda) * xtemp0 + *(aj + lda + 1) * xtemp1 + *(aj + lda + 2) * xtemp2;
                    y_ptr[j + 2] += *(aj + lda2) * xtemp0 + *(aj + lda2 + 1) * xtemp1 + *(aj + lda2 + 2) * xtemp2;
                    y_ptr[j + 3] += *(aj + lda3) * xtemp0 + *(aj + lda3 + 1) * xtemp1 + *(aj + lda3 + 2) * xtemp2;
                    aj += lda4;
                }

                for (; j < n; j++) {

                    y_ptr[j] += *aj * xtemp0 + *(aj + 1) * xtemp1 + *(aj + 2) * xtemp2;
                    aj += lda;
                }

            } else {

                for (j = 0; j < n; j++) {
                    *y_ptr += *aj * xtemp0 + *(aj + 1) * xtemp1 + *(aj + 2) * xtemp2;
                    y_ptr += inc_y;
                    aj += lda;
                }

            }

        }
        return (0);
    }

    if (m3 == 2) {
        FLOAT xtemp0 = *x_ptr * alpha;
        x_ptr += inc_x;
        FLOAT xtemp1 = *x_ptr * alpha;

        FLOAT *aj = a_ptr;
        y_ptr = y;

        if (lda == 2 && inc_y == 1) {

            for (j = 0; j < (n & -4); j += 4) {
                y_ptr[j] += aj[0] * xtemp0 + aj[1] * xtemp1;
                y_ptr[j + 1] += aj[2] * xtemp0 + aj[3] * xtemp1;
                y_ptr[j + 2] += aj[4] * xtemp0 + aj[5] * xtemp1;
                y_ptr[j + 3] += aj[6] * xtemp0 + aj[7] * xtemp1;
                aj += 8;

            }

            for (; j < n; j++) {
                y_ptr[j] += aj[0] * xtemp0 + aj[1] * xtemp1;
                aj += 2;
            }

        } else {
            if (inc_y == 1) {

                BLASLONG register lda2 = lda << 1;
                BLASLONG register lda4 = lda << 2;
                BLASLONG register lda3 = lda2 + lda;

                for (j = 0; j < (n & -4); j += 4) {

                    y_ptr[j] += *aj * xtemp0 + *(aj + 1) * xtemp1;
                    y_ptr[j + 1] += *(aj + lda) * xtemp0 + *(aj + lda + 1) * xtemp1;
                    y_ptr[j + 2] += *(aj + lda2) * xtemp0 + *(aj + lda2 + 1) * xtemp1;
                    y_ptr[j + 3] += *(aj + lda3) * xtemp0 + *(aj + lda3 + 1) * xtemp1;
                    aj += lda4;
                }

                for (; j < n; j++) {

                    y_ptr[j] += *aj * xtemp0 + *(aj + 1) * xtemp1;
                    aj += lda;
                }

            } else {
                for (j = 0; j < n; j++) {
                    *y_ptr += *aj * xtemp0 + *(aj + 1) * xtemp1;
                    y_ptr += inc_y;
                    aj += lda;
                }
            }

        }
        return (0);

    }

    FLOAT xtemp = *x_ptr * alpha;
    FLOAT *aj = a_ptr;
    y_ptr = y;
    if (lda == 1 && inc_y == 1) {
        for (j = 0; j < (n & -4); j += 4) {
            y_ptr[j] += aj[j] * xtemp;
            y_ptr[j + 1] += aj[j + 1] * xtemp;
            y_ptr[j + 2] += aj[j + 2] * xtemp;
            y_ptr[j + 3] += aj[j + 3] * xtemp;
        }
        for (; j < n; j++) {
            y_ptr[j] += aj[j] * xtemp;
        }


    } else {
        if (inc_y == 1) {

            BLASLONG register lda2 = lda << 1;
            BLASLONG register lda4 = lda << 2;
            BLASLONG register lda3 = lda2 + lda;
            for (j = 0; j < (n & -4); j += 4) {
                y_ptr[j] += *aj * xtemp;
                y_ptr[j + 1] += *(aj + lda) * xtemp;
                y_ptr[j + 2] += *(aj + lda2) * xtemp;
                y_ptr[j + 3] += *(aj + lda3) * xtemp;
                aj += lda4;
            }

            for (; j < n; j++) {
                y_ptr[j] += *aj * xtemp;
                aj += lda;
            }

        } else {
            for (j = 0; j < n; j++) {
                *y_ptr += *aj * xtemp;
                y_ptr += inc_y;
                aj += lda;
            }

        }
    }

    return (0);

}

