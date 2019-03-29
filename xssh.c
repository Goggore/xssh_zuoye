#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#define BUFLEN 128
#define SIZE 5
char* content = NULL; //save content for show commands.
/*-------------------------Initialize ArrayList-------------------------*/
typedef struct{
    int size;
    int capacity;
    int volume;
    char** list;
}ArrayList;

ArrayList* AL_init(){
    ArrayList* a =malloc(sizeof(ArrayList));
    if(!a){
        perror("Out of memory!");
        exit(1);
    }
    a->capacity=SIZE;
    a->list=malloc(sizeof(char*)*(a->capacity));
    if(!(a->list)){
        perror("Out of memory!");
        exit(1);
    }
    a->size=0;
    a->volume=0;
    return a;
};


void realCheck(char** tmp, ArrayList* a){
    if(tmp){
        a->list = tmp;
    }
    else{
        perror("resize fails");
        exit(1);
    }
}

void resize(ArrayList* a, int flag){

    if (flag==0){
        char** tmp = realloc(a->list,sizeof(char*)*(2*a->capacity));
        realCheck(tmp,a);
        a->capacity*=2;
    }
    else{
        char** tmp = realloc(a->list,(sizeof(char*)*(size_t)(0.6*a->capacity)));
        realCheck(tmp,a);
        a->capacity*=0.6;
    }
}

/*use this function if you have string that has local scope(like array chars) needed to be stored or you want it mutable. 
 * An empty string(contains only null characters) will be treated in a different way: 
 * The actual data will not be stored , instead the function will add a null pointer in the AL to keep its existence. 
 * You may call dump() to collect the memory allocated in this function.*/
void writeData(ArrayList* a, char* str){
    if ((a->size + 1) > a->capacity) {
        resize(a, 0);
    } else {
        while ((a->size + 5) < (0.4 * (a->capacity))) {
            resize(a, -1);
        }
    }
    if(strcmp(str,"")) {
        char *newStr = (char *) malloc(sizeof(char) * (strlen(str) + 1));
        if (!newStr) {
            perror("Out of memory!");
            exit(1);
        }
        strcpy(newStr, str);
        a->list[a->size] = newStr;
        a->size++;
        a->volume += (strlen(str) + 1);
    } else{
        a->list[a->size] = 0; //set as a null pointer if the incoming str is null character
        a->size++;
    }
}

/*
 * @Description: This function offers a more generic storage:
 * Use this function if you have a pointer already points to a allocated memory or
 * it doesn't point to a string. Be aware that it may cause a segfault if the data
 * goes out of scope. You should not call dump() to collect memory, since the memory
 * is never allocated by this function. Instead, call destroy() to collect your memory.
 */
int store(ArrayList* a, void* p){
    a->volume=-1;
    if ((a->size + 1) > a->capacity) {
        resize(a, 0);
    } else {
        while ((a->size + 5) < (0.4 * (a->capacity))) {
            resize(a, -1);
        }
    }
    a->list[a->size] = (char*)p;
    a->size++;
    return 0;
}

void destroy(ArrayList* a){//don't use this function unless you've used store() to store your data.
    free(a->list);
    free(a);
}

void showList(ArrayList* a){

    printf("List Content: \n");
    for(int i=0;i < a->size;i++){
        if(a->list[i])printf("%d.%s ",(i+1),a->list[i]);

    }
    printf("\nList Size: %d\n",a->size);
    printf("List Capacity: %d\n", a->capacity);
    printf("List Volume: %d char(s).\n", a->volume);

}

int removeIndex(ArrayList* a,int i){
    if(i >= 0 ){ //list[i] is not a null pointer
        if(a->list[i]){
            a->volume-= (strlen(a->list[i])+1);
            free(a->list[i]);
            while(i<a->size-1){
                a->list[i] = a->list[i+1];
                i++;
            }
            a->size-=1;
            if((a->size+5)<(0.4*(a->capacity))){
                resize(a,1);
            }
        } else{
            while(i<a->size-1){
                a->list[i] = a->list[i+1];
                i++;
            }
            a->size-=1;
            if((a->size+5)<(0.4*(a->capacity))){
                resize(a,1);
            }
        }
        return 0;
    }
    return -1;
}

