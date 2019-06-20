#include "mbed.h"
#include "rtos.h"
#include "stdlib.h"
#include "time.h"
Serial pc(USBTX,USBRX,9600);
#define THREAD_NUM 5
 
/* Mail */
typedef struct {
  int  entry1; 
} mail_t;
 
Mail<mail_t, THREAD_NUM-1> mail_box[THREAD_NUM];

void calculate_min_ping(int[],int);
void send(int[],int,int);
void receive(int,int[]);
int calculate_winner(int[]);
 

void activities(int *myid) {
    
    pc.printf("Sono il thread %d\n\r",*myid);
    
    int others[THREAD_NUM]; //vettore dove so l'id di tutti i thread compreso il mio! 
    int winner;
    
    int votes[THREAD_NUM]; // vettore dei voti
    for(int i = 0; i < THREAD_NUM; i++){
        votes[i] = 0;    
    }
    
    calculate_min_ping(votes,*myid);
    
    send(others,*myid,votes[*myid]); 
             
    receive(*myid,votes);
          
    winner = calculate_winner(votes); 
    
      
    pc.printf("Thread %d riconosce come vincente il thread %d\n\r",*myid,winner);
             
    
        
    /*for(i = 0; i < THREAD_NUM; i++){
        if(others[i] != -1)
            pc.printf("Thread %d conosce il thread %d\n\r",*myid,others[i]);            
    }*/
      
}

void calculate_min_ping(int votes[],int myid){
    
    srand(time(NULL));
    int my_ping_times[THREAD_NUM-1]; /* = {4,3,3,5}; */
    
    int i;
    //simula il ping!
    for(i = 0; i < THREAD_NUM-1; i++)
        my_ping_times[i] = 1+rand()%1000;
    
       
        
    //cerca il minimo fra i ping ed il nodo che l'ha prodotto
    int min = my_ping_times[0];
    int min_index = 0;
    for(i = 1; i < THREAD_NUM-1; i++){
        if(my_ping_times[i] < min){
            min = my_ping_times[i];
            min_index = i;
        }              
    }
        
    //inserisce il nodo con il ping minimo fra la lista dei votati nel suo indice.
    if(min_index >= myid){
        //votes[min_index+1] = min_index;
        votes[myid] = min_index+1;
        pc.printf("Thread %d dice che il ping minimo e' del thread %d\n\r",myid,min_index+1);
    }
    else{
        //votes[min_index] = min_index;
        votes[myid] = min_index;
        pc.printf("Thread %d dice che il ping minimo e' del thread %d\n\r",myid,min_index);
    }
        
    pc.printf("Il vettore dei voti del thread %d e':\n\r",myid);
    for(i = 0; i < THREAD_NUM; i++)
        pc.printf("Votes[%d] = %d\n\r",i,votes[i]);    
    
    
}

void send(int others[], int myid, int mylowest){
    
    int i;    
        
    for(i = 0; i < THREAD_NUM; i++){
        others[i] = i;    //0-1-2-3-4
    }
    
    for(i = 0; i < THREAD_NUM; i++){
        if(others[i] == myid)
            others[i] = -1;    //al mio posto metto -1 così so che gli altri sono caratterizzati da id >= 0
    }
    
    for(i = 0; i < THREAD_NUM; i++){
        if(others[i] != -1){
            mail_t *mail = mail_box[others[i]].alloc();
            mail->entry1 = mylowest; 
            mail_box[others[i]].put(mail);
            //pc.printf("Il thread %d ha appena inviato una mail al thread %d\n\r",*myid,others[i]);            
        }    
    }  
    
}

void receive(int myid,int votes[]){
    
    int counter = 0;
    
    while(true){
        osEvent evt = mail_box[myid].get();
        if(evt.status == osEventMail) {
            mail_t *mail = (mail_t*)evt.value.p;
            //pc.printf("\nIl thread %d ha appena letto %d \n\r",myid,mail->entry1);
            for(int i = 0; i < THREAD_NUM; i++){
                if(i == myid || votes[i] != 0 )
                    continue;
                else{
                    votes[i] = mail->entry1;
                    break;
                }     
            }
            mail_box[myid].free(mail);
            counter++; 
        }
        if(counter == THREAD_NUM-1)
            break;    
    }   
    int i;
    for(i = 0; i < THREAD_NUM; i++){
        pc.printf("Thread %d : VotesAfterElection[%d] = %d\n\r",myid,i,votes[i]);
        Thread::wait(50*(myid+1));   
    }
}

int calculate_winner(int votes[]){
    
    int max_count = 0;
    int winner;
    int i;
    
    for (i = 0; i < THREAD_NUM; i++){
        int count=1;
        for (int j = i + 1; j < THREAD_NUM; j++){
            if (votes[i] == votes[j])
                count++;
        }
        if (count>max_count)
        max_count = count;
    }
    
    for (i = 0; i < THREAD_NUM; i++){
        int count = 1;
        for (int j = i + 1; j < THREAD_NUM; j++){
            if (votes[i]==votes[j])
                count++;
        }
        if (count==max_count){
            winner = votes[i];
            break; //questo dovrebbe stoppare il ciclo al primo vincitore   
        } 
     }   
    return winner;
    
}
 
int main (void) {
    
    Thread thread[THREAD_NUM];
    int i; 
    
    pc.printf("Creazione pool di thread\n\r"); 
            
    for(i = 0; i < THREAD_NUM; i++){
       thread[i].start(callback(activities,&i));
       Thread::wait(500);
     } 
     
     //wait(1); 
         
    for(i = 0; i < THREAD_NUM; i++)
       thread[i].join();         
        
    pc.printf("Bye bye\n\r");
    
    return 0;
    
}