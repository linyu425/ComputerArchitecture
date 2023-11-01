#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint64_t get_cycles();
extern uint64_t get_instret();

void swap(int32_t *x, int32_t *y){
    int32_t t = *y;
    *y = *x;
    *x = t;
    return;
}
static inline int32_t getbit(int32_t value, int n)
{
    return (value >> n) & 1;
}
/* int32 multiply */
int32_t imul32(int32_t a, int32_t b)
{
    int32_t r = 0;
    while(b != 0){
        if (b & 1){
            r += a;
        }
        b = b >> 1;
        r = r >> 1;
    }
    r = r << 1;
    return r;
}
uint32_t count_leading_zeros(uint32_t x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);

    /* count ones (population count) */
    x -= ((x >> 1) & 0x55555555);
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    x = ((x >> 4) + x) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);

    return (32 - (x & 0x7F));
}

float unsigned_fadd32(float a,float b){
    int32_t ia = *(int32_t *)&a, ib = *(int32_t *)&b;

    int32_t a_tmp = ia & 0x7FFFFFFF;
    int32_t b_tmp = ib & 0x7FFFFFFF;

    if (a_tmp < b_tmp)
        swap(&ia, &ib);

    /* mantissa */
    int32_t ma = ia & 0x7FFFFF | 0x800000;
    int32_t mb = ib & 0x7FFFFF | 0x800000;
    /* exponent */
    int32_t ea = (ia >> 23) & 0xFF;
    int32_t eb = (ib >> 23) & 0xFF;

    int32_t align = (ea - eb > 24) ? 24 : (ea - eb);

    mb >>= align;
    if ((ia ^ ib) >> 31) {
        ma -= mb;
    } else {
        ma += mb;
    }

    int32_t clz = count_leading_zeros(ma);
    int32_t shift = 0;
    if (clz <= 8) {
        shift = 8 - clz;
        ma >>= shift;
        ea += shift;
    } else {
        shift = clz - 8;
        ma <<= shift;
        ea -= shift;
    }

    int32_t r = ia & 0x80000000 | ea << 23 | ma & 0x7FFFFF;
    return *(float *) &r;
}
/* float32 multiply */
float fmul32(float a, float b)
{
    int32_t ia = *(int32_t *) &a, ib = *(int32_t *) &b;

    /* sign */
    int sa = ia >> 31;
    int sb = ib >> 31;

    /* mantissa */
    int32_t ma = (ia & 0x7FFFFF) | 0x800000;
    int32_t mb = (ib & 0x7FFFFF) | 0x800000;

    /* exponent */
    int32_t ea = ((ia >> 23) & 0xFF);
    int32_t eb = ((ib >> 23) & 0xFF);

    /* 'r' = result */
    int32_t mrtmp = imul32(ma, mb);
    int mshift = getbit(mrtmp, 24);

    int32_t mr = mrtmp >> mshift;
    int32_t ertmp = ea + eb - 127;
    // int32_t er = mshift ? inc(ertmp) : ertmp;
    int32_t er = mshift + ertmp;

    int sr = sa ^ sb;
    int32_t r = (sr << 31) | ((er & 0xFF) << 23) | (mr & 0x7FFFFF);
    return *(float *) &r;
}

#define WORDS 12
#define ROUNDS 7

int main(void)
{
    unsigned int state[WORDS] = {0};

    /* measure cycles */
    uint64_t instret = get_instret();
    uint64_t oldcount = get_cycles();
    
    float image[3][3][3] = {{{0.90251149,0.03265091,0.8831173},{0.2139775,0.0737501,0.0399187},{0.21527551,0.8881527,0.7846363}},
    {{0.938326,0.64254336,0.0461617},{0.1413221,0.3307385,0.2508785},{0.3833867,0.689476,0.41071482}},
    {{0.8925364,0.1480669,0.6812473},{0.9288288,0.23190344,0.3070017},{0.6414362,0.34707349,0.5142535}}};
    float grayscale_image[3][3];
    for(int i=0;i<3;i=i+1){
        for(int j=0;j<3;j=j+1){
            grayscale_image[i][j] = unsigned_fadd32(unsigned_fadd32(fmul32(image[i][j][0], 0.299) , fmul32(image[i][j][1], 0.587)) , fmul32(image[i][j][2], 0.114));
        }
    }
    for(int i=0;i<3;i=i+1){
        for(int j=0;j<3;j=j+1){
            printf("%f ",grayscale_image[i][j]);
        }
        printf("\n");
    }
    
    uint64_t cyclecount = get_cycles() - oldcount;

    printf("cycle count: %u\n", (unsigned int) cyclecount);
    printf("instret: %x\n", (unsigned) (instret & 0xffffffff));

    memset(state, 0, WORDS * sizeof(uint32_t));

    return 0;
}