char* next(ArrayList* a){
    static int position = 0;
    if(position < a->size){
        char* tmp = a->list[position];
        position++;
        return tmp;
    }
    return NULL;
}

int assert(ArrayList* a, char* str){
    for(int i=0; i < a->size;i++) {
        if (!strcmp(a->list[i], str)) {
            return i;
        }
    }
    return -1;
}

int dump(ArrayList* a){
    while(a->size!=0){
        removeIndex(a,0);
    }
    return 0;
}

int removeData(ArrayList* a, char* str){
    int i = assert(a, str);
    return removeIndex(a,i);
}


/*-------------------------Initialize ArrayList End-------------------------*/


/*-------------------------Initialize VarContainer-------------------------*/
typedef struct{
    ArrayList* key;
    ArrayList* value;
} VarContainer;

VarContainer* VC_init(void){
    VarContainer* container = malloc(sizeof(VarContainer));
    container->key = AL_init();
    container->value = AL_init();
    return container;
}

int putVar(char* key,char* value,VarContainer* container){
    if(assert(container->key,key) < 0){
        writeData(container->key,key);
        writeData(container->value,value);
    } else{
        perror("Variable already exists!");
        return -1;
    }
    return 0;
}

int takeVar(char* key,VarContainer* container){
    int index = assert(container->key,key);
    if(index >= 0){
        removeIndex(container->key,index);
        removeIndex(container->value,index);
    } else{
        perror("Variable doesn't exists!");
        return -1;
    }
    return 0;
}

char* getValue(char*key, VarContainer* container){
    int index = assert(container->key, key);
    if(index >= 0){
        return container->value->list[index];
    } else{
        return NULL;
    }
}

int updateValue(char* key, char* str, VarContainer* container){
    int index = assert(container->key,key);
    if(index >= 0){
        char* newValue=(char*)malloc(sizeof(char)*(strlen(str)+1));
        if(!newValue){
            perror("Out of memory!");
            exit(1);
        }
        strcpy(newValue,str);
        free(container->value->list[index]);
        container->value->list[index]=newValue;
    } else{
        perror("Variable doesn't exists!");
        return -1;
    }
    return 0;
}

void showVar(VarContainer* container){
    printf("Variable(s): ");
    for(int i=0;i < container->key->size;i++){
        if(!container->value->list[i]){
            continue;
        }
        printf("%s=%s  ",container->key->list[i],container->value->list[i]);
    }
    printf("\n");
}

int pour(VarContainer* container){
    for(int i=0;i < container->key->size;i++){
        free(container->key->list[i]);
        free(container->value->list[i]);
    }
    free(container->key);
    free(container->value);
    free(container);
    return 0;
}
/*-------------------------Initialize VarContainer End-------------------------*/

/*-------------------------Initialize Historical Stack-------------------------*/
int push(ArrayList* a, char* str){
    if(a->size >= 20){
        removeIndex(a,0);
    }
    *strchr(str,'\n') = 0;
    writeData(a,str);
    return 0;
}

char* pop(ArrayList* a){
    char* tmp = a->list[a->size-1];
    removeIndex(a,a->size-1);
    return tmp;
}

/*-------------------------Initialize Historical Stack End-------------------------*/

#define get_block(i) ((Block*)blockList->list[i]) //make sure the arraylist containing blocks is named exactly as "blockList"!
#define io_check        if (block->outFile != NULL) {\
                            freopen(block->outFile, "w", stdout);\
                            }\
                        if (block->inFile != NULL) {\
                            if(!freopen(block->inFile, "r", stdin)){\
                                perror("File doesn't exist!");\
                                exit(EXIT_FAILURE);\
                            }\
                        }\
                        if(block->pipeEnd!=NULL){\
                            if(block->pipeEnd[0]>=0)dup2(block->pipeEnd[0],fileno(stdin));\
                            if(block->pipeEnd[1]>=0)dup2(block->pipeEnd[1],fileno(stdout));\
                        }fflush(stdout);

