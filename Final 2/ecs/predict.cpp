#include "predict.h"
#include <ctime>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstring>

int CPUTYPE[] = {0 , 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16, 32, 32, 32};
int MEMTYPE[] = {0, 1, 2, 4, 2, 4, 8, 4, 8, 16, 8, 16, 32, 16, 32, 64, 32, 64, 128};
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
		datetmp += YEARS[i];
	}
	return datetmp;	
}

typedef struct
{
	char serverName[20];
	int cpusize;
	int memsize;
	float rate;
} Server;

int* StartVmOnMachine(int predict[][2], int flavorType[], int flavorTypeNum, Server selectServer, int *totalFlavorNum)
{
	int *usedFlavor = new int[19];	                 //下标从1开始
	for(int i=0;i<19;i++)
	{
		usedFlavor[i] = 0;
	}		
	//从大的flavor到小找合适的flavor
	int TotalFlavorNum = *totalFlavorNum;
	int flag = 0;
	int bestIndex=0;
	int lastFlavorRate=0;
	float minErrorRate;
	float rateError;
	int recordLastSetflavorType = flavorTypeNum;	
	while(TotalFlavorNum)													    //寻找比例次之的flavor放置
	{
		if(TotalFlavorNum == 0)                                      //所有虚拟机已经放完
		{
			break;
		}
		if( (selectServer.cpusize == 0) || (selectServer.memsize == 0) )	//如果服务器的某个维度为0，则终止该服务器的装填，
		{			
			break;
		}
		int exitMinflavorType =0;
		for(int i=0;i<flavorTypeNum;i++)                          //找出当前剩余虚拟机中最小的类型
		{
			if(predict[flavorType[i]][1] != 0)
			{
				exitMinflavorType = i;
				break;
			}
		}		
		if((selectServer.cpusize < CPUTYPE[flavorType[exitMinflavorType]]) || (selectServer.memsize < MEMTYPE[flavorType[exitMinflavorType]]))  //服务器的剩余空间连最小的和次之虚拟机都放不下了，只能退出换下一个服务器
		{
			break;
		}			
		recordLastSetflavorType = flavorTypeNum;                         //重新遍历	
		while(TotalFlavorNum)                                            //寻找比例最适合且的flavor逐降放置  此循环过完后，说明比例最适应的已经遍历完成且已经全部装填完毕
		{
			float resourceRate;	
			if( (selectServer.cpusize == 0) || (selectServer.memsize == 0) )	//如果服务器的某个维度为0，则终止该服务器的装填，
			{
				break;
			}
			int exitMinflavorType =0;
			for(int i=0;i<flavorTypeNum;i++)                          //找出当前剩余虚拟机中最小的类型
			{
				if(predict[flavorType[i]][1] != 0)
				{
					exitMinflavorType = i;
					break;
				}
			}		
			if((selectServer.cpusize < CPUTYPE[flavorType[exitMinflavorType]])||(selectServer.memsize < MEMTYPE[flavorType[exitMinflavorType]]))  //服务器的剩余空间连最小的和次之虚拟机都放不下了，只能退出换下一个服务器
			{
				break;
			}			
			
			//计算剩余资源的比例
			resourceRate = (float)selectServer.memsize/(float)selectServer.cpusize;					
			
			for(int i=recordLastSetflavorType-1;i>=0;i--)    		//寻找比例最适合且最大的flavor
			{	
										
				if(predict[flavorType[i]][1] != 0)                      //当前虚拟机还有剩余
				{
					int flavorRate = MEMTYPE[flavorType[i]]/CPUTYPE[flavorType[i]];	
										
					if(lastFlavorRate == flavorRate)                      //判断比例是否相同，如果相同则跳过，因为要选取比例最适合的且最大的
					{
						continue ;
					}
					lastFlavorRate = flavorRate;                          //记录上一次比例
					rateError = fabs(resourceRate-flavorRate);
					if(flag)
					{
						if(minErrorRate > rateError)                    //找出比例最适应虚拟机
						{
							minErrorRate = rateError;
							bestIndex = i;
						}			
					}
					else
					{
						flag = 1;
						minErrorRate = rateError;
						bestIndex = i;
					}
				}				
			}			
			lastFlavorRate = 0;		
			flag = 0;		
			if((selectServer.cpusize >= CPUTYPE[flavorType[bestIndex]])&&(selectServer.memsize >= MEMTYPE[flavorType[bestIndex]]))
			{				
				selectServer.cpusize -= CPUTYPE[flavorType[bestIndex]];
				selectServer.memsize -= MEMTYPE[flavorType[bestIndex]];
				predict[flavorType[bestIndex]][1] -=1;
				TotalFlavorNum -= 1;
				usedFlavor[flavorType[bestIndex]] += 1;                     //记录当前服务器存放虚拟机列表
				recordLastSetflavorType = flavorTypeNum; 			
				
			}
			else
			{				
				 recordLastSetflavorType = bestIndex;                           //下一个循环遍历后面服务器
			}		
		}		
	} 	

	*totalFlavorNum = TotalFlavorNum;
	
	return usedFlavor;			
}

