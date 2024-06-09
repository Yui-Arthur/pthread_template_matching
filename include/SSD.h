#ifndef _SSD_H_
#define _SSD_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "config.h"
#include <pthread.h>
#include <sys/time.h>

float SSD(int *t, int *s, int rows, int columns, int kernel_size, int *res_pcc);
float SSD_pthread(int *t, int *s, int rows, int columns, int kernel_size, int *res_pcc);

typedef struct _SSD_calculate_arg
{
    int *t;
    int *s;
    int *z;
    int rows;
    int columns;
    int kernel_size;
    int row_st;
    int row_ed;
} SSD_calculate_arg;

void* SSD_calculate(void *arg);

float SSD(int *t, int *s, int rows, int columns, int kernel_size, int *res_pcc){
    printf("============== SSD CPU ==============\n");
    clock_t func_start = clock();

    clock_t cal_start = clock();
    for(int r=0; r<rows-kernel_size+1; r++){
        for(int c=0; c<columns-kernel_size+1; c++){
            int res = 0;
            for(int i=0; i<kernel_size; i++){
                for(int j=0; j<kernel_size; j++){
                    int val = t[(r+i)* columns + c+j] - s[i*kernel_size + j];
                    res += val*val;
                }
            }  
            
            res_pcc[r*columns + c] = res;
        }
    }
    clock_t cal_end = clock();
    clock_t func_end = clock();
    float function_elapsedTime = (func_end-func_start)/(double)(CLOCKS_PER_SEC);
    float cal_elapsedTime = (cal_end-cal_start)/(double)(CLOCKS_PER_SEC);
    printf("SSD Total Function Time : %10.10f ms\n", function_elapsedTime * 1000);
    printf("SSD Calculation Time on CPU: %10.10f ms\n", cal_elapsedTime * 1000) ;
    printf("=====================================\n");

    return cal_elapsedTime * 1000;
}

float SSD_pthread(int *t, int *s, int rows, int columns, int kernel_size, int *res_pcc){
    printf("============== SSD pthread ==============\n");
    
    pthread_t task[THREAD_COUNT];
    SSD_calculate_arg arg[THREAD_COUNT];

    int row_per_task = (rows / THREAD_COUNT) + 1;

    for(int i=0; i<THREAD_COUNT; i++){
        arg[i] = (SSD_calculate_arg) {
            .t = t,
            .s = s,
            .z = res_pcc,
            .rows = rows,
            .columns = columns,
            .kernel_size = kernel_size,
            .row_st = row_per_task*i,
            .row_ed = row_per_task*(i+1)
        };
        printf("%d ~ %d\n",arg[i].row_st, arg[i].row_ed);
    }

    struct timespec start, finish;
    double elapsed;
    printf("row_per_task %d\n", row_per_task);
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for(int i=0; i<THREAD_COUNT; i++) pthread_create(&task[i], NULL, SSD_calculate, &arg[i]);
    for(int i=0; i<THREAD_COUNT; i++){
        pthread_join(task[i], NULL);
    } 

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;


    printf("SSD Total Function Time : %10.10f ms\n", elapsed * 1000);

    printf("======================================\n");
    return elapsed * 1000;
}

void* SSD_calculate(void *arg){

    SSD_calculate_arg *p_arg = (SSD_calculate_arg*) arg;
    int total_kernel_size = p_arg->kernel_size * p_arg->kernel_size;


    for(int r = p_arg->row_st; r < p_arg->row_ed; r++){
        if(r + p_arg->kernel_size > p_arg->rows) continue;
        for(int c=0; c < p_arg->columns; c++){
            if(c + p_arg->kernel_size > p_arg->columns) continue;

            int res = 0;
            for(int i=0; i < p_arg->kernel_size; i++){
                for(int j=0; j < p_arg->kernel_size; j++){
                    int val = p_arg->t[(r+i)* p_arg->columns + c+j] - p_arg->s[i* p_arg->kernel_size + j];
                    res += val*val;
                }
            }

            p_arg->z[r* p_arg->columns + c] = res;
        }
    }
    pthread_exit(NULL);
}

#endif