char* ins[] = {"show","set","export","unexport","exit","wait","help","history","view"};
char* external_ins[]={"cat", "ls", "grep", "sleep", "cp","mv","vi", "rm", "head", "export", "mkdir", "chown", "chmod", "pwd"};
char* envv[] = {"/bin/", "/sbin/", "/usr/bin/", "/usr/sbin/", "./", "/usr/local/bin/", "/usr/local/sbin/"};
int envc = sizeof(envv)/ sizeof(envv[0]);
ArrayList* history; //historical commands
VarContainer* container; // contain key-value pairs
char buffer[BUFLEN];// buffer for user input
pid_t rootpid;
pid_t bgpid;
pid_t lastpid;

typedef struct{
    char* command;
    int* pipeEnd;
    char* inFile;
    char* outFile;
} Block;

void showBlock(Block* block){ //only for debugging purpose
    if(block->command)printf("Command: %s\n",block->command);

    if(block->pipeEnd){
        printf("Write end pipeFD: %d\n",block->pipeEnd[1]);
        printf("Read end pipeFD: %d\n",block->pipeEnd[0]);
    }
    if(block->inFile){
        printf("Input file name: %s\n",block->inFile);
    }
    if(block->outFile){
        printf("Output file name: %s\n",block->outFile);
    }

}
/*--------------------------INITIALIZE INTERNAL INSTRUCTION--------------------------*/
int export(VarContainer* container, char* str){
    return putVar(str,"",container);
}

int set(VarContainer* container, char* key, char* new_value){
    return updateValue(key,new_value,container);
}

int unexport(VarContainer* container, char* key){

    return takeVar(key,container);
};

void show(void){
    printf("%s\n",content);
    fflush(stdout);
    free(content);
}

void show_check(char* str){
    if(!strcmp(str,"show")){
        char* p;
        char* tmp;
        p = (str+5);
        p += strspn(p," ");
        tmp = strchr(p,'$');
        if(tmp && *(tmp+1)!=0 && *(tmp+1)!=' '){
            char* varBuffer[128];
            char* start = p;
            char* end = "";
            *tmp = 0;
            if(*(tmp+1) == '!'&&(*(tmp+2)==0||*(tmp+2)==' ')){
                end = tmp +2;
                sprintf(varBuffer,"%d",bgpid);
            } else if(*(tmp+1) == '$'&&(*(tmp+2)==0||*(tmp+2)==' ')){
                end = tmp +2;
                sprintf(varBuffer,"%d",getpid());
            } else{
                char* endStr = strchr(tmp+2,' ');
                if(endStr){
                    *endStr = 0 ;
                    end = endStr+1;
                }
                char* value = getValue(tmp+1,container);
                if(value){
                    strcpy(varBuffer,(value));
                    if(strcmp(end,"")){
                        strcat(varBuffer," ");
                    }
                } else{
                    perror("Variable doesn't exist!");
                    free(content);
                    return;
                }
            }
            content = malloc(sizeof(char)*(strlen(start)+strlen(end)+128+1));
            strcat(content,start);
            strcat(content,varBuffer);
            strcat(content,end);
        } else{
            content =  malloc(sizeof(char)*(strlen(p)+1));
            strcpy(content,p);
        }
    }

}

long* _isNumber(char* str, long* acc){
    char* pEnd;
    long int i = strtol(str,&pEnd,10);
    if(str+strlen(str) == pEnd) {
        *acc = i;
        return acc;
    }
    return NULL;
}

void xssh_exit(char** argv){
    long acc;
    if(argv[1] == 0) exit(0);
    long* p = _isNumber(argv[1],&acc);
    if(p && 0 <= acc&&acc < 256){
        exit((int)acc);
    } else exit(1);
}

