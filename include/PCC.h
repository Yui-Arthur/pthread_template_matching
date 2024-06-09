#ifndef _PCC_H_
#define _PCC_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "config.h"
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

float PCC(int *, int *, int, int, int, float*);
float PCC_faster(int *, int *, int, int, int, float*);
float PCC_pthread(int *t, int *s, int rows, int columns, int kernel_size, float*);
typedef struct _PCC_calculate_arg
{
    int *t;
    int *s;
    float *z;
    int rows;
    int columns;
    int kernel_size;
    int row_st;
    int row_ed;
} PCC_calculate_arg;

void* PCC_calculate(void *);



float PCC(int *t, int *s, int rows, int columns, int kernel_size, float *res_pcc){
    printf("============== PCC CPU ==============\n");
    clock_t func_start = clock();
    int sum_s = 0;
    float avg_s = 0;
    float *s_fp = (float*) malloc( kernel_size * kernel_size * sizeof(float));
    float res_x = 0, res_y = 0, res_z = 0;
    /* cal avg s, s - avg s, res_s */
    for(int i = 0; i < kernel_size; i++) for(int j = 0; j < kernel_size; j++) sum_s += s[i*kernel_size + j];
    avg_s =  sum_s / (float)(kernel_size*kernel_size);
    for(int i = 0; i < kernel_size; i++) {
        for(int j = 0; j < kernel_size; j++){
            s_fp[i*kernel_size + j] = s[i*kernel_size + j] - avg_s;
            res_y += s_fp[i*kernel_size + j] * s_fp[i*kernel_size + j];
        } 
    }
    printf("avg s %.5f\n", avg_s);
    res_y = sqrt(res_y);

    float avg_t = 0;
    int sum_t = 0;

    clock_t cal_start = clock();
    for(int r=0; r<rows-kernel_size+1; r++){
        for(int c=0; c<columns-kernel_size+1; c++){
            sum_t = 0;
            res_x = 0, res_z = 0, avg_t = 0;
            for(int i=0; i<kernel_size; i++) for(int j=0; j<kernel_size; j++) sum_t += t[(r+i)* columns + c+j];
            avg_t = sum_t / (float)(kernel_size*kernel_size);

            for(int i=0; i<kernel_size; i++){
                for(int j=0; j<kernel_size; j++){
                    float t_fp = t[(r+i)* columns + c+j] - avg_t;
                    res_x += t_fp * t_fp;
                    res_z += t_fp * s_fp[i * kernel_size + j];
                }
            }
            res_pcc[r*columns + c] = res_z / (sqrt(res_x) * res_y);
        }
    }
    clock_t cal_end = clock();
    clock_t func_end = clock();
    float function_elapsedTime = (func_end-func_start)/(double)(CLOCKS_PER_SEC);
    float cal_elapsedTime = (cal_end-cal_start)/(double)(CLOCKS_PER_SEC);
    printf("PCC Total Function Time : %10.10f ms\n", function_elapsedTime * 1000);
    printf("PCC Calculation Time on CPU: %10.10f ms\n", cal_elapsedTime * 1000) ;
    printf("=====================================\n");

    return cal_elapsedTime * 1000;
}

float PCC_faster(int *t, int *s, int rows, int columns, int kernel_size, float *res_pcc){
    printf("============== PCC Faster CPU ==============\n");
    clock_t func_start = clock();
    int sum_s = 0;
    float avg_s = 0;
    float res_y = 0;
    /* cal avg s, s - avg s, res_s */
    for(int i = 0; i < kernel_size; i++) for(int j = 0; j < kernel_size; j++) sum_s += s[i*kernel_size + j];
    avg_s =  sum_s / (float)(kernel_size*kernel_size);
    for(int i = 0; i < kernel_size; i++) {
        for(int j = 0; j < kernel_size; j++){
            res_y += (s[i*kernel_size + j] - avg_s) * (s[i*kernel_size + j] - avg_s);
        } 
    }
    res_y = sqrt(res_y);
    printf("avg s %.5f\n", avg_s);
    printf("res y : %.5f\n", res_y);

    int total_kernel_size = kernel_size * kernel_size ;

    clock_t cal_start = clock();
    for(int r=0; r<rows-kernel_size+1; r++){
        for(int c=0; c<columns-kernel_size+1; c++){
            
            int sum_t = 0, sum_pow_t = 0, sum_ts = 0;
            for(int i=0; i<kernel_size; i++){
                for(int j=0; j<kernel_size; j++){
                    int curr_t_value = t[(r+i)*columns + c+j], curr_s_value = s[i*kernel_size + j];
                    sum_t += curr_t_value;
                    sum_pow_t += curr_t_value * curr_t_value;
                    sum_ts += curr_t_value * curr_s_value;
                }
            }
            // printf("(%d , %d , %d)", sum_t, sum_pow_t, sum_ts);
            float avg_t = sum_t / (float)(total_kernel_size);
            float res_z = sum_ts - avg_t*sum_s - avg_s*sum_t + total_kernel_size*avg_s*avg_t;
            float res_x = sum_pow_t + avg_t * (total_kernel_size*avg_t - 2*sum_t);
            res_pcc[r*columns + c] = res_z / sqrt(res_x) / res_y;   
            // printf("%.3f ", res_pcc[r*columns + c]);
        }
        // printf("\n");
    }
    clock_t cal_end = clock();
    clock_t func_end = clock();
    float function_elapsedTime = (func_end-func_start)/(double)(CLOCKS_PER_SEC);
    float cal_elapsedTime = (cal_end-cal_start)/(double)(CLOCKS_PER_SEC);
    printf("PCC Faster Total Function Time : %10.10f ms\n", function_elapsedTime * 1000);
    printf("PCC Faster Calculation Time on CPU: %10.10f ms\n", cal_elapsedTime * 1000);
    printf("=====================================\n");

    return cal_elapsedTime * 1000;
}

