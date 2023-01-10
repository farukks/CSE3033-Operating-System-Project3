
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include "Vector.h"

pthread_mutex_t lock;

int doneRead = 0;
int doneAllReadThreads = 0;
vector data;  // global queue

char filePath[100];

typedef struct MData{
    char line[3080];
    char line_origin[3080];
    int index;
} MData;

MData* createMData(char* line,int index){
    MData* f = (struct MData*)malloc( sizeof(struct MData));
    // f->line = line;
    strcpy(f->line,line);
    strcpy(f->line_origin,line);

    f->line[strlen(f->line)-1] = '\0';
    f->line_origin[strlen(f->line_origin)-1] = '\0';


    f->index = index;
    return f;
}

typedef struct MParams{
    
    int currentLine;
    int threadId;

    // read
    int numberOfReadThreads;


    // uper
    int numberOfUpperThreads;

    // replace
    int numberOfReplaceThreads;

// write
    int numberOfWriteThreads;


}MParams;

MParams* createMParams(){
    MParams* f = (struct MParams*)malloc( sizeof(struct MParams));
    f->currentLine = 0;
    return f;
}

void *replaceThread(void* params);
void *replaceMainThread(void* params);

void *upperMainThread(void* params);
void *upperThread(void* params);

void *readThread(void* params);
void *readMainThread(void* params);

void *writeThread(void* params);
void *writeMainThread(void* params);





void mainThread(int numberOfReadThreads,int numberOfUpperThreads,int numberOfReplaceThreads,int numberOfWriteThreads);




void mainThread(int numberOfReadThreads,int numberOfUpperThreads,int numberOfReplaceThreads,int numberOfWriteThreads){

    vector_init(&data);

    pthread_t readMainThread_id;
    MParams* readParam = createMParams();
    readParam->numberOfReadThreads = numberOfReadThreads;
    pthread_create(&readMainThread_id, NULL, readMainThread,  (void *)readParam);

    pthread_t upperMainThread_id;
    MParams* upperParam = createMParams();
    upperParam->numberOfUpperThreads = numberOfUpperThreads;
    pthread_create(&upperMainThread_id, NULL, upperMainThread,  (void *)upperParam);

    pthread_t replaceMainThread_id;
    MParams* replaceParam = createMParams();
    replaceParam->numberOfReplaceThreads = numberOfReplaceThreads;
    pthread_create(&replaceMainThread_id, NULL, replaceMainThread,  (void *)replaceParam);

    pthread_t writeMainThread_id;
    MParams* writeParam = createMParams();
    writeParam->numberOfWriteThreads = numberOfWriteThreads;
    pthread_create(&writeMainThread_id, NULL, writeMainThread,  (void *)writeParam);

    pthread_join(readMainThread_id, NULL);
    pthread_join(upperMainThread_id, NULL);
    pthread_join(replaceMainThread_id, NULL);
    pthread_join(writeMainThread_id, NULL);
}

////////// write
void *writeMainThread(void* params){
    MParams* mParams = (MParams*)params;

    pthread_t thread_id[100];
    int currentLine = 0;

    while (1)
    {
        for(int i = 0 ; i < mParams->numberOfWriteThreads;i++){
            MParams* mParams = createMParams();
            mParams->currentLine = currentLine;
            mParams->threadId = i;
            currentLine++;
            pthread_create(&thread_id[i], NULL, writeThread,  (void *)mParams);
        }
        for(int i = 0 ; i < mParams->numberOfWriteThreads;i++){
            pthread_join(thread_id[i], NULL);
        }
        if(doneAllReadThreads == 1 && currentLine >= vector_total(&data)){ // all done
            break;
        }
    }
}

void *writeThread(void* params){
    MParams* mParams = (MParams*)params;

    MData* mData = NULL;
    while (mData == NULL)
    {
        if(doneAllReadThreads == 1 && mParams->currentLine >= vector_total(&data)){
            return NULL;
        }
        // pthread_mutex_lock(&lock);
        mData = (MData*)vector_get(&data,mParams->currentLine);
        // pthread_mutex_unlock(&lock);
    }

    while (1) // check line
    {
        int formated = 1;
        int i = 0;
        while (mData->line[i] != '\0')
        {
            if(mData->line[i] == ' '){
                formated = 0;
            }
            if(mData->line[i] >= 'a' && mData->line[i] <= 'z'){
                formated = 0;
            }
            i++;
        }
        if(formated == 1){
            break;
        }
    }


    pthread_mutex_lock(&lock);
    char replaceCommand[6273];
    snprintf(replaceCommand, sizeof(replaceCommand), "sed -i 's/%s/%s/g' %s", mData->line_origin,mData->line,filePath);

    system(replaceCommand);

    printf("Write_%d: ",mParams->threadId);
    printf("%s\n", mData->line);
    // sed -i 's/old-text/new-text/g' input.txt
    pthread_mutex_unlock(&lock);
}


