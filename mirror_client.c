#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>

#include "List.h"

#define MAX_SIZE 50 //Lenght of the IDs has to be at most 50 chars


//Declaring some global variables, mainly so the signal handlers can use them

struct id_list* iroot=NULL; //Each client has a list of the IDs from the other clients
char path_to_id[PATH_MAX]; //Paths used in signal handlers
char mirror_path[PATH_MAX];




int fd; //File Descriptor used in listdir function

char* logfile; //Logfile path used in final_handler (see below)

int sendpath=1; //Sendpath used in listdir function


char *strremove(char *str, const char *sub){ //This function is used to remove some substrings. It is needed in the reader process.
    size_t len = strlen(sub);
    
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }

    return str;
}

void listdirs(char* fpath, int MSGSIZE, int gpid){ //Recursive function used by writer process in order to send files from every folder

			
			struct dirent* newent;
			DIR* new = opendir(fpath);  //Opening the directory from the current path we're on (fpath)
			
        	if(sendpath==1){ //Sendpath variable is needed so we can know when to send the path to the reader process
				sendpath=0;  //Reset its value for later use
						
				int three=3;  //Byte no. 3 is sent so the reader process knows it has to read a path
       			int dnum = write(fd, &three , sizeof(three)); 
       			int path_size=strlen(fpath);  
       			int pwrite=write(fd, &path_size , sizeof(path_size)); //First we send the path size, because reader process needs it
       			int psend= write(fd, fpath , strlen(fpath)+1); //Then, the actual path
       			if(dnum == -1 || pwrite == -1 || psend == -1){ //Error checking
       				printf("Error here\n");
       				kill(gpid, SIGUSR1); //Signal use will be explained in the writer process
                    exit(1);
       			}
       					
       					
			}
       					    	while((newent = readdir(new)) != NULL){ //Opening the directory
							        struct stat st;
							        
							        if(strcmp(newent->d_name, ".") == 0 || strcmp(newent->d_name, "..") == 0){ //Skipping ".." and "." strings
							            continue;
							        }
							        
							        char checkpath[PATH_MAX]; //Checkpath string gets the path of the item we're on in order to check if it's dir or a regular file
							        strcpy(checkpath,fpath); 
							        strcat(checkpath,"/");
							       	strcat(checkpath,newent->d_name);
							        struct stat path_stat;
							    	stat(checkpath, &path_stat);

							    	if( S_ISREG(path_stat.st_mode)){ //Sending a regular file here
							    	
							    	int one=1; //Byte no.1 is used so the reader knows it has to receive a file
							        int fnum = write(fd,&one, sizeof(one));
							        if(fnum==-1){
							        	printf("Error\n");
							        	kill(gpid, SIGUSR1);
                        				exit(1);
							        }
							       

							        short int b2; //Sending bytes and name of the file
							        b2=strlen(newent->d_name);
							        int b2send = write(fd,&b2,sizeof(b2));
							        int fsend = write(fd,newent->d_name,strlen(newent->d_name)+1);
							        if(b2send==-1 || fsend==-1){
							        	printf("Error\n");
							        	kill(gpid, SIGUSR1);
                        				exit(1);
							        }
							       
									char pathh[PATH_MAX];
									strcpy(pathh,fpath);
									strcat(pathh,"/");
									strcat(pathh,newent->d_name); //Getting path of the file in order to get its size
									
									FILE* fp=fopen(pathh,"r"); 
									fseek(fp, 0, SEEK_END); //Seek to end of file
									int size = ftell(fp); //And get current file pointer, that's the size of the file
									fseek(fp, 0, SEEK_SET);
									int b4=size;
									
									
									int b4send = write(fd,&b4,sizeof(b4)); //Sending number of bytes of the file
									if(b4send==-1){
										printf("Error\n");
										kill(gpid, SIGUSR1);
                        				exit(1);
									}
									
									char line[128000]; //Now we start sending the actual string, 128000 can be its maximum length
									while (fgets(line, sizeof(line), fp)){ //Getting the string
									   		
									    int len = strlen(line); //Lenght of the string
									           
									    MSGSIZE=len;
									            
									    int num = write(fd, line , MSGSIZE); //Writing it to the pipe
									    if(num==-1){
									        printf("Error\n");
									        kill(gpid, SIGUSR1);
                        					exit(1);
									    }
									          
									}
									fclose(fp);
									int zeros=write(fd,"00",strlen("00")+1); //Sending the two zeros
									if(zeros==-1){
										printf("Error\n");
										kill(gpid, SIGUSR1);
                        				exit(1);
									}

									FILE* flog=fopen(logfile,"a"); //Logfile work. Writing the number of bytes sent, as well as the name of the file
									fprintf(flog, "%s %d\n", "Bytes Sent:", b4 );
									fclose(flog);

									FILE* flog1=fopen(logfile,"a");
									fprintf(flog1, "%s %s\n", "File Sent:", newent->d_name );
									fclose(flog1);


									}
							        if (fstatat(dirfd(new), newent->d_name, &st, 0) < 0){ //If it's not a directory, continue
							            perror(newent->d_name);
							            continue;
							        }

							        if (S_ISDIR(st.st_mode)){ //If the item is a directory
							        	
							        	char oldpath[PATH_MAX]; 
							        	strcpy(oldpath,fpath);
							        	strcat(oldpath,"/");
							        	strcat(oldpath,newent->d_name); //Getting the path of the directory we found
							        
       									sendpath=1; //And sending it to the reader process, same as before
							        	if(sendpath==1){
										sendpath=0;
										
										int three=3;
       									int dnum = write(fd, &three , sizeof(three));
       									int path_size=strlen(oldpath);
       									int pwrite=write(fd, &path_size , sizeof(path_size));
       									int psend= write(fd, oldpath , strlen(oldpath)+1);
       									if(dnum == -1 || pwrite == -1 || psend == -1){
       										printf("Error here\n");
       										kill(gpid, SIGUSR1);
                        					exit(1);
       									}

       									
										}
							        	
							        	listdirs(oldpath,MSGSIZE,gpid); //Recursive call for the new dir we found
							        	
							        	sendpath=1; //After finishing, send the path of the directory we were previously on
							        	if(sendpath==1){
										sendpath=0;
										
										int three=3;
       									int dnum = write(fd, &three , sizeof(three));
       									int path_size=strlen(fpath);
       									int pwrite=write(fd, &path_size , sizeof(path_size));
       									int psend= write(fd, fpath , strlen(fpath)+1);
       									if(dnum == -1 || pwrite == -1 || psend == -1){
       										printf("Error here\n");
       										kill(gpid, SIGUSR1);
                        					exit(1);
       									}
       									
										}


							        }


							    }
        	
        	sendpath=1;
        	closedir(new); //Closing the directory

}


