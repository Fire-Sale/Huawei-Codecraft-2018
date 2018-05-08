import math
def predict_vm(ecs_lines, input_lines):
    # Do your work from here#

    result = []
    cnt = 0
    flag = 0
    mode = 0 # mode 1 = optimize CPU, mode 2 = optimize MEM
    typeList = []
    trainData = []
    dayList = []
    data = []
    
    RUNYEAR = 0 # wheather a year is
    CPUTYPE = [0,1,1,1,2,2,2,4,4,4,8,8,8,16,16,16]
    MEMTYPE = [0,1,2,4,2,4,8,4,8,16,8,16,32,16,32,64]
    DAYNUM2 = [0,31,28,31,30,31,30,31,31,30,31,30,31]
    DAYNUM1 = [0,31,27,31,30,31,30,31,31,30,31,30,31]
    YEARS = [0,365,365,365,365,366,365,365,365,366,365,365,365,366,365,365,365,366,365,365,365,366]
    
    '''
    In order to process data across years, I let (2000,1,1) to be the 1st day, then calculate the date of 
    1st day of train data(let it be x), and then every date of train data = b - x + 1(b is the date transformed 
    according to (2000,1,1)
    '''

    def cal(year, month, day):
        RUNYEAR = 0
        if int(year) % 4 == 0:
            RUNYEAR = 1;
        if RUNYEAR == 0:
            datetmp = 0
            for i in range(1,(int)(month)):
                datetmp += DAYNUM1[i]
            datetmp += (int)(day)
        if RUNYEAR == 1:
            datetmp = 0
            for i in range(1,(int)(month)):
                datetmp += DAYNUM2[i]
            datetmp += (int)(day)
        for i in range (1,(int)(year)-2000+1):
            datetmp += (int)(YEARS[i])
        return datetmp


    if ecs_lines is None:
        print 'ecs information is none'
        return result
    if input_lines is None:
        print 'input file information is none'
        return result

    '''
    Read train data
    in this for loop our target is to get trainData
    which store data like this trainData = [[flavor1, date1, cnt1],[flacor2, date2, cnt2],...]
    '''

    trainData.append(0)
    
    for index, item in enumerate(ecs_lines): 
        cnt += 1                      
        values = item.split("\t")
        values[1] = values[1].replace('flavor','')
        if (int)(values[1]) >= 16:
            cnt -= 1
            continue       
        create = values[2].split(" ")
        year = values[2][0:4]
        month = values[2][5:7]
        day = values[2][8:10]
        date = cal(year, month, day)
        if cnt == 1:
            startDate = date-1
        date = date-startDate
        dayList.append(date)
        trainData.append([(int)(values[1]), date, cnt]) # trainData = [[type1, date1, cnt1],[]]
    
    maxDate = dayList[-1] # Last day of train data, i.e. the day before 1st day to be predicted
    data.append(0)
    for i in range(1, maxDate+20):
        data.append([i,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0])
    for i in range(1, cnt+1):
        date = trainData[i][1]
        flavor = trainData[i][0]
        data[date][flavor] += 1 # data = [[day1, type1num,..., type 15num],[day2,type1num,...],[]]
    #result.append(data)
    for index, item in enumerate(input_lines):
        print "index of input data"
        if index == 0:
            values = str(item)
            values = ' '.join(values.split())
            values = values.split(' ')
            cpuNum = values[0]
            memNum = values[1]
            diskNum = values[2]
        if index == 2:
            typeNum = item.split('\n')
        if item.split(' ')[0].find('flavor') != -1:
            tmp = item.split(' ')[0].replace('flavor','')
            typeList.append((int)(tmp))
        if item.find('CPU') != -1:
            mode = 1
        if item.find('MEM') != -1:
            mode = 2
        if item.split(' ')[0].find('-') != -1 and flag == 0:
            startDate = item.split(' ')[0]
            startYear = (int)(startDate[0:4])
            startMonth = (int)(startDate[5:7])
            startDay = (int)(startDate[8:10])
            flag = 1
        if item.split(' ')[0].find('-') != -1 and flag == 1:
            endDate = item.split(' ')[0]
            endYear = (int)(endDate[0:4])
            endMonth = (int)(endDate[5:7])
            endDay = (int)(endDate[8:10])
    
    
    
    
    
    
    
    
    flavorNum=[]
    for i in range (0,16):
        flavorNum.append(0)
    for i in range (1,16):
        flavorNum[i] = [ 0 for j in range(0,maxDate+1)]
        
    for i in range (1,16):
        for j in range (1,maxDate+1):
            flavorNum[i][j] = data[j][i]
    #result.append(flavorNum)
    
    
    

    
    
    
    
    typeList.sort()
    #result.append(typeList)
    
    
    
    currentTime = maxDate + 1
    startDate = cal(startYear, startMonth, startDay)
    endDate = cal(endYear, endMonth, endDay)
    predictTime = endDate - startDate
    #result.append(predictTime)
    # Prediction Process
    '''
    We store the predicted data in data[maxdate][]-data[maxdate+predictTime][]
    '''
    
    total = 0
    for i in range(1,predictTime+1):
        for j in typeList:
            tot = 0
            for k in range (i,maxDate+i+1):
                tot += data[k][j]
            tot = math.floor((tot/(maxDate))+0.5)
            total += tot
            data[maxDate+i][j] = tot
            data[maxDate+i][j] += 3
    #result.append(data)
    predictList = []
    totaltmp = 0 # total machines needed in all predictTime and all flavor
    for i in range (0,16):
        predictList.append(0)
    for i in typeList:
        tmp = 0
        for j in range(1,predictTime+1):
            tmp += data[maxDate+j][i]
        totaltmp += tmp
        predictList[i] = [i,tmp]
    result.append((int)(totaltmp))  #predictList stores data like [0, [1,flavor1Num],[2,flavor2Num],...]
    for i in typeList:
        result.append('flavor'+str(predictList[i][0])+' '+str((int)(predictList[i][1])))
    result[-1] += '\n'        
    
    copy = predictList
    reTypeList = typeList
    reTypeList.reverse()
    
    bag = []
    for i in range (0,10000):
        bag.append([(int)(cpuNum),(int)(memNum),(int)(diskNum)])
    
    
    
    
    
    curBag = 1 # currentBag
    usedBag = []
    for i in range (0,10000):
        usedBag.append([i,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0])
    
    '''
    for i in reTypeList:
        while copy[i][1]>0:
            if bag[curBag][0]-CPUTYPE[i]>=0 and bag[curBag][1]-MEMTYPE[i]>=0:
                copy[i][1] -= 1
                bag[curBag][0] -= CPUTYPE[i]
                bag[curBag][1] -= MEMTYPE[i]
                usedBag[curBag][i] += 1
            else:
                curBag += 1
    '''       
    for index, i in enumerate(reTypeList):
        tmp = index
        while copy[i][1]>0:
            for j in range(tmp, len(reTypeList)):
                #if copy[reTypelist[j]][1]>0 and bag[curBag][0]-CPUTYPE[reTypelist[j]]>=0 and bag[curBag][1]-MEMTYPE[reTypelist[j]]>=0:
                while copy[reTypeList[j]][1]>0:
                    if bag[curBag][0]-CPUTYPE[reTypeList[j]]>=0 and bag[curBag][1]-MEMTYPE[reTypeList[j]]>=0:
                        copy[reTypeList[j]][1] -= 1
                        bag[curBag][0] -= CPUTYPE[reTypeList[j]]
                        bag[curBag][1] -= MEMTYPE[reTypeList[j]]
                        usedBag[curBag][reTypeList[j]] += 1
                    else:
                        break
                if j == len(reTypeList)-1:
                    curBag += 1
            
            
    
    curBag -= 1
    
    '''
    for index,i in enumerate(reTypeList):
        while copy[i][1]>0:
            if bag[curBag][0]-CPUTYPE[i]>=0 and bag[curBag][1]-MEMTYPE[i]>=0:
                copy[i][1] -= 1
                bag[curBag][0] -= CPUTYPE[i]
                bag[curBag][1] -= MEMTYPE[i]
                usedBag[curBag][i] += 1
            else:
                tmp = index
                if tmp + 1 <= len(reTypelist):
                    tmp += 1
                    while tmp <= len(reTypelist):
                        if copy[reTypelist[tmp]][1]>0 and bag[curBag][0]-CPUTYPE[reTypelist[tmp]]>=0 and bag[curBag][1]-MEMTYPE[reTypelist[tmp]]>=0:
                            copy[reTypelist[tmp]][1] -= 1
                            bag[curBag][0] -= CPUTYPE[reTypelist[tmp]]
                            bag[curBag][1] -= MEMTYPE[reTypelist[tmp]]
                            usedBag[curBag][reTypelist[tmp]] += 1
    '''
                
                
                
                
    result.append(curBag)
    typeList.reverse()
    #result.append(typeList)
    for i in range(1,curBag+1):
        out = str(i)+' '
        for j in typeList:
            if usedBag[i][j] != 0:
                out += 'flavor'+str(j)+' '+str(usedBag[i][j])+' '
        out = out.rstrip(' ')
        result.append(out)
    #result.append('This is a TEST!')
    return result
