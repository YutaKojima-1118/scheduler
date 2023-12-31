//作成者  : 小島佑太
//作成年月: 2023/7/30
#include "scheduler.h"

void init(process* pro, int N)
{
    for(int i=0;i<N;i++){
        pro[i].label=0;
        pro[i].arrive=0;
        pro[i].start=-1;
        pro[i].finish=0;
        pro[i].response=0;
        pro[i].length=0;
        pro[i].remain=0;
        pro[i].status=NOT_ARRIVED;
    }
}

int checking_finish(process* pro, int N) //すべてNOT_EXISTの場合1, それ以外の場合0を返す
{
    if(N==0){
        return 0;
    }
    for(int i=0;i<N;i++){
        if(pro[i].status != NOT_EXIST) return 0;
    }
    return 1;
}


queue* gen_queue(process key)
{
    queue *new=(queue *)malloc(sizeof(queue));
    new->pro=key;
    new->next=0;
    return new;
}

process dequeue(queue **head)
{
    if(*head == NULL){
        process pro;
        pro.status = NOT_EXIST;
        return pro;
    }

    if((*head)->next == NULL){
        process tmp=(*head)->pro;
        free(*head);
        *head=NULL;
        return tmp;
    }

    process tmp_pro=(*head)->pro;
    queue *tmp_que=*head;
    *head=(*head)->next;
    free(tmp_que);
    tmp_que=NULL;
    return tmp_pro;
}

void enqueue(int Q, int j, process *pro, queue **head, int N)
{
    if(pro == NULL){
        return;
    }
    switch(Q){
        case FCFS: fcfs_enqueue(j,pro,head,N); return;
        case SPT:  spt_enqueue(j,pro,head,N);  return;
        case R_R:  R_R_enqueue(j,pro,head,N);  return;
        default: printf("error: in arrive(), Q!=0,1,2\n"); return;
    }
    return;
}

void arrive(process *pro, int N, int i, int Q, queue **head, int *label)
{
    if(N!=0){
        int j;
        for(j=0;j<N;j++){
            if(pro[j].arrive == i){
                printf("---arrive new process%d, length=%d\n",pro[j].label,pro[j].length);
                enqueue(Q,j,pro,head,N);
                break;
            }
        }
        return;
    }
    else{
        int p=rand()%6;
        if(*head==NULL || p==0){
            process new;
            init(&new,1);
            new.arrive = i;
            new.length = rand()%10+1;
            new.label = *label;
            *label+=1;
            new.remain = new.length;
            enqueue(Q,0,&new,head,N);
            printf("---arrive new process%d, length=%d\n",new.label,new.length);
            return;
        }
    }
}

void run(process *pro, int i)
{
    printf("---running process%d, remain=%d -> %d\n",pro->label,pro->remain,pro->remain-1);
    if(pro->status == RUNNING){
        pro->remain -= 1;
        if(pro->remain == 0){
            pro->status = NOT_EXIST;
            printf("---finish process%d,",pro->label);
            printf(" arrive=%d, start=%d, finish=%d, response=%d\n",pro->arrive,pro->start,i,i-pro->arrive+1);
        }
    }
}

process start_run(queue** head, int i)
{

    process tmp = dequeue(head); //queueに存在しなかったらNOT_EXISTなprocessを返す
    if(tmp.status == NOT_EXIST){
        init(&tmp,1);
        tmp.status = NOT_EXIST;
        return tmp;
    }
    if(tmp.start==-1) tmp.start = i;
    tmp.status = RUNNING;
    printf("---start run process%d\n",tmp.label);

    return tmp;
}

int terminate(process *pro, process terminated, int i, int N, int* sum, int *cnt) //terminatedを，proにある元データにコピー
{
    if(terminated.start == -1){
        return 1;
    }
    terminated.finish = i;
    terminated.response = terminated.finish - terminated.arrive;
    *sum += terminated.response;
    if(terminated.finish == 0){
        return 1;
    }

    //printf("---finish process%d,",terminated.label);
    //printf(" arrive=%d, start=%d, finish=%d, response=%d\n",terminated.arrive,terminated.start,terminated.finish,terminated.response);
    if(N==0){
        int f=0;
        *cnt += 1;
        printf("------input '1' to run next process or a number except 1 to finish: ");
        scanf("%d",&f);
        if(f==1){
            return 1;
        }
        else{
            printf("\n\nsum=%d, cnt=%d",*sum,*cnt);
            double ave = (double)(*sum) / (double)(*cnt);
            printf("\nresponse average = %.4f\n",ave);
            return 0;
        }
    }
    for(int j=0;j<N;j++){
        if(terminated.label == j){
            pro[j] = terminated;
            return 1;
        }
    }
}

void print_pro(process *pro, int N, int sum)
{
    printf("\n");
    for(int i=0;i<N;i++){
        printf("process%d: length=%d\n",i,pro[i].length);
        printf("  arrive=%d, start=%d, finish=%d, response=%d\n",pro[i].arrive,pro[i].start,pro[i].finish,pro[i].response);
    }
    printf("\nresponse time average = %.4f\n",((double)sum/(double)N));
}

void print_queue(queue* head)
{
    printf("print_queue\n");
    printf("******");
    for(queue *p=head; p!=NULL; p=p->next){
        printf("%d ",p->pro.label);
    }
    printf("\n");
}

void write_log(process *pro, int N, int Q)
{
    FILE *fp=NULL;
    switch(Q){
        case FCFS: fp = fopen("FCFS_log.txt","r"); break;
        case SPT:  fp = fopen("SPT_log.txt","r");  break;
        case R_R:  fp = fopen("R_R_log.txt","r");  break;
        default: printf("error Q=0,1,2\n"); return;
    }
    if(fp == NULL){
        printf("オープン失敗:1\n");
        return;
    }
    int cnt[11] = {0}; //cnt[0]は全体，それ以降のcnt[k]は，length=kだったプロセスの合計処理数
    double ave[11] = {0};  //cnt[0]は全体，それ以降のcnt[k]は，length=kだったプロセスの平均応答時間
    for(int i=0;i<11;i++){
        fscanf(fp,"%d %lf",&cnt[i],&ave[i]);
        ave[i] *= cnt[i];
    }
    fclose(fp);

    switch(Q){
        case FCFS: fp = fopen("FCFS_log.txt","w"); break;
        case SPT:  fp = fopen("SPT_log.txt","w");  break;
        case R_R:  fp = fopen("R_R_log.txt","w");  break;
        default: printf("error Q=0,1,2\n"); return;
    }
    if(fp == NULL){
        printf("オープン失敗:2\n");
        return;
    }

    for(int i=0;i<N;i++){
        cnt[0] += 1;
        ave[0] += pro[i].response;
        if(1 <= pro[i].length && pro[i].length <= 10){
            cnt[pro[i].length] += 1;
            ave[pro[i].length] += pro[i].response;
        }
    }

    for(int i=0;i<11;i++){
        if(cnt[i] != 0) ave[i] /= cnt[i];
        fprintf(fp,"%d %f \n",cnt[i],ave[i]);
    }
    fclose(fp);
}
