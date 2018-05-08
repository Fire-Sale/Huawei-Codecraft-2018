import math
def predict_vm(ecs_lines, input_lines):
    # Do your work from here#
    result = []
    uuid = []
    flavorName = []
    createDate = []
    createTime = []
    createDay = []
    createMonth = []
    cnt = 0
    flag = 0
    mode = -1
    typeList = []
    trainData = []
    dayList = []
    data = []
    RUNYEAR = 0
    YEARS = [0,365,365,365,365,366,365,365,365,366,365,365,365,366,365,365,365,366,365,365,365,366]
    DAYNUM1 = [0,31,27,31,30,31,30,31,31,30,31,30,31]
    DAYNUM2 = [0,31,28,31,30,31,30,31,31,30,31,30,31]
    cpuType = [0,1,1,1,2,2,2,4,4,4,8,8,8,16,16,16]
    memType = [0,1,2,4,2,4,8,4,8,16,8,16,32,16,32,64]
    test =[]
    if ecs_lines is None:
        print('ecs information is none')
        return result
    if input_lines is None:
        print('input file information is none')
        return result

    trainData.append(0)
    for index, item in enumerate(ecs_lines):
        cnt+=1
        values = item.split("\t")
        uuid.append(values[0])
        values[1] = values[1].replace('flavor','')
        flavorName.append(values[1])
        create = values[2].split(" ")
        
        year = values[2][0:4]
        month = values[2][5:7]
        day = values[2][8:10]
        
        if ((int)(year[0])+(int)(year[1])+(int)(year[2])+(int)(year[3]))%4==0:
            RUNYEAR = 1;
        if RUNYEAR == 0:
            date = 0
            for i in range(1,(int)(month)):
                date += DAYNUM1[i]
            date += (int)(day)
        if RUNYEAR == 1:
            date = 0
            for i in range(1,(int)(month)):
                date += DAYNUM2[i]
            date += (int)(day)
        for i in range (1,(int)(year)-2000+1):
            date += (int)(YEARS[i])
        if cnt == 1:
            startDate_ = date-1
        date = date-startDate_
        #createDay.append(values[2][8:10])
        #createMonth.append(values[2][5:7])
        dayList.append(date)
        trainData.append([(int)(values[1]),date,cnt])#type date cnt
    
    maxDate = dayList[-1]
    data.append(0)
    for i in range(1, maxDate+8):
        data.append([i,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0])
        
    for i in range(1, cnt):
        date = trainData[i][1]
        flavor = trainData[i][0]
  #      #test.append([date,flavor])
        data[date][flavor] += 1
        
        
    for index, item in enumerate(input_lines):
        print("index of input data")
        if index == 0:
            values = item.split(' ')
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
            startMonth = (int)(startDate[5:7])
            startDay = (int)(startDate[8:10])
            
            flag = 1
        if item.split(' ')[0].find('-') != -1 and flag == 1:
            endDate = item.split(' ')[0]
            endMonth = (int)(endDate[5:7])
            endDay = (int)(endDate[8:10])
        
    currentTime = maxDate + 1
    total = 0
    predictTime = endDay - startDay
    for i in range(1,predictTime+1):
        for j in typeList:
            tot = 0
            for k in range (maxDate-20+i,maxDate+i+1):
                tot += data[k][j]
            tot = math.floor((tot/(21))+0.5)
            total += tot
            data[maxDate+i][j] = tot
    #result.append(data)
    predictList = []
    totaltmp = 0
    for i in range (0,16):
        predictList.append(0)
    for i in typeList:
        tmp = 0
        for j in range(1,predictTime+1):
            tmp += data[maxDate+j][i]
        totaltmp += tmp
        predictList[i] = [i,tmp]
    #result.append(predictList)
    result.append(totaltmp)
    for i in typeList:
        result.append('flavor'+str(predictList[i][0])+' '+str(predictList[i][1]))
    #result.append(predictList)
    result[-1] += '\n'
    
    
    copy = predictList
    re = typeList
    re.reverse()
    
    bag = []
    for i in range (0,100):
        bag.append([(int)(cpuNum),(int)(memNum),diskNum])
    
    curBag = 1
    usedBag = []
    for i in range (0,100):
        usedBag.append([i,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
    for i in re:
        while copy[i][1]>0:
            if bag[curBag][0]-cpuType[i]>=0 and bag[curBag][1]-memType[i]>=0:
                copy[i][1] -= 1
                bag[curBag][0] -= cpuType[i]
                bag[curBag][1] -= memType[i]
                usedBag[curBag][i] += 1
            else:
                curBag += 1
                
    result.append(curBag)
    for i in range(1,curBag+1):
        char = str(i)+' '
        for j in typeList:
            if usedBag[i][j] != 0:
                char += 'flavor'+str(j)+' '+str(usedBag[i][j])+' '
        result.append(char)
        
    return result