typedef struct 
{
	char serverName[20];
	int flavorList[19];
} Usedserver;

Usedserver usedServer[3][20000];             //先预留500台服务器

void InitSelecteServer(int predict[][2], int flavorType[], int totalFlavorNum, int flavorTypeNum, Server serverType[], int needSever[])
{
	//while (未分配的flaver不等于0)		
	int needGeneralserverNum = 0;	
	int needLargeserverNum = 0;
	int needHighserverNum = 0;	
	int lastSeverFlag = 0;  						//0:表示最后一个服务器为Large 1:表示最后一个服务器为High 	
	while(totalFlavorNum)                                 //每次循环都是重新放置一个完全空的服务器，并记录下该服务器标号
	{
	//计算未分配flaver的CPU n内存比例
		int restOfCPU = 0;
		int restOfMEM = 0;
		int *placeFlavor;	               //该变量中包含了每个服务器装有虚拟机列表情况		
		for(int i=0;i<flavorTypeNum;i++)
		{
			restOfCPU += predict[flavorType[i]][1] * CPUTYPE[flavorType[i]];
			restOfMEM += predict[flavorType[i]][1] * MEMTYPE[flavorType[i]];
		}	
			
		float rate = (float)restOfMEM / (float)restOfCPU;		
		//根据比例选一个合适的宿主机
		if(abs(rate - serverType[1].rate) < abs(rate - serverType[2].rate))                                  //说明服务器1的比例更接近剩余资源比
		{
			lastSeverFlag = 0;	
			strcpy(usedServer[1][needLargeserverNum].serverName,serverType[1].serverName);
			placeFlavor = StartVmOnMachine(predict,flavorType,flavorTypeNum,serverType[1],&totalFlavorNum);
			for(int i=0;i<flavorTypeNum;i++)
			{
				usedServer[1][needLargeserverNum].flavorList[flavorType[i]] = placeFlavor[flavorType[i]];
			}			
			delete placeFlavor;			
			needLargeserverNum += 1;
		}
		else
		{		
			lastSeverFlag = 1;		
			//放置flvaler到宿主机上
            //每调用一次就放置完一个服务器	
			strcpy(usedServer[2][needHighserverNum].serverName,serverType[2].serverName);
			placeFlavor = StartVmOnMachine(predict,flavorType,flavorTypeNum,serverType[2],&totalFlavorNum);
			for(int i=0;i<flavorTypeNum;i++)
			{
				usedServer[2][needHighserverNum].flavorList[flavorType[i]] = placeFlavor[flavorType[i]];
			}
			delete placeFlavor;			
			needHighserverNum += 1;			
		}

	}
	//生成 宿主机上 VM列表  上面循环结束 就已经生成了一个服务器和虚拟机的列表
	   
	//return ;                       //先将程序调到该阶段

	//对最后一个服务器检测是否可以缩容
	
	int cpuSizeOfLastSever=0,memSizeOfLastSever=0;
	float utilizeRateOfLastSever = 0.0;
	if(lastSeverFlag)               //High Sever
	{
		for(int i=0;i<flavorTypeNum;i++)
		{
			cpuSizeOfLastSever += usedServer[2][needHighserverNum-1].flavorList[flavorType[i]]*CPUTYPE[flavorType[i]];
			memSizeOfLastSever += usedServer[2][needHighserverNum-1].flavorList[flavorType[i]]*MEMTYPE[flavorType[i]];
		}
		if((cpuSizeOfLastSever <= serverType[0].cpusize)&&((memSizeOfLastSever <= serverType[0].memsize)))          //General
		{
			strcpy(usedServer[0][needGeneralserverNum].serverName,serverType[0].serverName);
			for(int i=0;i<flavorTypeNum;i++)
			{
				usedServer[0][needGeneralserverNum].flavorList[flavorType[i]] = usedServer[2][needHighserverNum-1].flavorList[flavorType[i]] ;
			}
			needHighserverNum -= 1;
			needGeneralserverNum += 1;
		}
		else                                                                                                     
		{
			if((cpuSizeOfLastSever <= serverType[1].cpusize)&&((memSizeOfLastSever <= serverType[1].memsize)))          //Large
			{
				int tmprate = 0;
				utilizeRateOfLastSever = (((float)cpuSizeOfLastSever/(float)serverType[2].cpusize) + ((float)memSizeOfLastSever/(float)serverType[2].memsize))/2.0;
				tmprate = (((float)cpuSizeOfLastSever/(float)serverType[1].cpusize) + ((float)memSizeOfLastSever/(float)serverType[1].memsize))/2.0;
				if(tmprate>utilizeRateOfLastSever)
				{
					for(int i=0;i<flavorTypeNum;i++)
					{
						usedServer[1][needLargeserverNum].flavorList[flavorType[i]] = usedServer[2][needHighserverNum-1].flavorList[flavorType[i]] ;
					}
					needHighserverNum -= 1;
					needLargeserverNum += 1;
				}
			}
		}
	}
	else                            //Large Sever
	{
		for(int i=0;i<flavorTypeNum;i++)
		{
			cpuSizeOfLastSever += usedServer[1][needLargeserverNum-1].flavorList[flavorType[i]]*CPUTYPE[flavorType[i]];
			memSizeOfLastSever += usedServer[1][needLargeserverNum-1].flavorList[flavorType[i]]*MEMTYPE[flavorType[i]];
		}		
		if((cpuSizeOfLastSever <= serverType[0].cpusize)&&((memSizeOfLastSever <= serverType[0].memsize)))          //General
		{
			strcpy(usedServer[0][needGeneralserverNum].serverName,serverType[0].serverName);
			for(int i=0;i<flavorTypeNum;i++)
			{
				usedServer[0][needGeneralserverNum].flavorList[flavorType[i]] = usedServer[1][needLargeserverNum-1].flavorList[flavorType[i]] ;
			}					
			needLargeserverNum -= 1;
			needGeneralserverNum += 1;
		}
		else                                                                                                     
		{
			if((cpuSizeOfLastSever <= serverType[2].cpusize)&&((memSizeOfLastSever <= serverType[2].memsize)))          //Large
			{
				int tmprate = 0;
				utilizeRateOfLastSever = (((float)cpuSizeOfLastSever/(float)serverType[1].cpusize) + ((float)memSizeOfLastSever/(float)serverType[1].memsize))/2.0;
				tmprate = (((float)cpuSizeOfLastSever/(float)serverType[2].cpusize) + ((float)memSizeOfLastSever/(float)serverType[2].memsize))/2.0;
				if(tmprate>utilizeRateOfLastSever)
				{
					for(int i=0;i<flavorTypeNum;i++)
					{
						usedServer[2][needHighserverNum].flavorList[flavorType[i]] = usedServer[1][needLargeserverNum-1].flavorList[flavorType[i]] ;
					}					
					needLargeserverNum -= 1;
					needHighserverNum += 1;
				}
			}
		}
	}
	
	//检测最后一个物理机器到底用了多少利用率  看另外2中宿主机 如果利用率可以更高，更换最后一台为利用率最高的

	//生成 宿主机上 VM列表
	needSever[0] = needGeneralserverNum;
	needSever[1] = needLargeserverNum;
	needSever[2] = needHighserverNum;
}