void final_handler(int signum){ //Final Handler is used when a client receives the SIGINT and SIGQUIT signals
	
	printf("\nExiting the system.\n");

	FILE* flog=fopen(logfile,"a");  //Writes the string "I am gone!" to the logfile so it will be used by the get_stats.sh script
	fprintf(flog, "%s\n", "I am gone!");
	fclose(flog);

	DestroyIDList(iroot); //Destroying the list of the IDs he spotted
	
	char filetodelete[PATH_MAX];
	strcat(filetodelete,path_to_id); 
	
	int statrem = remove(filetodelete); //Removing his .id file from the common folder
	int retval=execl("/bin/rm", "rm", "-rf", mirror_path, NULL); //Removing his local mirror folder

	exit(0);

}

int trycount=0;
pid_t all_child[2];

void sigusr_handler(int signo){ //Sig_usr handler used when the father receives SIGUSR1 from his children
    if(signo == SIGUSR1){
    printf("SigUsr caught!\n");
    printf("Something went wrong during transfer. Let's try again. \n");
    trycount++; //Trycount variable counts how many times the father tried to fork the processes. If this reaches 4, it has to give up.

        int i=0;
        for(i ; i < 2; i++){ //The father also kills his two children to start anew. Their IDs are stored in the all_child array.
        	kill(all_child[i], SIGTERM);
        }
	}
    
}