void xssh_wait(char** argv){
    if(!strcmp(argv[1],"-1")){
        printf("Waiting for all child processes.\n");
        fflush(stdout);
        while(waitpid(-1,NULL,0) > 0);
        printf("All processes teminated\n");
        fflush(stdout);
    } else{
        long cpid;
        printf("Waiting for child process\n");
        if((!_isNumber(argv[1],&cpid)) || (waitpid((pid_t)cpid,NULL,0)) < 0){
            perror("Invalid child PID\n");
            return;
        }
        printf("Child process teminated\n");
        fflush(stdout);
    }


}

void xssh_help(void){
    printf("\
Team member: Ge Gao; Shang-Yung Hsu\n\n\
Supported instruction:(xssh instructions have higher priority than shell instructions.)\n\
                 xssh: \"show\",\"set\",\"export\",\"unexport\",\"exit\",\"wait\",\"help\",\"history\",\"view\" \n\
                shell: \"cat\", \"ls\", \"grep\", \"sleep\", \"cp\",\"mv\",\"vi\", \"rm\", \"head\", \"export\", \"mkdir\", \"chown\", \"chmod\", \"pwd\"\n\n\
Default path for shell instructions: \"/bin/\", \"/sbin/\", \"/usr/bin/\", \"/usr/sbin/\", \"./\", \"/usr/local/bin/\", \"/usr/local/sbin/\"\n\n\
Finished optional (a), (b) and (c). \n\n\
You can use pipe and file redirection(support both external instruction and internal instruction) at same time(e.g. show | cat > file) but \n\
our code only supports three formats of file redirection usages:  cat < file1 > file2, cat > file, and cat < file.(cat > file1 < file2 is invalid)\n\n\
Architecture:\n\
For optimization purpose, we implemented a data structure named ArrayList, and it functions as a resizable array. ArrayList is very \n\
common in our code. There’s another structure called VarContainer which is based on ArrayList, and it’s used exclusively for xssh export, xssh \n\
set and xssh unexport instructions.\n\
\"view\" and \"history\" instruction can be used for showing current variables and history commands respectively.\n\
"\
    );
    fflush(stdout);
}

int history_check(char* str){
    long commandNo;
    if(str[0] == '%') {
        if(_isNumber(&str[1],&commandNo) && commandNo < history->size) {
            strcpy(buffer, history->list[commandNo-1]);
            return -1;
        }
    }
    return 0;
}


int isInternal(char* argv[]){
    //check internal executable instruction
    int isInternal = -1;
    for(int i = 0; i < (sizeof(ins)/ sizeof(ins[0])); i++){
        if(!strcmp(argv[0],ins[i])){
            isInternal = i; //return the index of instruction
            break;
        }
    }
    return isInternal;

}

void substitute(char** argv, VarContainer* container){
    if(!strcmp(argv[0],"export") || !strcmp(argv[0],"set") || !strcmp(argv[0],"unexport")){
        return; //don't substitute if it's one of above instructions.
    }
    int index = 0;
    while(argv[index]) {
        if(!strcmp(argv[index],"$!")){
            char* str = realloc(argv[index], sizeof(char)*21);
            if(str){
                sprintf(str,"%d",getpid());
                argv[index] = str;}
            continue;
        }

        if(!strcmp(argv[index],"$$")){
            char* str = realloc(argv[index], sizeof(char)*21);
            if(str){
                sprintf(str,"%d",getpid());
                argv[index] = str;}
            continue;
        }


        char* tmp = getValue(argv[index],container);
        if(tmp){
            argv[index] = tmp;
        }

        index++;
    }
}

void showHistory(void){
    showList(history);
}

void view(void){
    showVar(container);
}


void ctrlsig(int sig){
    signal(SIGINT, ctrlsig);
    if(rootpid != lastpid){
        if(kill(lastpid,SIGKILL)>=0){
            lastpid = rootpid;
            printf("-xssh: Exit pid childpid");
        } else{
            perror("error:");
        }

    }

}

void catchctrlc(){
    signal(SIGINT,&ctrlsig);
}

/*--------------------------INITIALIZE INTERNAL INSTRUCTION END--------------------------*/