//////////////// replace
void *replaceMainThread(void* params){
    MParams* mParams = (MParams*)params;

    pthread_t thread_id[100];
    int currentLine = 0;

    while (1)
    {
        for(int i = 0 ; i < mParams->numberOfReplaceThreads;i++){
            MParams* mParams = createMParams();
            mParams->currentLine = currentLine;
            mParams->threadId = i;
            currentLine++;
            pthread_create(&thread_id[i], NULL, replaceThread,  (void *)mParams);
        }
        for(int i = 0 ; i < mParams->numberOfReplaceThreads;i++){
            pthread_join(thread_id[i], NULL);
        }
        if(doneAllReadThreads == 1 && currentLine >= vector_total(&data)){ // all done
            break;
        }
    }
}
void *replaceThread(void* params){
    MParams* mParams = (MParams*)params;

    MData* mData = NULL;
    while (mData == NULL)
    {
        if(doneAllReadThreads == 1 && mParams->currentLine >= vector_total(&data)){
            return NULL;
        }
        // pthread_mutex_lock(&lock);
        mData = (MData*)vector_get(&data,mParams->currentLine);
        // pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
   
    int i = 0;

    while (mData->line[i] != '\0')
    {
        if(mData->line[i] == ' '){
            mData->line[i] = '_';
        }
        i++;
    }
    // printf("c : %c\n",mData->line[0]);
    printf("Replace_%d: ",mParams->threadId);
    printf("%s\n", mData->line);
    pthread_mutex_unlock(&lock);
}


//////////////// upper
void *upperMainThread(void* params){
    MParams* mParams = (MParams*)params;

    pthread_t thread_id[100];
    int currentLine = 0;

    while (1)
    {
        for(int i = 0 ; i < mParams->numberOfUpperThreads;i++){
            MParams* mParams = createMParams();
            mParams->currentLine = currentLine;
            mParams->threadId = i;
            currentLine++;
            pthread_create(&thread_id[i], NULL, upperThread,  (void *)mParams);
        }
        for(int i = 0 ; i < mParams->numberOfUpperThreads;i++){
            pthread_join(thread_id[i], NULL);
        }
        if(doneAllReadThreads == 1 && currentLine >= vector_total(&data)){ // all done
            break;
        }
    }
}

void *upperThread(void* params){
    MParams* mParams = (MParams*)params;

    MData* mData = NULL;
    while (mData == NULL)
    {
        if(doneAllReadThreads == 1 && mParams->currentLine >= vector_total(&data)){
            return NULL;
        }
        // pthread_mutex_lock(&lock);
        mData = (MData*)vector_get(&data,mParams->currentLine);
        // pthread_mutex_unlock(&lock);
    }
    pthread_mutex_lock(&lock);
    int i = 0;
    while (mData->line[i] != '\0')
    {
        mData->line[i] = toupper(mData->line[i]);
        i++;
    }
    printf("Upper_%d: ",mParams->threadId);
    printf("%s\n", mData->line);
    pthread_mutex_unlock(&lock);
}

//////////////////// READ
void *readMainThread(void* params){
    MParams* mParams = (MParams*)params;

    pthread_t thread_id[100];

    int currentLine = 0;
    while (doneRead == 0)
    {
        for(int i = 0 ; i < mParams->numberOfReadThreads;i++){
            MParams* mParams = createMParams();
            mParams->currentLine = currentLine;
            mParams->threadId = i;
            currentLine++;
            pthread_create(&thread_id[i], NULL, readThread,  (void *)mParams);
        }
        for(int i = 0 ; i < mParams->numberOfReadThreads;i++){
            pthread_join(thread_id[i], NULL);
        }
    }
    doneAllReadThreads = 1;
    // printf("\n\ntotal : %d\n",vector_total(&data));
}

void *readThread(void* params){
    MParams* mParams = (MParams*)params;
    char buffer[3080]; 

    FILE* filePointer;
    filePointer = fopen(filePath, "r");

    int currentLine = 0;
    // if(mParams->currentLine == 1 || mParams->currentLine == 5){
    //     sleep(2);
    // }
    while(fgets(buffer, 3080, filePointer)){
        if(currentLine == mParams->currentLine){
            pthread_mutex_lock(&lock);

            printf("Read_%d: ",mParams->threadId);
            printf("%s\n", buffer);
            fclose(filePointer);

            MData* mData = createMData(buffer,mParams->currentLine);
            vector_add(&data,mData);
            pthread_mutex_unlock(&lock);

            return NULL;
        }
        currentLine++;
    }

    if(currentLine >= mParams->currentLine){
        doneRead = 1;
    }
    fclose(filePointer);
}

