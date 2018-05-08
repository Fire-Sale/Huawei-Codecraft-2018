#include "predict.h"
#include <ctime>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <string>
#include <cstring>
#include <stdio.h>

int CPUTYPE[] = {0 , 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16,32,32,32};
int MEMTYPE[] = {0, 1, 2, 4, 2, 4, 8, 4, 8, 16, 8, 16, 32, 16, 32, 64,32,64,128};
int DAYNUM2[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int DAYNUM1[] = {0, 31, 27, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int YEARS[] = {0, 365, 365, 365, 365, 366, 365, 365, 365, 366, 365, 365, 365, 366, 365, 365, 365, 366, 365, 365, 365, 366};


int cal(int year, int month, int day)
{	
	int RUNYEAR = 0;
	int datetmp = 0;
	if (((year % 4 == 0) && (year % 100) != 0) || (year % 400 == 0))
	{
		RUNYEAR = 1;
	}
	if (RUNYEAR == 0)
	{
		for (int i = 1; i < month; i++)
		{
			datetmp += DAYNUM1[i];
		}
		datetmp += day;
	}
	if (RUNYEAR == 1)
	{
		for (int i = 1; i < month; i++)
		{
			datetmp += DAYNUM2[i];
		}
		datetmp += day;
	}
	for (int i = 1; i < (year - 2000 + 1);i++)
	{
		datetmp+=YEARS[i];
	}
	return datetmp;	
}



typedef struct
{
	char severname[20];
	int cpusize;
	int memsize;
	float rate;
} Sever;

int* StartVmOnMechine(int predict[][2],int flavortype[],int flavortypenum,Sever selectsever,int *totalflavornum)
{
	int *usedFlavor = new int[19];	                 //下标从1开始
	for(int i=0;i<19;i++)
	{
		usedFlavor[i] = 0;
	}		
	//从大的flavor到小找合适的flavor
	int Totalflavornum = *totalflavornum;
	int flag = 0;
	int bestindex=0;
	int lastflavorate=0;
	float minerrorrate;
	float raterror;
	int recordlastsetflavortype = flavortypenum;	
	while(Totalflavornum)													    //寻找比例次之的flavor放置
	{
		if(Totalflavornum == 0)                                      //所有虚拟机已经放完
		{
			break;
		}
		if(((selectsever.cpusize==0)||(selectsever.memsize==0)))	//如果服务器的某个维度为0，则终止该服务器的装填，
		{			
			break;
		}
		int exitminflavortype =0;
		for(int i=0;i<flavortypenum;i++)                          //找出当前剩余虚拟机中最小的类型
		{
			if(predict[flavortype[i]][1] != 0)
			{
				exitminflavortype = i;
				break;
			}
		}		
		if((selectsever.cpusize < CPUTYPE[flavortype[exitminflavortype]])||(selectsever.memsize < MEMTYPE[flavortype[exitminflavortype]]))  //服务器的剩余空间连最小的和次之虚拟机都放不下了，只能退出换下一个服务器
		{
			break;
		}			
		recordlastsetflavortype = flavortypenum;                         //重新遍历	
		while(Totalflavornum)                                            //寻找比例最适合且的flavor逐降放置  此循环过完后，说明比例最适应的已经遍历完成且已经全部装填完毕
		{			
			float resourcerate;	
			if(((selectsever.cpusize==0)||(selectsever.memsize==0)))	//如果服务器的某个维度为0，则终止该服务器的装填，
			{
				break;
			}
			int exitminflavortype =0;
			for(int i=0;i<flavortypenum;i++)                          //找出当前剩余虚拟机中最小的类型
			{
				if(predict[flavortype[i]][1] != 0)
				{
					exitminflavortype = i;
					break;
				}
			}		
			if((selectsever.cpusize < CPUTYPE[flavortype[exitminflavortype]])||(selectsever.memsize < MEMTYPE[flavortype[exitminflavortype]]))  //服务器的剩余空间连最小的和次之虚拟机都放不下了，只能退出换下一个服务器
			{
				break;
			}			
			
			//计算剩余资源的比例
			resourcerate = (float)selectsever.memsize/(float)selectsever.cpusize;					
			
			for(int i=recordlastsetflavortype-1;i>=0;i--)    		//寻找比例最适合且最大的flavor
			{	
										
				if(predict[flavortype[i]][1] != 0)                      //当前虚拟机还有剩余
				{
					//printf("%d %d\n",i,flavortype[i]);
					int flavorate = MEMTYPE[flavortype[i]]/CPUTYPE[flavortype[i]];	
										
					if(lastflavorate == flavorate)                      //判断比例是否相同，如果相同则跳过，因为要选取比例最适合的且最大的
					{
						continue ;
					}
					lastflavorate = flavorate;                          //记录上一次比例
					raterror = fabs(resourcerate-flavorate);
					if(flag)
					{
						if(minerrorrate > raterror)                    //找出比例最适应虚拟机
						{
							minerrorrate = raterror;
							bestindex = i;
						}			
					}
					else
					{
						flag = 1;
						minerrorrate = raterror;
						bestindex = i;
					}
				}
				//ok									
			}			
			lastflavorate = 0;		
			flag = 0;		
						
			if((selectsever.cpusize >= CPUTYPE[flavortype[bestindex]])&&(selectsever.memsize >= MEMTYPE[flavortype[bestindex]]))
			{				
				selectsever.cpusize -= CPUTYPE[flavortype[bestindex]];
				selectsever.memsize -= MEMTYPE[flavortype[bestindex]];
				predict[flavortype[bestindex]][1] -=1;
				Totalflavornum -= 1;
				usedFlavor[flavortype[bestindex]] += 1;                     //记录当前服务器存放虚拟机列表
				recordlastsetflavortype = flavortypenum; 			
				
			}
			else
			{				
				 recordlastsetflavortype = bestindex;                           //下一个循环遍历后面服务器
			}		
		}		
	} 	

	*totalflavornum = Totalflavornum;
	
	return usedFlavor;			
}


typedef struct 
{
	char severname[20];
	int flavorlist[19];
} Usedsever;

Usedsever usedsever[3][20000];             //先预留20000台服务器

//初始化配置
void InitSelecteSever(int predict[][2],int flavortype[], int totalflavornum ,int flavortypenum ,Sever severtype[],int needsever[])
{
	//while (未分配的flaver不等于0)		
	int needGeneralsevernum=0;	
	int needLargesevernum=0;
	int needHighsevernum=0;	
	int lastSeverFlag =0;	
	while(totalflavornum)                                 //每次循环都是重新放置一个完全空的服务器，并记录下该服务器标号
	{
	//计算未分配flaver的CPU n内存比例
		int restofcpu = 0;
		int restofmem = 0;
		int *placeflavor;	               //该变量中包含了每个服务器装有虚拟机列表情况		
		for(int i=0;i<flavortypenum;i++)
		{
			restofcpu += predict[flavortype[i]][1]*CPUTYPE[flavortype[i]];
			restofmem += predict[flavortype[i]][1]*MEMTYPE[flavortype[i]];
		}	
			
		float rate = (float)restofmem/(float)restofcpu;		
		//根据比例选一个合适的宿主机
		if(abs(rate - severtype[1].rate)<abs(rate - severtype[2].rate))                                  //说明服务器1的比例更接近剩余资源比
		{
			lastSeverFlag = 0;	
			strcpy(usedsever[1][needLargesevernum].severname,severtype[1].severname);
			//printf("%f \n%s \n",rate,usedsever[1][needLargesevernum].severname);
			placeflavor=StartVmOnMechine(predict,flavortype,flavortypenum,severtype[1],&totalflavornum);
			for(int i=0;i<flavortypenum;i++)
			{
				usedsever[1][needLargesevernum].flavorlist[flavortype[i]] = placeflavor[flavortype[i]];
			}			
			delete placeflavor;			
			needLargesevernum += 1;
		}
		else
		{	
			lastSeverFlag = 1;			
			//放置flvaler到宿主机上
			//StartVmOnMechine(StartVmOnMechine[2]);                                                      //每调用一次就放置完一个服务器	
			strcpy(usedsever[2][needHighsevernum].severname,severtype[2].severname);
			//printf("%f \n%s \n",rate,usedsever[2][needHighsevernum].severname);
			placeflavor=StartVmOnMechine(predict,flavortype,flavortypenum,severtype[2],&totalflavornum);
			for(int i=0;i<flavortypenum;i++)
			{
				usedsever[2][needHighsevernum].flavorlist[flavortype[i]] = placeflavor[flavortype[i]];
			}
			delete placeflavor;			
			needHighsevernum += 1;			
		}

	}
	//生成 宿主机上 VM列表  上面循环结束 就已经生成了一个服务器和虚拟机的列表
	   
	//return ;                       //先将程序调到该阶段

	//对最后一个服务器检测是否可以缩容

	int cpuSizeOfLastSever=0,memSizeOfLastSever=0;
	float utilizeRateOfLastSever = 0.0;
	if(lastSeverFlag)               //High Sever
	{
		for(int i=0;i<flavortypenum;i++)
		{
			cpuSizeOfLastSever += usedsever[2][needHighsevernum-1].flavorlist[flavortype[i]]*CPUTYPE[flavortype[i]];
			memSizeOfLastSever += usedsever[2][needHighsevernum-1].flavorlist[flavortype[i]]*MEMTYPE[flavortype[i]];
		}
		if((cpuSizeOfLastSever <= severtype[0].cpusize)&&((memSizeOfLastSever <= severtype[0].memsize)))          //General
		{
			strcpy(usedsever[0][needGeneralsevernum].severname,severtype[0].severname);
			for(int i=0;i<flavortypenum;i++)
			{
				usedsever[0][needGeneralsevernum].flavorlist[flavortype[i]] = usedsever[2][needHighsevernum-1].flavorlist[flavortype[i]] ;
			}
			needHighsevernum -= 1;
			needGeneralsevernum += 1;
		}
		else                                                                                                     
		{
			if((cpuSizeOfLastSever <= severtype[1].cpusize)&&((memSizeOfLastSever <= severtype[1].memsize)))          //Large
			{
				int tmprate = 0;
				utilizeRateOfLastSever = (((float)cpuSizeOfLastSever/(float)severtype[2].cpusize) + ((float)memSizeOfLastSever/(float)severtype[2].memsize))/2.0;
				tmprate = (((float)cpuSizeOfLastSever/(float)severtype[1].cpusize) + ((float)memSizeOfLastSever/(float)severtype[1].memsize))/2.0;
				if(tmprate>utilizeRateOfLastSever)
				{
					for(int i=0;i<flavortypenum;i++)
					{
						usedsever[1][needLargesevernum].flavorlist[flavortype[i]] = usedsever[2][needHighsevernum-1].flavorlist[flavortype[i]] ;
					}
					needHighsevernum -= 1;
					needLargesevernum += 1;
				}
			}
		}
	}
	else                            //Large Sever
	{
		for(int i=0;i<flavortypenum;i++)
		{
			cpuSizeOfLastSever += usedsever[1][needLargesevernum-1].flavorlist[flavortype[i]]*CPUTYPE[flavortype[i]];
			memSizeOfLastSever += usedsever[1][needLargesevernum-1].flavorlist[flavortype[i]]*MEMTYPE[flavortype[i]];
		}		
		if((cpuSizeOfLastSever <= severtype[0].cpusize)&&((memSizeOfLastSever <= severtype[0].memsize)))          //General
		{
			strcpy(usedsever[0][needGeneralsevernum].severname,severtype[0].severname);
			for(int i=0;i<flavortypenum;i++)
			{
				usedsever[0][needGeneralsevernum].flavorlist[flavortype[i]] = usedsever[1][needLargesevernum-1].flavorlist[flavortype[i]] ;
			}					
			needLargesevernum -= 1;
			needGeneralsevernum += 1;
		}
		else                                                                                                     
		{
			if((cpuSizeOfLastSever <= severtype[2].cpusize)&&((memSizeOfLastSever <= severtype[2].memsize)))          //Large
			{
				int tmprate = 0;
				utilizeRateOfLastSever = (((float)cpuSizeOfLastSever/(float)severtype[1].cpusize) + ((float)memSizeOfLastSever/(float)severtype[1].memsize))/2.0;
				tmprate = (((float)cpuSizeOfLastSever/(float)severtype[2].cpusize) + ((float)memSizeOfLastSever/(float)severtype[2].memsize))/2.0;
				if(tmprate>utilizeRateOfLastSever)
				{
					for(int i=0;i<flavortypenum;i++)
					{
						usedsever[2][needHighsevernum].flavorlist[flavortype[i]] = usedsever[1][needLargesevernum-1].flavorlist[flavortype[i]] ;
					}					
					needLargesevernum -= 1;
					needHighsevernum += 1;
				}
			}
		}
	}


	   //检测最后一个物理机器到底用了多少利用率  看另外2中宿主机 如果利用率可以更高，更换最后一台为利用率最高的

	//生成 宿主机上 VM列表
	needsever[0] = needGeneralsevernum;
	needsever[1] = needLargesevernum;
	needsever[2] = needHighsevernum;
}

void change(void)
{
	//随机挑两个宿主机上面的虚拟机，互换位置

}

void IsBest(void)
{
 	//计算有没有宿主机超资源限制了， 如果有，非最优解

	 //对比互换前和互换后的利用率差值是否变大，如果变大，则更优秀。

	 //检查变大的这个宿主机资源能否 再装一个VM，并且装完后， 利用率差值更大。



}
//检查一下是否可以删掉/缩容 最后一个宿主机


void printData(float Data[][19],int maxDate)
{
	for(int i=0;i<=maxDate;i++)
	{
		printf("%d ",i);
		for(int j=0;j<19;j++)
		{
			printf("%2.1f\t",Data[i][j]);
		}
		printf("\n");
	}
}


void exponentialSmoothing(float alpha,int dataLen, float data[][19],int flavorType[], int flavorTypeNum,float s2_1_new1[][19],float s2_2_new1[][19] )
{
	// float s1[dataLen][19];
	// float s2[dataLen][19];
	float s1[dataLen+10][19];
	float s2[dataLen+10][19];
	
	for(int i=0;i<=dataLen+1;i++)   //init
	{
		for(int j=0;j<19;j++)
		{
			s1[i][j] = 0;
			s2[i][j] = 0;
			s2_1_new1[i][j]=0;
			s2_2_new1[i][j]=0;
		}				
	}
	int smooDateRange = 3+1;
	for(int i=1;i<19;i++)
	{
		float tmp=0;
		for(int j=1;j<smooDateRange;j++)
		{
			tmp += data[j][i];
		}
		tmp /= (smooDateRange-1);
		s1[0][i] = tmp;
		s2[0][i] = tmp;		
	}	
	for(int i=0 ;i<18;i++)
	{		
		for(int j=0;j<dataLen;j++)
		{			
			if(j==0)
			{				
				s2_1_new1[j][i] = alpha*data[j+1][i+1] + (1-alpha)*s1[j][i+1];										
			}
			else
			{
				s2_1_new1[j][i] = alpha*data[j+1][i+1] + (1-alpha)*s2_1_new1[j-1][i];			
			}			
		}
	}
	for(int i=0 ;i<18;i++)
	{		
		for(int j=0;j<dataLen;j++)
		{			
			if(j==0)
			{				
				s2_2_new1[j][i] = alpha*s2_1_new1[j][i] + (1-alpha)*s2[j][i+1];						
			}
			else
			{
				s2_2_new1[j][i] = alpha*s2_1_new1[j][i] + (1-alpha)*s2_2_new1[j-1][i];							
			}
		}
	}		
}


// //Change
//��Ҫ���ɵĹ��������� cputype              traindata            traindata line number  output result 
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// clock_t starttime,finishtime;  
    // start=clock(); 
	int cnt=0;
	int flag=0;
	//int mode=0;	
	if(data==NULL)
	{
		printf("data information is none\n");
		return ;
	}

	if(info==NULL)
	{
		printf("input file information is none\n");
		return ;
	}	
	int traindata[data_num+100][3];
	for(int i=0;i<3;i++)
	{
		traindata[0][i]=0;		
	}		
	int startdateorigin=0;	
	for(int index=0;index<data_num;index++)
	{		
		char *item;
		cnt+=1;
		item=data[index];
		char mac[30];		
		char flavortype[30];		
		int flavornum;
		char Date[30];	
		int year,month,day;			
		sscanf(item, "%s %s %s",mac,flavortype,Date);		
		sscanf(flavortype,"%*[^r]r%d",&flavornum);	
		sscanf(Date,"%d-%d-%d",&year,&month,&day);
		int date;
		date=cal(year,month,day);		
		if(cnt==1)
		{			
			startdateorigin=date-1;
		}
		date=date-startdateorigin;
		if(flavornum>=19)
		{
			cnt -= 1;
			continue;
		}
		traindata[cnt][0]=flavornum;
		traindata[cnt][1]=date;
		traindata[cnt][2]=cnt;	
	}	
	int maxDate = traindata[cnt][1];
	float Data[maxDate+100][19];
	for(int i=1;i<maxDate+60;i++)
	{
		Data[i][0]=i;
		for(int j=1;j<19;j++)
		{
			Data[i][j]=0;
		}
	}
	
	
	for(int i=1;i<cnt+1;i++)
	{
		int date ,flavor;		
		date=traindata[i][1];
		flavor=traindata[i][0];				
		Data[date][flavor]+=1;
						
	}	




	//输入文件处理
	//int cpuNum[3],memNum[3],diskNum[3];
	int flavortypeNum;
	int severtypenum;
	int typeList[19];                         //默认最大19种类型
	
	int startYear,startMonth,startDay;
	int endYear,endMonth,endDay;
	Sever severtype[3];                    	 //假设云平台有三种服务器	
	for(int index =0;;index++)
	{
		char *item;	
					
		item=info[index];
		
		if(item==NULL)
		{
			break;
		}
		if(index == 0)
		{
			sscanf(item,"%d",&severtypenum);              //提取服务器类型数
		}
		if((strstr(item,"General")!=NULL))
		{
			sscanf(item,"%s[^ ]",severtype[0].severname);
			sscanf(item,"%*[^ ] %d %d",&severtype[0].cpusize,&severtype[0].memsize);
			severtype[0].rate = (float)severtype[0].memsize/severtype[0].cpusize;
		}	
		if((strstr(item,"Large")!=NULL))
		{
			sscanf(item,"%s[^ ]",severtype[1].severname);
			sscanf(item,"%*[^ ] %d %d",&severtype[1].cpusize,&severtype[1].memsize);
			severtype[1].rate = (float)severtype[1].memsize/severtype[1].cpusize;
		}	
		if((strstr(item,"High")!=NULL))
		{
			sscanf(item,"%s[^ ]",severtype[2].severname);
			sscanf(item,"%*[^ ] %d %d",&severtype[2].cpusize,&severtype[2].memsize);
			severtype[2].rate = (float)severtype[2].memsize/severtype[2].cpusize;
		}	
		if(index==5)
		{
			sscanf(item,"%d",&flavortypeNum);						
		}
		if(strstr(item,"flavor")!=NULL)
		{
			int flavornum;
			static int index=0;
			sscanf(item,"%*[^r]r%d",&flavornum);	
			typeList[index]=flavornum;
			index++;
						
		}			
		if((strstr(item,"201")!=NULL)&&(flag==0))
		{
			flag=1;
			sscanf(item,"%d-%d-%d",&startYear,&startMonth,&startDay);
		}
		if((strstr(item,"201")!=NULL)&&(flag==1))
		{			
			sscanf(item,"%d-%d-%d",&endYear,&endMonth,&endDay);	
		}
	}

	int needSever[severtypenum];

	int startDate,endDate;
	int predictTime;
	int breakTime;
	startDate=cal(startYear,startMonth,startDay);
	endDate=cal(endYear,endMonth,endDay);
	startDate -= startdateorigin;
	endDate -= startdateorigin;
	predictTime =endDate-startDate;
	breakTime = startDate - maxDate;
	predictTime += breakTime;
     

	float avg[19];
	//float var[16];

	for(int i=0;i<19;i++)
	{
		avg[i]=0;
	//	var[i]=0;
	}
	
	for(int i=1;i<19;i++)
	{
		int tmp=0;
		for(int j=1;j<maxDate+1;j++)
		{
			tmp +=Data[j][i];
		}
		avg[i]=tmp/maxDate;		
	}	

	for(int i=1;i<19;i++)
	{
		for(int j=1;j<maxDate+1;j++)
		{
			if(Data[j][i]>avg[i]*6)
			{
				Data[j][i]=avg[i]*3.8;                
			}
		}
	}
	
	printData(Data,maxDate);
	//float DataTmp[60][19];
	// float total = 0;
	// for(int i=1;i<predictTime+1;i++)
	// {
	// 	for(int j=0;j<flavortypeNum;j++)
	// 	{
	// 		float tot=0;
	// 		for(int k=maxDate+i-30;k<maxDate+i+1;k++)
	// 		{
	// 			tot+=Data[k][typeList[j]];				
	// 		}			
	// 		tot = (int)(tot/30);
	// 		total +=tot;		
	// 		Data[maxDate+i][typeList[j]] = tot;		
	// 		Data[maxDate+i][typeList[j]] +=5.3;	
	// 	}				
	// }		
	float alpha = 0.92;
	float s1[maxDate+60][19];
	float s2[maxDate+60][19];
	float At=0,Bt=0;
	float sum1=0;
	for(int i=1;i<predictTime+1;i++)
	{
		exponentialSmoothing(alpha,maxDate+i-1,Data,typeList,flavortypeNum,s1,s2);
		//printData(s1,maxDate);
		//return ;
		for(int j=0;j<18;j++)
		{
			At = s2[maxDate+i-2][j]*2-s1[maxDate+i-2][j];
			Bt = (alpha/(1-alpha))*(s2[maxDate+i-2][j]-s1[maxDate+i-2][j]);
			sum1 = At + Bt;
			if((int)sum1 != 0)
			{
				sum1 +=1;
			}
			Data[maxDate+i][j+1] = sum1;
		}
	}
	
	//printf("%d\n",maxDate);
	//return;
	
	//return ;

	int predictList[19][2];
	for(int i=0;i<19;i++)
	{
		predictList[i][0] = 0;
		predictList[i][1] = 0;
	}

	int totaltmp=0;
	for(int i=0;i<flavortypeNum;i++)
	{
		float tmp=0;
		for(int j=1;j<predictTime+1-breakTime;j++)
		{			
			tmp+=Data[maxDate+j+breakTime][typeList[i]];
		}			
	
		totaltmp +=(int)tmp;
		predictList[typeList[i]][0]=typeList[i];
		predictList[typeList[i]][1]=(int)tmp;
	}	

	//int TEST[]={0,  5,5,5,10,5,   0,10,5,0,5,   0,10,0,0,0,   5,0,0};
    //              +  +  + + +    ~ + + +
	// for(int i=0;i<19;i++)
	// {
	// 	if(predictList[i][1]!=0)
	// 	{
	// 		predictList[i][1]+=TEST[i];
	// 		totaltmp += TEST[i];
	// 	}
	// }	
	
	
	int totalcpu=0,totalmem=0;
	for(int i=0;i<flavortypeNum;i++)
	{
		totalcpu += predictList[typeList[i]][1]*CPUTYPE[typeList[i]];
		totalmem += predictList[typeList[i]][1]*MEMTYPE[typeList[i]];
	}
	int copyPredictList[19][2];
	for(int i=0;i<19;i++)
	{
		copyPredictList[i][0] = predictList[i][0];
		copyPredictList[i][1] = predictList[i][1];
	}
	InitSelecteSever(copyPredictList,typeList,totaltmp,flavortypeNum,severtype,needSever);	
	

	// int reduceBagNum[3]={0,0,0};
	// for(int i=1;i<3;i++)
	// {
	// 	if(needSever[i] != 0)
	// 	{
	// 		for(int j=0;j<needSever[i];j++)
	// 		{
	// 			for(int k=0;k<flavortypeNum;k++)
	// 			{
	// 				totalcpu += usedsever[i][j].flavorlist[typeList[k]] * CPUTYPE[typeList[k]];
	// 				totalmem += usedsever[i][j].flavorlist[typeList[k]] * MEMTYPE[typeList[k]];
	// 			}
	// 			if(((totalcpu/severtype[i].cpusize)<0.95)||((totalmem/severtype[i].memsize)<0.95))
	// 			{
	// 				for(int k=0;k<flavortypeNum;k++)
	// 				{
	// 					predictList[typeList[k]][1] -= usedsever[i][j].flavorlist[typeList[k]];
	// 					totaltmp -= usedsever[i][j].flavorlist[typeList[k]];
	// 				}
	// 				reduceBagNum[i] += 1;
	// 				for(int m=j;m<needSever[i];m++)
	// 				{
	// 					for(int n=0;n<flavortypeNum;n++)
	// 					{
	// 						usedsever[i][m].flavorlist[typeList[n]] = usedsever[i][m+1].flavorlist[typeList[n]];
	// 					}
	// 				}
	// 		    }
	// 	    }
	//     }
	// }
	char  result_file[500000] ="\0";
	char  flavortotal[200];
	sprintf(flavortotal,"%d",totaltmp);
	strcat(flavortotal,"\n");
	strcat(result_file,flavortotal); 	

	for(int i=0;i<flavortypeNum;i++)                                   //依次将结果写入文件
	{
		char tmp2[5],tmp3[10];
		char tmp1[20] = "flavor";
		sprintf(tmp2,"%d",typeList[i]);
		strcat(tmp1,tmp2); 
        strcat(tmp1," ");
		sprintf(tmp3,"%d",predictList[typeList[i]][1]);
		strcat(tmp1,tmp3);
		strcat(tmp1,"\n");
		strcat(result_file,tmp1);			
	}
	strcat(result_file,"\n");

	char result_file1[400000]="\0";		
	for(int i=0;i<3;i++)
	{
		if(needSever[i] != 0)       //
		{			
			char severnum[10]="\0";
			strcat(result_file1,usedsever[i][0].severname);
			strcat(result_file1," ");
			sprintf(severnum,"%d",needSever[i]);
			strcat(result_file1,severnum);
			strcat(result_file1,"\n");			
		}	
		else
		{
			continue ;
		}		
		 for(int j=0;j<needSever[i];j++)
		 {
			char labeltype[100]="\0";
			char labelnum[10] = "\0";
			strcat(labeltype,usedsever[i][j].severname);			
			strcat(labeltype,"-");
			sprintf(labelnum,"%d",j+1);
			strcat(labeltype,labelnum);			
			strcat(labeltype," ");			
			strcat(result_file1,labeltype);
			for(int k=0;k<flavortypeNum;k++)
			{
				if(usedsever[i][j].flavorlist[typeList[k]] != 0)
				{
					char tmp2[5] = "\0";
					char tmp3[10] = "\0";
					char tmp1[20] = "flavor";
					sprintf(tmp2,"%d",typeList[k]);
					strcat(tmp1,tmp2);
					strcat(tmp1," ");				
					sprintf(tmp3,"%d",usedsever[i][j].flavorlist[typeList[k]]);
					//totalcount += usedsever[i][j].flavorlist[typeList[k]];
					strcat(tmp1,tmp3);					
					strcat(result_file1,tmp1);					
					strcat(result_file1," ");
				}
			}
			strcat(result_file1,"\n");		
		 }	
		strcat(result_file1,"\n");	
	}		
	strcat(result_file,result_file1);
	 printData(Data,maxDate+predictTime);
	printf("%s\n",result_file);	
	//printf("%d \n",totalcount);
	write_result(result_file, filename);	
}