Block* Block_init(void){
    Block* block = calloc(1,sizeof(Block));
    if(!block){
        perror("error");
        return NULL;
    }
    return block;
}


int* pipe_init(void){
    int* p =malloc(sizeof(int)*2);
    if(pipe(p)){
        perror("Error:");
        return NULL;
    }
    p[0] = -1;
    p[1] = -1;
    return p;
}

int commentCheck(char* str){
    char* p = strchr(str,'#');
    if(p){
        *p = 0;
        return 0;
    }
    return -1;
}

int isExternal(char* argv[]){

    int isExternal = 0;
    for(int i = 0; i < (sizeof(external_ins)/ sizeof(external_ins[0])); i++){
        if(!strcmp(argv[0],external_ins[i])){
            isExternal = 1;
            break;
        }
    }
    return isExternal;
}

int getFileIO(ArrayList*blockList){

    for (int i = 0; i < blockList->size; i++) {
        Block *block = (Block *) (blockList->list[i]);
        char* p;
        char* IFname;
        char* OFname;
        char* tmp = malloc(sizeof(char)*(strlen(block->command)+1));
        strcpy(tmp,block->command);
        p = strchr(tmp,'>');
        if(p){
            IFname = p+1;
            strtok(IFname," ");
            block->outFile = IFname;
            *p=0;
        }
        p = strchr(tmp,'<');
        if(p){
            OFname = p+1;
            strtok(OFname," ");
            block->inFile = OFname;
            *p=0;
        }

        free(block->command);
        block->command = tmp;


    }
    return 0;
}


void freeBlock(ArrayList* blockList){
    int fd;
    for (int i = 0; i < blockList->size; i++) {
        free(((Block *) (blockList->list[i]))->command);//this will free inFile and outFile as well
        if((fd = get_block(i)->pipeEnd[0]) != -1)close(fd);
        if((fd = get_block(i)->pipeEnd[1]) != -1)close(fd);
        free(get_block(i)->pipeEnd);
    }
    dump(blockList);
    free(blockList);
}

ArrayList* pipeSplit(char* str){
    ArrayList* blockList = AL_init();
    char* p;
    Block* tmp;
    p = strtok(str,"|");
    while(p != NULL){
        tmp = Block_init();
        tmp->command = malloc(sizeof(char)*(strlen(p)+1));
        commentCheck(p);
        strcpy(tmp->command, p);
        store(blockList, tmp);
        p = strtok(NULL,"|");
    };
    for(int i=0; i < blockList->size; i++){
        int p[2];
        if(i == 0){
            ((Block*)blockList->list[i])->pipeEnd = pipe_init();
        }
        else{
            ((Block*)blockList->list[i])->pipeEnd = pipe_init();
            pipe(p);
            ((Block*)blockList->list[i-1])->pipeEnd[1] = p[1];
            ((Block*)blockList->list[i])->pipeEnd[0] = p[0];
        }
    }
    return blockList;
}