// void change(void)
// {
// 	//随机挑两个宿主机上面的虚拟机，互换位置
// }

// void IsBest(void)
// {
//  	//计算有没有宿主机超资源限制了， 如果有，非最优解

// 	 //对比互换前和互换后的利用率差值是否变大，如果变大，则更优秀。

// 	 //检查变大的这个宿主机资源能否 再装一个VM，并且装完后， 利用率差值更大。
// }
// //检查一下是否可以删掉/缩容 最后一个宿主机

void printData(float Data[][19],int maxDate)
{
	for(int i=1;i<=maxDate;i++)
	{
		printf("%d ",i);
		for(int j=1;j<19;j++)
		{
			printf("%d\t",(int)Data[i][j]);
		}
		printf("\n");
	}
}

void printData(int Data[][2],int maxDate)
{
	for(int i=1;i<=maxDate;i++)
	{
		printf("%d\t%d\t",Data[i][0],Data[i][1]);
		printf("\n");
	}
}

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int dataNum, char *fileName)
{

	int cnt = 0;
	int flag = 0;	
	if(data == NULL)
	{
		printf("data information is none\n");
		return ;
	}
	if(info == NULL)
	{
		printf("input file information is none\n");
		return ;
	}	

	int trainData[dataNum+100][3];
	for(int i=0;i<3;i++)
	{
		trainData[0][i]=0;		
	}		

	int startDateOrigin=0;	
	for(int index=0;index<dataNum;index++)
	{		
		char *item;
		cnt += 1;
		item = data[index];
		char mac[30];		
		char flavorType[30];		
		char Date[30];	
		int year,month,day;			
		int flavor;
		sscanf(item,"%s %s %s",mac,flavorType,Date);		
		sscanf(flavorType,"%*[^r]r%d",&flavor);	
		sscanf(Date,"%d-%d-%d",&year,&month,&day);
		int date;
		date = cal(year,month,day);		
		if(cnt == 1)
		{			
			startDateOrigin = date - 1;
		}
		date = date - startDateOrigin;
		if(flavor >= 19)
		{
			cnt -= 1;
			continue;
		}
		trainData[cnt][0] = flavor;
		trainData[cnt][1] = date;
		trainData[cnt][2] = cnt;	
	}	

	int maxDate = trainData[cnt][1];
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
		int date,flavor;		
		date = trainData[i][1];
		flavor = trainData[i][0];				
		Data[date][flavor] += 1;	
	}	

	int flavorTypeNum;
	int serverTypeNum;
	int typeList[19];
	int startYear,startMonth,startDay;
	int endYear,endMonth,endDay;
	Server serverType[3];

	for(int index =0;;index++)
	{
		char *item;	
		item = info[index];

		if(item == NULL)
		{
			break;
		}
		if(index == 0)
		{
			sscanf(item,"%d",&serverTypeNum);              //提取服务器类型数
		}
		if((strstr(item,"General")!=NULL))
		{
			sscanf(item,"%s[^ ]",serverType[0].serverName);
			sscanf(item,"%*[^ ] %d %d",&serverType[0].cpusize,&serverType[0].memsize);
			serverType[0].rate = (float)serverType[0].memsize / serverType[0].cpusize;
		}	
		if((strstr(item,"Large")!=NULL))
		{
			sscanf(item,"%s[^ ]",serverType[1].serverName);
			sscanf(item,"%*[^ ] %d %d",&serverType[1].cpusize,&serverType[1].memsize);
			serverType[1].rate = (float)serverType[1].memsize / serverType[1].cpusize;
		}	
		if((strstr(item,"High")!=NULL))
		{
			sscanf(item,"%s[^ ]",serverType[2].serverName);
			sscanf(item,"%*[^ ] %d %d",&serverType[2].cpusize,&serverType[2].memsize);
			serverType[2].rate = (float)serverType[2].memsize / serverType[2].cpusize;
		}	
		if(index == 5)
		{
			sscanf(item,"%d",&flavorTypeNum);						
		}
		if(strstr(item,"flavor") != NULL)
		{
			int flavor;
			static int index=0;
			sscanf(item,"%*[^r]r%d",&flavor);	
			typeList[index] = flavor;
			index++;	
		}			
		if((strstr(item,"201") != NULL) && (flag == 0))
		{
			flag = 1;
			sscanf(item,"%d-%d-%d",&startYear,&startMonth,&startDay);
		}
		if((strstr(item,"201") != NULL) && (flag == 1))
		{			
			sscanf(item,"%d-%d-%d",&endYear,&endMonth,&endDay);	
		}
	}

	int needServer[serverTypeNum];
	int startDate,endDate;
	int predictTime;
	int breakTime;
	startDate = cal(startYear,startMonth,startDay);
	endDate = cal(endYear,endMonth,endDay);
	startDate -= startDateOrigin;
	endDate -= startDateOrigin;
	predictTime = endDate - startDate + 1;
	breakTime = startDate - maxDate;

	for(int k=0;k<predictTime+breakTime;k++)
	{
		int predictDay = maxDate + k;
		for(int i=0;i<flavorTypeNum;i++)
		{
			float tmp = 0;
			float avg = 0;
			int startDay = predictDay - 30;		
			for(int j=0;j<30;j++)
			{
				tmp += Data[startDay+j][typeList[i]];
			}
			avg = tmp / 30;
			Data[predictDay][typeList[i]] = avg;
		}
	}

	float totalRate = 0;
	int start14day = 0, end14day = 0;
	for(int i=0;i<14;i++)
	{
		for(int j=0;j<flavorTypeNum;j++)
		{
			start14day += Data[i][typeList[j]];			
		}
	}

	for(int i=0;i<14;i++)
	{
		for(int j=0;j<flavorTypeNum;j++)
		{
			end14day += Data[maxDate-i][typeList[j]];			
		}
	}
	totalRate = (float)end14day / (float)start14day;
    float dayRate = (totalRate - 1) / (maxDate - 14);

	int predictList[19][2];
	for(int i=0;i<19;i++)
	{
		predictList[i][0] = 0;
		predictList[i][1] = 0;
	}
	int totaltmp = 0;
	for(int i=0;i<flavorTypeNum;i++)
	{
		float tmp = 0;
		for(int j=0;j<predictTime;j++)
		{			
			tmp += Data[maxDate+j+breakTime][typeList[i]];
		}		
		float tmpN = dayRate * (maxDate + breakTime + predictTime - 7) + 1;
		if(tmpN > 1.55)                           //参数已经是最优
		{
			tmpN = 1.55;
		}
		if(tmpN < 1.5)
		{
			tmpN = 1.5;
		}		
		tmp *= tmpN;	
		totaltmp += (int)tmp;
		predictList[typeList[i]][0] = typeList[i];
		predictList[typeList[i]][1] = (int)tmp;
	}		
	int TEST[19]={0, -10, 13, 0, 0, 0,   0, 7, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0};
     
	//                                   ~        ?         ~  
	for(int i=0;i<19;i++)
	{
		if(predictList[i][1]!=0)
		{
			predictList[i][1]+=TEST[i];
			totaltmp += TEST[i];
		}
	}		
	
	char resultFile[500000] ="\0";
	char flavortotal[200];
	sprintf(flavortotal,"%d",totaltmp);
	strcat(flavortotal,"\n");
	strcat(resultFile,flavortotal); 	

	for(int i=0;i<flavorTypeNum;i++)                                   //依次将结果写入文件
	{
		char tmp2[5],tmp3[10];
		char tmp1[20] = "flavor";
		sprintf(tmp2,"%d",typeList[i]);
		strcat(tmp1,tmp2); 
        strcat(tmp1," ");
		sprintf(tmp3,"%d",predictList[typeList[i]][1]);
		strcat(tmp1,tmp3);
		strcat(tmp1,"\n");
		strcat(resultFile,tmp1);			
	}
	strcat(resultFile,"\n");
	int totalCPU = 0,totalMEM = 0;
	for(int i=0;i<flavorTypeNum;i++)
	{
		totalCPU += predictList[typeList[i]][1] * CPUTYPE[typeList[i]];
		totalMEM += predictList[typeList[i]][1] * MEMTYPE[typeList[i]];
	}
	// predictList[6][1] -= 5;
	// totaltmp -= 5;
	InitSelecteServer(predictList,typeList,totaltmp,flavorTypeNum,serverType,needServer);

	// printf("%s\n",resultFile);	
	// int count[19];
	// for(int i=1;i<3;i++)
	// {	
	// 	for(int k=1;k<19;k++)
	// 	{
	// 		for(int j=0;j < needServer[i]; j++)
	// 		{
	// 			 count[k] += usedServer[i][j].flavorList[k];
	// 		}
			
	// 	}
		
	// }
	// int totalcount = 0; 
	// for(int i=1;i<19;i++)
	// {
	// 	totalcount += count[i];
	// 	printf("%d %d\n",i ,count[i]);
	// }
	// printf("%d \n",totalcount);
	//  return ;
	// int totalsevercpu=0,totalsevermem=0;
	// float userate=0.0;
	// for(int i=0; i<3;i++)
	// {		
	// 	totalsevercpu += serverType[i].cpusize*needServer[i];	
	// 	totalsevermem += serverType[i].memsize*needServer[i];		
	// }
	// //printf("%d %d\n",totalsevercpu,totalsevermem);

	// userate =(((float)totalCPU/(float)totalsevercpu) + ((float)totalCPU/(float)totalsevercpu))/2;
	// //printf("%f\n",userate);
	
	// for(int k=0;k<flavorTypeNum1;k++)
	// {
	// 	printf("%d  ",typeList[k]);
	// }
	// return ;

	char resultFile1[400000]="\0";		
	for(int i=0;i<3;i++)
	{
		if(needServer[i] != 0)       //
		{			
			char serverNum[10] = "\0";
			strcat(resultFile1,usedServer[i][0].serverName);
			strcat(resultFile1," ");
			sprintf(serverNum,"%d",needServer[i]);
			strcat(resultFile1,serverNum);
			strcat(resultFile1,"\n");			
		}	
		else
		{
			continue ;
		}		
		 for(int j=0;j<needServer[i];j++)
		 {
			char labelType[100] = "\0";
			char labelNum[10] = "\0";
			strcat(labelType,usedServer[i][j].serverName);			
			strcat(labelType,"-");
			sprintf(labelNum,"%d",j+1);
			strcat(labelType,labelNum);			
			strcat(labelType," ");			
			strcat(resultFile1,labelType);
			for(int k=0;k<flavorTypeNum;k++)
			{
				if(usedServer[i][j].flavorList[typeList[k]] != 0)
				{
					char tmp1[20] = "flavor";
					char tmp2[5] = "\0";
					char tmp3[10] = "\0";
					sprintf(tmp2,"%d",typeList[k]);
					strcat(tmp1,tmp2);
					strcat(tmp1," ");				
					sprintf(tmp3,"%d",usedServer[i][j].flavorList[typeList[k]]);
					strcat(tmp1,tmp3);					
					strcat(resultFile1,tmp1);					
					strcat(resultFile1," ");
				}
			}
			strcat(resultFile1,"\n");		
		 }	
		strcat(resultFile1,"\n");	
	}		
	strcat(resultFile,resultFile1);
	printf("%s\n",resultFile);
	write_result(resultFile,fileName);
}