int main(int argc,char* argv[]){

	int thisid;
	int buffer;
	char* common;
	char* input_dir;
	char* mirror_dir;
	
	//Getting the parameters
	char* argm1=argv[1];
  	if(strcmp(argm1,"-n")==0){

    	 thisid=atoi(argv[2]);
  	}
	char* argm3=argv[3];
  	if(strcmp(argm3,"-c")==0){

    	 common=argv[4];
  	}
  	char* argm5=argv[5];
  	if(strcmp(argm5,"-i")==0){

    	 input_dir=argv[6];
  	}
  	char* argm7=argv[7];
  	if(strcmp(argm7,"-m")==0){

    	 mirror_dir=argv[8];
  	}
  	char* argm9=argv[9];
  	if(strcmp(argm9,"-b")==0){

    	 buffer=atoi(argv[10]);
  	}
  	char* argm11=argv[11];
  	if(strcmp(argm11,"-l")==0){

    	 logfile=argv[12];
  	}


  	char cwd[PATH_MAX];//Getting the current directory here, which will be needed throughout the program
  	if (getcwd(cwd, sizeof(cwd)) != NULL){
       printf("Current working directory is: %s\n", cwd);
   	}else{
       perror("getcwd() error");
       return 1;
   	}

   	char input_path[PATH_MAX];
   	strcat(cwd,"/");
   	strcpy(input_path,cwd);
   	strcat(input_path,input_dir); //Getting path to input folder
   	
   	struct stat st = {0};

   	int failure=0;
	if (stat(input_path, &st) == -1) {
		printf("Input dir doesn't exist. \n");
    	failure=1;
	}
	assert(failure!=1); //Assert macro ends the program if the input folder doesn't exist

	char common_path[PATH_MAX];
	strcpy(common_path,cwd);
	strcat(common_path,common); //Getting path to the common folder
	
	if (stat(common_path, &st) == -1) {
    	mkdir(common_path, 0777); //And creating it, if it doesn't exist
	}

	
	strcpy(mirror_path,cwd);
	strcat(mirror_path,mirror_dir); //Getting path to mirror dir 

	int mirfailure=0;
	DIR* mircheck = opendir(mirror_path);
	if (mircheck){
		printf("Mirror Folder already exists.\n");
		mirfailure=1;
    	
	}
	closedir(mircheck);
	assert(mirfailure!=1); //Assert macro ends the program if mirror dir already exists

	
	if (stat(mirror_path, &st) == -1) {
    	mkdir(mirror_path, 0777); //Creating mirror folder
	}


	FILE *flog;
	flog=fopen(logfile,"w+"); //Creating the logfile

	flog=fopen(logfile,"a");
	fprintf(flog, "%s %s\n", "ID:", argv[2] ); //And writing its ID which will be used by get_stats.sh 
	

	fclose(flog);

	char sthisid[MAX_SIZE];
	char ornumber[MAX_SIZE];
	strcpy(sthisid,argv[2]);
	strcpy(ornumber,sthisid);
	strcat(sthisid,".id"); //Creating the name of the .id file which will be placed in the common dir
	

	strcat(path_to_id,common_path);
	strcat(path_to_id,"/");
	strcat(path_to_id,sthisid); //Getting its path
	

	int filefailure=0;
	if( access( path_to_id, F_OK ) != -1 ) {
    
		filefailure=1;
		printf(".id file exists. \n");
	}
	assert(filefailure!=1); //Assert macro ends the program if the .id file already exists

	int fdid=open(path_to_id, O_WRONLY|O_CREAT|O_TRUNC , 0777); //Creating the desired file
	int gpid=getpid(); //Getting process id
	char sgpid[MAX_SIZE]; 
	sprintf(sgpid,"%d",gpid);
	write(fdid,sgpid,strlen(sgpid)); //And writing it to the .id file
	close(fdid);

	
	struct id_list* itemp=iroot; //Creating the id list
	iroot=new_list(sthisid);


	char cwd_n_input[PATH_MAX]; //Path to input dir
	strcpy(cwd_n_input,cwd);
	strcat(cwd_n_input,input_dir);

	//Final handler is called here
	signal(SIGINT,final_handler);
	signal(SIGQUIT,final_handler);


	 DIR *dp;
  struct dirent *ep;     
  int foundid=0;
  int foundold=0;
  int foundidg=0;
  int allgood=0;
  int exists=0;
	pid_t pid;
  
	printf("Time to start checking the common folder.\n"); //Time to enter the checking common folder mode

	while(1){ //General idea: Enter a While 1 loop, sleep for some time (7 sec), and after waking up, first check if new IDs arrived, then if any of the existing ones left the system

		sleep(7);


		//Code for new ids
		printf("Checking for new IDs.\n");

		dp = opendir(common_path);
  		if (dp != NULL){
    	while (ep = readdir (dp)){ //Checking every item from the common folder
      		char temp[MAX_SIZE];  //Temp string gets item name
      		strcpy(temp,ep->d_name);

      		int t=0;
      		if(strstr(temp,"fifo")){ //If it is a .fifo file ignore it 
      			continue;
      		}

      		while(temp[t]!='\0'){ //If it has the string .id it means we found a .id file
				if(temp[t]=='.' && temp[t+1]=='i' && temp[t+2]=='d'){
					foundid=1;
				}
				t++;
      		}
      		

      		if(foundid==1 && strcmp(temp,"..")!=0 && strcmp(temp,".")!=0){ //If we found a new one make sure it is not in the list

      			itemp=iroot;
      			while(itemp!=NULL){
      				if(strcmp(temp,itemp->ID)==0 || strcmp(temp,sthisid)==0){
      					foundold=1;
      					
      				}

      				itemp=itemp->next;
      			}
      		}
      		
      		if(foundold==0 && strcmp(temp,sthisid)!=0 && strcmp(temp,"..")!=0 && strcmp(temp,".")!=0){ //Foundold is 0. This means we have found an ID which we haven't done work for
      			
      			itemp=iroot;
      			itemp=get_last(itemp); //Adding it to the end of the list

      			struct id_list* new_node=Add_Node(temp);
      			itemp->next=new_node;

      			char number[MAX_SIZE]; //Number string gets the ID without the .id extension
      			int r=0;
      			while(temp[r]!='\0'){
      				
      				if(temp[r]=='.'){
      					break;
      				}
      				number[r]=temp[r];
      				r++;
      			}

      			printf("New ID spotted: %s\n",number ); 
      			printf("Time to begin the transfer.\n");

      			

      		
						int  MSGSIZE = buffer;	//MSGSIZE will be used when we"ll be getting the file contents. Same value as buffer size parameter
      		
						char fifoname[MAX_SIZE];  //Creating the fifo names and their paths
						char fifoname2[MAX_SIZE];

      					strcpy(fifoname,"id");
       					strcat(fifoname,ornumber);
       					strcat(fifoname,"_to_id");
       					strcat(fifoname,number);
       					strcat(fifoname,".fifo");
       					
       					char common_slash[PATH_MAX]; 
       					strcpy(common_slash,common_path);
       					strcat(common_slash,"/");
       					strcat(common_slash,fifoname);

       					
       					strcpy(fifoname2,"id");
       					strcat(fifoname2,number);
       					strcat(fifoname2,"_to_id");
       					strcat(fifoname2,ornumber);
       					strcat(fifoname2,".fifo");
       					
       					char common_slash2[PATH_MAX];
       					strcpy(common_slash2,common_path);
       					strcat(common_slash2,"/");
       					strcat(common_slash2,fifoname2);



      		int i=0;

      		 while(1){ //While 1's use will be explained later
        		i=0;
        		

      		for(i;i<2;i++){ //Creating the two child processes with fork
      			pid=fork();
      			all_child[i] = pid;         						
       			if(pid == -1){
           			printf("Error in fork() \n");
           			return 1;
       			}
       			if(pid==0){ //Pid is 0, this means we"ll do work for a child process. If counter is 0 then it's the writer, if it's 1 then we're on the reader
       				if(i==0){ //Writer Process 

       					
       					if( mkfifo( common_slash , 0666) ==  -1 ){ //Creating the fifo file
       						
       						if( errno != EEXIST ) {
     						
    						kill(gpid, SIGUSR1);
                        	exit(1);

  						}
  						
       					}
       					
    					int  nwrite;
						char  msgbuf[MSGSIZE +1];	
						 if( (fd=open(common_slash , O_WRONLY)) < 0){ //Opening the fifo file
  							perror("fifo  open  problem");
  							kill(gpid, SIGUSR1);
                        	exit(1);
						}

       					char inwork[PATH_MAX]; //Inwork is input folder's path along with "/". It will be used later on.
       					struct dirent* dent;
       					DIR* srcdir = opendir(input_path);
       					strcpy(inwork,input_path);
       					strcat(inwork,"/");
       					
       					
       					int zero=0; //Sending different bytes for extra information necessary for the reader. Bytes 0,1,2,3 may be sent. The use of each one will be explained below.
       					int dnum = write(fd, &zero , sizeof(zero)); //Byte 0 is used to send name of the input folder. If the reader gets this, he knows he will have to store files in the mirror/id/ folder
       					int ilen = strlen(input_dir);
       					int iwrite = write(fd, &ilen , sizeof(ilen));
       					int cnum = write(fd, input_dir, strlen(input_dir)+1);
       					if(dnum == -1 || iwrite == -1 || cnum == -1){
       						printf("Error here\n");
       						kill(gpid, SIGUSR1);
                        	exit(1);
       					}

       					
       					    	while((dent = readdir(srcdir)) != NULL){ //Opening the input directory
							        struct stat st;
							        
							        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){ //Skipping ".." and "." strings
							            continue;
							        }
							        

							        char checkpath[PATH_MAX]; //Checkpath string gets the path of the item we're on in order to check if it's dir or a regular file
							        strcpy(checkpath,input_path);
							        strcat(checkpath,"/");
							       	strcat(checkpath,dent->d_name);
							        struct stat path_stat;
							    	stat(checkpath, &path_stat);
							    	
							        

							    	if( S_ISREG(path_stat.st_mode)){ //Sending a regular file here

							    	int one=1; //Byte no.1 is used so the reader knows it has to receive a file
							        int fnum = write(fd,&one, sizeof(one));
							        if (fnum==-1){
							        	printf("Error\n");
							        	kill(gpid, SIGUSR1);
                        				exit(1);
							        }
									
						   		
							        short int b2; //Sending bytes and name of the file
							        b2=strlen(dent->d_name);
							        int b2send = write(fd,&b2,sizeof(b2));
							        int fsend = write(fd,dent->d_name,strlen(dent->d_name)+1);
							        if(b2send==-1 || fsend==-1){
							        	printf("Error\n");
							        	kill(gpid, SIGUSR1);
                        				exit(1);
							        }
							        

									char pathh[PATH_MAX];
									strcpy(pathh,inwork);
									strcat(pathh,dent->d_name);//Getting path of the file in order to get its size
									
									FILE* fp=fopen(pathh,"r");
									fseek(fp, 0, SEEK_END); //Seek to end of file
									int size = ftell(fp); //And get current file pointer, that's the size of the file
									fseek(fp, 0, SEEK_SET);
									int b4=size;
									
									int b4send = write(fd,&b4,sizeof(b4));//Sending number of bytes of the file
									if(b4send==-1){
										printf("Error\n");
										kill(gpid, SIGUSR1);
                        				exit(1);
									}

									
									
									char line[128000];//Now we start sending the actual string, 128000 can be its maximum length
									while (fgets(line, sizeof(line), fp)){//Getting the string
									   		
									            int len = strlen(line);//Lenght of the string
									           
									            MSGSIZE=len;
									           
									            int num = write(fd, line , MSGSIZE);//Writing it to the pipe
									            if(num==-1){
									            	printf("Error\n");
									            	kill(gpid, SIGUSR1);
                        							exit(1);
									            }
									          
									}
									
									fclose(fp);
									int zeros=write(fd,"00",strlen("00")+1); //Sending the two zeros
									if(zeros==-1){
										printf("Error\n");
										kill(gpid, SIGUSR1);
                        				exit(1);
									}

									FILE* flog=fopen(logfile,"a"); //Logfile work. Writing the number of bytes sent, as well as the name of the file
									fprintf(flog, "%s %d\n", "Bytes Sent:", b4 );
									fclose(flog);

									FILE* flog1=fopen(logfile,"a");
									fprintf(flog1, "%s %s\n", "File Sent:", dent->d_name );
									fclose(flog1);

									}

									if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0){ //If it's not a directory, continue
							            perror(dent->d_name);
							            continue;
							        }

									if (S_ISDIR(st.st_mode)){  //If the item is a directory
							        
							        	char oldpath[PATH_MAX];
							        	strcpy(oldpath,input_path);
							        	strcat(oldpath,"/");
							        	strcat(oldpath,dent->d_name);//Getting the path of the directory we found
							        	

							        	int zero=0; //Sending the name of the input folder, same as the first time
       									int dnum = write(fd, &zero , sizeof(zero));
       									int ilen = strlen(input_dir);
       									int iwrite = write(fd, &ilen , sizeof(ilen));
       									int cnum = write(fd, input_dir, strlen(input_dir)+1);
       									if(dnum == -1 || iwrite == -1 || cnum == -1){
       										printf("Error here\n");
       										kill(gpid, SIGUSR1);
                        					exit(1);
       									}
       									

							        	listdirs(oldpath,MSGSIZE,gpid); //Calling list dirs function for the directory we encountered

							        	int zero1=0;//Sending the name of the input folder, same as the first time
       									int dnum1 = write(fd, &zero , sizeof(zero));
       									int ilen1 = strlen(input_dir);
       									int iwrite1 = write(fd, &ilen , sizeof(ilen));
       									int cnum1 = write(fd, input_dir, strlen(input_dir)+1);
       									if(dnum1 == -1 || iwrite1 == -1 || cnum1 == -1){
       										printf("Error here\n");
       										kill(gpid, SIGUSR1);
                        					exit(1);
       									}


							        }
							        

							        

							    }
							    int two=2; //Byte no.2 is to tell the reader that we did all the work for the items in the input folder, so it may exit
							    int over=write(fd, &two , sizeof(two));
							    
							    if(over==-1){
							    	printf("Error\n");
							    	kill(gpid, SIGUSR1);
                        			exit(1);
							    }
							    close(fd);
							    unlink(common_slash); 
							    closedir(srcdir);

       				}
       				if(i==1){ //Reader Process
       					

       					if( mkfifo( common_slash2 , 0666) ==  -1 ){ //Creating the fifo file
       						
       						if( errno != EEXIST ) {
    							kill(gpid, SIGUSR1);
                        		exit(1);
							}

       					}

       					if( (fd=open(common_slash2 , O_RDONLY)) < 0){ //Opening the fifo file
  							
  							kill(gpid, SIGUSR1);
                        	exit(1);
						}
						

						  fd_set fds; //Declaring variables for the select function
						  int maxfd;
						  int res;
						  char buf[256];

						  FD_ZERO(&fds); 
						  FD_SET(fd, &fds);

						  maxfd = fd;
						  struct timeval timeout;
						  timeout.tv_sec = 0;
						  timeout.tv_usec = 0;

						  

						struct timespec c1start, c1end;

						clock_gettime(CLOCK_MONOTONIC, &c1start); //Starting a clock before the first call of select

						
						int retval=0;
						while (retval==0){
						  
						  FD_ZERO(&fds); 
						  FD_SET(fd, &fds);
						  retval =  select(maxfd+1, &fds, NULL, NULL, &timeout);
						  clock_gettime(CLOCK_MONOTONIC, &c1end);  //Getting time after calling select
						  long double create_start=c1start.tv_sec; 
						  long double create_end=c1end.tv_sec; 
						  long double temp1=create_end - create_start; //Getting the difference between start and end times
						  
						  if(temp1>=30){ //If it's more than 30 sec, we send the SIGUSR1 signal

						    printf("Too much time passed! \n");
						    kill(gpid, SIGUSR1);
                        	exit(1);
						    
						  }
						}



       					char currdir[PATH_MAX];
       					char cmsgbuf[PATH_MAX+1];
       					char hold[PATH_MAX+1];
       					
       					while(1){ //Reader enters a While 1 loop where it waits to read one of the 4 available bytes to proceed 
       						char  msgbuf[MSGSIZE +1];
       						int byte;
       						

       						if(read(fd, &byte , sizeof(byte)) < 0) {
  		
  								perror("problem  in  reading");
  								kill(gpid, SIGUSR1);
                        		exit(1);
							}
							
							
							if(byte==0){ //Byte 0 means it will read the name of the input folder of the writer

								int input_len;

								if(read(fd, &input_len , sizeof(input_len)) < 0) { //First read the size
  		
									kill(gpid, SIGUSR1);
                        			exit(1);
								}


								if(read(fd, cmsgbuf , input_len +1) < 0) { //Then its name
  		
  								
									kill(gpid, SIGUSR1);
                        			exit(1);
								}

								//It is used later on, when we receive byte number 3, we simply store it for now
								
								strcpy(currdir,mirror_path); //Getting path of mirror[receiver]/[writer] and creating the dir
								strcat(currdir,"/");
								strcat(currdir,number);

								strcpy(hold,currdir);

								struct stat sm = {0};
								if (stat(currdir, &sm) == -1) {
    							mkdir(currdir, 0777);
								}
								
							}
							

							if(byte==1){ //Byte 1 means we are reading a file
								
								
									short int sone;
									if(read(fd,&sone,sizeof(sone))<0){ //Bytes of file name
										kill(gpid, SIGUSR1);
                        				exit(1);
									}

									char stwo[sone+1];
								
									if(read(fd,stwo,sone+1)<0){ //File name
										
										kill(gpid, SIGUSR1);
                        				exit(1);
									}
								
								int sthree=0;

								char path_to_id[PATH_MAX];
								strcpy(path_to_id,currdir);
								strcat(path_to_id,"/");
								strcat(path_to_id,stwo);
								int fdcr=open(path_to_id, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL , 0777); //Opening the file we will be writing later on
								
								
									if(read(fd,&sthree,sizeof(sthree))<0){ //Getting bytes of the file
										kill(gpid, SIGUSR1);
                        				exit(1);
									}

									


								char sfour[MSGSIZE];
								
								int size = sthree;
      							int scounter=0;

      							if(size<MSGSIZE){ //If writer's string length is less than reader's buffer set it equal to the string's size
       							 MSGSIZE=size;
      							}

								
								while(1){ //Entering While 1 loop where we will be reading the file's contents
  
							  	if(read(fd, sfour , MSGSIZE ) < 0) {
							  		
							  		
								  	perror("problem  in  reading");
				   				    kill(gpid, SIGUSR1);
                        			exit(1);
								}else{ //If we read something successfully
								
									if(scounter>=size){ //If scounter greater or equal than size, this means we have reached the final bytes
										MSGSIZE=size;
								    	sfour[MSGSIZE]='\0'; //Set null byte to end the string
									}else{
									  sfour[MSGSIZE]='\0'; //Same here
								  	}
									
								
								if(strlen(sfour)<MSGSIZE){//Write the string and exit if length is less than the buffer
										FILE* pFile2 = fopen(path_to_id, "a");
			
							    		fprintf(pFile2,"%s", sfour );
									
										fclose(pFile2);
										
							  			break;
							  		}
							  		else{ //Otherwise write the string and enter the loop again (except if size is equal to bytes read)
							  			FILE* pFile2 = fopen(path_to_id, "a");
		
							        	   fprintf(pFile2,"%s", sfour);
							    		
										
										fclose(pFile2);
										if(size==MSGSIZE){
        									break;
      									}

										strcpy(sfour,"0"); //Reset sfour string for the next bytes
										sfour[0]='\0';
							      		scounter=MSGSIZE;
							      		size=size-scounter; //Remaining bytes are the ones we had read minus the bytes just read

							      		if(scounter>size){ //If scounter greater than size, this means we have reached the final bytes
									        MSGSIZE=size;
									    }

							     
							  			continue;
							  		}
								}
							}

									FILE* flog=fopen(logfile,"a"); //Logfile work. Write the number of bytes we received and the file's name
									fprintf(flog, "%s %d\n", "Bytes Received:", sthree );
									fclose(flog);

									FILE* flog1=fopen(logfile,"a");
									fprintf(flog1, "%s %s\n", "File Received:", stwo );
									fclose(flog1);

								char zeros[2];
						
       						if(read(fd, zeros , 2+1 ) < 0) { //Read the two zeros
  		
  								perror("problem  in  reading");
  								kill(gpid, SIGUSR1);
                        		exit(1);
							}
							

								
							}

							if(byte==2){ //Byte no. 2 means we are free to exit the While 1 loop
								
								
								close(fd);
								unlink(common_slash2); 
								break;
							}

							if(byte==3){ //Byte no. 3 means we may read a path from a subdir
											
								char newstring[PATH_MAX+1];

								int plen;

								if(read(fd, &plen , sizeof(plen)) < 0) { //Getting length of the path
  		
  						
									kill(gpid, SIGUSR1);
                        			exit(1);
								}

								char pmsgbuf[plen+1];
							
       						if(read(fd, pmsgbuf , plen +1) < 0) { //Getting the path
  	
  								kill(gpid, SIGUSR1);
                        		exit(1);
							}

						

								strcpy(newstring,cwd);
								strcat(newstring,cmsgbuf); //Newstring string has the current working dir along with the input name
								
								char* remd=strremove(pmsgbuf,newstring); //Using custom strremove function to remove the current working dir along with the input name from the path we just read
								

								strcpy(currdir,hold);
								strcat(currdir,remd); //Current directory where we store files is the mirror[receiver]/[writer] plus the string we just got (eg. mirror1/2/subdir1/subdir2)
								
								struct stat sm = {0};
								if (stat(currdir, &sm) == -1) {
    							mkdir(currdir, 0777); //Creating the desired directory
								}

							}

							
							
							
						}
						
       				}
       				
       				
       				exit(0); //Child process exits
      			}
      		}

				if (pid != 0) {	//If we're on the father

					if(signal(SIGUSR1,sigusr_handler) == SIG_ERR){ //SIGUSR1 is received here
						printf("Something went wrong regarding the signal.\n");
                        
                    }
          
          			for(i=0; i<2; i++){ //Waiting for the children to finish
          
	  					int status;
	  					pid_t child_pid;	
          				child_pid = wait(&status);
          				if(i==1){ //If the reader process exited successfully, we may print a message for the successful transfer
          					if (WIFEXITED(status)){ 
        						printf("All files transfered successfully!\n");
        					}
          				}
          
       				}
       				

       				if(trycount== 0){ //If trycount is 0 this means we managed to complete the transfer on the first try
       					
       					break;
       				}

       				if(trycount==4 ){ //Now, the trying again process works as follows: Since we're on a while 1 loop, if the trycount variable is >1 this means the process will enter the while 1 loop to fork the children again
       					printf("I give up!\n"); //If trycount is 4 this means, we tried 3 times the transfering stage and it failed so we may move on.
                        break;
                    }else{
                        continue; //Otherwise, we're free to try again
                    }

    			}

    		}//While 1 loop ends here

    		}
      	

      	foundold=0; //Reseting the found variables for future use
      	foundid=0;
      	}
      	

    	(void) closedir (dp);  
    		
  		}else{
  			printf("error\n");
  		}

		//Code for bygone IDs
		printf("\nNow checking for the IDs that left the system.\n");
		itemp=iroot;
		while(itemp!=NULL){ //We open the common folder and check if every .id file has a match with the IDs of the client's list
			dp = opendir(common_path);
  			if (dp != NULL){
  				while (ep = readdir (dp)){
  					char temp1[MAX_SIZE];
  					strcpy(temp1,ep->d_name);
  					if(strcmp(itemp->ID,temp1)==0){
  						foundidg=1;
  						break;
  					}
  				}
  				if(foundidg==1){ //Found an ID on the folder
  					itemp=itemp->next;
  				}
  				if(foundidg==0){ //We didn't find an ID on the folder
  					
  					char numberdel[MAX_SIZE];
  					char temp[MAX_SIZE];
  					strcpy(temp,itemp->ID);
      			int r=0;
      			while(temp[r]!='\0'){ //Here we get the number without the .id extension
      				
      				if(temp[r]=='.'){
      					break;
      				}
      				numberdel[r]=temp[r];

      				
      				r++;
      			}

  					itemp->to_delete=1; //To_delete variable is set to 1 so we will delete this ID from the list later on
  					pid=fork();	//Creating a child to delete the desired folder					
       				if(pid == -1){
           				printf("Error in fork() \n");
           				return 1;
       				}
       				if(pid == 0){	
       					
       					printf("Removing mirror directory of client %s\n",numberdel );

       					char remthisd[PATH_MAX];
       					strcpy(remthisd,mirror_path);
       					strcat(remthisd,"/");
       					strcat(remthisd,numberdel); //Getting path to the folder for deletion
       					
       					int retval=execl("/bin/rm", "rm", "-rf", remthisd, NULL);
       					exit(0);
       				}
       				if(pid != 0){
       					int status;
	  					pid_t child_pid;	
          				child_pid = wait(&status);
       				}


  					itemp=itemp->next;
  				}

  				(void) closedir (dp);	
			}else{
  				printf("Error\n");
  			}
  			foundidg=0;
			
		}
		
		

		Removal(itemp,iroot,allgood); //Removal function removes from the list the IDs who exited
			

	}



}