int execute(ArrayList* blockList, char* envv[], int envc, VarContainer* container) {
    for (int k = 0; k < blockList->size; k++) {  //execute every block
        int waitFlag = 1;
        int instr_index;
        pid_t cpid;
        char* bg;
        Block *block = (Block *) (blockList->list[k]);
        if((bg = strchr(block->command,'&'))){
            *bg = 0;
            waitFlag = 0;// put it in background
        }
/*--------------------------CONVERT COMMAND TO AN ARRAY--------------------------*/
        char *p = strtok(block->command, " ");
        show_check(p);
        if(history_check(p)){//check if it's historical instruction like %1
            pop(history);
            if(k != 0){
                perror("invalid instruction");
                return -1;
            }
            return 1;
        }
        ArrayList *tmp = AL_init();//save every token parsed into an arraylist
        while (p != NULL) {
            store(tmp, p);
            p = strtok(NULL, " ");
        }
        char *argv[tmp->size + 1];
        argv[tmp->size] = 0;
        for (int j = 0; j < tmp->size; j++) {
            argv[j] = tmp->list[j];
        }
/*--------------------------CONVERT COMMAND TO AN ARRAY END --------------------------*/
        int i;
        substitute(argv,container);
        if ((instr_index = isInternal(argv)) >= 0) {
            switch(instr_index){
                case 0:
                    if ((cpid = fork()) == 0) {
                        io_check
                        show();
                        exit(0);
                    } else {
                        if(waitFlag && cpid != rootpid){
                            lastpid = cpid;
                            waitpid(cpid,NULL,0);
                        } else{
                            bgpid = cpid;
                        }
                    }
                    break;
                case 1:
                    set(container,argv[1],argv[2]);
                    break;
                case 2:
                    export(container,argv[1]);
                    break;
                case 3:
                    unexport(container,argv[1]);
                    break;
                case 4:
                    xssh_exit(argv);
                    break;
                case 5:
                    xssh_wait(argv);
                    break;
                case 6:
                    if ((cpid = fork()) == 0) {
                        io_check
                        xssh_help();
                        exit(0);
                    } else {
                        if(waitFlag && cpid != rootpid){
                            lastpid = cpid;
                            waitpid(cpid,NULL,0);
                        } else{
                            bgpid = cpid;
                        }
                    }
                    break;
                case 7:
                    if ((cpid = fork()) == 0) {
                        io_check
                        showHistory();
                        exit(0);
                    } else {
                        if(waitFlag && cpid != rootpid){
                            lastpid = cpid;
                            waitpid(cpid,NULL,0);
                        }else{
                            bgpid = cpid;
                        }
                    }
                    break;
                case 8:
                    if ((cpid = fork()) == 0) {
                        io_check
                        view();
                        exit(0);
                    } else {
                        if(waitFlag && cpid != rootpid){
                            lastpid = cpid;
                            waitpid(cpid,NULL,0);
                        }else{
                            bgpid = cpid;
                        }
                    }
                    break;
                default:
                    perror("Invalid internal instruction");
                    return -1;
            }
        } else if(isExternal(argv)){
            for (i = 0; i < envc; i++) {
                char *filePath = calloc(strlen(envv[i]) + strlen(argv[0]) + 1, sizeof(char));
                if (!filePath) {
                    perror("error");
                    return -1;
                }
                strcpy(filePath, envv[i]);
                strcat(filePath, argv[0]);
                if (access(filePath, X_OK)) { // when the file exists and is executable
                    continue;
                }
                if ((cpid = fork()) == 0) {
                    io_check
                    execv(filePath, argv);
                } else {
                    if(waitFlag && cpid != rootpid){
                        lastpid = cpid;
                        waitpid(cpid,NULL,0);
                    }else{
                        bgpid = cpid;
                    }
                    free(filePath);
                    break;
                }
            }

        } else{
            perror("invalid instruction");
            pop(history);
            return -1;
        }
        destroy(tmp);
    }
    return 0;
}

/*our xssh will parse user variable if and only if the variable has a key-value pair!
 *
 * If type command lines as following order: 1. "export dir" 2."ls dir". The xssh will not consider dir as a user defined variable.
 * The xssh will replace the variables if it receives command lines like this: 1."export dir" 2."set dir Desktop/myfolder" 3."ls dir"
 * (same as "ls Desktop/myfolder")
 *
 * If you want to print a variable by using "show" command, you need to add a $ sign before your varibale name.(Executing 1."export x" 2."set
 * x variable" 3."show this is $x" will print out "this is variable". Only support one variable including $$ or $!. If you input more than one
 * variables in the show arguments, the other variable(s) will not be parsed!)
 * */
void run(char* buffer){
    printf(">>>");
    push(history,fgets(buffer,BUFLEN,stdin));
    label:;
    ArrayList* blockList = pipeSplit(buffer);
    getFileIO(blockList);
    if(execute(blockList,envv,envc, container) == 1) goto label;
    /* will go back to label if it's a historical instruction*/
    freeBlock(blockList);
}

int main(){
    rootpid = getpid();
    lastpid = rootpid;
    history = AL_init();
    container = VC_init();
    catchctrlc();
    while (1){
        run(buffer);
    }
}
