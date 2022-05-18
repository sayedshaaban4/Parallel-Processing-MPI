#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mpi.h"


int main(int argc , char * argv[])
{
	/* Total Number Of Students Passed */
	int ans=0;
	
	/* Rank Of Each Process */
    int my_rank;
	
	/* Number Of Processes */
    int p;
	
	/* Return Status For Recieve */
	MPI_Status status;


    /* Start Up MPI */
    MPI_Init( &argc , &argv );

    /* Find Out Process Rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find Out Number Of Processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);


	/* Master process */
	if(my_rank==0)
	{
		/* Some Needed Variables At This Block */
		int num,cnt=0,sz,part,need,exceed,tot,index=0,i,j,k,dataArr[1000000],processArr[1000000],needtoAdd;
		bool hasExtra=true;
		
		/* 
		File Format is : Stud ID  Grade
		ex : 20190000 84
		*/
		FILE* file;
		
		/* Here I Count Number Of Students From The File */
		file = fopen("/shared/Data.txt","r");
		while(fscanf(file,"%d",&num)==1)
			cnt++;
		fclose(file);
		
		/* Here I Assign The Students' Data To 1D Array */
		file = fopen("/shared/testcases.txt","r");
		cnt=0;
		while(fscanf(file,"%d",&num)==1)
		{
			dataArr[cnt]=num;
			cnt++;
		}
		fclose(file);
		
		/* 
		Here after Calc. number of students,
		then divided them to all processes
		and if the division is not perfect,
		then will be more students assign to last process
		*/
		sz=(cnt/2);
		part=(sz/(p-1));
		need=(sz%(p-1));
		exceed = need/(p-1);
		
		/* Assign to each prosses array of students */
		for(i=1;i<p;i++)
		{
			/* The processes have the same number of students  */ 
			MPI_Send(&part,1,MPI_INT , i , 0 , MPI_COMM_WORLD);
			for(j=0;j<(part*2);j++)
			{
				processArr[j]=dataArr[index];
				index++;
			}
		
			/*
			Remaining students that do not belong to any process 
			will be divided to all processes
			*/ 			
				needtoAdd=0;
				hasExtra=true;
				if(need>0)
				{
					hasExtra=false;
					if(need%(p-1)!=0) needtoAdd=exceed+1;
					else needtoAdd=exceed;
					need-=needtoAdd;
					int lastIndx=(part*2);
					for(k=0;k<(needtoAdd*2);k++)
					{
						processArr[lastIndx]=dataArr[index];
						lastIndx++;
						index++;
					}
					MPI_Send(&needtoAdd,1,MPI_INT , i , 0 , MPI_COMM_WORLD);
				}
				
				/* 
				send to process its extra students,
				then send the updated array
				*/
				if(hasExtra==true)
					MPI_Send(&needtoAdd,1,MPI_INT , i , 0 , MPI_COMM_WORLD);
				MPI_Send(&processArr,1000000,MPI_INT , i , 0 , MPI_COMM_WORLD);
		}
		
		/* Here to Calc. the sum of passed students recieved from each process */
		for(i=1;i<p;i++)
		{
			int sum=0;
			MPI_Recv(&sum, 1 , MPI_INT, i, 0, MPI_COMM_WORLD, &status );
			ans+=sum;
		}
		printf("Total number of Students Passed The Exam is %d out of %d \n",ans,sz);
	}
	
	/* Slave Processes */
	else
	{
		/* Some Needed Variables At This Block */
		int num,processArr[1000000],i,cnt=0,toadd;
		
		/* 
		First recieve the number of students,
		second recieve the number of extra students,
		then recieve the array of students
		*/
		MPI_Recv(&num, 1 , MPI_INT, 0 , 0 , MPI_COMM_WORLD , &status);
		MPI_Recv(&toadd, 1 , MPI_INT, 0 , 0 , MPI_COMM_WORLD , &status);
		MPI_Recv(&processArr, 1000000 , MPI_INT, 0 , 0 , MPI_COMM_WORLD , &status);
		num+=toadd;
		
		/* 
		loop over the array and calc. the passed students 
		and appear message if the student pass or not 
		*/
		for(i=0;i<(num*2);i+=2)
		{
			if(processArr[i+1]>59)
			{
				printf(" %d Passed The Exam \n", processArr[i]);
				cnt++;
			}
			else
			{
				printf(" %d Failed. Please Repeat The Exam \n", processArr[i]);
			}
		}
		MPI_Send(&cnt,1,MPI_INT , 0 , 0 , MPI_COMM_WORLD);
	}
	
    
  /* shutdown MPI */
    MPI_Finalize();
	
    return 0;
}
