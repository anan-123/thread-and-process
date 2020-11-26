#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

typedef struct index index;
struct index 
{
    int l,r;
};
pthread_mutex_t mutex;
int arr3[100000],base[100000];

void merge(int arr[], int l, int m, int r)
{
    int n1=m-l+1,n2=r-m;
    int l_arr[n1],r_arr[n2];

    for(int i=0;i<n1;i++)
    {
        l_arr[i] = arr[l+i];
    }
    for(int j=0;j<n2;j++)
    {
        r_arr[j]=arr[m+j+1];
    }
    int i=0,j=0,k=l;
    while(i<n1 && j<n2)
    {
        if(l_arr[i] <= r_arr[j])
        {
            arr[k++] = l_arr[i++];
        }
        else
        {
            arr[k++] = r_arr[j++];
        }
        
    }
    while(i<n1)
    {
        arr[k++]=l_arr[i++];
    }
    
    while(j<n2)
    {
        arr[k++]=r_arr[j++];
    }

}

void normal_merge_sort(int arr[],int l,int r)
{
    
   int n=r-l+1;
    //selection sort
    if(n<6)
    {
        int x,y;
        for(int i=0;i<n;i++)
        {
            x = l+i;
            for(int j=i+1;j<n;j++)
            {
                if(arr[l+j]<arr[x])
                {
                    x=l+j;
                }
            }
            y=arr[x];
            arr[x]=arr[l+i];
            arr[l+i]=y;
        }
        return;
    }

    if(l<r)
    {
        int m = l+ (r-l)/2;
        normal_merge_sort(arr,l,m);
        normal_merge_sort(arr,m+1,r);
        merge(arr,l,m,r);
    }
}

void process_merge_sort(int arr[],int l,int r)
{
   int n=r-l+1;
    //selection sort
    if(n<6)
    {
        int x,y;
        for(int i=0;i<n;i++)
        {
            x = l+i;
            for(int j=i+1;j<n;j++)
            {
                if(arr[l+j]<arr[x])
                {
                    x=l+j;
                }
            }
            y=arr[x];
            arr[x]=arr[l+i];
            arr[l+i]=y;
        }
    }
    else
    {
        pid_t left_id,right_id;
        int status;
        left_id = fork();
        if(left_id==0)
        {
            process_merge_sort(arr,l,l+r/2-1);
            _exit(0);
        }
        else
        {
            right_id = fork();
            if(right_id==0)
            {
                process_merge_sort(arr,l+r/2,r);
                _exit(0);
            }
            
        }
        waitpid(left_id,&status,0);
        waitpid(right_id,&status,0);
        merge(arr,l,l+r/2-1,r);


    }
    
}

void * thread_merge_sort(void * base)
{
    index * element=(index *)base;
    int l,r;
    l=element->l;
    r=element->r;
    if(l<r)
    {
         int n=r-l+1;
        //selection sort
        if(n<6)
        {
            pthread_mutex_lock(&mutex);
            int x,y;
            for(int i=0;i<n;i++)
            {
                x = l+i;
                for(int j=i+1;j<n;j++)
                {
                    if(arr3[l+j]<arr3[x])
                    {
                        x=l+j;
                    }
                }
                y=arr3[x];
                arr3[x]=arr3[l+i];
                arr3[l+i]=y;
            }
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }
        index left,right;
        int mid=(l+r)/2;
        pthread_t left_id,right_id;
        left.l=l;
        left.r=mid;
        right.l=mid+1;
        right.r=r;
        pthread_create(&left_id,NULL,thread_merge_sort,&left);
        pthread_create(&right_id,NULL,thread_merge_sort,&right);
        pthread_join(left_id,NULL);
        pthread_join(right_id,NULL);
        pthread_mutex_lock(&mutex);
        merge(arr3,l,mid,r);
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
    }
    else
    {
        pthread_exit(0);
    }
    
}

void printArray(int A[], int size) 
{ 
    for(int i = 0; i < size; i++) 
        printf("%d ",A[i]);

    printf("\n");
} 
int main()
{
    struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     long double st = ts.tv_nsec/(1e9)+ts.tv_sec;
    //input
    int n,*arr1;
    printf("enter the number of elements of array\n");
    scanf("%d",&n);
    int arr2[n];
    int shmid;
    key_t key = IPC_PRIVATE;
    shmid=shmget(key,sizeof(int)*n,IPC_CREAT | 0666);
    arr1 =shmat(shmid,NULL,0);
    printf("enter the array elements\n");
    for(int i=0;i<n;i++)
    {
        scanf("%d",&arr1[i]);
    }
    for(int i=0;i<n;i++)
    {
        arr2[i]=arr1[i];
        arr3[i]=arr1[i];
    }

    //normal merge sort 
    printf("executing normal merge sort\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    normal_merge_sort(arr2,0,n-1);
    printf("the sorted array is :\n");
    printArray(arr2,n); 
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
     printf("time = %Lf\n", en - st);
     long double t1 = en-st;

    //merge sort using system calls
   
    printf("executing merge sort as a process\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    process_merge_sort(arr1,0,n-1);
    printf("the sorted array is :\n");
    printArray(arr1,n); 
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     en = ts.tv_nsec/(1e9)+ts.tv_sec;
     printf("time = %Lf\n", en - st);
     long double t2 = en-st;

    //merge sort using threads
    index base;
    base.l=0;
    base.r=n-1;
    pthread_t tid;
    printf("executing merge sort using threads\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    pthread_create(&tid,NULL,thread_merge_sort,&base);
    pthread_join(tid,NULL);
    printf("the sorted array is :\n");
    printArray(arr3,n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;
    printf("normal_mergesort ran:\n\t[ %Lf ] times faster than concurrent_mergesort\n\t[ %Lf ] times faster than threaded_concurrent_mergesort\n\n\n", t1/t3, t2/t3);
    return 0;
}