float PCC_pthread(int *t, int *s, int rows, int columns, int kernel_size, float *res_pcc){
    printf("============== PCC pthread ==============\n");
    
    
    pthread_t task[THREAD_COUNT];
    PCC_calculate_arg arg[THREAD_COUNT];

    int row_per_task = (rows / THREAD_COUNT) + 1;

    for(int i=0; i<THREAD_COUNT; i++){
        arg[i] = (PCC_calculate_arg) {
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
    
    for(int i=0; i<THREAD_COUNT; i++) pthread_create(&task[i], NULL, PCC_calculate, &arg[i]);
    for(int i=0; i<THREAD_COUNT; i++){
        pthread_join(task[i], NULL);
    } 

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;


    printf("PCC Total Function Time : %10.10f ms\n", elapsed * 1000);
    // printf("PCC Calculation Time on pthread: %10.10f ms\n", cal_elapsedTime * 1000);

    printf("======================================\n");
    return elapsed * 1000;
}

void* PCC_calculate(void *arg){

    PCC_calculate_arg *p_arg = (PCC_calculate_arg*) arg;
    int total_kernel_size = p_arg->kernel_size * p_arg->kernel_size;

    int sum_s = 0;
    float avg_s = 0;
    float res_y = 0;
    /* cal avg s, s - avg s, res_s */
    for(int i = 0; i < p_arg->kernel_size; i++) for(int j = 0; j < p_arg->kernel_size; j++) sum_s += p_arg->s[i*p_arg->kernel_size + j];
    avg_s =  sum_s / (float)(total_kernel_size);
    for(int i = 0; i < p_arg->kernel_size; i++) {
        for(int j = 0; j < p_arg->kernel_size; j++){
            res_y += (p_arg->s[i*p_arg->kernel_size + j] - avg_s) * (p_arg->s[i*p_arg->kernel_size + j] - avg_s);
        } 
    }
    res_y = sqrt(res_y);

    for(int r = p_arg->row_st; r < p_arg->row_ed; r++){
        if(r + p_arg->kernel_size > p_arg->rows) continue;
        for(int c=0; c < p_arg->columns; c++){
            if(c + p_arg->kernel_size > p_arg->columns) continue;
    
            int sum_t = 0, sum_pow_t = 0, sum_ts = 0;
            for(int i=0; i < p_arg->kernel_size; i++){
                for(int j=0; j < p_arg->kernel_size; j++){
                    int curr_t_value = p_arg->t[(r+i) * p_arg->columns + c+j], curr_s_value = p_arg->s[i * p_arg->kernel_size + j];
                    sum_t += curr_t_value;
                    sum_pow_t += curr_t_value * curr_t_value;
                    sum_ts += curr_t_value * curr_s_value;
                }
            }
            float avg_t = sum_t / (float)(total_kernel_size);
            float res_z = sum_ts - avg_t*sum_s - avg_s*sum_t + total_kernel_size*avg_s*avg_t;
            float res_x = sum_pow_t + avg_t * (total_kernel_size*avg_t - 2*sum_t);
            p_arg->z[r* p_arg->columns + c] = res_z / sqrt(res_x) / res_y;   
        }
    }
    pthread_exit(NULL);
}


#endif